

#ifdef _WIN32
	#include <Windows.h>
	#define OS_WINDOWS
	
	typedef HANDLE Mutex_Handle;
	typedef HANDLE Thread_Handle;
	typedef HMODULE Dynamic_Library_Handle;
	
#elif defined(__linux__)
	// Include whatever #Incomplete #Portability
	#define OS_LINUX
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
	typedef SOMETHING Dynamic_Library_Handle;
	
	#error "Linux is not supported yet";
#elif defined(__APPLE__) && defined(__MACH__)
	// Include whatever #Incomplete #Portability
	#define OS_MAC
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
	typedef SOMETHING Dynamic_Library_Handle;
	#error "Mac is not supported yet";
#else
	#error "Current OS not supported!";
#endif

#ifndef max
	#define max(a, b) ((a) > (b) ? (a) : (b))
	#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define _INTSIZEOF(n)         ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define oogabooga_va_start(ap, v)       (ap = (va_list)&v + _INTSIZEOF(v))
#define oogabooga_va_arg(ap, t)         (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define oogabooga_va_end(ap)            (ap = (va_list)0)

typedef void* (__cdecl *Crt_Memcpy_Proc)   (void*, const void*, size_t);
typedef int   (__cdecl *Crt_Memcmp_Proc)   (const void*, const void*, size_t);
typedef void* (__cdecl *Crt_Memset_Proc)   (void*, int, size_t);
typedef int   (__cdecl *Crt_Printf_Proc)   (const char*, ...);
typedef int   (__cdecl *Crt_Vprintf_Proc)   (const char*, va_list);
typedef int   (__cdecl *Crt_Vsnprintf_Proc)   (char*, size_t, const char*, va_list);

typedef struct Os_Info {
	u64 page_size;
	u64 granularity;
	
	Dynamic_Library_Handle crt;
	
	Crt_Memcpy_Proc    crt_memcpy;
	Crt_Memcmp_Proc    crt_memcmp;
	Crt_Memset_Proc    crt_memset;
	Crt_Printf_Proc    crt_printf; // #Cleanup remove after we have our own print
	Crt_Vprintf_Proc   crt_vprintf; // #Cleanup remove after we have our own print
	Crt_Vsnprintf_Proc crt_vsnprintf;
    
} Os_Info;
Os_Info os;

inline void* naive_memcpy(void* dest, const void* source, size_t size) {
	for (u64 i = 0; i < (u64)size; i++) ((u8*)dest)[i] = ((u8*)source)[i];
	return dest;
}
inline void* memcpy(void* dest, const void* source, size_t size) {
	if (!os.crt_memcpy) return naive_memcpy(dest, source, size);
	return os.crt_memcpy(dest, source, size);
}
inline int naive_memcmp(const void* a, const void* b, size_t amount) {
	// I don't understand the return value of memcmp but I also dont care
	for (u64 i = 0; i < (u64)amount; i++) {
		if (((u8*)a)[i] != ((u8*)b)[i])  return -1;
	}
	return 0;
}
inline int memcmp(const void* a, const void* b, size_t amount) {
	if (!os.crt_memcmp)  return naive_memcmp(a, b, amount);
	return os.crt_memcmp(a, b, amount);
}
inline void* naive_memset(void* dest, int value, size_t amount) {
	for (u64 i = 0; i < (u64)amount; i++) ((u8*)dest)[i] = (u8)value;
	return dest;
}
inline void* memset(void* dest, int value, size_t amount) {
	if (!os.crt_memset)  return naive_memset(dest, value, amount);
	return os.crt_memset(dest, value, amount);
}
inline int printf(const char* fmt, ...) {
	char fast_buffer[8196];
	char *large_buffer = 0;

	va_list args;
	oogabooga_va_start(args, fmt);
	int r = vprintf(fmt, args);
	oogabooga_va_end(args);
	
	return r;
}
void os_write_string_to_stdout(string s);
inline int vprintf(const char* fmt, va_list args) {
	if (os.crt_vprintf)  return os.crt_vprintf(fmt, args);
	else {
		os_write_string_to_stdout(cstr(fmt));
		os_write_string_to_stdout(cstr(" <crt_vprintf is not loaded so we cannot vprintf.>"));
		return 0;
	}
}
inline int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args) {
	os.crt_vsnprintf(buffer, n, fmt, args);
}

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


///
///
// Dynamic Libraries
///

Dynamic_Library_Handle os_load_dynamic_library(string path);
void *os_dynamic_library_load_symbol(Dynamic_Library_Handle l, string identifier);
void os_unload_dynamic_library(Dynamic_Library_Handle l);



///
///
// IO
///

void os_write_string_to_stdout(string s);