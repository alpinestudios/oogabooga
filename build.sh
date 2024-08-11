#!/bin/sh

if [ -d build ]; then
    rm -r build
fi

CC=x86_64-w64-mingw32-gcc
CFLAGS="-g -O0 -std=c11 --static -D_CRT_SECURE_NO_WARNINGS
        -Wextra -Wno-sign-compare -Wno-unused-parameter
        -lkernel32 -lgdi32 -luser32 -lruntimeobject
        -lwinmm -ld3d11 -ldxguid -ld3dcompiler 
        -lshlwapi -lole32 -lavrt -lksuser -ldbghelp"
SRC=../build.c
EXENAME=game.exe

mkdir build
cd build
$CC $SRC -o $EXENAME $CFLAGS
cd ..