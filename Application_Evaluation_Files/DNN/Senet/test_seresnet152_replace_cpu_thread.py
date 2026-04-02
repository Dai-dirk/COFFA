      
import os
import sys
import json
import struct
import math
from typing import Any
import torch
import torch.nn as nn
from torchvision import datasets, transforms
from tqdm import tqdm
import time
from concurrent.futures import ProcessPoolExecutor
import multiprocessing as mp

# 添加当前目录到路径，以便导入senet模块
sys.path.insert(0, os.path.dirname(__file__))
import senet_all
from senet_all import se_resnet152

# Import LNS from the same directory (files have been moved here)
from LNS_Top.LNS_Top import lns_top


# Global process-pool management (reused across module instances)
_process_pool = None
_process_pool_lock = None
_DEFAULT_NUM_WORKERS = 10


def _init_process_pool_lock():
    """Initialize a lock to guard process-pool creation (Windows safe)."""
    global _process_pool_lock
    if _process_pool_lock is None:
        _process_pool_lock = mp.Lock()
    return _process_pool_lock


def _process_single_value(args):
    """Worker helper that computes one sigmoid approximation via LNS."""
    x_bits, params = args
    try:
        result = lns_top(
            seg = 7,
            x_in=x_bits,
            y_in=params['y_in'],
            n=params['n'],
            float_flag=params['float_flag'],
            break_points_in=params['break_points_in'],
            TRG=params['TRG'],
            VEC=params['VEC'],
            qi=params['qi'],
            Div=params['Div'],
            TRi=params['TRi'],
            power=params['power'],
            bias_sel=params['bias_sel'],
            constant_bias_in=params['constant_bias_in'],
            logc_in_0=params['logc_in_0'],
            K_in_0=params['K_in_0'],
            logc_in_1=params['logc_in_1'],
            K_in_1=params['K_in_1'],
            logc_in_2=params['logc_in_2'],
            K_in_2=params['K_in_2'],
            logc_in_3=params['logc_in_3'],
            K_in_3=params['K_in_3'],
            logc_in_4=params['logc_in_4'],
            K_in_4=params['K_in_4'],
            logc_in_5=params['logc_in_5'],
            K_in_5=params['K_in_5'],
            logc_in_6=params['logc_in_6'],
            K_in_6=params['K_in_6'],
            conv_type=3,
        )
        y_bits = result['tri_result']
        y_val = bits32_to_float(y_bits)
    except Exception:
        print("Error in _process_single_value")
        x_val = bits32_to_float(x_bits)
        y_val = 1.0 / (1.0 + math.exp(-x_val))
    return y_val


def float_to_bits32(value: float) -> str:
    """Convert float to 32-bit binary string (IEEE 754)."""
    b = struct.pack('>f', value)
    u = int.from_bytes(b, byteorder='big')
    return format(u & 0xFFFFFFFF, '032b')


def bits32_to_float(bits: str) -> float:
    """Convert 32-bit binary string to float (IEEE 754)."""
    u = int(bits, 2)
    b = u.to_bytes(4, byteorder='big')
    return struct.unpack('>f', b)[0]


