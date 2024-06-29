@echo off
rmdir /S /Q build
mkdir build

pushd build

mkdir release
pushd release

cl.exe /Fe:cgame.exe ..\..\build.c /Ox /std:c11 /W4 /wd4273 /wd4018 /wd4100 gdi32.lib user32.lib opengl32.lib

popd
popd