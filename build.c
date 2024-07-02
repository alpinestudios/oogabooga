

///
// Build config stuff
#define RUN_TESTS 0

// This is only for people developing oogabooga!
#define OOGABOOGA_DEV 1

// When we need very debug
// #define CONFIGURATION VERY_DEBUG

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#define GFX_RENDERER GFX_RENDERER_D3D11

#include "oogabooga/oogabooga.c"


//
// Comment & Uncomment to swap projects

#include "entry_engine_test.c"
// #include "entry_minimal_example.c"
// #include "entry_randygame.c"