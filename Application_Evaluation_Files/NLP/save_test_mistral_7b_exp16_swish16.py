import argparse
import json
import shutil
import textwrap
from pathlib import Path

import torch
from transformers import AutoModelForCausalLM, AutoTokenizer


SCRIPT_DIR = Path(__file__).resolve().parent
LNS_DIR = SCRIPT_DIR / "LNS"
SWISH_TABLE = LNS_DIR / "swish_1616_C_table.pt"
EXP_TABLE = LNS_DIR / "exp_1616_C_table.pt"

DEFAULT_OUTPUT_DIR = Path("/capsule/home/huangdaiwei/test_zgb/model/mistral_7b_exp16_swish16")
DEFAULT_SOURCE_REPO = "mistralai/Mistral-7B-v0.1"

MODEL_MODULE_NAME = "modeling_mistral_swish16_exp16"
MODEL_CLASS_NAME = "Swish16Exp16MistralForCausalLM"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export Mistral 7B with Swish16/Exp16 lookup activations.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=f"Destination directory for the packaged model (default: {DEFAULT_OUTPUT_DIR}).",
    )
    parser.add_argument(
        "--source-repo",
        type=str,
        default=DEFAULT_SOURCE_REPO,
        help="Base model identifier or local path. Defaults to the cached Mistral-7B snapshot.",
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
        default="39GiB",
        help="Per-GPU max memory budget passed to transformers (e.g., 39GiB).",
    )
    return parser.parse_args()


def ensure_tables_exist() -> None:
    missing = [path for path in (SWISH_TABLE, EXP_TABLE) if not path.exists()]
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


