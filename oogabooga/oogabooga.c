
#define DEBUG 0
#define VERY_DEBUG 1
#define RELEASE 2

#if !defined(CONFIGURATION)

	#if defined(NDEBUG)
		#define CONFIGURATION RELEASE
	#else
		#define CONFIGURATION DEBUG
	#endif

#endif

#ifndef ENTRY_PROC
	#define ENTRY_PROC entry
#endif

#define WINDOWS 0
#define LINUX   1
#define MACOS   2

// This needs to be included before dependencies
#include "base.c"


///
///
// Dependencies
///

#include <math.h>

// Custom allocators for lodepng
void* lodepng_malloc(size_t size) {
	return context.allocator.proc((u64)size, 0, ALLOCATOR_ALLOCATE);
}
void* lodepng_realloc(void* ptr, size_t new_size) {
	return context.allocator.proc((u64)new_size, ptr, ALLOCATOR_REALLOCATE);
}
void lodepng_free(void* ptr) {
	if (!ptr) return;
	context.allocator.proc(0, ptr, ALLOCATOR_DEALLOCATE);
}
#define LODEPNG_NO_COMPILE_ALLOCATORS
//#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_NO_COMPILE_ENCODER
// One day I might write my own png decoder so we don't even need this
#include "third_party/lodepng.h" 
#include "third_party/lodepng.c"
/*
	To decode a png file:
	
	// 1. Read to memory
	string path = fixed_string("my_image.png");
	string png;
	os_read_entire_file(path, &png);
	
	// 2. Decode
	u8 *image;
	u32 width;
	u32 height;
	u32 error = lodepng_decode32(&image, &width, &height, png.data, png.count);
	
	// Cleanup
	dealloc_string(png);
	dealloc(image);
*/



/////



#ifdef _WIN32
	#include <Windows.h>
	#define TARGET_OS WINDOWS
	#define OS_PATHS_HAVE_BACKSLASH 1
#elif defined(__linux__)
	// Include whatever #Incomplete #Portability
	#define TARGET_OS LINUX
	#error "Linux is not supported yet";
	#define OS_PATHS_HAVE_BACKSLASH 0
#elif defined(__APPLE__) && defined(__MACH__)
	// Include whatever #Incomplete #Portability
	#define TARGET_OS MACOS
	#error "Mac is not supported yet";
	#define OS_PATHS_HAVE_BACKSLASH 1
#else
	#error "Current OS not supported!";
#endif

#define GFX_RENDERER_D3D11  0
#define GFX_RENDERER_VULKAN 1
#define GFX_RENDERER_METAL  2

#ifndef GFX_RENDERER
// #Portability
	#if TARGET_OS == WINDOWS
		#define GFX_RENDERER GFX_RENDERER_D3D11
	#elif TARGET_OS == LINUX
		#define GFX_RENDERER GFX_RENDERER_VULKAN
	#elif TARGET_OS == MACOS
		#define GFX_RENDERER GFX_RENDERER_METAL
	#endif
#endif


#include "string.c"
#include "unicode.c"
#include "string_format.c"
#include "path_utils.c"

#include "linmath.c"
#include "os_interface.c"
#include "gfx_interface.c"

// I hope whoever caused this @ microsoft is fired.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#include "random.c"


#include "memory.c"
#include "input.c"

#include "drawing.c"

// #Portability
#if GFX_RENDERER == GFX_RENDERER_D3D11
	#include "gfx_impl_d3d11.c"
#elif GFX_RENDERER == GFX_RENDERER_VULKAN
	#error "We only have a D3D11 renderer at the moment"
#elif GFX_RENDERER == GFX_RENDERER_METAL
	#error "We only have a D3D11 renderer at the moment"
#elif GFX_RENDERER == GFX_RENDERER_LEGACY_OPENGL
	#include "gfx_impl_legacy_opengl.c"
#else
	#error "Unknown renderer GFX_RENDERER defined"
#endif

#if TARGET_OS == WINDOWS
	#include "os_impl_windows.c"
#elif TARGET_OS == LINUX

#elif TARGET_OS == MACOS

#endif

#include "tests.c"


void oogabooga_init(u64 program_memory_size) {
	context.logger = default_logger;
	os_init(program_memory_size);
	gfx_init();
	heap_init();
	temporary_storage_init();
}

#ifndef INITIAL_PROGRAM_MEMORY_SIZE
	#define INITIAL_PROGRAM_MEMORY_SIZE (1024ULL * 1024ULL * 100ULL) // Start with 100mb program memory
#endif
#ifndef RUN_TESTS
	#define RUN_TESTS 0
#endif


int ENTRY_PROC(int argc, char **argv);

int main(int argc, char **argv) {
	context.allocator.proc = initialization_allocator_proc;
	oogabooga_init(INITIAL_PROGRAM_MEMORY_SIZE); 
	printf("Ooga booga program started\n");
	
	// This can be disabled in build.c
	#if RUN_TESTS
		oogabooga_run_tests();
	#endif
	
	int code = ENTRY_PROC(argc, argv);
	
	printf("Ooga booga program exit with code %i\n", code);
	
	return code;
}