def load_lns_params(param_file: str = 'parameter.json', func_name: str = 'sigmoid'):
    """Load LNS parameters from JSON (script dir, or LNS_Top/parameter.json)."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    param_path = os.path.join(script_dir, param_file)
    if not os.path.exists(param_path):
        param_path = os.path.join(script_dir, 'LNS_Top', os.path.basename(param_file))
    if not os.path.exists(param_path):
        raise FileNotFoundError(f"Parameter file not found: {param_path}")
    with open(param_path, 'r', encoding='utf-8') as f:
        params_all = json.load(f)
    if func_name not in params_all:
        raise KeyError(f"Function '{func_name}' not found in {param_path}. Available: {list(params_all.keys())}")
    return params_all[func_name]


class LNSApproxSigmoid(nn.Module):
    """
    Replace nn.Sigmoid using LNS_Top approximation.
    - Compute on CPU via LNS pipeline
    - Move results back to the original device
    - Outside [-16, 16]: clamp to 0/1
    """
    def __init__(self, num_workers: int = _DEFAULT_NUM_WORKERS):
        super().__init__()
        params = load_lns_params('parameter.json', 'sigmoid_A_float')
        self.y_in = params['y_in']
        self.n = params['n']
        self.float_flag = params['float_flag']
        self.break_points_in = params['break_points_in']
        self.TRG = params['TRG']
        self.VEC = params['VEC']
        self.qi = params['qi']
        self.Div = params['Div']
        self.TRi = params['TRi']
        self.power = params['power']
        self.bias_sel = params['bias_sel']
        self.constant_bias_in = params['constant_bias_in']
        self.logc_in_0 = params['logc_in_0']
        self.K_in_0 = params['K_in_0']
        self.logc_in_1 = params['logc_in_1']
        self.K_in_1 = params['K_in_1']
        self.logc_in_2 = params['logc_in_2']
        self.K_in_2 = params['K_in_2']
        self.logc_in_3 = params['logc_in_3']
        self.K_in_3 = params['K_in_3']
        self.logc_in_4 = params['logc_in_4']
        self.K_in_4 = params['K_in_4']
        self.logc_in_5 = params['logc_in_5']
        self.K_in_5 = params['K_in_5']
        self.logc_in_6 = params['logc_in_6']
        self.K_in_6 = params['K_in_6']

        self.num_workers = max(1, int(num_workers))
        self._params_dict = {
            'y_in': self.y_in,
            'n': self.n,
            'float_flag': self.float_flag,
            'break_points_in': self.break_points_in,
            'TRG': self.TRG,
            'VEC': self.VEC,
            'qi': self.qi,
            'Div': self.Div,
            'TRi': self.TRi,
            'power': self.power,
            'bias_sel': self.bias_sel,
            'constant_bias_in': self.constant_bias_in,
            'logc_in_0': self.logc_in_0,
            'K_in_0': self.K_in_0,
            'logc_in_1': self.logc_in_1,
            'K_in_1': self.K_in_1,
            'logc_in_2': self.logc_in_2,
            'K_in_2': self.K_in_2,
            'logc_in_3': self.logc_in_3,
            'K_in_3': self.K_in_3,
            'logc_in_4': self.logc_in_4,
            'K_in_4': self.K_in_4,
            'logc_in_5': self.logc_in_5,
            'K_in_5': self.K_in_5,
            'logc_in_6': self.logc_in_6,
            'K_in_6': self.K_in_6,
        }
        self._cache = {}

    def _get_pool(self):
        global _process_pool
        if self.num_workers <= 1:
            return None
        lock = _init_process_pool_lock()
        if _process_pool is None:
            with lock:
                if _process_pool is None:
                    _process_pool = ProcessPoolExecutor(max_workers=self.num_workers)
        return _process_pool

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        original_device = x.device
        # x_cpu = x.detach().to('cpu')
        x_cpu = x
        print("x_cpu.shape:", x_cpu.shape)
        out_cpu = torch.zeros_like(x_cpu)

        # Regions
        region_lo = x_cpu < -16.0
        region_hi = x_cpu > 16.0
        region_mid = (~region_lo) & (~region_hi)

        out_cpu[region_lo] = 0.0
        out_cpu[region_hi] = 1.0

        if region_mid.any():
            flat_in = x_cpu.reshape(-1)
            flat_out = out_cpu.reshape(-1)
            mid_idx = torch.nonzero(region_mid.flatten(), as_tuple=False).squeeze(-1)
            if mid_idx.dim() == 0:
                mid_idx = mid_idx.unsqueeze(0)

            if mid_idx.numel() > 0:
                pending = {}
                for idx in mid_idx.tolist():
                    x_val = float(flat_in[idx].item())
                    x_bits = float_to_bits32(x_val)
                    cached = self._cache.get(x_bits)
                    if cached is not None:
                        flat_out[idx] = cached
                    else:
                        pending.setdefault(x_bits, []).append(idx)

                if pending:
                    pending_items = list(pending.items())
                    args_list = [(bits, self._params_dict) for bits, _ in pending_items]
                    if self.num_workers <= 1:
                        results_iter = map(_process_single_value, args_list)
                    else:
                        pool = self._get_pool()
                        results_iter = pool.map(_process_single_value, args_list)

                    for (bits, indices), result_val in zip(pending_items, results_iter):
                        y_val = float(result_val)
                        self._cache[bits] = y_val
                        for idx in indices:
                            flat_out[idx] = y_val

                # Simple cache size guard to avoid unbounded growth
                if len(self._cache) > 100000:
                    self._cache.clear()

        return out_cpu.to(original_device)

    def __del__(self):
        global _process_pool
        if _process_pool is not None:
            lock = _init_process_pool_lock()
            with lock:
                if _process_pool is not None:
                    _process_pool.shutdown(wait=False)
                    _process_pool = None


def replace_sigmoid_with_lns(model: nn.Module, num_workers: int = _DEFAULT_NUM_WORKERS) -> nn.Module:
    """Recursively replace nn.Sigmoid by LNSApproxSigmoid."""
    for name, module in model.named_children():
        if isinstance(module, nn.Sigmoid):
            setattr(model, name, LNSApproxSigmoid(num_workers=num_workers))
        else:
            replace_sigmoid_with_lns(module, num_workers=num_workers)
    return model


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
        'LNSApproxSigmoid': RunningMinMax(),
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
        elif isinstance(m, LNSApproxSigmoid):
            handles.append(m.register_forward_hook(make_hook('LNSApproxSigmoid')))

    return stats, handles


def load_imagenet_val(val_dir, batch_size=64, workers=4, device=None):
    """加载ImageNet验证集
    
    Args:
        val_dir: 验证集目录
        batch_size: 批次大小
        workers: 数据加载进程数
        device: torch设备，用于自动设置pin_memory（CUDA时启用，CPU时禁用）
    """
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
    
    # 根据设备类型自动设置pin_memory
    # pin_memory只在CUDA设备上有意义，CPU模式下必须为False
    if device is None:
        pin_memory = False
    else:
        pin_memory = (device.type == 'cuda')
    
    val_loader = torch.utils.data.DataLoader(
        val_dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=workers,
        pin_memory=pin_memory
    )
    
    print(f'验证集大小: {len(val_dataset)}')
    print(f'类别数量: {len(val_dataset.classes)}')
    print(f'pin_memory: {pin_memory} (设备类型: {device.type if device else "未指定"})')
    
    return val_loader


def validate_model(model, val_loader, criterion, device):
    """验证模型 - 使用CPU多核加速"""
    model.eval()
    
    # 启用TorchScript JIT编译以优化CPU推理（可选，但可能提升性能）
    # 注意：如果模型包含自定义操作，可能需要调整
    try:
        # 使用torch.jit.optimize_for_inference可以进一步优化
        if device.type == 'cpu':
            # 确保使用多线程
            torch.set_num_threads(torch.get_num_threads())
    except:
        print("Error in validate_model")
        pass
    
    running_loss = 0.0
    correct_top1 = 0
    correct_top5 = 0
    total = 0
    
    with torch.no_grad():
        for images, labels in tqdm(val_loader, desc='验证中'):
            images = images.to(device, non_blocking=True)
            labels = labels.to(device, non_blocking=True)
            
            # 前向传播 - PyTorch CPU后端会自动使用多核
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
    # ========== CPU多核加速配置 ==========
    # 获取CPU核心数
    num_cores = mp.cpu_count()
    print(f'检测到CPU核心数: {num_cores}')
    
    # 设置PyTorch使用的线程数（用于矩阵运算等）
    # 建议设置为物理核心数，避免超线程导致的性能下降
    num_threads = num_cores
    torch.set_num_threads(num_threads)
    # 设置操作间并行度（用于并行执行多个独立操作）
    torch.set_num_interop_threads(min(4, num_cores // 2))  # 通常设置为2-4
    
    # 设置环境变量以控制底层库的线程数
    # MKL (Intel Math Kernel Library)
    os.environ['MKL_NUM_THREADS'] = str(num_threads)
    os.environ['MKL_DOMAIN_NUM_THREADS'] = f'MKL_BLAS={num_threads}'
    # OpenMP
    os.environ['OMP_NUM_THREADS'] = str(num_threads)
    # NumPy (如果使用)
    os.environ['NUMEXPR_NUM_THREADS'] = str(num_threads)
    
    print(f'PyTorch线程数: {torch.get_num_threads()}')
    print(f'PyTorch操作间并行度: {torch.get_num_interop_threads()}')
    print(f'MKL线程数: {os.environ.get("MKL_NUM_THREADS", "未设置")}')
    print(f'OMP线程数: {os.environ.get("OMP_NUM_THREADS", "未设置")}')
    
    # 设置设备
    # device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    device = torch.device('cpu')
    print(f'使用设备: {device}')
    
    # 设置路径 - 验证集在EfficientNet文件夹下
    val_dir = '/home/gbzou/EfficientNet/ILSVRC2012_img_val'
    
    # 加载验证集
    # num_workers: 数据加载的进程数，建议设置为CPU核心数的1/4到1/2
    # 注意：数据加载进程和模型计算线程会共享CPU资源，需要平衡
    data_workers = max(1, min(8, num_cores // 4))  # 避免过多进程导致资源竞争
    print('\n正在加载验证集...')
    print(f'数据加载进程数: {data_workers}')
    val_loader = load_imagenet_val(val_dir, batch_size=128, workers=data_workers, device=device)
    
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
    
    # 替换Sigmoid为LNS近似
    print('\n正在替换Sigmoid为LNS近似...')
    model = replace_sigmoid_with_lns(model)
    print('Sigmoid替换完成')
    
    model = model.to(device)
    
    # 注册激活函数输入范围统计
    act_stats, act_handles = register_activation_input_range_hooks(model)
    print('\n已启用激活函数输入范围统计 (ReLU, Sigmoid, LNSApproxSigmoid)')
    
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
    for h in act_handles:
        h.remove()
    
    # 打印激活输入范围统计
    print('\n激活函数输入范围统计 (基于整个验证集的前向过程):')
    for kind, mm in act_stats.items():
        if mm.num_updates == 0:
            print(f'- {kind}: 无数据')
        else:
            print(f'- {kind}: min={mm.global_min:.6f}, max={mm.global_max:.6f}, updates={mm.num_updates}')


if __name__ == '__main__':
    main()

    