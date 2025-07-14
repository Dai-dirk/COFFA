# set env for built LLVM
export LLVM_HOME=/home/dai-dirk/llvm-10.0.0-built/usr/local/bin
export PATH=$LLVM_HOME:$PATH

# export LLVM_HOME=/home/jhlou/projects/FDRA-app-compiler/llvm-project-llvmorg-10.0.0/build/bin
# export PATH=$LLVM_HOME:$PATH

# cmake & make
mkdir -p ./build
cd build
cmake ..
make all
