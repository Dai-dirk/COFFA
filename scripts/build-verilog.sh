#!/bin/bash

cd ../../sims/verilator/
make verilog CONFIG=FusionMemSoCConfig VERILATOR_THREADS=6 EXTRA_FIRRTL_OPTIONS=--emission-options=disableMemRandomization,disableRegisterRandomization

