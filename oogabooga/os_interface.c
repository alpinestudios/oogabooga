

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

typedef int   (__cdecl *Crt_Vsnprintf_Proc) (char*, size_t, const char*, va_list);

typedef struct Os_Info {
	u64 page_size;
	u64 granularity;
	
	Dynamic_Library_Handle crt;
	
	Crt_Vsnprintf_Proc crt_vsnprintf;
    
    void *static_memory_start, *static_memory_end;
    
} Os_Info;
Os_Info os;

inline bool bytes_match(void *a, void *b, u64 count) { return memcmp(a, b, count) == 0; }

inline int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args) {
	return os.crt_vsnprintf(buffer, n, fmt, args);
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
	u64 id; // This is valid after os_thread_start
	Context initial_context;
	void* data;
	Thread_Proc proc;
	Thread_Handle os_handle;
	Allocator allocator;  // Deprecated !! #Cleanup
} Thread;

///
// Thread primitive
DEPRECATED(Thread* os_make_thread(Thread_Proc proc, Allocator allocator), "Use os_thread_init instead");
DEPRECATED(void os_destroy_thread(Thread *t), "Use os_thread_destroy instead");
DEPRECATED(void os_start_thread(Thread* t), "Use os_thread_start instead");
DEPRECATED(void os_join_thread(Thread* t), "Use os_thread_join instead");

void os_thread_init(Thread *t, Thread_Proc proc);
void os_thread_destroy(Thread *t);
void os_thread_start(Thread *t);
void os_thread_join(Thread *t);


///
// Low-level Mutex primitive. Mutex in concurrency.c is probably a better alternative.
Mutex_Handle os_make_mutex();
void os_destroy_mutex(Mutex_Handle m);
void os_lock_mutex(Mutex_Handle m);
void os_unlock_mutex(Mutex_Handle m);

///
// Threading utilities

void os_sleep(u32 ms);
void os_yield_thread();
void os_high_precision_sleep(f64 ms);

///
///
// Time
///

DEPRECATED(u64 os_get_current_cycle_count(), "use rdtsc() instead");
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

File os_file_open_s(string path, Os_Io_Open_Flags flags);
void os_file_close(File f);
bool os_file_delete_s(string path);

bool os_make_directory_s(string path, bool recursive);
bool os_delete_directory_s(string path, bool recursive);

bool os_file_write_string(File f, string s);
bool os_file_write_bytes(File f, void *buffer, u64 size_in_bytes);

bool os_file_read(File f, void* buffer, u64 bytes_to_read, u64 *actual_read_bytes);

bool os_file_set_pos(File f, s64 pos_in_bytes);
s64  os_file_get_pos(File f);

s64 os_file_get_size(File f);
s64 os_file_get_size_from_path(string path);

bool os_write_entire_file_handle(File f, string data);
bool os_write_entire_file_s(string path, string data);
bool os_read_entire_file_handle(File f, string *result, Allocator allocator);
bool os_read_entire_file_s(string path, string *result, Allocator allocator);

bool os_is_file_s(string path);
bool os_is_directory_s(string path);

bool os_is_path_absolute(string path);

bool os_get_absolute_path(string path, string *result, Allocator allocator);
bool os_get_relative_path(string from, string to, string *result, Allocator allocator);

bool os_do_paths_match(string a, string b);

// It's a little unfortunate that we need to do this but I can't think of a better solution

inline File os_file_open_f(const char *path, Os_Io_Open_Flags flags) {return os_file_open_s(STR(path), flags);}
#define os_file_open(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_file_open_s, \
                           default: os_file_open_f \
                          )(__VA_ARGS__)
                          
inline bool os_file_delete_f(const char *path) {return os_file_delete_s(STR(path));}
#define os_file_delete(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_file_delete_s, \
                           default: os_file_delete_f \
                          )(__VA_ARGS__)
                          
inline bool os_make_directory_f(const char *path, bool recursive) { return os_make_directory_s(STR(path), recursive); }
#define os_make_directory(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_make_directory_s, \
                           default: os_make_directory_f \
                          )(__VA_ARGS__)
inline bool os_delete_directory_f(const char *path, bool recursive) { return os_delete_directory_s(STR(path), recursive); }
#define os_delete_directory(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_delete_directory_s, \
                           default: os_delete_directory_f \
                          )(__VA_ARGS__)

inline bool os_write_entire_file_f(const char *path, string data) {return os_write_entire_file_s(STR(path), data);}
#define os_write_entire_file(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_write_entire_file_s, \
                           default: os_write_entire_file_f \
                          )(__VA_ARGS__)
                          
inline bool os_read_entire_file_f(const char *path, string *result, Allocator allocator) {return os_read_entire_file_s(STR(path), result, allocator);}
#define os_read_entire_file(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_read_entire_file_s, \
                           default: os_read_entire_file_f \
                          )(__VA_ARGS__)
                          
inline bool os_is_file_f(const char *path) {return os_is_file_s(STR(path));}
#define os_is_file(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_is_file_s, \
                           default: os_is_file_f \
                          )(__VA_ARGS__)
                          
inline bool os_is_directory_f(const char *path) {return os_is_directory_s(STR(path));}
#define os_is_directory(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_is_directory_s, \
                           default: os_is_directory_f \
                          )(__VA_ARGS__)
                          
                          

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
void* 
os_get_stack_base();
void* 
os_get_stack_limit();


///
///
// Debug
///
string *
os_get_stack_trace(u64 *trace_count, Allocator allocator);

void dump_stack_trace() {
	u64 count;
	string *strings = os_get_stack_trace(&count, get_temporary_allocator());
	
	for (u64 i = 0; i < count; i++) {
		string s = strings[i];
		print("\n%s", s);
	}
}

///
///
// Window management
///
typedef struct Os_Window {

	// Keep in mind that setting these in runtime is potentially slow!
	string title;
	union { s32 width;  s32 pixel_width;  };
	union { s32 height; s32 pixel_height; };
	s32 scaled_width; // DPI scaled!
	s32 scaled_height; // DPI scaled!
	s32 x;
	s32 y;
	Vector4 clear_color;
	bool enable_vsync;
	
	bool should_close;
	
	// readonly
	bool _initialized;
	Window_Handle _os_handle;
	
} Os_Window;
Os_Window window;

void os_update();