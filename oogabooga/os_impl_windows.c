
#define CINTERFACE
#include <Shlwapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <initguid.h>
#include <avrt.h>

#define VIRTUAL_MEMORY_BASE ((void*)0x0000690000000000ULL)

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

void
win32_audio_init();
void 
win32_init_window() {
	memset(&window, 0, sizeof(window));
	
	window.title = STR("Unnamed Window");
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
    HINSTANCE instance = GetModuleHandle(0);
    assert(instance != INVALID_HANDLE_VALUE, "Failed getting current HINSTANCE");

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = win32_window_proc;
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "sigma balls";
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

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
        0, 0, instance, 0);
    assert(window._os_handle != 0, "Window creation failed, error: %lu", GetLastError());
	window._initialized = true;
    ShowWindow(window._os_handle, SW_SHOWDEFAULT);
    UpdateWindow(window._os_handle);
}

void 
win32_audio_thread(Thread *t);
void 
win32_audio_poll_default_device_thread(Thread *t);

bool win32_has_audio_thread_started = false;

void os_init(u64 program_memory_size) {
	
#if CONFIGURATION == DEBUG
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
#endif
	
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    win32_check_hr(hr);
	
	context.thread_id = GetCurrentThreadId();
	
	
	
#if CONFIGURATION == RELEASE
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	
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
            if (os.static_memory_start == 0) {
                os.static_memory_start = mbi.BaseAddress;
            }
            os.static_memory_end = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
        }
        addr += mbi.RegionSize;
    }


	program_memory_mutex = os_make_mutex();
	os_grow_program_memory(program_memory_size);
	
	heap_init();
	
	os.crt = os_load_dynamic_library(STR("msvcrt.dll"));
	assert(os.crt != 0, "Could not load win32 crt library. Might be compiled with non-msvc? #Incomplete #Portability");
	os.crt_vsnprintf = (Crt_Vsnprintf_Proc)os_dynamic_library_load_symbol(os.crt, STR("vsnprintf"));
	assert(os.crt_vsnprintf, "Missing vsnprintf in crt");
	
    win32_init_window();
    
    
    local_persist Thread audio_thread, audio_poll_default_device_thread;
    
    os_thread_init(&audio_thread, win32_audio_thread);
    os_thread_init(&audio_poll_default_device_thread, win32_audio_poll_default_device_thread);
    
    os_thread_start(&audio_thread);
    os_thread_start(&audio_poll_default_device_thread);
    
    while (!win32_has_audio_thread_started) { os_yield_thread(); }
}

void s64_to_null_terminated_string_reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

void s64_to_null_terminated_string(s64 num, char* str, int base)
{
    int i = 0;
    bool neg = false;
 
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
 
    if (num < 0 && base == 10) {
        neg = true;
        num = -num;
    }
 
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    if (neg)
        str[i++] = '-';
 
    str[i] = '\0';
    s64_to_null_terminated_string_reverse(str, i);
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
		// #Cleanup this mess
		// Allocation size doesn't actually need to be aligned to granularity, page size is enough.
		// Doesn't matter that much tho, but this is just a bit unfortunate to look at.
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
		assert(result == tail);
#if CONFIGURATION == DEBUG
		volatile u8 a = *(u8*)tail = 69;
#endif
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

	
	char size_str[32];
	s64_to_null_terminated_string(program_memory_size/1024, size_str, 10);
	
	os_write_string_to_stdout(STR("Program memory grew to "));
	os_write_string_to_stdout(STR(size_str));
	os_write_string_to_stdout(STR(" kb\n"));
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

#if CONFIGURATION == RELEASE
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

	Thread *t = (Thread*)param;
	temporary_storage_init();
	context = t->initial_context;
	context.thread_id = GetCurrentThreadId();
	t->proc(t);
	return 0;
}


////// DEPRECATED   vvvvvvvvvvvvvvvvv
Thread* os_make_thread(Thread_Proc proc, Allocator allocator) {
	Thread *t = (Thread*)alloc(allocator, sizeof(Thread));
	t->id = 0; // This is set when we start it
	t->proc = proc;
	t->initial_context = context;
	t->allocator = allocator;
	
	return t;
}
void os_destroy_thread(Thread *t) {
	os_join_thread(t);
	CloseHandle(t->os_handle);
	dealloc(t->allocator, t);
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
}
////// DEPRECATED   ^^^^^^^^^^^^^^^^

