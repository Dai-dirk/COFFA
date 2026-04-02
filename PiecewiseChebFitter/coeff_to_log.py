import random
from deap import base, creator, tools, algorithms
from typing import List, Tuple, Any
import numpy as np
import math

def float_to_fixed(value: float) -> str:
    # 定义Q8.16格式的参数
    integer_bits = 9  # 包括符号位
    fractional_bits = 16
    total_bits = integer_bits + fractional_bits  # 24 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 检查是否超出 Q8.16 的表示范围
    max_value = (1 << (total_bits)) - 1  # 最大值：2^21 - 1

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))
    print(f"fixed_point_value: {fixed_point_value}") # 处理负数（转换为补码）
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 补码表示
    # fixed_point_value = min(max(fixed_point_value, 0), max_value)  # 限制在0~0xFFFFFF之间

    if fixed_point_value > max_value :
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 确保输出是 24 位（使用掩码截断高位）
    fixed_point_value &= (1 << (total_bits - 1)) - 1

    int_part = fixed_point_value >> fractional_bits
    frac_part = fixed_point_value & ((1 << fractional_bits) - 1)
    print(f"int_part: {int_part}; frac_part: {frac_part}") # 处理负数（转换为补码）
    
    # 格式化为二进制字符串
    bin_int = bin(int_part)[2:].zfill(8)
    bin_frac = bin(frac_part)[2:].zfill(16)
    return f"{bin_int}{bin_frac}"

def coeff_to_log(value):
    if value == 0:
        bin_log = '10000000000000000000000000'
    elif value > 0:
        log_value = math.log2(value)
        print(f"log_value: {log_value}")
        bin_log = f"00{float_to_fixed(log_value)}"
    else:
        log_value = math.log2(abs(value))
        bin_log = f"01{float_to_fixed(log_value)}"
    return bin_log

if __name__ == "__main__":
  result = coeff_to_log(1/3) 
  print("result: " + result)
  result = coeff_to_log(1/5) 
  print("result: " + result)
  result = coeff_to_log(1/7) 
  print("result: " + result)
  result = coeff_to_log(1/9) 
  print("result: " + result)
  result = coeff_to_log(1/11) 
  print("result: " + result)
  result = coeff_to_log(1/13) 
  print("result: " + result)
  result = coeff_to_log(1/15) 
  print("result: " + result)
  result = coeff_to_log(1/17) 
  print("result: " + result)
