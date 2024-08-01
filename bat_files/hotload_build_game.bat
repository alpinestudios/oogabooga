@echo on

pushd build

clang ../build_game.c -g -shared -o game.dll -O0 -std=c11 -D_CRT_SECURE_NO_WARNINGS -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -luser32 -fuse-ld=lld -L. -lengine -Xlinker /SUBSYSTEM:CONSOLE


popd