void os_thread_init(Thread *t, Thread_Proc proc) {
	memset(t, 0, sizeof(Thread));
	t->id = 0;
	t->proc = proc;
	t->initial_context = context;
}
void os_thread_destroy(Thread *t) {
	os_thread_join(t);
	CloseHandle(t->os_handle);
}
void os_thread_start(Thread *t) {
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
void os_thread_join(Thread *t) {
	WaitForSingleObject(t->os_handle, INFINITE);
}

///
// Mutex primitive

Mutex_Handle os_make_mutex() {
	local_persist const int MAX_ATTEMPTS = 100;
	 
	HANDLE m = CreateMutexW(0, FALSE, 0);

	int attempts = 1;	
	while (m == 0) {
		assert(attempts <= MAX_ATTEMPTS, "Failed creating win32 mutex. error %d", GetLastError());
		m = CreateMutex(0, FALSE, 0);
		attempts += 1;
	}
	
	return m;
}
void os_destroy_mutex(Mutex_Handle m) {
	CloseHandle(m);
}
void os_lock_mutex(Mutex_Handle m) {
	DWORD wait_result = WaitForSingleObject(m, INFINITE);
	
	switch (wait_result) {
        case WAIT_OBJECT_0:
            break;

        //case WAIT_ABANDONED:
        //    break;

        default:
        	assert(false, "Unexpected mutex lock result");
            break;
    }
}
void os_unlock_mutex(Mutex_Handle m) {
	BOOL result = ReleaseMutex(m);
	assert(result, "Unlock mutex 0x%x failed with error %d", m, GetLastError());
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
	s32 sleep_time = (s32)((end-start)-1.0);
	bool do_sleep = sleep_time >= 1;
	
	timeBeginPeriod(1);
	
	if (do_sleep)  os_sleep(sleep_time);
	
	while (os_get_current_time_in_seconds() < end) {
		os_yield_thread();
	}
	
	timeEndPeriod(1);
}


///
///
// Time
///


u64 os_get_current_cycle_count() {
	return rdtsc();
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
	
	WriteFile(win32_stdout, s.data, s.count, 0, 0);
}

u16 *win32_fixed_utf8_to_null_terminated_wide(string utf8, Allocator allocator) {

	if (utf8.count == 0) {
		u16 *utf16_str = (u16 *)alloc(allocator, (1) * sizeof(u16));
		*utf16_str = 0;
		return utf16_str;
	}

    u64 utf16_length = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8.data, (int)utf8.count, 0, 0);

    u16 *utf16_str = (u16 *)alloc(allocator, (utf16_length + 1) * sizeof(u16));

    int result = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)utf8.data, (int)utf8.count, utf16_str, utf16_length);
    if (result == 0) {
        dealloc(allocator, utf16_str);
        return 0;
    }

    utf16_str[utf16_length] = 0;

    return utf16_str;
}
u16 *temp_win32_fixed_utf8_to_null_terminated_wide(string utf8) {
	return win32_fixed_utf8_to_null_terminated_wide(utf8, temp);
}
string win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16, Allocator allocator) {
    u64 utf8_length = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)utf16, -1, 0, 0, 0, 0);

	if (utf8_length == 0) {
		string utf8;
		utf8.count = 0;
		utf8.data = 0;
		return utf8;
	}

    u8 *utf8_str = (u8 *)alloc(allocator, utf8_length * sizeof(u8));

    int result = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)utf16, -1, (LPSTR)utf8_str, (int)utf8_length, 0, 0);
    if (result == 0) {
        dealloc(allocator, utf8_str);
        return (string){0, 0};
    }

    string utf8;
    utf8.data = utf8_str;
    utf8.count = utf8_length-1;

    return utf8;
}

string temp_win32_null_terminated_wide_to_fixed_utf8(const u16 *utf16) {
    return win32_null_terminated_wide_to_fixed_utf8(utf16, temp);
}


File os_file_open_s(string path, Os_Io_Open_Flags flags) {
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

    return CreateFileW(wide, access, FILE_SHARE_READ, 0, creation, FILE_ATTRIBUTE_NORMAL, 0);
}

void os_file_close(File f) {
    CloseHandle(f);
}

bool os_file_delete_s(string path) {
	u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	return (bool)DeleteFileW(path_wide);
}

