
#if !defined(DEBUG) && !defined(RELEASE)

#ifdef _DEBUG
#define DEBUG
#elif defined(NDEBUG)
#define RELEASE
#endif

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

#include "base.c"

#include "string.c"

#include "os_interface.c"

#include "memory.c"




#ifdef OS_WINDOWS
	#include "os_impl_windows.c"
#elif defined (OS_LINUX)

#elif defined (OS_MAC)

#endif

#include "tests.c"


void oogabooga_init(u64 program_memory_size) {
	os_init(program_memory_size);
		heap_init();
	temporary_storage_init();
}

#ifndef INITIAL_PROGRAM_MEMORY_SIZE
	#define INITIAL_PROGRAM_MEMORY_SIZE (1024ULL * 1024ULL * 100ULL) // Start with 100mb program memory
#endif
#ifndef RUN_TESTS
	#define RUN_TESTS 0
#endif


int oogabooga_main(int argc, char **argv);

int main(int argc, char **argv) {
	context.allocator.proc = initialization_allocator_proc;
	oogabooga_init(INITIAL_PROGRAM_MEMORY_SIZE); 
	printf("Ooga booga program started\n");
	
	// This can be disabled in build.c
	#if RUN_TESTS
		oogabooga_run_tests();
	#endif
	
	int code = oogabooga_main(argc, argv);
	
	printf("Ooga booga program exit with code %i\n", code);
	
	return code;
}