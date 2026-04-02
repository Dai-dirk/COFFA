"""
segSel.py - Python implementation of segment selector
重构自 segSel.v，用于判断输入值属于哪个分段
"""

import struct


def binary_to_float(binary_str):
    """
    将32位二进制字符串转换为IEEE 754单精度浮点数
    
    Args:
        binary_str: 32位二进制字符串
    
    Returns:
        float: 转换后的浮点数
    """
    # 确保是32位
    if len(binary_str) != 32:
        raise ValueError(f"Binary string must be 32 bits, got {len(binary_str)} bits")
    
    # 转换为整数
    int_value = int(binary_str, 2)
    
    # 打包为4字节
    bytes_value = int_value.to_bytes(4, byteorder='big')
    
    # 解包为浮点数
    return struct.unpack('>f', bytes_value)[0]


def binary_to_signed_int(binary_str):
    """
    将32位二进制字符串转换为有符号整数（补码表示）
    
    Args:
        binary_str: 32位二进制字符串
    
    Returns:
        int: 转换后的有符号整数
    """
    # 确保是32位
    if len(binary_str) != 32:
        raise ValueError(f"Binary string must be 32 bits, got {len(binary_str)} bits")
    
    # 转换为整数（无符号）
    unsigned_value = int(binary_str, 2)
    
    # 如果是负数（最高位为1），转换为有符号数
    if unsigned_value & 0x80000000:
        return unsigned_value - 0x100000000
    else:
        return unsigned_value


def segSel(x_in, is_fp, break_points_in):
    """
    分段选择函数
    
    Args:
        x_in: 输入的32位二进制字符串
        is_fp: 布尔值，True表示浮点数，False表示定点数
        break_points_in: 包含5个32位二进制字符串的列表，按从小到大顺序排列
    
    Returns:
        int: 分段索引，范围0-5
            - 0: x_in < break_points_in[0]
            - 1: break_points_in[0] <= x_in < break_points_in[1]
            - 2: break_points_in[1] <= x_in < break_points_in[2]
            - 3: break_points_in[2] <= x_in < break_points_in[3]
            - 4: break_points_in[3] <= x_in < break_points_in[4]
            - 5: x_in >= break_points_in[4]
    """
    # 确保 break_points_in 有5个元素
    if len(break_points_in) != 5:
        raise ValueError(f"break_points_in must have 5 elements, got {len(break_points_in)}")
    
    # 转换输入值
    if is_fp:
        x_value = binary_to_float(x_in)
        break_points = [binary_to_float(bp) for bp in break_points_in]
    else:
        x_value = binary_to_signed_int(x_in)
        break_points = [binary_to_signed_int(bp) for bp in break_points_in]
    
    # print("x_value:", x_value)
    # print("break_points:", break_points)
    # 判断属于哪个分段
    # 从大到小检查，找到第一个满足 x_value >= break_point 的分段
    for i in range(4, -1, -1):  # 从 index 4 到 0
        if x_value >= break_points[i]:
            return i + 1
    
    # 如果所有 break_points 都大于 x_value，返回 0
    return 0


def segSel7(x_in, is_fp, break_points_in):
    """
    分段选择函数
    
    Args:
        x_in: 输入的32位二进制字符串
        is_fp: 布尔值，True表示浮点数，False表示定点数
        break_points_in: 包含5个32位二进制字符串的列表，按从小到大顺序排列
    
    Returns:
        int: 分段索引，范围0-6
            - 0: x_in < break_points_in[0]
            - 1: break_points_in[0] <= x_in < break_points_in[1]
            - 2: break_points_in[1] <= x_in < break_points_in[2]
            - 3: break_points_in[2] <= x_in < break_points_in[3]
            - 4: break_points_in[3] <= x_in < break_points_in[4]
            - 5: break_points_in[4] <= x_in < break_points_in[5]
            - 6: x_in >= break_points_in[5]
    """
    # 确保 break_points_in 有5个元素
    if len(break_points_in) != 6:
        raise ValueError(f"break_points_in must have 6 elements, got {len(break_points_in)}")
    
    # 转换输入值
    if is_fp:
        x_value = binary_to_float(x_in)
        break_points = [binary_to_float(bp) for bp in break_points_in]
    else:
        x_value = binary_to_signed_int(x_in)
        break_points = [binary_to_signed_int(bp) for bp in break_points_in]
    
    # print("x_value:", x_value)
    # print("break_points:", break_points)
    # 判断属于哪个分段
    # 从大到小检查，找到第一个满足 x_value >= break_point 的分段
    for i in range(5, -1, -1):  # 从 index 4 到 0
        if x_value >= break_points[i]:
            return i + 1
    
    # 如果所有 break_points 都大于 x_value，返回 0
    return 0


# 测试函数
if __name__ == "__main__":
    # 测试用例：定点数
    print("测试定点数:")
    x_fixed = "00000000000000000000000000000010"  # 2 (十进制)
    break_points_fixed = [
        "00000000000000000000000000000001",  # 1
        "00000000000000000000000000000011",  # 3
        "00000000000000000000000000000111",  # 7
        "00000000000000000000000000001111",  # 15
        "00000000000000000000000000011111",  # 31
    ]
    index = segSel(x_fixed, False, break_points_fixed)
    print(f"x_in = {binary_to_signed_int(x_fixed)}, index = {index}")
    
    # 测试用例：浮点数
    print("\n测试浮点数:")
    # 1.0 的 IEEE 754 表示: 0x3F800000
    x_float = "00111111100000000000000000000000"  # 1.0
    break_points_float = [
        "00111111000000000000000000000000",  # 0.5
        "00111111100000000000000000000000",  # 1.0
        "00111111110000000000000000000000",  # 1.5
        "01000000000000000000000000000000",  # 2.0
        "01000000010000000000000000000000",  # 2.5
    ]
    index = segSel(x_float, True, break_points_float)
    print(f"x_in = {binary_to_float(x_float)}, index = {index}")

