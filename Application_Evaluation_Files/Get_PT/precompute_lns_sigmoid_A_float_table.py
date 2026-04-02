import argparse
import json
import math
import os
import struct
import sys
from multiprocessing import Pool, get_context
from typing import Any, Dict, Iterable, List, Tuple

import torch
from tqdm import tqdm
import matplotlib.pyplot as plt

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LNS_DIR = os.path.join(SCRIPT_DIR, "LNS")


def float_to_bits32(value: float) -> str:
    packed = struct.pack(">f", float(value))
    integer = int.from_bytes(packed, byteorder="big", signed=False)
    return format(integer & 0xFFFFFFFF, "032b")

def bits32_to_float(bits: str) -> float:
    integer = int(bits, 2)
    packed = integer.to_bytes(4, byteorder="big", signed=False)
    return struct.unpack(">f", packed)[0]


def float_to_q16_16_bits(value: float) -> str:
    scale = 1 << 16
    scaled = int(round(value * scale))
    limit = 1 << 31
    if scaled < -limit or scaled > limit - 1:
        raise ValueError(f"Value {value} is out of Q16.16 range.")
    if scaled < 0:
        scaled = (scaled + (1 << 32)) & 0xFFFFFFFF
    return format(scaled & 0xFFFFFFFF, "032b")


def bits_to_q16_16_float(bits: str) -> float:
    unsigned = int(bits, 2)
    if unsigned & (1 << 31):
        signed = unsigned - (1 << 32)
    else:
        signed = unsigned
    return signed / float(1 << 16)


def load_lns_params(param_file: str, func_name: str) -> Dict[str, Any]:
    with open(param_file, "r", encoding="utf-8") as f:
        params_all = json.load(f)
    if func_name not in params_all:
        raise KeyError(
            f"Function '{func_name}' not found in {param_file}. "
            f"Available keys: {list(params_all.keys())}"
        )
    return params_all[func_name]


def quantized_range(lower: float, upper: float, frac_bits: int) -> Tuple[List[int], float]:
    scale = 1 << frac_bits
    count = int(round((upper - lower) * scale)) + 1
    indices = list(range(count))
    step = 1.0 / scale
    return indices, step


def _worker_init(params: Dict[str, Any], lns_dir: str) -> None:
    global WORKER_PARAMS
    global LNS_TOP_FUNC
    WORKER_PARAMS = params
    if lns_dir not in sys.path:
        sys.path.append(lns_dir)
    from LNS_Top import lns_top  # type: ignore import

    LNS_TOP_FUNC = lns_top


def _worker_compute(args: Tuple[int, float, float]) -> Tuple[float, float, bool]:
    idx, lower, step = args
    x_val = lower + idx * step
    true_val = 1.0 / (1.0 + math.exp(-x_val))

    bits_in = float_to_bits32(x_val)
    try:
        result = LNS_TOP_FUNC(
            seg = 7,
            x_in=bits_in,
            y_in=WORKER_PARAMS["y_in"],
            n=WORKER_PARAMS["n"],
            float_flag=WORKER_PARAMS["float_flag"],
            break_points_in=WORKER_PARAMS["break_points_in"],
            TRG=WORKER_PARAMS["TRG"],
            VEC=WORKER_PARAMS["VEC"],
            qi=WORKER_PARAMS["qi"],
            Div=WORKER_PARAMS["Div"],
            TRi=WORKER_PARAMS["TRi"],
            power=WORKER_PARAMS["power"],
            bias_sel=WORKER_PARAMS["bias_sel"],
            constant_bias_in=WORKER_PARAMS["constant_bias_in"],
            logc_in_0=WORKER_PARAMS["logc_in_0"],
            K_in_0=WORKER_PARAMS["K_in_0"],
            logc_in_1=WORKER_PARAMS["logc_in_1"],
            K_in_1=WORKER_PARAMS["K_in_1"],
            logc_in_2=WORKER_PARAMS["logc_in_2"],
            K_in_2=WORKER_PARAMS["K_in_2"],
            logc_in_3=WORKER_PARAMS["logc_in_3"],
            K_in_3=WORKER_PARAMS["K_in_3"],
            logc_in_4=WORKER_PARAMS["logc_in_4"],
            K_in_4=WORKER_PARAMS["K_in_4"],
            logc_in_5=WORKER_PARAMS["logc_in_5"],
            K_in_5=WORKER_PARAMS["K_in_5"],
            logc_in_6=WORKER_PARAMS["logc_in_6"],
            K_in_6=WORKER_PARAMS["K_in_6"],
            conv_type=4,
        )
        bits_out = result["tri_result"]
        approx_val = bits32_to_float(bits_out)
        return approx_val, true_val, False
    except Exception:
        return true_val, true_val, True