bool os_make_directory_s(string path, bool recursive) {
    wchar_t *wide_path = temp_win32_fixed_utf8_to_null_terminated_wide(path);

    // Convert forward slashes to backslashes
    for (wchar_t *p = wide_path; *p; ++p) {
        if (*p == L'/') {
            *p = L'\\';
        }
    }

    if (recursive) {
        wchar_t *sep = wcschr(wide_path + 1, L'\\');
        while (sep) {
            *sep = 0;
            if (!CreateDirectoryW(wide_path, 0) && GetLastError() != ERROR_ALREADY_EXISTS) {
                return false;
            }
            *sep = L'\\';
            sep = wcschr(sep + 1, L'\\');
        }
    }

    if (!CreateDirectoryW(wide_path, 0) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }

    return true;
}
bool os_delete_directory_s(string path, bool recursive) {
    wchar_t *wide_path = temp_win32_fixed_utf8_to_null_terminated_wide(path);

    if (recursive) {
        WIN32_FIND_DATAW findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        wchar_t search_path[MAX_PATH];
        wcscpy(search_path, wide_path);
        wcscat(search_path, L"\\*");

        hFind = FindFirstFileW(search_path, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        } else {
            do {
                if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
                    wchar_t child_path[MAX_PATH];
                    wcscpy(child_path, wide_path);
                    wcscat(child_path, L"\\");
                    wcscat(child_path, findFileData.cFileName);

                    if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (!os_delete_directory_s(temp_win32_null_terminated_wide_to_fixed_utf8(child_path), true)) {
                            FindClose(hFind);
                            return false;
                        }
                    } else {
                        if (!DeleteFileW(child_path)) {
                            FindClose(hFind);
                            return false;
                        }
                    }
                }
            } while (FindNextFileW(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
    }

    if (!RemoveDirectoryW(wide_path)) {
        return false;
    }

    return true;
}

bool os_file_write_string(File f, string s) {
    DWORD written;
    BOOL result = WriteFile(f, s.data, s.count, &written, 0);
    return result && (written == s.count);
}

bool os_file_write_bytes(File f, void *buffer, u64 size_in_bytes) {
    DWORD written;
    BOOL result = WriteFile(f, buffer, (DWORD)size_in_bytes, &written, 0);
    return result && (written == size_in_bytes);
}

bool os_file_read(File f, void* buffer, u64 bytes_to_read, u64 *actual_read_bytes) {
    DWORD read;
    BOOL result = ReadFile(f, buffer, (DWORD)bytes_to_read, &read, 0);
    if (actual_read_bytes) {
        *actual_read_bytes = read;
    }
    return result;
}

bool os_file_set_pos(File f, s64 pos_in_bytes) {
	if (pos_in_bytes < 0) return false;
    LARGE_INTEGER pos;
    pos.QuadPart = pos_in_bytes;
    return SetFilePointerEx(f, pos, NULL, FILE_BEGIN);
}

s64 
os_file_get_size(File f) {
	s64 backup_pos = os_file_get_pos(f);
	if (backup_pos < 0) return -1;

	LARGE_INTEGER file_size;
    file_size.QuadPart = 0;

    if (!SetFilePointerEx(f, file_size, &file_size, FILE_END)) {
    	os_file_set_pos(f, backup_pos);
        return -1;
    }

    // The new position of the file pointer is the size of the file
    u64 result = (u64)file_size.QuadPart;
    
    os_file_set_pos(f, backup_pos);
    
    return result;
}

s64 
os_file_get_size_from_path(string path) {
	File f = os_file_open(path, O_READ);
	if (f == OS_INVALID_FILE) return -1;
	
	s64 size = os_file_get_size(f);
	
	os_file_close(f);
	
	return size;
}

s64 os_file_get_pos(File f) {
    LARGE_INTEGER pos = {0};
    LARGE_INTEGER new_pos;
    if (SetFilePointerEx(f, pos, &new_pos, FILE_CURRENT)) {
        return new_pos.QuadPart;
    }
    return (s64)-1;
}

bool os_write_entire_file_handle(File f, string data) {
    return os_file_write_string(f, data);
}

bool os_write_entire_file_s(string path, string data) {
    File file = os_file_open_s(path, O_WRITE | O_CREATE);
    if (file == OS_INVALID_FILE) {
        return false;
    }
    bool result = os_file_write_string(file, data);
    os_file_close(file);
    return result;
}

bool os_read_entire_file_handle(File f, string *result, Allocator allocator) {
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(f, &file_size)) {
        return false;
    }
    
    u64 actual_read = 0;
    result->data = (u8*)alloc(allocator, file_size.QuadPart);
    result->count = file_size.QuadPart;
    
    bool ok = os_file_read(f, result->data, file_size.QuadPart, &actual_read);
    if (!ok) {
		dealloc(allocator, result->data);
		result->data = 0;
		return false;
	}
    
    return actual_read == file_size.QuadPart;
}

