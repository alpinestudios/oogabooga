// Custom allocators for lodepng
Allocator get_heap_allocator();
/*Allocator lodepng_allocator = {0};
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
#include "third_party/lodepng.c"*/

#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

typedef unsigned char   u8;
typedef signed   char   s8;
typedef unsigned short  u16;
typedef signed   short  s16;
typedef unsigned int    u32;
typedef signed   int    s32;

// #Temporary #Incomplete #Memory
// This should use the allocator we pass to the gfx font system.
// Probably just do a thread local global here
void *stbtt_malloc(size_t size) {
	if (!size) return 0;
	return alloc(get_heap_allocator(), size);
}
#define STBTT_malloc(x,u) ((void)(u),stbtt_malloc(x))
void stbtt_free(void *p) {
	if (!p) return;
	dealloc(get_heap_allocator(), p);
}
#define STBTT_free(x,u)    ((void)(u),stbtt_free(x))

#define STBTT_assert(x)    assert(x)

size_t stbtt_strlen(const char* str) {
	size_t count = 0;
	while (str[count] != 0) count += 1;
	return count;
}
#define STBTT_strlen(x) stbtt_strlen(x)
#define STBTT_memcpy memcpy
#define STBTT_memset memset


#define STBI_NO_STDIO
#define STBI_ASSERT(x) {if (!(x)) *(volatile char*)0 = 0;}

#define STBI_MALLOC(sz)           stbtt_malloc(sz)
#define STBI_REALLOC(p,newsz)     get_heap_allocator().proc(newsz, p, ALLOCATOR_REALLOCATE, 0)
#define STBI_FREE(p)              stbtt_free(p)

#include "third_party/stb_image.h"
#include "third_party/stb_truetype.h"