# build
mkdir -p build && cd build
cmake ..
# cmake -DCMAKE_BUILD_TYPE=Release ..
make -j 16

# clean
# make -C build clean