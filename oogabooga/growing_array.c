
/*

	Full API:
	
		void growing_array_init_reserve(void **array, u64 block_size_in_bytes, u64 count_to_reserve, Allocator allocator);
		void growing_array_init(void **array, u64 block_size_in_bytes, Allocator allocator);
		void growing_array_deinit(void **array);
		
		void *growing_array_add_empty(void **array);
		void *growing_array_add_multiple_empty(void **array);
		void growing_array_add(void **array, void *item);
		void growing_array_add_multiple(void **array, void *items, u64 count);
		
		void growing_array_reserve(void **array, u64 count_to_reserve);
		void growing_array_resize(void **array, u64 new_count);
		void growing_array_pop(void **array);
		void growing_array_clear(void **array);
		
		// Returns -1 if not found
		s32  growing_array_find_index_from_left_by_pointer(void **array, void *p);
		s32  growing_array_find_index_from_left_by_value(void **array, void *p);
		
		void growing_array_ordered_remove_by_index(void **array, u32 index);
		void growing_array_unordered_remove_by_index(void **array, u32 index);
		bool growing_array_ordered_remove_by_pointer(void **array, void *p);
		bool growing_array_unordered_remove_by_pointer(void **array, void *p);
		bool growing_array_ordered_remove_one_by_value(void **array, void *p);
		bool growing_array_unordered_remove_one_by_value(void **array, void *p);
		
		u32  growing_array_get_valid_count(void *array);
		u32  growing_array_get_allocated_count(void *array);

	Usage:
	
	    Thing *things;
	    growing_array_init(&things, sizeof(Thing));
	    
	    growing_array_deinit(&things);
	    
	    Thing new_thing;
	    growing_array_add(&things, &new_thing); // 'thing' is copied
	    
	    Thing *nth_thing = &things[n];
	    
	    growing_array_reserve_count(&things, 690);
	    growing_array_resize_count(&things, 69);
	    
	    // "Slow", but stuff in the array keeps the same order
	    growing_array_ordered_remove_by_index(&things, i);
	    
	    // Fast, but will not keep stuff ordered
	    growing_array_unordered_remove_by_index(&things, i);
	    
	    growing_array_ordered_remove_by_pointer(&things, nth_thing);
	    growing_array_unordered_remove_by_pointer(&things, nth_thing);
	    
	    Thing thing_prototype;
	    growing_array_ordered_remove_one_by_value(&things, thing_prototype);
	    growing_array_unordered_remove_one_by_value(&things, thing_prototype);
	    growing_array_ordered_remove_all_by_value(&things, thing_prototype);
	    growing_array_unordered_remove_all_by_value(&things, thing_prototype);
	    
	    growing_array_get_valid_count(&things);
	    growing_array_get_allocated_count(&things);
    
*/

#define GROWING_ARRAY_SIGNATURE 2224364215

typedef struct Growing_Array_Header {
	u32 signature;
    u32 valid_count;
    u32 allocated_count;
    u32 block_size_in_bytes;
    Allocator allocator;
} Growing_Array_Header;

bool 
check_growing_array_signature(void **array) {
	Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
	if (header->signature != GROWING_ARRAY_SIGNATURE) return false;
	return true;
}

void
growing_array_init_reserve(void **array, u64 block_size_in_bytes, u64 count_to_reserve, Allocator allocator) {
    
    count_to_reserve = get_next_power_of_two(count_to_reserve);
    u64 bytes_to_allocate = count_to_reserve*block_size_in_bytes + sizeof(Growing_Array_Header);
    
    Growing_Array_Header *header = (Growing_Array_Header*)alloc(allocator, bytes_to_allocate);
    
    header->allocator = allocator;
    header->block_size_in_bytes = block_size_in_bytes;
    header->valid_count = 0;
    header->allocated_count = count_to_reserve;
    header->signature = GROWING_ARRAY_SIGNATURE;
    
    *array = header+1;
}
void
growing_array_init(void **array, u64 block_size_in_bytes, Allocator allocator) {
    growing_array_init_reserve(array, block_size_in_bytes, 8, allocator);
}
void
growing_array_deinit(void **array) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    dealloc(header->allocator, header);
}

void
growing_array_reserve(void **array, u64 count_to_reserve) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    if (header->allocated_count >= count_to_reserve) return;
    
    u64 old_allocated_bytes = header->allocated_count*header->block_size_in_bytes+sizeof(Growing_Array_Header);
    count_to_reserve = get_next_power_of_two(count_to_reserve);
    u64 bytes_to_allocate = count_to_reserve*header->block_size_in_bytes+sizeof(Growing_Array_Header);
    Growing_Array_Header *new_header = (Growing_Array_Header*)alloc(header->allocator, bytes_to_allocate);
    
    memcpy(new_header, header, old_allocated_bytes);
    
    
    *array = new_header+1;
    
    new_header->allocated_count = count_to_reserve;
    
    dealloc(header->allocator, header);
}

