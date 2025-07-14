import sys
import networkx as nx
import matplotlib.pyplot as plt

def runII(*argv):
    argv_size= len(argv)
    nodes_size = int(argv[0])
    access_node_list = [str(i)for i in range(0, nodes_size)]
    argv_conflict =[]
    for j in range(1, argv_size):
        argv_conflict.append(str(int(argv[j])))
    # print(conflict_pairs)
    conflict_pairs = [(argv_conflict[i], argv_conflict[i + 1])for i in range(0, len(argv_conflict), 2)]
    # print(conflict pairs)
    # 创建图
    G = nx.Graph()
    # 添加节点
    G.add_nodes_from(access_node_list)#添加边，表示互斥关系
    G.add_edges_from(conflict_pairs)
    # 使用图着色算法
    coloring = nx.greedy_color(G, strategy="largest_first")# 打印每个节点的着色
    # print(coloring)
    # 找到需要的时间步骤
    num_steps = max(coloring.values()) + 1
    # print("最小时间:", num_steps)
    return num_steps

def runCtrlStep(*argv):
    argv_size= len(argv)
    nodes_size = int(argv[0])
    access_node_list = [str(i)for i in range(0, nodes_size)]
    argv_conflict =[]
    for j in range(1, argv_size):
        argv_conflict.append(str(int(argv[j])))
    # print(conflict_pairs)
    conflict_pairs = [(argv_conflict[i], argv_conflict[i + 1])for i in range(0, len(argv_conflict), 2)]
    # print(conflict pairs)
    # 创建图
    G = nx.Graph()
    # 添加节点
    G.add_nodes_from(access_node_list)#添加边，表示互斥关系
    G.add_edges_from(conflict_pairs)
    # 使用图着色算法
    coloring = nx.greedy_color(G, strategy="largest_first")# 打印每个节点的着色
    # print(coloring)
    # print(type(coloring))

    # 找到需要的时间步骤
    num_steps = max(coloring.values()) + 1
    # print("最小时间:", num_steps)
    return coloring

if __name__ == "__main__":
    argv_size= len(sys.argv)-1 #the first arg is file name
    nodes_size = int(sys.argv[1])
    access_node_list = [str(i)for i in range(1, nodes_size + 1)]
    argv_conflict =[]
    for j in range(2, argv_size + 1):
        argv_conflict.append(str(int(sys.argv[j])))
    # print(conflict_pairs)
    conflict_pairs = [(argv_conflict[i], argv_conflict[i + 1])for i in range(0, len(argv_conflict), 2)]
    # print(conflict pairs)
    # 创建图
    G = nx.Graph()
    # 添加节点
    G.add_nodes_from(access_node_list)#添加边，表示互斥关系
    G.add_edges_from(conflict_pairs)
    # 使用图着色算法
    coloring = nx.greedy_color(G, strategy="largest_first")# 打印每个节点的着色
    # print(coloring)
    # 找到需要的时间步骤
    num_steps = max(coloring.values()) + 1
    # print("最小时间:", num_steps)

