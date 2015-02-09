rm -r build/
mkdir -p build/
cd build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
make clean
make
cp src/circlesim ..
