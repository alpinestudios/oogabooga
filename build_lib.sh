#!/bin/bash

set -x

pushd lib

clang -c ../oogabooga/oogabooga.c -o oogabooga.obj -DOOGABOOGA_BUILD_SHARED_LIBRARY=1
llvm-lib /out:oogabooga.lib oogabooga.obj

popd
