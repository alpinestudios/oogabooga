// #Portability rip ARM
typedef struct Cpu_Info_X86 {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
} Cpu_Info_X86;

typedef struct Cpu_Capabilities {
	bool sse1;
	bool sse2;
	bool sse3;
	bool ssse3;
	bool sse41;
	bool sse42;
	bool any_sse;
	bool avx;
	bool avx2;
	bool avx512;
	
} Cpu_Capabilities;

// I think this is the standard? (sse1)
#define COMPILER_CAN_DO_SSE 1

///
// Compiler specific stuff
#if COMPILER_MVSC
	#define inline __forceinline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    #include <intrin.h>
    #pragma intrinsic(__rdtsc)
    inline u64 rdtsc() {
        return __rdtsc();
    }
    inline Cpu_Info_X86 cpuid(u32 function_id) {
    	Cpu_Info_X86 i;
    	__cpuid((int*)&i, function_id);
    	return i;
    }
    
    #if _M_IX86_FP >= 2
		#define COMPILER_CAN_DO_SSE2 1
		#define COMPILER_CAN_DO_SSE41 1
	#else
		#define COMPILER_CAN_DO_SSE2 0
		#define COMPILER_CAN_DO_SSE41 0
	#endif
	#ifdef __AVX__
		#define COMPILER_CAN_DO_AVX 1
	#else
		#define COMPILER_CAN_DO_AVX 0
	#endif
	#ifdef __AVX2__
		#define COMPILER_CAN_DO_AVX2 1
	#else
		#define COMPILER_CAN_DO_AVX2 0
	#endif
	#ifdef __AVX512F__
		#define COMPILER_CAN_DO_AVX512 1
	#else
		#define COMPILER_CAN_DO_AVX512 0
	#endif
#elif COMPILER_GCC || COMPILER_CLANG
	#define inline __attribute__((always_inline)) inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline u64 rdtsc() {
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
    inline Cpu_Info_X86 cpuid(u32 function_id) {
    Cpu_Info_X86 info;
	    __asm__ __volatile__(
	        "cpuid"
	        : "=a"(info.eax), "=b"(info.ebx), "=c"(info.ecx), "=d"(info.edx)
	        : "a"(function_id), "c"(0));
	    return info;
	}
	
	#ifdef __SSE2__
		#define COMPILER_CAN_DO_SSE2 1
	#else
		#define COMPILER_CAN_DO_SSE2 0
	#endif
	#ifdef __SSE4_1__
		#define COMPILER_CAN_DO_SSE41 1
	#else
		#define COMPILER_CAN_DO_SSE41 0
	#endif
	#ifdef __AVX__
		#define COMPILER_CAN_DO_AVX 1
	#else
		#define COMPILER_CAN_DO_AVX 0
	#endif
	#ifdef __AVX2__
		#define COMPILER_CAN_DO_AVX2 1
	#else
		#define COMPILER_CAN_DO_AVX2 0
	#endif
	#ifdef __AVX512F__
		#define COMPILER_CAN_DO_AVX512 1
	#else
		#define COMPILER_CAN_DO_AVX512 0
	#endif
#else
	#define inline inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 0
    
    inline u64 rdtsc() { return 0; }
    inline Cpu_Info_X86 cpuid(u32 function_id) {return (Cpu_Info_X86){0};}
    #define COMPILER_CAN_DO_SSE2 0
    #define COMPILER_CAN_DO_AVX 0
    #define COMPILER_CAN_DO_AVX2 0
    #define COMPILER_CAN_DO_AVX512 0
    
    #warning "Compiler is not explicitly supported, some things will probably not work as expected"
#endif


Cpu_Capabilities query_cpu_capabilities() {
    Cpu_Capabilities result = {0};

    Cpu_Info_X86 info = cpuid(1);

    result.sse1  = (info.edx & (1 << 25)) != 0;
    result.sse2  = (info.edx & (1 << 26)) != 0;
    result.sse3  = (info.ecx & (1 << 0)) != 0;
    result.ssse3 = (info.ecx & (1 << 9)) != 0;
    result.sse41 = (info.ecx & (1 << 19)) != 0;
    result.sse42 = (info.ecx & (1 << 20)) != 0;
    result.any_sse = result.sse1 || result.sse2 || result.sse3 || result.ssse3 || result.sse41 || result.sse42;
    
    result.avx = (info.ecx & (1 << 28)) != 0;

    Cpu_Info_X86 ext_info = cpuid(7);
    result.avx2 = (ext_info.ebx & (1 << 5)) != 0;
    
    result.avx512 = (ext_info.ebx & (1 << 16)) != 0;

    return result;
}

