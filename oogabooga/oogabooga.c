
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
	return context.allocator.proc((u64)new_size, 0, ALLOCATOR_REALLOCATE);
}
void lodepng_free(void* ptr) {
	context.allocator.proc(0, ptr, ALLOCATOR_DEALLOCATE);
}
#define LODEPNG_NO_COMPILE_ALLOCATORS
#include "third_party/lodepng.c"

/////

#define DEBUG 0
#define VERY_DEBUG 1
#define RELEASE 2

#if !defined(CONFIGURATION)

	#ifdef _DEBUG
		#define CONFIGURATION DEBUG
	#elif defined(NDEBUG)
		#define CONFIGURATION RELEASE
	#endif

#endif

#ifndef ENTRY_PROC
	#define ENTRY_PROC entry
#endif

#ifdef _WIN32
	#include <Windows.h>
	#define OS_WINDOWS
#elif defined(__linux__)
	// Include whatever #Incomplete #Portability
	#define OS_LINUX
	#error "Linux is not supported yet";
#elif defined(__APPLE__) && defined(__MACH__)
	// Include whatever #Incomplete #Portability
	#define OS_MAC
	#error "Mac is not supported yet";
#else
	#error "Current OS not supported!";
#endif

#define GFX_RENDERER_D3D11  0
#define GFX_RENDERER_VULKAN 1
#define GFX_RENDERER_METAL  2
#define GFX_RENDERER_LEGACY_OPENGL  3 // #Temporary #Cleanup

// #Temporary #Cleanup
#undef GFX_RENDERER
#define GFX_RENDERER GFX_RENDERER_LEGACY_OPENGL

#ifndef GFX_RENDERER
// #Portability
	#ifdef OS_WINDOWS
		#define GFX_RENDERER GFX_RENDERER_D3D11
	#elif defined (OS_LINUX)
		#define GFX_RENDERER GFX_RENDERER_VULKAN
	#elif defined (OS_MAC)
		#define GFX_RENDERER GFX_RENDERER_METAL
	#endif
#endif


#include "string.c"
#include "unicode.c"
#include "string_format.c"

#include "os_interface.c"

#include "random.c"

#include "linmath.c"

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
	#error "Unknown renderer defined in GFX_RENDERER"
#endif

#ifdef OS_WINDOWS
	#include "os_impl_windows.c"
#elif defined (OS_LINUX)

#elif defined (OS_MAC)

#endif

#include "tests.c"


void oogabooga_init(u64 program_memory_size) {
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