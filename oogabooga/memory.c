

#define KB(x) (x*1024ull)
#define MB(x) ((KB(x))*1024ull)
#define GB(x) ((MB(x))*1024ull)

#ifndef INIT_MEMORY_SIZE
	#define INIT_MEMORY_SIZE KB(50)
#endif
// We may need to allocate stuff in initialization time before the heap is ready.
// That's what this is for.
u8 init_memory_arena[INIT_MEMORY_SIZE];
u8 *init_memory_head = init_memory_arena;

void* initialization_allocator_proc(u64 size, void *p, Allocator_Message message, void *data) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			p = init_memory_head;
			init_memory_head += size;
			
			if (init_memory_head >= ((u8*)init_memory_arena+INIT_MEMORY_SIZE)) {
				os_write_string_to_stdout(STR("Out of initialization memory! Please provide more by increasing INIT_MEMORY_SIZE"));
				crash();
			}
			return p;
			break;
		}
		case ALLOCATOR_DEALLOCATE: {
			return 0;
		}
		case ALLOCATOR_REALLOCATE: {
			return 0;
		}
	}
	return 0;
}

Allocator get_initialization_allocator() {
	Allocator a;
	a.proc = initialization_allocator_proc;
	return a;
}

///
///
// Basic general heap allocator, free list
///
// Technically thread safe but synchronization is horrible.
// Fragmentation is catastrophic.
// We could fix it by merging free nodes every now and then
// BUT: We aren't really supposed to allocate/deallocate directly on the heap too much anyways...

#define MAX_HEAP_BLOCK_SIZE align_next(MB(500), os.page_size)
#define DEFAULT_HEAP_BLOCK_SIZE (min(MAX_HEAP_BLOCK_SIZE, program_memory_capacity))
#define HEAP_ALIGNMENT (sizeof(Heap_Free_Node))
typedef struct Heap_Free_Node Heap_Free_Node;
typedef struct Heap_Block Heap_Block;

typedef struct Heap_Free_Node {
	u64 size;
	Heap_Free_Node *next;
} Heap_Free_Node;

typedef struct Heap_Block {
	u64 size;
	Heap_Free_Node *free_head;
	void* start;
	Heap_Block *next;
	// 32 bytes !!
#if CONFIGURATION == DEBUG
	u64 total_allocated;
	u64 padding;
#endif
} Heap_Block;

#define HEAP_META_SIGNATURE 6969694206942069ull
typedef alignat(16) struct Heap_Allocation_Metadata {
	u64 size;
	Heap_Block *block;
#if CONFIGURATION == DEBUG
	u64 signature;
	u64 padding;
#endif
} Heap_Allocation_Metadata;

// #Global
ogb_instance Heap_Block *heap_head;
ogb_instance bool heap_initted;
ogb_instance Spinlock heap_lock;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Heap_Block *heap_head;
bool heap_initted = false;
Spinlock heap_lock;
#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE
	

u64 get_heap_block_size_excluding_metadata(Heap_Block *block) {
	return block->size - sizeof(Heap_Block);
}
u64 get_heap_block_size_including_metadata(Heap_Block *block) {
	return block->size;
}

bool is_pointer_in_program_memory(void *p) {
	return (u8*)p >= (u8*)program_memory && (u8*)p<((u8*)program_memory+program_memory_capacity);
}
bool is_pointer_in_stack(void* p) {
    void* stack_base = os_get_stack_base();
    void* stack_limit = os_get_stack_limit();
    return (uintptr_t)p >= (uintptr_t)stack_limit && (uintptr_t)p < (uintptr_t)stack_base;
}
bool is_pointer_in_static_memory(void* p) {
    return (uintptr_t)p >= (uintptr_t)os.static_memory_start && (uintptr_t)p < (uintptr_t)os.static_memory_end;
}
bool is_pointer_valid(void *p) {
	return is_pointer_in_program_memory(p) || is_pointer_in_stack(p) || is_pointer_in_static_memory(p);
}

