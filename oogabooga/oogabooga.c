

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

#ifndef DO_ZERO_INITIALIZATION
	#define DO_ZERO_INITIALIZATION 1
#endif

#ifndef ENABLE_SIMD
	#define ENABLE_SIMD 1
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

Allocator get_heap_allocator();

// Custom allocators for lodepng
Allocator lodepng_allocator = {0};
void* lodepng_malloc(size_t size) {
	if (lodepng_allocator.proc) return alloc(lodepng_allocator, size);
	return alloc(get_heap_allocator(), size);
}
void* lodepng_realloc(void* ptr, size_t new_size) {
	if (lodepng_allocator.proc) return lodepng_allocator.proc(new_size, ptr, ALLOCATOR_REALLOCATE, lodepng_allocator.data);
	return get_heap_allocator().proc(new_size, ptr, ALLOCATOR_REALLOCATE, get_heap_allocator().data);
}
void lodepng_free(void* ptr) {
	if (!ptr) return;
	if (lodepng_allocator.proc) dealloc(lodepng_allocator, ptr);
	else dealloc(get_heap_allocator(), ptr);
}
#define LODEPNG_NO_COMPILE_ALLOCATORS
//#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_NO_COMPILE_ENCODER
// One day I might write my own png decoder so we don't even need this
#include "third_party/lodepng.h" 
#include "third_party/lodepng.c"




/////

#include "cpu.c"

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


// #Cleanup #Temporary
// This should be somewhere else
String_Builder _profile_output = {0};
bool profiler_initted = false;
Spinlock *_profiler_lock = 0;
void dump_profile_result() {
	File file = os_file_open("google_trace.json", O_CREATE | O_WRITE);
	
	os_file_write_string(file, STR("["));
	os_file_write_string(file, _profile_output.result);
	os_file_write_string(file, STR("{}]"));
	
	os_file_close(file);
	
	log_verbose("Wrote profiling result to google_trace.json");
}
void _profiler_report_time_cycles(string name, u64 count, u64 start) {
	if (!profiler_initted) {
		_profiler_lock = os_make_spinlock(get_heap_allocator());
		profiler_initted = true;
		
		string_builder_init_reserve(&_profile_output, 1024*1000, get_heap_allocator());	
		
	}
	
	os_spinlock_lock(_profiler_lock);
	
	string fmt = STR("{\"cat\":\"function\",\"dur\":%.3f,\"name\":\"%s\",\"ph\":\"X\",\"pid\":0,\"tid\":%zu,\"ts\":%lld},");
	string_builder_print(&_profile_output, fmt, (float64)count*1000, name, GetCurrentThreadId(), start*1000);
	
	os_spinlock_unlock(_profiler_lock);
}
#if ENABLE_PROFILING
#define tm_scope_cycles(name) \
    for (u64 start_time = os_get_current_cycle_count(), end_time = start_time, elapsed_time = 0; \
         elapsed_time == 0; \
         elapsed_time = (end_time = os_get_current_cycle_count()) - start_time, _profiler_report_time_cycles(STR(name), elapsed_time, start_time))
#define tm_scope_cycles_var(name, var) \
    for (u64 start_time = os_get_current_cycle_count(), end_time = start_time, elapsed_time = 0; \
         elapsed_time == 0; \
         elapsed_time = (end_time = os_get_current_cycle_count()) - start_time, var=elapsed_time)
#define tm_scope_cycles_accum(name, var) \
    for (u64 start_time = os_get_current_cycle_count(), end_time = start_time, elapsed_time = 0; \
         elapsed_time == 0; \
         elapsed_time = (end_time = os_get_current_cycle_count()) - start_time, var+=elapsed_time)
#else
	#define tm_scope_cycles(...)
	#define tm_scope_cycles_var(...)
	#define tm_scope_cycles_accum(...)
#endif

// I hope whoever caused this @ microsoft is fired.
#ifdef near
#undef near
#endif
#ifdef far
#undef far 
#endif

#include "random.c"
#include "color.c"

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
	temp = get_initialization_allocator();
	init_cpu_specific();
	Cpu_Capabilities features = query_cpu_capabilities();
	os_init(program_memory_size);
	heap_init();
	temporary_storage_init();
	gfx_init();
	log_verbose("CPU has sse1: %cs", features.sse1 ? "true" : "false");
	log_verbose("CPU has sse2: %cs", features.sse2 ? "true" : "false");
	log_verbose("CPU has sse3: %cs", features.sse3 ? "true" : "false");
	log_verbose("CPU has ssse3: %cs", features.ssse3 ? "true" : "false");
	log_verbose("CPU has sse41: %cs", features.sse41 ? "true" : "false");
	log_verbose("CPU has sse42: %cs", features.sse42 ? "true" : "false");
	log_verbose("CPU has avx: %cs", features.avx ? "true" : "false");
	log_verbose("CPU has avx2: %cs", features.avx2 ? "true" : "false");
	log_verbose("CPU has avx512: %cs", features.avx512 ? "true" : "false");
}

#ifndef INITIAL_PROGRAM_MEMORY_SIZE
	#define INITIAL_PROGRAM_MEMORY_SIZE (max((1024ULL * 1024ULL * 100ULL), TEMPORARY_STORAGE_SIZE*2))
#endif
#ifndef RUN_TESTS
	#define RUN_TESTS 0
#endif


int ENTRY_PROC(int argc, char **argv);

int main(int argc, char **argv) {

	printf("Ooga booga program started\n");
	oogabooga_init(INITIAL_PROGRAM_MEMORY_SIZE); 
	
	assert(main != ENTRY_PROC, "You've ooga'd your last booga");
	
	// This can be disabled in build.c
	#if RUN_TESTS
		oogabooga_run_tests();
	#endif
	
	int code = ENTRY_PROC(argc, argv);
	
#if ENABLE_PROFILING
	
	dump_profile_result();
	
#endif
	
	printf("Ooga booga program exit with code %i\n", code);
	
	return code;
}

