cmake_minimum_required(VERSION 3.5)
set(PROJECT_NAME llvm-cdfg)
project(${PROJECT_NAME} VERSION 0.1.0)

set(MAIN_PATH ${PROJECT_SOURCE_DIR})
configure_file(llvm-pass/include/py_path.h.in ../llvm-pass/include/py_path.h)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Release")

set(LLVM_INCLUDE_DIRS "/home/dai-dirk/llvm-10/llvm-10.0.0-built/usr/local/include")
set(LLVM_LIBRARY_DIRS "/home/dai-dirk/llvm-10/llvm-10.0.0-built/usr/local/lib")

# set(LLVM_INCLUDE_DIRS "/home/jhlou/projects/FDRA-app-compiler/llvm-project-llvmorg-10.0.0/build/include")
# set(LLVM_LIBRARY_DIRS "/home/jhlou/projects/FDRA-app-compiler/llvm-project-llvmorg-10.0.0/build/lib")
#find_package(LLVM 10 REQUIRED)
# add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

#include_directories(
#  "/usr/include/llvm-10"
#  "/usr/include/llvm-c-10"
#)

#link_directories(
#  "/usr/lib/llvm-10/lib"
#)

add_subdirectory(llvm-pass)  
