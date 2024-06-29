

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