

#define VIRTUAL_MEMORY_BASE ((void*)0x0000010000000000ULL)

void* heap_alloc(u64);
void heap_dealloc(void*);

// Persistent
Input_State_Flags win32_key_states[INPUT_KEY_CODE_COUNT];

void win32_send_key_event(Input_Key_Code code, Input_State_Flags state) {
	Input_Event e;
	e.kind = INPUT_EVENT_KEY;
	e.key_code = code;
	e.key_state = state;
	input_frame.events[input_frame.number_of_events] = e;
	input_frame.number_of_events += 1;
}

void win32_handle_key_up(Input_Key_Code code) {
	if (code == KEY_UNKNOWN) return;
	
	Input_State_Flags last_state = win32_key_states[code];
	Input_State_Flags state = 0;
	
	if (last_state & INPUT_STATE_DOWN)  state |= INPUT_STATE_JUST_RELEASED;
	
	win32_key_states[code] = state;
	
	win32_send_key_event(code, state);
}
void win32_handle_key_down(Input_Key_Code code) {
	if (code == KEY_UNKNOWN) return;
	
	Input_State_Flags last_state = win32_key_states[code];
	Input_State_Flags state = INPUT_STATE_DOWN;
	
	if (!(last_state & INPUT_STATE_DOWN))  state |= INPUT_STATE_JUST_PRESSED;
	
	win32_key_states[code] = state;
	
	win32_send_key_event(code, state);
}
void win32_handle_key_repeat(Input_Key_Code code) {
	if (code == KEY_UNKNOWN) return;
	
	win32_key_states[code] |= INPUT_STATE_REPEAT;
	
	win32_send_key_event(code, win32_key_states[code]);
}
LRESULT CALLBACK win32_window_proc(HWND passed_window, UINT message, WPARAM wparam, LPARAM lparam) {
	
	if (window._initialized) {
		assert(passed_window == window._os_handle, "Event from another window?? wut (%d)", message);
	}
	
    switch (message) {
        case WM_CLOSE:
        	window.should_close = true;
            DestroyWindow(window._os_handle);
			
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
        	bool is_repeat = (lparam & 0x40000000) != 0;
        	
        	if (is_repeat) win32_handle_key_repeat(os_key_to_key_code((void*)wparam));
	        else           win32_handle_key_down  (os_key_to_key_code((void*)wparam));
	        goto DEFAULT_HANDLE;
	    case WM_KEYUP:
	        win32_handle_key_up(os_key_to_key_code((void*)wparam));
	        goto DEFAULT_HANDLE;
	    case WM_LBUTTONDOWN:
	        win32_handle_key_down(MOUSE_BUTTON_LEFT);
	        goto DEFAULT_HANDLE;
	    case WM_RBUTTONDOWN:
	        win32_handle_key_down(MOUSE_BUTTON_RIGHT);
	        goto DEFAULT_HANDLE;
	    case WM_MBUTTONDOWN:
	        win32_handle_key_down(MOUSE_BUTTON_MIDDLE);
	        goto DEFAULT_HANDLE;
	    case WM_LBUTTONUP:
	        win32_handle_key_up(MOUSE_BUTTON_LEFT);
	        goto DEFAULT_HANDLE;
	    case WM_RBUTTONUP:
	        win32_handle_key_up(MOUSE_BUTTON_RIGHT);
	        goto DEFAULT_HANDLE;
	    case WM_MBUTTONUP:
			win32_handle_key_up(MOUSE_BUTTON_MIDDLE);
	        goto DEFAULT_HANDLE;
	    case WM_MOUSEWHEEL: {
	        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
	        int ticks = delta / WHEEL_DELTA;
	        Input_Event e;
	        e.kind = INPUT_EVENT_SCROLL;
	        e.yscroll = (float64)delta/(float64)WHEEL_DELTA;
	        e.xscroll = 0;
	        input_frame.events[input_frame.number_of_events] = e;
			input_frame.number_of_events += 1;
	        goto DEFAULT_HANDLE;
	    }
	    case WM_MOUSEHWHEEL: {
	        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
	        Input_Event e;
	        e.kind = INPUT_EVENT_SCROLL;
	        e.yscroll = 0;
	        e.xscroll = (float64)delta/(float64)WHEEL_DELTA;
	        input_frame.events[input_frame.number_of_events] = e;
			input_frame.number_of_events += 1;
	        goto DEFAULT_HANDLE;
	    }
	    case WM_CHAR: {
	        wchar_t utf16 = (wchar_t)wparam;
	        
	        Input_Event e;
	        e.kind = INPUT_EVENT_TEXT;
	        utf16_to_utf32(&utf16, 1, &e.utf32);
	        
	        input_frame.events[input_frame.number_of_events] = e;
			input_frame.number_of_events += 1;
	        
	        goto DEFAULT_HANDLE;
	    }
        default:
        
        DEFAULT_HANDLE:
            return DefWindowProc(passed_window, message, wparam, lparam);
    }
    return 0;
}


