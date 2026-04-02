"""Evaluate DCT compression using LNS-based cosine approximation on Set12."""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image

from compression_utils import compress_image_with_matrices
from dct_block_compression import compute_mse_psnr, hilbert_matrix
# [[ 0.35355339  0.35355339  0.35355339  0.35355339  0.35355339  0.35355339
#    0.35355339  0.35355339]
#  [ 0.49039263  0.41573482  0.27778537  0.09754561 -0.09754561 -0.27778537
#   -0.41573482 -0.49039263]
#  [ 0.46193973  0.19134173 -0.19134173 -0.46193973 -0.46193973 -0.19134173
#    0.19134173  0.46193973]
#  [ 0.41573482 -0.09754561 -0.49039263 -0.27778537  0.27778537  0.49039263
#    0.09754561 -0.41573482]
#  [ 0.35355324 -0.35355324 -0.35355324  0.35355324  0.35355324 -0.35355324
#   -0.35355324  0.35355324]
#  [ 0.27778537 -0.49039263  0.09754561  0.41573482 -0.41573482 -0.09754561
#    0.49039263 -0.27778537]
#  [ 0.19134173 -0.46193973  0.46193973 -0.19134173 -0.19134173  0.46193973
#   -0.46193973  0.19134173]
#  [ 0.09754561 -0.27778537  0.41573482 -0.49039263  0.49039263 -0.41573482
#    0.27778537 -0.09754561]]

LNS_DIR = Path(__file__).resolve().parent / "LNS"
if str(LNS_DIR) not in sys.path:
    sys.path.insert(0, str(LNS_DIR))

from LNS_Top import lns_top  # type: ignore  # noqa: E402


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"}


Q3_29_SCALE = 1 << 29
Q3_29_MIN = -4 * Q3_29_SCALE
Q3_29_MAX = (4 * Q3_29_SCALE) - 1


def _saturate_q3_29(value: int) -> int:
    min_val = Q3_29_MIN
    max_val = Q3_29_MAX
    return max(min(value, max_val), min_val)


def float_to_q3_29_bits(value: float) -> str:
    scaled = int(round(value * Q3_29_SCALE))
    scaled = _saturate_q3_29(scaled)
    if scaled < 0:
        scaled = (1 << 32) + scaled
    return format(scaled & 0xFFFFFFFF, "032b")


def q3_29_bits_to_float(bits: str) -> float:
    if len(bits) != 32:
        raise ValueError("Expected 32-bit string for Q3.29 conversion.")
    value = int(bits, 2)
    if value & (1 << 31):
        value -= 1 << 32
    return value / Q3_29_SCALE

def float_to_flolat_bits(value: float) -> str:
    """Convert a Python float to IEEE754 single-precision (float32) 32-bit binary string."""
    import struct as _struct

    packed = _struct.pack(">f", float(value))
    bits_int = _struct.unpack(">I", packed)[0]
    return f"{bits_int:032b}"

def float_bits_to_float(bits: str) -> float:
    """Convert a 32-bit binary string to a Python float."""
    import struct as _struct
    bits_int = int(bits, 2)
    packed = _struct.pack(">I", bits_int)
    return _struct.unpack(">f", packed)[0]

def load_lns_config(parameter_path: Path, function_key: str) -> dict[str, Any]:
    if not parameter_path.is_file():
        raise FileNotFoundError(f"Parameter file not found: {parameter_path}")

    data = json.loads(parameter_path.read_text(encoding="utf-8"))
    try:
        return data[function_key]
    except KeyError as exc:
        available = ", ".join(sorted(data.keys()))
        raise KeyError(
            f"Function key '{function_key}' not found in {parameter_path}. Available keys: {available}"
        ) from exc


def _reduce_theta(theta: float) -> tuple[float, float]:
    theta = ((theta + math.pi) % (2 * math.pi)) - math.pi
    sign = 1.0
    if theta > math.pi / 2:
        theta = math.pi - theta
        sign = -1.0
    elif theta < -math.pi / 2:
        theta = -math.pi - theta
        sign = -1.0
    return theta, sign