void*
growing_array_add_empty(void **array) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    growing_array_reserve(array, header->valid_count+1);
    
    // Pointer might have been invalidated after reserve
    header = ((Growing_Array_Header*)*array) - 1; 
    
    void *item = (u8*)*array + header->valid_count*header->block_size_in_bytes;
    
    header->valid_count += 1;
    
    return item;
}
void*
growing_array_add_multiple_empty(void **array, u64 count) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    growing_array_reserve(array, header->valid_count+count);
    
    // Pointer might have been invalidated after reserve
    header = ((Growing_Array_Header*)*array) - 1; 
    
    void *start = (u8*)*array + header->valid_count*header->block_size_in_bytes;
    
    header->valid_count += count;
    
    return start;
}
void
growing_array_add(void **array, void *item) {

    void *new = growing_array_add_empty(array);

    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    memcpy(new, item, header->block_size_in_bytes);
}
void
growing_array_add_multiple(void **array, void *items, u64 count) {

    void *start = growing_array_add_multiple_empty(array, count);

    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    memcpy(start, items, header->block_size_in_bytes*count);
}

void growing_array_resize(void **array, u64 new_count) {
    growing_array_reserve(array, new_count);
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    header->valid_count = new_count;
}

void growing_array_pop(void **array) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    assert(header->valid_count > 0, "No items to pop in growing array");
    header->valid_count -= 1;
}

void growing_array_clear(void **array) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    header->valid_count = 0;
}

void 
growing_array_ordered_remove_by_index(void **array, u32 index) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    assert(index < header->valid_count, "Growing array index out of range");
    
    if (index == header->valid_count-1) {
        growing_array_pop(array);
        return;
    }
    
    u64 byte_index = header->block_size_in_bytes*index;
    
    memcpy(
        (u8*)*array + byte_index, 
        (u8*)*array + byte_index + header->block_size_in_bytes,
        (header->valid_count-index-1)*header->block_size_in_bytes
    );
    header->valid_count -= 1;
}
void 
growing_array_unordered_remove_by_index(void **array, u32 index) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    assert(index < header->valid_count, "Growing array index out of range");
    
    if (index == header->valid_count-1) {
        growing_array_pop(array);
        return;
    }
    
    u64 byte_index = header->block_size_in_bytes*index;
    u64 last_item_index = header->block_size_in_bytes*header->valid_count-header->block_size_in_bytes;
    
    memcpy(
        (u8*)*array + byte_index, 
        (u8*)*array + last_item_index,
        header->block_size_in_bytes
    );
    header->valid_count -= 1;
}

s32
growing_array_find_index_from_left_by_pointer(void **array, void *p) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    for (u32 i = 0; i < header->valid_count; i++) {
        void *next = (u8*)*array + i*header->block_size_in_bytes;
        
        if (next == p) {
            return i;
        }
    }
    return -1;
}
s32
growing_array_find_index_from_left_by_value(void **array, void *p) {
	assert(check_growing_array_signature(array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    for (u32 i = 0; i < header->valid_count; i++) {
        void *next = (u8*)*array + i*header->block_size_in_bytes;
        
        if (bytes_match(next, p, header->block_size_in_bytes)) {
            return i;
        }
    }
    return -1;
}

bool
growing_array_ordered_remove_by_pointer(void **array, void *p) {
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    s32 i = growing_array_find_index_from_left_by_pointer(array, p);
    
    if (i < 0) return false;
    
    growing_array_ordered_remove_by_index(array, i);
    
    return true;
}
bool 
growing_array_unordered_remove_by_pointer(void **array, void *p) {
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    s32 i = growing_array_find_index_from_left_by_pointer(array, p);
    
    if (i < 0) return false;
    
    growing_array_unordered_remove_by_index(array, i);
    
    return true;
}
bool 
growing_array_ordered_remove_one_by_value(void **array, void *p) {
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    s32 i = growing_array_find_index_from_left_by_value(array, p);
    
    if (i < 0) return false;
    
    growing_array_ordered_remove_by_index(array, i);
    
    return true;
}
bool 
growing_array_unordered_remove_one_by_value(void **array, void *p) {
    Growing_Array_Header *header = ((Growing_Array_Header*)*array) - 1;
    
    s32 i = growing_array_find_index_from_left_by_value(array, p);
    
    if (i < 0) return false;
    
    growing_array_unordered_remove_by_index(array, i);
    
    return true;
}

// #Incomplete
// s32 growing_array_ordered_remove_one_by_value(void **array, void *p)
// s32 growing_array_unordered_remove_one_by_value(void **array, void *p)

u32
growing_array_get_valid_count(void *array) {
	assert(check_growing_array_signature(&array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)array) - 1;
    return header->valid_count;
}
u32
growing_array_get_allocated_count(void *array) {
	assert(check_growing_array_signature(&array), "Not a valid growing array");
    Growing_Array_Header *header = ((Growing_Array_Header*)array) - 1;
    return header->allocated_count;
}