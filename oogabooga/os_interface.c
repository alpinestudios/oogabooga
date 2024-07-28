

#ifdef _WIN32
	typedef HANDLE Mutex_Handle;
	typedef HANDLE Thread_Handle;
	typedef HMODULE Dynamic_Library_Handle;
	typedef HWND Window_Handle;
	typedef HANDLE File;
	
#elif defined(__linux__)
    #ifndef OOGABOOGA_HEADLESS
    #define "Linux is only supported for headless builds"
    #endif
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

// #Global
ogb_instance Os_Window window;
ogb_instance Os_Info os;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE

Os_Info os;
Os_Window window;

#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE

inline int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args) {
	return os.crt_vsnprintf(buffer, n, fmt, args);
}





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
	u64 temporary_storage_size; // Defaults to KB(10)
	Thread_Proc proc;
	Thread_Handle os_handle;
	
	
	Allocator allocator;  // Deprecated !! #Cleanup
} Thread;

///
// Thread primitive #Cleanup
DEPRECATED(Thread* os_make_thread(Thread_Proc proc, Allocator allocator), "Use os_thread_init instead");
DEPRECATED(void os_destroy_thread(Thread *t), "Use os_thread_destroy instead");
DEPRECATED(void os_start_thread(Thread* t), "Use os_thread_start instead");
DEPRECATED(void os_join_thread(Thread* t), "Use os_thread_join instead");

void ogb_instance
os_thread_init(Thread *t, Thread_Proc proc);

void ogb_instance
os_thread_destroy(Thread *t);

void ogb_instance
os_thread_start(Thread *t);

void ogb_instance
os_thread_join(Thread *t);



///
// Low-level Mutex primitive. Mutex in concurrency.c is probably a better alternative.
Mutex_Handle ogb_instance
os_make_mutex();

void ogb_instance
os_destroy_mutex(Mutex_Handle m);

void ogb_instance
os_lock_mutex(Mutex_Handle m);

void ogb_instance
os_unlock_mutex(Mutex_Handle m);

///
// Threading utilities

void ogb_instance
os_sleep(u32 ms);

void ogb_instance
os_yield_thread();

void ogb_instance
os_high_precision_sleep(f64 ms);


///
///
// Time
///

DEPRECATED(u64 os_get_current_cycle_count(), "use rdtsc() instead");

float64 ogb_instance
os_get_current_time_in_seconds();

///
///
// Dynamic Libraries
///

// #Cleanup this naming is bleh

Dynamic_Library_Handle ogb_instance
os_load_dynamic_library(string path);

ogb_instance void*
os_dynamic_library_load_symbol(Dynamic_Library_Handle l, string identifier);

void ogb_instance
os_unload_dynamic_library(Dynamic_Library_Handle l);

///
///
// IO
///

ogb_instance const File OS_INVALID_FILE;

void ogb_instance
os_write_string_to_stdout(string s);

typedef enum Os_Io_Open_Flags {
	O_READ   = 0,
	O_CREATE = 1<<0, // Will replace existing file and start writing from 0 (if writing)
	O_WRITE  = 1<<1,
	
	// To append, pass WRITE flag without CREATE flag
} Os_Io_Open_Flags;

// Returns OS_INVALID_FILE on fail
File ogb_instance
os_file_open_s(string path, Os_Io_Open_Flags flags);

void ogb_instance
os_file_close(File f);

bool ogb_instance
os_file_delete_s(string path);

bool ogb_instance
os_file_copy_s(string from, string to, bool replace_if_exists);


bool ogb_instance
os_make_directory_s(string path, bool recursive);

bool ogb_instance
os_delete_directory_s(string path, bool recursive);


bool ogb_instance
os_file_write_string(File f, string s);

bool ogb_instance
os_file_write_bytes(File f, void *buffer, u64 size_in_bytes);


bool ogb_instance
os_file_read(File f, void* buffer, u64 bytes_to_read, u64 *actual_read_bytes);


bool ogb_instance
os_file_set_pos(File f, s64 pos_in_bytes);

s64 ogb_instance
os_file_get_pos(File f);


s64 ogb_instance
os_file_get_size(File f);

s64 ogb_instance
os_file_get_size_from_path(string path);


bool ogb_instance
os_write_entire_file_handle(File f, string data);

bool ogb_instance
os_write_entire_file_s(string path, string data);

bool ogb_instance
os_read_entire_file_handle(File f, string *result, Allocator allocator);

bool ogb_instance
os_read_entire_file_s(string path, string *result, Allocator allocator);