// Meant for debug
void sanity_check_block(Heap_Block *block) {
#if CONFIGURATION == DEBUG
	assert(is_pointer_in_program_memory(block), "Heap_Block pointer is corrupt");
	assert(is_pointer_in_program_memory(block->start), "Heap_Block pointer is corrupt");
	if(block->next) { assert(is_pointer_in_program_memory(block->next), "Heap_Block next pointer is corrupt"); }
	assert(block->size < GB(256), "A heap block is corrupt.");
	assert(block->size >= INITIAL_PROGRAM_MEMORY_SIZE, "A heap block is corrupt.");
	assert((u64)block->start == (u64)block + sizeof(Heap_Block), "A heap block is corrupt.");
	

	Heap_Free_Node *node = block->free_head;	
	
	u64 total_free = 0;
	while (node != 0) {
		Heap_Free_Node *other_node = node->next;
		
		assert(node->size < GB(256), "Heap is corrupt");
		assert(is_pointer_in_program_memory(node), "Heap is corrupt");
		
		while (other_node != 0) {
			assert(is_pointer_in_program_memory(other_node), "Heap is corrupt");
			assert(other_node != node, "Circular reference in heap free node tree. This is probably an internal error, or an extremely unlucky result from heap corruption.");
			other_node = other_node->next;
		}
		total_free += node->size;
		assert(total_free <= block->size, "Free nodes are fucky wucky. This might be heap corruption, or possibly an internal error.");
		node = node->next;
	}
	
	u64 expected_size = get_heap_block_size_excluding_metadata(block);
	assert(block->total_allocated+total_free == expected_size, "Heap is corrupt.")
#endif
}
inline void check_meta(Heap_Allocation_Metadata *meta) {
#if CONFIGURATION == DEBUG
	assert(meta->signature == HEAP_META_SIGNATURE, "Heap error. Either 1) You passed a bad pointer to dealloc or 2) You corrupted the heap.");
#endif
// If > 256GB then prolly not legit lol
	assert(meta->size < 1024ULL*1024ULL*1024ULL*256ULL, "Heap error. Either 1) You passed a bad pointer to dealloc or 2) You corrupted the heap.");	
	assert(is_pointer_in_program_memory(meta->block), "Heap error. Either 1) You passed a bad pointer to dealloc or 2) You corrupted the heap."); 

	assert((u64)meta >= (u64)meta->block->start && (u64)meta < (u64)meta->block->start+meta->block->size, "Heap error: Pointer is not in it's metadata block. This could be heap corruption but it's more likely an internal error. That's not good.");
}

typedef struct {
	Heap_Free_Node *best_fit;
	Heap_Free_Node *previous;
	u64 delta;
} Heap_Search_Result;
Heap_Search_Result search_heap_block(Heap_Block *block, u64 size) {
	
	if (block->free_head == 0)  return (Heap_Search_Result){0, 0, 0};
		
	Heap_Free_Node *node = block->free_head;
	Heap_Free_Node *previous = 0;
	
	Heap_Free_Node *best_fit = 0;
	Heap_Free_Node *before_best_fit = 0;
	u64 best_fit_delta = 0;
	
	while (node != 0) {
		
		if (node->size == size) {
			Heap_Search_Result result;
			result.best_fit = node;
			result.previous = previous;
			result.delta = 0;
			assert(result.previous != result.best_fit, "Internal goof");
			return result;
		}
		
		if (node->size >= size) {
			u64 delta = node->size - size;
			
			if (delta < best_fit_delta || !best_fit) {
				before_best_fit = previous;
				best_fit = node;
				best_fit_delta = delta;
			}
		}
		
		if (node->next) previous = node;
		node = node->next;
	}
	
	if (!best_fit)  return (Heap_Search_Result){0, 0, 0};

	Heap_Search_Result result;
	result.best_fit = best_fit;
	result.previous = before_best_fit;
	result.delta = best_fit_delta;
	assert(result.previous != result.best_fit, "Internal goof");
	return result;
}

Heap_Block *make_heap_block(Heap_Block *parent, u64 size) {

	size += sizeof(Heap_Block);

	size = align_next(size, os.page_size);

	Heap_Block *block = (Heap_Block*)os_reserve_next_memory_pages(size);
		
	assert((u64)block % os.page_size == 0, "Heap block not aligned to page size");
	
	if (parent) parent->next = block;
	os_unlock_program_memory_pages(block, size);
	
#if CONFIGURATION == DEBUG
	block->total_allocated = 0;
#endif
	
	block->start = ((u8*)block)+sizeof(Heap_Block);
	block->size = size;
	block->next = 0;
	block->free_head = (Heap_Free_Node*)block->start;
	block->free_head->size = get_heap_block_size_excluding_metadata(block);
	block->free_head->next = 0;
	
	return block;
}

