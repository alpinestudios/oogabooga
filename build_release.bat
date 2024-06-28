@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

cl ../../build.c /Zi /Fecgame.exe /Ox /DRELEASE /MD /std:c11

popd
popd