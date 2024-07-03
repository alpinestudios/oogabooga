

///
// Build config stuff

#define RUN_TESTS 0

// This is only for people developing oogabooga!
#define OOGABOOGA_DEV 1

#define ENABLE_PROFILING 0

// Requires CPU to support at least SSE1 but I will be very surprised if you find a system today which don't.
#define ENABLE_SIMD 1

#define INITIAL_PROGRAM_MEMORY_SIZE MB(5)

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#define GFX_RENDERER GFX_RENDERER_D3D11

// This defaults to "entry", but we can set it to anything (except "main" or other existing proc names"
#define ENTRY_PROC entry

// Ooga booga needs to be included AFTER configuration and BEFORE the program code
#include "oogabooga/oogabooga.c"

// #Volatile tutorial below
// #Volatile tutorial below
// #Volatile tutorial below

//
// Comment & Uncomment these to swap projects (only include one at a time)
//

// this is a minimal starting point for new projects. Copy & rename to get started
#include "oogabooga/examples/minimal_game_loop.c"

// An engine dev stress test for rendering
// #include "oogabooga/examples/renderer_stress_test.c"

// Randy's example game that he's building out as a tutorial for using the engine
// #include "entry_randygame.c"

// This is where you swap in your own project!
// #include "entry_yourepicgamename.c"
