

#define UTF16_SURROGATE_HIGH_START  0xD800
#define UTF16_SURROGATE_HIGH_END    0xDBFF
#define UTF16_SURROGATE_LOW_START   0xDC00
#define UTF16_SURROGATE_LOW_END     0xDFFF
#define UTF16_SURROGATE_OFFSET      0x10000
#define UTF16_SURROGATE_MASK        0x3FF

// Returns how many utf16 units were converted
int utf16_to_utf32(const u16 *utf16, u64 length, u32 *utf32) {
    if (length == 0 || utf16 == NULL || utf32 == NULL) {
        return -1;
    }

    u16 first = utf16[0];

    if (first >= UTF16_SURROGATE_HIGH_START && first <= UTF16_SURROGATE_HIGH_END) {
        if (length < 2) {
            return -1;
        }

        u16 second = utf16[1];
        if (second >= UTF16_SURROGATE_LOW_START && second <= UTF16_SURROGATE_LOW_END) {
            *utf32 = ((first - UTF16_SURROGATE_HIGH_START) << 10)
                     + (second - UTF16_SURROGATE_LOW_START)
                     + UTF16_SURROGATE_OFFSET;
            return 2;
        } else {
            return -1;
        }
    } else if (first >= UTF16_SURROGATE_LOW_START && first <= UTF16_SURROGATE_LOW_END) {
        return -1;
    } else {
        *utf32 = first;
        return 1;
    }
}

// Yoinked from jai Unicode.jai

#define UNI_REPLACEMENT_CHAR 0x0000FFFD
#define UNI_MAX_UTF32        0x7FFFFFFF
#define UNI_MAX_UTF16        0x0010FFFF
#define SURROGATES_START     0xD800
#define SURROGATES_END       0xDFFF

typedef struct {
	u32 utf32;
	s64 continuation_bytes;
	bool reached_end;
	bool error;
} Utf8_To_Utf32_Result;

const u8 trailing_bytes_for_utf8[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
const u8 utf8_inital_byte_mask[] = { 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

Utf8_To_Utf32_Result utf8_to_utf32(u8 *s, s64 source_length, bool strict) {
    s64 continuation_bytes = trailing_bytes_for_utf8[s[0]];

    if (continuation_bytes + 1 > source_length) {
        return (Utf8_To_Utf32_Result){UNI_REPLACEMENT_CHAR, source_length, true, true};
    }

    u32 ch = s[0] & utf8_inital_byte_mask[continuation_bytes];

    for (u64 i = 1; i <= continuation_bytes; i++) {  // Do nothing if it is 0.
        ch = ch << 6;
        if (strict) if ((s[i] & 0xC0) != 0x80)  return (Utf8_To_Utf32_Result){UNI_REPLACEMENT_CHAR, i - 1, true, true};
    	ch |= s[i] & 0x3F;
    }

    if (strict) {
        if (ch > UNI_MAX_UTF16 ||
          (SURROGATES_START <= ch && ch <= SURROGATES_END) ||
          (ch <= 0x0000007F && continuation_bytes != 0) ||
          (ch <= 0x000007FF && continuation_bytes != 1) ||
          (ch <= 0x0000FFFF && continuation_bytes != 2) ||
          continuation_bytes > 3) {
            return (Utf8_To_Utf32_Result){UNI_REPLACEMENT_CHAR, continuation_bytes+1, true, true};
        }
    }

    if (ch > UNI_MAX_UTF32) {
        ch = UNI_REPLACEMENT_CHAR;
    }

	return (Utf8_To_Utf32_Result){ ch, continuation_bytes+1, false, false };
}

// Returns 0 on fail
u32 next_utf8(string *s) {
	Utf8_To_Utf32_Result result = utf8_to_utf32(s->data, s->count, false);

    s->data  += result.continuation_bytes;
    s->count -= result.continuation_bytes;
    assert(s->count >= 0);

	if (result.error) return 0;

    return result.utf32;
}

u64 utf8_index_to_byte_index(string str, u64 index) {
	u64 byte_index = 0;
	u64 utf8_index = 0;
	while (utf8_index < index && str.count != 0) {
		string last_str = str;
		u32 codepoint = next_utf8(&str);
		if (!codepoint) break;

		u64 byte_diff = ((u8*)str.data)-((u8*)last_str.data);
		assert(byte_diff != 0);
		byte_index += byte_diff;
		utf8_index += 1;
	}
	return byte_index;
}
string utf8_slice(string str, u64 index, u64 count) {
	u64 byte_index = utf8_index_to_byte_index(str, index);
	u64 byte_end_index = utf8_index_to_byte_index(str, index+count);
	u64 byte_count = byte_end_index - byte_index;

	return string_view(str, byte_index, byte_count);
}
