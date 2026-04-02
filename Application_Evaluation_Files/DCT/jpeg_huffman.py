"""Utilities to estimate JPEG Huffman coding bit lengths for 8×8 DCT blocks."""

from __future__ import annotations

import math
from typing import Iterable, Sequence, Tuple

import numpy as np

# Canonical luminance DC Huffman table definitions from the JPEG standard (Annex K)
_STD_LUMA_DC_BITS: Sequence[int] = (
    0,
    1,
    5,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
)

_STD_LUMA_DC_HUFFVAL: Sequence[int] = tuple(range(12))

# Canonical luminance AC Huffman table definitions from the JPEG standard (Annex K)
_STD_LUMA_AC_BITS: Sequence[int] = (
    0x00,
    0x02,
    0x01,
    0x03,
    0x03,
    0x02,
    0x04,
    0x03,
    0x05,
    0x05,
    0x04,
    0x04,
    0x00,
    0x00,
    0x01,
    0x7D,
)

_STD_LUMA_AC_HUFFVAL: Sequence[int] = (
    0x01,
    0x02,
    0x03,
    0x00,
    0x04,
    0x11,
    0x05,
    0x12,
    0x21,
    0x31,
    0x41,
    0x06,
    0x13,
    0x51,
    0x61,
    0x07,
    0x22,
    0x71,
    0x14,
    0x32,
    0x81,
    0x91,
    0xA1,
    0x08,
    0x23,
    0x42,
    0xB1,
    0xC1,
    0x15,
    0x52,
    0xD1,
    0xF0,
    0x24,
    0x33,
    0x62,
    0x72,
    0x82,
    0x09,
    0x0A,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1A,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6A,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7A,
    0x83,
    0x84,
    0x85,
    0x86,
    0x87,
    0x88,
    0x89,
    0x8A,
    0x92,
    0x93,
    0x94,
    0x95,
    0x96,
    0x97,
    0x98,
    0x99,
    0x9A,
    0xA2,
    0xA3,
    0xA4,
    0xA5,
    0xA6,
    0xA7,
    0xA8,
    0xA9,
    0xAA,
    0xB2,
    0xB3,
    0xB4,
    0xB5,
    0xB6,
    0xB7,
    0xB8,
    0xB9,
    0xBA,
    0xC2,
    0xC3,
    0xC4,
    0xC5,
    0xC6,
    0xC7,
    0xC8,
    0xC9,
    0xCA,
    0xD2,
    0xD3,
    0xD4,
    0xD5,
    0xD6,
    0xD7,
    0xD8,
    0xD9,
    0xDA,
    0xE1,
    0xE2,
    0xE3,
    0xE4,
    0xE5,
    0xE6,
    0xE7,
    0xE8,
    0xE9,
    0xEA,
    0xF1,
    0xF2,
    0xF3,
    0xF4,
    0xF5,
    0xF6,
    0xF7,
    0xF8,
    0xF9,
    0xFA,
)


def _build_code_length_table(bits: Sequence[int], symbols: Sequence[int]) -> dict[int, int]:
    """Build a mapping from symbol to canonical Huffman code length."""

    table: dict[int, int] = {}
    index = 0
    for bit_length, count in enumerate(bits, start=1):
        for _ in range(count):
            if index >= len(symbols):
                raise ValueError("Inconsistent Huffman table definition.")
            table[int(symbols[index])] = bit_length
            index += 1
    if index != len(symbols):
        raise ValueError("Unused Huffman symbols remain after table construction.")
    return table


LUMA_DC_CODE_LENGTHS = _build_code_length_table(_STD_LUMA_DC_BITS, _STD_LUMA_DC_HUFFVAL)
LUMA_AC_CODE_LENGTHS = _build_code_length_table(_STD_LUMA_AC_BITS, _STD_LUMA_AC_HUFFVAL)


def generate_zigzag_indices(n: int = 8) -> Tuple[Tuple[int, int], ...]:
    """Generate zigzag scan indices for an n×n block."""

    indices: list[Tuple[int, int]] = []
    for s in range(2 * n - 1):
        if s % 2 == 0:
            for i in range(s, -1, -1):
                j = s - i
                if 0 <= i < n and 0 <= j < n:
                    indices.append((i, j))
        else:
            for j in range(s, -1, -1):
                i = s - j
                if 0 <= i < n and 0 <= j < n:
                    indices.append((i, j))
    return tuple(indices)


ZIGZAG_INDICES = generate_zigzag_indices(8)


def zigzag_scan(block: np.ndarray) -> np.ndarray:
    """Return the zigzag-ordered coefficients of an 8×8 block."""

    if block.shape != (8, 8):
        raise ValueError("JPEG zigzag scan is defined for 8x8 blocks.")
    return np.array([block[i, j] for i, j in ZIGZAG_INDICES], dtype=np.int32)


def _category(value: int) -> int:
    """Compute the JPEG category (magnitude bit length) for a coefficient."""

    value = int(value)
    if value == 0:
        return 0
    return int(math.floor(math.log2(abs(value)))) + 1


def jpeg_block_huffman_bits(block: np.ndarray, prev_dc: int) -> tuple[int, int]:
    """Estimate the Huffman bit length for a single quantized 8×8 block.

    Args:
        block: Quantized coefficients as an (8, 8) integer array.
        prev_dc: The DC coefficient of the previous block in scan order.

    Returns:
        A tuple ``(bit_length, new_prev_dc)``. ``bit_length`` is the number of
        Huffman-coded bits required for the block; ``new_prev_dc`` is the block's
        DC coefficient for chaining.
    """

    coeffs = zigzag_scan(block)

    dc_coeff = int(coeffs[0])
    diff = dc_coeff - int(prev_dc)
    dc_category = _category(diff)
    try:
        dc_bits = LUMA_DC_CODE_LENGTHS[dc_category]
    except KeyError as exc:
        raise ValueError(f"Unsupported DC category {dc_category}.") from exc

    total_bits = dc_bits + dc_category

    run_length = 0
    for value in coeffs[1:]:
        value = int(value)
        if value == 0:
            run_length += 1
            continue

        while run_length >= 16:
            total_bits += LUMA_AC_CODE_LENGTHS[0xF0]
            run_length -= 16

        size = _category(value)
        symbol = (run_length << 4) | size
        try:
            ac_bits = LUMA_AC_CODE_LENGTHS[symbol]
        except KeyError as exc:
            raise ValueError(f"Unsupported AC symbol 0x{symbol:02X}.") from exc

        total_bits += ac_bits + size
        run_length = 0

    if run_length > 0:
        total_bits += LUMA_AC_CODE_LENGTHS[0x00]

    return total_bits, dc_coeff


def jpeg_huffman_bitcount(blocks: Iterable[np.ndarray]) -> int:
    """Compute the total Huffman-coded bit length for a sequence of 8×8 blocks."""

    total_bits = 0
    prev_dc = 0
    for block in blocks:
        bits, prev_dc = jpeg_block_huffman_bits(block, prev_dc)
        total_bits += bits
    return total_bits