def approximate_cos(theta: float, params: dict[str, Any]) -> float:
    reduced_theta, sign = _reduce_theta(theta)
    if not (-math.pi / 2 < reduced_theta < math.pi / 2):
        reduced_theta = max(min(reduced_theta, math.pi / 2 - 1e-9), -math.pi / 2 + 1e-9)

    # print("reduced_theta:", reduced_theta)
    x_bits = float_to_flolat_bits(reduced_theta)
    # print("x_bits:", x_bits)
    result = lns_top(
        seg = 6,
        conv_type = 3,
        x_in=x_bits,
        y_in=params.get("y_in", "0" * 32),
        n=params.get("n", "00000"),
        float_flag=params.get("float_flag", "1"),
        break_points_in=params.get("break_points_in", []),
        TRG=params.get("TRG", "0"),
        VEC=params.get("VEC", "0"),
        qi=params.get("qi", "0"),
        Div=params.get("Div", "0"),
        TRi=params.get("TRi", "0"),
        power=params.get("power", "0"),
        bias_sel=params.get("bias_sel", "0" * 12),
        constant_bias_in=params.get("constant_bias_in"),
        logc_in_0=params.get("logc_in_0"),
        K_in_0=params.get("K_in_0"),
        logc_in_1=params.get("logc_in_1"),
        K_in_1=params.get("K_in_1"),
        logc_in_2=params.get("logc_in_2"),
        K_in_2=params.get("K_in_2"),
        logc_in_3=params.get("logc_in_3"),
        K_in_3=params.get("K_in_3"),
        logc_in_4=params.get("logc_in_4"),
        K_in_4=params.get("K_in_4"),
        logc_in_5=params.get("logc_in_5"),
        K_in_5=params.get("K_in_5"),
    )

    approx_bits = result.get("tri_result")
    if approx_bits is None:
        raise ValueError("LNS pipeline did not return 'tri_result'.")
    # print("approx_bits:", approx_bits)
    value = float_bits_to_float(approx_bits)
    # print("value:", value)
    if not math.isfinite(value):
        print("value is not finite, returning cos(theta)")
        return math.cos(theta)

    value = max(-1.0, min(1.0, value))
    return sign * value


def generate_dct_matrix_lns(block_size: int, params: dict[str, Any]) -> np.ndarray:
    matrix = np.zeros((block_size, block_size), dtype=np.float64)

    for i in range(block_size):
        for j in range(block_size):
            theta = i * (2 * j + 1) * math.pi / (2.0 * block_size)
            matrix[i, j] = approximate_cos(theta, params)

    matrix *= math.sqrt(2.0 / block_size)
    matrix[0, :] /= math.sqrt(2.0)
    return matrix


def block_dct_compress_lns(
    image: np.ndarray,
    c_matrix: np.ndarray,
    quant_matrix: np.ndarray,
) -> tuple[np.ndarray, int]:
    """Compress image with precomputed matrices, returning reconstruction and Huffman bits."""

    return compress_image_with_matrices(image, c_matrix=c_matrix, quant_matrix=quant_matrix)


def load_grayscale(path: Path) -> np.ndarray:
    return np.array(Image.open(path).convert("L"))


def gather_images(folder: Path) -> list[Path]:
    return sorted(
        [p for p in folder.iterdir() if p.suffix.lower() in SUPPORTED_EXTENSIONS]
    )


