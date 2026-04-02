import numpy as np
import matplotlib.pyplot as plt
from piecewise_cheb_fitter import PiecewiseChebFitter
import sys
import time

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Error: python parse_file.py <filename>")
        sys.exit(1)

    chebfitter = PiecewiseChebFitter(sys.argv[1])
    config = chebfitter.config
    start = time.time()
    cheb_coeffs, poly_coeffs = chebfitter.fit()
    end = time.time()
    execute_time = end - start

    chebfitter.print_result(execute_time)
    chebfitter.print_coeff_breakpoints()
    chebfitter.save_result(execute_time)
    chebfitter.plot_result()

