

///
// Build config stuff

#define RUN_TESTS 0

// This is only for people developing oogabooga!
#define OOGABOOGA_DEV 1

#define ENABLE_PROFILING 0

#define INITIAL_PROGRAM_MEMORY_SIZE MB(5)

// When we need very debug
// #define CONFIGURATION VERY_DEBUG

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#define GFX_RENDERER GFX_RENDERER_D3D11

// This defaults to "entry", but we can set it to anything (except "main" or other existing proc names"
#define ENTRY_PROC entry

#include "oogabooga/oogabooga.c"


//
// Comment & Uncomment to swap projects

#include "oogabooga/examples/renderer_stress_test.c"
// #include "oogabooga/examples/minimal_game_loop.c"
// #include "entry_randygame.c"