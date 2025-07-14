# COFFA-Compiler
=======================

Here is the COFFA compiler with front-end and back-end tools.

## Dependencies

### LLVM-10.0.0 with Polly included

1. Download llvm source codes from https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-10.0.0.tar.gz
    
 Extract and change the directory name

```sh
    tar xvf llvmorg-10.0.0.tar.gz
    mv llvmorg-10.0.0 llvm-project-10.0.0
```

2. Build llvm

```sh
    mkdir llvm-10.0.0-built
    cd llvm-project-10.0.0
    mkdir build
    cd build
    cmake -DLLVM_ENABLE_PROJECTS='polly;clang' -G "Unix Makefiles" ../llvm
    # multi-thread consumes lots of memory, e.g. -j4 : 30G+
    make -j4
    # DESTDIR set install directory
    make install DESTDIR=/xxx/llvm/llvm-10.0.0-built
```

3. Set llvm env

```sh
    # add following env to .bashrc and then source ~/.bashrc.
    # or directly export the env
    export LLVM_HOME=/xxx/llvm/llvm-10.0.0-built/usr/local/bin
    export PATH=$LLVM_HOME:$PATH
    # or set the LLVM path in the CMakeLists.txt
    set(LLVM_INCLUDE_DIRS "/xxx/llvm-10.0.0-built/usr/local/include")
    set(LLVM_LIBRARY_DIRS "/xxx/llvm-10.0.0-built/usr/local/lib")
```
### CMake and C++-11

### Yosys

Please refer to https://yosyshq.net/yosys/documentation.html for Yosys installation.


## File catalog

### Front-end tool

**./front-end-tool/llvm-pass/src:** the source codes of the LLVM-based front-end tool

### Back-end tool

**./back-end-tool/src:** the source codes of the back-end tool

**./back-end-tool/Syn:** the lib and script for the Back-end tool calling Yosys
     

