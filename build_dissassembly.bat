
@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

clang -o cgame.asm ../../build.c -Ofast -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -ffast-math -funroll-loops -finline-functions -fvectorize -fslp-vectorize -fomit-frame-pointer -fno-exceptions -fno-rtti -S -masm=intel

popd
popd