
// YOINKED FROM https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
// (stb_vorbis uses math.h log() but that collides with oogabooga log() procedure)
/////////////////////////////////////////////////////////////
// ln.c
//
// simple, fast, accurate natural log approximation
// when without <math.h>

// featuring * floating point bit level hacking,
//           * x=m*2^p => ln(x)=ln(m)+ln(2)p,
//           * Remez algorithm

// by Lingdong Huang, 2020. Public domain.

// ============================================

float ln(float x) {
  unsigned int bx = * (unsigned int *) (&x);
  unsigned int ex = bx >> 23;
  signed int t = (signed int)ex-(signed int)127;
  unsigned int s = (t < 0) ? (-t) : t;
  bx = 1065353216 | (bx & 8388607);
  x = * (float *) (&bx);
  return -1.49278+(2.11263+(-0.729104+0.10969*x)*x)*x+0.6931471806*t;
}
// done.






// ============================================
// Exact same function, with added comments:

float natural_log(float x) {

  // ASSUMING: 
  // - non-denormalized numbers i.e. x > 2^−126
  // - integer is 32 bit. float is IEEE 32 bit.

  // INSPIRED BY:
  // - https://stackoverflow.com/a/44232045
  // - http://mathonweb.com/help_ebook/html/algorithms.htm#ln
  // - https://en.wikipedia.org/wiki/Fast_inverse_square_root

  // FORMULA: 
  // x = m * 2^p =>
  //   ln(x) = ln(m) + ln(2)p,

  // first normalize the value to between 1.0 and 2.0
  // assuming normalized IEEE float
  //    sign  exp       frac
  // 0b 0    [00000000] 00000000000000000000000
  // value = (-1)^s * M * 2 ^ (exp-127)
  //
  // exp = 127 for x = 1, 
  // so 2^(exp-127) is the multiplier

  // evil floating point bit level hacking
  unsigned int bx = * (unsigned int *) (&x);

  // extract exp, since x>0, sign bit must be 0
  unsigned int ex = bx >> 23;
  signed int t = (signed int)ex-(signed int)127;
  unsigned int s = (t < 0) ? (-t) : t;

  // reinterpret back to float
  //   127 << 23 = 1065353216
  //   0b11111111111111111111111 = 8388607
  bx = 1065353216 | (bx & 8388607);
  x = * (float *) (&bx);


  // use remez algorithm to find approximation between [1,2]
  // - see this answer https://stackoverflow.com/a/44232045
  // - or this usage of C++/boost's remez implementation
  //   https://computingandrecording.wordpress.com/2017/04/24/
  // e.g.
  // boost::math::tools::remez_minimax<double> approx(
  //    [](const double& x) { return log(x); },
  // 4, 0, 1, 2, false, 0, 0, 64);
  //
  // 4th order is:
  // { -1.74178, 2.82117, -1.46994, 0.447178, -0.0565717 }
  // 
  // 3rd order is:
  // { -1.49278, 2.11263, -0.729104, 0.10969 }

  return 

  /* less accurate */
    -1.49278+(2.11263+(-0.729104+0.10969*x)*x)*x      
    
  /* OR more accurate */      
  // -1.7417939+(2.8212026+(-1.4699568+(0.44717955-0.056570851*x)*x)*x)*x

  /* compensate for the ln(2)s. ln(2)=0.6931471806 */    
    + 0.6931471806*t;
}


//////////////////////////////////////////////////////////////////////////////


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
void *third_party_realloc(void *p, size_t size) {
	assert(third_party_allocator.proc, "No third party allocator was set, but it was used!");
	if (!size) return 0;
	if (!p) return third_party_malloc(size);
	return third_party_allocator.proc(size, p, ALLOCATOR_REALLOCATE, 0);
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

