import argparse
import os
import sys
from typing import Optional

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from tqdm import tqdm

# Allow importing local modules
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

from senet_all import se_resnet152  # type: ignore import


LNS_DIR = os.path.join(SCRIPT_DIR, "LNS")
DEFAULT_TABLE_PATH = os.path.join(LNS_DIR, "sigmoid_1616_B_table.pt")


class Sigmoid1616Lookup(nn.Module):
    def __init__(self, table_path: Optional[str] = None):
        super().__init__()
        self.table_path = table_path or DEFAULT_TABLE_PATH
        if not os.path.exists(self.table_path):
            raise FileNotFoundError(f"Sigmoid lookup table not found at '{self.table_path}'.")

        payload = torch.load(self.table_path, map_location="cpu")
        values = payload["values"].view(-1).float()
        inputs = payload.get("inputs")
        if inputs is not None:
            inputs = inputs.view(-1).float()
        meta = payload.get("metadata", {})

        self.lower_bound = float(meta.get("lower", -16.0))
        self.upper_bound = float(meta.get("upper", 16.0))
        self.frac_bits = int(meta.get("frac_bits", 16))
        self.scale = float(meta.get("scale", float(1 << self.frac_bits)))
        self.step = float(meta.get("step", 1.0 / self.scale))

        self.register_buffer("lookup_values", values, persistent=False)
        if inputs is not None:
            self.register_buffer("lookup_inputs", inputs, persistent=False)
        else:
            indices = torch.arange(values.numel(), dtype=torch.float32)
            base_inputs = self.lower_bound + indices * self.step
            self.register_buffer("lookup_inputs", base_inputs, persistent=False)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("Sigmoid1616Lookup expects a floating point tensor.")

        lookup_values = self.lookup_values
        if lookup_values is None or lookup_values.numel() == 0:
            raise RuntimeError("Lookup table is not initialized.")

        work = x.to(dtype=torch.float32)
        result = work.clone()

        mask_lo = work <= self.lower_bound
        mask_hi = work >= self.upper_bound
        mask_mid = (~mask_lo) & (~mask_hi)

        if mask_mid.any():
            mid_vals = work[mask_mid]
            idx = torch.round((mid_vals - self.lower_bound) * self.scale).to(torch.long)
            idx = torch.clamp(idx, 0, lookup_values.numel() - 1)
            values = lookup_values
            if values.device != result.device:
                values = values.to(result.device, non_blocking=True)
            result[mask_mid] = values[idx]

        result[mask_lo] = 0.0
        result[mask_hi] = 1.0
        return result.to(dtype=x.dtype, device=x.device)


def replace_sigmoid_with_lookup(module: nn.Module, table_path: Optional[str] = None) -> nn.Module:
    for name, child in module.named_children():
        if isinstance(child, nn.Sigmoid):
            setattr(module, name, Sigmoid1616Lookup(table_path))
        else:
            replace_sigmoid_with_lookup(child, table_path)
    return module


def build_dataloader(data_dir: str, batch_size: int, workers: int) -> DataLoader:
    transform = transforms.Compose(
        [
            transforms.Resize(256),
            transforms.CenterCrop(224),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
        ]
    )
    dataset = datasets.ImageFolder(data_dir, transform=transform)
    return DataLoader(
        dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=workers,
        pin_memory=True,
    )


def evaluate(model: nn.Module, loader: DataLoader, device: torch.device) -> tuple[float, float, float]:
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

    avg_loss = loss_sum / total
    top1 = top1_correct / total * 100.0
    top5 = top5_correct / total * 100.0
    return avg_loss, top1, top5


def parse_args():
    parser = argparse.ArgumentParser(
        description="Evaluate SE-ResNet152 with Sigmoid replaced by precomputed sigmoid1616 lookup table."
    )
    parser.add_argument(
        "--checkpoint",
        default=os.path.join(SCRIPT_DIR, "se_resnet152-d17c99b7.pth"),
        help="Path to SE-ResNet152 checkpoint.",
    )
    parser.add_argument(
        "--data-dir",
        required=False,
        default="/home/gbzou/EfficientNet/ILSVRC2012_img_val",
        help="Path to validation dataset (e.g. EfficientNet/ILSVRC2012_img_val).",
    )
    parser.add_argument("--batch-size", type=int, default=64, help="Evaluation batch size.")
    parser.add_argument("--workers", type=int, default=8, help="Number of data loading workers.")
    parser.add_argument(
        "--device",
        default="cuda" if torch.cuda.is_available() else "cpu",
        help="Device to run on (default: auto-detect).",
    )
    parser.add_argument(
        "--table-path",
        default=DEFAULT_TABLE_PATH,
        help="Path to precomputed sigmoid lookup table (.pt).",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    device = torch.device(args.device)

    if not os.path.exists(args.data_dir):
        raise FileNotFoundError(f"Validation directory '{args.data_dir}' does not exist.")

    model = se_resnet152(num_classes=1000, pretrained=None)
    if args.checkpoint and os.path.exists(args.checkpoint):
        state_dict = torch.load(args.checkpoint, map_location="cpu")
        model.load_state_dict(state_dict, strict=False)
    else:
        print(f"Warning: checkpoint '{args.checkpoint}' not found. Using randomly initialized model.")

    replace_sigmoid_with_lookup(model, args.table_path)
    model.to(device)

    loader = build_dataloader(args.data_dir, args.batch_size, args.workers)
    avg_loss, top1, top5 = evaluate(model, loader, device)

    print(f"Validation Loss: {avg_loss:.4f}")
    print(f"Top-1 Accuracy: {top1:.4f}%")
    print(f"Top-5 Accuracy: {top5:.4f}%")


if __name__ == "__main__":
    main()


