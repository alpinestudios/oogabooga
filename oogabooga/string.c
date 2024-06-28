
/*
Usage:

	// We need to use macro const_string to convert literal to string
	string fmt = const_string("Pointer address: 0x%x");
	print(fmt, cast(u64)&a); // Print to stdout
	
	// Format a string and allocate with context.allocator
	string a = sprint("Hello, %cs!\n", "balls"); // %cs for char*
	
	string balls = const_string("balls");
	// tprint for temporary allocation
	string b = tprint(const_string("Hello, %s!\n"), balls); // %s for string
	
	// Allocate a new string of length 12 (with context allocator)
	string c = alloc_string(12);
	dealloc_string(c);
	
	// We can use raw char* for format with printf/sprintf/tprintf
	printf("Hello, %!\n", balls);
	
	// concatenation
	string concatenated = string_concat(a, b);
	
	// Use temporary memory to make a null-terminated copy of fixed-length string
	char* cstring = temp_convert_to_null_terminated_string(balls);
	
	// To convert a cstring to string (using same memory)
	string s;
	s.data = (u8*)cstring;
	s.count = strlen(cstring);
	
	// String matching
	bool match = strings_match(a, b);
	
	// View into "Hello, balls!\n" from index 7 with a count of 5; "balls"
	string balls2 = string_view(a, 7, 5);
	
	
*/

void * memcpy (void *,const void *,size_t);
void* talloc(u64);

typedef struct string {
	u64 count;
	u8 *data;
} string;

void push_temp_allocator();

#define cstr const_string
#define const_string(s) (string){ length_of_null_terminated_string(s), (u8*)s }

inline u64 length_of_null_terminated_string(const char* cstring) {
	u64 len = 0;
	while (*cstring != 0) {
		len += 1;
		cstring += 1;
	}
	return len;
}

string alloc_string(u64 count) {
	string s;
	s.count = count;
	s.data = cast(u8*)alloc(count);
	return s;
}
void dealloc_string(string s) {
	dealloc(s.data);
}

// context.allocator !
string string_concat(const string left, const string right) {
	string result;
	result.count = left.count + right.count;
	result.data = cast(u8*)alloc(result.count);
	memcpy(result.data, left.data, left.count);
	memcpy(result.data+left.count, right.data, right.count);
	return result;
}
// context.allocator !
char *convert_to_null_terminated_string(const string s) {
	char *cstring = cast(char*)alloc(s.count+1);
	memcpy(cstring, s.data, s.count);
	cstring[s.count] = 0;
	return cstring;
}

char *temp_convert_to_null_terminated_string(const string s) {
	push_temp_allocator();
	char *c = convert_to_null_terminated_string(s);
	pop_allocator();
	return c;
}
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
                assert(is_pointer_valid(s), "You passed an invalid pointer to %%cs");
                u64 len = 0;
                while (*s != '\0' && (bufp - buffer) < count - 1) {
                	assert(is_pointer_valid(s) && len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
                	if (buffer) {
                		*bufp = *s;
                	}
                	s += 1;
                    bufp += 1;
                    len += 1;
                    assert(is_pointer_valid(s) && len < (1024ULL*1024ULL*1024ULL*1ULL), "The argument passed to %%cs is either way too big, missing null-termination or simply not a char*.");
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
string sprint_null_terminated_string_va_list_to_buffer(const char *fmt, va_list args, void* buffer, u64 count) {
    u64 formatted_length = format_string_to_buffer((char*)buffer, count, fmt, args);
    
    string result;
    result.data = (u8*)buffer;
    
    if (formatted_length >= 0 && formatted_length < count) {
        result.count = formatted_length; 
    } else {
        result.count = count - 1; 
    }

    return result;
}
string sprint_va_list_to_buffer(const string fmt, va_list args, void* buffer, u64 count) {
	
	char* fmt_cstring = temp_convert_to_null_terminated_string(fmt);
	return sprint_null_terminated_string_va_list_to_buffer(fmt_cstring, args, buffer, count);
}
// context.allocator
string sprint_va_list(const string fmt, va_list args) {

    char* fmt_cstring = temp_convert_to_null_terminated_string(fmt);
    u64 count = format_string_to_buffer(NULL, 0, fmt_cstring, args) + 1; 

    char* buffer = NULL;

    buffer = (char*)alloc(count);

    return sprint_null_terminated_string_va_list_to_buffer(fmt_cstring, args, buffer, count);
}

// context.allocator
string sprint(const string fmt, ...) {
	va_list args = 0;
	va_start(args, fmt);
	string s = sprint_va_list(fmt, args);
	va_end(args);
	return s;
}

// temp allocator
string tprint(const string fmt, ...) {
	va_list args = 0;
	va_start(args, fmt);
	push_temp_allocator();
	string s = sprint_va_list(fmt, args);
	pop_allocator();
	va_end(args);
	return s;
}

// context.allocator
string sprintf(const char *fmt, ...) {
	string sfmt;
	sfmt.data = cast(u8*)fmt;
	sfmt.count = strlen(fmt);
	
	va_list args;
	va_start(args, fmt);
	string s = sprint_va_list(sfmt, args);
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
	push_temp_allocator();
	string s = sprint_va_list(sfmt, args);
	pop_allocator();
	va_end(args);
	
	return s;
}

bool strings_match(string a, string b) {
	if (a.count != b.count) return false;
	
	// Count match, pointer match: they are the same
	if (a.data == b.data) return true;
	
	return memcmp(a.data, b.data, a.count) == 0;
}

string string_view(string s, u64 start_index, u64 count) {
	assert(start_index < s.count, "array_view start_index % out of range for string count %", start_index, s.count);
	assert(count > 0, "array_view count must be more than 0");
	assert(start_index + count <= s.count, "array_view start_index + count is out of range");
	
	string result;
	result.data = s.data+start_index;
	result.count = count;
	
	return result;
}

// Defined in os interface
void print(string s, ...);