bool os_read_entire_file_s(string path, string *result, Allocator allocator) {
    File file = os_file_open_s(path, O_READ);
    if (file == OS_INVALID_FILE) {
        return false;
    }
    bool res = os_read_entire_file_handle(file, result, allocator);
    os_file_close(file);
    return res;
}

bool os_is_file_s(string path) {
	u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	assert(path_wide, "Invalid path string");
    if (path_wide == 0) {
        return false;
    }

    DWORD attributes = GetFileAttributesW(path_wide);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool os_is_directory_s(string path) {
    u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
	assert(path_wide, "Invalid path string");
    if (path_wide == 0) {
        return false;
    }

    DWORD attributes = GetFileAttributesW(path_wide);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool os_is_path_absolute(string path) {
	// #Incomplete #Portability not sure this is very robust.
	
    if (path.count < 2) return false;

    if (path.data[1] == ':' && ((path.data[0] >= 'A' && path.data[0] <= 'Z') || (path.data[0] >= 'a' && path.data[0] <= 'z'))) {
        return true;
    }

    if (path.count > 1 && path.data[0] == '\\' && path.data[1] == '\\') {
        return true;
    }

    return false;
}

bool os_get_absolute_path(string path, string *result, Allocator allocator) {
    u16 buffer[MAX_PATH];
    u16 *path_wide = temp_win32_fixed_utf8_to_null_terminated_wide(path);
    DWORD count = GetFullPathNameW(path_wide, MAX_PATH, buffer, 0);

    if (count == 0) {
        return false;
    }
    
	*result = win32_null_terminated_wide_to_fixed_utf8(buffer, allocator);
    
    return true;
}

bool os_get_relative_path(string from, string to, string *result, Allocator allocator) {
    
	if (!os_is_path_absolute(from)) {
		bool abs_ok = os_get_absolute_path(from, &from, temp);
		if (!abs_ok) return false;
	}
	if (!os_is_path_absolute(to)) {
		bool abs_ok = os_get_absolute_path(to, &to, temp);
		if (!abs_ok) return false;
	}
	
	u16 buffer[MAX_PATH];

	u16 *from_wide = temp_win32_fixed_utf8_to_null_terminated_wide(from);
	u16 *to_wide = temp_win32_fixed_utf8_to_null_terminated_wide(to);

	// #Speed is_file and is_directory potentially slow
	DWORD attr_from = os_is_file(from) ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
	DWORD attr_to   = os_is_file(to)   ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;

    BOOL success = PathRelativePathToW(buffer,
                                       from_wide, attr_from,
                                       to_wide, attr_to);
    if (!success) {
        return false;
    }

    u64 count = 0;
    while (buffer[count] != 0) count += 1;

	*result = win32_null_terminated_wide_to_fixed_utf8(buffer, allocator);
    
    return true;
}

bool os_do_paths_match(string a, string b) {
    wchar_t *wide_path_a = temp_win32_fixed_utf8_to_null_terminated_wide(a);
    wchar_t *wide_path_b = temp_win32_fixed_utf8_to_null_terminated_wide(b);

    wchar_t full_path_a[MAX_PATH];
    wchar_t full_path_b[MAX_PATH];

    // Get the full path for both paths
    if (!GetFullPathNameW(wide_path_a, MAX_PATH, full_path_a, 0)) {
        return false;
    }
    if (!GetFullPathNameW(wide_path_b, MAX_PATH, full_path_b, 0)) {
        return false;
    }

    // Compare the full paths
    if (wcscmp(full_path_a, full_path_b) == 0) {
        return true;
    }

    return false;
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

///
///
// Debug
///
#define WIN32_MAX_STACK_FRAMES 64
#define WIN32_MAX_SYMBOL_NAME_LENGTH 256
string *
os_get_stack_trace(u64 *trace_count, Allocator allocator) {
#if CONFIGURATION == DEBUG
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT context;
    STACKFRAME64 stack;
    memset(&stack, 0, sizeof(STACKFRAME64));

    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

#ifdef _M_IX86
    int machineType = IMAGE_FILE_MACHINE_I386;
    stack.AddrPC.Offset = context.Eip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Ebp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Esp;
    stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    int machineType = IMAGE_FILE_MACHINE_AMD64;
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rsp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    int machineType = IMAGE_FILE_MACHINE_IA64;
    stack.AddrPC.Offset = context.StIIP;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.IntSp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrBStore.Offset = context.RsBSP;
    stack.AddrBStore.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.IntSp;
    stack.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

    string *stack_strings = (string *)alloc(allocator, WIN32_MAX_STACK_FRAMES * sizeof(string));
    *trace_count = 0;

    for (int i = 0; i < WIN32_MAX_STACK_FRAMES; i++) {
        if (!StackWalk64(machineType, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            break;
        }

        DWORD64 displacement = 0;
        char buffer[sizeof(SYMBOL_INFO) + WIN32_MAX_SYMBOL_NAME_LENGTH * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = WIN32_MAX_SYMBOL_NAME_LENGTH;

        if (SymFromAddr(process, stack.AddrPC.Offset, &displacement, symbol)) {
            IMAGEHLP_LINE64 line;
            DWORD displacement_line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            char *result;
            if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &displacement_line, &line)) {
                u64 length = (u64)(symbol->NameLen + strlen(line.FileName) + 50);
                result = (char *)alloc(allocator, length);
                format_string_to_buffer_va(result, length, "%cs:%d: %cs", line.FileName, line.LineNumber, symbol->Name);
            } else {
                u64 length = (u64)(symbol->NameLen + 1);
                result = (char *)alloc(allocator, length);
                memcpy(result, symbol->Name, symbol->NameLen + 1);
            }
            stack_strings[*trace_count].data = (u8 *)result;
            stack_strings[*trace_count].count = strlen(result);
            (*trace_count)++;
        } else {
            stack_strings[*trace_count].data = (u8 *)alloc(allocator, 32);
            stack_strings[*trace_count].count = format_string_to_buffer_va((char *)stack_strings[*trace_count].data, 32, "0x%llx", stack.AddrPC.Offset);
            (*trace_count)++;
        }
    }

    return stack_strings;
#else // DEBUG
	
	*trace_count = 1;
	string *result = alloc(allocator, 3+sizeof(string));
	result->count = 3;
	result->data = (u8*)result+sizeof(string);
	string s = STR("<0>");
	memcpy(result->data, s.data, 3);
	return result;
	
#endif // NOT DEBUG
}

// Actually fuck you bill gates
const GUID CLSID_MMDeviceEnumerator = {0xbcde0395, 0xe52f, 0x467c, {0x8e,0x3d, 0xc4,0x57,0x92,0x91,0x69,0x2e}};
const GUID IID_IMMDeviceEnumerator = {0xa95664d2, 0x9614, 0x4f35, {0xa7,0x46, 0xde,0x8d,0xb6,0x36,0x17,0xe6}};
const GUID IID_IAudioClient = {0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1,0x78, 0xc2,0xf5,0x68,0xa7,0x03,0xb2}};
const GUID IID_IAudioRenderClient = {0xf294acfc, 0x3146, 0x4483, {0xa7,0xbf, 0xad,0xdc,0xa7,0xc2,0x60,0xe2}};
DEFINE_GUID(IID_ISimpleAudioVolume, 
0x87CE5498, 0x68D6, 0x44E5, 0x92, 0x15, 0x6D, 0xA4, 0x7E, 0xF8, 0x83, 0xD8);

IAudioClient* win32_audio_client;
IAudioRenderClient* win32_render_client;
bool win32_audio_deactivated = false;
Audio_Format audio_output_format; // For use when loading audio sources
IMMDevice* win32_audio_device = 0;
IMMDeviceEnumerator* win32_device_enumerator = 0;
ISimpleAudioVolume* win32_audio_volume = 0;
Mutex audio_init_mutex;

void
win32_audio_init() {

	win32_audio_client = 0;
	win32_render_client = 0;
	win32_audio_deactivated = 0;
	win32_audio_device = 0;
	win32_device_enumerator = 0;

	HRESULT hr;
    WAVEFORMATEX* device_base_format = 0;
    WAVEFORMATEX* output_format = 0;

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&win32_device_enumerator);
    win32_check_hr(hr);
    
    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(win32_device_enumerator, eRender, eConsole, &win32_audio_device);
    win32_check_hr(hr);
    
    hr = IMMDevice_Activate(
    	win32_audio_device, 
		&IID_IAudioClient, 
		CLSCTX_ALL, 0, 
		(void**)&win32_audio_client
	);
    win32_check_hr(hr);
    
    hr = IAudioClient_GetMixFormat(win32_audio_client, &device_base_format);
    
     WAVEFORMATEXTENSIBLE *format_f32 
     	= (WAVEFORMATEXTENSIBLE*)CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE));
     WAVEFORMATEXTENSIBLE *format_s16 
     	= (WAVEFORMATEXTENSIBLE*)CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE));
     	
    memcpy(format_f32, device_base_format, sizeof(WAVEFORMATEX));
    
    format_f32->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    format_f32->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
    format_f32->Samples.wValidBitsPerSample = format_f32->Format.wBitsPerSample;
    format_f32->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    
    memcpy(format_s16, format_f32, sizeof(WAVEFORMATEXTENSIBLE));
    
    format_f32->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    format_f32->Format.wBitsPerSample = 32;
    format_f32->Format.nBlockAlign 
    	= format_f32->Format.nChannels * format_f32->Format.wBitsPerSample / 8;
    format_f32->Format.nAvgBytesPerSec 
    	= format_f32->Format.nSamplesPerSec * format_f32->Format.nBlockAlign;
    
    format_s16->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    format_s16->Format.wBitsPerSample = 16;
    format_s16->Format.nBlockAlign = format_s16->Format.nChannels * format_s16->Format.wBitsPerSample / 8;
    format_s16->Format.nAvgBytesPerSec = format_s16->Format.nSamplesPerSec * format_s16->Format.nBlockAlign;
    
    // First look for f32 support
    WAVEFORMATEX *closest_match = NULL;
    hr = IAudioClient_IsFormatSupported(
    	win32_audio_client, 
    	AUDCLNT_SHAREMODE_SHARED, 
    	(WAVEFORMATEX *)format_f32, 
    	&closest_match
	);
	
	// If f32 fails, look for s16
	if (hr != S_OK) {	
	    hr = IAudioClient_IsFormatSupported(
	    	win32_audio_client, 
	    	AUDCLNT_SHAREMODE_SHARED, 
	    	(WAVEFORMATEX *)format_s16,
	    	&closest_match
		);
		if (hr != S_OK) {
			win32_audio_deactivated = true;
			log_error("Default audio output device is not supported.");
			return;
		}
		output_format = (WAVEFORMATEX*)format_s16;
	} else {
		output_format = (WAVEFORMATEX*)format_f32;
	}
	
	const s64 BUFFER_DURATION_MS = 40;
	hr = IAudioClient_Initialize(
    	win32_audio_client, 
    	AUDCLNT_SHAREMODE_SHARED, 
    	0, 
    	BUFFER_DURATION_MS*10000ll, 0, 
    	output_format, 0
	);
    win32_check_hr(hr);
	
    hr = IAudioClient_GetService(win32_audio_client, &IID_IAudioRenderClient, (void**)&win32_render_client);
    win32_check_hr(hr);
    
    audio_output_format.channels = output_format->nChannels;
    audio_output_format.sample_rate = output_format->nSamplesPerSec;
    if (output_format == (WAVEFORMATEX*)format_s16) {
    	audio_output_format.bit_width = AUDIO_BITS_16;
    } else if (output_format == (WAVEFORMATEX*)format_f32) {
    	audio_output_format.bit_width = AUDIO_BITS_32;
    } else {
    	panic("What");
    }
    
    DWORD task_index;
	AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &task_index);

	hr = IAudioClient_GetService(win32_audio_client, &IID_ISimpleAudioVolume, (void**)&win32_audio_volume);
	if (SUCCEEDED(hr)) {
	    hr = ISimpleAudioVolume_SetMasterVolume(win32_audio_volume, 1.0f, 0);
	    win32_check_hr(hr);
	    ISimpleAudioVolume_Release(win32_audio_volume);
	}

    log_info("Successfully initialized default audio device. Channels: %d, sample_rate: %d, bits: %d", audio_output_format.channels, audio_output_format.sample_rate, get_audio_bit_width_byte_size(audio_output_format.bit_width)*8);
}