void heap_init() {
	if (heap_initted) return;
	assert(HEAP_ALIGNMENT == 16);
	assert(sizeof(Heap_Allocation_Metadata) % HEAP_ALIGNMENT == 0);
	heap_initted = true;
	heap_head = make_heap_block(0, DEFAULT_HEAP_BLOCK_SIZE);
	spinlock_init(&heap_lock);
}

void *heap_alloc(u64 size) {

	if (!heap_initted) heap_init();

	// #Sync #Speed oof
	spinlock_acquire_or_wait(&heap_lock);
	



	
	size += sizeof(Heap_Allocation_Metadata);
	
	size = (size+HEAP_ALIGNMENT) & ~(HEAP_ALIGNMENT-1);
	
	assert(size < MAX_HEAP_BLOCK_SIZE, "Past Charlie has been lazy and did not handle large allocations like this. I apologize on behalf of past Charlie. A quick fix could be to increase the heap block size for now. #Incomplete #Limitation");
	
	
#if VERY_DEBUG
	{
		Heap_Block *block = heap_head;
		
		while (block != 0) {
			sanity_check_block(block);
			block = block->next;
		}
	}
#endif
	
	Heap_Block *block = heap_head;
	Heap_Block *last_block = 0;
	Heap_Free_Node *best_fit = 0;
	Heap_Block *best_fit_block = 0;
	Heap_Free_Node *previous = 0;
	u64 best_fit_delta = 0;
	// #Speed
	// Maybe instead of going through EVERY free node to find best fit we do a good-enough fit
	while (block != 0) {

		if (get_heap_block_size_excluding_metadata(block) < size) {
			last_block = block;
			block = block->next;
			continue;
		}
	
		Heap_Search_Result result = search_heap_block(block, size);
		Heap_Free_Node *node = result.best_fit;
		if (node) {
			if (node->size < size) continue;
			if (node->size == size) {
				best_fit = node;
				best_fit_block = block;
				previous = result.previous;
				best_fit_delta = 0;
				break;
			}
			
			u64 delta = node->size-size;
			if (delta < best_fit_delta || !best_fit) {
				best_fit = node;
				best_fit_block = block;
				previous = result.previous;
				best_fit_delta = delta;
			}
		}
		
		last_block = block;
		block = block->next;
	}
	
	if (!best_fit) {
		block = make_heap_block(last_block, max(DEFAULT_HEAP_BLOCK_SIZE, size));
		previous = 0;
		best_fit = block->free_head;
		best_fit_block = block;
	}
		
	
	assert(best_fit != 0, "Internal heap error");
	
	// Unlock best fit
	
	// #Copypaste
	void *free_tail = (u8*)best_fit + best_fit->size;
	void *first_page = (void*)align_previous(best_fit, os.page_size);
	void *last_page_end = (void*)align_previous(free_tail, os.page_size);
	if ((u8*)last_page_end > (u8*)first_page) {
		os_unlock_program_memory_pages(first_page, (u64)last_page_end-(u64)first_page);
	}
	
	Heap_Free_Node *new_free_node = 0;
	if (size != best_fit->size) {
		u64 remainder = best_fit->size - size;
		new_free_node = (Heap_Free_Node*)(((u8*)best_fit)+size);
		new_free_node->size = remainder;
		new_free_node->next = best_fit->next;
		
		// Lock remaining free node
		// #Copypaste
		void *free_tail = (u8*)new_free_node + new_free_node->size;
		void *next_page = (void*)align_next(new_free_node, os.page_size);
		void *last_page_end = (void*)align_previous(free_tail, os.page_size);
		if ((u8*)last_page_end > (u8*)next_page) {
			os_lock_program_memory_pages(next_page, (u64)last_page_end-(u64)next_page);
		}
	}
	
	
	if (previous && new_free_node) {
		assert(previous->next == best_fit, "Internal heap error");
		previous->next = new_free_node;
	} else if (previous) {
		assert(previous->next == best_fit, "Internal heap error");
		previous->next = best_fit->next;
	}
	
	if (best_fit_block->free_head == best_fit) {
		// If we allocated the first free node then replace with new free node or just
		// remove it if perfect fit.
		if (new_free_node) {
			new_free_node->next = best_fit_block->free_head->next;
			best_fit_block->free_head = new_free_node;
		} else best_fit_block->free_head = best_fit_block->free_head->next;
	}
	
	Heap_Allocation_Metadata *meta = (Heap_Allocation_Metadata*)best_fit;
	meta->size = size;
	meta->block = best_fit_block;
#if CONFIGURATION == DEBUG
	meta->signature = HEAP_META_SIGNATURE;
	meta->block->total_allocated += size;
#endif

	check_meta(meta);

#if VERY_DEBUG
	sanity_check_block(meta->block);
#endif
	
	// #Sync #Speed oof
	spinlock_release(&heap_lock);
	
	
	void *p = ((u8*)meta)+sizeof(Heap_Allocation_Metadata);
	assert((u64)p % HEAP_ALIGNMENT == 0, "Internal heap error. Result pointer is not aligned to HEAP_ALIGNMENT");
	return p;
}
void heap_dealloc(void *p) {
	// #Sync #Speed oof
	
	if (!heap_initted) heap_init();

	spinlock_acquire_or_wait(&heap_lock);
	
	assert(is_pointer_in_program_memory(p), "A bad pointer was passed tp heap_dealloc: it is out of program memory bounds!"); 
	p = (u8*)p-sizeof(Heap_Allocation_Metadata);
	Heap_Allocation_Metadata *meta = (Heap_Allocation_Metadata*)(p);
	check_meta(meta);
	
	// Yoink meta data before we start overwriting it
	Heap_Block *block = meta->block;
	u64 size = meta->size;
	
#if CONFIGURATION == DEBUG
	memset(p, 0x69696969, size);
#endif
	
	#if VERY_DEBUG
		sanity_check_block(block);
	#endif
	
	Heap_Free_Node *new_node = cast(Heap_Free_Node*)p;
	new_node->size = size;
	
	if (new_node < block->free_head) {
		// #Copypaste
		void *free_tail = (u8*)new_node + new_node->size;
		void *next_page = (void*)align_next(new_node, os.page_size);
		void *last_page_end = (void*)align_previous(free_tail, os.page_size);
		if ((u8*)last_page_end > (u8*)next_page) {
			os_lock_program_memory_pages(next_page, (u64)last_page_end-(u64)next_page);
		}
		
		if ((u8*)new_node+size == (u8*)block->free_head) {
			new_node->size = size + block->free_head->size;
			new_node->next = block->free_head->next;
			block->free_head = new_node;
		} else {
			new_node->next = block->free_head;
			block->free_head = new_node;
		}
		
	} else {
	
		if (!block->free_head) {
			block->free_head = new_node;
			new_node->next = 0;
			
			// #Copypaste
			void *free_tail = (u8*)new_node + new_node->size;
			void *next_page = (void*)align_next(new_node, os.page_size);
			void *last_page_end = (void*)align_previous(free_tail, os.page_size);
			if ((u8*)last_page_end > (u8*)next_page) {
				os_lock_program_memory_pages(next_page, (u64)last_page_end-(u64)next_page);
			}
			
		} else {
			Heap_Free_Node *node = block->free_head;
		
			while (true) {
			
				assert(node != 0, "We didn't find where the free node should be! This is likely heap corruption (or, hopefully not, an internal error)");
				
				// In retrospect, I don't remember a good reason to care about where the
				// free nodes are... maybe I'm just dumb right now? #Speed #Memory
				// ... ACtually, it's probably to easily know when to merge free nodes.
				// BUT. Maybe it's not worth the performance hit? Then again, if the heap
				// allocator slows down your program you should rethink your memory management
				// anyways...
				
				if (new_node >= node) {
					u8* node_tail = (u8*)node + node->size;
					if (cast(u8*)new_node == node_tail) {
						
						// We need to account for the cases where we coalesce free blocks with start/end in the middle
						// of a page.
						
						u64 new_node_size = new_node->size;
						
						// #Copypaste
						void *free_tail = (u8*)new_node + new_node->size;
						void *next_page = (void*)align_previous(node_tail, os.page_size);
						void *last_page_end = (void*)align_previous(free_tail, os.page_size);

						if ((u8*)next_page < (u8*)node) next_page = (u8*)next_page + os.page_size;
						
						if ((u8*)last_page_end > (u8*)next_page) {
							os_lock_program_memory_pages(next_page, (u64)last_page_end-(u64)next_page);
						}
						
						node->size += new_node_size;
						
						break;
					} else {
						new_node->next = node->next;
						node->next = new_node;
						
						u8* new_node_tail = (u8*)new_node + new_node->size;
						if (new_node->next && (u8*)new_node->next == new_node_tail) {
							new_node->size += new_node->next->size;
							new_node->next = new_node->next->next;
						}
						
						// #Copypaste
						void *free_tail = (u8*)new_node + new_node->size;
						void *next_page = (void*)align_next(new_node, os.page_size);
						void *last_page_end = (void*)align_previous(free_tail, os.page_size);
						if ((u8*)last_page_end > (u8*)next_page) {
							os_lock_program_memory_pages(next_page, (u64)last_page_end-(u64)next_page);
						}
						
						break;
					}
				}
				
				node = node->next;
			}
		}
	}
	

#if CONFIGURATION == DEBUG
	block->total_allocated -= size;
#endif

#if VERY_DEBUG
	sanity_check_block(block);
#endif
	// #Sync #Speed oof
	spinlock_release(&heap_lock);
}

