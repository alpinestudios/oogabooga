

#ifdef _WIN32
	#include <Windows.h>
	#define OS_WINDOWS
	
	typedef HANDLE Mutex_Handle;
	typedef HANDLE Thread_Handle;
	
#elif defined(__linux__)
	// Include whatever #Incomplete #Portability
	#define OS_LINUX
	
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
	
#elif defined(__APPLE__) && defined(__MACH__)
	// Include whatever #Incomplete #Portability
	#define OS_MAC
	
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
#else
	#error "Current OS not supported!";
#endif

typedef struct Os_Info {
	u64 page_size;
	u64 granularity;
} Os_Info;
Os_Info os;




void* program_memory = 0;
u64 program_memory_size = 0;
Mutex_Handle program_memory_mutex = 0;

bool os_grow_program_memory(size_t new_size);

///
///
// Threading
///

typedef struct Thread Thread;

typedef void(*Thread_Proc)(Thread*);

typedef struct Thread {
	u64 id;
	Context initial_context;
	void* data;
	Thread_Proc proc;
	Thread_Handle os_handle;
	
} Thread;

///
// Thread primitive
Thread* os_make_thread();
void os_start_thread(Thread* t);
void os_join_thread(Thread* t);

void os_sleep(u32 ms);
void os_yield_thread();

///
// Mutex primitive
Mutex_Handle os_make_mutex();
void os_destroy_mutex(Mutex_Handle m);
void os_lock_mutex(Mutex_Handle m);
void os_unlock_mutex(Mutex_Handle m);


///
///
// Time
///

u64 os_get_current_cycle_count();
float64 os_get_current_time_in_seconds();