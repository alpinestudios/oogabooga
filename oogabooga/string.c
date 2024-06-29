
/*
Usage:

	// We need to use macro const_string to convert literal to string
	string fmt = const_string("Pointer address: 0x%x");
	print(fmt, cast(u64)&a); // Print to stdout
	
	// Format a string and allocate with context.allocator
	string a = sprint("Hello, %cs!\n", "balls"); // %cs for char*
	
	string balls = const_string("balls");
	// tprint for temporary allocation
	string b = tprint("Hello, %s!\n", balls); // %s for string
	
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

// Not sure what to call this lol
#define fixed_string const_string
#define cstr const_string
#define const_string(s) ((string){ length_of_null_terminated_string(s), (u8*)s })

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
string talloc_string(u64 count) {
	push_temp_allocator();
	string s = alloc_string(count);
	pop_allocator();
	return s;
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