void os_init(u64 program_memory_size) {
	
	SYSTEM_INFO si;
    GetSystemInfo(&si);
	os.granularity = cast(u64)si.dwAllocationGranularity;
	os.page_size = cast(u64)si.dwPageSize;
	
	os.static_memory_start = 0;
	os.static_memory_end = 0;
	
	MEMORY_BASIC_INFORMATION mbi;
    
    
    unsigned char* addr = 0;
    while (VirtualQuery(addr, &mbi, sizeof(mbi))) {
        if (mbi.Type == MEM_IMAGE) {
            if (os.static_memory_start == NULL) {
                os.static_memory_start = mbi.BaseAddress;
            }
            os.static_memory_end = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
        }
        addr += mbi.RegionSize;
    }


	program_memory_mutex = os_make_mutex();
	os_grow_program_memory(program_memory_size);
	
	
	heap_init();
	context.allocator = get_heap_allocator();
	
	os.crt = os_load_dynamic_library(const_string("msvcrt.dll"));
	assert(os.crt != 0, "Could not load win32 crt library. Might be compiled with non-msvc? #Incomplete #Portability");
	os.crt_vsnprintf = (Crt_Vsnprintf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("vsnprintf"));
	assert(os.crt_vsnprintf, "Missing vsnprintf in crt");
	os.crt_vprintf = (Crt_Vprintf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("vprintf"));
	assert(os.crt_vprintf, "Missing vprintf in crt");
	os.crt_vsprintf = (Crt_Vsprintf_Proc)os_dynamic_library_load_symbol(os.crt, const_string("vsprintf"));
	assert(os.crt_vsprintf, "Missing vsprintf in crt");
	os.crt_memcpy = (Crt_Memcpy_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memcpy"));
	assert(os.crt_memcpy, "Missing memcpy in crt");
	os.crt_memcmp = (Crt_Memcmp_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memcmp"));
	assert(os.crt_memcmp, "Missing crt_memcmp in crt");
	os.crt_memset = (Crt_Memset_Proc)os_dynamic_library_load_symbol(os.crt, const_string("memset"));
	assert(os.crt_memset, "Missing memset in crt");
	
	window.title = fixed_string("Unnamed Window");
	window.width = 1280;
	window.height = 720;
	window.x = 0;
	window.y = 0;
	window.should_close = false;
	window._initialized = false;
	window.clear_color.r = 0.392f; 
	window.clear_color.g = 0.584f;
	window.clear_color.b = 0.929f;
	window.clear_color.a = 1.0f;
	
	WNDCLASSEX wc = (WNDCLASSEX){0};
    MSG msg;
    HINSTANCE instance = GetModuleHandle(NULL);
    assert(instance != INVALID_HANDLE_VALUE, "Failed getting current HINSTANCE");

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = win32_window_proc;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "sigma balls";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	BOOL ok = RegisterClassEx(&wc);
	assert(ok, "Failed registering window class (error code %lu)", GetLastError());
	
	RECT rect = {0, 0, window.width, window.height};
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD ex_style = WS_EX_CLIENTEDGE;
	ok = AdjustWindowRectEx(&rect, style, FALSE, ex_style);
	assert(ok != 0, "AdjustWindowRectEx failed with error code %lu", GetLastError());
	
	u32 actual_window_width = rect.right - rect.left;
	u32 actual_window_height = rect.bottom - rect.top;
    // Create the window
    window._os_handle = CreateWindowEx(
        ex_style,
        "sigma balls",
        temp_convert_to_null_terminated_string(window.title),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, actual_window_width, actual_window_height,
        NULL, NULL, instance, NULL);
    assert(window._os_handle != NULL, "Window creation failed, error: %lu", GetLastError());
	window._initialized = true;
    ShowWindow(window._os_handle, SW_SHOWDEFAULT);
    UpdateWindow(window._os_handle);
    
    
    
    
    
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

		program_memory = VirtualAlloc(aligned_base, aligned_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (program_memory == 0) { 
			os_unlock_mutex(program_memory_mutex); // #Sync
			return false;
		}
		program_memory_size = aligned_size;
		
		memset(program_memory, 0xBA, program_memory_size);
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
		memset(result, 0xBA, amount_to_allocate);
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


///
///
// Threading
///


///
// Thread primitive

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

///
// Mutex primitive

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

///
// Spinlock "primitive"

Spinlock *os_make_spinlock() {
	Spinlock *l = cast(Spinlock*)alloc(sizeof(Spinlock));
	l->locked = false;
	return l;
}
void os_spinlock_lock(Spinlock *l) {
    while (true) {
        bool expected = false;
        if (os_compare_and_swap_bool(&l->locked, true, expected)) {
            return;
        }
        while (l->locked) {
            // spinny boi
        }
    }
}

void os_spinlock_unlock(Spinlock *l) {
    bool expected = true;
    bool success = os_compare_and_swap_bool(&l->locked, false, expected);
    assert(success, "This thread should have acquired the spinlock but compare_and_swap failed");
}


///
// Concurrency utilities

bool os_compare_and_swap_8(u8 *a, u8 b, u8 old) {
	// #Portability not sure how portable this is.
    return _InterlockedCompareExchange8((volatile CHAR*)a, (CHAR)b, (CHAR)old) == (CHAR)old;
}

bool os_compare_and_swap_16(u16 *a, u16 b, u16 old) {
    return InterlockedCompareExchange16((volatile SHORT*)a, (SHORT)b, (SHORT)old) == (SHORT)old;
}

bool os_compare_and_swap_32(u32 *a, u32 b, u32 old) {
    return InterlockedCompareExchange((volatile LONG*)a, (LONG)b, (LONG)old) == (LONG)old;
}

bool os_compare_and_swap_64(u64 *a, u64 b, u64 old) {
    return InterlockedCompareExchange64((volatile LONG64*)a, (LONG64)b, (LONG64)old) == (LONG64)old;
}

bool os_compare_and_swap_bool(bool *a, bool b, bool old) {
	return os_compare_and_swap_8(cast(u8*)a, cast(u8)b, cast(u8)old);
}



void os_sleep(u32 ms) {
    Sleep(ms);
}

void os_yield_thread() {
    SwitchToThread();
}

void os_high_precision_sleep(f64 ms) {
	
	const f64 s = ms/1000.0;
	
	f64 start = os_get_current_time_in_seconds();
	f64 end = start + (f64)s;
	u32 sleep_time = (u32)((end-start)-1.0);
	bool do_sleep = sleep_time >= 1;
	
	timeBeginPeriod(1); // I don't see a reason to reset this
	
	if (do_sleep)  os_sleep(sleep_time);
	
	while (os_get_current_time_in_seconds() < end) {
		os_yield_thread();
	}
}


///
///
// Time
///

#include <intrin.h> // #Cdep
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


///
///
// Dynamic Libraries
///

Dynamic_Library_Handle os_load_dynamic_library(string path) {
	return LoadLibraryA(temp_convert_to_null_terminated_string(path));
}
void *os_dynamic_library_load_symbol(Dynamic_Library_Handle l, string identifier) {
	return GetProcAddress(l, temp_convert_to_null_terminated_string(identifier));
}
void os_unload_dynamic_library(Dynamic_Library_Handle l) {
	FreeLibrary(l);
}


///
///
// IO
///

const File OS_INVALID_FILE = INVALID_HANDLE_VALUE;
void os_write_string_to_stdout(string s) {
	HANDLE win32_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (win32_stdout == INVALID_HANDLE_VALUE) return;
	
	WriteFile(win32_stdout, s.data, s.count, 0, NULL);
}

// context.allocator
u16 *win32_fixed_utf8_to_null_terminated_wide(string utf8) {
    u64 utf16_length = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8.data, (int)utf8.count, 0, 0);
    
    u16 *utf16_str = (u16 *)alloc((utf16_length + 1) * sizeof(u16));

    int result = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8.data, (int)utf8.count, utf16_str, utf16_length);
    if (result == 0) {
        dealloc(utf16_str);
        return NULL;
    }

    utf16_str[utf16_length] = 0;

    return utf16_str;
}
u16 *temp_win32_fixed_utf8_to_null_terminated_wide(string utf8) {
	push_temp_allocator();
	u16 *result = win32_fixed_utf8_to_null_terminated_wide(utf8);
	pop_allocator();
	return result;
}
string win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16) {
    u64 utf8_length = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)utf16, -1, 0, 0, 0, 0);

    u8 *utf8_str = (u8 *)alloc(utf8_length * sizeof(u8));

    int result = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)utf16, -1, (LPSTR)utf8_str, (int)utf8_length, 0, 0);
    if (result == 0) {
        dealloc(utf8_str);
        return (string){0, 0};
    }

    string utf8;
    utf8.data = utf8_str;
    utf8.count = utf8_length-1;

    return utf8;
}

