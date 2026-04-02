import numpy as np
import math

def float_to_q6_16(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 6  # 包括符号位
    fractional_bits = 16
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value
def float_to_q8_24(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 8  # 包括符号位
    fractional_bits = 24
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value
def float_to_q32_0(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 32  # 包括符号位
    fractional_bits = 0
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value

def float_to_q32_1(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 31  # 包括符号位
    fractional_bits = 1
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value

def float_to_q16_16(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 16  # 包括符号位
    fractional_bits = 16
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value

def q6_16_to_float(fixed_point_value: int) -> float:
    # 定义Q6.16格式的参数
    integer_bits = 6  # 包括符号位
    fractional_bits = 16
    total_bits = integer_bits + fractional_bits  # 22 位

    # 检查是否为负数的补码表示
    if fixed_point_value & (1 << (total_bits - 1)):
        # 负数，转换为补码表示的负值
        fixed_point_value = fixed_point_value - (1 << total_bits)

    # 转换为浮点数
    scale_factor = 1 << fractional_bits
    return fixed_point_value / scale_factor
def float_to_q3_29(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 3  # 包括符号位
    fractional_bits = 29
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value
def float_to_q4_28(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 4  # 包括符号位
    fractional_bits = 28
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value
def float_to_q5_27(value: float) -> int:
    # 定义Q6.16格式的参数
    integer_bits = 5  # 包括符号位
    fractional_bits = 27
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value

def float_to_fixed(value: float, intWidth: int) -> int:
    # 定义Q6.16格式的参数
    integer_bits = intWidth  # 包括符号位
    fractional_bits = 32 - intWidth
    total_bits = integer_bits + fractional_bits  # 22 位

    # 计算缩放因子 (用于将浮点数转换为定点数)
    scale_factor = 1 << fractional_bits  # 即 2^16

    # 将浮点数转换为定点数 (未考虑符号位)
    fixed_point_value = int(round(value * scale_factor))

    # 检查是否超出 Q6.16 的表示范围
    max_value = (1 << (total_bits - 1)) - 1  # 最大值：2^21 - 1
    min_value = -(1 << (total_bits - 1))     # 最小值：-2^21

    if fixed_point_value > max_value or fixed_point_value < min_value:
        raise ValueError(f"Value {value} is out of range for Q6.16 representation.")

    # 如果是负数，则转换为补码表示
    if fixed_point_value < 0:
        fixed_point_value = (1 << total_bits) + fixed_point_value  # 计算补码

    # 确保输出是 22 位（使用掩码截断高位）
    fixed_point_value &= (1 << total_bits) - 1

    return fixed_point_value

# 示例用法
# value = 0.5
# fixed_point_value = float_to_q6_16(value)
# print(f"Decimal {value} in Q6.16 fixed-point format: {bin(fixed_point_value)} ({fixed_point_value})")

# value = -1.0
# fixed_point_value = float_to_q6_16(value)
# print(f"Decimal {value} in Q6.16 fixed-point format: {bin(fixed_point_value)} ({fixed_point_value})")

# # 测试负数
# value = -2.584962501
# fixed_point_value = float_to_q6_16(value)
# print(f"Decimal {value} in Q6.16 fixed-point format: {bin(fixed_point_value)} ({fixed_point_value})")

# # 将定点数转换回浮点数
# restored_value = q6_16_to_float(fixed_point_value)
# print(f"Restored float value: {restored_value}")



