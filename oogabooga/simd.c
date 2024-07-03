


inline void basic_add_float32_64 (float32 *a, float32 *b, float32* result);
inline void basic_add_float32_128(float32 *a, float32 *b, float32* result);
inline void basic_add_float32_256(float32 *a, float32 *b, float32* result);
inline void basic_add_float32_512(float32 *a, float32 *b, float32* result);
inline void basic_sub_float32_64 (float32 *a, float32 *b, float32* result);
inline void basic_sub_float32_128(float32 *a, float32 *b, float32* result);
inline void basic_sub_float32_256(float32 *a, float32 *b, float32* result);
inline void basic_sub_float32_512(float32 *a, float32 *b, float32* result);
inline void basic_mul_float32_64 (float32 *a, float32 *b, float32* result);
inline void basic_mul_float32_128(float32 *a, float32 *b, float32* result);
inline void basic_mul_float32_256(float32 *a, float32 *b, float32* result);
inline void basic_mul_float32_512(float32 *a, float32 *b, float32* result);
inline void basic_div_float32_64 (float32 *a, float32 *b, float32* result);
inline void basic_div_float32_128(float32 *a, float32 *b, float32* result);
inline void basic_div_float32_256(float32 *a, float32 *b, float32* result);
inline void basic_div_float32_512(float32 *a, float32 *b, float32* result);
inline void basic_add_int32_128(s32 *a, s32 *b, s32* result);
inline void basic_add_int32_256(s32 *a, s32 *b, s32* result);
inline void basic_add_int32_512(s32 *a, s32 *b, s32* result);
inline void basic_sub_int32_128(s32 *a, s32 *b, s32* result);
inline void basic_sub_int32_256(s32 *a, s32 *b, s32* result);
inline void basic_sub_int32_512(s32 *a, s32 *b, s32* result);
inline void basic_mul_int32_128(s32 *a, s32 *b, s32* result);
inline void basic_mul_int32_256(s32 *a, s32 *b, s32* result);
inline void basic_mul_int32_512(s32 *a, s32 *b, s32* result);



#if ENABLE_SIMD

#if !COMPILER_CAN_DO_SSE
	#error "Compiler cannot generate SSE instructions but ENABLE_SIMD was 1. Did you pass the sse flag to your compiler?"
#endif

#include <immintrin.h>
#include <intrin.h>


