


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

inline float32 basic_dot_product_float32_64(float32 *a, float32 *b);
inline float32 basic_dot_product_float32_96(float32 *a, float32 *b);
inline float32 basic_dot_product_float32_128(float32 *a, float32 *b);
inline void basic_sqrt_float32_64(float32 *a, float32 *result);
inline void basic_sqrt_float32_96(float32 *a, float32 *result);
inline void basic_sqrt_float32_128(float32 *a, float32 *result);
inline void basic_sqrt_float32_256(float32 *a, float32 *result);
inline void basic_sqrt_float32_512(float32 *a, float32 *result);
inline void basic_rsqrt_float32_64(float32 *a, float32 *result);
inline void basic_rsqrt_float32_96(float32 *a, float32 *result);
inline void basic_rsqrt_float32_128(float32 *a, float32 *result);
inline void basic_rsqrt_float32_256(float32 *a, float32 *result);
inline void basic_rsqrt_float32_512(float32 *a, float32 *result);



#if ENABLE_SIMD

#if !COMPILER_CAN_DO_SSE
	#error "Compiler cannot generate SSE instructions but ENABLE_SIMD was 1. Did you pass the sse flag to your compiler?"
#endif

#include <immintrin.h>
#include <intrin.h>


// SSE
inline void simd_add_float32_64(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_sub_float32_64(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_mul_float32_64(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_div_float32_64(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vb = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_add_float32_128(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_storeu_ps(result, vr);
}
inline void simd_add_float32_128_aligned(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_load_ps(a);
    __m128 vb = _mm_load_ps(b);
    __m128 vr = _mm_add_ps(va, vb);
    _mm_store_ps(result, vr);
}

inline void simd_sub_float32_128(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_storeu_ps(result, vr);
}
inline void simd_sub_float32_128_aligned(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_load_ps(a);
    __m128 vb = _mm_load_ps(b);
    __m128 vr = _mm_sub_ps(va, vb);
    _mm_store_ps(result, vr);
}

inline void simd_mul_float32_128(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_storeu_ps(result, vr);
}
inline void simd_mul_float32_128_aligned(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_load_ps(a);
    __m128 vb = _mm_load_ps(b);
    __m128 vr = _mm_mul_ps(va, vb);
    _mm_store_ps(result, vr);
}

inline void simd_div_float32_128(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vb = _mm_loadu_ps(b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_storeu_ps(result, vr);
}
inline void simd_div_float32_128_aligned(float32 *a, float32 *b, float* result) {
    __m128 va = _mm_load_ps(a);
    __m128 vb = _mm_load_ps(b);
    __m128 vr = _mm_div_ps(va, vb);
    _mm_store_ps(result, vr);
}
inline void simd_sqrt_float32_96(float32 *a, float32 *result) {
    __m128 va = _mm_loadu_ps(a);
    va = _mm_and_ps(va, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));  // Mask last element
    __m128 vr = _mm_sqrt_ps(va);
    _mm_storeu_ps(result, vr);
}

inline void simd_rsqrt_float32_96(float32 *a, float32 *result) {
    __m128 va = _mm_loadu_ps(a);
    va = _mm_and_ps(va, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));  // Mask last element
    __m128 vr = _mm_rsqrt_ps(va);
    _mm_storeu_ps(result, vr);
}
inline void simd_sqrt_float32_64(float32 *a, float32 *result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vr = _mm_sqrt_ps(va);
    _mm_storel_pi((__m64*)result, vr);
}

inline void simd_rsqrt_float32_64(float32 *a, float32 *result) {
    __m128 va = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vr = _mm_rsqrt_ps(va);
    _mm_storel_pi((__m64*)result, vr);
}
inline void simd_sqrt_float32_128(float32 *a, float32 *result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vr = _mm_sqrt_ps(va);
    _mm_storeu_ps(result, vr);
}

inline void simd_rsqrt_float32_128(float32 *a, float32 *result) {
    __m128 va = _mm_loadu_ps(a);
    __m128 vr = _mm_rsqrt_ps(va);
    _mm_storeu_ps(result, vr);
}
inline void simd_sqrt_float32_128_aligned(float32 *a, float32 *result) {
    __m128 va = _mm_load_ps(a);
    __m128 vr = _mm_sqrt_ps(va);
    _mm_store_ps(result, vr);
}

inline void simd_rsqrt_float32_128_aligned(float32 *a, float32 *result) {
    __m128 va = _mm_load_ps(a);
    __m128 vr = _mm_rsqrt_ps(va);
    _mm_store_ps(result, vr);
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
inline void simd_add_int32_128_aligned(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_load_si128((__m128i*)a);
    __m128i vb = _mm_load_si128((__m128i*)b);
    __m128i vr = _mm_add_epi32(va, vb);
    _mm_store_si128((__m128i*)result, vr);
}
inline void simd_sub_int32_128(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vr = _mm_sub_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result, vr);
}
inline void simd_sub_int32_128_aligned(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_load_si128((__m128i*)a);
    __m128i vb = _mm_load_si128((__m128i*)b);
    __m128i vr = _mm_sub_epi32(va, vb);
    _mm_store_si128((__m128i*)result, vr);
}

#else
	#define simd_add_int32_128 		basic_add_int32_128
	#define simd_sub_int32_128 		basic_sub_int32_128
	
	#define simd_add_int32_128_aligned 		basic_add_int32_128
	#define simd_sub_int32_128_aligned 		basic_sub_int32_128
	
#endif

#if SIMD_ENABLE_SSE41
inline void simd_mul_int32_128(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vr = _mm_mullo_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result, vr);
}
inline void simd_mul_int32_128_aligned(s32 *a, s32 *b, s32* result) {
    __m128i va = _mm_load_si128((__m128i*)a);
    __m128i vb = _mm_load_si128((__m128i*)b);
    __m128i vr = _mm_mullo_epi32(va, vb);
    _mm_store_si128((__m128i*)result, vr);
}
inline float32 simd_dot_product_float32_64(float32 *a, float32 *b) {
    __m128 vec1 = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)a);
    __m128 vec2 = _mm_loadl_pi(_mm_setzero_ps(), (__m64*)b);
    __m128 dot_product = _mm_dp_ps(vec1, vec2, 0x31);
    return _mm_cvtss_f32(dot_product);
}
inline float32 simd_dot_product_float32_96(float32 *a, float32 *b) {
    __m128 vec1 = _mm_loadu_ps(a);
    __m128 vec2 = _mm_loadu_ps(b);
    vec1 = _mm_and_ps(vec1, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));
    vec2 = _mm_and_ps(vec2, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));
    __m128 dot_product = _mm_dp_ps(vec1, vec2, 0x71);
    return _mm_cvtss_f32(dot_product);
}
inline float32 simd_dot_product_float32_128(float32 *a, float32 *b) {
    __m128 vec1 = _mm_loadu_ps(a);
    __m128 vec2 = _mm_loadu_ps(b);
    __m128 dot_product = _mm_dp_ps(vec1, vec2, 0xF1);
    return _mm_cvtss_f32(dot_product);
}
inline float32 simd_dot_product_float32_128_aligned(float32 *a, float32 *b) {
    __m128 vec1 = _mm_load_ps(a);
    __m128 vec2 = _mm_load_ps(b);
    __m128 dot_product = _mm_dp_ps(vec1, vec2, 0xF1);
    return _mm_cvtss_f32(dot_product);
}
#else
	#define simd_mul_int32_128 		basic_mul_int32_128
	#define simd_mul_int32_128_aligned 		basic_mul_int32_128
	#define simd_dot_product_float32_64 basic_dot_product_float32_64
	#define simd_dot_product_float32_96 basic_dot_product_float32_96
	#define simd_dot_product_float32_128 basic_dot_product_float32_128
	#define simd_dot_product_float32_128_aligned basic_dot_product_float32_128
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
inline void simd_add_float32_256_aligned(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);
    __m256 vr = _mm256_add_ps(va, vb);
    _mm256_store_ps(result, vr);
}
inline void simd_sub_float32_256(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_sub_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_sub_float32_256_aligned(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);
    __m256 vr = _mm256_sub_ps(va, vb);
    _mm256_store_ps(result, vr);
}
inline void simd_mul_float32_256(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_mul_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_mul_float32_256_aligned(float32 *a, float32 *b, float32* result) {
    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);
    __m256 vr = _mm256_mul_ps(va, vb);
    _mm256_store_ps(result, vr);
}
inline void simd_div_float32_256(float32 *a, float32 *b, float32* result){
    __m256 va = _mm256_loadu_ps(a);
    __m256 vb = _mm256_loadu_ps(b);
    __m256 vr = _mm256_div_ps(va, vb);
    _mm256_storeu_ps(result, vr);
}
inline void simd_div_float32_256_aligned(float32 *a, float32 *b, float32* result){
    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);
    __m256 vr = _mm256_div_ps(va, vb);
    _mm256_store_ps(result, vr);
}
inline void simd_sqrt_float32_256(float32 *a, float32 *result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vr = _mm256_sqrt_ps(va);
    _mm256_storeu_ps(result, vr);
}

