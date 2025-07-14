# 打开文本文件并读取内容
with open('Usage.txt', 'r') as file:
    content = file.readlines()

# 提取数字部分并转换为整数列表
numbers = []
for line in content:
    line = line.strip()  # 去除行尾的换行符
    if ':' in line:
        line = line.split(':')[1]  # 提取冒号后面的部分
        numbers += [int(num) for num in line.split()]

# 找到最大元素
max_number = max(numbers)

# 输出最大元素
print("最大元素为:", max_number)