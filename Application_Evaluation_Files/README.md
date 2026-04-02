# Python environment (ISCA_AE / Software)

This document describes how to set up the Python environment for experiments under **ISCA_AE / Software** (DCT, DNN, language-model evaluation, etc.). It is aligned with environments such as **`test_isca`**: **Python 3.11** and **PyTorch 2.6 with CUDA 12.6**.

## Requirements

| Item | Recommendation |
|------|----------------|
| Python | **3.11.x** (e.g. 3.11.14) |
| CUDA (GPU) | Match the official PyTorch wheels, e.g. **12.6** (`cu126`) |
| Packaging | `conda` or `venv` + `pip` |

## Create an environment

### Conda

```bash
conda create -n test_isca python=3.11 -y
conda activate test_isca
```

### venv

```bash
python3.11 -m venv .venv
source .venv/bin/activate   # Linux / macOS
```

## Install PyTorch (GPU)

Install `torch`, `torchvision`, and `torchaudio` builds that match your CUDA version. See [PyTorch Get Started](https://pytorch.org/get-started/locally/) (e.g. **Stable · Linux · Pip · CUDA 12.6**):

```bash
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu126
```

For CPU-only, use the CPU install command from the same page.

## Install other dependencies

From the directory that contains `requirements.txt`:

```bash
pip install -r requirements.txt
```

If PyTorch is already installed, you may comment out the `torch` / `torchvision` / `torchaudio` lines in `requirements.txt` before running `pip install` to avoid version clashes.

The file lists **core** packages only, not a full `pip freeze`. To capture everything:

```bash
pip freeze > freeze-full.txt
```

---

## DCT evaluation (`DCT/`)

Run the scripts below from the `DCT` directory (or adjust paths as needed):

| Script | Role |
|--------|------|
| `python test_set12.py` | Baseline |
| `python test_set12_tylor.py` | Taylor approximation |
| `python test_set12_lns_float.py` | LNS / Xcore-style float evaluation |

Each run produces the corresponding evaluation result.

## DNN evaluation (`DNN/`)

The MobileNetV3 and SE-ResNet (SENet) code under `DNN/` is adapted from upstream projects: [xiaolai-sqlai/mobilenetv3](https://github.com/xiaolai-sqlai/mobilenetv3) and [moskomule/senet.pytorch](https://github.com/moskomule/senet.pytorch).

Before DNN evaluation, use the Python scripts under `Get_PT` to generate the `.pt` lookup tables for each nonlinear function, and place them in the corresponding `LNS` directories.

After the environment is configured:

1. Point dataset paths in the relevant scripts to your ImageNet (or other) data locations.
2. Run the Python entry point for the model you need (e.g. MobileNetV3, EfficientNet, SE-ResNet under each subfolder).

## NLP evaluation (`NLP/`)

Before NLP evaluation, use the Python scripts under `Get_PT` to generate the `.pt` lookup tables for each nonlinear function, and place them in the corresponding `LNS` directories.

After the environment is configured:

1. Run the appropriate `save_*.py` scripts to export models with activation functions replaced.
2. Run the corresponding `*.sh` launcher scripts for `lm-eval` (or your pipeline).

---

## Troubleshooting

- **CUDA OOM**: Reduce batch size or use a smaller model; with multi-GPU `DataParallel`, note per-device memory.
- **PyTorch / CUDA mismatch**: Check with `python -c "import torch; print(torch.version.cuda)"` and reinstall wheels that match your driver/CUDA.