inline void simd_rsqrt_float32_256(float32 *a, float32 *result) {
    __m256 va = _mm256_loadu_ps(a);
    __m256 vr = _mm256_rsqrt_ps(va);
    _mm256_storeu_ps(result, vr);
}
inline void simd_sqrt_float32_256_aligned(float32 *a, float32 *result) {
    __m256 va = _mm256_load_ps(a);
    __m256 vr = _mm256_sqrt_ps(va);
    _mm256_store_ps(result, vr);
}

inline void simd_rsqrt_float32_256_aligned(float32 *a, float32 *result) {
    __m256 va = _mm256_load_ps(a);
    __m256 vr = _mm256_rsqrt_ps(va);
    _mm256_store_ps(result, vr);
}
#else
	#define simd_add_float32_256 	basic_add_float32_256
	#define simd_sub_float32_256 	basic_sub_float32_256
	#define simd_mul_float32_256 	basic_mul_float32_256
	#define simd_div_float32_256 	basic_div_float32_256
	#define simd_sqrt_float32_256   		basic_sqrt_float32_256
	#define simd_rsqrt_float32_256  		basic_rsqrt_float32_256
	#define simd_add_float32_256_aligned 	basic_add_float32_256
	#define simd_sub_float32_256_aligned 	basic_sub_float32_256
	#define simd_mul_float32_256_aligned 	basic_mul_float32_256
	#define simd_div_float32_256_aligned 	basic_div_float32_256
	#define simd_sqrt_float32_256_aligned   basic_sqrt_float32_256
	#define simd_rsqrt_float32_256_aligned  basic_rsqrt_float32_256
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
inline void simd_add_int32_256_aligned(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_load_si256((__m256i*)a);
    __m256i vb = _mm256_load_si256((__m256i*)b);
    __m256i vr = _mm256_add_epi32(va, vb);
    _mm256_store_si256((__m256i*)result, vr);
}
inline void simd_sub_int32_256(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_loadu_si256((__m256i*)a);
    __m256i vb = _mm256_loadu_si256((__m256i*)b);
    __m256i vr = _mm256_sub_epi32(va, vb);
    _mm256_storeu_si256((__m256i*)result, vr);
}
inline void simd_sub_int32_256_aligned(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_load_si256((__m256i*)a);
    __m256i vb = _mm256_load_si256((__m256i*)b);
    __m256i vr = _mm256_sub_epi32(va, vb);
    _mm256_store_si256((__m256i*)result, vr);
}
inline void simd_mul_int32_256(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_loadu_si256((__m256i*)a);
    __m256i vb = _mm256_loadu_si256((__m256i*)b);
    __m256i vr = _mm256_mullo_epi32(va, vb);
    _mm256_storeu_si256((__m256i*)result, vr);
}
inline void simd_mul_int32_256_aligned(s32 *a, s32 *b, s32* result) {
	__m256i va = _mm256_load_si256((__m256i*)a);
    __m256i vb = _mm256_load_si256((__m256i*)b);
    __m256i vr = _mm256_mullo_epi32(va, vb);
    _mm256_store_si256((__m256i*)result, vr);
}
#else
	#define simd_add_int32_256 		basic_add_int32_256
	#define simd_sub_int32_256 		basic_sub_int32_256
	#define simd_mul_int32_256 		basic_mul_int32_256
	#define simd_add_int32_256_aligned 		basic_add_int32_256
	#define simd_sub_int32_256_aligned 		basic_sub_int32_256
	#define simd_mul_int32_256_aligned 		basic_mul_int32_256
