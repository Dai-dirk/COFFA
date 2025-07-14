with open('Usage.txt', 'r') as file:
    lines = file.readlines()

new_lines = []
for line in lines:
    line_data = line.split()
    print(line_data)
    if line_data[1] != "-1":
        line_data.pop(1) 
    # new_line = ' '.join(line.split()[1:]) + '\n'
    new_lines.append(' '.join(line_data) + '\n') 

with open('Usage.txt', 'w') as file:
    file.writelines(new_lines)


