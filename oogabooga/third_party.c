
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

typedef unsigned char   u8;
typedef signed   char   s8;
typedef unsigned short  u16;
typedef signed   short  s16;
typedef unsigned int    u32;
typedef signed   int    s32;

thread_local Allocator third_party_allocator = {0};
void *third_party_malloc(size_t size) {
	assert(third_party_allocator.proc, "No third party allocator was set, but it was used!");
	if (!size) return 0;
	return alloc(third_party_allocator, size);
}
void third_party_free(void *p) {
	assert(third_party_allocator.proc, "No third party allocator was set, but it was used!");
	if (!p) return;
	dealloc(third_party_allocator, p);
}

#define STBTT_malloc(x,u) ((void)(u),third_party_malloc(x))
#define STBTT_free(x,u)    ((void)(u),third_party_free(x))
#define STBTT_assert(x)    assert(x)
size_t stbtt_strlen(const char* str) {
	size_t count = 0;
	while (str[count] != 0) count += 1;
	return count;
}
#define STBTT_strlen(x) stbtt_strlen(x)
#define STBTT_memcpy memcpy
#define STBTT_memset memset
#include "third_party/stb_truetype.h"


#define STBI_NO_STDIO
#define STBI_ASSERT(x) {if (!(x)) *(volatile char*)0 = 0;}
#define STBI_MALLOC(sz)           third_party_malloc(sz)
#define STBI_REALLOC(p,newsz)     third_party_allocator.proc(newsz, p, ALLOCATOR_REALLOCATE, 0)
#define STBI_FREE(p)              third_party_free(p)
#include "third_party/stb_image.h"

#define STB_VORBIS_NO_CRT
#include "third_party/stb_vorbis.c"
// Why Sean ?????
#undef R
#undef C
#undef L

#define DR_MP3_NO_STDIO
#define DRMP3_ASSERT(exp) assert(exp, "dr_mp3 assertion failed")
#define DRMP3_MALLOC(sz)           third_party_malloc(sz)
#define DRMP3_REALLOC(p,newsz)     third_party_allocator.proc(newsz, p, ALLOCATOR_REALLOCATE, 0)
#define DRMP3_FREE(p)              third_party_free(p)
#define DR_MP3_IMPLEMENTATION
#include "third_party/dr_mp3.h"

#define DR_WAV_NO_STDIO
#define DR_WAV_NO_WCHAR
#define DRWAV_ASSERT(exp) assert(exp, "dr_mp3 assertion failed")
#define DRWAV_MALLOC(sz)           third_party_malloc(sz)
#define DRWAV_REALLOC(p,newsz)     third_party_allocator.proc(newsz, p, ALLOCATOR_REALLOCATE, 0)
#define DRWAV_FREE(p)              third_party_free(p)
#define DR_WAV_IMPLEMENTATION
#include "third_party/dr_wav.h"