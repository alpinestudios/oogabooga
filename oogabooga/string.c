
/*

	
	
*/

ogb_instance void* talloc(u64);

typedef struct string {
	u64 count;
	u8 *data;
} string;

const string null_string = {0, 0};

#define fixed_string STR
#define STR(s) ((string){ length_of_null_terminated_string((const char*)s), (u8*)s })

inline u64 
length_of_null_terminated_string(const char* cstring) {
	u64 len = 0;
	while (*cstring != 0) {
		len += 1;
		cstring += 1;
	}
	return len;
}

string 
alloc_string(Allocator allocator, u64 count) {
	string s;
	s.count = count;
	s.data = cast(u8*)alloc(allocator, count);
	return s;
}
void 
dealloc_string(Allocator allocator, string s) {
	assert(s.count > 0 && s.data, "You tried to deallocate an empty string. That's doesn't make sense.");
	dealloc(allocator, s.data);
}
string 
talloc_string(u64 count) {
	string s = alloc_string(get_temporary_allocator(), count);
	return s;
}

string 
string_concat(const string left, const string right, Allocator allocator) {

	if (right.count + left.count == 0) return null_string;
	if (left.count == 0) return right;
	if (right.count == 0) return left;

	string result;
	result.count = left.count + right.count;
	result.data = cast(u8*)alloc(allocator, result.count);
	memcpy(result.data, left.data, left.count);
	memcpy(result.data+left.count, right.data, right.count);
	return result;
}
char *
convert_to_null_terminated_string(const string s, Allocator allocator) {
	char *cstring = cast(char*)alloc(allocator, s.count+1);
	memcpy(cstring, s.data, s.count);
	cstring[s.count] = 0;
	return cstring;
}

char *
temp_convert_to_null_terminated_string(const string s) {
	char *c = convert_to_null_terminated_string(s, get_temporary_allocator());
	return c;
}
bool 
strings_match(string a, string b) {
	if (a.count != b.count) return false;
	
	// Count match, pointer match: they are the same
	if (a.data == b.data) return true;

	return memcmp(a.data, b.data, a.count) == 0;
}

string 
string_view(string s, u64 start_index, u64 count) {
	assert(start_index < s.count, "string_view start_index % out of range for string count %", start_index, s.count);
	assert(count > 0, "string_view count must be more than 0");
	assert(start_index + count <= s.count, "string_view start_index + count is out of range");
	
	string result;
	result.data = s.data+start_index;
	result.count = count;
	
	return result;
}

// Returns first index from left where "sub" matches in "s". Returns -1 if no match is found.
s64 
string_find_from_left(string s, string sub) {
	for (s64 i = 0; i <= s.count-sub.count; i++) {
		if (strings_match(string_view(s, i, sub.count), sub)) {
			return i;
		}
	}
	
	return -1;
}

// Returns first index from right where "sub" matches in "s" Returns -1 if no match is found.
s64 
string_find_from_right(string s, string sub) {
	for (s64 i = s.count-sub.count; i >= 0 ; i--) {
		if (strings_match(string_view(s, i, sub.count), sub)) {
			return i;
		}
	}
	
	return -1;
}

bool 
string_starts_with(string s, string sub) {
	if (s.count < sub.count) return false;
	
	s.count = sub.count;
	
	return strings_match(s, sub);
}

string
string_copy(string s, Allocator allocator) {
	string c = alloc_string(allocator, s.count);
	memcpy(c.data, s.data, s.count);
	return c;
}


typedef struct String_Builder {
	union {
		struct {u64 count;u8 *buffer;};
		string result;
	};
	u64 buffer_capacity;
	Allocator allocator;
} String_Builder;


void 
string_builder_reserve(String_Builder *b, u64 required_capacity) {
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
void 
string_builder_init_reserve(String_Builder *b, u64 reserved_capacity, Allocator allocator) {
	reserved_capacity = max(reserved_capacity, 128);
	b->allocator = allocator;
	b->buffer_capacity = 0;
	b->buffer = 0;
	string_builder_reserve(b, reserved_capacity);
	b->count = 0;
}
void 
string_builder_init(String_Builder *b, Allocator allocator) {
	string_builder_init_reserve(b, 128, allocator);
}
void 
string_builder_deinit(String_Builder *b) {
	dealloc(b->allocator, b->buffer);
}
void 
string_builder_append(String_Builder *b, string s) {
	assert(b->allocator.proc, "String_Builder is missing allocator");
	string_builder_reserve(b, b->count+s.count);
	
	memcpy(b->buffer+b->count, s.data, s.count);
	b->count += s.count;
}
string 
string_builder_get_string(String_Builder b) {
	return b.result;
}


string 
string_replace_all(string s, string old, string new, Allocator allocator) {

	if (!s.data || !s.count) return string_copy(null_string, allocator);

	String_Builder builder;
	string_builder_init_reserve(&builder, s.count, allocator);
	
	while (s.count > 0) {
		if (s.count >= old.count && strings_match(string_view(s, 0, old.count), old)) {
			if (new.count != 0) string_builder_append(&builder, new);
			s.data += old.count;
			s.count -= old.count;
		} else {
			string_builder_append(&builder, string_view(s, 0, 1));
			s.data += 1;
			s.count -= 1;
		}
	}
	
	return string_builder_get_string(builder);
}
	
string
string_trim_left(string s) {
	while (s.count > 0 && *s.data == ' ') {
		s.data += 1;
		s.count -= 1;
	}
	return s;
}
string
string_trim_right(string s) {

	while (s.count > 0 && s.data[s.count-1] == ' ') {
		s.count -= 1;
	}
	return s;
}
string
string_trim(string s) {
	s = string_trim_left(s);
	return string_trim_right(s);
}