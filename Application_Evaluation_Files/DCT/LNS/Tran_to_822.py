"""
Tran_to_822.py - Python refactor of Tran_to_822 and FLOAT_TRAN_TO_Q822

All inputs/outputs are binary strings (MSB->LSB). WIDTH is 32.
"""

from typing import Tuple


def _u32(bits: str) -> int:
    return int(bits, 2) & 0xFFFFFFFF


def _bits(value: int, width: int) -> str:
    return format(value & ((1 << width) - 1), f"0{width}b")


def tran_to_822(B0_bits: str, n_bits: str) -> str:
    """
    Python version of Verilog Tran_to_822.
    Inputs:
      - B0_bits: 32-bit two's complement integer as bitstring
      - n_bits: 5-bit unsigned as bitstring
    Returns:
      - new_B0: 30-bit bitstring
    """
    if len(B0_bits) != 32 or len(n_bits) != 5:
        raise ValueError('B0 must be 32 bits and n must be 5 bits')
    B0 = _u32(B0_bits)
    n = int(n_bits, 2)
    sign_bit = (B0 >> 31) & 1
    # Build a list of B0 bits MSB->LSB for slicing similar to Verilog
    b0_str = _bits(B0, 32)

    def slice_msb_to_lsb(msb: int, lsb: int) -> str:
        # Verilog [msb:lsb], with msb>=lsb, both inclusive
        # Our string index 0 is MSB bit31
        start = 31 - msb
        end = 31 - lsb
        return b0_str[start:end + 1]

    if 0 <= n <= 22:
        # {B0[31], B0[6+n:0], (22-n)'b0}
        mid = slice_msb_to_lsb(6 + n, 0)
        zeros = '0' * (22 - n)
        return f"{sign_bit}{mid}{zeros}"
    elif n == 23:
        # {B0[31], B0[29:1]}
        mid = slice_msb_to_lsb(29, 1)
        return f"{sign_bit}{mid}"
    elif n == 24:
        # {B0[31], B0[30:2]}
        mid = slice_msb_to_lsb(30, 2)
        return f"{sign_bit}{mid}"
    elif 25 <= n <= 31:
        # {{(n-24){B0[31]}}, B0[31:(n-?)]}
        # From RTL: 25: {B0[31], B0[31:3]}
        #           26: {{2{B0[31]}}, B0[31:4]}
        # ... up to 31: {{7{B0[31]}}, B0[31:9]}
        rep = n - 24
        lsb = n - 22  # 25->3, 26->4, ..., 31->9
        prefix = str(sign_bit) * rep
        mid = slice_msb_to_lsb(31, lsb)
        return f"{prefix}{mid}"
    else:
        # default: B0[31:2]
        mid = slice_msb_to_lsb(31, 2)
        return mid


def float_tran_to_q822(float_y_bits: str) -> Tuple[str, str, str]:
    """
    Python version of Verilog FLOAT_TRAN_TO_Q822.
    Input:
      - float_y_bits: 32-bit IEEE-754 as bitstring
    Returns:
      - Q822y: 30-bit bitstring
      - overflow_y: '0'/'1'
      - underflow_y: '0'/'1'
    """
    if len(float_y_bits) != 32:
        raise ValueError('float_y must be 32 bits')

    s = int(float_y_bits[0], 2)
    exp = int(float_y_bits[1:9], 2)
    mant = int(float_y_bits[9:], 2)

    # exponent = {1'b0, exp} + 0x81 (8-bit wrap)
    exponent = ((exp) + 0x81) & 0xFF

    # Q2_23 (25-bit signed)
    if s == 0:
        q = (0 << 24) | (1 << 23) | mant
    else:
        q = ((1 << 24) | (0 << 23) | ((~mant) & ((1 << 23) - 1)))
        q = (q + 1) & ((1 << 25) - 1)

    def q_bits() -> str:
        return _bits(q, 25)

    incase = False
    Q822y = '0' * 30

    # Helper for sign bit of q (bit24)
    def q_sign() -> str:
        return q_bits()[0]

    # Cases 0..6
    if 0x00 <= exponent <= 0x06:
        incase = True
        if exponent == 0x00:
            # {{6{q[24]}}, q[24:1]}
            qb = q_bits()
            body = qb[0:24]  # bits 24..1 -> first 24 bits
            Q822y = q_sign() * 6 + body
        elif exponent == 0x01:
            # {{5{q[24]}}, q[24:0]}
            Q822y = q_sign() * 5 + q_bits()
        else:
            # {{(6-exp){q[24]}}, q[24:0], (exp-1)'b0}
            signrep = 6 - exponent
            zeros = exponent - 1
            Q822y = q_sign() * signrep + q_bits() + ('0' * zeros)
    # Cases FF..F0
    elif 0xF0 <= exponent <= 0xFF:
        incase = True
        # shift_right = (0xFF - exponent) + 2
        shift = (0xFF - exponent) + 2
        signrep = 7 + (0xFF - exponent)
        qb = q_bits()
        # q[24:shift] -> first (25-shift) bits
        body = qb[0:25 - shift]
        Q822y = q_sign() * signrep + body
    else:
        incase = False
        Q822y = '0' * 30

    overflow_y = '1' if (((exponent >> 7) & 1) == 0 and not incase) else '0'
    underflow_y = '1' if (((exponent >> 7) & 1) == 1 and not incase) else '0'

    return Q822y, overflow_y, underflow_y


if __name__ == '__main__':
    # Basic smoke tests
    # tran_to_822
    print(tran_to_822('0' * 31 + '1', '00000'))  # n=0
    # float_tran_to_q822 for 1.0 (0x3F800000)
    q, ov, uv = float_tran_to_q822('00111111100000000000000000000000')
    print(q, ov, uv)