#endif

#if SIMD_ENABLE_AVX512
// AVX-512
#if !COMPILER_CAN_DO_AVX512
	#error "Compiler cannot generate AVX512 instructions but SIMD_ENABLE_AVX512 was 1. Did you pass the avx512 flag to your compiler?"
#endif
inline void simd_add_float32_512(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_add_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}
inline void simd_add_float32_512_aligned(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    __m512 vr = _mm512_add_ps(va, vb);
    _mm512_store_ps(result, vr);
}

inline void simd_sub_float32_512(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_sub_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}
inline void simd_sub_float32_512_aligned(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    __m512 vr = _mm512_sub_ps(va, vb);
    _mm512_store_ps(result, vr);
}

inline void simd_mul_float32_512(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_mul_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}
inline void simd_mul_float32_512_aligned(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    __m512 vr = _mm512_mul_ps(va, vb);
    _mm512_store_ps(result, vr);
}

inline void simd_div_float32_512(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __m512 vr = _mm512_div_ps(va, vb);
    _mm512_storeu_ps(result, vr);
}
inline void simd_div_float32_512_aligned(float32 *a, float32 *b, float* result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    __m512 vr = _mm512_div_ps(va, vb);
    _mm512_store_ps(result, vr);
}
inline void simd_add_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_add_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}
inline void simd_add_int32_512_aligned(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    __m512i vr = _mm512_add_epi32(va, vb);
    _mm512_store_si512((__m512i*)result, vr);
}

