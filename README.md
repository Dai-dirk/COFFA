# COFFA: A Co-Design Framework for Fused-Grained Reconfigurable Architecture towards Efficient Irregular Loop Handling (Updating)

## Key Futures

COFFA is an open-source framework for a hybrid system with a RISC-V core and a fused-grained reconfigurable accelerator.


## File catalog:

**COFFA-Architecture:** The FGRA + RISC-V SoC modeling by Chisel.

**COFFA-Compiler:** The COFFA-Compiler includes 1) an LLVM-based front-end tool for CDFG generation and 2) a back-end tool for CDFG to FGRA mapping.

**FGRA-BO-DSE:** The BO-based DSE process for FGRA.

**Benchmark:** The irregular or memory-intensive benchmarks used in the paper.

**Generated_Arch:** The FGRA specification file (which describes the design parameters) and generated ADFs.

**Scripts:** The scripts for running different flows within Chipyard. 


All folders contain README files with introductions; the detailed instructions for running COFFA can be found in the COFFA manual [Manual](https://github.com/Dai-dirk/COFFA/blob/main/COFFA-Manual-1.0.pdf).

## Demo:
The Demo of the COFFA FPGA prototype is available at [Demo](https://youtu.be/9Y9i-jm0xQY?si=cCk1B8UQJmg_zFQZ). Special thanks to Rachel🐷 for her contributions to this video!

## Example flow
![demo.png](https://github.com/Dai-dirk/COFFA/blob/main/demo.png)



