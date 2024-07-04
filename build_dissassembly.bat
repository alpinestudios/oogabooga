
@echo off
if exist build/dissassembly (
  rmdir /s /q build
)
if not exist build  (
	mkdir build
)

pushd build

mkdir dissassembly
pushd dissassembly

clang -o cgame.asm ../../build.c -Ofast -std=c11 -Wextra -Wno-incompatible-library-redeclaration  -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -Wno-deprecated-declarations  -finline-functions -ffast-math -fno-math-errno -funsafe-math-optimizations -freciprocal-math -ffinite-math-only -fassociative-math -fno-signed-zeros -fno-trapping-math -ftree-vectorize  -fomit-frame-pointer -funroll-loops -fno-rtti -fno-exceptions -S -masm=intel

popd
popd