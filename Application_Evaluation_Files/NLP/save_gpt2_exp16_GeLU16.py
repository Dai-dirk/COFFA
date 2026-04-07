import argparse
import json
import shutil
from pathlib import Path
from typing import Tuple

import textwrap

import torch
from torch import nn
from transformers import AutoTokenizer, GPT2LMHeadModel

SCRIPT_DIR = Path(__file__).resolve().parent
LNS_DIR = SCRIPT_DIR / "LNS"
GELU_TABLE = LNS_DIR / "gelu_1616_C_table.pt"
EXP_TABLE = LNS_DIR / "exp_1616_C_table.pt"

MODULES_DIR_NAME = "modules"
MODEL_MODULE_NAME = "modeling_gpt2_gelu16"
MODEL_CLASS_NAME = "GeLU16GPT2LMHeadModel"
DEFAULT_SOURCE_REPO = "openai-community/gpt2-xl"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export GPT-2 with GeLU16/Exp16 lookup activations.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        required=False,
        default="/home/gbzou/models/gpt2-xl-exp16-gelu16",
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
    missing = [p for p in (GELU_TABLE, EXP_TABLE) if not p.exists()]
    if missing:
        raise FileNotFoundError(f"Lookup table(s) not found: {missing}")


def _load_lookup_table(path: Path) -> Tuple[torch.Tensor, float, float, float]:
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


class GeLU16Lookup(nn.Module):
    def __init__(self, table_path: Path | str):
        super().__init__()
        values, lower, upper, scale = _load_lookup_table(Path(table_path))
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("GeLU16Lookup expects a floating point tensor.")

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
            values = self.lookup_values
            if values.device != mid_vals.device:
                values = values.to(mid_vals.device)
            result[mask_mid] = values[idx]

        return result.to(dtype=x.dtype, device=x.device)


def replace_gelu_with_lookup(model: GPT2LMHeadModel, table_path: Path | str) -> None:
    for block in model.transformer.h:
        act_device = block.mlp.c_fc.weight.device
        block.mlp.act = GeLU16Lookup(table_path).to(act_device)


def write_modules_package(destination: Path) -> None:
    modules_dir = destination / MODULES_DIR_NAME
    modules_dir.mkdir(parents=True, exist_ok=True)

    module_content = textwrap.dedent(
        """
        from pathlib import Path
        from typing import Tuple

        import torch
        import torch.nn.functional as F
        from torch import nn
        from transformers import GPT2LMHeadModel


        def _load_lookup_table(path: Path) -> Tuple[torch.Tensor, float, float, float]:
            payload = torch.load(path, map_location="cpu")
            if isinstance(payload, dict):
                values = payload.get("values", payload.get("table"))
                if values is None:
                    raise KeyError(f"'values' missing in lookup table {path}")
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
                raise TypeError(f"Unsupported lookup table type: {type(payload)}")
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
            raise FileNotFoundError(f"Lookup table '{filename}' not found. Checked: {candidates}")


        class GeLU16Lookup(nn.Module):
            def __init__(self, table_path: Path | str):
                super().__init__()
                values, lower, upper, scale = _load_lookup_table(Path(table_path))
                self.lower_bound = lower
                self.upper_bound = upper
                self.scale = scale
                self.register_buffer("lookup_values", values, persistent=False)

            def forward(self, x: torch.Tensor) -> torch.Tensor:
                if not torch.is_floating_point(x):
                    raise TypeError("GeLU16Lookup expects a floating point tensor.")
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
                    values = self.lookup_values
                    if values.device != mid_vals.device:
                        values = values.to(mid_vals.device)
                    result[mask_mid] = values[idx]
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
                    values = self.lookup_values
                    if values.device != mid_vals.device:
                        values = values.to(mid_vals.device)
                    result[mask_mid] = values[idx]
                mask_hi = work > self.upper_bound
                if mask_hi.any():
                    result[mask_hi] = torch.exp(work[mask_hi])
                return result.to(dtype=x.dtype, device=x.device)


        def _replace_gelu_with_lookup(model: GPT2LMHeadModel, table_path: Path | str) -> None:
            table = GeLU16Lookup(table_path)
            for block in model.transformer.h:
                act_device = block.mlp.c_fc.weight.device
                block.mlp.act = table.to(act_device)


        class GeLU16GPT2LMHeadModel(GPT2LMHeadModel):
            def __init__(self, config):
                super().__init__(config)
                gelu_table = _resolve_lookup_path(config, config.gelu_lookup_table)
                _replace_gelu_with_lookup(self, gelu_table)

            @classmethod
            def from_pretrained(cls, pretrained_model_name_or_path, *model_args, **kwargs):
                # Avoid Accelerate device_map + meta tensors (breaks custom GeLU lookup buffers).
                kwargs = dict(kwargs)
                kwargs["device_map"] = None
                kwargs["low_cpu_mem_usage"] = False
                model = super().from_pretrained(pretrained_model_name_or_path, *model_args, **kwargs)
                gelu_table = _resolve_lookup_path(model.config, model.config.gelu_lookup_table)
                _replace_gelu_with_lookup(model, gelu_table)
                return model
        """
    )

    root_module_path = destination / f"{MODEL_MODULE_NAME}.py"
    root_module_path.write_text(module_content, encoding="utf-8")

    module_path = modules_dir / f"{MODEL_MODULE_NAME}.py"
    module_path.write_text(module_content, encoding="utf-8")

    init_file = modules_dir / "__init__.py"
    init_file.write_text("from ..modeling_gpt2_gelu16 import GeLU16GPT2LMHeadModel\n", encoding="utf-8")


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
    max_memory = {i: args.max_memory for i in range(torch.cuda.device_count())} if device.type == "cuda" else None

    tokenizer = AutoTokenizer.from_pretrained(args.source_repo, use_fast=True, local_files_only=True)
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token

    model = GPT2LMHeadModel.from_pretrained(
        args.source_repo,
        torch_dtype=torch.float32,
        device_map="auto" if max_memory else None,
        max_memory=max_memory,
        local_files_only=True,
    )

    replace_gelu_with_lookup(model, GELU_TABLE)
    model.config.pad_token_id = tokenizer.pad_token_id

    model.save_pretrained(output_dir, safe_serialization=True)
    tokenizer.save_pretrained(output_dir)

    shutil.copy2(GELU_TABLE, output_dir / GELU_TABLE.name)
    shutil.copy2(EXP_TABLE, output_dir / EXP_TABLE.name)

    write_modules_package(output_dir)
    patch_config(output_dir / "config.json")

    print(f"Model exported to {output_dir}")
    print("Next steps:")
    print(
        "  lm_eval --model hf --model_args pretrained="
        f"{output_dir},trust_remote_code=True,device_map=False --tasks arc_easy,hellaswag,commonsense_qa,copa --device cuda"
    )


if __name__ == "__main__":
    main()