inline void simd_sub_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_sub_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}
inline void simd_sub_int32_512_aligned(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    __m512i vr = _mm512_sub_epi32(va, vb);
    _mm512_store_si512((__m512i*)result, vr);
}

inline void simd_mul_int32_512(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    __m512i vr = _mm512_mullo_epi32(va, vb);
    _mm512_storeu_si512((__m512i*)result, vr);
}
inline void simd_mul_int32_512_aligned(int32 *a, int32 *b, int32* result) {
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    __m512i vr = _mm512_mullo_epi32(va, vb);
    _mm512_store_si512((__m512i*)result, vr);
}
inline void simd_sqrt_float32_512(float32 *a, float32 *result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vr = _mm512_sqrt_ps(va);
    _mm512_storeu_ps(result, vr);
}

inline void simd_rsqrt_float32_512(float32 *a, float32 *result) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vr = _mm512_rsqrt14_ps(va);  // AVX-512 does not have _mm512_rsqrt_ps
    _mm512_storeu_ps(result, vr);
}
inline void simd_sqrt_float32_512_aligned(float32 *a, float32 *result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vr = _mm512_sqrt_ps(va);
    _mm512_store_ps(result, vr);
}

inline void simd_rsqrt_float32_512_aligned(float32 *a, float32 *result) {
    __m512 va = _mm512_load_ps(a);
    __m512 vr = _mm512_rsqrt14_ps(va);
    _mm512_store_ps(result, vr);
}
#else 
	#define simd_add_float32_512 	basic_add_float32_512
	#define simd_sub_float32_512 	basic_sub_float32_512
	#define simd_mul_float32_512 	basic_mul_float32_512
	#define simd_div_float32_512 	basic_div_float32_512
	#define simd_add_int32_512 		basic_add_int32_512
	#define simd_sub_int32_512 		basic_sub_int32_512
	#define simd_mul_int32_512 		basic_mul_int32_512
	#define simd_sqrt_float32_512   basic_sqrt_float32_512
	#define simd_rsqrt_float32_512  basic_rsqrt_float32_512
	#define simd_add_float32_512_aligned 	basic_add_float32_512
	#define simd_sub_float32_512_aligned 	basic_sub_float32_512
	#define simd_mul_float32_512_aligned 	basic_mul_float32_512
	#define simd_div_float32_512_aligned 	basic_div_float32_512
	#define simd_add_int32_512_aligned 		basic_add_int32_512
	#define simd_sub_int32_512_aligned 		basic_sub_int32_512
	#define simd_mul_int32_512_aligned 		basic_mul_int32_512
	#define simd_sqrt_float32_512_aligned   basic_sqrt_float32_512
	#define simd_rsqrt_float32_512_aligned  basic_rsqrt_float32_512
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
#define simd_sqrt_float32_64   	basic_sqrt_float32_64
#define simd_sqrt_float32_128   basic_sqrt_float32_128
#define simd_rsqrt_float32_64   basic_rsqrt_float32_64
#define simd_rsqrt_float32_128  basic_rsqrt_float32_128
#define simd_add_float32_128_aligned 	basic_add_float32_128
#define simd_sub_float32_128_aligned 	basic_sub_float32_128
#define simd_mul_float32_128_aligned 	basic_mul_float32_128
#define simd_div_float32_128_aligned 	basic_div_float32_128
#define simd_sqrt_float32_128_aligned   basic_sqrt_float32_128
#define simd_rsqrt_float32_128_aligned  basic_rsqrt_float32_128

