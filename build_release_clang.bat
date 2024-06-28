@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

clang -o cgame.exe ../../build.c -Ofast -DRELEASE -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter

popd
popd