"""Replicate a MATLAB-style block DCT compression demo in Python.

This script reads an RGB image, converts it to grayscale using the same
coefficients as the MATLAB snippet, and then performs an 8×8 block DCT-based
compression and reconstruction pipeline that mirrors the example logic.

Usage (from the repository root):

```
python -m DCT.dct_block_compression --color-image XX.jpg --gray-image SCUgray.jpg
```

The script will show the intermediate grayscale image and the reconstructed
output produced by the block-DCT pipeline. Adjust file paths and the quantizer
scaling factor `p` with command-line options as needed.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from PIL import Image


def rgb_to_weighted_gray(rgb: np.ndarray) -> np.ndarray:
    """Convert an RGB image to grayscale with ITU-R BT.709 coefficients."""

    weights = np.array([0.2126, 0.7152, 0.0722], dtype=np.float64)
    gray = np.tensordot(rgb.astype(np.float64), weights, axes=([2], [0]))
    return np.clip(gray, 0, 255).astype(np.uint8)


def hilbert_matrix(n: int) -> np.ndarray:
    """Create the size-``n`` Hilbert matrix (entries 1/(i+j-1))."""

    i = np.arange(1, n + 1, dtype=np.float64)
    j = i[:, None]
    return 1.0 / (j + i[None, :] - 1.0)


def dct_matrix(n: int) -> np.ndarray:
    """Construct the orthonormal DCT-II transform matrix of size ``n``."""

    k = np.arange(n, dtype=np.float64)
    j = (2 * np.arange(1, n + 1, dtype=np.float64) - 1.0)[None, :]
    mat = np.cos((k[:, None]) * j * np.pi / (2.0 * n))
    mat *= np.sqrt(2.0 / n)
    mat[0, :] /= np.sqrt(2.0)
    return mat


def block_dct_compress(
    image: np.ndarray,
    block_size: int = 8,
    quant_scale: float = 1.0,
) -> np.ndarray:
    """Apply block DCT compression with the MATLAB-style Hilbert quantizer."""

    if image.ndim != 2:
        raise ValueError("Expected a single-channel (grayscale) image.")

    height, width = image.shape
    if height % block_size or width % block_size:
        raise ValueError(
            "Image dimensions must be divisible by the block size for this demo."
        )

    c = dct_matrix(block_size)
    # print(c)
    q = quant_scale * 8.0 / hilbert_matrix(block_size)
    # print(q)

    output = np.empty_like(image)

    for row in range(0, height, block_size):
        for col in range(0, width, block_size):
            block = image[row : row + block_size, col : col + block_size].astype(np.float64)
            shifted = block - 128.0
            dct_coeffs = c @ shifted @ c.T
            quantized = np.round(dct_coeffs / q)
            # if row == 0 and col == 0:
            #     print(quantized)

            recovered = c.T @ (quantized * q) @ c
            restored = recovered + 128.0
            output[row : row + block_size, col : col + block_size] = np.clip(
                restored, 0, 255
            ).astype(np.uint8)

    return output


def load_image(path: Path) -> np.ndarray:
    """Load an image file into a NumPy array (always returns an RGB image)."""

    img = Image.open(path).convert("RGB")
    return np.array(img)


def compute_mse_psnr(original: np.ndarray, reconstructed: np.ndarray) -> tuple[float, float]:
    """Compute the MSE and PSNR between two grayscale images."""

    if original.shape != reconstructed.shape:
        raise ValueError("Images must share the same dimensions for comparison.")

    diff = original.astype(np.float64) - reconstructed.astype(np.float64)
    print("max(diff), min(diff):", np.max(diff), np.min(diff))
    mse = np.mean(diff ** 2)
    if mse == 0:
        return 0.0, float("inf")

    psnr = 20.0 * np.log10(255.0 / np.sqrt(mse))
    return mse, psnr


def main() -> None:
    parser = argparse.ArgumentParser(description="8x8 block DCT compression demo")
    parser.add_argument(
        "--color-image",
        type=Path,
        required=True,
        help="Path to the original RGB image (e.g., XX.jpg)",
    )
    parser.add_argument(
        "--gray-image",
        type=Path,
        required=True,
        help="Path to the pre-converted grayscale image (e.g., SCUgray.jpg)",
    )
    parser.add_argument(
        "--quant-scale",
        type=float,
        default=1.0,
        help="Scale factor p applied to the Hilbert-based quantization matrix.",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the grayscale and reconstructed images with matplotlib.",
    )
    args = parser.parse_args()

    color_image = load_image(args.color_image)
    gray_by_formula = rgb_to_weighted_gray(color_image)

    gray_image = Image.open(args.gray_image).convert("L")
    gray_array = np.array(gray_image)

    reconstructed = block_dct_compress(gray_array, block_size=8, quant_scale=args.quant_scale)
    mse, psnr = compute_mse_psnr(gray_array, reconstructed)

    if args.show:
        plt.figure(figsize=(12, 4))
        plt.subplot(1, 3, 1)
        plt.title("Original RGB")
        plt.imshow(color_image)
        plt.axis("off")

        plt.subplot(1, 3, 2)
        plt.title("Grayscale (Formula)")
        plt.imshow(gray_by_formula, cmap="gray")
        plt.axis("off")

        plt.subplot(1, 3, 3)
        plt.title("DCT Reconstruction")
        plt.imshow(reconstructed, cmap="gray")
        plt.axis("off")
        plt.tight_layout()
        plt.show()

    save_path = args.gray_image.with_name(
        f"{args.gray_image.stem}_dct_recon_p{args.quant_scale:g}{args.gray_image.suffix}"
    )
    Image.fromarray(reconstructed).save(save_path)
    print(f"Saved reconstructed image to {save_path}")
    print(f"MSE: {mse:.6f}")
    if np.isinf(psnr):
        print("PSNR: Infinity (perfect reconstruction)")
    else:
        print(f"PSNR: {psnr:.3f} dB")


if __name__ == "__main__":
    main()

