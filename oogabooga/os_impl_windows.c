

#define VIRTUAL_MEMORY_BASE ((void*)0x0000010000000000ULL)

void* heap_alloc(u64);
void heap_dealloc(void*);

void* heap_allocator_proc(u64 size, void *p, Allocator_Message message) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return heap_alloc(size);
			break;
		}
		case ALLOCATOR_DEALLOCATE: {
			heap_dealloc(p);
			return 0;
		}
	}
	return 0;
}

void os_init(u64 program_memory_size) {
	
	SYSTEM_INFO si;
    GetSystemInfo(&si);
	os.granularity = cast(u64)si.dwAllocationGranularity;
	os.page_size = cast(u64)si.dwPageSize;


	program_memory_mutex = os_make_mutex();
	os_grow_program_memory(program_memory_size);
	
	Allocator heap_allocator;
	
	heap_allocator.proc = heap_allocator_proc;
	heap_allocator.data = 0;
	
	context.allocator = heap_allocator;
	
	os.crt = os_load_dynamic_library(const_string("msvcrt.dll"));
	assert(os.crt != 0, "Could not load win32 crt library. Might be compiled with non-msvc? #Incomplete #Portability");
	os.crt_vprintf = (Crt_Vprintf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("vprintf"));
	assert(os.crt_vprintf, "Missing vprintf in crt");
	os.crt_printf = (Crt_Printf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("printf"));
	assert(os.crt_printf, "Missing printf in crt");
	os.crt_vsnprintf = (Crt_Vsnprintf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("vsnprintf"));
	assert(os.crt_vsnprintf, "Missing vsnprintf in crt");
	os.crt_memcpy = (Crt_Memcpy_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memcpy"));
	assert(os.crt_memcpy, "Missing memcpy in crt");
	os.crt_memcmp = (Crt_Memcmp_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memcmp"));
	assert(os.crt_memcmp, "Missing crt_memcmp in crt");
	os.crt_memset = (Crt_Memset_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memset"));
	assert(os.crt_memset, "Missing memset in crt");
}

bool os_grow_program_memory(u64 new_size) {
	os_lock_mutex(program_memory_mutex); // #Sync
	if (program_memory_size >= new_size) {
		os_unlock_mutex(program_memory_mutex); // #Sync
		return true;
	}

	
	
	bool is_first_time = program_memory == 0;
	
	if (is_first_time) {
		u64 aligned_size = (new_size+os.granularity) & ~(os.granularity);
		void* aligned_base = (void*)(((u64)VIRTUAL_MEMORY_BASE+os.granularity) & ~(os.granularity-1));

		u64 m = aligned_size & os.granularity;
		assert(m == 0);
		
		program_memory = VirtualAlloc(aligned_base, aligned_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (program_memory == 0) { 
			os_unlock_mutex(program_memory_mutex); // #Sync
			return false;
		}
		program_memory_size = aligned_size;
	} else {
		void* tail = (u8*)program_memory + program_memory_size;
		u64 m = ((u64)program_memory_size % os.granularity);
		assert(m == 0, "program_memory_size is not aligned to granularity!");
		m = ((u64)tail % os.granularity);
		assert(m == 0, "Tail is not aligned to granularity!");
		u64 amount_to_allocate = new_size-program_memory_size;
		amount_to_allocate = ((amount_to_allocate+os.granularity)&~(os.granularity-1));
		m = ((u64)amount_to_allocate % os.granularity);
		assert(m == 0, "amount_to_allocate is not aligned to granularity!");
		// Just keep allocating at the tail of the current chunk
		void* result = VirtualAlloc(tail, amount_to_allocate, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (result == 0) { 
			os_unlock_mutex(program_memory_mutex); // #Sync
			return false;
		}
		assert(tail == result, "It seems tail is not aligned properly. o nein");
		
		program_memory_size += amount_to_allocate;

		m = ((u64)program_memory_size % os.granularity);
		assert(m == 0, "program_memory_size is not aligned to granularity!");
	}

	
	
	os_unlock_mutex(program_memory_mutex); // #Sync
	return true;
}

Mutex_Handle os_make_mutex() {
	return CreateMutex(0, FALSE, 0);
}
void os_destroy_mutex(Mutex_Handle m) {
	CloseHandle(m);
}
void os_lock_mutex(Mutex_Handle m) {
	DWORD wait_result = WaitForSingleObject(m, INFINITE);
	
	switch (wait_result) {
        case WAIT_OBJECT_0:
            break;

        case WAIT_ABANDONED:
            break;

        default:
        	assert(false, "Unexpected mutex lock result");
            break;
    }
}
void os_unlock_mutex(Mutex_Handle m) {
	BOOL result = ReleaseMutex(m);
	assert(result, "Unlock mutex failed");
}

DWORD WINAPI win32_thread_invoker(LPVOID param) {
	Thread *t = (Thread*)param;
	temporary_storage_init();
	context = t->initial_context;
	t->proc(t);
	return 0;
}

Thread* os_make_thread(Thread_Proc proc) {
	Thread *t = (Thread*)alloc(sizeof(Thread));
	t->id = 0; // This is set when we start it
	t->proc = proc;
	t->initial_context = context;
	
	return t;
}
void os_start_thread(Thread *t) {
	t->os_handle = CreateThread(
        0,
        0,
        win32_thread_invoker,
        t,
        0,
        (DWORD*)&t->id
    );
    
    assert(t->os_handle, "Failed creating thread");
}
void os_join_thread(Thread *t) {
	WaitForSingleObject(t->os_handle, INFINITE);
	CloseHandle(t->os_handle);
}

void os_sleep(u32 ms) {
    Sleep(ms);
}

void os_yield_thread() {
    SwitchToThread();
}

#include <intrin.h>
u64 os_get_current_cycle_count() {
	return __rdtsc();
}

float64 os_get_current_time_in_seconds() {
    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) {
        return -1.0;
    }
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}


Dynamic_Library_Handle os_load_dynamic_library(string path) {
	return LoadLibraryA(temp_convert_to_null_terminated_string(path));
}
void *os_dynamic_library_load_symbol(Dynamic_Library_Handle l, string identifier) {
	return GetProcAddress(l, temp_convert_to_null_terminated_string(identifier));
}
void os_unload_dynamic_library(Dynamic_Library_Handle l) {
	FreeLibrary(l);
}


void os_write_string_to_stdout(string s) {
	HANDLE win32_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (win32_stdout == INVALID_HANDLE_VALUE) return;
	
	WriteFile(win32_stdout, s.data, s.count, 0, NULL);
}