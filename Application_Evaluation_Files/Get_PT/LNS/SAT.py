"""
SAT.py - Python implementations of SAT_with_TRG_VEC_overflow and SAT_with_VEC_overflow

Inputs/outputs follow the Verilog modules in LNS_Top.v. All bit-vectors are
binary strings. WIDTH is assumed to be 32, so input_sat must be 38 bits
([WIDTH+5:0]).
"""

from typing import Tuple


def _bit(x) -> int:
    if isinstance(x, str):
        return 1 if x == '1' else 0
    return 1 if x else 0


def _twobits_to_tuple(s: str) -> Tuple[int, int]:
    if len(s) != 2 or any(c not in '01' for c in s):
        raise ValueError('Two-bit string expected')
    # s[1] is MSB in Verilog indexing [1:0]
    return (int(s[0]), int(s[1]))


def _bin_concat(bits: list) -> str:
    return ''.join(str(int(b)) for b in bits)


def SAT_with_TRG_VEC_overflow(
    input_sat: str,
    TRi: str,
    TRG: str,
    VEC: str,
    TRG_B_sign: str,
    Tri_sign: str,
    alogc_stage_power_sign: str,
    input_logA_zero_sign: str,
    input_logB_zero_sign: str,
    input_logC_zero_sign: str,
    alogc_stage_logA_sign: str,
) -> Tuple[str, str, str, str]:
    """
    Returns (Tri_overflow_one, TRG_overflow, VEC_overflow, input_anti) as strings.
    input_sat: 38-bit string (WIDTH+6 with WIDTH=32) indexing [WIDTH+5:0]
    *_sign flags and *_zero_sign are bit/two-bit strings.
    """
    WIDTH = 32
    if len(input_sat) != WIDTH + 6:
        raise ValueError('input_sat must be 38 bits for WIDTH=32')

    tri = _bit(TRi)
    trg = _bit(TRG)
    vec = _bit(VEC)
    trg_b_sign = _bit(TRG_B_sign)
    tri_sign = _bit(Tri_sign)
    power_sign = _bit(alogc_stage_power_sign)
    # print("input_logA_zero_sign:", input_logA_zero_sign)
    # print("input_logB_zero_sign:", input_logB_zero_sign)
    # print("tri_sign:", tri_sign)
    # print("input_logC_zero_sign:", input_logC_zero_sign)
    # print("alogc_stage_logA_sign:", alogc_stage_logA_sign)
    logA1, logA0 = _twobits_to_tuple(input_logA_zero_sign)
    logB1, logB0 = _twobits_to_tuple(input_logB_zero_sign)
    logC1, logC0 = _twobits_to_tuple(input_logC_zero_sign)
    logA_sign = _bit(alogc_stage_logA_sign)

    # Slicing per Verilog: input_sat[WIDTH-3:0] and [WIDTH+5:WIDTH-3]
    # Python strings are MSB->LSB left-to-right; assume input_sat[0] is MSB.
    # Compute indices accordingly.
    # MSB index = 0, LSB index = -1 (via negative)
    # For WIDTH=32: upper9 = [37:29], lower30 = [29:0]
    upper9 = input_sat[0:9]  # bits 37..29
    lower30 = input_sat[8:]  # bits 28..0? We need 30 LSBs total; ensure length 30
    if len(lower30) != 29 + 1:  # 30
        # Derive programmatically to avoid confusion
        raise ValueError('lower30 slicing error')

    # Build input_anti
    if vec == 1:
        b31 = logA1 | logB1
        b30 = logA0 ^ logB0
        input_anti = _bin_concat([b31, b30]) + lower30
    elif power_sign == 1:
        b31 = logA1
        b30 = 0
        input_anti = _bin_concat([b31, b30]) + lower30
    elif trg == 0 and logB1 == 1:
        input_anti = '0' * WIDTH
    elif trg == 0:
        b31 = logA1
        b30 = logA0
        input_anti = _bin_concat([b31, b30]) + lower30
    elif tri == 1 and tri_sign == 0:
        b31 = logA1 | logC1
        b30 = 0 ^ logC0
        input_anti = _bin_concat([b31, b30]) + lower30
    else:
        b31 = logA1 | logC1
        b30 = logA0 ^ logC0
        input_anti = _bin_concat([b31, b30]) + lower30

    # Tri_overflow_one
    if tri == 0 or (tri == 1 and (upper9 == '111111111' or upper9 == '000000000')):
        Tri_overflow_one = '0'
    else:
        Tri_overflow_one = '1'

    # TRG_overflow
    if trg == 1 or (trg == 0 and (((logA_sign ^ trg_b_sign) == 1 and upper9 == '111111111') or ((logA_sign ^ trg_b_sign) == 0 and upper9 == '000000000'))):
        TRG_overflow = '0'
    else:
        TRG_overflow = '1'

    # VEC_overflow
    if vec == 0 or (vec == 1 and (upper9 == '111111111' or upper9 == '000000000')):
        VEC_overflow = '0'
    else:
        VEC_overflow = '1'

    return Tri_overflow_one, TRG_overflow, VEC_overflow, input_anti


def SAT_with_VEC_overflow(
    input_sat: str,
    TRi: str,
    input_logA_zero_sign: str,
    input_logC_zero_sign: str,
    alogc_stage_logA_sign: str,  # unused in this block but kept for signature parity
    alogc_stage_B_sign: str,      # unused
    Tri_sign: str,
) -> Tuple[str, str]:
    """
    Returns (Tri_overflow_one, input_anti) as strings.
    input_sat: 38-bit string; others are bit/two-bit strings.
    """
    WIDTH = 32
    if len(input_sat) != WIDTH + 6:
        raise ValueError('input_sat must be 38 bits for WIDTH=32')

    tri = _bit(TRi)
    tri_sign = _bit(Tri_sign)
    logA1, logA0 = _twobits_to_tuple(input_logA_zero_sign)
    logC1, logC0 = _twobits_to_tuple(input_logC_zero_sign)
    # print("input_logA_zero_sign:", input_logA_zero_sign)
    # print("tri_sign:", tri_sign)
    # print("input_logC_zero_sign:", input_logC_zero_sign)

    upper9 = input_sat[0:9]
    lower30 = input_sat[8:]
    if len(lower30) != 30:
        raise ValueError('lower30 slicing error')

    if tri == 1 and tri_sign == 0:
        b31 = logA1 | logC1
        b30 = 0 ^ logC0
        input_anti = _bin_concat([b31, b30]) + lower30
    else:
        b31 = logA1 | logC1
        b30 = logA0 ^ logC0
        input_anti = _bin_concat([b31, b30]) + lower30

    if tri == 0 or (tri == 1 and (upper9 == '111111111' or upper9 == '000000000')):
        Tri_overflow_one = '0'
    else:
        Tri_overflow_one = '1'

    return Tri_overflow_one, input_anti


