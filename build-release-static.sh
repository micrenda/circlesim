rm -rf build/
gyp build.gyp --depth . --generator-output=build -DLINKING=static -DTARGET=release
cd build/
make
cp out/Default/circlesim ..
