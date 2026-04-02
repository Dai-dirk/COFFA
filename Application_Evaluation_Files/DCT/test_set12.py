"""Batch-evaluate DCT reconstruction quality on a folder of grayscale images."""

from __future__ import annotations

import argparse
from pathlib import Path
import math

import numpy as np
from PIL import Image

from compression_utils import compress_image_with_matrices
from dct_block_compression import compute_mse_psnr, dct_matrix, hilbert_matrix


SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"}


# [[ 0.35355339  0.35355339  0.35355339  0.35355339  0.35355339  0.35355339
#    0.35355339  0.35355339]
#  [ 0.49039264  0.41573481  0.27778512  0.09754516 -0.09754516 -0.27778512
#   -0.41573481 -0.49039264]
#  [ 0.46193977  0.19134172 -0.19134172 -0.46193977 -0.46193977 -0.19134172
#    0.19134172  0.46193977]
#  [ 0.41573481 -0.09754516 -0.49039264 -0.27778512  0.27778512  0.49039264
#    0.09754516 -0.41573481]
#  [ 0.35355339 -0.35355339 -0.35355339  0.35355339  0.35355339 -0.35355339
#   -0.35355339  0.35355339]
#  [ 0.27778512 -0.49039264  0.09754516  0.41573481 -0.41573481 -0.09754516
#    0.49039264 -0.27778512]
#  [ 0.19134172 -0.46193977  0.46193977 -0.19134172 -0.19134172  0.46193977
#   -0.46193977  0.19134172]
#  [ 0.09754516 -0.27778512  0.41573481 -0.49039264  0.49039264 -0.41573481
#    0.27778512 -0.09754516]]


def load_grayscale(path: Path) -> np.ndarray:
    """Load an image from disk and return it as a 2D uint8 NumPy array."""

    return np.array(Image.open(path).convert("L"))


def gather_images(folder: Path) -> list[Path]:
    """Collect all supported image files in ``folder`` (non-recursive)."""

    return sorted(
        [p for p in folder.iterdir() if p.suffix.lower() in SUPPORTED_EXTENSIONS]
    )


def write_log(lines: list[str], log_path: Path) -> None:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Evaluate block DCT compression on grayscale images in a folder"
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
        default=Path("DCT_set12_results.log"),
        help="Destination log file for per-image metrics.",
    )
    parser.add_argument(
        "--quant-scale",
        type=float,
        default=0.75,
        help="Scale factor p for the Hilbert-based quantization matrix.",
    )
    parser.add_argument(
        "--block-size",
        type=int,
        default=8,
        help="Block size for the DCT transform (default: 8).",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("DCT_outputs"),
        help="Directory to store reconstructed images.",
    )
    args = parser.parse_args()

    if not args.folder.is_dir():
        raise FileNotFoundError(f"Input folder {args.folder} does not exist or is not a directory.")

    image_paths = gather_images(args.folder)
    if not image_paths:
        raise FileNotFoundError(
            f"No supported images found in {args.folder}. Expected extensions: {sorted(SUPPORTED_EXTENSIONS)}"
        )

    args.output_dir.mkdir(parents=True, exist_ok=True)

    log_lines = ["Image,MSE,PSNR(dB),HuffmanBits,CompressionRatio"]
    mse_values: list[float] = []
    psnr_values: list[float] = []
    huffman_bits_values: list[int] = []
    original_bits_values: list[int] = []
    compression_ratios: list[float] = []

    c_matrix = dct_matrix(args.block_size)
    quant_matrix = args.quant_scale * 8.0 / hilbert_matrix(args.block_size)

    for image_path in image_paths:
        gray = load_grayscale(image_path)

        height, width = gray.shape
        if height % args.block_size or width % args.block_size:
            raise ValueError(
                f"Image {image_path.name} has size {gray.shape}, which is not divisible by block size {args.block_size}."
            )

        reconstructed, huffman_bits = compress_image_with_matrices(
            gray, c_matrix=c_matrix, quant_matrix=quant_matrix
        )
        mse, psnr = compute_mse_psnr(gray, reconstructed)

        mse_values.append(mse)
        psnr_values.append(psnr if np.isfinite(psnr) else np.inf)
        huffman_bits_values.append(huffman_bits)

        num_blocks = (gray.shape[0] // args.block_size) * (gray.shape[1] // args.block_size)
        original_bits = num_blocks * args.block_size * args.block_size * 8
        original_bits_values.append(original_bits)

        compression_ratio = original_bits / huffman_bits if huffman_bits else math.inf
        compression_ratios.append(compression_ratio)

        psnr_str = "Infinity" if np.isinf(psnr) else f"{psnr:.3f}"
        ratio_str = "Infinity" if not math.isfinite(compression_ratio) else f"{compression_ratio:.3f}"
        log_lines.append(f"{image_path.name},{mse:.6f},{psnr_str},{huffman_bits},{ratio_str}")
        print(
            f"{image_path.name}: Huffman bits = {huffman_bits}, "
            f"compression ratio (original/huffman) = {ratio_str}"
        )

        output_name = image_path.stem + "_recon" + image_path.suffix
        output_path = args.output_dir / output_name
        Image.fromarray(reconstructed).save(output_path)

    avg_mse = float(np.mean(mse_values))
    finite_psnr = [v for v in psnr_values if np.isfinite(v)]
    avg_psnr = float(np.mean(finite_psnr)) if finite_psnr else float("inf")

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

    print(f"Processed {len(image_paths)} images from {args.folder}.")
    print(f"Average MSE: {avg_mse:.6f}")
    if avg_psnr_str == "Infinity":
        print("Average PSNR: Infinity (all reconstructions perfect)")
    else:
        print(f"Average PSNR: {avg_psnr:.4f} dB")
    print(f"Total Huffman bits: {total_huffman_bits}")
    if math.isfinite(avg_ratio):
        print(f"Average compression ratio (per-image mean): {avg_ratio:.3f}")
    else:
        print("Average compression ratio (per-image mean): Infinity")
    if math.isfinite(overall_ratio):
        print(f"Overall compression ratio (original/huffman): {overall_ratio:.3f}")
    else:
        print("Overall compression ratio (original/huffman): Infinity")
    print(f"Detailed results saved to {args.log}")


if __name__ == "__main__":
    main()

