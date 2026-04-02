import numpy as np
from scipy.optimize import minimize, brentq
from itertools import product
from cheb_solver import ChebSolver
from cheb_config import ChebConfig
from ga_order_optimizer import GAOrderOptimizer
from tabulate import tabulate
import matplotlib.pyplot as plt
import json
import struct
import math
import float2fix
from joblib import Parallel, delayed
import warnings
from scipy.optimize import OptimizeWarning



def float_to_fp32_hex(value):
    # 将浮点数转为单精度（FP32）的字节序列
    bytes_fp32 = struct.pack('>f', value)  # '>f' 表示大端序单精度浮点数
    # 将字节序列转为整数，再转为十六进制
    hex_fp32 = bytes_fp32.hex().upper()  # 示例输出：C0C76666
    return hex_fp32

def float_to_fixed_8_22(value: float) -> str:
    # 定义Q8.16格式的参数
    integer_bits = 9  # 包括符号位
    fractional_bits = 22
    total_bits = integer_bits + fractional_bits  # 32 位

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
    
    # 格式化为二进制字符串
    bin_int = bin(int_part)[2:].zfill(8)
    bin_frac = bin(frac_part)[2:].zfill(22)
    return f"{bin_int}{bin_frac}"

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
    
    # 格式化为二进制字符串
    bin_int = bin(int_part)[2:].zfill(8)
    bin_frac = bin(frac_part)[2:].zfill(16)
    return f"{bin_int}{bin_frac}"

def coeff_to_log(value):
    if value == 0:
        bin_log = '10000000000000000000000000000000'
    elif value > 0:
        log_value = math.log2(value)
        bin_log = f"00{float_to_fixed_8_22(log_value)}"
    else:
        log_value = math.log2(abs(value))
        bin_log = f"01{float_to_fixed_8_22(log_value)}"
    return bin_log

