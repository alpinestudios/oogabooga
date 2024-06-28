@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

cl ../../main.c /Zi /Fecgame.exe /Ox /DRELEASE /std:c11

popd
popd