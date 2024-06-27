@echo off
rmdir /S /Q build
mkdir build

pushd build

cl ../main.cpp /Zi

popd