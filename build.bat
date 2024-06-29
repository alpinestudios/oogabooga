@echo off
rmdir /S /Q build
mkdir build

pushd build

cl.exe /Fe:cgame.exe ..\build.c /Od /std:c11 /W4 /wd4273 /wd4018 /wd4100 /Zi gdi32.lib user32.lib opengl32.lib

popd