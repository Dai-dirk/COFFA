"""
CPA_tree_with_MAD.py - Python refactor of CPA_tree_with_MAD.v

- Inputs: six 32-bit bitstrings t0,t1,t2,t3,t4,bias and a float_flag (bool/int)
- Behavior: sum the six numbers using a CPA tree:
    s0 = t0 + t1
    s1 = t2 + t3
    s2 = t4 + bias
    s3 = s0 + s1
    tri_result = s3 + s2
- If float_flag==1: interpret inputs as IEEE-754 float32 and perform float32 additions
- Else: interpret inputs as signed 32-bit integers (two's complement) and perform 32-bit wraparound adds

Note: NaN/Inf are not specially handled (same as Verilog TODO). We perform float32
rounding at each addition step.
"""

import struct
import ctypes
from typing import Tuple


def bits32_to_uint(bits: str) -> int:
    if len(bits) != 32 or any(c not in '01' for c in bits):
        raise ValueError("Input must be a 32-bit binary string")
    return int(bits, 2)


def uint_to_bits32(value: int) -> str:
    return format(value & 0xFFFFFFFF, '032b')


def bits32_to_int_signed(bits: str) -> int:
    u = bits32_to_uint(bits)
    return u - 0x100000000 if (u & 0x80000000) else u


def int_signed_to_bits32(value: int) -> str:
    return uint_to_bits32(value & 0xFFFFFFFF)


def bits32_to_float(bits: str) -> float:
    u = bits32_to_uint(bits)
    b = u.to_bytes(4, byteorder='big')
    return struct.unpack('>f', b)[0]


def float_to_bits32(value: float) -> str:
    b = struct.pack('>f', value)
    u = int.from_bytes(b, byteorder='big')
    return uint_to_bits32(u)


def add_int32_bits(a_bits: str, b_bits: str) -> str:
    a = bits32_to_int_signed(a_bits)
    b = bits32_to_int_signed(b_bits)
    s = (a + b) & 0xFFFFFFFF
    return uint_to_bits32(s)