// SSE
inline void simd_add_float32_64(float *a, float *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_sub_float32_64(float *a, float *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_mul_float32_64(float *a, float *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_div_float32_64(float *a, float *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_add_float32_128(float *a, float *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storeu_ps(result, vr);
}

inline void simd_sub_float32_128(float *a, float *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storeu_ps(result, vr);
}

inline void simd_mul_float32_128(float *a, float *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storeu_ps(result, vr);
}

inline void simd_div_float32_128(float *a, float *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storeu_ps(result, vr);
}

#if SIMD_ENABLE_SSE2
// SSE2
#if !COMPILER_CAN_DO_SSE2
	#error "Compiler cannot generate SSE2 instructions but SIMD_ENABLE_SSE2 was 1. Did you pass the sse2 flag to your compiler?"
#endif
inline void simd_add_int32_128(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vr = _mm_add_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result, vr);
}
inline void simd_sub_int32_128(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vr = _mm_sub_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result, vr);
}

#else
	#define simd_add_int32_128 		basic_add_int32_128
	#define simd_sub_int32_128 		basic_sub_int32_128
	
#endif

#if SIMD_ENABLE_SSE41
inline void simd_mul_int32_128(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vr = _mm_mullo_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result, vr);
}
#else
	#define simd_mul_int32_128 		basic_mul_int32_128
#endif // SIMD_ENABLE_SSE41

#if SIMD_ENABLE_AVX
// AVX
#if !COMPILER_CAN_DO_AVX
	#error "Compiler cannot generate AVX instructions but SIMD_ENABLE_AVX was 1. Did you pass the avx flag to your compiler?"
#endif
inline void simd_add_float32_256(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_add_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_sub_float32_256(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_sub_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_mul_float32_256(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_mul_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_div_float32_256(float32 *a, float32 *b, float32* result){
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_div_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
#else
	#define simd_add_float32_256 	basic_add_float32_256
	#define simd_sub_float32_256 	basic_sub_float32_256
	#define simd_mul_float32_256 	basic_mul_float32_256
	#define simd_div_float32_256 	basic_div_float32_256
#endif

#if SIMD_ENABLE_AVX2
// AVX2
#if !COMPILER_CAN_DO_AVX2
	#error "Compiler cannot generate AVX2 instructions but SIMD_ENABLE_AVX2 was 1. Did you pass the avx2 flag to your compiler?"
#endif
inline void simd_add_int32_256(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_loadu_si256((__m256i*)a);
    __m256i vb = _mm256_loadu_si256((__m256i*)b);
    __m256i vr = _mm256_add_epi32(va, vb);
    _mm256_storeu_si256((__m256i*)result, vr);
}
inline void simd_sub_int32_256(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_loadu_si256((__m256i*)a);
    __m256i vb = _mm256_loadu_si256((__m256i*)b);
    __m256i vr = _mm256_sub_epi32(va, vb);
    _mm256_storeu_si256((__m256i*)result, vr);
}
inline void simd_mul_int32_256(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_loadu_si256((__m256i*)a);
    __m256i vb = _mm256_loadu_si256((__m256i*)b);
    __m256i vr = _mm256_mullo_epi32(va, vb);
    _mm256_storeu_si256((__m256i*)result, vr);
}
#else
	#define simd_add_int32_256 		basic_add_int32_256
	#define simd_sub_int32_256 		basic_sub_int32_256
	#define simd_mul_int32_256 		basic_mul_int32_256
#endif

#if SIMD_ENABLE_AVX512
// AVX-512
#if !COMPILER_CAN_DO_AVX512
	#error "Compiler cannot generate AVX512 instructions but SIMD_ENABLE_AVX512 was 1. Did you pass the avx512 flag to your compiler?"
#endif
inline void simd_add_float32_512(float *a, float *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_add_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}

inline void simd_sub_float32_512(float *a, float *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_sub_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}

inline void simd_mul_float32_512(float *a, float *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_mul_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}

inline void simd_div_float32_512(float *a, float *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_div_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}
inline void simd_add_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_add_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}

inline void simd_sub_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_sub_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}

inline void simd_mul_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_mullo_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}
#else 
	#define simd_add_float32_512 	basic_add_float32_512
	#define simd_sub_float32_512 	basic_sub_float32_512
	#define simd_mul_float32_512 	basic_mul_float32_512
	#define simd_div_float32_512 	basic_div_float32_512
	#define simd_add_int32_512 		basic_add_int32_512
	#define simd_sub_int32_512 		basic_sub_int32_512
	#define simd_mul_int32_512 		basic_mul_int32_512
#endif // SIMD_ENABLE_AVX512

#else

// SSE
#define simd_add_float32_64 	basic_add_float32_64
#define simd_add_float32_128 	basic_add_float32_128
#define simd_sub_float32_64 	basic_sub_float32_64
#define simd_sub_float32_128 	basic_sub_float32_128
#define simd_mul_float32_64 	basic_mul_float32_64
#define simd_mul_float32_128 	basic_mul_float32_128
#define simd_div_float32_64 	basic_div_float32_64
#define simd_div_float32_128 	basic_div_float32_128

// SSE2
#define simd_add_int32_128 		basic_add_int32_128
#define simd_sub_int32_128 		basic_sub_int32_128
#define simd_mul_int32_128 		basic_mul_int32_128

// AVX
#define simd_add_float32_256 	basic_add_float32_256
#define simd_sub_float32_256 	basic_sub_float32_256
#define simd_mul_float32_256 	basic_mul_float32_256
#define simd_div_float32_256 	basic_div_float32_256

// AVX2
#define simd_add_int32_256 		basic_add_int32_256
#define simd_sub_int32_256 		basic_sub_int32_256
#define simd_mul_int32_256 		basic_mul_int32_256

// AVX512
#define simd_add_float32_512 	basic_add_float32_512
#define simd_sub_float32_512 	basic_sub_float32_512
#define simd_mul_float32_512 	basic_mul_float32_512
#define simd_div_float32_512 	basic_div_float32_512
#define simd_add_int32_512 		basic_add_int32_512
#define simd_sub_int32_512 		basic_sub_int32_512
#define simd_mul_int32_512 		basic_mul_int32_512

#endif

inline void basic_add_float32_64 (float32 *a, float32 *b, float32* result) {
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
}
inline void basic_add_float32_128(float32 *a, float32 *b, float32* result) {
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];
	result[3] = a[3] + b[3];
}
inline void basic_add_float32_256(float32 *a, float32 *b, float32* result) {
	simd_add_float32_128(a, b, result);
	simd_add_float32_128(a+4, b+4, result+4);
}
inline void basic_add_float32_512(float32 *a, float32 *b, float32* result) {
	simd_add_float32_256(a, b, result);
	simd_add_float32_256(a+8, b+8, result+8);
}
inline void basic_sub_float32_64 (float32 *a, float32 *b, float32* result) {
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
}
inline void basic_sub_float32_128(float32 *a, float32 *b, float32* result) {
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
	result[2] = a[2] - b[2];
	result[3] = a[3] - b[3];
}
inline void basic_sub_float32_256(float32 *a, float32 *b, float32* result) {
	simd_sub_float32_128(a, b, result);
	simd_sub_float32_128(a+4, b+4, result+4);
}
inline void basic_sub_float32_512(float32 *a, float32 *b, float32* result) {
	simd_sub_float32_256(a, b, result);
	simd_sub_float32_256(a+8, b+8, result+8);
}
inline void basic_mul_float32_64 (float32 *a, float32 *b, float32* result) {
	result[0] = a[0] * b[0];
	result[1] = a[1] * b[1];
}
inline void basic_mul_float32_128(float32 *a, float32 *b, float32* result) {
	result[0] = a[0] * b[0];
	result[1] = a[1] * b[1];
	result[2] = a[2] * b[2];
	result[3] = a[3] * b[3];
}
inline void basic_mul_float32_256(float32 *a, float32 *b, float32* result) {
	simd_mul_float32_128(a, b, result);
	simd_mul_float32_128(a+4, b+4, result+4);
}
inline void basic_mul_float32_512(float32 *a, float32 *b, float32* result) {
	simd_mul_float32_256(a, b, result);
	simd_mul_float32_256(a+8, b+8, result+8);
}
inline void basic_div_float32_64 (float32 *a, float32 *b, float32* result) {
	result[0] = a[0] / b[0];
	result[1] = a[1] / b[1];
}
inline void basic_div_float32_128(float32 *a, float32 *b, float32* result) {
	result[0] = a[0] / b[0];
	result[1] = a[1] / b[1];
	result[2] = a[2] / b[2];
	result[3] = a[3] / b[3];
}
inline void basic_div_float32_256(float32 *a, float32 *b, float32* result) {
	simd_div_float32_128(a, b, result);
	simd_div_float32_128(a+4, b+4, result+4);
}
inline void basic_div_float32_512(float32 *a, float32 *b, float32* result) {
	simd_div_float32_256(a, b, result);
	simd_div_float32_256(a+8, b+8, result+8);
}
inline void basic_add_int32_128(s32 *a, s32 *b, s32* result) {
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];
	result[3] = a[3] + b[3];
}
inline void basic_add_int32_256(s32 *a, s32 *b, s32* result) {
	simd_add_int32_128(a, b, result);
	simd_add_int32_128(a+4, b+4, result+4);
}
inline void basic_add_int32_512(s32 *a, s32 *b, s32* result) {
	simd_add_int32_256(a, b, result);
	simd_add_int32_256(a+8, b+8, result+8);
}
inline void basic_sub_int32_128(s32 *a, s32 *b, s32* result) {
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
	result[2] = a[2] - b[2];
	result[3] = a[3] - b[3];
}
inline void basic_sub_int32_256(s32 *a, s32 *b, s32* result) {
	simd_sub_int32_128(a, b, result);
	simd_sub_int32_128(a+4, b+4, result+4);
}
inline void basic_sub_int32_512(s32 *a, s32 *b, s32* result) {
	simd_sub_int32_256(a, b, result);
	simd_sub_int32_256(a+8, b+8, result+8);
}
inline void basic_mul_int32_128(s32 *a, s32 *b, s32* result) {
	result[0] = a[0] * b[0];
	result[1] = a[1] * b[1];
	result[2] = a[2] * b[2];
	result[3] = a[3] * b[3];
}
inline void basic_mul_int32_256(s32 *a, s32 *b, s32* result) {
	simd_mul_int32_128(a, b, result);
	simd_mul_int32_128(a+4, b+4, result+4);
}
inline void basic_mul_int32_512(s32 *a, s32 *b, s32* result) {
	simd_mul_int32_256(a, b, result);
	simd_mul_int32_256(a+8, b+8, result+8);
}






