rm -rf build/
gyp build.gyp --depth . --generator-output=build -DLINKING=dynamic -DTARGET=debug
cd build/
make viewer
cp out/Default/viewer    ..
