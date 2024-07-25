@echo on
if exist build (
  rmdir /s /q build
)
mkdir build

pushd build

clang ../build_engine.c -g -shared -o engine.dll -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -fuse-ld=lld -lkernel32 -lgdi32 -luser32 -lruntimeobject -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -lole32 -lavrt -lksuser -ldbghelp -femit-all-decls -Xlinker /IMPLIB:engine.lib -Xlinker /MACHINE:X64 -Xlinker /SUBSYSTEM:CONSOLE

clang ../build_launcher.c -g -o launcher.exe -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker /SUBSYSTEM:CONSOLE

clang ../build_game.c -g -shared -o game.dll -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker /SUBSYSTEM:CONSOLE


popd