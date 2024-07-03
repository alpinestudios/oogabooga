@echo off
rmdir /S /Q build
mkdir build

pushd build

clang -g -o cgame.exe ../build.c -O0 -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -lgdi32 -luser32  -lwinmm -ld3d11 -ldxguid -ld3dcompiler -mavx2 -mavx512f -msse4.1 -msse2

popd