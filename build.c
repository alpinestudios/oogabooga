

///
// Build config stuff
#define VERY_DEBUG 0
#define RUN_TESTS 0

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#define GFX_RENDERER GFX_RENDERER_D3D11

#define ENTRY_PROC start // This is "entry" by default but we're not like all the other girls so we call it start

#include "oogabooga/oogabooga.c"


// Includes for game goes here	
// ...

#include "entry.c"