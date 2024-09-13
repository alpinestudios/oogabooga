
/*

	Format a string and print to stdout:
		print(string fmt, ...)
		
	Allocate a new string and format into it:
		sprint(Allocator allocator, string fmt, ...)

	Allocate a new string with the temporary allocator and format into it:
		tprint(string fmt, ...)
		
	Example:
		print("Int: %d, Float: %f, String: %s", my_int, my_float, my_string);
		
	Format specifiers:
		%d, %i: Any SIGNED integer
		%u    : Any UNSIGNED integer
		%f    : Float32 or float64
		%s    : string
		%b    : bool
		%c    : Character
		%v2   : Vector2
		%v3   : Vector3
		%v4   : Vector4
		
	Also includes all of the standard C printf-like format specifiers:
	https://www.geeksforgeeks.org/format-specifiers-in-c/
*/

ogb_instance void os_write_string_to_stdout(string s);
inline int crt_sprintf(char *str, const char *format, ...);
int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args);
bool is_pointer_valid(void *p);

u64 format_string_to_buffer(char* buffer, u64 count, const char* fmt, va_list args);
u64 format_string_to_buffer_vararg(char* buffer, u64 count, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	u64 n = format_string_to_buffer(buffer, count, fmt, args);
	va_end(args);
	return n;
}
typedef struct _8_Bytes {u8 _[8];} _8_Bytes;
typedef struct _12_Bytes {u8 _[12];} _12_Bytes;
typedef struct _16_Bytes {u8 _[16];} _16_Bytes;
u64 format_string_to_buffer(char* buffer, u64 count, const char* fmt, va_list args) {
	if (!buffer) count = UINT64_MAX;
    const char* p = fmt;
    char* bufp = buffer;
    while (*p != '\0' && (bufp - buffer) < count - 1) {
        if (*p == '%') {
            p += 1;
            if (*p == 's') {
            	p += 1;
            	// We replace %s formatting with our fixed length string (if it is a valid such, otherwise treat as char*)
				va_list args2; // C varargs are so good
				va_copy(args2, args);
                string s = va_arg(args2, string);
                va_end(args2);
            	// Ooga booga moment
            	bool is_valid_fixed_length_string = s.count < 1024ULL*1024ULL*1024ULL*256ULL && is_pointer_valid(s.data);
            	if (is_valid_fixed_length_string) {
            		va_arg(args, string);
	                for (u64 i = 0; i < s.count && (bufp - buffer) < count - 1; i++) {
	                	if (buffer) *bufp = s.data[i];
	                    bufp += 1;
	                }
                } else {
	            	// #Copypaste
	            	// We extend the standard formatting and add %cs so we can format c strings if we need to
	                char* s = va_arg(args, char*);
	                u64 len = 0;
	                while (*s != '\0' && (bufp - buffer) < count - 1) {
	                	assert(len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
	                	if (buffer) {
	                		*bufp = *s;
	                	}
	                	s += 1;
	                    bufp += 1;
	                    len += 1;
	                    assert(len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
                    }
                }
            } else if (*p == 'c' && *(p+1) == 's') {
            	// #Copypaste
            	// We extend the standard formatting and add %cs so we can format c strings if we need to
                p += 2;
                char* s = va_arg(args, char*);
                u64 len = 0;
                while (*s != '\0' && (bufp - buffer) < count - 1) {
                	assert(len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
                	if (buffer) {
                		*bufp = *s;
                	}
                	s += 1;
                    bufp += 1;
                    len += 1;
                    assert(len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
                }
            } else if (*p == 'b') {
            	p += 1;
            	int data = va_arg(args, int);
            	bool val = !(data == 0);
            	
            	char *result = (val ? "true" : "false");
            	
            	if (buffer) {
            		memcpy(bufp, result, strlen(result));
            	}
        		bufp += strlen(result);
            	
            } else if (*p == 'v' && *(p+1) == '2') {
            	p += 2;
            	
            	_8_Bytes data = va_arg(args, _8_Bytes);
            	
            	f32 x = *(((f32*)(&data))+0);
            	f32 y = *(((f32*)(&data))+1);
            	
                u64 n = format_string_to_buffer_vararg(buffer ? bufp : 0, 128, "{ X: %f, Y: %f }", x, y);
                
                bufp += n;
            } else if (*p == 'v' && *(p+1) == '3') {
            	p += 2;
            	
            	_12_Bytes data = va_arg(args, _12_Bytes);
            	
            	f32 x = *(((f32*)(&data))+0);
            	f32 y = *(((f32*)(&data))+1);
            	f32 z = *(((f32*)(&data))+2);
            	
                u64 n = format_string_to_buffer_vararg(buffer ? bufp : 0, 196, "{ X: %f, Y: %f, Z: %f }", x, y, z);
                
                bufp += n;
            } else if (*p == 'v' && *(p+1) == '4') {
            	p += 2;
            	
            	_16_Bytes data = va_arg(args, _16_Bytes);
            	
            	f32 x = *(((f32*)(&data))+0);
            	f32 y = *(((f32*)(&data))+1);
            	f32 z = *(((f32*)(&data))+2);
            	f32 w = *(((f32*)(&data))+3);
            	
                u64 n = format_string_to_buffer_vararg(buffer ? bufp : 0, 256, "{ X: %f, Y: %f, Z: %f, W: %f }", x, y, z, w);
                
                bufp += n;
            } else {
                // Fallback to standard vsnprintf
                char temp_buffer[512];
                char format_specifier[64];
                int specifier_len = 0;
                format_specifier[specifier_len++] = '%';

                while (*p != '\0' && strchr("diuoxXfFeEgGaAcCpn%", *p) == NULL) {
                    format_specifier[specifier_len++] = *p++;
                }
                if (*p != '\0') {
                    format_specifier[specifier_len++] = *p++;
                }
                format_specifier[specifier_len] = '\0';

                int temp_len = vsnprintf(temp_buffer, sizeof(temp_buffer), format_specifier, args);
                switch (format_specifier[specifier_len - 1]) {
                    case 'd': case 'i': va_arg(args, int); break;
                    case 'u': case 'x': case 'X': case 'o': va_arg(args, unsigned int); break;
                    case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A': va_arg(args, double); break;
                    case 'c': va_arg(args, int); break;
                    case 's': va_arg(args, char*); break;
                    case 'p': va_arg(args, void*); break;
                    case 'n': va_arg(args, int*); break;
                    default: break;
                }
                
                if (temp_len < 0) {
                    return -1; // Error in formatting
                }

                for (int i = 0; i < temp_len && (bufp - buffer) < count - 1; i++) {
                    if (buffer) *bufp = temp_buffer[i];
                    bufp += 1;
                }
            }
        } else {
            if (buffer) {
                *bufp = *p;
            }
            bufp += 1;
            p += 1;
        }
    }
    if (buffer)  *bufp = '\0';
    
    return bufp - buffer;
}
u64 format_string_to_buffer_va(char* buffer, u64 count, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	u64 r = format_string_to_buffer(buffer, count, fmt, args);
	va_end(args);
	return r;
}
string sprint_null_terminated_string_va_list_to_buffer(const char *fmt, va_list args, void* buffer, u64 buffer_size) {
    u64 formatted_length = format_string_to_buffer((char*)buffer, buffer_size, fmt, args);
    
    string result;
    result.data = (u8*)buffer;
    
    if (formatted_length >= 0 && formatted_length < buffer_size) {
        result.count = formatted_length; 
    } else {
        result.count = buffer_size - 1; 
    }

    return result;
}
string sprint_va_list_to_buffer(const string fmt, va_list args, void* buffer, u64 buffer_size) {
	
	char* fmt_cstring = temp_convert_to_null_terminated_string(fmt);
	return sprint_null_terminated_string_va_list_to_buffer(fmt_cstring, args, buffer, buffer_size);
}

string sprint_va_list(Allocator allocator, const string fmt, va_list args) {

    char* fmt_cstring = temp_convert_to_null_terminated_string(fmt);
    u64 count = format_string_to_buffer(NULL, 0, fmt_cstring, args) + 1; 

    char* buffer = NULL;

    buffer = (char*)alloc(allocator, count);

    return sprint_null_terminated_string_va_list_to_buffer(fmt_cstring, args, buffer, count);
}


string sprints(Allocator allocator, const string fmt, ...) {
	va_list args = 0;
	va_start(args, fmt);
	string s = sprint_va_list(allocator, fmt, args);
	va_end(args);
	return s;
}

// temp allocator
string tprints(const string fmt, ...) {
	va_list args = 0;
	va_start(args, fmt);
	string s = sprint_va_list(get_temporary_allocator(), fmt, args);
	va_end(args);
	return s;
}


string sprintf(Allocator allocator, const char *fmt, ...) {
	string sfmt;
	sfmt.data = cast(u8*)fmt;
	sfmt.count = strlen(fmt);
	
	va_list args;
	va_start(args, fmt);
	string s = sprint_va_list(allocator, sfmt, args);
	va_end(args);
	
	return s;
}
// temp allocator
string tprintf(const char *fmt, ...) {
	string sfmt;
	sfmt.data = cast(u8*)fmt;
	sfmt.count = strlen(fmt);
	
	va_list args;
	va_start(args, fmt);
	string s = sprint_va_list(get_temporary_allocator(), sfmt, args);
	va_end(args);
	
	return s;
}

// prints for 'string' and printf for 'char*'

#define PRINT_BUFFER_SIZE 4096
// Avoids all and any allocations but overhead in speed and memory.
// Need this for standard printing so we don't get infinite recursions.
// (for example something in memory might fail assert and it needs to print that)
void print_va_list_buffered(const string fmt, va_list args) {

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
		os_write_string_to_stdout(s);
		
		current.count -= size;
		current.data += size;
	}
}



void prints(const string fmt, ...) {
	va_list args;
	va_start(args, fmt);
	print_va_list_buffered(fmt, args);
	va_end(args);	
}

void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	string s;
	s.data = cast(u8*)fmt;
	s.count = strlen(fmt);
	print_va_list_buffered(s, args);
	va_end(args);
}




typedef void(*Logger_Proc)(Log_Level level, string s);
#define LOG_BASE(level, ...) if (get_context().logger) ((Logger_Proc)get_context().logger)(level, tprint(__VA_ARGS__))


#define log_verbose(...) LOG_BASE(LOG_VERBOSE, __VA_ARGS__)
#define log_info(...)    LOG_BASE(LOG_INFO,    __VA_ARGS__)
#define log_warning(...) LOG_BASE(LOG_WARNING, __VA_ARGS__)
#define log_error(...)   LOG_BASE(LOG_ERROR,   __VA_ARGS__)

#define log(...) LOG_BASE(LOG_INFO, __VA_ARGS__)




void string_builder_prints(String_Builder *b, string fmt, ...) {
	assert(b->allocator.proc, "String_Builder is missing allocator");
	
	va_list args1 = 0;
	va_start(args1, fmt);
	va_list args2 = 0;
	va_copy(args2, args1);
	
	u64 formatted_count = format_string_to_buffer(0, 0, temp_convert_to_null_terminated_string(fmt), args1);
	
	va_end(args1);
	
	string_builder_reserve(b, b->count+formatted_count+1);
	
	format_string_to_buffer((char*)b->buffer+b->count, b->count+formatted_count+1, temp_convert_to_null_terminated_string(fmt), args2);
	b->count += formatted_count;
	
	va_end(args2);
}
void string_builder_printf(String_Builder *b, const char *fmt, ...) {
	assert(b->allocator.proc, "String_Builder is missing allocator");
	
	va_list args1 = 0;
	va_start(args1, fmt);
	va_list args2 = 0;
	va_copy(args2, args1);
	
	u64 formatted_count = format_string_to_buffer(0, 0, fmt, args1);
	
	va_end(args1);
	
	string_builder_reserve(b, b->count+formatted_count+1);
	
	format_string_to_buffer((char*)b->buffer+b->count, b->buffer_capacity-b->count+1, fmt, args2);
	b->count += formatted_count;
	
	va_end(args2);
}

#define string_builder_print(...) _Generic((SECOND_ARG(__VA_ARGS__)), \
                           string:  string_builder_prints, \
                           default: string_builder_printf \
                          )(__VA_ARGS__)