def build_arg_list(indices: Iterable[int], lower: float, step: float) -> List[Tuple[int, float, float]]:
    return [(idx, lower, step) for idx in indices]


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Precompute LNS sigmoid lookup table with 16.16 quantization."
    )
    parser.add_argument(
        "--param-file",
        default=os.path.join(SCRIPT_DIR, "LNS", "parameter.json"),
        help="Path to the parameter.json containing sigmoid1616 configuration.",
    )
    parser.add_argument(
        "--func-name",
        default="sigmoid_A_float",
        help="Parameter entry name to use from the JSON file.",
    )
    parser.add_argument(
        "--output",
        default=os.path.join(SCRIPT_DIR, "LNS", "sigmoid_1616_A_float_table.pt"),
        help="Destination path for the serialized lookup table.",
    )
    parser.add_argument("--lower", type=float, default=-16.0, help="Lower bound of the domain.")
    parser.add_argument("--upper", type=float, default=16.0, help="Upper bound of the domain.")
    parser.add_argument(
        "--frac-bits",
        type=int,
        default=16,
        help="Fractional bits for fixed-point sampling (16 => step = 1/65536).",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=max(1, os.cpu_count() or 1),
        help="Number of parallel workers.",
    )
    parser.add_argument("--chunksize", type=int, default=512, help="Chunk size for multiprocessing imap.")
    args = parser.parse_args()

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)

    params = load_lns_params(args.param_file, args.func_name)

    indices, step = quantized_range(args.lower, args.upper, args.frac_bits)
    total = len(indices)

    worker_args = build_arg_list(indices, args.lower, step)

    ctx = get_context("spawn")
    approx_results = []
    true_results = []
    fallback_count = 0
    with ctx.Pool(
        processes=args.workers,
        initializer=_worker_init,
        initargs=(params, LNS_DIR),
    ) as pool:
        for approx_val, true_val, did_fallback in tqdm(
            pool.imap(_worker_compute, worker_args, chunksize=args.chunksize),
            total=total,
        ):
            approx_results.append(approx_val)
            true_results.append(true_val)
            if did_fallback:
                fallback_count += 1

    values_tensor = torch.tensor(approx_results, dtype=torch.float32)
    true_tensor = torch.tensor(true_results, dtype=torch.float32)
    indices_tensor = torch.arange(total, dtype=torch.float32)
    inputs_tensor = args.lower + indices_tensor * step
    abs_errors = (values_tensor - true_tensor).abs()
    max_abs_error = float(abs_errors.max().item())
    mean_abs_error = float(abs_errors.mean().item())

     # 绘制误差图（同时包含硬近似和 soft 近似）
    plt.figure(figsize=(12, 6))
    plt.plot(
        inputs_tensor.numpy(),
        abs_errors.numpy(),
        linewidth=0.5,
        alpha=0.7,
        label="hard abs error",
    )
    plt.xlabel("Input Value", fontsize=12)
    plt.ylabel("Absolute Error", fontsize=12)
    plt.title(
        f"Absolute Error of LNS Swish Approximation (hard & soft) ({args.func_name})",
        fontsize=14,
    )
    plt.grid(True, alpha=0.3)
    plt.yscale("log")  # 使用对数刻度以便更好地查看误差分布
    plt.legend()

    # 保存图片到当前文件夹
    error_plot_path = os.path.join(SCRIPT_DIR, f"abs_error_plot_{args.func_name}.png")
    plt.savefig(error_plot_path, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Saved error plot to {error_plot_path}")

    metadata = {
        "lower": args.lower,
        "upper": args.upper,
        "frac_bits": args.frac_bits,
        "step": step,
        "scale": float(1 << args.frac_bits),
        "func_name": args.func_name,
        "param_file": os.path.abspath(args.param_file),
        "count": total,
        "max_abs_error": max_abs_error,
        "mean_abs_error": mean_abs_error,
        "fallback_evaluations": fallback_count,
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
    if fallback_count > 0:
        print(f"Warning: {fallback_count} evaluations fell back to true sigmoid.")


if __name__ == "__main__":
    main()