void
win32_audio_poll_default_device_thread(Thread *t) {
	while (!win32_has_audio_thread_started) {
		MEMORY_BARRIER;
		os_yield_thread();
	}

	while (!window.should_close) {
		while (win32_audio_deactivated) {
			os_sleep(100);
		}
		
		mutex_acquire_or_wait(&audio_init_mutex);
		MEMORY_BARRIER;
	    mutex_release(&audio_init_mutex);
		MEMORY_BARRIER;
	
		IMMDevice *now_default = 0;
		HRESULT hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(win32_device_enumerator, eRender, eConsole, &now_default);
		win32_check_hr(hr);
		
		WCHAR *now_default_id = NULL;
	    hr = IMMDevice_GetId(now_default, &now_default_id);
	    win32_check_hr(hr);
	    
	    WCHAR *previous_id = NULL;
	    hr = IMMDevice_GetId(win32_audio_device, &previous_id);
	    win32_check_hr(hr);
	    
	    if (wcscmp(now_default_id, previous_id) != 0) {
	        win32_audio_deactivated = true;
	    }
	    
	    CoTaskMemFree(now_default_id);
	    CoTaskMemFree(previous_id);
	
	    IMMDevice_Release(now_default);
	    
		os_sleep(100);
		
	}

}
void 
win32_audio_thread(Thread *t) {
	
	mutex_init(&audio_init_mutex);
	
    mutex_acquire_or_wait(&audio_init_mutex);
    win32_has_audio_thread_started = true;
    MEMORY_BARRIER;
	win32_audio_init();
    mutex_release(&audio_init_mutex);
        
    timeBeginPeriod(1);
	
	u32 buffer_frame_count;
    HRESULT hr = IAudioClient_GetBufferSize(win32_audio_client, &buffer_frame_count);
    if (FAILED(hr)) win32_audio_deactivated = true;
    
    bool started = false;
    
	while (!window.should_close) tm_scope("Audio update") {
		if (win32_audio_deactivated) tm_scope("Retry audio device") {
			os_sleep(100);
			mutex_acquire_or_wait(&audio_init_mutex);
			win32_audio_init();
			mutex_release(&audio_init_mutex);
			started = false;
			if (win32_audio_deactivated) {
				hr = IAudioClient_GetBufferSize(win32_audio_client, &buffer_frame_count);
	    		if (FAILED(hr)) win32_audio_deactivated = true;
			}
			continue;
		}
        
        if (win32_audio_deactivated) continue;
		
		BYTE *buffer = 0;
		
    	u32 num_frames_available = 0;
    	hr = IAudioClient_GetCurrentPadding(win32_audio_client, &num_frames_available);
		if (FAILED(hr)) {
			win32_audio_deactivated = true;
			continue;
		}
    	u32 num_frames_to_write = buffer_frame_count - num_frames_available;
    	
    	if (!started) {
	    	hr = IAudioClient_Start(win32_audio_client);
	    	win32_check_hr(hr);
	    	started = true;
    	}
    	
    	while (num_frames_to_write == 0) tm_scope("Chill") {
    		// We yield & sleep until we have any work to do
    		os_yield_thread();
    		os_sleep(1);
			
	    	hr = IAudioClient_GetCurrentPadding(win32_audio_client, &num_frames_available);
			if (FAILED(hr)) {
				win32_audio_deactivated = true;
				break;
			}
	    	num_frames_to_write = buffer_frame_count - num_frames_available;
	    	
    	}
    	if (win32_audio_deactivated) continue;
		
		
		if (num_frames_to_write > 0) tm_scope("Output frames") {
			hr = IAudioRenderClient_GetBuffer(
				win32_render_client, 
				num_frames_to_write, 
				&buffer
			);
			if (FAILED(hr)) {
				win32_audio_deactivated = true;
				continue;
			}
			
			do_program_audio_sample(num_frames_to_write, audio_output_format, buffer);
			//f32 s = 0.5;
			//for (u32 i = 0; i < num_frames_to_write * audio_output_format.channels; ++i) {
			//	((f32*)buffer)[i] = s;
			//}
			/*float64 time = 0;
			float *fbuffer = (float *)buffer;
			for (UINT32 frameIndex = 0; frameIndex < num_frames_to_write; frameIndex++) {
	            float amplitude = (float)(sin(time)*0.2);
	
	            *fbuffer++ = amplitude; // left
	            *fbuffer++ = amplitude; // right
	
	            time += 0.05;
	        }*/
			
			
			//for (u64 i = 0; i < num_frames_to_write; i++) {
			//	f32 s = *(((f32*)buffer)+i*audio_output_format.channels);
			//	print("%f ", s);
			//}
			hr = IAudioRenderClient_ReleaseBuffer(
				win32_render_client, 
				num_frames_to_write, 
				0
			);
			if (FAILED(hr)) {
				win32_audio_deactivated = true;
				continue;
			}
			
		}
        
	}
}

