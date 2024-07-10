


// This is a very niche sort algorithm.
// I use it for Z sorting quads.
// help_buffer should be same size as collection.
// This only works with integers, and it will use the first number_of_bits in the integer
// at sort_value_offset_in_item for sorting.
// There is a cost of memory as we need to double the buffer we're sorting BUT the performance
// gain is very promising.
// At 21 bits I'm able to sort a completely randomized collection of 100k integers at around
// 8m cycles (or 2.5-2.6ms on my shitty laptop i5-11300H)
void radix_sort(void *collection, void *help_buffer, u64 item_count, u64 item_size, u64 sort_value_offset_in_item, u64 number_of_bits) {

    local_persist const int RADIX = 256;
    local_persist const int BITS_PER_PASS = 8;
    local_persist const int MASK = (RADIX - 1);
    
    const int PASS_COUNT = ((number_of_bits + BITS_PER_PASS - 1) / BITS_PER_PASS);
    const u64 SIGN_SHIFT = 1ULL << (number_of_bits - 1);

    u64* count = (u64*)alloc(get_temporary_allocator(), RADIX * sizeof(u64));
    u8* items = (u8*)collection;
    u8* buffer = (u8*)help_buffer;

    for (u32 pass = 0; pass < PASS_COUNT; ++pass) {
        u32 shift = pass * BITS_PER_PASS;

        for (u32 i = 0; i < RADIX; ++i) {
            count[i] = 0;
        }

        for (u64 i = 0; i < item_count; ++i) {
            u64 sort_value = *(u64*)(items + i * item_size + sort_value_offset_in_item);
            sort_value += SIGN_SHIFT;  // Transform the value to handle negative numbers
            u32 digit = (sort_value >> shift) & MASK;
            ++count[digit];
        }

        u64 sum = 0;
        for (u32 i = 0; i < RADIX; ++i) {
            u64 temp = count[i];
            count[i] = sum;
            sum += temp;
        }

        for (u64 i = 0; i < item_count; ++i) {
            u64 sort_value = *(u64*)(items + i * item_size + sort_value_offset_in_item);
            u64 transformed_value = sort_value + SIGN_SHIFT;  // Transform the value to handle negative numbers
            u32 digit = (transformed_value >> shift) & MASK;
            memcpy(buffer + count[digit] * item_size, items + i * item_size, item_size);
            ++count[digit];
        }

        memcpy(items, buffer, item_count * item_size);
    }

    dealloc(get_temporary_allocator(), count);
}

void merge_sort(void *collection, void *help_buffer, u64 item_count, u64 item_size, int (*compare)(const void *, const void *)) {
    u8 *items = (u8 *)collection;
    u8 *buffer = (u8 *)help_buffer;

    for (u64 width = 1; width < item_count; width *= 2) {
        for (u64 i = 0; i < item_count; i += 2 * width) {
            u64 left = i;
            u64 right = (i + width < item_count) ? (i + width) : item_count;
            u64 end = (i + 2 * width < item_count) ? (i + 2 * width) : item_count;

            u64 left_index = left;
            u64 right_index = right;
            u64 k = left;

            while (left_index < right && right_index < end) {
                if (compare(items + left_index * item_size, items + right_index * item_size) <= 0) {
                    memcpy(buffer + k * item_size, items + left_index * item_size, item_size);
                    left_index++;
                } else {
                    memcpy(buffer + k * item_size, items + right_index * item_size, item_size);
                    right_index++;
                }
                k++;
            }

            while (left_index < right) {
                memcpy(buffer + k * item_size, items + left_index * item_size, item_size);
                left_index++;
                k++;
            }

            while (right_index < end) {
                memcpy(buffer + k * item_size, items + right_index * item_size, item_size);
                right_index++;
                k++;
            }

            for (u64 j = left; j < end; j++) {
                memcpy(items + j * item_size, buffer + j * item_size, item_size);
            }
        }
    }
}
