import argparse

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from tqdm import tqdm

from mobilenetv3 import MobileNetV3_Large

_Q16_SCALE = float(1 << 16)


def _quantize_q16_16(tensor: torch.Tensor) -> torch.Tensor:
    return torch.round(tensor * _Q16_SCALE) / _Q16_SCALE


class CustomHardSwish(nn.Module):
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if not torch.is_floating_point(x):
            raise TypeError("CustomHardSwish expects a floating point tensor.")

        work = x.to(dtype=torch.float32)
        result = torch.empty_like(work)

        mask_lo = work <= -3.0
        mask_hi = work >= 3.0
        mask_mid = (~mask_lo) & (~mask_hi)

        result[mask_lo] = 0.0
        result[mask_hi] = work[mask_hi]

        if mask_mid.any():
            mid_vals = work[mask_mid]
            mid_q = _quantize_q16_16(mid_vals)
            mid_sq = _quantize_q16_16(mid_q * mid_q)

            term1 = _quantize_q16_16(mid_q / 2.0)
            term2 = _quantize_q16_16(mid_sq / 6.0)
            mid_result = _quantize_q16_16(term1 + term2)

            result[mask_mid] = mid_result

        return result.to(dtype=x.dtype, device=x.device)


def replace_hardswish_with_custom(module: nn.Module) -> nn.Module:
    for name, child in module.named_children():
        if isinstance(child, nn.Hardswish):
            setattr(module, name, CustomHardSwish())
        else:
            replace_hardswish_with_custom(child)
    return module


def parse_args():
    parser = argparse.ArgumentParser(
        description="Evaluate MobileNetV3-Large checkpoint with custom Taylor hard-swish approximation."
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
        help="Device to run on (default: auto-detect).",
    )
    return parser.parse_args()


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


def load_model(checkpoint_path: str, device: torch.device) -> nn.Module:
    model = MobileNetV3_Large(num_classes=1000)
    state = torch.load(checkpoint_path, map_location="cpu")
    state_dict = state.get("model", state)
    model.load_state_dict(state_dict)
    replace_hardswish_with_custom(model)
    model.to(device)
    model.eval()
    return model


@torch.no_grad()
def evaluate(model: nn.Module, loader: DataLoader, device: torch.device):
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

