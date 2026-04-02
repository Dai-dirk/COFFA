"""Shared helpers for block-based DCT compression experiments."""

from __future__ import annotations

from typing import Tuple

import numpy as np

from jpeg_huffman import jpeg_block_huffman_bits


def compress_image_with_matrices(
    image: np.ndarray,
    c_matrix: np.ndarray,
    quant_matrix: np.ndarray,
) -> Tuple[np.ndarray, int]:
    """Compress and reconstruct ``image`` using the provided DCT matrix and quantizer.

    Args:
        image: Grayscale image as a 2D uint8 NumPy array.
        c_matrix: Orthonormal transform matrix (typically 8×8).
        quant_matrix: Quantization matrix matching ``c_matrix`` dimensions.

    Returns:
        A tuple ``(reconstructed_image, huffman_bits)`` where ``reconstructed_image`` is
        the uint8 result of the inverse transform and ``huffman_bits`` is the cumulative
        JPEG luminance Huffman bit length over all processed blocks.
    """

    if image.ndim != 2:
        raise ValueError("Expected a 2D grayscale image.")

    block_size = c_matrix.shape[0]
    if image.shape[0] % block_size or image.shape[1] % block_size:
        raise ValueError("Image dimensions must be divisible by the block size.")

    output = np.empty_like(image)
    prev_dc = 0
    total_bits = 0
    c_t = c_matrix.T

    for row in range(0, image.shape[0], block_size):
        for col in range(0, image.shape[1], block_size):
            block = image[row : row + block_size, col : col + block_size].astype(np.float64)
            shifted = block - 128.0
            dct_coeffs = c_matrix @ shifted @ c_t
            quantized = np.round(dct_coeffs / quant_matrix).astype(np.int32)

            bits, prev_dc = jpeg_block_huffman_bits(quantized, prev_dc)
            total_bits += bits

            recovered = c_t @ (quantized.astype(np.float64) * quant_matrix) @ c_matrix
            restored = recovered + 128.0
            output[row : row + block_size, col : col + block_size] = np.clip(
                restored, 0, 255
            ).astype(np.uint8)

    return output, total_bits


