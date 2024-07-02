@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

clang -o cgame.exe ../../build.c -Ofast -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -lgdi32 -luser32  -lwinmm -ld3d11 -ldxguid -ld3dcompiler

popd
popd