class PiecewiseChebFitter:
    def __init__(self, config_file="default.json"):
        """
        初始化优化器

        参数:
            config_file: 配置文件路径
        """
        self.config = ChebConfig(config_file)

    def fit(self):
        """
        拟合函数

        返回:
            cheb_coeffs: 各段的Chebyshev系数
            poly_coeffs: 各段的多项式系数
        """
        self.setup_samples()
        self.result = self.optimize()

        cheb_coeffs = []
        poly_coeffs = []
        segments = self._create_segments(self.result["breakpoints"])
        max_order = self.config.max_order
        # 计算预测值
        curvature_factor = 10.0  # 曲率影响因子 (1.0-3.0)
        # for (a, b), order in zip(segments, self.result["orders"]):
        parity_usage = self.result["parity"]
        sum_MAE = 0
        total_sample_count = 0
        for idx, ((a, b), order) in enumerate(zip(segments, self.result["orders"])):
            # x_segment = np.linspace(a, b, self.config.sample_count // len(segments))
            # y_segment = self.config.target_function(x_segment)
            # x_segment = self.simple_nonuniform_sampling(a, b, self.config.sample_count, curvature_factor)
            x_segment = self.find_samples((a, b))
            current_sample_count = len(x_segment)
            xmin = min(x_segment)
            xmax = max(x_segment)
            y_segment = self.config.target_function(x_segment)
            current_MAE = 0
            # if self.config.Parity in [1, 2]:
            if parity_usage[idx] == 1:
                parity_order = order * 2 - self.config.Parity % 2
                maxBound = max(abs(xmin), abs(xmax))
                # x_fill = self.simple_nonuniform_sampling(-maxBound, maxBound, self.config.sample_count, curvature_factor)
                # x_fill = self.find_samples((-maxBound, maxBound))
                x_fill = (-maxBound, maxBound)
                # y_fill = self.config.target_function(x_fill)
                cheb_coeff = ChebSolver.chebfit_normCoeff((-maxBound, maxBound), x_segment, y_segment, parity_order, self.config.Parity, self.config.bound, self.config.coef_norm, self.config.int_width, self.config.frac_width)
                poly_coeff = ChebSolver.cheb2poly_norm(x_fill, cheb_coeff)
                y_pred = ChebSolver.evaluate_cheby_norm(cheb_coeff, x_segment, np.min(x_segment), np.max(x_segment))
                current_MAE = np.sum(np.abs(y_segment - y_pred))
            else:
                cheb_coeff = ChebSolver.chebfit_normCoeff((xmin, xmax), x_segment, y_segment, order, 0, self.config.bound, self.config.coef_norm, self.config.int_width, self.config.frac_width)
                poly_coeff = ChebSolver.cheb2poly_norm(x_segment, cheb_coeff)
                y_pred = ChebSolver.evaluate_cheby_norm(cheb_coeff, x_segment, np.min(x_segment), np.max(x_segment))
                current_MAE = np.sum(np.abs(y_segment - y_pred))
            # if(self.config.coef_norm):
            #     cheb_coeff = ChebSolver.chebfit_normCoeff(x_segment, y_segment, order, self.config.Parity, self.config.bound)
            # else:
            #     cheb_coeff = ChebSolver.chebfit_normCoeff(x_segment, y_segment, order, self.config.Parity, [float('-inf'), float('inf')])
                # cheb_coeff = ChebSolver.chebfit(x_segment, y_segment, order)
            cheb_coeffs.append(cheb_coeff)
            # poly_coeff = ChebSolver.to_poly_coeffs(cheb_coeff)
            # poly_coeff = ChebSolver.cheb2poly_norm(x_segment, cheb_coeff)
            poly_coeffs.append(poly_coeff)
            total_sample_count += current_sample_count
            sum_MAE += current_MAE

        # print("cheb_coeffs: ")
        # print(cheb_coeffs)
        self.result["cheb_coeffs"] = cheb_coeffs
        self.result["poly_coeffs"] = poly_coeffs
        final_MAE = sum_MAE / total_sample_count
        self.result["MAE"] = final_MAE
        for idx, order in enumerate(self.result["orders"]):
            if parity_usage[idx] == 1:
                self.result["orders"][idx] = order * 2 - self.config.Parity % 2
                
        return cheb_coeffs, poly_coeffs

    def print_result(self, time):
        """
        打印结果
        """
        # segment_count = self.result['segment_count']
        # breakpoints = self.result['breakpoints']
        # orders = self.result['orders']
        # cheb_coeffs = self.result['cheb_coeffs']
        # poly_coeffs = self.result['poly_coeffs']
        # parity_usage = self.result["parity"]
        # if self.config.symmetric == True:
        #     self.result['order_sum'] *= 2
        #     self.result['segment_count'] *= 2
        #     self.config.max_segments *= 2
        #     domain_1 = self.config.domain[1]
        #     a, b = -domain_1, domain_1
        #     self.config.domain = [a, b]
        #     # print(self.result['breakpoints'], type(breakpoints))
        #     if len(breakpoints) > 0:
        #         negative_part = -np.flip(breakpoints)
        #         self.result['breakpoints'] = np.concatenate([negative_part, [0], breakpoints])
        #     else: 
        #         self.result['breakpoints'] = [0]
        #     self.result['orders'] = list(reversed(orders)) + orders
        #     self.result['parity'] = list(reversed(parity_usage)) + parity_usage
        #     # odd: f(x) = -f(-x); even: f(x) = f(-x)
        #     if self.config.Parity == 1:
        #         self.result['cheb_coeffs'] = [[-x for x in sublist] for sublist in reversed(cheb_coeffs)] + cheb_coeffs
        #         self.result['poly_coeffs'] = [[-x for x in sublist] for sublist in reversed(poly_coeffs)] + poly_coeffs
        #     else:
        #         self.result['cheb_coeffs'] = [[x for x in sublist] for sublist in reversed(cheb_coeffs)] + cheb_coeffs
        #         self.result['poly_coeffs'] = [[x for x in sublist] for sublist in reversed(poly_coeffs)] + poly_coeffs

        print(
            f"""
Number of segments: {self.result['segment_count']}
Breakpoints: {self.result['breakpoints']}
Orders per segment: {self.result['orders']}
Total order: {self.result['order_sum']}
Error: {self.result['error']}
MAE: {self.result['MAE']}
Objective value: {self.result['objective']}
cheb_coeffs:\n{tabulate(self.result['cheb_coeffs'], tablefmt="grid")}
poly_coeffs:\n{tabulate(self.result['poly_coeffs'], tablefmt="grid")}
time (second): {time}

        """
        )

    def save_result(self, time, file_name=None):
        """
        保存结果
        """
        if file_name is None:
            file_name = f"result/{self.config.name}/{self.config.name}_{self.config.max_segments}_{self.config.max_order}_{self.config.int_width}.{self.config.frac_width}.result"
        with open(file_name, "w") as f:
            f.write(f"Number of segments: {self.result['segment_count']}\n")
            f.write(f"Breakpoints: {self.result['breakpoints']}\n")
            f.write(f"Orders per segment: {self.result['orders']}\n")
            f.write(f"Total order: {self.result['order_sum']}\n")
            f.write(f"误差: {self.result['error']}\n")
            f.write(f"MAE: {self.result['MAE']}\n")
            f.write(f"目标函数值: {self.result['objective']}\n")
            f.write(f"time (second): {time}\n")
            f.write(
                f"cheb_coeffs:\n{tabulate(self.result['cheb_coeffs'], tablefmt='grid')}\n"
            )
            f.write(
                f"poly_coeffs:\n{tabulate(self.result['poly_coeffs'], tablefmt='grid')}\n"
            )
            parity_usage = self.result['parity']
            for coeff_idx, coeffs in enumerate(self.result['poly_coeffs']):
                f.write(f"Segment index:{coeff_idx}\n")
                hex_coeff = []
                log_coeff = []
                log_bin_coeff = []
                log_bin_coeff_str = []
                norm_coeff = []
                norm_log_bin_coeff = []
                norm_log_bin_coeff_str = []
                is_use_parity = parity_usage[coeff_idx]
                order_index = []
                coeffs_noconst = coeffs[1:]
                const_term = coeffs[0]
                hex_fp_const = float_to_fp32_hex(const_term)
                if self.config.int_width != 0:
                    fix_const = float2fix.float_to_fixed(const_term, self.config.int_width)
                else:
                    fix_const = 0
                parity = self.config.Parity
                for idx, coeff in enumerate(coeffs):
                    if coeff != 0:
                        if parity == 1:
                            order_index.append(idx)
                        else:
                            if idx != 0:
                               order_index.append(idx) 

                bin_6bit_list = []
                bin_30bit_groups = []
                for idx in order_index:
                    # 转换为6位二进制，高位补0
                    bin_str = bin(idx)[2:].zfill(6)
                    bin_6bit_list.append(bin_str)
                # 每5个二进制字符串拼成一组（30位）
                for i in range(0, len(bin_6bit_list), 5):
                    group = bin_6bit_list[i:i+5]
                    group = group[:: -1]
                    # 如果最后一组不足4个，用'000000'补足
                    while len(group) < 5:
                        number_zero = 5 - len(group)
                        group = ['0' * 6] * number_zero + group
                    bin_30bit_groups.append(''.join(group))
                if is_use_parity > 0:
                    if parity == 1:
                        coeffs_noconst = coeffs_noconst[0::2]
                    else:
                        coeffs_noconst = coeffs_noconst[1::2]

                for value in coeffs_noconst:
                    hex_coeff.append(float_to_fp32_hex(value))
                for value in coeffs_noconst:
                    if(value != 0):
                        log_coeff.append(math.log2(abs(value)))
                    else:
                        log_coeff.append(value)
                    log_bin_coeff.append(coeff_to_log(value))
                for i in range(0, len(log_bin_coeff), 5):
                    log_bin_coeff_str.append(''.join(log_bin_coeff[i:i+5]))
                if self.config.int_width != 0:
                    f.write(f"Constant term value:{const_term}; in fp format: {hex_fp_const}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_const)}\n")
                else:
                    f.write(f"Constant term value:{const_term}; in fp format: {hex_fp_const}\n")
                f.write(f"coeffs in decimal:{coeffs}; in hex:{hex_coeff}; in log:{log_coeff}; in bin:{log_bin_coeff}; no constant cascade:{log_bin_coeff_str}\n")
                f.write(f"polynimial order in decimal: {order_index}; in binary: {bin_30bit_groups}\n")
                # print(f"normalized_coeffs in decimal:{norm_coeff}; in bin:{norm_log_bin_coeff}; cascade:{norm_log_bin_coeff_str}")
                f.write("\n")

            for break_point_idx, break_point in enumerate(self.result['breakpoints']):
                if self.config.int_width != 0:
                    fix_break_point = float2fix.float_to_fixed(break_point, self.config.int_width)
                    f.write(f"Break Point index:{break_point_idx} and the value:{break_point}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_break_point)}\n")
                    f.write("\n")
                else:
                    hex_fp_break_point = float_to_fp32_hex(break_point)
                    f.write(f"Break Point index:{break_point_idx} and the value:{break_point}; in fp format: {hex_fp_break_point}\n")
                    f.write("\n")

    def plot_result(self, file_name=None):
        """
        绘制结果
        """
        if file_name is None:
            file_name = f"fig/{self.config.name}/{self.config.name}_{self.config.max_segments}_{self.config.max_order}_{self.config.int_width}.{self.config.frac_width}.png"
        config = self.config
        x = np.linspace(config.domain[0], config.domain[1], config.sample_count)
        segments = self._create_segments(self.result["breakpoints"])
        y_true = config.target_function(x)
        y_pred = np.zeros_like(x)
        parity_usage = self.result['parity']
        # print(f"~~~~~~~~~~~~~~~~~~")
        # print(f"segments:{segments}")
        # print(f"cheb_coeffs:{self.result}")
        # for (a, b), cheb_coeff in zip(segments, self.result["cheb_coeffs"]):
        for idx, ((a, b), cheb_coeff) in enumerate(zip(segments, self.result["cheb_coeffs"])):
            # print(f"seg: {idx}, [{a}, {b}]")
            mask = (x >= a) & (x <= b)
            #@yuan: in fact, the range of x has changed
            x_real = self.find_samples((a, b))
            xmax = max(x_real)
            xmin = min(x_real)
            # neg_part = False
            # if self.config.symmetric == True and a < 0:
            #     neg_part = True
            if parity_usage[idx] > 0:
                maxBound = max(abs(xmin), abs(xmax))
                y_pred[mask] = ChebSolver.evaluate_cheby_norm(cheb_coeff, x[mask], -maxBound, maxBound)
            else:
                y_pred[mask] = ChebSolver.evaluate_cheby_norm(cheb_coeff, x[mask], xmin, xmax)
            # print(f"y_pred[mask]:{y_pred[mask]}")

        plt.figure(figsize=(10, 6))
        plt.plot(x, y_true, label="True Function")
        plt.plot(x, y_pred, label="Chebyshev Fit", linestyle="--")
        for bp in self.result["breakpoints"]:
            plt.axvline(bp, color="red", linestyle=":", alpha=0.5)
        plt.legend()
        plt.title(f"Chebyshev Fit of {config.name} (ERROR={self.result['error']:.4f})")
        plt.savefig(file_name)
        # plt.show()
        plt.close()
        error = []
        error_rela = []
        for idx in range(0, len(x), 1):
            error.append(y_true[idx] - y_pred[idx])
            if(y_true[idx] == 0):
                error_rela.append(abs(y_true[idx] - y_pred[idx])/0.0000001)
            else:
                error_rela.append(abs(y_true[idx] - y_pred[idx])/y_true[idx])
        # error graph
        plt.figure(figsize=(10, 6))
        plt.plot(x, error, 'b-', label='AE')
        plt.scatter(x, error, c='red', s=20, alpha=0.5)
        
        plt.xlabel('Input Value')
        plt.ylabel('AE')
        plt.title('Absolute Error')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        
        # 保存和显示二选一
        plt.savefig(f"fig/cheby_{self.config.name}_{self.config.max_segments}_{self.config.max_order}_{self.config.max_order_sum}_{self.config.order_distribute_method}_AE.png", dpi=300)

        # error graph
        plt.figure(figsize=(10, 6))
        plt.plot(x, error_rela, 'b-', label='RAE')
        plt.scatter(x, error_rela, c='red', s=20, alpha=0.5)
        
        plt.xlabel('Input Value')
        plt.ylabel('RAE')
        plt.title('Relative Absolute Error')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        
        # 保存和显示二选一
        plt.savefig(f"fig/cheby_{self.config.name}_{self.config.max_segments}_{self.config.max_order}_{self.config.max_order_sum}_{self.config.order_distribute_method}_RAE.png", dpi=300)
        
    def evaluate_segment(self, segment, order, parity, max_iter=30000, eps_abs=1e-8, eps_rel=1e-8):
        """
        评估给定分段方案的拟合质量
        """
        #for each segment, if the target function is odd/even function
        #we need to evaluate twice
        # print(f"evaluate_segments [a, b] [{a}, {b}] order: {order}")
        #@yuan: fill the range to satisfy the parity
        a, b = segment
        config = self.config
        x_original = self.find_samples((a, b))
        xmax = max(x_original)
        xmin = min(x_original)
        y_original = config.target_function(x_original)
        # first, evaluate w/o parity
        coeff = ChebSolver.chebfit_normCoeff((xmin, xmax), x_original, y_original, order, 0, config.bound, config.coef_norm, config.int_width, config.frac_width, max_iter, eps_abs, eps_rel)
        y_pred = ChebSolver.evaluate_cheby_norm(coeff, x_original, np.min(x_original), np.max(x_original))
        print(y_pred.dtype)
        max_bias = np.max(np.abs(y_original - y_pred))
        mean_bias = np.mean(y_original - y_pred)
        parity_usage_flag = 0
        if config.error_evaluation == "mse":
            best_error = np.sum((y_original - y_pred) ** 2)
        else:
            best_error = np.sum(np.abs(y_original - y_pred))
        #second, evaluate with parity
        if parity in [1, 2] and order != 0:
            maxBound = max(abs(xmin), abs(xmax))
            # order_parity = order * 2 + parity % 2
            order_parity = order * 2 - parity % 2
            coeff_parity = ChebSolver.chebfit_normCoeff((-maxBound, maxBound), x_original, y_original, order_parity, parity, config.bound, config.coef_norm, config.int_width, config.frac_width, max_iter, eps_abs, eps_rel)
            y_pred_parity = ChebSolver.evaluate_cheby_norm(coeff_parity, x_original, xmin, xmax)
            if config.error_evaluation == "mse":
                if np.sum((y_original - y_pred_parity) ** 2) < best_error:
                    best_error = np.sum((y_original - y_pred_parity) ** 2)
                    parity_usage_flag = 1
                    max_bias = max(max_bias, np.max(np.abs(y_original - y_pred_parity)))
                    mean_bias = np.mean(y_original - y_pred_parity)
            else:                   
                if np.sum(np.abs(y_original - y_pred_parity)) < best_error:
                    best_error = np.sum(np.abs(y_original - y_pred_parity))
                    parity_usage_flag = 1
                    max_bias = max(max_bias, np.max(np.abs(y_original - y_pred_parity)))
                    mean_bias = np.mean(y_original - y_pred_parity)

        return best_error, max_bias, mean_bias, parity_usage_flag
    
    def evaluate_segments(self, segments, orders, parity,):
        """
        评估给定分段方案的拟合质量

        参数:
            segments: 分段区间列表 [(a0, a1), (a1, a2), ...]
            orders: 各段的阶数列表
            parity: the parity of the target function; 0: no; 1: odd; 2: even

        返回:
            error: 误差
        """
        total_error = 0.0
        error = 0.0
        segment_count = len(segments)

        # sample_count = self.config.sample_count

        parity_usage_flags = [0] * segment_count
        # print(f"segment_count: {segment_count}, parity_usage_flags: {parity_usage_flags}")
        best_error = 0.0
        max_bias = 0.0
        real_sample_count = 0
        for idx, ((a, b), order) in enumerate(zip(segments, orders)):
            seg_sample_count = len(self.find_samples((a, b)))
            best_error, max_bias, mean_bias, parity_usage_flag = self.evaluate_segment((a, b), order, parity)
            parity_usage_flags[idx] = parity_usage_flag
            total_error += best_error
            real_sample_count += seg_sample_count
            # print(f"total_mse:{total_mse}; total_mae:{total_mae}")
        total_error /= real_sample_count
        # print(f"total_mse:{total_mse}; total_mae:{total_mae}")
        # print(f"alfter solving parity_usage_flags: {parity_usage_flags}")
        if self.config.error_evaluation == "mse" or self.config.error_evaluation == "mae":
            if math.isnan(total_error):
                error = float("inf")
            else:
                error = total_error
        else:
            raise ValueError(f"未识别的误差评估方法: {self.config.error_evaluation}")
        return error, parity_usage_flags

    def objective_function(self, params, orders, parity):
        """
        目标函数：MSE

        参数:
            params: 优化参数 [breakpoint1, breakpoint2, ...]

        返回:
            误差值
        """
        config = self.config
        breakpoints = sorted(params)
        # print(f"one break points: {breakpoints}, orders: {orders}")
        breakpoints = np.clip(
            breakpoints,
            config.domain[0] + config.piecewise_threshold,
            config.domain[1] - config.piecewise_threshold,
        )
        if(len(breakpoints) > 1):#@yuan: in the case that the solver find the breakpoints is the the bounds
            if(len(np.unique(breakpoints)) < breakpoints.size):
                error = 100
                return error
        segments = self._create_segments(breakpoints)
        # print(f"segments: {segments}")
        error, _ = self.evaluate_segments(segments, orders, parity)
        # print(f"error: {error}")
        return error
    
    def optimize_breakpoints(self, orders_distribution):
        """
        优化断点位置
        """
        #to here, there is one specific order distribution strategy; e.g., [5, 6]
        #in the following code, we should determine the breakpoints
        print(f"orders_distribution: {orders_distribution}")
        config = self.config
        parity = config.Parity
        segment_count = len(orders_distribution)
        # print(f"segment_count: {segment_count}")
        # initial_breakpoints = np.linspace(
        #     config.domain[0], config.domain[1], segment_count + 1
        # )[1:-1]
        # initial_breakpoints = self.initialize_breakpoints(segment_count, method="curvature")
        initial_breakpoints = self.curvature_breakpoints_dict[segment_count]
        min_segment_length = 0.02 * (config.domain[1] - config.domain[0])
        bounds = [
            (config.domain[0], config.domain[1])
            for _ in range(len(initial_breakpoints))
        ]
        constraints = []
        if segment_count != 1:
            constraints += [ 
                {"type": "ineq", "fun": lambda x, i=i: x[i+1] - x[i] - min_segment_length * 5}
                for i in range(len(initial_breakpoints) - 1)
            ]
            constraints += [ #断点不能是边界
                 {"type": "ineq", "fun": lambda x: x[0] - config.domain[0] - min_segment_length}
            ]
            constraints += [#断点不能是边界
                {"type": "ineq", "fun": lambda x: config.domain[1] - x[-1] - min_segment_length}
            ]
        breakpoints = initial_breakpoints
        if segment_count != 1:
            candidates = []
            result_uniform =  minimize(
                self.objective_function,
                initial_breakpoints,
                args=(orders_distribution,parity),
                bounds=bounds,
                method='SLSQP',  
                constraints=constraints,
                options={"maxiter": 10000},
            )
            if result_uniform.success:
                # print(f"result.x: {result.x} size: {len(result.x)}")
                candidates.append(sorted(result_uniform.x))
            for _ in range(3):
                random_points = np.sort(
                    np.random.uniform(
                        config.domain[0] + min_segment_length,
                        config.domain[1] - min_segment_length,
                        len(initial_breakpoints)
                ))
                result_random = minimize(
                    self.objective_function,
                    random_points,
                    args=(orders_distribution,parity),
                    bounds=bounds,
                    method='SLSQP',  
                    constraints=constraints,
                    options={"maxiter": 10000},
                )
                if result_random.success:
                    candidates.append(sorted(result_random.x))

            if candidates:
                best_points = min(
                    candidates,
                    key=lambda x: self.objective_function(x, orders_distribution, parity)
                )
                breakpoints = np.sort(best_points)
            else:
                print(f"优化断点位置失败!")
                return {
                    "error": float("inf"),
                    "objective": float("inf"),
                    "segment_count": segment_count,
                    "order_sum": sum(orders_distribution),
                    "breakpoints": initial_breakpoints,
                    "orders": orders_distribution,
                    "parity": [0] * segment_count,
                    "success": False,
                }
        error, parity_usage = self.evaluate_segments(
            self._create_segments(breakpoints), orders_distribution, parity
        )
        objective = (
            self.config.alpha * error
            # + self.config.beta * sum(orders_distribution)
            + self.config.beta * self.order_penalty(orders_distribution)
        )
        print(f"优化断点位置success: {objective}; error: {error}; oders_sum: {sum(orders_distribution)}; seg_count: {segment_count}")
        return {
            "error": error,
            "objective": objective,
            "segment_count": segment_count,
            "order_sum": sum(orders_distribution),
            "breakpoints": breakpoints,
            "orders": orders_distribution,
            "parity" : parity_usage,
            "success": True,
        }
    
    def optimize_breakpoints_ed(self, orders_distribution, maxiter=10, divide_maxiter=100, var_convergence=1e-10, var_tolerance=1e-3, divide_tolerance=1.5e-5):
        """
        优化断点位置，尝试三种方法（等误差、曲率、均匀分段），选择最优结果。
        参考：Vershik, A.M., Malozemov, V.N. & Pevnyi, A.B. Best piecewise polynomial approximation. Sib Math J 16, 706–717 (1975). 
        https://doi.org/10.1007/BF00967102
        """
        print(f"orders_distribution: {orders_distribution}")
        config = self.config
        segment_count = len(orders_distribution)
        initial_breakpoints = self.curvature_breakpoints_dict[segment_count]
        print(f"segment_count: {segment_count}, initial_breakpoints: {initial_breakpoints}")
        
        candidates = []  # Store results from all three methods
        
        # Method 1: Equal Error (ED) Method
        try:
            last_var = 0
            nodes = [config.domain[0]] + initial_breakpoints.tolist() + [config.domain[1]]
            ls_max_bias = [0] * (segment_count)
            errors = [0] * (segment_count)
            parity_usages = [0] * (segment_count)
            
            for iter in range(maxiter):
                for i in range(1, segment_count):
                    nodes[i] = self.ed_devide((nodes[i-1], nodes[i+1]), orders_distribution[i-1], orders_distribution[i], config.Parity, divide_maxiter, divide_tolerance, safe_zone=0.05*(nodes[i+1] - nodes[i-1]))
                
                for idx in range(1, segment_count):
                    error, max_bias, mean_bias, parity_usage = self.evaluate_segment((nodes[idx-1], nodes[idx]), orders_distribution[idx-1], config.Parity)
                    ls_max_bias[idx-1] = max_bias
                    errors[idx-1] = error
                    parity_usages[idx-1] = parity_usage
                error, max_bias, mean_bias, parity_usage = self.evaluate_segment((nodes[segment_count-1], nodes[segment_count]), orders_distribution[segment_count-1], config.Parity)
                ls_max_bias[segment_count-1] = max_bias
                errors[segment_count-1] = error
                parity_usages[segment_count-1] = parity_usage
                
                # normalize the ls_max_bias
                norm_max_bias = np.array(ls_max_bias) / np.max(np.abs(ls_max_bias))
                var_norm_max_bias = np.var(norm_max_bias)
                
                # terminal check
                if var_norm_max_bias < var_tolerance or (np.abs(var_norm_max_bias - last_var) < var_convergence and iter > 1):
                    break
                else:
                    last_var = var_norm_max_bias
            
            ed_error = np.sum(errors) / config.sample_count
            ed_objective = config.alpha * ed_error + config.beta * self.order_penalty(orders_distribution)
            
            if np.var(norm_max_bias) > var_tolerance:
                print(f"  [ED Method] Inaccurate! error: {ed_error:.6e}; objective: {ed_objective:.6e}; var_norm_max_bias: {np.var(norm_max_bias):.6e}")
            else:
                print(f"  [ED Method] Success! error: {ed_error:.6e}; objective: {ed_objective:.6e}; var_norm_max_bias: {np.var(norm_max_bias):.6e}")
            
            candidates.append({
                "method": "ED",
                "error": ed_error,
                "objective": ed_objective,
                "breakpoints": np.array(nodes[1:-1]),
                "parity": parity_usages,
            })
        except Exception as e:
            print(f"  [ED Method] Failed: {e}")
        
        # Method 2: Curvature-based Method
        try:
            cur_breakpoints = self.curvature_breakpoints_dict[segment_count]
            cur_error, cur_parity_usage = self.evaluate_segments(
                self._create_segments(cur_breakpoints), orders_distribution, config.Parity
            )
            cur_objective = config.alpha * cur_error + config.beta * self.order_penalty(orders_distribution)
            print(f"  [Curvature Method] error: {cur_error:.6e}; objective: {cur_objective:.6e}")
            
            candidates.append({
                "method": "Curvature",
                "error": cur_error,
                "objective": cur_objective,
                "breakpoints": cur_breakpoints,
                "parity": cur_parity_usage,
            })
        except Exception as e:
            print(f"  [Curvature Method] Failed: {e}")
        
        # Method 3: Uniform Method
        try:
            uni_breakpoints = self.uniform_breakpoints_dict[segment_count]
            uni_error, uni_parity_usage = self.evaluate_segments(
                self._create_segments(uni_breakpoints), orders_distribution, config.Parity
            )
            uni_objective = config.alpha * uni_error + config.beta * self.order_penalty(orders_distribution)
            print(f"  [Uniform Method] error: {uni_error:.6e}; objective: {uni_objective:.6e}")
            
            candidates.append({
                "method": "Uniform",
                "error": uni_error,
                "objective": uni_objective,
                "breakpoints": uni_breakpoints,
                "parity": uni_parity_usage,
            })
        except Exception as e:
            print(f"  [Uniform Method] Failed: {e}")
        
        # Select the best candidate
        if not candidates:
            print(f"All Breakpoint Optimization Methods Failed!")
            return {
                "error": float("inf"),
                "objective": float("inf"),
                "segment_count": segment_count,
                "order_sum": sum(orders_distribution),
                "breakpoints": initial_breakpoints,
                "orders": orders_distribution,
                "parity": [0] * segment_count,
                "success": False,
            }
        
        best = min(candidates, key=lambda x: x["objective"])
        print(f"  [Best Method: {best['method']}] error: {best['error']:.6e}; objective: {best['objective']:.6e}; orders_sum: {sum(orders_distribution)}; seg_count: {segment_count}")
        
        return {
            "error": best["error"],
            "objective": best["objective"],
            "segment_count": segment_count,
            "order_sum": sum(orders_distribution),
            "breakpoints": best["breakpoints"],
            "orders": orders_distribution,
            "parity": best["parity"],
            "success": True,
        }


    def ed_devide(self, segment, order_left, order_right, parity, maxiter, tolerance, safe_zone=0.05):
        """
        等误差划分
        """
        def check_invalid(bias, isleft = True):
            if not np.isfinite(bias):
                # print(f"警告：检测到非有限值 - bias: {bias}")
                # # 返回一个大的正值，指示该分割点不合适
                return 1e10 if isleft else -1e10
        def error_diff(node):
            a, b = segment
            assert node > a and node < b, f"node is out of range. node: {node}, segment: {segment}"
            assert len(self.find_samples((a, node))) > 0 and len(self.find_samples((node, b))) > 0, "not enough samples in the segment."
            left_error, left_max_bias, left_mean_bias, left_parity_usage = self.evaluate_segment((a, node), order_left, parity, 35000, 1e-6, 1e-6)
            if not np.isfinite(left_max_bias):
                return float("inf")
            right_error, right_max_bias, right_mean_bias, right_parity_usage = self.evaluate_segment((node, b), order_right, parity, 35000, 1e-6, 1e-6)
            if not np.isfinite(right_max_bias):
                return float("-inf")
            return left_max_bias - right_max_bias
        
        assert segment[0] + safe_zone < segment[1] - safe_zone, f"segment is invalid: {segment}"
        # use brent method to find the root
        try:
            root, result = brentq(error_diff, segment[0]+safe_zone, segment[1]-safe_zone, xtol=tolerance, maxiter=maxiter, full_output=True)
        except Exception as e:
            raise ValueError("ED Error: Cannot find the ED point. Terminate the iteration.")
            # if "different signs" in str(e):
            #     # print(f"Cannot find the ED point. Terminate the iteration.")
            #     raise ValueError("ED Error: Cannot find the ED point. Terminate the iteration.")
            # else:
            #     print(f"ED Error: brentq error: {e}, return the middle point: {(segment[0] + segment[1]) / 2}")
            #     return (segment[0] + segment[1]) / 2

        if result.converged:
            # print(f"brentq converged: {result.converged}, root: {root}, result: {result}")
            return root
        else:
            print(f"brentq failed, return the middle point: {result.converged}, root: {root}, result: {result}")
            return (segment[0] + segment[1]) / 2

    def optimize(self):
        """
        优化分段方案

        参数:
            alpha, beta: 目标函数权重

        返回:
            优化结果: (breakpoints, orders, segments, mse, mae, order_sum)
        """
        config = self.config
        best_result = {
            "objective": float("inf"),
            "error": float("inf"),
            "segment_count": 0,
            "order_sum": 0,
            "breakpoints": [],
            "orders": [],
            "parity": [],
            "success": False,
        }
        # 无断点的情况
        segment = [(config.domain[0], config.domain[1])]
        for order in range(0, config.max_order + 1):
            # the result when we don't use parity
            try:
                error, parity_usage = self.evaluate_segments(segment, [order], self.config.Parity)
            except Exception as e:
                print(f"Error when no breakpoints: {e}, order: {order}")
                continue
            # print(f"order: {order}, error: {error}")
            objective = config.alpha * error + config.beta * math.ceil(order / 4)
            if objective < best_result["objective"]:
                best_result = {
                    "objective": objective,
                    "error": error,
                    "segment_count": 1,
                    "order_sum": order,
                    "breakpoints": [],
                    "orders": [order],
                    "parity": parity_usage,
                    "success": True,
                }
        print(
            f"no breakpoints:\norder: {best_result['orders']}, objective: {best_result['objective']}, error: {best_result['error']}, parity_usage: {best_result['parity']}\n"
        )
        # exit(0)
        # 遍历分段数
        base_generation = config.ga_config["generations"]
        for segment_count in range(2, config.max_segments + 1):
            if config.order_distribute_method == "ga":
                #@yang: IMPORTANT: there is some bug in chebfit when using multiprocessing,
                # so we just simply disable the multiprocessing when coef_norm is true and parity is 0, cause it will be fast anyway.
                # mp = config.ga_config["multiprocessing"] and not (config.coef_norm and config.Parity == 0)
                # print(f"multiprocessing: {mp}")
                increment_generation = int(base_generation * 1.2 ** (segment_count - 2))
                ga_optimizer = GAOrderOptimizer(
                    # self.optimize_breakpoints,
                    self.optimize_breakpoints_ed,
                    n_segments=2,
                    initial_orders_dict=self.initial_orders_dict,
                    use_initial_orders_ratio=0.9,
                    max_degree=config.max_order,
                    max_total_degree=config.max_order_sum,
                    population_size=config.ga_config["population_size"],
                    lambda_=config.ga_config["lambda_"],
                    num_generations=increment_generation,
                    cxpb=config.ga_config["crossover_rate"],
                    mutpb=config.ga_config["mutation_rate"],
                    parity=config.Parity,
                    # multiprocessing=mp,
                    pool_size=config.ga_config["pool_size"],
                    verbose=config.verbose,
                )
            print(f"current segment_count: {segment_count}")
            if config.order_distribute_method == "traversal":
                # 遍历每个分段的阶数
                for orders in product(
                    range(0, config.max_order + 1), repeat=segment_count
                ):
                    order_sum = sum(orders)
                    if order_sum <= config.max_order_sum:
                        current_result = self.optimize_breakpoints(orders)
                        if config.verbose:
                            print(
                                f"current orders: {orders},\t\t objective: {current_result['objective']}"
                            )
                        if current_result["objective"] < best_result["objective"]:
                            best_result = current_result
            elif config.order_distribute_method == "ga":
                ga_optimizer.set_config({"n_segments": segment_count})
                current_result, logbook = ga_optimizer.optimize()
                if config.verbose:
                    print(
                        f"current objective: {current_result['objective']}"
                    )
                # print("!@#$")
                # best_result = current_result
                if current_result["objective"] < best_result["objective"]:
                    best_result = current_result


            print(
                f"segment_count: {segment_count}, current_objective: {best_result['objective']}"
            )
            print(
                f"error: {best_result['error']}, order_sum: {best_result['order_sum']}, orders: {best_result['orders']}, parity_usage: {best_result['parity']}"
            )
            print(f"breakpoints: {best_result['breakpoints']}\n")

        if best_result["success"]:
            self.result = best_result
            return best_result
        else:
            raise ValueError("优化失败")

    def _create_segments(self, breakpoints):
        """创建分段区间列表"""
        if len(breakpoints) == 0:
            return [(self.config.domain[0], self.config.domain[1])]
        else:
            breakpoints = sorted(breakpoints)
            segments = []
            segments.append((self.config.domain[0], breakpoints[0]))
            for i in range(len(breakpoints) - 1):
                segments.append((breakpoints[i], breakpoints[i + 1]))
            segments.append((breakpoints[-1], self.config.domain[1]))
            return segments 
    # def simple_nonuniform_sampling(self, a, b, sample_count, curvature_factor=1.5):
    #     """
    #     简单非均匀采样方法

    #     参数:
    #         a, b: 区间端点
    #         sample_count: 总采样点数
    #         curvature_factor: 曲率影响因子 (1.0-3.0)
    #         min_samples: 最小采样点数

    #     返回:
    #         x: 非均匀采样点数组
    #     """
    #     # 1. 基础均匀采样（使用部分点数）
    #     base_points = np.linspace(a, b, sample_count)
    #     available_extra_point = int(sample_count * 0.02)

    #     # 2. 计算函数值
    #     y = self.config.target_function(base_points)
    #     # print(f"base_points: {base_points}")
    #     # 3. 计算曲率（简单二阶差分近似）
    #     dx = base_points[1] - base_points[0]
    #     if dx == 0:
    #         return base_points
    #     print(f"dx: {dx}")
    #     dy = np.gradient(y, dx)
    #     d2y = np.gradient(dy, dx)
    #     curvature = np.abs(d2y)

    #     # 4. 归一化曲率并加权
    #     if np.max(curvature) > 0:
    #         curvature /= np.max(curvature)
    #     # print(f"curvature: {curvature}")
    #     weights = curvature * curvature_factor + 1.0  # 确保最小权重为1

    #     # 5. 计算每个区间应添加的点数
    #     interval_weights = weights[:-1] + weights[1:]  # 区间权重为两端点平均值
    #     max_weight = max(interval_weights)
    #     total_weight = np.sum(interval_weights)
    #     insert_factor = 2.5*total_weight/max_weight #@yuan: max_weight/total_weight * k = 2.5 (average insert 2.5 nodes)
    #     # top_wights = np.sort(np.partition(interval_weights, -extra_point)[-extra_point:])
    #     # print(f"max interval_weights: {max_weight}")
    #     # print(f"interval_weights: {interval_weights}")
    #     # print(f"insert_factor: {insert_factor}")
    #     # print(f"total_weight: {total_weight}")

    #     # 6. 分配剩余采样点
    #     additional_points = []
    #     for i in range(len(interval_weights)):
    #         # 计算本区间应添加点数
    #         n_add = int(round(insert_factor * interval_weights[i] / total_weight))
    #         # print(f"n_add: {n_add}")
    #         if n_add > 0:
    #             # 在区间内添加点
    #             start = base_points[i]
    #             end = base_points[i+1]
    #             new_points = np.linspace(start, end, n_add + 2)[1:-1]  # 不包含端点
    #             additional_points.extend(new_points)
    #             available_extra_point -= n_add
    #         if available_extra_point <= 0:
    #             break

    #     # 7. 合并所有点
    #     all_points = np.concatenate([base_points, additional_points])

    #     # 8. 如果点数超过要求，均匀缩减
    #     # if len(all_points) > sample_count + extra_point:
    #     #     indices = np.linspace(0, len(all_points)-1, sample_count).astype(int)
    #     #     return np.sort(all_points[indices])

    #     return np.sort(all_points)



    def simple_nonuniform_sampling(self, a, b, sample_count, curvature_factor=1.5):
        """
        简单非均匀采样方法
        参数:
            func: 目标函数
            a, b: 区间端点
            sample_count: 总采样点数
            curvature_factor: 曲率影响因子 (1.0-3.0)
        返回:
            x: 非均匀采样点数组
        """
        # 1. 基础均匀采样（使用部分点数）
        base_points = np.linspace(a, b, sample_count)
        available_extra_point = int(sample_count * 0.1)

        # 2. 计算函数值
        y = self.config.target_function(base_points)

        # 3. 计算曲率（简单二阶差分近似）
        dx = base_points[1] - base_points[0]
        if dx == 0:
            return base_points

        dy = np.gradient(y, dx)
        d2y = np.gradient(dy, dx)
        curvature = np.abs(d2y) / (1 + dy**2)**1.5

        # 4. 归一化曲率并加权
        if np.max(curvature) > 0:
            curvature /= np.max(curvature)

        weights = curvature * curvature_factor + 1.0  # 确保最小权重为1

        # 5. 计算每个区间应添加的点数
        interval_weights = weights[:-1] + weights[1:]  # 区间权重为两端点平均值
        max_weight = max(interval_weights)
        insert_factor = 3/max_weight

        # 6. 计算每个区间理论上应添加的点数
        n_add_list = []
        for i in range(len(interval_weights)):
            n_add = int(round(insert_factor * interval_weights[i]))
            n_add_list.append(n_add)

        # 7. 按权重排序并分配额外的点数
        additional_points = []

        # 获取按权重从大到小排序的区间索引
        sorted_indices = np.argsort(-interval_weights)  # 负号表示从大到小排序

        for idx in sorted_indices:
            # 如果还有额外的点数可用
            if available_extra_point > 0:
                # 计算实际可插入的点数（不超过理论值和剩余可用点数）
                actual_n_add = min(n_add_list[idx], available_extra_point)

                if actual_n_add > 0:
                    # 在区间内添加点
                    start = base_points[idx]
                    end = base_points[idx+1]
                    new_points = np.linspace(start, end, actual_n_add + 2)[1:-1]  # 不包含端点
                    additional_points.extend(new_points)
                    available_extra_point -= actual_n_add

        # 8. 合并所有点
        all_points = np.concatenate([base_points, additional_points])
        return np.sort(all_points)
    
    def setup_samples(self):
        """
        设置采样点
        """
        self.all_points = self.simple_nonuniform_sampling(self.config.domain[0], self.config.domain[1], self.config.sample_count, curvature_factor=30)
        self.initial_orders_dict = self.setup_initial_orders(self.config.sample_count)
        self.curvature_breakpoints_dict = {}
        self.uniform_breakpoints_dict = {}
        for segment_count in range(2, self.config.max_segments + 1):
            self.curvature_breakpoints_dict[segment_count] = self.initialize_breakpoints(segment_count, method="curvature")
            self.uniform_breakpoints_dict[segment_count] = self.initialize_breakpoints(segment_count, method="uniform")
    
    def find_samples(self, segment):
        """
        根据segment的端点，从all_points中找到对应的采样点。注意all_points是排序后的。
        
        参数:
            segment: (a, b) 形式的区间元组，表示分段的起始和结束点
            all_points: 已排序的采样点数组
            
        返回:
            numpy.ndarray: 属于该segment的采样点数组
        """
        a, b = segment
        mask = (self.all_points >= a) & (self.all_points <= b)
        segment_points = self.all_points[mask]
        
        return segment_points
    

    def print_coeff_breakpoints(self):
        parity_usage = self.result['parity']
        for coeff_idx, coeffs in enumerate(self.result['poly_coeffs']):
            print(f"Segment index:{coeff_idx}")
            hex_coeff = []
            log_coeff = []
            log_bin_coeff = []
            log_bin_coeff_str = []
            norm_coeff = []
            norm_log_bin_coeff = []
            norm_log_bin_coeff_str = []
            is_use_parity = parity_usage[coeff_idx]
            order_index = []
            coeffs_noconst = coeffs[1:]
            const_term = coeffs[0]
            hex_fp_const = float_to_fp32_hex(const_term)
            if self.config.int_width != 0:
                fix_const = float2fix.float_to_fixed(const_term, self.config.int_width)
            else:
                fix_const = 0
            parity = self.config.Parity
            for idx, coeff in enumerate(coeffs):
                if coeff != 0:
                    if parity == 1:
                        order_index.append(idx)
                    else:
                        if idx != 0:
                           order_index.append(idx) 

            bin_6bit_list = []
            bin_30bit_groups = []
            for idx in order_index:
                # 转换为6位二进制，高位补0
                bin_str = bin(idx)[2:].zfill(6)
                bin_6bit_list.append(bin_str)
            # 每5个二进制字符串拼成一组（30位）
            for i in range(0, len(bin_6bit_list), 5):
                group = bin_6bit_list[i:i+5]
                group = group[:: -1]
                # 如果最后一组不足4个，用'000000'补足
                while len(group) < 5:
                    number_zero = 5 - len(group)
                    group = ['0' * 6] * number_zero + group
                bin_30bit_groups.append(''.join(group))
            if is_use_parity > 0:
                if parity == 1:
                    coeffs_noconst = coeffs_noconst[0::2]
                else:
                    coeffs_noconst = coeffs_noconst[1::2]
            
            for value in coeffs_noconst:
                hex_coeff.append(float_to_fp32_hex(value))
            for value in coeffs_noconst:
                if(value != 0):
                    log_coeff.append(math.log2(abs(value)))
                else:
                    log_coeff.append(value)
                log_bin_coeff.append(coeff_to_log(value))
            for i in range(0, len(log_bin_coeff), 5):
                log_bin_coeff_str.append(''.join(log_bin_coeff[i:i+5]))
            for value in norm_coeff:
                norm_log_bin_coeff.append(coeff_to_log(value))
            if self.config.int_width != 0:
                print(f"Constant term value:{const_term}; in fp format: {hex_fp_const}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_const)}")
            else:
                print(f"Constant term value:{const_term}; in fp format: {hex_fp_const}")
            print(f"coeffs in decimal:{coeffs}; in hex:{hex_coeff}; in log:{log_coeff}; in bin:{log_bin_coeff}; no constant cascade:{log_bin_coeff_str}")
            print(f"polynimial order in decimal: {order_index}; in binary: {bin_30bit_groups}")
            # print(f"normalized_coeffs in decimal:{norm_coeff}; in bin:{norm_log_bin_coeff}; cascade:{norm_log_bin_coeff_str}")
            print("\n")

        for break_point_idx, break_point in enumerate(self.result['breakpoints']):
            if self.config.int_width != 0:
                fix_break_point = float2fix.float_to_fixed(break_point, self.config.int_width)
                print(f"Break Point index:{break_point_idx} and the value:{break_point}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_break_point)}")
                print("\n")
            else:
                hex_fp_break_point = float_to_fp32_hex(break_point)
                print(f"Break Point index:{break_point_idx} and the value:{break_point}; in fp format: {hex_fp_break_point}")
                print("\n")

        return
    
    # def print_coeff_breakpoints(self):
    #     parity_usage = self.result['parity']
    #     for coeff_idx, coeffs in enumerate(self.result['poly_coeffs']):
    #         print(f"Segment index:{coeff_idx}")
    #         hex_coeff = []
    #         log_coeff = []
    #         log_bin_coeff = []
    #         log_bin_coeff_str = []
    #         norm_coeff = []
    #         norm_log_bin_coeff = []
    #         norm_log_bin_coeff_str = []
    #         is_use_parity = parity_usage[coeff_idx]
    #         order_index = []
    #         coeffs_noconst = coeffs[1:]
    #         const_term = coeffs[0]
    #         hex_fp_const = float_to_fp32_hex(const_term)
    #         fix_const = float2fix.float_to_fixed(const_term, self.config.int_width)
    #         parity = self.config.Parity
    #         for idx, coeff in enumerate(coeffs):
    #             if coeff != 0:
    #                 if parity == 1:
    #                     order_index.append(idx)
    #                 else:
    #                     if idx != 0:
    #                        order_index.append(idx) 

    #         bin_6bit_list = []
    #         bin_24bit_groups = []
    #         for idx in order_index:
    #             # 转换为6位二进制，高位补0
    #             bin_str = bin(idx)[2:].zfill(6)
    #             bin_6bit_list.append(bin_str)
    #         # 每4个二进制字符串拼成一组（24位）
    #         for i in range(0, len(bin_6bit_list), 4):
    #             group = bin_6bit_list[i:i+4]
    #             group = group[:: -1]
    #             # 如果最后一组不足4个，用'000000'补足
    #             while len(group) < 4:
    #                 number_zero = 4 - len(group)
    #                 group = ['0' * 6] * number_zero + group
    #             bin_24bit_groups.append(''.join(group))
    #         if is_use_parity > 0:
    #             if parity == 1:
    #                 coeffs_noconst = coeffs_noconst[0::2]
    #             else:
    #                 coeffs_noconst = coeffs_noconst[1::2]
            
    #         for value in coeffs_noconst:
    #             hex_coeff.append(float_to_fp32_hex(value))
    #         for value in coeffs_noconst:
    #             if(value != 0):
    #                 log_coeff.append(math.log2(abs(value)))
    #             else:
    #                 log_coeff.append(value)
    #             log_bin_coeff.append(coeff_to_log(value))
    #         for i in range(0, len(log_bin_coeff), 4):
    #             log_bin_coeff_str.append(''.join(log_bin_coeff[i:i+4]))
    #         for value in norm_coeff:
    #             norm_log_bin_coeff.append(coeff_to_log(value))
    #         for i in range(0, len(norm_log_bin_coeff), 4):
    #             norm_log_bin_coeff_str.append(''.join(norm_log_bin_coeff[i:i+4]))
    #         print(f"Constant term value:{const_term}; in fp format: {hex_fp_const}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_const)}")
    #         print(f"coeffs in decimal:{coeffs}; in hex:{hex_coeff}; in log:{log_coeff}; in bin:{log_bin_coeff}; no constant cascade:{log_bin_coeff_str}")
    #         print(f"polynimial order in decimal: {order_index}; in binary: {bin_24bit_groups}")
    #         # print(f"normalized_coeffs in decimal:{norm_coeff}; in bin:{norm_log_bin_coeff}; cascade:{norm_log_bin_coeff_str}")
    #         print("\n")

    #     for break_point_idx, break_point in enumerate(self.result['breakpoints']):
    #         hex_fp_break_point = float_to_fp32_hex(break_point)
    #         fix_break_point = float2fix.float_to_fixed(break_point, self.config.int_width)
    #         print(f"Break Point index:{break_point_idx} and the value:{break_point}; in fp format: {hex_fp_break_point}; in fixed_format_{self.config.int_width}_{32 - self.config.int_width}: {hex(fix_break_point)}")
    #         print("\n")

    #     return
    
    def order_penalty(self, orders_distribution):
        penalty = 0
        for order in orders_distribution:
            XCore_count = math.ceil(order / 4)
            penalty += XCore_count

        return penalty

    def initialize_breakpoints(self, segment_count, method="curvature", sample_count=10000):
        """
        基于曲率的方法确定初始断点，让每一段的曲率累计值相等
        
        参数:
            domain: 拟合域 [a, b]
            segment_count: 分段数
            config: 配置对象，包含目标函数
        
        返回:
            breakpoints: 断点列表，包含起点和终点
        """
        config = self.config
        a, b = config.domain[0], config.domain[1]

        if method == "curvature":
        
            # 生成密集采样点用于曲率计算
            x = np.linspace(a, b, sample_count)
            y = config.target_function(x)
            
            # 数值计算一阶导数
            dx = x[1] - x[0]
            dy_dx = np.gradient(y, dx)
            
            # 数值计算二阶导数
            d2y_dx2 = np.gradient(dy_dx, dx)
            
            # 计算曲率 κ = |y''| / (1 + (y')^2)^(3/2)
            # 为了数值稳定性，我们添加一个小的正则化项
            eps = 1e-10
            curvature = np.abs(d2y_dx2) / (1 + dy_dx**2 + eps)**(3/2)
            
            # 处理可能的无穷大或NaN值
            curvature = np.nan_to_num(curvature, nan=0.0, posinf=1e6, neginf=0.0)
            
            # 计算曲率的累积积分（使用梯形法则）
            curvature_cumsum = np.cumsum(curvature) * dx
            total_curvature = curvature_cumsum[-1]
            
            # 如果总曲率为0（比如线性函数），回退到等距分布
            if total_curvature < eps:
                return np.linspace(a, b, segment_count + 1)
            
            # 计算每段应有的曲率累积值
            target_curvature_per_segment = total_curvature / segment_count
            
            # 找到断点位置
            breakpoints = []  # 起始点
            
            for i in range(1, segment_count):
                target_cumulative = i * target_curvature_per_segment
                # 找到最接近目标累积曲率值的位置
                idx = np.argmin(np.abs(curvature_cumsum - target_cumulative))
                breakpoint = x[idx]
                breakpoints.append(breakpoint)
            
            # 确保断点单调递增且在定义域内
            breakpoints = np.array(breakpoints)
            breakpoints = np.clip(breakpoints, a, b)
            breakpoints = np.unique(breakpoints)  # 去除重复点
            
            # 如果去重后断点数量不够，用等距补充
            if len(breakpoints) < segment_count - 1:
                return np.linspace(a, b, segment_count + 1)
        
        elif method == "uniform":
            return np.linspace(a, b, segment_count + 1)[1:-1]
        elif method.startswith("random"):
            random_seed = int(method.split("_")[1])
            np.random.seed(random_seed)
            return np.sort(np.random.uniform(a, b, segment_count - 1))

        return breakpoints

    def setup_initial_orders(self, sample_count = 10000):
        """
        根据曲率，设置初始阶数。方法是在定义域上平均取sample_count个点，然后计算曲率。
        随后对于每个2-maxSegment的分段数，将定义域平均分成若干段。计算曲率和，以此分配阶数。最后返回一个dict，key为分段数，value为阶数。
        """
        a, b = self.config.domain[0], self.config.domain[1]
        x = np.linspace(a, b, sample_count)
        y = self.config.target_function(x)
        dx = x[1] - x[0]
        dy_dx = np.gradient(y, dx)
        d2y_dx2 = np.gradient(dy_dx, dx)
        curvature = np.abs(d2y_dx2) / (1 + dy_dx**2 + 1e-10)**(3/2)
        curvature = np.nan_to_num(curvature, nan=0.0, posinf=1e6, neginf=0.0)
        # 计算曲率的累积积分（使用梯形法则）
        curvature_cumsum = np.cumsum(curvature) * dx

        # 根据累计积分寻找初始阶数分配
        initial_orders = {}
        for segment_count in range(2, self.config.max_segments + 1):
            step = sample_count // segment_count
            orders = []
            for i in range(segment_count):
                start = i * step
                end = start + step - 1
                orders.append(curvature_cumsum[end] - curvature_cumsum[start])
            # normalize the initial orders
            orders = [int(round(order / sum(orders) * self.config.max_order)) for order in orders]
            # check the sum of initial orders
            if sum(orders) > self.config.max_order_sum:
                orders = [int(round(order / sum(orders) * self.config.max_order)) for order in orders]
            # check the sum of initial orders
            if sum(orders) < self.config.max_order_sum:
                orders = [int(round(order / sum(orders) * self.config.max_order)) for order in orders]
            initial_orders[segment_count] = orders
        return initial_orders