rm -rf build/
gyp build.gyp --depth . --generator-output=build -DLINKING=dynamic -DTARGET=debug
cd build/
make circlesim
cp out/Default/circlesim ..
