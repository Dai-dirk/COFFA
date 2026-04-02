import argparse
import os
from typing import Optional

import torch
import torch.nn as nn
from torchvision import datasets, transforms, models
from tqdm import tqdm

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LNS_DIR = os.path.join(SCRIPT_DIR, "LNS")
DEFAULT_SIGMOID_TABLE = os.path.join(LNS_DIR, "sigmoid_1616_C_table.pt")
DEFAULT_SWISH_TABLE = os.path.join(LNS_DIR, "swish_1616_Ctable.pt")


def _load_lookup_table(path: str) -> tuple[torch.Tensor, float, float, float]:
    if not os.path.exists(path):
        raise FileNotFoundError(f"Lookup table not found at '{path}'.")
    payload = torch.load(path, map_location="cpu")
    values = payload["values"].view(-1).float()
    meta = payload.get("metadata", {})
    lower = float(meta.get("lower", -16.0))
    upper = float(meta.get("upper", 16.0))
    scale = float(meta.get("scale", float(1 << int(meta.get("frac_bits", 16)))))
    return values, lower, upper, scale


class Sigmoid1616Lookup(nn.Module):
    def __init__(self, table_path: Optional[str] = None):
        super().__init__()
        table = table_path or DEFAULT_SIGMOID_TABLE
        values, lower, upper, scale = _load_lookup_table(table)
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Sigmoid1616Lookup expects a floating point tensor.")

        work = x.to(dtype=torch.float32)
        values = self.lookup_values
        if values.device != work.device:
            values = values.to(work.device, non_blocking=True)

        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        work_clamped = work.clamp(self.lower_bound, self.upper_bound)
        idx = torch.round((work_clamped - self.lower_bound) * self.scale).to(torch.long)
        idx = torch.clamp(idx, 0, values.numel() - 1)
        lookup = torch.take(values, idx.view(-1)).view_as(work)

        result = torch.where(mask_lo, torch.zeros_like(work), lookup)
        result = torch.where(mask_hi, torch.ones_like(work), result)

        return result.to(dtype=x.dtype, device=x.device)


class Swish1616Lookup(nn.Module):
    def __init__(self, table_path: Optional[str] = None):
        super().__init__()
        table = table_path or DEFAULT_SWISH_TABLE
        values, lower, upper, scale = _load_lookup_table(table)
        self.lower_bound = lower
        self.upper_bound = upper
        self.scale = scale
        self.register_buffer("lookup_values", values, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Swish1616Lookup expects a floating point tensor.")

        work = x.to(dtype=torch.float32)
        values = self.lookup_values
        if values.device != work.device:
            values = values.to(work.device, non_blocking=True)

        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        work_clamped = work.clamp(self.lower_bound, self.upper_bound)
        idx = torch.round((work_clamped - self.lower_bound) * self.scale).to(torch.long)
        idx = torch.clamp(idx, 0, values.numel() - 1)
        lookup = torch.take(values, idx.view(-1)).view_as(work)

        result = torch.where(mask_lo, torch.zeros_like(work), lookup)
        result = torch.where(mask_hi, work, result)

        return result.to(dtype=x.dtype, device=x.device)


def _replace_activations(
    module: nn.Module,
    sigmoid_table: Optional[str] = None,
    swish_table: Optional[str] = None,
) -> nn.Module:
    for name, child in module.named_children():
        if isinstance(child, nn.Sigmoid):
            setattr(module, name, Sigmoid1616Lookup(sigmoid_table))
        elif isinstance(child, nn.SiLU):
            setattr(module, name, Swish1616Lookup(swish_table))
        else:
            _replace_activations(child, sigmoid_table, swish_table)
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
        "Swish1616Lookup": RunningMinMax(),
        "Sigmoid1616Lookup": RunningMinMax(),
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
        elif isinstance(m, Swish1616Lookup):
            handles.append(m.register_forward_hook(make_hook("Swish1616Lookup")))
        elif isinstance(m, Sigmoid1616Lookup):
            handles.append(m.register_forward_hook(make_hook("Sigmoid1616Lookup")))

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
        description="Evaluate EfficientNet-B7 with Sigmoid and Swish replaced by Q16.16 lookup tables."
    )
    if torch.cuda.is_available():
        default_device = "cuda:2" if torch.cuda.device_count() > 1 else "cuda"
    else:
        default_device = "cpu"
    parser.add_argument(
        "--data-dir",
        default="/home/gbzou/EfficientNet/ILSVRC2012_img_val",
        help="Path to validation dataset.",
    )
    parser.add_argument("--batch-size", type=int, default=32, help="Evaluation batch size.")
    parser.add_argument("--workers", type=int, default=4, help="Number of data loading workers.")
    parser.add_argument(
        "--device",
        default=default_device,
        help="Device to run on (default: auto-detect).",
    )
    parser.add_argument(
        "--sigmoid-table",
        default=DEFAULT_SIGMOID_TABLE,
        help="Path to sigmoid Q16.16 lookup table (.pt).",
    )
    parser.add_argument(
        "--swish-table",
        default=DEFAULT_SWISH_TABLE,
        help="Path to swish Q16.16 lookup table (.pt).",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    device = torch.device(args.device)
    print("device:", device)

    if not os.path.exists(args.data_dir):
        raise FileNotFoundError(f"Validation directory '{args.data_dir}' does not exist.")

    model = models.efficientnet_b7(weights="EfficientNet_B7_Weights.IMAGENET1K_V1")
    _replace_activations(model, args.sigmoid_table, args.swish_table)
    model.to(device)

    stats, handles = register_activation_input_range_hooks(model)

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


