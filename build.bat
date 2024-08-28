@echo off
if not exist build (
	mkdir build
)

pushd build

clang -g -fuse-ld=lld  -o cgame.exe ../build.c -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -lkernel32 -lgdi32 -luser32 -lruntimeobject -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -lole32 -lshcore -lavrt -lksuser -ldbghelp -femit-all-decls

popd