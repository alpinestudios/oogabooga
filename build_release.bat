@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

clang -o cgame.exe ../../build.c -Ofast -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -lgdi32 -luser32  -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -mavx2 -mavx512f -msse4.1 -msse2 -finline-functions -ffast-math -fno-math-errno -funsafe-math-optimizations -freciprocal-math -ffinite-math-only -fassociative-math -fno-signed-zeros -fno-trapping-math -ftree-vectorize  -fomit-frame-pointer -funroll-loops -fno-rtti -fno-exceptions

popd
popd