void os_update() {

	UINT dpi = GetDpiForWindow(window._os_handle);
    float dpi_scale_factor = dpi / 96.0f;

	local_persist Os_Window last_window;

	if (!strings_match(last_window.title, window.title)) {
		SetWindowText(window._os_handle, temp_convert_to_null_terminated_string(window.title));
	}

	if (last_window.scaled_width != window.scaled_width || last_window.scaled_height != window.scaled_height) {
		window.width = window.scaled_width*dpi_scale_factor;
		window.height = window.scaled_height*dpi_scale_factor;
	}
	
	BOOL ok;
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	DWORD style = (DWORD)GetWindowLong(window._os_handle, GWL_STYLE);
	DWORD ex_style = (DWORD)GetWindowLong(window._os_handle, GWL_EXSTYLE);
	if (last_window.x != window.x || last_window.y != window.y || last_window.width != window.width || last_window.height != window.height) {
	    RECT update_rect;
	    update_rect.left = window.x;
	    update_rect.right = window.x + window.width;
	    update_rect.top = window.y;
	    update_rect.bottom = window.y + window.height; 
	
	    BOOL ok = AdjustWindowRectEx(&update_rect, style, FALSE, ex_style);
	    assert(ok != 0, "AdjustWindowRectEx failed with error code %lu", GetLastError());
	
	    u32 actual_width = update_rect.right - update_rect.left;
	    u32 actual_height = update_rect.bottom - update_rect.top;
	    u32 actual_x = update_rect.left;
	    u32 actual_y = screen_height - update_rect.top - (update_rect.bottom - update_rect.top);
	    
	    SetWindowPos(window._os_handle, 0, actual_x, actual_y, actual_width, actual_height, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	
	RECT client_rect;
	ok = GetClientRect(window._os_handle, &client_rect);
	assert(ok, "GetClientRect failed with error code %lu", GetLastError());
	
	RECT adjusted_rect = client_rect;
	ok = AdjustWindowRectEx(&adjusted_rect, style, FALSE, ex_style);
    assert(ok != 0, "AdjustWindowRectEx failed with error code %lu", GetLastError());
    
    RECT window_rect;
	ok = GetWindowRect(window._os_handle, &window_rect);
	assert(ok, "GetWindowRect failed with error code %lu", GetLastError());
	
	/*u32 style_space_left =   abs(client_rect.left-adjusted_rect.left);
	u32 style_space_right =  abs(client_rect.left-adjusted_rect.right);
	u32 style_space_bottom = abs(client_rect.left-adjusted_rect.bottom);
	u32 style_space_top =    abs(client_rect.left-adjusted_rect.top);
	
	framebuffer_rect.left += style_space_left;
	framebuffer_rect.right -= style_space_right;
	framebuffer_rect.top += style_space_top;
	framebuffer_rect.bottom -= style_space_bottom;*/
	
	POINT top_left;
	top_left.x = client_rect.left;
	top_left.y = client_rect.top;
	ok = ClientToScreen(window._os_handle, &top_left);
	assert(ok, "ClientToScreen failed with error code %lu", GetLastError());
	
	POINT bottom_right;
	bottom_right.x = client_rect.right;
	bottom_right.y = client_rect.bottom;
	ok = ClientToScreen(window._os_handle, &bottom_right);
	assert(ok, "ClientToScreen failed with error code %lu", GetLastError());
	
	window.pixel_width = (u32)(bottom_right.x - top_left.x);
	window.pixel_height = (u32)(bottom_right.y - top_left.y);
	window.x = (u32)window_rect.left;
	window.y = screen_height-window_rect.bottom;
    
    window.scaled_width = (u32)((bottom_right.x - top_left.x) * dpi_scale_factor);
    window.scaled_height = (u32)((bottom_right.y - top_left.y) * dpi_scale_factor);
	
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
			&& PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
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
	input_frame.mouse_x = p.x;
	input_frame.mouse_y = p.y;
	
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