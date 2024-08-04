#!/bin/bash

set -x

pushd lib

clang -c ../oogabooga/oogabooga.c -o oogabooga.obj -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls
llvm-lib /out:oogabooga.lib oogabooga.obj

popd
