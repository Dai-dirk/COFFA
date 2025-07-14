# Benchmarks
=======================

Here are the eight memory-intensive benchmarks that are used in the paper.

## File catalog

In each file:

 **benchmark_name.c:** the source code of the benchmark

 **benchmark_name_riscv.c:** the souce code for RISC-V CPU, including 1）using CPU for computation; 2) using FGRA for computation; 3) comparing the results of CPU and FGRA

 **affine.dot/json:** the CDFG of benchmark in DOT/JSON format

 **cgra_execute.c:** the FGRA calling function

 **mapped_adg.dot:** this figure shows how the CDFG is mapped to FGRA

 **mapped_dfg.dot:** this figure shows the scheduling results for each node

 
              


