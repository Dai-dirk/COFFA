export LLVM_HOME=/home/dai-dirk/llvm-10/llvm-10.0.0-built/usr/local/bin
export PATH=$LLVM_HOME:$PATH

clang -D CGRA_COMPILER -target i386-unknown-linux-gnu -c -emit-llvm -mllvm -polly -O2 -fno-tree-vectorize -fno-unroll-loops $1.c -S -o $1.ll

opt  -mem2reg -memdep -memcpyopt -lcssa -loop-simplify -licm -loop-deletion -indvars -loop-simplifycfg -simplifycfg -mergereturn -indvars $1.ll -S -o $1_gvn.ll

opt -load /home/dai-dirk/chipyard/generators/fusion_mem_fp/app-compiler/app-compiler-main/build/llvm-pass/libCDFGPass.so -fn $2 -cdfg $1_gvn.ll -S -o $1_cdfg.ll
