
void os_write_string_to_stdout(string s);
inline int crt_sprintf(char *str, const char *format, ...);
int vsnprintf(char* buffer, size_t n, const char* fmt, va_list args);
bool is_pointer_valid(void *p);

u64 format_string_to_buffer(char* buffer, u64 count, const char* fmt, va_list args) {
	if (!buffer) count = UINT64_MAX;
    const char* p = fmt;
    char* bufp = buffer;
    while (*p != '\0' && (bufp - buffer) < count - 1) {
        if (*p == '%') {
            p += 1;
            if (*p == 's') {
            	// We replace %s formatting with our fixed length string
                p += 1;
                string s = va_arg(args, string);
                assert(s.count < (1024ULL*1024ULL*1024ULL*256ULL), "Ypu passed something else than a fixed-length 'string' to %%s. Maybe you passed a char* and should do %%cs instead?");
                for (u64 i = 0; i < s.count && (bufp - buffer) < count - 1; i++) {
                	if (buffer) *bufp = s.data[i];
                    bufp += 1;
                }
            } else if (*p == 'c' && *(p+1) == 's') {
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
	string s = sprint_va_list(temp, fmt, args);
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
	string s = sprint_va_list(temp, sfmt, args);
	va_end(args);
	
	return s;
}

// print for 'string' and printf for 'char*'

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
#define LOG_BASE(level, ...) if (context.logger) ((Logger_Proc)context.logger)(level, tprint(__VA_ARGS__))


#define log_verbose(...) LOG_BASE(LOG_VERBOSE, __VA_ARGS__)
#define log_info(...)    LOG_BASE(LOG_INFO,    __VA_ARGS__)
#define log_warning(...) LOG_BASE(LOG_WARNING, __VA_ARGS__)
#define log_error(...)   LOG_BASE(LOG_ERROR,   __VA_ARGS__)

#define log(...) LOG_BASE(LOG_INFO, __VA_ARGS__)


typedef struct String_Builder {
	union {
		struct {u64 count;u8 *buffer;};
		string result;
	};
	u64 buffer_capacity;
	Allocator allocator;
} String_Builder;


void string_builder_reserve(String_Builder *b, u64 required_capacity) {
	if (b->buffer_capacity >= required_capacity) return;
	
	u64 new_capacity = max(b->buffer_capacity*2, (u64)(required_capacity*1.5));
	u8 *new_buffer = alloc(b->allocator, new_capacity);
	if (b->buffer) {
		memcpy(new_buffer, b->buffer, b->count);
		dealloc(b->allocator, b->buffer);
	}
	b->buffer = new_buffer;
	b->buffer_capacity = new_capacity;
}
void string_builder_init_reserve(String_Builder *b, u64 reserved_capacity, Allocator allocator) {
	reserved_capacity = max(reserved_capacity, 128);
	b->allocator = allocator;
	b->buffer_capacity = 0;
	b->buffer = 0;
	string_builder_reserve(b, reserved_capacity);
	b->count = 0;
}
void string_builder_init(String_Builder *b, Allocator allocator) {
	string_builder_init_reserve(b, 128, allocator);
}
void string_builder_append(String_Builder *b, string s) {
	assert(b->allocator.proc, "String_Builder is missing allocator");
	string_builder_reserve(b, b->count+s.count);
	
	memcpy(b->buffer+b->count, s.data, s.count);
	b->count += s.count;
}

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

string string_builder_get_string(String_Builder *b) {
	return b->result;
}