string temp_win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16) {
    push_temp_allocator();
    string result = win32_null_terminated_wide_to_fixed_utf8(utf16);
    pop_allocator();
    return result;
}


File os_file_open(string path, Os_Io_Open_Flags flags) {
    DWORD access = GENERIC_READ;
    DWORD creation = 0;

    if (flags & O_WRITE) {
        access |= GENERIC_WRITE;
    }
    if (flags & O_CREATE) {
        creation = CREATE_ALWAYS;
    } else {
        creation = OPEN_EXISTING;
    }
    
    u16 *wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);

    return CreateFileW(wide, access, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
}

void os_file_close(File f) {
    CloseHandle(f);
}

bool os_file_delete(string path) {
	u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	return (bool)DeleteFileW(path_wide);
}

bool os_file_write_string(File f, string s) {
    DWORD written;
    BOOL result = WriteFile(f, s.data, s.count, &written, NULL);
    return result && (written == s.count);
}

bool os_file_write_bytes(File f, void *buffer, u64 size_in_bytes) {
    DWORD written;
    BOOL result = WriteFile(f, buffer, (DWORD)size_in_bytes, &written, NULL);
    return result && (written == size_in_bytes);
}

bool os_file_read(File f, void* buffer, u64 bytes_to_read, u64 *actual_read_bytes) {
    DWORD read;
    BOOL result = ReadFile(f, buffer, (DWORD)bytes_to_read, &read, NULL);
    if (actual_read_bytes) {
        *actual_read_bytes = read;
    }
    return result;
}

