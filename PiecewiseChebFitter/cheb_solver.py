import numpy as np
from numpy.polynomial.chebyshev import chebfit, chebval
from scipy.optimize import minimize
from numpy.polynomial.chebyshev import chebvander, cheb2poly
import cvxpy as cp
from scipy import sparse
import math


class ChebSolver:

    _poly_coeffs_table = []

    @classmethod
    def _init_coeff_table(cls):
        """惰性初始化方法"""
        if len(cls._poly_coeffs_table) != 0:
            return

        cls._poly_coeffs_table = []
        # 初始化前20项存储空间
        for _ in range(20):
            cls._poly_coeffs_table.append(np.zeros(20))

        # 初始化前两项
        cls._poly_coeffs_table[0][0] = 1  # T0 = 1
        cls._poly_coeffs_table[1][1] = 1  # T1 = x

        # 递推计算后续项
        for i in range(2, 20):
            shift = np.zeros(20)
            shift[1:] = cls._poly_coeffs_table[i-1][:-1]
            cls._poly_coeffs_table[i] = 2 * shift - cls._poly_coeffs_table[i-2] # 2x* Tn-1 - Tn-2

    @classmethod
    def to_poly_coeffs(cls, cheby_coeffs):
        if len(cheby_coeffs) > 20:
            raise ValueError("系数超过20项")

        # 惰性初始化检查
        if len(cls._poly_coeffs_table) == 0:
            cls._init_coeff_table()

        n = len(cheby_coeffs)
        poly_coeffs = np.zeros(n)
        # print(f"cheby_coeffs size: {len(cheby_coeffs)}")
        # print(f"cls._poly_coeffs_table size: {len(cls._poly_coeffs_table)}")
        for i in range(n):
            poly_coeffs += (
                np.array(cheby_coeffs[i]) * np.array(cls._poly_coeffs_table[i])[:n]
            )
            
        return poly_coeffs

    @staticmethod
    def evaluate_cheby(cheby_coeffs, x):
        """计算切比雪夫多项式在点x处的值"""
        return chebval(x, cheby_coeffs)
    
    @staticmethod
    def evaluate_cheby_norm(cheby_coeffs, x, x_min, x_max):
        """计算切比雪夫多项式在点x处的值"""
        x_norm = 2 * (x - x_min)/(x_max - x_min) - 1
        # print(f"x_norm: {x_norm}")
        # if is_neg_part:
        #     x_norm = - x_norm
        if np.any(x_norm < -1) or np.any(x_norm > 1):
            print(f"warning：{np.sum(x_norm < -1)} less than 1,{np.sum(x_norm > 1)}bigger than one")
        return chebval(x_norm, cheby_coeffs)

    @staticmethod
    def evaluate_poly(coeffes, x):
        """计算切比雪夫多项式在点集x处的值"""
        return np.polyval(coeffes, x)

    @staticmethod
    def chebfit(x, y, order):
        """使用切比雪夫多项式拟合数据点(x, y)，返回系数"""
        return chebfit(x, y, order)
    
    @staticmethod
    def chebfit_normCoeff(x_bound, x, y, order, parity, bounds=(-2, 2), hasBound=False, int_width = 5, frac_width = 5, max_iter=30000, eps_abs=1e-8, eps_rel=1e-8):
        """
        带标准多项式系数约束的切比雪夫拟合（改进版）
        1. 将x归一化到[-1,1]区间进行拟合
        2. 保持对原始x的标准多项式系数约束
        3. 返回针对原始x的多项式系数

        Parameters:
            x, y: 输入数据
            order: 切比雪夫多项式阶数
            parity: 0-无约束,1-奇函数,2-偶函数
            bounds: 标准多项式系数的约束范围
            hasBound: 是否启用约束
        Returns:
            cheb_coeff: 切比雪夫系数（基于归一化x）
            poly_coeff: 对应的标准多项式系数（针对原始x）
        """
        # 1. 数据归一化
        x_min, x_max = x_bound[0], x_bound[1]
        x_norm = 2 * (x - x_min)/(x_max - x_min) - 1  # 映射到[-1,1]
        x_scale = 2/(x_max - x_min)
        x_offset = -(x_min + x_max)/(x_max - x_min)
        # x_original_norm = 2 * (x_original - x_min)/(x_max - x_min) - 1
        # print(f"xmin:{np.min(x)};xmax:{np.max(x)}; bound: {x_min}, {x_max}; parity: {parity}")
        # print(f"x_scale:{x_scale}; x_offset:{x_offset}")

        # 3. 根据奇偶性选择基函数
        if parity == 1:
            indices = [i for i in range(1, order+1, 2)] if order > 0 else [0]
        elif parity == 2:
            indices = [i for i in range(0, order+1, 2)]
        else:
            indices = list(range(order+1))
        # print(f"indices: {indices}, order: {order}, parity: {parity}")

        # 4. 构造切比雪夫基矩阵（稀疏矩阵优化）
        V = sparse.csr_matrix(chebvander(x_norm, order)[:, indices])

        # 5. 定义优化变量
        c = cp.Variable(len(indices))

        # 6. 构建标准多项式系数约束（针对原始x）
        constraints = []
        # if hasBound: //@yuan: maybe the coefficient must be constrainted
        # print(f"constraint")
        #1. the constraint of coeffcient
        # 预计算转换关系矩阵：原始x多项式系数与切比雪夫系数的关系
        y_abs = np.abs(y)
        y_abs_max = y_abs[np.argmax(y_abs)]
        x_abs = np.abs(x)
        x_abs_max = x_abs[np.argmax(x_abs)]
        mul_error_factor = 0.004
        A = np.zeros((order+1, len(indices)))
        for j in range(order + 1):
            if (parity == 1 and j % 2 == 0) or (parity == 2 and j % 2 == 1):
                continue
            # 计算x^j项的系数转换关系（考虑归一化缩放）
            for idx, k in enumerate(indices):
                if k >= j:
                    # 计算T_k(x')中x'^j的系数
                    poly_Tk = np.zeros(order+1)
                    poly_Tk[k] = 1
                    standard_Tk = cheb2poly(poly_Tk)
                    if j < len(standard_Tk):
                        # 考虑x' = scale*x + offset的变量替换
                        # 需要展开 (scale*x + offset)^j 的各项
                        for m in range(j+1):
                            binom = math.comb(j, m)
                            scale_factor = binom * (x_scale**m) * (x_offset**(j-m))
                            A[m, idx] += standard_Tk[j] * scale_factor
            # 添加约束（针对原始x的x^j系数）
            x_upper = x_abs_max ** j
            # another constraint is to avoid each term of polynomial is too large, although the error for each term is small, the error may be too large for the target function value
            # print(f" x_max: {x_max}; x_upper: {x_upper}; y_abs_max: {y_abs_max}")
            if int_width == 0:
                Q_bound = 1e20
            else:
                Q_bound = 2**(int_width - 1) - 2**(-frac_width)
            constraint = min(y_abs_max/mul_error_factor, Q_bound)
            constraints += [cp.abs(A[j,:] @ c * x_upper) <= (constraint)]
            # constraints += [cp.abs(A[j,:] @ c * x_upper)*mul_error_factor <= (y_abs_max)]
            # constraints += [cp.abs(A[j,:] @ c * x_upper) <= (Q_bound)]
        #2. the constraint of the sum of the polyminal
        # y_upper_bound = np.full_like(y, bounds[1])
        # y_lower_bound = np.full_like(y, bounds[0])
        # if len(y_upper_bound) != len(y) or len(y_lower_bound) != len(y):
        #     raise ValueError("y_bound must be scalar or same length as y")
        # constraints.append(V @ c <= y_upper_bound)
        # constraints.append(V @ c >= y_lower_bound)
             

        # 7. 构建并求解优化问题
        # objective = cp.Minimize(cp.sum_squares(V @ c - y) + 1e-10*cp.sum_squares(c))
        objective = cp.Minimize(cp.sum_squares(V @ c - y))
        problem = cp.Problem(objective, constraints)

        try:
            #@yuan: the setting for the solvers
            verbose = False
            osqp_args = {
            "max_iter": max_iter,
            "eps_abs": eps_abs,
            "eps_rel": eps_rel,
            "polish": True,
            "warm_start": True,
            "verbose": verbose
            }
            problem.solve(solver=cp.OSQP, **osqp_args)
        except Exception as e:
            raise Exception(f"SolverError: {e}, status: {problem.status}")
            
        
        # 8. 处理结果
        cheb_coeff = np.zeros(order+1)
        # print(f"problem.status: {problem.status}")
        # if problem.status in [cp.OPTIMAL, cp.OPTIMAL_INACCURATE, cp.USER_LIMIT]:
        if problem.status in [cp.OPTIMAL, cp.OPTIMAL_INACCURATE, cp.USER_LIMIT]:
            if c.value is not None:
                cheb_coeff[indices] = c.value
        else:
            cheb_coeff = np.full(order + 1, float("inf"))
            raise Exception("SolverError")

        # 9. 转换为原始x的多项式系数
        # poly_norm = cheb2poly(cheb_coeff)  # 归一化x域的多项式系数
        # poly_coeff = _transform_poly(poly_norm, x_scale, x_offset)
        # print(f"poly_coeff: {poly_coeff}")
        return cheb_coeff
       
    # @staticmethod
    # def chebfit_normCoeff(x_bound, x, y, order, parity, bounds=(-2, 2), hasBound=False, int_width = 5, max_iter=25000, eps_abs=1e-8, eps_rel=1e-8):
    #     """
    #     带标准多项式系数约束的切比雪夫拟合（改进版）
    #     1. 将x归一化到[-1,1]区间进行拟合
    #     2. 保持对原始x的标准多项式系数约束
    #     3. 返回针对原始x的多项式系数

    #     Parameters:
    #         x, y: 输入数据
    #         order: 切比雪夫多项式阶数
    #         parity: 0-无约束,1-奇函数,2-偶函数
    #         bounds: 标准多项式系数的约束范围
    #         hasBound: 是否启用约束
    #     Returns:
    #         cheb_coeff: 切比雪夫系数（基于归一化x）
    #         poly_coeff: 对应的标准多项式系数（针对原始x）
    #     """
    #     # 1. 数据归一化
    #     x_min, x_max = x_bound[0], x_bound[1]
    #     x_norm = 2 * (x - x_min)/(x_max - x_min) - 1  # 映射到[-1,1]
    #     x_scale = 2/(x_max - x_min)
    #     x_offset = -(x_min + x_max)/(x_max - x_min)
    #     # x_original_norm = 2 * (x_original - x_min)/(x_max - x_min) - 1
    #     # print(f"x:{x}; x_norm:{x_norm}")
    #     # print(f"x_scale:{x_scale}; x_offset:{x_offset}")

    #     # 3. 根据奇偶性选择基函数
    #     if parity == 1:
    #         indices = [i for i in range(1, order+1, 2)] if order > 0 else [0]
    #     elif parity == 2:
    #         indices = [i for i in range(0, order+1, 2)]
    #     else:
    #         indices = list(range(order+1))
    #     # print(f"indices: {indices}, order: {order}, parity: {parity}")

    #     # 4. 构造切比雪夫基矩阵（稀疏矩阵优化）
    #     V = sparse.csr_matrix(chebvander(x_norm, order)[:, indices])

    #     # 5. 定义优化变量
    #     c = cp.Variable(len(indices))

    #     # 6. 构建标准多项式系数约束（针对原始x）
    #     constraints = []
    #     # if hasBound: //@yuan: maybe the coefficient must be constrainted
    #     # print(f"constraint")
    #     #1. the constraint of coeffcient
    #     # 预计算转换关系矩阵：原始x多项式系数与切比雪夫系数的关系
    #     y_abs = np.abs(y)
    #     y_abs_max = y_abs[np.argmax(y_abs)]
    #     x_abs = np.abs(x)
    #     x_abs_max = x_abs[np.argmax(x_abs)]
    #     mul_error_factor = 0.00000288
    #     A = np.zeros((order+1, len(indices)))
    #     for j in range(order + 1):
    #         if (parity == 1 and j % 2 == 0) or (parity == 2 and j % 2 == 1):
    #             continue
    #         # 计算x^j项的系数转换关系（考虑归一化缩放）
    #         for idx, k in enumerate(indices):
    #             if k >= j:
    #                 # 计算T_k(x')中x'^j的系数
    #                 poly_Tk = np.zeros(order+1)
    #                 poly_Tk[k] = 1
    #                 standard_Tk = cheb2poly(poly_Tk)
    #                 if j < len(standard_Tk):
    #                     # 考虑x' = scale*x + offset的变量替换
    #                     # 需要展开 (scale*x + offset)^j 的各项
    #                     for m in range(j+1):
    #                         binom = math.comb(j, m)
    #                         scale_factor = binom * (x_scale**m) * (x_offset**(j-m))
    #                         A[m, idx] += standard_Tk[j] * scale_factor
    #         # 添加约束（针对原始x的x^j系数）
    #         #@yuan: constraint the coefficient for each term based on the range of x
    #         #each row of A means the coefficient contribution for x^j for each T
    #         x_upper = x_abs_max ** j
    #         if int_width != 0:# contraints for fixed point data
    #             if x_upper == 0:
    #                 x_upper = 1e-9
    #             upper_bound = bounds[1] / x_upper
    #             lower_bound = bounds[0] / x_upper
    #             upper_bound = min(upper_bound, 2**31 - 1)
    #             upper_bound = max(upper_bound, 1 / (2 ** (32 - int_width)))
    #             lower_bound = max(lower_bound, -2**31)
    #             lower_bound = min(lower_bound, -1 / (2 ** (32 - int_width)))
    #             # print(f" x_max: {x_max}; x_upper: {x_upper}; upper_bound: {upper_bound}; lower_bound: {lower_bound}")
    #             constraints += [A[j,:] @ c <= (upper_bound), 
    #                           A[j,:] @ c >= (lower_bound)]
    #         # another constraint is to avoid each term of polynomial is too large, although the error for each term is small, the error may too large for the target function value
    #         # print(f" x_max: {x_max}; x_upper: {x_upper}; y_abs_max: {y_abs_max}")
    #         constraints += [cp.abs(A[j,:] @ c * x_upper) <= (y_abs_max*10)]
    #     #2. the constraint of the sum of the polyminal
    #     # y_upper_bound = np.full_like(y, bounds[1])
    #     # y_lower_bound = np.full_like(y, bounds[0])
    #     # if len(y_upper_bound) != len(y) or len(y_lower_bound) != len(y):
    #     #     raise ValueError("y_bound must be scalar or same length as y")
    #     # constraints.append(V @ c <= y_upper_bound)
    #     # constraints.append(V @ c >= y_lower_bound)
             

    #     # 7. 构建并求解优化问题
    #     # objective = cp.Minimize(cp.sum_squares(V @ c - y) + 1e-10*cp.sum_squares(c))
    #     objective = cp.Minimize(cp.sum_squares(V @ c - y))
    #     problem = cp.Problem(objective, constraints)

    #     try:
    #         #@yuan: the setting for the solvers
    #         verbose = False
    #         osqp_args = {
    #         "max_iter": max_iter,
    #         "eps_abs": eps_abs,
    #         "eps_rel": eps_rel,
    #         "polish": True,
    #         "warm_start": True,
    #         "verbose": verbose
    #         }
    #         problem.solve(solver=cp.OSQP, **osqp_args)
    #     except cp.error.SolverError:
    #         pass
            
        
    #     # 8. 处理结果
    #     cheb_coeff = np.zeros(order+1)
    #     # print(f"problem.status: {problem.status}")
    #     # if problem.status in [cp.OPTIMAL, cp.OPTIMAL_INACCURATE, cp.USER_LIMIT]:
    #     if problem.status in [cp.OPTIMAL, cp.OPTIMAL_INACCURATE, cp.USER_LIMIT]:
    #         if c.value is not None:
    #             cheb_coeff[indices] = c.value
    #     else:
    #         cheb_coeff = np.full(order + 1, float("inf"))

    #     # 9. 转换为原始x的多项式系数
    #     # poly_norm = cheb2poly(cheb_coeff)  # 归一化x域的多项式系数
    #     # poly_coeff = _transform_poly(poly_norm, x_scale, x_offset)
    #     # print(f"poly_coeff: {poly_coeff}")
    #     return cheb_coeff
    
    @staticmethod
    def cheb2poly_norm(x, cheb_coeff):
        def _transform_poly(poly_norm, scale, offset):
            """
            将归一化域的多项式系数转换为原始x域的系数
            poly_norm: 归一化域x'的多项式系数
            scale: x' = scale*x + offset
            """
            n = len(poly_norm)
            poly_coeff = np.zeros(n)

            # 多项式变量替换：x' = scale*x + offset
            for k in range(n):
                if poly_norm[k] == 0:
                    continue
                # 展开 (scale*x + offset)^k
                for j in range(k+1):
                    binom = math.comb(k, j)
                    term = binom * (scale**j) * (offset**(k-j))
                    poly_coeff[j] += poly_norm[k] * term

            return poly_coeff
        #转换为原始x的多项式系数
        x_min, x_max = np.min(x), np.max(x)
        x_scale = 2/(x_max - x_min)
        x_offset = -(x_min + x_max)/(x_max - x_min)
        poly_norm = cheb2poly(cheb_coeff)  # 归一化x域的多项式系数
        poly_coeff = _transform_poly(poly_norm, x_scale, x_offset)
        # print(f"poly_coeff: {poly_coeff}")
        return poly_coeff