void* heap_allocator_proc(u64 size, void *p, Allocator_Message message, void* data) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return heap_alloc(size);
			break;
		}
		case ALLOCATOR_DEALLOCATE: {
			heap_dealloc(p);
			return 0;
		}
		case ALLOCATOR_REALLOCATE: {
			if (!p) {
				return heap_alloc(size);
			}
			assert(is_pointer_valid(p), "Invalid pointer passed to heap allocator reallocate");
			Heap_Allocation_Metadata *meta = (Heap_Allocation_Metadata*)(((u64)p)-sizeof(Heap_Allocation_Metadata));
			check_meta(meta);
			void *new = heap_alloc(size);
			memcpy(new, p, min(size, meta->size));
			heap_dealloc(p);
			return new;
		}
	}
	return 0;
}

Allocator get_heap_allocator() {
	Allocator heap_allocator;
	
	heap_allocator.proc = heap_allocator_proc;
	heap_allocator.data = 0;
	
	return heap_allocator;
}

///
///
// Temporary storage
///

#ifndef TEMPORARY_STORAGE_SIZE
	#define TEMPORARY_STORAGE_SIZE (1024ULL*1024ULL*2ULL) // 2mb
#endif

ogb_instance void* talloc(u64);
ogb_instance void* temp_allocator_proc(u64 size, void *p, Allocator_Message message, void*);

