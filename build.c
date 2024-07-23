

///
// Build config stuff

#define INITIAL_PROGRAM_MEMORY_SIZE MB(5)

typedef struct Context_Extra
{
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

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

// This is a minimal starting point for new projects. Copy & rename to get started
// #include "oogabooga/examples/minimal_game_loop.c"

// #include "oogabooga/examples/text_rendering.c"
// #include "oogabooga/examples/custom_logger.c"
// #include "oogabooga/examples/renderer_stress_test.c"
// #include "oogabooga/examples/tile_game.c"
// #include "oogabooga/examples/audio_test.c"
// #include "oogabooga/examples/custom_shader.c"

// This is where you swap in your own project!
#include "globals.c"
#include "entity.c"
#include "ui.c"
#include "utils.c"
#include "game.c"
