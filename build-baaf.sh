rm -r build/
mkdir -p build/
cd build/
cmake \
	-D CMAKE_BUILD_TYPE=Release \
	-D CMAKE_C_COMPILER=/data/mrenda/bin/gcc-4.9.2/usr/local/bin/g++ \
	-D CMAKE_CXX_COMPILER=/data/mrenda/bin/gcc-4.9.2/usr/local/bin/gcc \
	..
make clean
make 
cp src/circlesim ..
