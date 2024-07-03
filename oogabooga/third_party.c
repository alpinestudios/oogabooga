// Custom allocators for lodepng
Allocator get_heap_allocator();
Allocator lodepng_allocator = {0};
void* lodepng_malloc(size_t size) {
#ifdef LODEPNG_MAX_ALLOC
	  if(size > LODEPNG_MAX_ALLOC) return 0;
#endif
	return alloc(get_heap_allocator(), size);
}
void* lodepng_realloc(void* ptr, size_t new_size) {
	if (!ptr) return lodepng_malloc(new_size);
	
#ifdef LODEPNG_MAX_ALLOC
	  if(new_size > LODEPNG_MAX_ALLOC) return 0;
#endif
	if (lodepng_allocator.proc) return lodepng_allocator.proc(new_size, ptr, ALLOCATOR_REALLOCATE, lodepng_allocator.data);
	return get_heap_allocator().proc(new_size, ptr, ALLOCATOR_REALLOCATE, get_heap_allocator().data);
}
void lodepng_free(void* ptr) {
	if (!ptr) return;
	if (lodepng_allocator.proc) dealloc(lodepng_allocator, ptr);
	else dealloc(get_heap_allocator(), ptr);
}
#define LODEPNG_NO_COMPILE_ALLOCATORS
//#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_NO_COMPILE_ENCODER
// One day I might write my own png decoder so we don't even need this
#include "third_party/lodepng.h" 
#include "third_party/lodepng.c"