def write_log(lines: list[str], log_path: Path) -> None:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Evaluate DCT compression using LNS-based cosine approximation"
    )
    parser.add_argument(
        "--folder",
        type=Path,
        default=Path("Set12"),
        help="Directory containing grayscale images (default: ./Set12)",
    )
    parser.add_argument(
        "--log",
        type=Path,
        default=Path("DCT_set12_lns_results.log"),
        help="File to store per-image metrics.",
    )
    parser.add_argument(
        "--matrix-log",
        type=Path,
        default=Path("DCT_first_image_matrices.log"),
        help="Destination log file for the first image's gray and reconstruction matrices.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("DCT_lns_outputs"),
        help="Directory to store reconstructed images.",
    )
    parser.add_argument(
        "--quant-scale",
        type=float,
        default=1.5,
        help="Scale factor p for the Hilbert-based quantization matrix.",
    )
    parser.add_argument(
        "--block-size",
        type=int,
        default=8,
        help="Block size for the DCT transform (default: 8).",
    )
    parser.add_argument(
        "--parameter-path",
        type=Path,
        default=LNS_DIR / "parameter.json",
        help="Path to the LNS parameter JSON file.",
    )
    parser.add_argument(
        "--function-key",
        type=str,
        default="cos_float",
        help="Key inside the parameter file containing cosine approximation parameters.",
    )
    args = parser.parse_args()

    if not args.folder.is_dir():
        raise FileNotFoundError(f"Input folder {args.folder} not found or not a directory.")

    image_paths = gather_images(args.folder)
    if not image_paths:
        raise FileNotFoundError(
            f"No supported images found in {args.folder}. Expected extensions: {sorted(SUPPORTED_EXTENSIONS)}"
        )

    lns_params = load_lns_config(args.parameter_path, args.function_key)
    c_matrix = generate_dct_matrix_lns(args.block_size, lns_params)
    quant_matrix = args.quant_scale * 8.0 / hilbert_matrix(args.block_size)
    print(c_matrix)

    args.output_dir.mkdir(parents=True, exist_ok=True)

    log_lines = ["Image,MSE,PSNR(dB),HuffmanBits,CompressionRatio"]
    mse_values: list[float] = []
    psnr_values: list[float] = []
    huffman_bits_values: list[int] = []
    original_bits_values: list[int] = []
    compression_ratios: list[float] = []
    first_gray_values: list[list[int]] | None = None
    first_recon_values: list[list[int]] | None = None

    for idx, image_path in enumerate(image_paths):
        gray = load_grayscale(image_path)

        reconstructed, huffman_bits = block_dct_compress_lns(
            gray,
            c_matrix=c_matrix,
            quant_matrix=quant_matrix,
        )
        mse, psnr = compute_mse_psnr(gray, reconstructed)

        mse_values.append(mse)
        psnr_values.append(psnr if math.isfinite(psnr) else math.inf)
        huffman_bits_values.append(huffman_bits)

        num_blocks = (gray.shape[0] // args.block_size) * (gray.shape[1] // args.block_size)
        original_bits = num_blocks * args.block_size * args.block_size * 8
        original_bits_values.append(original_bits)

        compression_ratio = original_bits / huffman_bits if huffman_bits else math.inf
        compression_ratios.append(compression_ratio)

        psnr_str = "Infinity" if math.isinf(psnr) else f"{psnr:.3f}"
        ratio_str = "Infinity" if not math.isfinite(compression_ratio) else f"{compression_ratio:.3f}"
        log_lines.append(f"{image_path.name},{mse:.6f},{psnr_str},{huffman_bits},{ratio_str}")
        print(
            f"{image_path.name}: Huffman bits = {huffman_bits}, "
            f"compression ratio (original/huffman) = {ratio_str}"
        )

        if idx == 0:
            first_gray_values = gray.astype(int).tolist()
            first_recon_values = reconstructed.astype(int).tolist()

        output_name = image_path.stem + "_lns_recon" + image_path.suffix
        output_path = args.output_dir / output_name
        Image.fromarray(reconstructed).save(output_path)

    avg_mse = float(np.mean(mse_values))
    finite_psnr = [value for value in psnr_values if math.isfinite(value)]
    avg_psnr = float(np.mean(finite_psnr)) if finite_psnr else math.inf

    total_huffman_bits = int(np.sum(huffman_bits_values)) if huffman_bits_values else 0
    total_original_bits = int(np.sum(original_bits_values)) if original_bits_values else 0
    overall_ratio = (
        total_original_bits / total_huffman_bits if total_huffman_bits else math.inf
    )

    avg_psnr_str = "Infinity" if not finite_psnr else f"{avg_psnr:.3f}"
    finite_ratios = [r for r in compression_ratios if math.isfinite(r)]
    avg_ratio = float(np.mean(finite_ratios)) if finite_ratios else math.inf
    overall_ratio_str = "Infinity" if not math.isfinite(overall_ratio) else f"{overall_ratio:.3f}"
    avg_ratio_str = "Infinity" if not math.isfinite(avg_ratio) else f"{avg_ratio:.3f}"
    log_lines.append(
        f"Totals,{avg_mse:.6f},{avg_psnr_str},{total_huffman_bits},{overall_ratio_str}"
    )

    write_log(log_lines, args.log)

    if first_gray_values is not None and first_recon_values is not None:
        matrix_lines = [
            "FirstImageGray=",
            json.dumps(first_gray_values),
            "FirstImageReconstructed=",
            json.dumps(first_recon_values),
        ]
        write_log(matrix_lines, args.matrix_log)
        matrix_log_path = args.matrix_log
    else:
        matrix_log_path = None

    print(f"Processed {len(image_paths)} images from {args.folder}.")
    print(f"Average MSE: {avg_mse:.6f}")
    if avg_psnr_str == "Infinity":
        print("Average PSNR: Infinity (all reconstructions perfect)")
    else:
        print(f"Average PSNR: {avg_psnr:.4f} dB")
    print(f"Detailed results saved to {args.log}")
    print(f"Total Huffman bits: {total_huffman_bits}")
    if math.isfinite(avg_ratio):
        print(f"Average compression ratio (per-image mean): {avg_ratio:.3f}")
    else:
        print("Average compression ratio (per-image mean): Infinity")
    if math.isfinite(overall_ratio):
        print(f"Overall compression ratio (original/huffman): {overall_ratio:.3f}")
    else:
        print("Overall compression ratio (original/huffman): Infinity")
    if matrix_log_path is not None:
        print(f"First image matrices saved to {matrix_log_path}")


if __name__ == "__main__":
    main()