// SSE 2 int32
/*inline void sse_add_int32_128(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "movdqa (%0), %%xmm0\n\t"
        "movdqa (%1), %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_sub_int32_128(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "movdqa (%0), %%xmm0\n\t"
        "movdqa (%1), %%xmm1\n\t"
        "psubd %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_mul_int32_128(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "movdqa (%0), %%xmm0\n\t"
        "movdqa (%1), %%xmm1\n\t"
        "pmulld %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

// SSE4.2 float32
inline void sse_add_float32_64(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t" 
        "addps %%xmm1, %%xmm0\n\t" 
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_add_float32_128(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "addps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_sub_float32_64(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "subps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_sub_float32_128(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "subps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
    
}

inline void sse_mul_float32_64(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "mulps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_mul_float32_128(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "mulps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_div_float32_64(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "divps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

inline void sse_div_float32_128(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "movups (%0), %%xmm0\n\t"
        "movups (%1), %%xmm1\n\t"
        "divps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, (%2)\n\t"
        :
        : "r" (a), "r" (b), "r" (result)
        : "xmm0", "xmm1"
    );
}

// AVX float32
inline void avx_add_float32_256(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%ymm0\n\t"
        "vmovups %2, %%ymm1\n\t"
        "vaddps %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

inline void avx_sub_float32_256(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%ymm0\n\t"
        "vmovups %2, %%ymm1\n\t"
        "vsubps %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

inline void avx_mul_float32_256(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%ymm0\n\t"
        "vmovups %2, %%ymm1\n\t"
        "vmulps %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

inline void avx_div_float32_256(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%ymm0\n\t"
        "vmovups %2, %%ymm1\n\t"
        "vdivps %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

// AVX2 int32
inline void avx2_add_int32_256(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu %1, %%ymm0\n\t"
        "vmovdqu %2, %%ymm1\n\t"
        "vpaddd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovdqu %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

inline void avx2_sub_int32_256(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu %1, %%ymm0\n\t"
        "vmovdqu %2, %%ymm1\n\t"
        "vpsubd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovdqu %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

inline void avx2_mul_int32_256(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu %1, %%ymm0\n\t"
        "vmovdqu %2, %%ymm1\n\t"
        "vpmulld %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovdqu %%ymm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "ymm0", "ymm1"
    );
}

// AVX-512 float32
inline void avx512_add_float32_512(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%zmm0\n\t"
        "vmovups %2, %%zmm1\n\t"
        "vaddps %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovups %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

inline void avx512_sub_float32_512(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%zmm0\n\t"
        "vmovups %2, %%zmm1\n\t"
        "vsubps %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovups %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

inline void avx512_mul_float32_512(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%zmm0\n\t"
        "vmovups %2, %%zmm1\n\t"
        "vmulps %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovups %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

inline void avx512_div_float32_512(float32 *a, float32 *b, float32* result) {
    __asm__ (
        "vmovups %1, %%zmm0\n\t"
        "vmovups %2, %%zmm1\n\t"
        "vdivps %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovups %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

// AVX-512 int32
inline void avx512_add_int32_512(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu32 %1, %%zmm0\n\t"
        "vmovdqu32 %2, %%zmm1\n\t"
        "vpaddd %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovdqu32 %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

inline void avx512_sub_int32_512(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu32 %1, %%zmm0\n\t"
        "vmovdqu32 %2, %%zmm1\n\t"
        "vpsubd %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovdqu32 %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}

inline void avx512_mul_int32_512(s32 *a, s32 *b, s32* result) {
    __asm__ (
        "vmovdqu32 %1, %%zmm0\n\t"
        "vmovdqu32 %2, %%zmm1\n\t"
        "vpmulld %%zmm1, %%zmm0, %%zmm0\n\t"
        "vmovdqu32 %%zmm0, %0\n\t"
        : "=m" (*result)
        : "m" (*a), "m" (*b)
        : "zmm0", "zmm1"
    );
}*/
