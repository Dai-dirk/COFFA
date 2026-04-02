cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME fgra-compiler)
project(${PROJECT_NAME} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE "Debug")

set(MAIN_PATH ${PROJECT_SOURCE_DIR})
configure_file(include/dfg/python_path.h.in ../include/dfg/python_path.h)

file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)
# file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/graph/*.cpp ${CMAKE_SOURCE_DIR}/src/adg/*.cpp ${CMAKE_SOURCE_DIR}/src/dfg/*.cpp ${CMAKE_SOURCE_DIR}/src/ir/*.cpp ${CMAKE_SOURCE_DIR}/src/op/*.cpp)
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
find_package(PythonLibs REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PYTHON_LIBRARIES})
