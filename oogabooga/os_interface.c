

#ifdef _WIN32
	typedef HANDLE Mutex_Handle;
	typedef HANDLE Thread_Handle;
	typedef HMODULE Dynamic_Library_Handle;
	typedef HWND Window_Handle;
	typedef HANDLE File;
	
#elif defined(__linux__)
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
	typedef SOMETHING Dynamic_Library_Handle;
	typedef SOMETHING Window_Handle;
	typedef SOMETHING File;
	#error "Linux is not supported yet";
#elif defined(__APPLE__) && defined(__MACH__)
	typedef SOMETHING Mutex_Handle;
	typedef SOMETHING Thread_Handle;
	typedef SOMETHING Dynamic_Library_Handle;
	typedef SOMETHING Window_Handle;
	typedef SOMETHING File;
	#error "Mac is not supported yet";
#else
	#error "Current OS not supported!";
#endif

#ifndef max
	#define max(a, b) ((a) > (b) ? (a) : (b))
	#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define _INTSIZEOF(n)         ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

typedef void* (__cdecl *Crt_Memcpy_Proc)    (void*, const void*, size_t);
typedef int   (__cdecl *Crt_Memcmp_Proc)    (const void*, const void*, size_t);
typedef void* (__cdecl *Crt_Memset_Proc)    (void*, int, size_t);
typedef int   (__cdecl *Crt_Vprintf_Proc)   (const char*, va_list);
typedef int   (__cdecl *Crt_Vsnprintf_Proc) (char*, size_t, const char*, va_list);
typedef int   (__cdecl *Crt_Vsprintf_Proc)  (char*, const char*, va_list);

typedef struct Os_Info {
	u64 page_size;
	u64 granularity;
	
	Dynamic_Library_Handle crt;
	
	Crt_Memcpy_Proc    crt_memcpy;
	Crt_Memcmp_Proc    crt_memcmp;
	Crt_Memset_Proc    crt_memset;
	Crt_Vprintf_Proc   crt_vprintf;
	Crt_Vsnprintf_Proc crt_vsnprintf;
	Crt_Vsprintf_Proc  crt_vsprintf;
    
    void *static_memory_start, *static_memory_end;
    
} Os_Info;
Os_Info os;

inline int crt_vprintf(const char* fmt, va_list args) {
	return os.crt_vprintf(fmt, args);
}

#if !defined(COMPILER_HAS_MEMCPY_INTRINSICS) || CONFIGURATION == DEBUG
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
#endif

inline int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args) {
	return os.crt_vsnprintf(buffer, n, fmt, args);
}

inline int crt_sprintf(char *str, const char *format, ...) {
	va_list args;
	va_start(args, format);
	int r = os.crt_vsprintf(str, format, args);
	va_end(args);
	return r;
}

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
Thread* os_make_thread(Thread_Proc proc);
void os_start_thread(Thread* t);
void os_join_thread(Thread* t);



///
// Mutex primitive
Mutex_Handle os_make_mutex();
void os_destroy_mutex(Mutex_Handle m);
void os_lock_mutex(Mutex_Handle m);
void os_unlock_mutex(Mutex_Handle m);

///
// Spinlock "primitive"
typedef struct Spinlock {
	bool locked;
} Spinlock;
Spinlock *os_make_spinlock();
void os_spinlock_lock(Spinlock* l);
void os_spinlock_unlock(Spinlock* l);

///
// Concurrency utilities

bool os_compare_and_swap_8   (u8   *a, u8   b, u8   old);
bool os_compare_and_swap_16  (u16  *a, u16  b, u16  old);
bool os_compare_and_swap_32  (u32  *a, u32  b, u32  old);
bool os_compare_and_swap_64  (u64  *a, u64  b, u64  old);
bool os_compare_and_swap_bool(bool *a, bool b, bool old);

void os_sleep(u32 ms);
void os_yield_thread();
void os_high_precision_sleep(f64 ms);

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

forward_global const File OS_INVALID_FILE;

void os_write_string_to_stdout(string s);

typedef enum Os_Io_Open_Flags {
	O_READ   = 0,
	O_CREATE = 1<<0, // Will replace existing file and start writing from 0 (if writing)
	O_WRITE  = 1<<1,
	
	// To append, pass WRITE flag without CREATE flag
} Os_Io_Open_Flags;

File os_file_open(string path, Os_Io_Open_Flags flags);
void os_file_close(File f);
bool os_file_delete(string path);

bool os_file_write_string(File f, string s);
bool os_file_write_bytes(File f, void *buffer, u64 size_in_bytes);

bool os_file_read(File f, void* buffer, u64 bytes_to_read, u64 *actual_read_bytes);

bool os_write_entire_file_handle(File f, string data);
bool os_write_entire_file(string path, string data);
bool os_read_entire_file_handle(File f, string *result);
bool os_read_entire_file(string path, string *result);


void fprints(File f, string fmt, ...);
void fprintf(File f, const char* fmt, ...);
#define fprint(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  fprints, \
                           default: fprintf \
                          )(__VA_ARGS__)

void fprint_va_list_buffered(File f, const string fmt, va_list args) {

	string current = fmt;

	char buffer[PRINT_BUFFER_SIZE];
	
	while (true) {
		u64 size = min(current.count, PRINT_BUFFER_SIZE-1);
		if (current.count <= 0) break;
		
		memcpy(buffer, current.data, size);
		
		char fmt_cstring[PRINT_BUFFER_SIZE+1];
		memcpy(fmt_cstring, current.data, size);
		fmt_cstring[size] = 0;
		
		string s = sprint_null_terminated_string_va_list_to_buffer(fmt_cstring, args, buffer, PRINT_BUFFER_SIZE);
		os_file_write_string(f, s);
		
		current.count -= size;
		current.data += size;
	}
}


///
///
// Memory
///
void* os_get_stack_base();
void* os_get_stack_limit();



///
///
// Window management
///
typedef struct Os_Window {

	// Keep in mind that setting these in runtime is potentially slow!
	string title;
	u32 width;
	u32 height;
	u32 x;
	u32 y;
	struct {
		float32 r, g, b, a;
	} clear_color;
	
	bool should_close;
	
	// readonly
	bool _initialized;
	Window_Handle _os_handle;
	
} Os_Window;
Os_Window window;




///
///
// Window input
///



void os_update();