// SSE2
#define simd_add_int32_128 		basic_add_int32_128
#define simd_sub_int32_128 		basic_sub_int32_128
#define simd_mul_int32_128 		basic_mul_int32_128
#define simd_add_int32_128_aligned 		basic_add_int32_128
#define simd_sub_int32_128_aligned 		basic_sub_int32_128
#define simd_mul_int32_128_aligned 		basic_mul_int32_128

// SSE41
#define simd_mul_int32_128 		basic_mul_int32_128
#define simd_mul_int32_128_aligned 		basic_mul_int32_128
#define simd_dot_product_float32_64 basic_dot_product_float32_64
#define simd_dot_product_float32_96 basic_dot_product_float32_96
#define simd_dot_product_float32_128 basic_dot_product_float32_128
#define simd_dot_product_float32_128_aligned basic_dot_product_float32_128

// AVX
#define simd_add_float32_256 	basic_add_float32_256
#define simd_sub_float32_256 	basic_sub_float32_256
#define simd_mul_float32_256 	basic_mul_float32_256
#define simd_div_float32_256 	basic_div_float32_256
#define simd_sqrt_float32_256   		basic_sqrt_float32_256
#define simd_rsqrt_float32_256  		basic_rsqrt_float32_256
#define simd_add_float32_256_aligned 	basic_add_float32_256
#define simd_sub_float32_256_aligned 	basic_sub_float32_256
#define simd_mul_float32_256_aligned 	basic_mul_float32_256
#define simd_div_float32_256_aligned 	basic_div_float32_256
#define simd_sqrt_float32_256_aligned   basic_sqrt_float32_256
#define simd_rsqrt_float32_256_aligned  basic_rsqrt_float32_256

// AVX2
#define simd_add_int32_256 		basic_add_int32_256
#define simd_sub_int32_256 		basic_sub_int32_256
#define simd_mul_int32_256 		basic_mul_int32_256
#define simd_add_int32_256_aligned 		basic_add_int32_256
#define simd_sub_int32_256_aligned 		basic_sub_int32_256
#define simd_mul_int32_256_aligned 		basic_mul_int32_256