// #Global
ogb_instance Allocator 
get_temporary_allocator();

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
thread_local void * temporary_storage = 0;
thread_local void * temporary_storage_pointer = 0;
thread_local bool   has_warned_temporary_storage_overflow = false;
thread_local Allocator temp_allocator;

ogb_instance Allocator 
get_temporary_allocator() {
	if (!temporary_storage) return get_initialization_allocator();
	return temp_allocator;
}
#endif

ogb_instance void* 
temp_allocator_proc(u64 size, void *p, Allocator_Message message, void* data);

ogb_instance void 
temporary_storage_init(u64 arena_size);

ogb_instance void* 
talloc(u64 size);

ogb_instance void 
reset_temporary_storage();


#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
void* temp_allocator_proc(u64 size, void *p, Allocator_Message message, void* data) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return talloc(size);
			break;
		}
		case ALLOCATOR_DEALLOCATE: {
			return 0;
		}
		case ALLOCATOR_REALLOCATE: {
			return 0;
		}
	}
	return 0;
}

void temporary_storage_init(u64 arena_size) {
	
	temporary_storage = heap_alloc(arena_size);
	assert(temporary_storage, "Failed allocating temporary storage");
	temporary_storage_pointer = temporary_storage;

	temp_allocator.proc = temp_allocator_proc;
	temp_allocator.data = 0;
	
	temp_allocator.proc = temp_allocator_proc;
}

void* talloc(u64 size) {
	
	assert(size < TEMPORARY_STORAGE_SIZE, "Bruddah this is too large for temp allocator");
	
	void* p = temporary_storage_pointer;
	
	temporary_storage_pointer = (u8*)temporary_storage_pointer + size;
	
	if ((u8*)temporary_storage_pointer >= (u8*)temporary_storage+TEMPORARY_STORAGE_SIZE) {
		if (!has_warned_temporary_storage_overflow) {
			os_write_string_to_stdout(STR("WARNING: temporary storage was overflown, we wrap around at the start.\n"));
			has_warned_temporary_storage_overflow = true;
		}
		temporary_storage_pointer = temporary_storage;
		return talloc(size);;
	}
	
	return p;
}

void reset_temporary_storage() {
	temporary_storage_pointer = temporary_storage;	
	has_warned_temporary_storage_overflow = false;
}

#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE