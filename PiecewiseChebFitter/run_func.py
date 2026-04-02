#!/usr/bin/env python3
"""
Batch runner for piecewise Chebyshev fitting using pre-generated configurations.
Usage: python3 run_func.py <function_name>
Example: python3 run_func.py sin
"""

import os
import sys
import subprocess
from pathlib import Path


def run_fitting(func_name):
    """Run fitting for all config files of the specified function."""
    config_dir = Path(f"config/{func_name}")
    result_dir = Path(f"result/{func_name}")
    fig_dir = Path(f"fig/{func_name}")
    
    # Check if config directory exists
    if not config_dir.exists():
        print(f"Error: Config directory not found: {config_dir}")
        return 1
    
    # Create necessary directories
    result_dir.mkdir(parents=True, exist_ok=True)
    fig_dir.mkdir(parents=True, exist_ok=True)
    
    # Find all JSON config files
    config_files = sorted(config_dir.glob("*.json"))
    
    if not config_files:
        print(f"Error: No JSON config files found in {config_dir}")
        return 1
    
    print(f"Running fitting for function: {func_name}")
    print(f"Config directory: {config_dir}")
    print(f"Result directory: {result_dir}")
    print(f"Figure directory: {fig_dir}")
    print(f"Found {len(config_files)} config files")
    print("=" * 60)
    
    total = len(config_files)
    success_count = 0
    failed_count = 0
    
    for idx, config_file in enumerate(config_files, 1):
        config_name = config_file.stem  # e.g., "sin_6_5_2.5" from "sin_6_5_2.5.json"
        log_file = result_dir / f"{config_name}.log"
        
        print(f"[{idx}/{total}] Processing: {config_file.name}")
        
        # Run main.py and redirect output to log file
        try:
            with open(log_file, 'w') as log:
                result = subprocess.run(
                    ["python3", "main.py", str(config_file)],
                    stdout=log,
                    stderr=log,
                    check=True
                )
            print(f"  ✓ Completed, log: {log_file.name}")
            success_count += 1
        except subprocess.CalledProcessError as e:
            print(f"  ✗ Failed with return code {e.returncode}, check log: {log_file.name}")
            failed_count += 1
        except Exception as e:
            print(f"  ✗ Error: {e}")
            failed_count += 1
        
        print()
    
    print("=" * 60)
    print(f"Summary for {func_name}:")
    print(f"  Total: {total}")
    print(f"  Success: {success_count}")
    print(f"  Failed: {failed_count}")
    
    return 0 if failed_count == 0 else 1


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 run_func.py <function_name>")
        print("Example: python3 run_func.py sin")
        return 1
    
    func_name = sys.argv[1]
    return run_fitting(func_name)


if __name__ == "__main__":
    sys.exit(main())
