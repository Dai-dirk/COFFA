#!/bin/bash

# 使用 find 命令递归查找所有 affine.dot 文件
find . -type f -name "affine.dot" | while read dotfile; do
    # 获取文件所在目录
    dir=$(dirname "$dotfile")
    # 获取文件名（不含路径）
    filename=$(basename "$dotfile")
    
    # 检查文件名是否包含 "mapped"
    if [[ ! "$filename" =~ "mapped" ]]; then
        # 生成输出文件名
        jsonfile="${dotfile%.dot}.json"
        
        # 转换文件
        dot -Tdot_json "$dotfile" -o "$jsonfile"
        echo "$dotfile -> $jsonfile"
    fi
done
