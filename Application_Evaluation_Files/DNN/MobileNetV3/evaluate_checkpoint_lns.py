import argparse
import json
import os
import struct
import sys
import warnings
from typing import Any, Dict, Optional

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from tqdm import tqdm

from mobilenetv3 import MobileNetV3_Large

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LNS_DIR = os.path.join(SCRIPT_DIR, "LNS")
if LNS_DIR not in sys.path:
    sys.path.append(LNS_DIR)

DEFAULT_TABLE_PATH = os.environ.get(
    "MNV3_LNS_TABLE_PATH", os.path.join(LNS_DIR, "hardswish_1616_C_table.pt")
)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Evaluate MobileNetV3-Large checkpoint with LNS HardSwish approximation."
    )
    parser.add_argument(
        "--checkpoint",
        required=False,
        default="/home/gbzou/ISCA_AE/Software/DNN/MobileNetV3/450_act3_mobilenetv3_large.pth",
        help="Path to the checkpoint file, e.g. mobilenetv3/450_act3_mobilenetv3_large.pth",
    )
    parser.add_argument(
        "--data-dir",
        required=False,
        default="/home/gbzou/EfficientNet/ILSVRC2012_img_val",
        help="Path to the validation images, e.g. EfficientNet/ILSVRC2012_img_val",
    )
    parser.add_argument("--batch-size", type=int, default=128, help="Evaluation batch size.")
    parser.add_argument("--workers", type=int, default=8, help="Number of data loading workers.")
    parser.add_argument(
        "--device",
        default="cuda" if torch.cuda.is_available() else "cpu",
        # default = "cpu",
        help="Device to run on (default: auto-detect).",
    )
    return parser.parse_args()


def float_to_bits32(value: float) -> str:
    packed = struct.pack(">f", float(value))
    integer = int.from_bytes(packed, byteorder="big", signed=False)
    return format(integer & 0xFFFFFFFF, "032b")


def bits32_to_float(bits: str) -> float:
    integer = int(bits, 2)
    packed = integer.to_bytes(4, byteorder="big", signed=False)
    return struct.unpack(">f", packed)[0]


def load_lns_params(func_name: str) -> Dict[str, Any]:
    param_path = os.path.join(LNS_DIR, "parameter.json")
    with open(param_path, "r", encoding="utf-8") as f:
        params_all = json.load(f)
    if func_name not in params_all:
        raise KeyError(f"Function '{func_name}' not found in {param_path}.")
    return params_all[func_name]


class LNSHardSwish(nn.Module):
    def __init__(
        self,
        inplace: bool = False,
        func_name: str = "hard_swish_1616",
        table_path: Optional[str] = None,
        build_value_map: bool = True,
    ):
        super().__init__()
        self.inplace = inplace
        self.func_name = func_name
        self.table_path = table_path or DEFAULT_TABLE_PATH
        self.lower_bound = -3.0
        self.upper_bound = 3.0
        self.frac_bits = 16
        self.scale = float(1 << self.frac_bits)
        self.step = 1.0 / self.scale
        self.table_available = False
        self.params: Optional[Dict[str, Any]] = None
        self.cache: Dict[str, float] = {}
        self.cache_limit = 1 << 20
        self.value_map: Optional[Dict[float, float]] = None
        self.build_value_map = build_value_map

        if os.path.exists(self.table_path):
            payload = torch.load(self.table_path, map_location="cpu")
            values = payload["values"].view(-1).float()
            inputs = payload.get("inputs")
            if inputs is not None:
                inputs = inputs.view(-1).float()
            else:
                inputs = torch.arange(values.numel(), dtype=torch.float32) * self.step + self.lower_bound
            meta = payload.get("metadata", {})
            self.lower_bound = float(meta.get("lower", self.lower_bound))
            self.upper_bound = float(meta.get("upper", self.upper_bound))
            self.frac_bits = int(meta.get("frac_bits", self.frac_bits))
            self.scale = float(meta.get("scale", float(1 << self.frac_bits)))
            self.step = float(meta.get("step", 1.0 / self.scale))
            self.register_buffer("lookup_values", values, persistent=False)
            self.register_buffer("lookup_inputs", inputs, persistent=False)
            self.table_available = True
            if self.build_value_map:
                self.value_map = dict(zip(inputs.tolist(), values.tolist()))
        else:
            raise RuntimeError("Lookup table is not initialized.")

    def _lookup_forward(self, x: torch.Tensor) -> torch.Tensor:
        # print("x.shape: ", x.shape)
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
            if lookup_values.device != result.device:
                lookup_values = lookup_values.to(result.device)
            result[mask_mid] = lookup_values[idx]

        result[mask_lo] = 0.0
        result[mask_hi] = work[mask_hi]
        return result.to(dtype=x.dtype)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("LNSHardSwish expects floating point tensor.")

        if self.table_available and self.lookup_values.numel() > 0:
            approx = self._lookup_forward(x)
            if self.inplace:
                x.copy_(approx)
                return x
            return approx
        else:
            raise RuntimeError("Lookup table is not initialized.")


def build_dataloader(data_dir, batch_size, workers):
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


def load_model(checkpoint_path, device):
    model = MobileNetV3_Large(num_classes=1000, act=LNSHardSwish)
    checkpoint = torch.load(checkpoint_path, map_location="cpu")
    state_dict = checkpoint.get("model", checkpoint)
    model.load_state_dict(state_dict, strict=False)
    model.to(device)
    print("device: ", device)
    model.eval()
    return model


@torch.no_grad()
def evaluate(model, loader, device):
    criterion = nn.CrossEntropyLoss()
    loss_sum = 0.0
    top1_correct = 0
    top5_correct = 0
    total = 0

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


def main():
    args = parse_args()
    device = torch.device(args.device)
    loader = build_dataloader(args.data_dir, args.batch_size, args.workers)
    model = load_model(args.checkpoint, device)
    avg_loss, top1, top5 = evaluate(model, loader, device)
    print(f"Validation Loss: {avg_loss:.4f}")
    print(f"Top-1 Accuracy: {top1:.4f}%")
    print(f"Top-5 Accuracy: {top5:.4f}%")


if __name__ == "__main__":
    main()