bool ogb_instance
os_is_file_s(string path);

bool ogb_instance
os_is_directory_s(string path);


bool ogb_instance
os_is_path_absolute(string path);


bool ogb_instance
os_get_absolute_path(string path, string *result, Allocator allocator);

bool ogb_instance
os_get_relative_path(string from, string to, string *result, Allocator allocator);


bool ogb_instance
os_do_paths_match(string a, string b);


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
                          
inline bool os_file_copy_f(const char *from, const char *to, bool replace_if_exists) {return os_file_copy_s(STR(from), STR(to), replace_if_exists);}
#define os_file_copy(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  os_file_copy_s, \
                           default: os_file_copy_f \
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
                          
                          

void ogb_instance
fprints(File f, string fmt, ...);

void ogb_instance
fprintf(File f, const char* fmt, ...);

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
// Queries
///
ogb_instance void*
os_get_stack_base();
ogb_instance void*
os_get_stack_limit();

ogb_instance u64
os_get_number_of_logical_processors();


///
///
// Debug
///
ogb_instance string*
os_get_stack_trace(u64 *trace_count, Allocator allocator);

inline void 
dump_stack_trace() {
	u64 count;
	string *strings = os_get_stack_trace(&count, get_temporary_allocator());
	
	for (u64 i = 0; i < count; i++) {
		string s = strings[i];
		print("\n%s", s);
	}
}


///
///
// Memory
///

// #Global
ogb_instance void *program_memory;
ogb_instance void *program_memory_next;
ogb_instance u64 program_memory_capacity;
ogb_instance Mutex_Handle program_memory_mutex;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
void *program_memory = 0;
void *program_memory_next = 0;
u64 program_memory_capacity = 0;
Mutex_Handle program_memory_mutex = 0;
#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE

bool ogb_instance
os_grow_program_memory(size_t new_size);

// BEWARE:
// - size must be aligned to os.page_size
// - Pages will not always belong to the same region (although they will be contigious in virtual adress space)
// - Pages will be locked (Win32 PAGE_NOACCESS) so you need to unlock with os_unlock_program_memory_pages() before use.
ogb_instance void*
os_reserve_next_memory_pages(u64 size);

void ogb_instance
os_unlock_program_memory_pages(void *start, u64 size);
void ogb_instance
os_lock_program_memory_pages(void *start, u64 size);

///
///
// Mouse pointer

typedef enum Mouse_Pointer_Kind {
    MOUSE_POINTER_DEFAULT           = 0,   // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_arrow.png
    MOUSE_POINTER_TEXT_SELECT       = 10,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_ibeam.png
    MOUSE_POINTER_BUSY              = 20,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_wait.png
    MOUSE_POINTER_BUSY_BACKGROUND   = 30,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_appstarting.png
    MOUSE_POINTER_CROSS             = 40,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_cross.png
    MOUSE_POINTER_ARROW_N           = 50,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_uparrow.png
    MOUSE_POINTER_ARROWS_NW_SE      = 60,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_sizenwse.png
    MOUSE_POINTER_ARROWS_NE_SW      = 70,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_sizenesw.png
    MOUSE_POINTER_ARROWS_HORIZONTAL = 80,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_sizewe.png
    MOUSE_POINTER_ARROWS_VERTICAL   = 90,  // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_sizens.png
    MOUSE_POINTER_ARROWS_ALL        = 100, // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_sizeall.png
    MOUSE_POINTER_NO                = 110, // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_no.png
    MOUSE_POINTER_POINT             = 120, // https://learn.microsoft.com/en-us/windows/win32/menurc/images/idc_hand.png
    
    MOUSE_POINTER_MAX,
} Mouse_Pointer_Kind;

typedef void* Custom_Mouse_Pointer;

void ogb_instance
os_set_mouse_pointer_standard(Mouse_Pointer_Kind kind);
void ogb_instance
os_set_mouse_pointer_custom(Custom_Mouse_Pointer p);

// Expects 32-bit rgba
// Returns 0 on fail
Custom_Mouse_Pointer ogb_instance
os_make_custom_mouse_pointer(void *image, int width, int height, int hotspot_x, int hotspot_y);

// Returns 0 on fail
// Will free everything that's allocated, passing temp allocator should be fine as long as the image is small
Custom_Mouse_Pointer ogb_instance
os_make_custom_mouse_pointer_from_file(string path, int hotspot_x, int hotspot_y, Allocator allocator);


///////////////////////////////////////////////
void ogb_instance
os_init(u64 program_memory_size);

void ogb_instance
os_update();