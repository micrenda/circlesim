rm -rf build/
gyp build.gyp --depth . --generator-output=build -DLINKING=dynamic -DTARGET=release
cd build/
make viewer
cp out/Default/viewer    ..