// AVX512
#define simd_add_float32_512 	basic_add_float32_512
#define simd_sub_float32_512 	basic_sub_float32_512
#define simd_mul_float32_512 	basic_mul_float32_512
#define simd_div_float32_512 	basic_div_float32_512
#define simd_add_int32_512 		basic_add_int32_512
#define simd_sub_int32_512 		basic_sub_int32_512
#define simd_mul_int32_512 		basic_mul_int32_512
#define simd_sqrt_float32_512   basic_sqrt_float32_512
#define simd_rsqrt_float32_512  basic_rsqrt_float32_512
#define simd_add_float32_512_aligned 	basic_add_float32_512
#define simd_sub_float32_512_aligned 	basic_sub_float32_512
#define simd_mul_float32_512_aligned 	basic_mul_float32_512
#define simd_div_float32_512_aligned 	basic_div_float32_512
#define simd_add_int32_512_aligned 		basic_add_int32_512
#define simd_sub_int32_512_aligned 		basic_sub_int32_512
#define simd_mul_int32_512_aligned 		basic_mul_int32_512
#define simd_sqrt_float32_512_aligned   basic_sqrt_float32_512
#define simd_rsqrt_float32_512_aligned  basic_rsqrt_float32_512

#endif

float64 __cdecl sqrt(_In_ float64 _X);
float64 __cdecl rsqrt(_In_ float64 _X);

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
inline float32 basic_dot_product_float32_64(float32 *a, float32 *b) {
    return a[0] * b[0] + a[1] * b[1];
}
inline float32 basic_dot_product_float32_96(float32 *a, float32 *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
inline float32 basic_dot_product_float32_128(float32 *a, float32 *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}
inline void basic_sqrt_float32_64(float32 *a, float32 *result) {
    result[0] = sqrt(a[0]);
    result[1] = sqrt(a[1]);
}
inline void basic_sqrt_float32_96(float32 *a, float32 *result) {
    result[0] = sqrt(a[0]);
    result[1] = sqrt(a[1]);
    result[2] = sqrt(a[2]);
}
inline void basic_sqrt_float32_128(float32 *a, float32 *result) {
    result[0] = sqrt(a[0]);
    result[1] = sqrt(a[1]);
    result[2] = sqrt(a[2]);
    result[3] = sqrt(a[3]);
}
inline void basic_sqrt_float32_256(float32 *a, float32 *result) {
    basic_sqrt_float32_128(a, result);
    basic_sqrt_float32_128(a+4, result+4);
}
inline void basic_sqrt_float32_512(float32 *a, float32 *result) {
    basic_sqrt_float32_256(a, result);
    basic_sqrt_float32_256(a+8, result+8);
}
inline void basic_rsqrt_float32_64(float32 *a, float32 *result) {
    result[0] = rsqrt(a[0]);
    result[1] = rsqrt(a[1]);
}
inline void basic_rsqrt_float32_96(float32 *a, float32 *result) {
    result[0] = rsqrt(a[0]);
    result[1] = rsqrt(a[1]);
    result[2] = rsqrt(a[2]);
}
inline void basic_rsqrt_float32_128(float32 *a, float32 *result) {
    result[0] = rsqrt(a[0]);
    result[1] = rsqrt(a[1]);
    result[2] = rsqrt(a[2]);
    result[3] = rsqrt(a[3]);
}
inline void basic_rsqrt_float32_256(float32 *a, float32 *result) {
    basic_rsqrt_float32_128(a, result);
    basic_rsqrt_float32_128(a+4, result+4);
}
inline void basic_rsqrt_float32_512(float32 *a, float32 *result) {
    basic_rsqrt_float32_256(a, result);
    basic_rsqrt_float32_256(a+8, result+8);
}