def add_float32_bits(a_bits: str, b_bits: str) -> str:
    # Parse fields
    ua = bits32_to_uint(a_bits)
    ub = bits32_to_uint(b_bits)
    sa = (ua >> 31) & 1
    sb = (ub >> 31) & 1
    ea = (ua >> 23) & 0xFF
    eb = (ub >> 23) & 0xFF
    ma = ua & 0x7FFFFF
    mb = ub & 0x7FFFFF

    # exponent difference via 9-bit signed arithmetic (ea - eb)
    subexp = ea - eb
    subexp_neg = 1 if subexp < 0 else 0
    abs_subexp = -subexp if subexp < 0 else subexp
    if abs_subexp > 31:
        abs_subexp = 31

    # normal_exponent: larger exponent
    normal_exponent = ea if subexp_neg == 0 else eb

    # Denormalized support: 24-bit fraction with hidden bit for normals
    frac0 = ((0 if ea == 0 else 1) << 23) | ma
    frac1 = ((0 if eb == 0 else 1) << 23) | mb

    # Select 'a' (23-bit) not shifted; select input to shift (24-bit)
    a_23 = mb if subexp_neg == 1 else ma
    input_shift_24 = frac0 if subexp_neg == 1 else frac1

    # Shift the smaller exponent's fraction right by abs_subexp (cap 31)
    b_24 = (input_shift_24 >> abs_subexp) & ((1 << 24) - 1)

    # Determine add/sub based on signs
    same_sign = (sa == sb)
    sub_flag = 0 if same_sign else 1
    # Compose operands for 32-bit CPA as in RTL
    cpa_a = (0 << 24) | (1 << 23) | (a_23 & 0x7FFFFF)  # {8'b0,1'b1,a}
    if same_sign:
        add_b = (0 << 24) | b_24                           # {8'b0,b}
    else:
        add_b = ((0xFF << 24) | ((~b_24) & ((1 << 24) - 1)))  # {8'hFF, ~b}

    # 32-bit addition with optional carry-in=1 for subtraction
    result = (cpa_a + add_b + sub_flag) & 0xFFFFFFFF

    # abs_result on 26-bit slice [25:0]
    result_26 = result & ((1 << 26) - 1)
    result_26_sign = (result_26 >> 25) & 1
    if result_26_sign == 0:
        abs_result_25 = result_26 & ((1 << 25) - 1)
    else:
        abs_result_25 = ((~result_26 + 1) & ((1 << 26) - 1)) & ((1 << 25) - 1)

    # LOD equivalent: x_in = abs_result[24:0]
    x_in = abs_result_25 & ((1 << 25) - 1)
    top_bit = (x_in >> 24) & 1
    if top_bit == 1:
        # special path
        k = 63  # 6'b111111
        new_result = (x_in >> 1) & ((1 << 23) - 1)
    else:
        # find leading one in lower 24 bits [23:0]
        y = x_in & ((1 << 24) - 1)
        if y == 0:
            k_ = 0
        else:
            # count leading zeros in 24 bits
            k_ = 0
            for i in range(23, -1, -1):
                if (y >> i) & 1:
                    # leading one at i => k_ equals number of zeros before it: 23 - i
                    k_ = 23 - i
                    break
        k = k_  # 6'b0_k_
        new_result = ((y & ((1 << 23) - 1)) << k_) & ((1 << 23) - 1)

    # final_sub_k = - sign-extended k (6-bit to 9-bit)
    k_signed = k - 64 if (k & 0x20) else k
    final_sub_k = -k_signed
    # final_sub_exponent (9-bit)
    final_sub_exponent = (normal_exponent + final_sub_k) & 0x1FF

    # underflow/overflow detection
    underflow_temp = (sa ^ sb == 1) and ((final_sub_exponent >> 8) == 1 or k == 31)
    overflow_temp = (normal_exponent == 0xFF) and (sa ^ sb == 0)

    if underflow_temp:
        final_exp = 0
        final_man = 0
    elif overflow_temp:
        final_exp = 0xFF
        final_man = (1 << 23) - 1
    else:
        final_exp = final_sub_exponent & 0xFF
        final_man = new_result & 0x7FFFFF

    # Determine final sign per RTL case({sa,sb, subexp[8], result[24]})
    subexp_msb = subexp_neg
    res_bit24 = (result >> 24) & 1
    case_val = (sa << 3) | (sb << 2) | (subexp_msb << 1) | res_bit24
    if case_val in (0b0000, 0b0001, 0b0010, 0b0011):
        final_sign = 0
    elif case_val in (0b1100, 0b1101, 0b1110, 0b1111):
        final_sign = 1
    elif case_val == 0b0100:
        final_sign = 0
    elif case_val == 0b0101:
        final_sign = 1
    elif case_val == 0b0110:
        final_sign = 1
    elif case_val == 0b0111:
        final_sign = 0
    elif case_val == 0b1000:
        final_sign = 1
    elif case_val == 0b1001:
        final_sign = 0
    elif case_val == 0b1010:
        final_sign = 0
    elif case_val == 0b1011:
        final_sign = 1
    else:
        final_sign = 0

    out_u = ((final_sign & 1) << 31) | ((final_exp & 0xFF) << 23) | (final_man & 0x7FFFFF)
    return uint_to_bits32(out_u)

    


def cpa_tree_with_mad(t0: str, t1: str, t2: str, t3: str, t4: str, bias: str, float_flag: int) -> str:
    if float_flag:
        add_bits = add_float32_bits
    else:
        add_bits = add_int32_bits

    s0 = add_bits(t0, t1)
    s1 = add_bits(t2, t3)
    s2 = add_bits(t4, bias)
    s3 = add_bits(s0, s1)
    tri_result = add_bits(s3, s2)
    return tri_result


if __name__ == '__main__':
    # Quick self-test
    # Fixed-point: 1+2+3+4+5+6 = 21
    t0 = '00000000000000000000000000000001'
    t1 = '00000000000000000000000000000010'
    t2 = '00000000000000000000000000000011'
    t3 = '00000000000000000000000000000100'
    t4 = '00000000000000000000000000000101'
    bias = '00000000000000000000000000000110'
    res_fixed = cpa_tree_with_mad(t0, t1, t2, t3, t4, bias, 0)
    # print('fixed result int:', bits32_to_int_signed(res_fixed), 'bits:', res_fixed)

    # Float: 1.0+2.0+3.0+4.0+5.0+6.0 = 21.0
    def fbits(f: float) -> str:
        return float_to_bits32(ctypes.c_float(f).value)

    t0f = fbits(1.0)
    t1f = fbits(2.0)
    t2f = fbits(3.0)
    t3f = fbits(4.0)
    t4f = fbits(5.0)
    biasf = fbits(6.0)
    res_float_bits = cpa_tree_with_mad(t0f, t1f, t2f, t3f, t4f, biasf, 1)
    res_float = bits32_to_float(res_float_bits)
    # print('float result:', res_float, 'bits:', res_float_bits)


