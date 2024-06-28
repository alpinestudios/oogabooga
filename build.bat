@echo off
rmdir /S /Q build
mkdir build

pushd build

cl ../build.c /Zi /Fecgame.exe /Od /DDEBUG /MD /DEBUG /std:c11

popd