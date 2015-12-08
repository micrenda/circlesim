rm -rf build/
gyp build.gyp --depth . --generator-output=build -DLINKING=dynamic -DTARGET=release
cd build/
make circlesim
cp out/Default/circlesim ..
