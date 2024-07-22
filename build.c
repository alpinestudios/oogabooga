

///
// Build config stuff

#define INITIAL_PROGRAM_MEMORY_SIZE MB(5)

// You might want to increase this if you get a log warning saying the temporary storage was overflown.
// In many cases, overflowing the temporary storage should be fine since it just wraps back around and
// allocations made way earlier in the frame are likely not used anymore.
// This might however not always be the case, so it's probably a good idea to make sure you always have
// enough temporary storage for your game.
#define TEMPORARY_STORAGE_SIZE MB(2) 

// Enable VERY_DEBUG if you are having memory bugs to detect things like heap corruption earlier.
// #define VERY_DEBUG 1

typedef struct Context_Extra {
	int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

// This defaults to "entry", but we can set it to anything (except "main" or other existing proc names"
	#define ENTRY_PROC entry

// Ooga booga needs to be included AFTER configuration and BEFORE the program code
#include "oogabooga/oogabooga.c"


//
// Comment & Uncomment these to swap projects (only include one at a time)
//

// This is a minimal starting point for new projects. Copy & rename to get started
// #include "oogabooga/examples/minimal_game_loop.c"

// #include "oogabooga/examples/text_rendering.c"
// #include "oogabooga/examples/custom_logger.c"
#include "oogabooga/examples/renderer_stress_test.c"
// #include "oogabooga/examples/tile_game.c"
// #include "oogabooga/examples/audio_test.c"
// #include "oogabooga/examples/custom_shader.c"

// This is where you swap in your own project!
// #include "entry_yourepicgamename.c"
