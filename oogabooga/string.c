
/*

	
	
*/

void* talloc(u64);

typedef struct string {
	u64 count;
	u8 *data;
} string;

const string null_string = {0, 0};

#define fixed_string STR
#define STR(s) ((string){ length_of_null_terminated_string((const char*)s), (u8*)s })

inline u64 length_of_null_terminated_string(const char* cstring) {
	u64 len = 0;
	while (*cstring != 0) {
		len += 1;
		cstring += 1;
	}
	return len;
}

string alloc_string(Allocator allocator, u64 count) {
	string s;
	s.count = count;
	s.data = cast(u8*)alloc(allocator, count);
	return s;
}
void dealloc_string(Allocator allocator, string s) {
	assert(s.count > 0 && s.data, "You tried to deallocate an empty string. That's doesn't make sense.");
	dealloc(allocator, s.data);
}
string talloc_string(u64 count) {
	string s = alloc_string(temp, count);
	return s;
}

string string_concat(const string left, const string right, Allocator allocator) {

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
char *convert_to_null_terminated_string(const string s, Allocator allocator) {
	char *cstring = cast(char*)alloc(allocator, s.count+1);
	memcpy(cstring, s.data, s.count);
	cstring[s.count] = 0;
	return cstring;
}

char *temp_convert_to_null_terminated_string(const string s) {
	char *c = convert_to_null_terminated_string(s, temp);
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

// Returns first index from left where "sub" matches in "s". Returns -1 if no match is found.
s64 string_find_from_left(string s, string sub) {
	for (s64 i = 0; i <= s.count-sub.count; i++) {
		if (strings_match(string_view(s, i, sub.count), sub)) {
			return i;
		}
	}
	
	return -1;
}

// Returns first index from right where "sub" matches in "s" Returns -1 if no match is found.
s64 string_find_from_right(string s, string sub) {
	for (s64 i = s.count-sub.count; i >= 0 ; i--) {
		if (strings_match(string_view(s, i, sub.count), sub)) {
			return i;
		}
	}
	
	return -1;
}

bool string_starts_with(string s, string sub) {
	if (s.count < sub.count) return false;
	
	s.count = sub.count;
	
	return strings_match(s, sub);
}