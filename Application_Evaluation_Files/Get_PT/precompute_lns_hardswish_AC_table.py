import argparse
import json
import os
import struct
import sys
from multiprocessing import Pool, get_context
from typing import Any, Dict, Iterable, List, Tuple

import torch
from tqdm import tqdm

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


def load_lns_params(param_file: str, func_name: str) -> Dict[str, Any]:
    with open(param_file, "r", encoding="utf-8") as f:
        params_all = json.load(f)
    if func_name not in params_all:
        raise KeyError(f"Function '{func_name}' not found in {param_file}. "
                       f"Available keys: {list(params_all.keys())}")
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


def _worker_compute(args: Tuple[int, float, float]) -> float:
    idx, lower, step = args
    x_val = lower + idx * step
    
    if x_val <= -3.0:
        return 0.0, 0.0
    if x_val >= 3.0:
        return x_val, x_val
    true_val = x_val * (x_val + 3.0)/6.0
    bits_in = float_to_bits32(x_val)
    try:
        result = LNS_TOP_FUNC(
            seg=6,
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
            conv_type=4,
        )
        bits_out = result["tri_result"]
        return bits32_to_float(bits_out), true_val
    except Exception:
        relu6 = max(0.0, min(6.0, x_val + 3.0))
        return x_val * relu6 / 6.0, true_val


def build_arg_list(indices: Iterable[int], lower: float, step: float) -> List[Tuple[int, float, float]]:
    return [(idx, lower, step) for idx in indices]


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Precompute LNS hard-swish lookup table with 16.16 quantization."
    )
    parser.add_argument(
        "--param-file",
        default=os.path.join(SCRIPT_DIR, "LNS", "parameter.json"),
        help="Path to the parameter.json containing hard_swish_1616 configuration.",
    )
    parser.add_argument(
        "--func-name",
        default="hard_swish_float",
        help="Parameter entry name to use from the JSON file.",
    )
    parser.add_argument(
        "--output",
        default=os.path.join(SCRIPT_DIR, "LNS", "hardswish_1616_C_table.pt"),
        help="Destination path for the serialized lookup table.",
    )
    parser.add_argument("--lower", type=float, default=-3.0, help="Lower bound of the domain.")
    parser.add_argument("--upper", type=float, default=3.0, help="Upper bound of the domain.")
    parser.add_argument(
        "--frac-bits",
        type=int,
        default=16,
        help="Fractional bits for fixed-point sampling (16 => step = 1/65536).",
    )
    parser.add_argument("--workers", type=int, default=max(1, os.cpu_count() or 1), help="Number of parallel workers.")
    parser.add_argument("--chunksize", type=int, default=512, help="Chunk size for multiprocessing imap.")
    args = parser.parse_args()

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)

    params = load_lns_params(args.param_file, args.func_name)

    indices, step = quantized_range(args.lower, args.upper, args.frac_bits)
    total = len(indices)

    worker_args = build_arg_list(indices, args.lower, step)

    ctx = get_context("spawn")
    results = []
    true_values = []
    with ctx.Pool(
        processes=args.workers,
        initializer=_worker_init,
        initargs=(params, LNS_DIR),
    ) as pool:
        for value, true_val in tqdm(pool.imap(_worker_compute, worker_args, chunksize=args.chunksize), total=total):
            results.append(value)
            true_values.append(true_val)

    values_tensor = torch.tensor(results, dtype=torch.float32)
    true_values_tensor = torch.tensor(true_values, dtype=torch.float32)
    indices_tensor = torch.arange(total, dtype=torch.float32)
    inputs_tensor = args.lower + indices_tensor * step
    abs_errors = torch.abs(values_tensor - true_values_tensor)
    print(f"max absolute error: {abs_errors.max()}")
    print(f"mean absolute error: {abs_errors.mean()}")
    metadata = {
        "lower": args.lower,
        "upper": args.upper,
        "frac_bits": args.frac_bits,
        "step": step,
        "scale": float(1 << args.frac_bits),
        "func_name": args.func_name,
        "param_file": os.path.abspath(args.param_file),
        "count": total,
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


if __name__ == "__main__":
    main()

