import argparse
import math
import os
from typing import List, Tuple

import torch
from tqdm import tqdm

from compute_tylor_exp import exp_q16, q16_to_float

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def float_to_q16_16_bits(value: float) -> str:
    scale = 1 << 16
    scaled = int(round(value * scale))
    limit = 1 << 31
    if scaled < -limit or scaled > limit - 1:
        raise ValueError(f"Value {value} is out of Q16.16 range.")
    if scaled < 0:
        scaled = (scaled + (1 << 32)) & 0xFFFFFFFF
    return format(scaled & 0xFFFFFFFF, "032b")


def bits_to_q16_16_int(bits: str) -> int:
    unsigned = int(bits, 2)
    if unsigned & (1 << 31):
        signed = unsigned - (1 << 32)
    else:
        signed = unsigned
    return signed


def quantized_range(lower: float, upper: float, frac_bits: int) -> Tuple[List[int], float]:
    scale = 1 << frac_bits
    count = int(round((upper - lower) * scale)) + 1
    indices = list(range(count))
    step = 1.0 / scale
    return indices, step


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Precompute Taylor series exp lookup table with 16.16 quantization."
    )
    parser.add_argument("--lower", type=float, default=-16.0, help="Lower bound of the domain.")
    parser.add_argument("--upper", type=float, default=10.0, help="Upper bound of the domain.")
    parser.add_argument(
        "--frac-bits",
        type=int,
        default=16,
        help="Fractional bits for fixed-point sampling (16 => step = 1/65536).",
    )
    parser.add_argument(
        "--output",
        default=os.path.join(SCRIPT_DIR, "LNS", "new_exp_taylor_5_1616_16_10_table.pt"),
        help="Destination path for the serialized lookup table.",
    )
    args = parser.parse_args()

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)

    indices, step = quantized_range(args.lower, args.upper, args.frac_bits)
    total = len(indices)

    approx_results: List[float] = []
    true_results: List[float] = []

    for idx in tqdm(indices, desc="Computing Taylor exp table"):
        x = args.lower + idx * step
        bits_in = float_to_q16_16_bits(x)
        x_q16 = bits_to_q16_16_int(bits_in)
        approx_q16 = exp_q16(x_q16)
        approx_val = q16_to_float(approx_q16)
        true_val = math.exp(x)

        approx_results.append(approx_val)
        true_results.append(true_val)

    values_tensor = torch.tensor(approx_results, dtype=torch.float32)
    true_tensor = torch.tensor(true_results, dtype=torch.float32)
    indices_tensor = torch.arange(total, dtype=torch.float32)
    inputs_tensor = args.lower + indices_tensor * step
    abs_errors = (values_tensor - true_tensor).abs()
    max_abs_error = float(abs_errors.max().item())
    mean_abs_error = float(abs_errors.mean().item())

    metadata = {
        "lower": args.lower,
        "upper": args.upper,
        "frac_bits": args.frac_bits,
        "step": step,
        "scale": float(1 << args.frac_bits),
        "count": total,
        "max_abs_error": max_abs_error,
        "mean_abs_error": mean_abs_error,
        "method": "taylor_exp_q16",
    }
    torch.save(
        {
            "metadata": metadata,
            "inputs": inputs_tensor.to(dtype=torch.float32),
            "values": values_tensor,
        },
        args.output,
    )
    print(f"Saved lookup table with {total} entries to {args.output}")
    print(f"Max absolute error: {max_abs_error:.6e}")
    print(f"Mean absolute error: {mean_abs_error:.6e}")


if __name__ == "__main__":
    main()

