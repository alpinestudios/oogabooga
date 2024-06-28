

///
// Build config stuff
#define VERY_DEBUG 0
#define RUN_TESTS 1

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#include "oogabooga/oogabooga.c"


// Includes for game goes here	
// ...

#include "main.c"