"""Evaluate DCT compression using Taylor-series cosine approximation on Set12."""

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
#  [ 0.49039263  0.4157348   0.27778524  0.09754845 -0.0975484  -0.27778518
#   -0.4157348  -0.49039263]
#  [ 0.46193975  0.19134241 -0.19134235 -0.46193972 -0.46193972 -0.19134235
#    0.19134241  0.46193975]
#  [ 0.4157348  -0.0975484  -0.49039263 -0.27778518  0.27778524  0.49039263
#    0.09754845 -0.4157348 ]
#  [ 0.35355341 -0.35355338 -0.35355338  0.35355341  0.35355341 -0.35355338
#   -0.35355338  0.35355341]
#  [ 0.27778524 -0.49039263  0.09754845  0.4157348  -0.4157348  -0.0975484
#    0.49039263 -0.27778518]
#  [ 0.19134241 -0.46193972  0.46193975 -0.19134235 -0.19134235  0.46193975
#   -0.46193972  0.19134241]
#  [ 0.09754845 -0.27778518  0.4157348  -0.49039263  0.49039263 -0.4157348
#    0.27778524 -0.0975484 ]]


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"}


def float32(value: float | np.floating[Any]) -> np.float32:
    return np.float32(value)


def float32_add(a: float | np.floating[Any], b: float | np.floating[Any]) -> np.float32:
    return np.float32(float32(a) + float32(b))


def float32_mul(a: float | np.floating[Any], b: float | np.floating[Any]) -> np.float32:
    return np.float32(float32(a) * float32(b))


def float32_sub(a: float | np.floating[Any], b: float | np.floating[Any]) -> np.float32:
    return np.float32(float32(a) - float32(b))


def float32_clip(value: float | np.floating[Any], low: float, high: float) -> np.float32:
    return np.float32(min(max(float32(value), float32(low)), float32(high)))


_TAYLOR_COEFF_CACHE: dict[int, list[np.float32]] = {}


def _get_taylor_coefficients(max_order: int) -> list[np.float32]:
    if max_order % 2 != 0:
        raise ValueError("Taylor series order must be an even integer.")
    if max_order < 0:
        raise ValueError("Taylor series order must be non-negative.")

    cached = _TAYLOR_COEFF_CACHE.get(max_order)
    if cached is not None:
        return cached

    coeffs: list[np.float32] = []
    for k in range(0, max_order // 2 + 1):
        numerator = float32((-1) ** k)
        denominator = float32(math.factorial(2 * k))
        coeff = float32(numerator / denominator)
        coeffs.append(coeff)

    _TAYLOR_COEFF_CACHE[max_order] = coeffs
    return coeffs


def _reduce_theta(theta: float) -> tuple[np.float32, np.float32]:
    wrapped = float32(((theta + math.pi) % (2 * math.pi)) - math.pi)
    sign = float32(1.0)
    half_pi = float32(math.pi / 2)

    if wrapped > half_pi:
        wrapped = float32_sub(float32(math.pi), wrapped)
        sign = float32(-1.0)
    elif wrapped < float32(-math.pi / 2):
        wrapped = float32_sub(float32(-math.pi), wrapped)
        sign = float32(-1.0)

    return wrapped, sign


def approximate_cos_taylor(theta: float, order: int) -> float:
    reduced_theta, sign = _reduce_theta(theta)
    # Constrain the reduced theta to half-pi interval to avoid large errors.
    half_pi = float32(math.pi / 2)
    eps = float32(1e-9)
    high = float32_sub(half_pi, eps)
    low = float32_add(float32(-half_pi), eps)
    reduced_theta = float32_clip(reduced_theta, low, high)

    x = reduced_theta
    x2 = float32_mul(x, x)

    coeffs = _get_taylor_coefficients(order)
    print("coeffs:", coeffs)
    result = coeffs[-1]

    for coeff in reversed(coeffs[:-1]):
        result = float32_add(float32_mul(result, x2), coeff)

    value = float32_mul(result, sign)
    value = float32_clip(value, -1.0, 1.0)

    return float(value)


def generate_dct_matrix_taylor(block_size: int, order: int) -> np.ndarray:
    matrix = np.zeros((block_size, block_size), dtype=np.float64)

    for i in range(block_size):
        for j in range(block_size):
            theta = i * (2 * j + 1) * math.pi / (2.0 * block_size)
            matrix[i, j] = float(approximate_cos_taylor(theta, order))

    matrix *= math.sqrt(2.0 / block_size)
    matrix[0, :] /= math.sqrt(2.0)
    return matrix


def block_dct_compress_taylor(
    image: np.ndarray,
    c_matrix: np.ndarray,
    quant_matrix: np.ndarray,
) -> tuple[np.ndarray, int]:
    """Compress and reconstruct using Taylor-based DCT matrix, returning Huffman bits."""

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
        description="Evaluate DCT compression using Taylor-series cosine approximation"
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
        default=Path("DCT_set12_taylor_results.log"),
        help="File to store per-image metrics.",
    )
    parser.add_argument(
        "--matrix-log",
        type=Path,
        default=Path("DCT_taylor_first_image_matrices.log"),
        help="Destination log file for the first image's gray and reconstruction matrices.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("DCT_taylor_outputs"),
        help="Directory to store reconstructed images.",
    )
    parser.add_argument(
        "--quant-scale",
        type=float,
        default=0.125,
        help="Scale factor p for the Hilbert-based quantization matrix.",
    )
    parser.add_argument(
        "--block-size",
        type=int,
        default=8,
        help="Block size for the DCT transform (default: 8).",
    )
    parser.add_argument(
        "--taylor-order",
        type=int,
        default=4,
        help="Maximum order (even) of the Taylor series used to approximate cosine.",
    )
    args = parser.parse_args()

    if not args.folder.is_dir():
        raise FileNotFoundError(f"Input folder {args.folder} not found or not a directory.")

    if args.taylor_order < 0 or args.taylor_order % 2 != 0:
        raise ValueError("Taylor order must be a non-negative even integer.")

    image_paths = gather_images(args.folder)
    if not image_paths:
        raise FileNotFoundError(
            f"No supported images found in {args.folder}. Expected extensions: {sorted(SUPPORTED_EXTENSIONS)}"
        )

    c_matrix = generate_dct_matrix_taylor(args.block_size, args.taylor_order)
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

        reconstructed, huffman_bits = block_dct_compress_taylor(
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

        output_name = image_path.stem + "_taylor_recon" + image_path.suffix
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
    log_lines.append(f"Totals,{avg_mse:.6f},{avg_psnr_str},{total_huffman_bits},{overall_ratio_str}")

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


