#!/bin/bash

set -x

pushd build

clang ../game_loop.c -g -shared -o game.dll -O0 -std=c11 -DOOGABOOGA_LINK_EXTERNAL_INSTANCE=1 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker --subsystem=console

popd
