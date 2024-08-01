#!/bin/bash

set -x

if [ -d "build" ]; then
  rm -rf build
fi

mkdir build

pushd build

clang ../entry_point.c -g -shared -o engine.dll -O0 -std=c11 -DOOGABOOGA_BUILD_SHARED_LIBRARY=1 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -fuse-ld=lld -lkernel32 -lgdi32 -luser32 -lruntimeobject -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -lole32 -lavrt -lksuser -ldbghelp -femit-all-decls -Xlinker --out-implib=engine.lib -Xlinker --machine=X64 -Xlinker --subsystem=console

clang ../build_launcher.c -g -o launcher.exe -O0 -std=c11 -DOOGABOOGA_LINK_EXTERNAL_INSTANCE=1  -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker --subsystem=console

clang ../game_loop.c -g -shared -o game.dll -O0 -std=c11 -DOOGABOOGA_LINK_EXTERNAL_INSTANCE=1 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker --subsystem=console

popd
