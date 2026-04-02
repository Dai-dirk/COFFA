import argparse
import os
from typing import Optional

import torch
import torch.nn as nn
from torchvision import datasets, transforms, models
from tqdm import tqdm

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LNS_DIR = os.path.join(SCRIPT_DIR, "LNS")
DEFAULT_EXP_TABLE = os.path.join(LNS_DIR, "exp_taylor_4_1616_16_10_table.pt")
_Q16_SCALE = float(1 << 16)


def _quantize_q16_16(tensor: torch.Tensor) -> torch.Tensor:
    return torch.round(tensor * _Q16_SCALE) / _Q16_SCALE


def _load_exp_table(path: str) -> tuple[torch.Tensor, float, float, float]:
    if not os.path.exists(path):
        raise FileNotFoundError(f"Exponential lookup table not found at '{path}'.")
    payload = torch.load(path, map_location="cpu")
    values = payload["values"].view(-1).float()
    meta = payload.get("metadata", {})
    lower = float(meta.get("lower", -16.0))
    upper = float(meta.get("upper", 10.0))
    scale = float(meta.get("scale", float(1 << int(meta.get("frac_bits", 16)))))
    return values, lower, upper, scale


class SigmoidTaylorLookup(nn.Module):
    def __init__(self, table_path: Optional[str] = None):
        super().__init__()
        table = table_path or DEFAULT_EXP_TABLE
        values, lower, upper, scale = _load_exp_table(table)
        self.exp_lower = lower
        self.exp_upper = upper
        self.exp_scale = scale
        self.register_buffer("exp_lookup_values", values, persistent=False)

        self.sigmoid_low = -10.0
        self.sigmoid_high = 16.0

    def _lookup_exp(self, x: torch.Tensor) -> torch.Tensor:
        values = self.exp_lookup_values
        if values.device != x.device:
            values = values.to(x.device, non_blocking=True)
        idx = torch.round((x - self.exp_lower) * self.exp_scale).to(torch.long)
        idx = torch.clamp(idx, 0, values.numel() - 1)
        return values[idx]

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        # print("x.shape: ", x.shape)
        if not torch.is_floating_point(x):
            raise TypeError("SigmoidTaylorLookup expects a floating point tensor.")

        work = x.to(dtype=torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= self.sigmoid_low
        mask_hi = work >= self.sigmoid_high
        mask_mid = (~mask_lo) & (~mask_hi)

        result[mask_lo] = 0.0
        result[mask_hi] = 1.0

        if mask_mid.any():
            mid_vals = work[mask_mid]
            neg_mid = -mid_vals
            exp_vals = self._lookup_exp(neg_mid)
            denom = 1.0 + exp_vals
            sigmoid_mid = _quantize_q16_16(1.0 / denom)
            result[mask_mid] = sigmoid_mid

        return result.to(dtype=x.dtype, device=x.device)


class SwishTaylorLookup(nn.Module):
    def __init__(self, table_path: Optional[str] = None):
        super().__init__()
        table = table_path or DEFAULT_EXP_TABLE
        values, lower, upper, scale = _load_exp_table(table)
        self.exp_lower = lower
        self.exp_upper = upper
        self.exp_scale = scale
        self.register_buffer("exp_lookup_values", values, persistent=False)

        self.swish_low = -10.0
        self.swish_high = 16.0

    def _lookup_exp(self, x: torch.Tensor) -> torch.Tensor:
        values = self.exp_lookup_values
        if values.device != x.device:
            values = values.to(x.device, non_blocking=True)
        idx = torch.round((x - self.exp_lower) * self.exp_scale).to(torch.long)
        idx = torch.clamp(idx, 0, values.numel() - 1)
        return values[idx]

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        # print("swish taylor lookup x.shape: ", x.shape)
        if not torch.is_floating_point(x):
            raise TypeError("SwishTaylorLookup expects a floating point tensor.")

        work = x.to(dtype=torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= self.swish_low
        mask_hi = work >= self.swish_high
        mask_mid = (~mask_lo) & (~mask_hi)

        result[mask_lo] = 0.0
        # if mask_hi.any():
        #     result[mask_hi] = _quantize_q16_16(work[mask_hi])
        result[mask_hi] = work[mask_hi]

        if mask_mid.any():
            mid_vals = work[mask_mid]
            neg_mid = -mid_vals
            exp_vals = self._lookup_exp(neg_mid)
            denom = 1.0 + exp_vals
            sigmoid_mid = _quantize_q16_16(1.0 / denom)
            mid_vals_q = _quantize_q16_16(mid_vals)
            swish_mid = _quantize_q16_16(mid_vals_q * sigmoid_mid)
            result[mask_mid] = swish_mid

        return result.to(dtype=x.dtype, device=x.device)


def _replace_activations(module: nn.Module, table_path: Optional[str] = None) -> nn.Module:
    for name, child in module.named_children():
        if isinstance(child, nn.Sigmoid):
            setattr(module, name, SigmoidTaylorLookup(table_path))
        elif isinstance(child, nn.SiLU):
            setattr(module, name, SwishTaylorLookup(table_path))
        else:
            _replace_activations(child, table_path)
    return module


class RunningMinMax:
    def __init__(self):
        self.global_min = float("inf")
        self.global_max = float("-inf")
        self.num_updates = 0

    def update(self, x: torch.Tensor) -> None:
        vmin = torch.min(x).item()
        vmax = torch.max(x).item()
        if vmin < self.global_min:
            self.global_min = vmin
        if vmax > self.global_max:
            self.global_max = vmax
        self.num_updates += 1


def register_activation_input_range_hooks(model: nn.Module):
    stats = {
        "SiLU": RunningMinMax(),
        "Sigmoid": RunningMinMax(),
        "SwishTaylorLookup": RunningMinMax(),
        "SigmoidTaylorLookup": RunningMinMax(),
    }
    handles = []

    def make_hook(kind: str):
        def hook(_module, inputs, _output):
            if not inputs:
                return
            x = inputs[0]
            if isinstance(x, (tuple, list)):
                x = x[0]
            if not torch.is_tensor(x):
                return
            with torch.no_grad():
                stats[kind].update(x)

        return hook

    for m in model.modules():
        if isinstance(m, nn.SiLU):
            handles.append(m.register_forward_hook(make_hook("SiLU")))
        elif isinstance(m, nn.Sigmoid):
            handles.append(m.register_forward_hook(make_hook("Sigmoid")))
        elif isinstance(m, SwishTaylorLookup):
            handles.append(m.register_forward_hook(make_hook("SwishTaylorLookup")))
        elif isinstance(m, SigmoidTaylorLookup):
            handles.append(m.register_forward_hook(make_hook("SigmoidTaylorLookup")))

    return stats, handles


def build_dataloader(data_dir: str, batch_size: int, workers: int):
    normalizer = transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    dataset = datasets.ImageFolder(
        data_dir,
        transforms.Compose(
            [
                transforms.Resize(600),
                transforms.CenterCrop(600),
                transforms.ToTensor(),
                normalizer,
            ]
        ),
    )
    return torch.utils.data.DataLoader(
        dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=workers,
        pin_memory=True,
    )


def evaluate(model: nn.Module, loader, device: torch.device):
    criterion = nn.CrossEntropyLoss()
    loss_sum = 0.0
    top1_correct = 0
    top5_correct = 0
    total = 0

    model.eval()
    with torch.no_grad():
        for images, targets in tqdm(loader, desc="Evaluating", leave=False):
            images = images.to(device, non_blocking=True)
            targets = targets.to(device, non_blocking=True)

            outputs = model(images)
            loss = criterion(outputs, targets)

            loss_sum += loss.item() * images.size(0)
            _, pred_top1 = outputs.max(dim=1)
            top1_correct += pred_top1.eq(targets).sum().item()

            _, pred_top5 = outputs.topk(5, dim=1, largest=True, sorted=True)
            top5_correct += pred_top5.eq(targets.unsqueeze(1)).sum().item()
            total += targets.size(0)

    avg_loss = loss_sum / total if total > 0 else float("nan")
    top1 = top1_correct / total * 100.0
    top5 = top5_correct / total * 100.0
    return avg_loss, top1, top5


def parse_args():
    parser = argparse.ArgumentParser(
        description="Evaluate EfficientNet-B7 with Sigmoid and Swish replaced by Taylor-based lookup tables."
    )
    parser.add_argument(
        "--data-dir",
        default="/home/gbzou/EfficientNet/ILSVRC2012_img_val",
        help="Path to validation dataset.",
    )
    parser.add_argument("--batch-size", type=int, default=32, help="Evaluation batch size.")
    parser.add_argument("--workers", type=int, default=4, help="Number of data loading workers.")
    parser.add_argument(
        "--device",
        default="cuda" if torch.cuda.is_available() else "cpu",
        help="Device to run on (default: auto-detect).",
    )
    parser.add_argument(
        "--exp-table",
        default=DEFAULT_EXP_TABLE,
        help="Path to exponential Taylor lookup table (.pt).",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    device = torch.device(args.device)
    print("device: ", device)

    if device.type == "cuda" and not torch.cuda.is_available():
        raise RuntimeError("CUDA device requested but no CUDA devices are available.")

    if not os.path.exists(args.data_dir):
        raise FileNotFoundError(f"Validation directory '{args.data_dir}' does not exist.")

    base_model = models.efficientnet_b7(weights="EfficientNet_B7_Weights.IMAGENET1K_V1")
    _replace_activations(base_model, args.exp_table)

    if device.type == "cuda" and torch.cuda.device_count() > 1:
        num_devices = torch.cuda.device_count()
        print(f"Using DataParallel on {num_devices} GPUs.")
        model = nn.DataParallel(base_model, device_ids=list(range(num_devices)))
        model.to(device)
        hooks_target = model.module
    else:
        model = base_model.to(device)
        hooks_target = model

    stats, handles = register_activation_input_range_hooks(hooks_target)

    loader = build_dataloader(args.data_dir, args.batch_size, args.workers)
    avg_loss, top1, top5 = evaluate(model, loader, device)

    print(f"Validation Loss: {avg_loss:.4f}")
    print(f"Top-1 Accuracy: {top1:.4f}%")
    print(f"Top-5 Accuracy: {top5:.4f}%")

    for h in handles:
        h.remove()

    print("\nActivation input ranges:")
    for kind, mm in stats.items():
        if mm.num_updates == 0:
            print(f"- {kind}: no data")
        else:
            print(f"- {kind}: min={mm.global_min:.6f}, max={mm.global_max:.6f}, updates={mm.num_updates}")


if __name__ == "__main__":
    main()
# nohup python -u test_efficientnetb7_replace_sigmoid_swish_taylor.py >> taylor_script_4.log 2>&1 &
