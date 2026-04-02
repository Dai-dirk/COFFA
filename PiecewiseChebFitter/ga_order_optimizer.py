import random
from deap import base, creator, tools, algorithms
from typing import List, Tuple, Any
import numpy as np
# from multiprocessing import Pool, cpu_count
from pathos.multiprocessing import ProcessingPool as Pool, cpu_count
# import deap


class GAOrderOptimizer:
    """
    使用遗传算法优化阶数分配的类。

    参数:
        eval_function (function): 评估函数
        n_segments (int): 分段数
        max_degree (int): 每个分段的最大阶数
        max_total_degree (int): 所有分段阶数之和的最大值
        population_size (int): 种群大小
        num_generations (int): 迭代代数
        cxpb (float): 交叉概率
        mutpb (float): 变异概率
    """

    def __init__(
        self,
        eval_function,
        n_segments: int = 5,
        initial_orders_dict: dict = None,  # 来自setup_initial_orders的结果
        max_degree: int = 10,
        max_total_degree: int = 30,
        population_size: int = 100,
        lambda_: int = 50,
        num_generations: int = 50,
        cxpb: float = 0.7,
        mutpb: float = 0.2,
        parity: int = 0, #@yuan: the parity of the target function
        # multiprocessing: bool = False,
        pool_size: int = 4,
        verbose: bool = True,
        use_initial_orders_ratio: float = 0.3,  # 使用初始阶数作为基础的个体比例
    ):
        self.eval_function = eval_function
        self.n_segments = n_segments
        self.initial_orders_dict = initial_orders_dict
        self.use_initial_orders_ratio = use_initial_orders_ratio
        self.max_degree = max_degree
        self.max_total_degree = max_total_degree
        self.population_size = population_size
        self.num_generations = num_generations
        self.mu = population_size
        self.lambda_ = lambda_
        self.cxpb = cxpb
        self.mutpb = mutpb
        # self.multiprocessing = multiprocessing
        self.pool_size = pool_size
        self.verbose = verbose
        self.parity = parity
        # 自定义缓存字典
        self.eval_cache = {}
        # 设置适应度和个体类型
        if "FitnessMin" not in creator.__dict__:
            creator.create("FitnessMin", base.Fitness, weights=(-1.0,))
        if "Individual" not in creator.__dict__:
            creator.create("Individual", list, fitness=creator.FitnessMin)
        # creator.create("FitnessMin", base.Fitness, weights=(-1.0,))
        # creator.create("Individual", list, fitness=creator.FitnessMin)
        self.pool = None  # Pool 延迟初始化
        # 初始化工具箱
        self.toolbox = base.Toolbox()
        self._setup_toolbox()

    def set_config(self, config: dict):
        """
        设置配置
        """
        for key, value in config.items():
            setattr(self, key, value)

    def clear_cache(self):
        """
        清理评估缓存
        """
        self.eval_cache.clear()

    def _enforce_parity_symmetry(self, orders: List[int]) -> List[int]:
        """
        强制执行奇偶性对称约束
        
        Args:
            orders: 原始阶数列表
            
        Returns:
            满足对称性约束的阶数列表
        """
        if self.parity <= 0:
            return orders.copy()
        
        size = len(orders)
        result = orders.copy()
        
        if size % 2 == 0:  # 偶数分段数
            half = size // 2
            for i in range(half):
                sym_idx = size - 1 - i
                # 取两个对称位置的平均值（四舍五入）
                avg_order = round((result[i] + result[sym_idx]) / 2)
                avg_order = max(0, min(self.max_degree, avg_order))
                result[i] = avg_order
                result[sym_idx] = avg_order
        else:  # 奇数分段数
            half = (size - 1) // 2
            for i in range(half):
                sym_idx = size - 1 - i
                # 取两个对称位置的平均值（四舍五入）
                avg_order = round((result[i] + result[sym_idx]) / 2)
                avg_order = max(0, min(self.max_degree, avg_order))
                result[i] = avg_order
                result[sym_idx] = avg_order
        
        return result

    def _generate_from_initial_orders(self) -> List[int]:
        """
        基于setup_initial_orders的结果生成个体，添加随机扰动并确保满足奇偶性
        
        Returns:
            满足约束的个体
        """
        if self.initial_orders_dict is None:
            return self._random_composition()
        
        # 获取当前分段数对应的初始阶数
        if self.n_segments not in self.initial_orders_dict:
            return self._random_composition()
        
        base_orders = self.initial_orders_dict[self.n_segments].copy()
        
        # 添加随机扰动
        for i in range(len(base_orders)):
            # 小幅度随机调整 (-2 到 +2)
            delta = random.randint(-2, 2)
            base_orders[i] = max(0, min(self.max_degree, base_orders[i] + delta))
        
        # 强制执行奇偶性对称约束
        base_orders = self._enforce_parity_symmetry(base_orders)
        
        # 修复约束（总阶数限制）
        base_orders = self._repair(base_orders)
        
        return base_orders

    def _smart_composition(self) -> List[int]:
        """
        智能初始化策略：按比例混合使用初始阶数和随机生成
        
        Returns:
            生成的个体
        """
        # 根据设定的比例决定是否使用初始阶数作为基础
        if (self.initial_orders_dict is not None and 
            random.random() < self.use_initial_orders_ratio):
            return self._generate_from_initial_orders()
        else:
            return self._random_composition()

    def _setup_toolbox(self) -> None:
        """设置工具箱中的各种操作"""
        self.toolbox.register(
            "individual", lambda: creator.Individual(self._smart_composition())
        )
        self.toolbox.register(
            "population", tools.initRepeat, list, self.toolbox.individual
        )
        self.toolbox.register("evaluate", self._eval_degree)
        self.toolbox.register("mate", self._cx_composition)
        self.toolbox.register("mutate", self._mut_transfer, indpb=0.5)
        # self.toolbox.register("select", tools.selTournament, tournsize=3)
        self.toolbox.register("select", self.stochastic_tournament, tournament_size=3, selection_prob=0.7)

    def _random_composition(self) -> List[int]:
        """
        随机生成满足以下条件的n_segments个数：
        1. 每个元素不超过max_degree
        2. 所有元素之和不超过max_total_degree
        3. 对于具有奇偶性的函数来说，需要避免重复遍历一些含义相同的阶数分布
        """
        if self.parity > 0: #@yuan: for the function is odd or even, the order distribution is symmetric
            if self.n_segments % 2 == 0:  # the number of segment is an even number
                half = self.n_segments // 2
                first_half = [random.randint(0, self.max_degree) for _ in range(half)]
                individual = first_half + first_half[::-1]
            else:  # odd number 
                half = self.n_segments // 2
                first_half = [random.randint(0, self.max_degree) for _ in range(half)]
                center = random.randint(0, self.max_degree)
                individual = first_half + [center] + first_half[::-1]

        else:
                # 首先随机生成不超过max_degree的阶数
            individual = [
                random.randint(0, self.max_degree) for _ in range(self.n_segments)
            ]
        # 如果总和超过max_total_degree，则随机减少某些分段的阶数
        while sum(individual) > self.max_total_degree:
            if self.parity > 0 and self.n_segments % 2 == 0:  # 偶数对称
                # 选择前半段的一个随机索引
                idx = random.randint(0, self.n_segments // 2 - 1)
                sym_idx = self.n_segments - 1 - idx  # 对称位置
                
                # 如果两个位置都大于0，则同时减少
                if individual[idx] > 0 and individual[sym_idx] > 0:
                    individual[idx] -= 1
                    individual[sym_idx] -= 1
            
            elif self.parity > 0 and self.n_segments % 2 != 0:  # 奇数对称
                # 有1/3概率减少中心点，2/3概率减少对称对
                if random.random() < 1/3:
                    center_idx = self.n_segments // 2
                    if individual[center_idx] > 0:
                        individual[center_idx] -= 1
                else:
                    idx = random.randint(0, self.n_segments // 2 - 1)
                    sym_idx = self.n_segments - 1 - idx  # 对称位置
                    if individual[idx] > 0 and individual[sym_idx] > 0:
                        individual[idx] -= 1
                        individual[sym_idx] -= 1
            
            else:  # 无对称性要求
                # 随机选择一个非零的分段
                non_zero_indices = [i for i, v in enumerate(individual) if v > 0]
                if not non_zero_indices:
                    break
                idx = random.choice(non_zero_indices)
                individual[idx] -= 1

        return individual

    def _eval_degree(self, individual: List[int]) -> Tuple[float]:
        """
        评估函数：计算个体与理想阶数分配的误差
        """
        # ideal = self.max_degree / self.n_segments
        # error = sum((x - ideal) ** 2 for x in individual)
        # 将 individual 转换为 tuple 作为缓存键
        cache_key = tuple(individual)
        if cache_key in self.eval_cache:
            return self.eval_cache[cache_key]
        objective = self.eval_function(individual)["objective"]
        self.eval_cache[cache_key] = objective
        return (objective,)

    def _repair(self, individual: List[int]) -> List[int]:
        """
        修正个体，确保：
        1. 每个元素不超过max_degree
        2. 所有元素之和不超过max_total_degree
        """ 
        # 确保每个元素不超过max_degree
        for i in range(len(individual)):
            if individual[i] > self.max_degree:
                    individual[i] = self.max_degree
            elif individual[i] < 0:
                    individual[i] = 0
        size = len(individual)
        if self.parity > 0 and size > 1:
            # 确保对称位置值相同
            if size % 2 == 0:  # 偶数段数
                half = size // 2
                for i in range(half):
                    sym_idx = size - 1 - i
                    # 如果对称位置值不同，随机选择其中一个值
                    if individual[i] != individual[sym_idx]:
                        chosen_val = random.choice([individual[i], individual[sym_idx]])
                        individual[i] = chosen_val
                        individual[sym_idx] = chosen_val
            else:  # 奇数段数
                half = (size - 1) // 2
                for i in range(half):
                    sym_idx = size - 1 - i
                    # 如果对称位置值不同，随机选择其中一个值
                    if individual[i] != individual[sym_idx]:
                        chosen_val = random.choice([individual[i], individual[sym_idx]])
                        individual[i] = chosen_val
                        individual[sym_idx] = chosen_val

        # 然后确保总和不超过max_total_degree
        while sum(individual) > self.max_total_degree:
            if self.parity > 0 and self.n_segments % 2 == 0:  # 偶数对称
                # 选择前半段的一个随机索引
                idx = random.randint(0, self.n_segments // 2 - 1)
                sym_idx = self.n_segments - 1 - idx  # 对称位置
                
                # 如果两个位置都大于0，则同时减少
                if individual[idx] > 0 and individual[sym_idx] > 0:
                    individual[idx] -= 1
                    individual[sym_idx] -= 1
            
            elif self.parity > 0 and self.n_segments % 2 != 0:  # 奇数对称
                # 有1/3概率减少中心点，2/3概率减少对称对
                if random.random() < 1/3:
                    center_idx = self.n_segments // 2
                    if individual[center_idx] > 0:
                        individual[center_idx] -= 1
                else:
                    idx = random.randint(0, self.n_segments // 2 - 1)
                    sym_idx = self.n_segments - 1 - idx  # 对称位置
                    if individual[idx] > 0 and individual[sym_idx] > 0:
                        individual[idx] -= 1
                        individual[sym_idx] -= 1
            
            else:  # 无对称性要求
                # 随机选择一个非零的分段
                non_zero_indices = [i for i, v in enumerate(individual) if v > 0]
                if not non_zero_indices:
                    break
                idx = random.choice(non_zero_indices)
                individual[idx] -= 1

        return individual

    # def _cx_composition(
    #     self, ind1: List[int], ind2: List[int]
    # ) -> Tuple[List[int], List[int]]:
    #     """
    #     自定义交叉操作
    #     """
    #     size = len(ind1)
    #     # 使用算术交叉
    #     alpha = random.random()
    #     child1 = [round(alpha * ind1[i] + (1 - alpha) * ind2[i]) for i in range(size)]
    #     child2 = [round((1 - alpha) * ind1[i] + alpha * ind2[i]) for i in range(size)]

    #     # 确保满足约束条件
    #     self._repair(child1)
    #     self._repair(child2)

    #     ind1[:] = child1
    #     ind2[:] = child2
    #     return ind1, ind2
    def _cx_composition(
        self, ind1: List[int], ind2: List[int]
    ) -> Tuple[List[int], List[int]]:
        """
        考虑对称性的交叉操作：
        - 当需要对称性时（parity > 0），只交叉前半部分，然后对称复制
        - 否则使用标准的均匀交叉
        """
        size = len(ind1)
        if self.parity > 0:
            # 对称性处理 - 只交叉前半部分，然后对称复制
            if size % 2 == 0:  # 偶数段数
                half = size // 2
                child1_front = []
                child2_front = []

                for i in range(half):
                    # 均匀交叉
                    if random.random() < 0.5:
                        gene1 = ind1[i]
                    else:
                        gene1 = ind2[i]

                    if random.random() < 0.5:
                        gene2 = ind1[i]
                    else:
                        gene2 = ind2[i]

                    # 加入扰动
                    delta1 = random.choice([-2, -1, 0, 1, 2])
                    delta2 = random.choice([-2, -1, 0, 1, 2])

                    child1_front.append(max(0, min(self.max_degree, gene1 + delta1)))
                    child2_front.append(max(0, min(self.max_degree, gene2 + delta2)))

                # 构建完整对称个体
                child1 = child1_front + child1_front[::-1]
                child2 = child2_front + child2_front[::-1]

            else:  # 奇数段数
                half = (size - 1) // 2
                child1_front = []
                child2_front = []

                # 交叉前半部分
                for i in range(half):
                    if random.random() < 0.5:
                        gene1 = ind1[i]
                    else:
                        gene1 = ind2[i]

                    if random.random() < 0.5:
                        gene2 = ind1[i]
                    else:
                        gene2 = ind2[i]

                    # 加入扰动
                    delta1 = random.choice([-2, -1, 0, 1, 2])
                    delta2 = random.choice([-2, -1, 0, 1, 2])

                    child1_front.append(max(0, min(self.max_degree, gene1 + delta1)))
                    child2_front.append(max(0, min(self.max_degree, gene2 + delta2)))

                # 交叉中心点
                center_idx = half
                if random.random() < 0.5:
                    center1 = ind1[center_idx]
                    center2 = ind2[center_idx]
                else:
                    center1 = ind2[center_idx]
                    center2 = ind1[center_idx]

                # 中心点扰动
                delta_center1 = random.choice([-2, -1, 0, 1, 2])
                delta_center2 = random.choice([-2, -1, 0, 1, 2])

                center1 = max(0, min(self.max_degree, center1 + delta_center1))
                center2 = max(0, min(self.max_degree, center2 + delta_center2))

                # 构建完整对称个体
                child1 = child1_front + [center1] + child1_front[::-1]
                child2 = child2_front + [center2] + child2_front[::-1]

        else:
            # 非对称情况 - 标准均匀交叉
            child1 = []
            child2 = []

            for i in range(size):
                if random.random() < 0.5:
                    gene1 = ind1[i]
                else:
                    gene1 = ind2[i]

                if random.random() < 0.5:
                    gene2 = ind1[i]
                else:
                    gene2 = ind2[i]

                # 加入扰动
                delta1 = random.choice([-2, -1, 0, 1, 2])
                delta2 = random.choice([-2, -1, 0, 1, 2])

                child1.append(max(0, min(self.max_degree, gene1 + delta1)))
                child2.append(max(0, min(self.max_degree, gene2 + delta2)))

        # 确保满足约束条件
        self._repair(child1)
        self._repair(child2)

        ind1[:] = child1
        ind2[:] = child2
        return ind1, ind2

    def _mut_transfer(self, individual: List[int], indpb: float) -> Tuple[List[int]]:
        size = len(individual)
        if self.parity > 0:
            if size % 2 == 0: #偶数项数
                half = size // 2
                for i in range(half):
                    sym_idx = size - 1 - i
                    if random.random() < indpb:
                        delta = 1
                        if random.random() < 0.2:
                            delta = random.randint(-self.max_degree, self.max_degree)
                        else:
                            delta = random.choice([-2, -1, 1, 2])
                        if delta > 0:
                            if (
                                individual[i] +  delta <= self.max_degree
                                and sum(individual) + 2*delta <= self.max_total_degree
                            ):
                                individual[i] += delta
                                individual[sym_idx] += delta
                        else:
                            if individual[i] + delta > 0:
                                individual[i] += delta
                                individual[sym_idx] += delta
            else: 
                half = (size - 1) // 2
                for i in range(half):
                    sym_idx = size - 1 - i
                    if random.random() < indpb:
                        delta = 1
                        if random.random() < 0.2:
                            delta = random.randint(-self.max_degree, self.max_degree)
                        else:
                            delta = random.choice([-2, -1, 1, 2])
                        if delta > 0:
                            if (
                                individual[i] +  delta <= self.max_degree
                                and sum(individual) + 2*delta <= self.max_total_degree
                            ):
                                individual[i] += delta
                                individual[sym_idx] += delta
                        else:
                            if individual[i] + delta > 0:
                                individual[i] += delta
                                individual[sym_idx] += delta
                
                center_idx = half #中心点
                if random.random() < indpb:
                    delta = 1
                    if random.random() < 0.2:
                        delta = random.randint(-self.max_degree, self.max_degree)
                    else:
                        delta = random.choice([-2, -1, 1, 2])
                    if delta > 0:
                        if (
                            individual[center_idx] +  delta <= self.max_degree
                            and sum(individual) + delta <= self.max_total_degree
                        ):
                            individual[center_idx] += delta
                    else:
                        if individual[i] + delta > 0:
                            individual[center_idx] += delta
        else:
            for i in range(len(individual)):
                if random.random() < indpb:
                    delta = 1
                    if random.random() < 0.2:
                        delta = random.randint(-self.max_degree, self.max_degree)
                    else:
                        delta = random.choice([-2, -1, 1, 2])
                    if delta > 0:
                        if (
                            individual[i] +  delta <= self.max_degree
                            and sum(individual) + delta <= self.max_total_degree
                        ):
                            individual[i] += delta
                    else:
                        if individual[i] + delta > 0:
                            individual[i] += delta
        self._repair(individual)
        return (individual,)
    def stochastic_tournament(self, population, k, tournament_size=3, selection_prob=0.7):
        """
        随机竞赛选择：有一定概率选择非最优个体
        selection_prob: 选择最优个体的概率
        """
        selected = []
        for _ in range(k):
            # 随机选择 tournament_size 个个体
            contestants = random.sample(population, tournament_size)

            # 以 selection_prob 的概率选择最优个体
            if random.random() < selection_prob:
                winner = min(contestants, key=lambda x: x.fitness.values[0])
            else:
                # 否则随机选择一个非最优个体
                non_best = [ind for ind in contestants if ind != min(contestants, key=lambda x: x.fitness.values[0])]
                winner = random.choice(non_best) if non_best else contestants[0]

            selected.append(winner)

        return selected

    def optimize(self) -> Tuple[List[int], float]:
        """
        运行遗传算法优化过程

        返回:
            Tuple[List[int], float]: 最优解和对应的目标函数值
        """
        # if self.multiprocessing:
        self.pool = Pool(nodes=self.pool_size)
        self.toolbox.register("map", self.pool.map)

        try:
            population = self.toolbox.population(n=self.population_size)
            
            hof = tools.HallOfFame(8)# hall of fame, which records the best result

            stats = tools.Statistics(lambda ind: ind.fitness.values[0])
            stats.register("avg", np.mean)
            stats.register("std", np.std)
            stats.register("min", np.min)
            stats.register("max", np.max)

            result_population, logbook = algorithms.eaMuPlusLambda(
                population,
                self.toolbox,
                mu=self.mu,
                lambda_=self.lambda_,
                cxpb=self.cxpb,
                mutpb=self.mutpb,
                ngen=self.num_generations,
                stats=stats,
                verbose=self.verbose,
                halloffame=hof
            )
            #@yuan: after solving, we re-evaluate the results from hall of fame
            numCandidate = min(8, len(hof))
            candidates = list(hof[:numCandidate])
            print(f"candidates size: {len(candidates)}")
            candidate_results = list(self.pool.map(self.eval_function, candidates))
            best_index = np.argmin([res['objective'] for res in candidate_results])
            best_result = candidate_results[best_index]
            best_order = candidates[best_index]

            orders = tools.selBest(hof, 1)[0]
            # orders = tools.selBest(result_population, 1)[0]
            # final_population = result_population + list(hof)
            # best_orders = hof[0]
            # print(f"best order: {best_order}, hof size: {len(hof)}, hof: {hof}")
            # result = self.eval_function(orders)
            # result = self.eval_function(best_orders)
            return best_result, logbook

        finally:
            if self.pool is not None:
                self.pool.close()
                self.pool.join()
                self.pool.clear()
                self.pool = None
            # 清理缓存以释放内存
            self.clear_cache()