class Swish16Lookup(torch.nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Swish16Lookup expects a floating point tensor.")

        work = x.to(torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        mask_mid = (~mask_lo) & (~mask_hi)

        result[mask_lo] = 0.0
        result[mask_hi] = work[mask_hi]

        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            lookup_values = self.lookup_values
            if lookup_values.device != mid_vals.device:
                lookup_values = lookup_values.to(mid_vals.device)
            result[mask_mid] = lookup_values[idx]

        return result.to(dtype=x.dtype, device=x.device)


class Exp16Lookup(torch.nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Exp16Lookup expects a floating point tensor.")

        work = x.to(torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= self.lower_bound
        mask_mid = (~mask_lo) & (work <= self.upper_bound)

        result[mask_lo] = 0.0

        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            lookup_values = self.lookup_values
            if lookup_values.device != mid_vals.device:
                lookup_values = lookup_values.to(mid_vals.device)
            result[mask_mid] = lookup_values[idx]

        mask_hi = work > self.upper_bound
        if mask_hi.any():
            result[mask_hi] = torch.exp(work[mask_hi])

        return result.to(dtype=x.dtype, device=x.device)


class SoftmaxApprox:
    def __init__(self, exp_lookup: Exp16Lookup):
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
    def __init__(self, exp_lookup: Exp16Lookup):
        self.exp_lookup = exp_lookup
        self._orig_softmax = torch.nn.functional.softmax
        self.softmax_fn = SoftmaxApprox(exp_lookup)

    def __enter__(self):
        functional = torch.nn.functional
        softmax_fn = self.softmax_fn

        def wrapped(input: torch.Tensor, dim=None, _stacklevel=3, dtype=None):
            return softmax_fn(input, dim=dim, dtype=dtype)

        functional.softmax = wrapped  # type: ignore[assignment]
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        torch.nn.functional.softmax = self._orig_softmax  # type: ignore[assignment]


def replace_silu_with_lookup(model: AutoModelForCausalLM, table_path: Path | str) -> None:
    if not hasattr(model, "model") or not hasattr(model.model, "layers"):
        raise AttributeError("Unexpected model structure; cannot locate decoder layers.")
    for block in model.model.layers:
        act_device = block.mlp.down_proj.weight.device
        lookup = Swish16Lookup(table_path)
        if act_device.type != "meta":
            lookup = lookup.to(act_device)
        block.mlp.act_fn = lookup


def write_modeling_module(destination: Path) -> None:
    module_template = """
from pathlib import Path
from typing import Tuple

import torch
import torch.nn.functional as F
from torch import nn
from transformers import MistralForCausalLM


def _load_lookup_table(path: Path) -> Tuple[torch.Tensor, float, float, float]:
    payload = torch.load(path, map_location="cpu")
    if isinstance(payload, dict):
        values = payload.get("values", payload.get("table"))
        if values is None:
            raise KeyError(f"'values' not found in lookup table '{{path}}'.")
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
        raise TypeError(f"Unsupported lookup table type: {{type(payload)}} from '{{path}}'.")
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


class Swish16Lookup(nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Swish16Lookup expects a floating point tensor.")
        work = x.to(torch.float32)
        result = torch.empty_like(work)
        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        mask_mid = (~mask_lo) & (~mask_hi)
        result[mask_lo] = 0.0
        result[mask_hi] = work[mask_hi]
        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            lookup_values = self.lookup_values
            if lookup_values.device != mid_vals.device:
                lookup_values = lookup_values.to(mid_vals.device)
            result[mask_mid] = lookup_values[idx]
        return result.to(dtype=x.dtype, device=x.device)


class Exp16Lookup(nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Exp16Lookup expects a floating point tensor.")
        work = x.to(torch.float32)
        result = torch.empty_like(work)
        mask_lo = work <= self.lower_bound
        mask_mid = (~mask_lo) & (work <= self.upper_bound)
        result[mask_lo] = 0.0
        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, self.lookup_values.numel() - 1)
            lookup_values = self.lookup_values
            if lookup_values.device != mid_vals.device:
                lookup_values = lookup_values.to(mid_vals.device)
            result[mask_mid] = lookup_values[idx]
        mask_hi = work > self.upper_bound
        if mask_hi.any():
            result[mask_hi] = torch.exp(work[mask_hi])
        return result.to(dtype=x.dtype, device=x.device)


class SoftmaxApprox:
    def __init__(self, exp_lookup: Exp16Lookup):
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
    def __init__(self, exp_lookup: Exp16Lookup):
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


def _replace_silu_with_lookup(model: MistralForCausalLM, table_path: Path | str) -> None:
    for block in model.model.layers:
        act_device = block.mlp.down_proj.weight.device
        lookup = Swish16Lookup(table_path)
        if act_device.type != "meta":
            lookup = lookup.to(act_device)
        block.mlp.act_fn = lookup


class {model_class}(MistralForCausalLM):
    def __init__(self, config):
        super().__init__(config)
        swish_table = _resolve_lookup_path(config, config.swish_lookup_table)
        exp_table = _resolve_lookup_path(config, config.exp_lookup_table)
        _replace_silu_with_lookup(self, swish_table)
        self._exp_lookup = Exp16Lookup(exp_table)

    def forward(self, *model_args, **kwargs):
        with SoftmaxOverride(self._exp_lookup):
            return super().forward(*model_args, **kwargs)

    @classmethod
    def from_pretrained(cls, pretrained_model_name_or_path, *model_args, **kwargs):
        model = super().from_pretrained(pretrained_model_name_or_path, *model_args, **kwargs)
        swish_table = _resolve_lookup_path(model.config, model.config.swish_lookup_table)
        exp_table = _resolve_lookup_path(model.config, model.config.exp_lookup_table)
        _replace_silu_with_lookup(model, swish_table)
        model._exp_lookup = Exp16Lookup(exp_table)
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
    config["swish_lookup_table"] = SWISH_TABLE.name
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
        cuda_count = torch.cuda.device_count()
        if cuda_count == 0:
            raise RuntimeError("No CUDA devices found.")
        max_memory = {i: args.max_memory for i in range(cuda_count)}
        device_map = "auto"
    else:
        max_memory = None
        device_map = None

    tokenizer = AutoTokenizer.from_pretrained(args.source_repo, use_fast=False, local_files_only=True)
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token

    model = AutoModelForCausalLM.from_pretrained(
        args.source_repo,
        torch_dtype=torch.float32,
        device_map=device_map,
        max_memory=max_memory,
        local_files_only=True,
    )

    replace_silu_with_lookup(model, SWISH_TABLE)
    exp_lookup = Exp16Lookup(EXP_TABLE)
    model.config.pad_token_id = tokenizer.pad_token_id
    model.config.swish_lookup_table = SWISH_TABLE.name
    model.config.exp_lookup_table = EXP_TABLE.name

    with SoftmaxOverride(exp_lookup):
        model.save_pretrained(output_dir, safe_serialization=True)

    tokenizer.save_pretrained(output_dir)

    shutil.copy2(SWISH_TABLE, output_dir / SWISH_TABLE.name)
    shutil.copy2(EXP_TABLE, output_dir / EXP_TABLE.name)

    write_modeling_module(output_dir)
    patch_config(output_dir / "config.json")

    print(f"Model exported to {output_dir}")
    print("Next steps:")
    print(
        "  python -m transformers-cli env  # verify environment\n"
        "  python - <<'PY'\n"
        f"from transformers import AutoModelForCausalLM\n"
        f"model = AutoModelForCausalLM.from_pretrained('{output_dir}', trust_remote_code=True)\n"
        "print('Loaded model:', type(model))\n"
        "PY"
    )


if __name__ == "__main__":
    main()