bool os_write_entire_file_handle(File f, string data) {
    return os_file_write_string(f, data);
}

bool os_write_entire_file(string path, string data) {
    File file = os_file_open(path, O_WRITE | O_CREATE);
    if (file == OS_INVALID_FILE) {
        return false;
    }
    bool result = os_file_write_string(file, data);
    os_file_close(file);
    return result;
}

bool os_read_entire_file_handle(File f, string *result) {
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(f, &file_size)) {
        return false;
    }
    
    u64 actual_read = 0;
    result->data = (u8*)alloc(file_size.QuadPart);
    result->count = file_size.QuadPart;
    
    bool ok = os_file_read(f, result->data, file_size.QuadPart, &actual_read);
    if (!ok) {
		dealloc(result->data);
		result->data = 0;
		return false;
	}
    
    return actual_read == file_size.QuadPart;
}

bool os_read_entire_file(string path, string *result) {
    File file = os_file_open(path, O_READ);
    if (file == OS_INVALID_FILE) {
        return false;
    }
    bool res = os_read_entire_file_handle(file, result);
    os_file_close(file);
    return res;
}

bool os_is_file(string path) {
	u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	assert(path_wide, "Invalid path string");
    if (path_wide == NULL) {
        return false;
    }

    DWORD attributes = GetFileAttributesW(path_wide);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool os_is_directory(string path) {
    u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	assert(path_wide, "Invalid path string");
    if (path_wide == NULL) {
        return false;
    }

    DWORD attributes = GetFileAttributesW(path_wide);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

void fprints(File f, string fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprint_va_list_buffered(f, fmt, args);
	va_end(args);
}
void fprintf(File f, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	string s;
	s.data = cast(u8*)fmt;
	s.count = strlen(fmt);
	fprint_va_list_buffered(f, s, args);
	va_end(args);
}





///
///
// Memory
///

void* os_get_stack_base() {
	NT_TIB* tib = (NT_TIB*)NtCurrentTeb();
    return tib->StackBase;
}
void* os_get_stack_limit() {
	NT_TIB* tib = (NT_TIB*)NtCurrentTeb();
    return tib->StackLimit;
}





void os_update() {

	local_persist Os_Window last_window;

	if (!strings_match(last_window.title, window.title)) {
		SetWindowText(window._os_handle, temp_convert_to_null_terminated_string(window.title));
	}
	
	BOOL ok;
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	DWORD style = (DWORD)GetWindowLong(window._os_handle, GWL_STYLE);
	DWORD ex_style = (DWORD)GetWindowLong(window._os_handle, GWL_EXSTYLE);
	if (last_window.x != window.x || last_window.y != window.y || last_window.width != window.width || last_window.y != window.y) {
		RECT update_rect;
		update_rect.left = window.x;
		update_rect.right = window.x + window.width;
		update_rect.bottom = screen_height-(window.y);
		update_rect.top = screen_height-(window.y+window.height);
		ok = AdjustWindowRectEx(&update_rect, style, FALSE, ex_style);
		assert(ok != 0, "AdjustWindowRectEx failed with error code %lu", GetLastError());
		
		u32 actual_x = update_rect.left;
		u32 actual_y = update_rect.top;
		u32 actual_width  = update_rect.right-update_rect.left;
		u32 actual_height = update_rect.bottom-update_rect.top;
		
		SetWindowPos(window._os_handle, 0, actual_x, actual_y, actual_width, actual_height, 0);
	}
	
	RECT client_rect;
	ok = GetClientRect(window._os_handle, &client_rect);
	assert(ok, "GetClientRect failed with error code %lu", GetLastError());
	
	// Convert the client area rectangle top-left corner to screen coordinates
	POINT top_left;
	top_left.x = client_rect.left;
	top_left.y = client_rect.top;
	ok = ClientToScreen(window._os_handle, &top_left);
	assert(ok, "ClientToScreen failed with error code %lu", GetLastError());
	
	// Convert the client area rectangle bottom-right corner to screen coordinates
	POINT bottom_right;
	bottom_right.x = client_rect.right;
	bottom_right.y = client_rect.bottom;
	ok = ClientToScreen(window._os_handle, &bottom_right);
	assert(ok, "ClientToScreen failed with error code %lu", GetLastError());

	
	window.x = (u32)top_left.x;
	window.y = (u32)(screen_height-bottom_right.y);
	window.width  = (u32)(client_rect.right - client_rect.left);
	window.height = (u32)(client_rect.bottom - client_rect.top);
	
	last_window = window;
	
	
	// Reflect what the backend did to input state before we query for OS inputs
	memcpy(win32_key_states, input_frame.key_states, sizeof(input_frame.key_states));
	input_frame.number_of_events = 0;
	
	// #Simd ?
	for (u64 i = 0; i < INPUT_KEY_CODE_COUNT; i++) {
		win32_key_states[i] &= ~(INPUT_STATE_REPEAT);
		win32_key_states[i] &= ~(INPUT_STATE_JUST_PRESSED);
		win32_key_states[i] &= ~(INPUT_STATE_JUST_RELEASED);
	}
	
	MSG msg;
	while (input_frame.number_of_events < MAX_EVENTS_PER_FRAME 
			&& PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            window.should_close = true;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
	memcpy(input_frame.key_states, win32_key_states, sizeof(input_frame.key_states));
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(window._os_handle, &p);
	p.y = window.height - p.y;
	input_frame.mouse_x = (float32)p.x;
	input_frame.mouse_y = (float32)p.y;
	
	if (window.should_close) {
		win32_window_proc(window._os_handle, WM_CLOSE, 0, 0);
	}
}

Input_Key_Code os_key_to_key_code(void* os_key) {

	UINT win32_key = (UINT)(u64)os_key;

	if (win32_key >= 'A' && win32_key <= 'Z') {
        return (Input_Key_Code)win32_key;
    }
    if (win32_key >= '0' && win32_key <= '9') {
        return (Input_Key_Code)win32_key;
    }

    switch (win32_key) {
        case VK_BACK:         return KEY_BACKSPACE;
        case VK_TAB:          return KEY_TAB;
        case VK_RETURN:       return KEY_ENTER;
        case VK_ESCAPE:       return KEY_ESCAPE;
        case VK_SPACE:        return KEY_SPACEBAR;
        case VK_DELETE:       return KEY_DELETE;
        case VK_UP:           return KEY_ARROW_UP;
        case VK_DOWN:         return KEY_ARROW_DOWN;
        case VK_LEFT:         return KEY_ARROW_LEFT;
        case VK_RIGHT:        return KEY_ARROW_RIGHT;
        case VK_PRIOR:        return KEY_PAGE_UP;
        case VK_NEXT:         return KEY_PAGE_DOWN;
        case VK_HOME:         return KEY_HOME;
        case VK_END:          return KEY_END;
        case VK_INSERT:       return KEY_INSERT;
        case VK_PAUSE:        return KEY_PAUSE;
        case VK_SCROLL:       return KEY_SCROLL_LOCK;
        case VK_MENU:         return KEY_ALT;
        case VK_CONTROL:      return KEY_CTRL;
        case VK_SHIFT:        return KEY_SHIFT;
        case VK_LWIN:         return KEY_CMD;
        case VK_RWIN:         return KEY_CMD;
        case VK_F1:           return KEY_F1;
        case VK_F2:           return KEY_F2;
        case VK_F3:           return KEY_F3;
        case VK_F4:           return KEY_F4;
        case VK_F5:           return KEY_F5;
        case VK_F6:           return KEY_F6;
        case VK_F7:           return KEY_F7;
        case VK_F8:           return KEY_F8;
        case VK_F9:           return KEY_F9;
        case VK_F10:          return KEY_F10;
        case VK_F11:          return KEY_F11;
        case VK_F12:          return KEY_F12;
        case VK_SNAPSHOT:     return KEY_PRINT_SCREEN;
        case VK_LBUTTON:      return MOUSE_BUTTON_LEFT;
        case VK_MBUTTON:      return MOUSE_BUTTON_MIDDLE;
        case VK_RBUTTON:      return MOUSE_BUTTON_RIGHT;
        default:              return KEY_UNKNOWN;
    }
}

void* key_code_to_os_key(Input_Key_Code key_code) {

	if (key_code >= 'A' && key_code <= 'Z') {
        return (void*)key_code;
    }
    if (key_code >= '0' && key_code <= '9') {
        return (void*)key_code;
    }

    switch (key_code) {
        case KEY_BACKSPACE:       return (void*)VK_BACK;
        case KEY_TAB:             return (void*)VK_TAB;
        case KEY_ENTER:           return (void*)VK_RETURN;
        case KEY_ESCAPE:          return (void*)VK_ESCAPE;
        case KEY_SPACEBAR:        return (void*)VK_SPACE;
        case KEY_DELETE:          return (void*)VK_DELETE;
        case KEY_ARROW_UP:        return (void*)VK_UP;
        case KEY_ARROW_DOWN:      return (void*)VK_DOWN;
        case KEY_ARROW_LEFT:      return (void*)VK_LEFT;
        case KEY_ARROW_RIGHT:     return (void*)VK_RIGHT;
        case KEY_PAGE_UP:         return (void*)VK_PRIOR;
        case KEY_PAGE_DOWN:       return (void*)VK_NEXT;
        case KEY_HOME:            return (void*)VK_HOME;
        case KEY_END:             return (void*)VK_END;
        case KEY_INSERT:          return (void*)VK_INSERT;
        case KEY_PAUSE:           return (void*)VK_PAUSE;
        case KEY_SCROLL_LOCK:     return (void*)VK_SCROLL;
        case KEY_ALT:             return (void*)VK_MENU;
        case KEY_CTRL:            return (void*)VK_CONTROL;
        case KEY_SHIFT:           return (void*)VK_SHIFT;
        case KEY_CMD:             return (void*)VK_LWIN; // Assuming left win key for both
        case KEY_F1:              return (void*)VK_F1;
        case KEY_F2:              return (void*)VK_F2;
        case KEY_F3:              return (void*)VK_F3;
        case KEY_F4:              return (void*)VK_F4;
        case KEY_F5:              return (void*)VK_F5;
        case KEY_F6:              return (void*)VK_F6;
        case KEY_F7:              return (void*)VK_F7;
        case KEY_F8:              return (void*)VK_F8;
        case KEY_F9:              return (void*)VK_F9;
        case KEY_F10:             return (void*)VK_F10;
        case KEY_F11:             return (void*)VK_F11;
        case KEY_F12:             return (void*)VK_F12;
        case KEY_PRINT_SCREEN:    return (void*)VK_SNAPSHOT;
        case MOUSE_BUTTON_LEFT:   return (void*)VK_LBUTTON;
        case MOUSE_BUTTON_MIDDLE: return (void*)VK_MBUTTON;
        case MOUSE_BUTTON_RIGHT:  return (void*)VK_RBUTTON;
        
        
        case INPUT_KEY_CODE_COUNT:
        case KEY_UNKNOWN: 
        	break;
    }
    
    panic("Invalid key code %d", key_code);
    return 0;
}