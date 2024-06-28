
#include <stdio.h> // #Cleanup just using this for printf
#include <stdlib.h>

#include "base.c"

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