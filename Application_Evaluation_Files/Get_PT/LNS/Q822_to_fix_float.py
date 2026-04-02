"""
Q822_to_fix_float.py - Python refactor of Tran_log_to_mn and Tran_Q822_to_float

All inputs/outputs use binary strings (MSB->LSB). WIDTH is 32.
"""

from typing import Tuple


def _bits(value: int, width: int) -> str:
    return format(value & ((1 << width) - 1), f"0{width}b")


def _slice_str(msb_lsb_str: str, msb: int, lsb: int) -> str:
    # msb_lsb_str[0] is MSB; index 31 is LSB for 32-bit, here lengths vary
    n = len(msb_lsb_str)
    # For a vector of length n, bit index i maps to string index (n-1 - i)
    start = n - 1 - msb
    end = n - 1 - lsb
    return msb_lsb_str[start:end + 1]


def tran_log_to_mn(log30_bits: str, n_bits: str) -> str:
    """
    Python version of Verilog Tran_log_to_mn.
    Inputs:
      - log30_bits: 30-bit signed value as bitstring (log30)
      - n_bits: 5-bit unsigned as bitstring
    Returns:
      - log32_bits: 32-bit bitstring
    """
    if len(log30_bits) != 30 or len(n_bits) != 5:
        raise ValueError('log30 must be 30 bits and n must be 5 bits')
    n = int(n_bits, 2)

    sign = log30_bits[0]  # MSB of 30-bit

    def sext(count: int) -> str:
        return sign * count

    # Helper to mirror the RTL cases (WIDTH=32 => WIDTH-3 == 29)
    # log30[WIDTH-3:x] => log30[29:x]
    if n == 0:
        return sext(24) + _slice_str(log30_bits, 29, 22)
    if n == 1:
        return sext(23) + _slice_str(log30_bits, 29, 21)
    if n == 2:
        return sext(22) + _slice_str(log30_bits, 29, 20)
    if n == 3:
        return sext(21) + _slice_str(log30_bits, 29, 19)
    if n == 4:
        return sext(20) + _slice_str(log30_bits, 29, 18)
    if n == 5:
        return sext(19) + _slice_str(log30_bits, 29, 17)
    if n == 6:
        return sext(18) + _slice_str(log30_bits, 29, 16)
    if n == 7:
        return sext(17) + _slice_str(log30_bits, 29, 15)
    if n == 8:
        return sext(16) + _slice_str(log30_bits, 29, 14)
    if n == 9:
        return sext(15) + _slice_str(log30_bits, 29, 13)
    if n == 10:
        return sext(14) + _slice_str(log30_bits, 29, 12)
    if n == 11:
        return sext(13) + _slice_str(log30_bits, 29, 11)
    if n == 12:
        return sext(12) + _slice_str(log30_bits, 29, 10)
    if n == 13:
        return sext(11) + _slice_str(log30_bits, 29, 9)
    if n == 14:
        return sext(10) + _slice_str(log30_bits, 29, 8)
    if n == 15:
        return sext(9) + _slice_str(log30_bits, 29, 7)
    if n == 16:
        return sext(8) + _slice_str(log30_bits, 29, 6)
    if n == 17:
        return sext(7) + _slice_str(log30_bits, 29, 5)
    if n == 18:
        return sext(6) + _slice_str(log30_bits, 29, 4)
    if n == 19:
        return sext(5) + _slice_str(log30_bits, 29, 3)
    if n == 20:
        return sext(4) + _slice_str(log30_bits, 29, 2)
    if n == 21:
        return sext(3) + _slice_str(log30_bits, 29, 1)
    if n == 22:
        return sext(2) + _slice_str(log30_bits, 29, 0)
    if n == 23:
        return sext(1) + _slice_str(log30_bits, 29, 0) + '0'
    if n == 24:
        return _slice_str(log30_bits, 29, 0) + '00'
    if n == 25:
        return log30_bits[0] + _slice_str(log30_bits, 27, 0) + '000'
    if n == 26:
        return log30_bits[0] + _slice_str(log30_bits, 26, 0) + '0000'
    if n == 27:
        return log30_bits[0] + _slice_str(log30_bits, 25, 0) + '00000'
    if n == 28:
        return log30_bits[0] + _slice_str(log30_bits, 24, 0) + '000000'
    if n == 29:
        return log30_bits[0] + _slice_str(log30_bits, 23, 0) + '0000000'
    if n == 30:
        return log30_bits[0] + _slice_str(log30_bits, 22, 0) + '00000000'
    if n == 31:
        return log30_bits[0] + _slice_str(log30_bits, 21, 0) + '000000000'
    # default
    return sign * 2 + _slice_str(log30_bits, 29, 0)


def tran_q822_to_float(log30_bits: str) -> str:
    """
    Python version of Verilog Tran_Q822_to_float.
    Input: log30_bits (30-bit signed)
    Output: log32_bits (32-bit IEEE-754)
    """
    if len(log30_bits) != 30:
        raise ValueError('log30 must be 30 bits')

    sign = log30_bits[0]
    # Compute absolute value in two's complement for 30-bit value
    if sign == '0':
        abs_x = log30_bits
    else:
        inv = ''.join('1' if c == '0' else '0' for c in log30_bits)
        abs_x = _bits((int(inv, 2) + 1) & ((1 << 30) - 1), 30)

    # Leading-one detection to get k matching RTL expectations.
    # Find first '1' from MSB (index 0) to LSB (index 29). If none, output 0.
    if '1' not in abs_x:
        return '0' * 32
    # print("abs_x:", abs_x)
    p = abs_x.find('1')  # 0..29 where 0 means bit29
    # Map to k per analysis: k = clamp(30 - (29 - p)) = clamp(1 + p)
    # print("p:", p)
    
    k = 2 + p
    if k < 2:
        k = 2
    if k > 31:
        k = 31

    # Build exponent and mantissa according to RTL case mapping
    if k == 2:
        exponent = 0x86
        mantissa = '0' * 23
    elif 3 <= k <= 8:
        exponent = 0x88 - k
        top = 30 - k
        hi = top
        lo = top - 22
        mantissa = _slice_str(abs_x, hi, lo)
    else:  # 9..31
        exponent = 0x88 - k
        top = 30 - k
        base = _slice_str(abs_x, top, 0) if top >= 0 else ''
        zeros = '0' * (k - 8)
        mantissa = (base + zeros)[-23:].rjust(23, '0')

    log32 = sign + _bits(exponent, 8) + mantissa
    return log32


if __name__ == '__main__':
    # Simple smoke tests
    # Tran_log_to_mn
    print(tran_log_to_mn('1' + '0' * 29, '00000'))
    # Tran_Q822_to_float on zero
    print(tran_q822_to_float('0' + '1' * 29))


