import os
import sys
import torch
import torch.nn as nn
from torchvision import datasets, transforms
from tqdm import tqdm
import time

# 添加当前目录到路径，以便导入senet模块
sys.path.insert(0, os.path.dirname(__file__))
import senet_all
from senet_all import se_resnet152


class RunningMinMax:
    def __init__(self):
        self.global_min = float('inf')
        self.global_max = float('-inf')
        self.num_updates = 0

    def update(self, x: torch.Tensor) -> None:
        vmin = torch.min(x).item()
        vmax = torch.max(x).item()
        if vmin < self.global_min:
            self.global_min = vmin
        if vmax > self.global_max:
            self.global_max = vmax
        self.num_updates += 1


def register_activation_input_range_hooks(model: nn.Module):
    stats = {
        'ReLU': RunningMinMax(),
        'Sigmoid': RunningMinMax(),
    }
    handles = []

    def make_hook(kind: str):
        def hook(_module, inputs, _output):
            if not inputs:
                return
            x = inputs[0]
            if isinstance(x, (tuple, list)):
                x = x[0]
            if not torch.is_tensor(x):
                return
            with torch.no_grad():
                stats[kind].update(x)
        return hook

    for m in model.modules():
        if isinstance(m, nn.ReLU):
            handles.append(m.register_forward_hook(make_hook('ReLU')))
        elif isinstance(m, nn.Sigmoid):
            handles.append(m.register_forward_hook(make_hook('Sigmoid')))

    return stats, handles


def load_imagenet_val(val_dir, batch_size=64, workers=4, pin_memory=True):
    """加载ImageNet验证集"""
    # ImageNet标准化参数
    normalizer = transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                     std=[0.229, 0.224, 0.225])
    
    val_dataset = datasets.ImageFolder(
        val_dir,
        transforms.Compose([
            transforms.Resize(256),  # SE-ResNet使用256
            transforms.CenterCrop(224),  # SE-ResNet使用224
            transforms.ToTensor(),
            normalizer
        ])
    )
    
    val_loader = torch.utils.data.DataLoader(
        val_dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=workers,
        pin_memory=pin_memory
    )
    
    print(f'验证集大小: {len(val_dataset)}')
    print(f'类别数量: {len(val_dataset.classes)}')
    
    return val_loader


def validate_model(model, val_loader, criterion, device):
    """验证模型"""
    model.eval()
    
    running_loss = 0.0
    correct_top1 = 0
    correct_top5 = 0
    total = 0
    
    with torch.no_grad():
        for images, labels in tqdm(val_loader, desc='验证中'):
            images = images.to(device)
            labels = labels.to(device)
            
            # 前向传播
            outputs = model(images)
            loss = criterion(outputs, labels)
            
            # 统计
            running_loss += loss.item()
            total += labels.size(0)
            
            # Top-1 accuracy
            _, predicted = outputs.max(1)
            correct_top1 += predicted.eq(labels).sum().item()
            
            # Top-5 accuracy
            _, top5_preds = outputs.topk(5, 1, largest=True, sorted=True)
            correct_top5 += top5_preds.eq(labels.view(-1, 1).expand_as(top5_preds)).sum().item()
    
    avg_loss = running_loss / len(val_loader)
    top1_acc = 100. * correct_top1 / total
    top5_acc = 100. * correct_top5 / total
    
    return avg_loss, top1_acc, top5_acc


def main():
    # 设置设备
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    # device = torch.device('cpu')
    print(f'使用设备: {device}')
    
    # 设置路径 - 验证集在EfficientNet文件夹下
    val_dir = '/home/gbzou/EfficientNet/ILSVRC2012_img_val'
    
    # 加载验证集
    print('\n正在加载验证集...')
    val_loader = load_imagenet_val(val_dir, batch_size=64, workers=4)
    
    # 加载SE-ResNet152预训练模型
    print('\n正在加载SE-ResNet152预训练模型...')
    model = se_resnet152(num_classes=1000, pretrained=None)
    
    # 加载预训练权重
    weight_path = 'se_resnet152-d17c99b7.pth'
    if os.path.exists(weight_path):
        print(f'从 {weight_path} 加载预训练权重...')
        state_dict = torch.load(weight_path, map_location='cpu')
        model.load_state_dict(state_dict)
        print('预训练权重加载成功')
    else:
        print(f'警告: 未找到预训练权重文件 {weight_path}，使用随机初始化模型')
    
    model = model.to(device)
    
    # 注册激活函数输入范围统计
    # act_stats, act_handles = register_activation_input_range_hooks(model)
    # print('\n已启用激活函数输入范围统计 (ReLU, Sigmoid)')
    
    # 损失函数
    criterion = nn.CrossEntropyLoss()
    
    # 验证
    print('\n开始验证...')
    start_time = time.time()
    
    loss, top1_acc, top5_acc = validate_model(model, val_loader, criterion, device)
    
    elapsed_time = time.time() - start_time
    
    # 打印结果
    print('\n' + '='*60)
    print('验证结果:')
    print('='*60)
    print(f'Average Loss: {loss:.5f}')
    print(f'Top-1 Accuracy: {top1_acc:.5f}%')
    print(f'Top-5 Accuracy: {top5_acc:.5f}%')
    print(f'验证时间: {elapsed_time/60:.5f} 分钟')
    print('='*60)
    
    # 取消注册hooks
    # for h in act_handles:
    #     h.remove()
    
    # 打印激活输入范围统计
    # print('\n激活函数输入范围统计 (基于整个验证集的前向过程):')
    # for kind, mm in act_stats.items():
    #     if mm.num_updates == 0:
    #         print(f'- {kind}: 无数据')
    #     else:
    #         print(f'- {kind}: min={mm.global_min:.6f}, max={mm.global_max:.6f}, updates={mm.num_updates}')


if __name__ == '__main__':
    main()

