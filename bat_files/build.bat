@echo off
if exist build (
  rmdir /s /q build
)
mkdir build

pushd build

clang -g -o cgame.exe ../build.c -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -lkernel32 -lgdi32 -luser32 -lruntimeobject -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -lole32 -lavrt -lksuser -ldbghelp -femit-all-decls

popd