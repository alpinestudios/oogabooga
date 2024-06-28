
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

int main(int argc, char **argv) {
	printf("Ooga booga program started\n");
	oogabooga_init(INITIAL_PROGRAM_MEMORY_SIZE); 
	
	// This can be disabled in build.c
	#if RUN_TESTS
		oogabooga_run_tests();
	#endif
	
	int code = oogabooga_main(argc, argv);
	
	printf("Ooga booga program exit with code %i\n", code);
	
	return code;
}