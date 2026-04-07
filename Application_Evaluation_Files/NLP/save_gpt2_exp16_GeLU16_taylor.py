import argparse
import json
import shutil
import textwrap
from pathlib import Path

import torch
from transformers import AutoTokenizer, GPT2LMHeadModel


SCRIPT_DIR = Path(__file__).resolve().parent
LNS_DIR = SCRIPT_DIR / "LNS"
GELU_TABLE = LNS_DIR / "gelu_taylor_5_1616_table.pt"
EXP_TABLE = LNS_DIR / "exp_taylor_5_1616_16_10_table.pt"

MODEL_MODULE_NAME = "modeling_gpt2_gelu16_taylor"
MODEL_CLASS_NAME = "GeLU16TaylorGPT2LMHeadModel"
DEFAULT_SOURCE_REPO = "openai-community/gpt2-xl"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export GPT-2 with Taylor-series GeLU/Exp lookup activations.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        required=False,
        default="/home/gbzou/models/gpt2-xl-exp16-gelu16-taylor",
        help="Destination directory for the packaged model.",
    )
    parser.add_argument(
        "--source-repo",
        type=str,
        default=DEFAULT_SOURCE_REPO,
        help="Base model identifier or local path. Defaults to gpt2-xl.",
    )
    parser.add_argument(
        "--device",
        type=str,
        default="cuda",
        help="Device to host the model while exporting (e.g., cuda, cpu).",
    )
    parser.add_argument(
        "--max-memory",
        type=str,
        default="16GiB",
        help="Per-GPU max memory budget passed to transformers (e.g., 16GiB).",
    )
    return parser.parse_args()


def ensure_tables_exist() -> None:
    missing = [path for path in (GELU_TABLE, EXP_TABLE) if not path.exists()]
    if missing:
        raise FileNotFoundError(f"Lookup table(s) not found: {missing}")


def _load_lookup_table(path: Path) -> tuple[torch.Tensor, float, float, float]:
    payload = torch.load(path, map_location="cpu")
    if isinstance(payload, dict):
        values = payload.get("values", payload.get("table"))
        if values is None:
            raise KeyError(f"'values' not found in lookup table '{path}'.")
        values = values.view(-1).float()
        meta = payload.get("metadata", {})
        lower = float(meta.get("lower", -16.0))
        upper = float(meta.get("upper", 16.0))
        scale = float(meta.get("scale", float(1 << int(meta.get("frac_bits", 16)))))
    elif isinstance(payload, torch.Tensor):
        values = payload.view(-1).float()
        lower = -16.0
        upper = 16.0
        scale = float(values.numel() - 1) / (upper - lower)
    else:
        raise TypeError(f"Unsupported lookup table type: {type(payload)} from '{path}'.")
    return values, lower, upper, scale


