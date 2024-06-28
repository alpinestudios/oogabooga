
void * memcpy (void *,const void *,size_t);

typedef struct string {
	u8 *data;
	u64 count;
} string;

void push_temp_allocator();

#define cstr const_string
#define const_string(s) (string){ (u8*)s, length_of_null_terminated_string(s) }

inline u64 length_of_null_terminated_string(const char* cstring) {
	u64 len = 0;
	while (*cstring != 0) {
		len += 1;
		cstring += 1;
	}
	return len;
}

// context.allocator !
string string_concat(string left, string right) {
	string result;
	result.count = left.count + right.count;
	result.data = cast(u8*)alloc(result.count);
	memcpy(result.data, left.data, left.count);
	memcpy(result.data+left.count, right.data, right.count);
	return result;
}
// context.allocator !
char *convert_to_null_terminated_string(string s) {
	char *cstring = cast(char*)alloc(s.count+1);
	memcpy(cstring, s.data, s.count);
	cstring[s.count] = 0;
	return cstring;
}

char *temp_convert_to_null_terminated_string(string s) {
	push_temp_allocator();
	char *c = convert_to_null_terminated_string(s);
	pop_allocator();
	return c;
}