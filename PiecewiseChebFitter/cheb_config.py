import json
import numpy as np
import re
import ast
from typing import List, Dict, Any, Callable, Union, Optional
import math


class ChebConfig:
    """
    配置类，用于从JSON文件读取配置，并支持通过字符串或表达式定义目标函数
    """

    # 预定义的函数映射
    FUNCTION_MAP = {
        "sinh": np.sinh,
        "cosh": np.cosh,
        "tanh": np.tanh,
        "arctanh": lambda x: 0.5 * np.log((1 + x) / (1 - x)),
        "sigmoid": lambda x: 1 / (1 + np.exp(-np.clip(x, -500, 500))),
        "swish": lambda x: x * (1 / (1 + np.exp(-x))),
        "relu": lambda x: np.maximum(0, x),
        "gelu": lambda x: 0.5 * x * (1 + np.tanh(np.sqrt(2 / np.pi) * (x + 0.044715 * x**3))),
        "sin": np.sin,
        "cos": np.cos,
        "tan": np.tan,
        "exp": np.exp,
        "log": np.log,
        "sqrt": np.sqrt,
        "reciprocal": lambda x: 1/x,
        "softsign": lambda x: x / (1 + np.abs(x)),
        "tshrink": lambda x: x - np.tanh(x),
        "mish": lambda x: x * np.tanh(np.log(1 + np.exp(x))),
        "exp_swish": lambda x: np.exp(-x) * x * (1 / (1 + np.exp(-np.clip(x, -500, 500)))),
        "log_x_c-x": lambda x: np.log(x / (100 - x)),
        "lnx-1_x": lambda x: np.log(x / (1 - x)),
        "log_1+exp_x": lambda x: np.log(1 + np.exp(x)),
        "arcsine": lambda x: np.arcsin(x),
        "arccosh": lambda x: np.log(x + np.sqrt(x**2 - 1)),
        "silu": lambda x: x * (1 / (1 + np.exp(-x))),  # SILU (Swish)
        "softplus": lambda x: np.log(1 + np.exp(x)),  # Softplus
    }

    def __init__(self, config_file: str = None):
        """
        初始化配置类

        参数:
            config_file: JSON配置文件路径，如果为None，则使用默认配置
        """
        self.name = "swish"
        self.alpha = 100000
        self.beta = 1
        # self.gamma = 2#@yuan: there is no need to consider the number of segment, as this is pre-determined by the XCore
        self.domain = [-32768, 32767]
        self.max_segments = 4
        self.max_order = 10
        self.max_order_sum = 10
        self.sample_count = 50000
        self.piecewise_threshold = 0.01
        self.Parity = 0 #@yuan: 0: no parity; 1: odd function; 2: even function
        self.coef_norm = False #@yuan: for fixed-point LNS
        self.bound = (-1.5, 1.5)#@yuan: the bound for coefficient
        # self.symmetric = False
        self.int_width = 5 #@yuan: for int_width = 0, it means the target format is float point
        self.frac_width = 5
        self.error_evaluation = "mse"
        self.order_distribute_method = "traversal"
        self.ga_config = {
            "population_size": 100,
            "generations": 20,
            "mutation_rate": 0.2,
            "crossover_rate": 0.7,
        }
        self.verbose = True

        # 如果提供了配置文件，则从文件加载配置
        if config_file:
            self.load_from_file(config_file)
        else:
            self.load_from_file("default.json")
        #calculate the bound based on the int_width
        if(self.int_width != 0):
            lowerbound = -(2 ** (self.int_width - 1)) - (2 ** (-self.frac_width))
            upperbound = (2 ** (self.int_width - 1)) - (2 ** (-self.frac_width))
            self.bound = (lowerbound, upperbound)
            # print(f"bound: {self.bound}")
        #@yuan: for the odd/even function has a symmetric domain, we only need to solve the half of the domain
        # if self.Parity in [1, 2]:
        #     if self.domain[0] == -self.domain[1]:
        #         a, b = 0, self.domain[1]
        #         self.domain = [a, b]
        #         self.max_segments = int(self.max_segments/2)
        #         self.symmetric = True
        #@yuan: update the max_order according to the parity of target function
        # if(self.Parity == 1):
        #      #@yuan: double the max_order since some terms become 0 when the target function is odd/even
        #      self.max_order = self.max_order*2 + 1
        #      self.max_order_sum = self.max_order * self.max_segments
        # elif(self.Parity == 2):
        #      self.max_order = self.max_order*2
        #      self.max_order_sum = self.max_order * self.max_segments



    def print_config(self):
        """打印配置成为一个大String"""
        return f"""
        函数名: {self.name}
        类型: {self.type}
        表达式: {self.expression}
        定义域: {self.domain}
        最大分段数: {self.max_segments}
        每段最大阶数: {self.max_order_per_segment}
        所有段阶数总和最大值: {self.max_all_order}
        奇偶性: {self.Parity}
        采样点数: {self.sample_count}
        分段阈值: {self.piecewise_threshold}
        误差评估方法: {self.error_evaluation}
        阶数分配方法: {self.order_distribute_method}
        目标函数: {self.target_function}
        目标函数权重: {self.alpha}, {self.beta}
        遗传算法配置: {self.ga_config}
        是否打印详细信息: {self.verbose}
        """

    def load_from_file(self, config_file: str) -> None:
        """
        从JSON文件加载配置

        参数:
            config_file: JSON配置文件路径
        """
        try:
            with open(config_file, "r", encoding="utf-8") as f:
                config_data = json.load(f)

            # 将配置数据直接赋值给类成员
            # print(f"config_data.items():{config_data.items()}")
            for key, value in config_data.items():
                if hasattr(self, key):
                    setattr(self, key, value)

            # 解析目标函数
            if self.name in self.FUNCTION_MAP:
                self.target_function = self.FUNCTION_MAP[self.name]
            else:
                raise ValueError(f"目标函数 {self.name} 未找到")
        except Exception as e:
            print(f"加载配置文件失败: {e}")