class GeLUTaylorLookup(torch.nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = max(lower, -4.0)
        self.upper_bound = min(upper, 4.0)
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("GeLUTaylorLookup expects a floating point tensor.")

        work = x.to(torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        mask_mid = (~mask_lo) & (~mask_hi)

        result[mask_lo] = 0.0
        if mask_hi.any():
            result[mask_hi] = torch.round(work[mask_hi] * (1 << 16)) / float(1 << 16)

        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            values = self.lookup_values
            if values.device != result.device:
                values = values.to(result.device, non_blocking=True)
            result[mask_mid] = values[idx]

        return result.to(dtype=x.dtype, device=x.device)


def replace_gelu_with_lookup(model: GPT2LMHeadModel, table_path: Path | str) -> None:
    for block in model.transformer.h:
        lookup = GeLUTaylorLookup(table_path)
        act_device = block.mlp.c_fc.weight.device
        if act_device.type != "meta":
            lookup = lookup.to(act_device)
        block.mlp.act = lookup


def write_modeling_module(destination: Path) -> None:
    module_template = """
from pathlib import Path
from typing import Tuple

import torch
import torch.nn.functional as F
from torch import nn
from transformers import GPT2LMHeadModel


_Q16_SCALE = float(1 << 16)


def _quantize_q16_16(tensor: torch.Tensor) -> torch.Tensor:
    return torch.round(tensor * _Q16_SCALE) / _Q16_SCALE


def _load_lookup_table(path: Path) -> Tuple[torch.Tensor, float, float, float]:
    payload = torch.load(path, map_location="cpu")
    if isinstance(payload, dict):
        values = payload.get("values", payload.get("table"))
        if values is None:
            raise KeyError(f"'values' missing in lookup table {{path}}")
        values = values.view(-1).float()
        meta = payload.get("metadata", {{}})
        lower = float(meta.get("lower", -16.0))
        upper = float(meta.get("upper", 16.0))
        scale = float(meta.get("scale", float(1 << int(meta.get("frac_bits", 16)))))
    elif isinstance(payload, torch.Tensor):
        values = payload.view(-1).float()
        lower = -16.0
        upper = 16.0
        scale = float(values.numel() - 1) / (upper - lower)
    else:
        raise TypeError(f"Unsupported lookup table type: {{type(payload)}}")
    return values, lower, upper, scale


def _resolve_lookup_path(config, filename: str) -> Path:
    candidates = []
    name_or_path = getattr(config, "_name_or_path", None)
    if name_or_path is not None:
        candidates.append(Path(name_or_path) / filename)
    candidates.append(Path(__file__).resolve().parent / filename)
    candidates.append(Path(__file__).resolve().parent.parent / filename)
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError(f"Lookup table '{{filename}}' not found. Checked: {{candidates}}")


class GeLUTaylorLookup(nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = max(lower, -4.0)
        self.upper_bound = min(upper, 4.0)
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("GeLUTaylorLookup expects a floating point tensor.")
        work = x.to(torch.float32)
        result = torch.empty_like(work)
        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        mask_mid = (~mask_lo) & (~mask_hi)
        result[mask_lo] = 0.0
        if mask_hi.any():
            result[mask_hi] = _quantize_q16_16(work[mask_hi])
        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            values = self.lookup_values
            if values.device != result.device:
                values = values.to(result.device, non_blocking=True)
            result[mask_mid] = values[idx]
        return result.to(dtype=x.dtype, device=x.device)


class ExpTaylorLookup(nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = max(lower, -16.0)
        self.upper_bound = min(upper, 10.0)
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("ExpTaylorLookup expects a floating point tensor.")
        work = x.to(torch.float32)
        result = torch.empty_like(work)
        mask_lo = work <= self.lower_bound
        mask_mid = (~mask_lo) & (work <= self.upper_bound)
        result[mask_lo] = 0.0
        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            values = self.lookup_values
            if values.device != result.device:
                values = values.to(result.device, non_blocking=True)
            result[mask_mid] = values[idx]
        mask_hi = work > self.upper_bound
        if mask_hi.any():
            result[mask_hi] = torch.exp(work[mask_hi])
        return result.to(dtype=x.dtype, device=x.device)


class SoftmaxApprox:
    def __init__(self, exp_lookup: ExpTaylorLookup):
        self.exp_lookup = exp_lookup

    def __call__(self, input: torch.Tensor, dim: int | None = None, dtype=None) -> torch.Tensor:
        if dim is None:
            dim = -1
        work = input.to(torch.float32)
        max_vals, _ = work.max(dim=dim, keepdim=True)
        shifted = work - max_vals
        exp_vals = self.exp_lookup(shifted)
        sum_exp = exp_vals.sum(dim=dim, keepdim=True).clamp_min(1e-9)
        output = exp_vals / sum_exp
        if dtype is not None:
            output = output.to(dtype)
        else:
            output = output.to(input.dtype)
        return output


class SoftmaxOverride:
    def __init__(self, exp_lookup: ExpTaylorLookup):
        self.exp_lookup = exp_lookup
        self._orig_softmax = F.softmax
        self.softmax_fn = SoftmaxApprox(exp_lookup)

    def __enter__(self):
        softmax_fn = self.softmax_fn

        def wrapped(input: torch.Tensor, dim=None, _stacklevel=3, dtype=None):
            return softmax_fn(input, dim=dim, dtype=dtype)

        F.softmax = wrapped  # type: ignore[assignment]
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        F.softmax = self._orig_softmax  # type: ignore[assignment]


def _replace_gelu_with_lookup(model: GPT2LMHeadModel, table_path: Path | str) -> None:
    for block in model.transformer.h:
        lookup = GeLUTaylorLookup(table_path)
        act_device = block.mlp.c_fc.weight.device
        if act_device.type != "meta":
            lookup = lookup.to(act_device)
        block.mlp.act = lookup


class {model_class}(GPT2LMHeadModel):
    def __init__(self, config):
        super().__init__(config)
        gelu_table = _resolve_lookup_path(config, config.gelu_lookup_table)
        exp_table = _resolve_lookup_path(config, config.exp_lookup_table)
        _replace_gelu_with_lookup(self, gelu_table)
        self._exp_lookup = ExpTaylorLookup(exp_table)

    def forward(self, *model_args, **kwargs):
        with SoftmaxOverride(self._exp_lookup):
            return super().forward(*model_args, **kwargs)

    @classmethod
    def from_pretrained(cls, pretrained_model_name_or_path, *model_args, **kwargs):
        model = super().from_pretrained(pretrained_model_name_or_path, *model_args, **kwargs)
        gelu_table = _resolve_lookup_path(model.config, model.config.gelu_lookup_table)
        exp_table = _resolve_lookup_path(model.config, model.config.exp_lookup_table)
        _replace_gelu_with_lookup(model, gelu_table)
        model._exp_lookup = ExpTaylorLookup(exp_table)
        return model
"""

    module_content = textwrap.dedent(module_template).format(model_class=MODEL_CLASS_NAME)
    (destination / f"{MODEL_MODULE_NAME}.py").write_text(module_content, encoding="utf-8")


def patch_config(config_path: Path) -> None:
    config = json.loads(config_path.read_text())
    config["architectures"] = [MODEL_CLASS_NAME]
    config["auto_map"] = {
        "AutoModelForCausalLM": f"{MODEL_MODULE_NAME}.{MODEL_CLASS_NAME}",
    }
    config["gelu_lookup_table"] = GELU_TABLE.name
    config["exp_lookup_table"] = EXP_TABLE.name
    config_path.write_text(json.dumps(config, indent=2) + "\n")


def main() -> None:
    args = parse_args()
    ensure_tables_exist()

    output_dir: Path = args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    device = torch.device(args.device)
    if device.type == "cuda" and not torch.cuda.is_available():
        raise RuntimeError("CUDA device requested but not available.")

    if device.type == "cuda":
        max_memory = {i: args.max_memory for i in range(torch.cuda.device_count())}
        device_map = "auto"
    else:
        max_memory = None
        device_map = None

    tokenizer = AutoTokenizer.from_pretrained(args.source_repo, use_fast=True, local_files_only=True)
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token

    model = GPT2LMHeadModel.from_pretrained(
        args.source_repo,
        torch_dtype=torch.float32,
        device_map=device_map,
        max_memory=max_memory,
        local_files_only=True,
    )

    replace_gelu_with_lookup(model, GELU_TABLE)
    model.config.pad_token_id = tokenizer.pad_token_id

    model.save_pretrained(output_dir, safe_serialization=True)
    tokenizer.save_pretrained(output_dir)

    shutil.copy2(GELU_TABLE, output_dir / GELU_TABLE.name)
    shutil.copy2(EXP_TABLE, output_dir / EXP_TABLE.name)

    write_modeling_module(output_dir)
    patch_config(output_dir / "config.json")

    print(f"Model exported to {output_dir}")
    print("Next steps:")
    print(
        "  lm_eval --model hf --model_args pretrained="
        f"{output_dir},trust_remote_code=True,device_map=False --tasks arc_easy,hellaswag,commonsense_qa,copa --device {args.device}"
    )


if __name__ == "__main__":
    main()

