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
	#define alignat(x) __declspec(align(x))
	#define noreturn __declspec(noreturn)
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline void 
    crash() noreturn {
		__debugbreak();
		volatile int *a = 0;
		*a = 5;
		a = (volatile int*)0xDEADBEEF;
    	*a = 5;
	}
    #include <intrin.h>
    #pragma intrinsic(__rdtsc)
    inline u64 
    rdtsc() {
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
	
	#define DEPRECATED(proc, msg) __declspec(deprecated(msg)) func
	
	#pragma intrinsic(_InterlockedCompareExchange8)
	#pragma intrinsic(_InterlockedCompareExchange16)
	#pragma intrinsic(_InterlockedCompareExchange)
	#pragma intrinsic(_InterlockedCompareExchange64)
	
	inline bool 
	compare_and_swap_8(volatile uint8_t *a, uint8_t b, uint8_t old) {
	    return _InterlockedCompareExchange8((volatile char*)a, (char)b, (char)old) == old;
	}
	
	inline bool 
	compare_and_swap_16(volatile uint16_t *a, uint16_t b, uint16_t old) {
	    return _InterlockedCompareExchange16((volatile short*)a, (short)b, (short)old) == old;
	}
	
	inline bool 
	compare_and_swap_32(volatile uint32_t *a, uint32_t b, uint32_t old) {
	    return _InterlockedCompareExchange((volatile long*)a, (long)b, (long)old) == old;
	}
	
	inline bool 
	compare_and_swap_64(volatile uint64_t *a, uint64_t b, uint64_t old) {
	    return _InterlockedCompareExchange64((volatile long long*)a, (long long)b, (long long)old) == old;
	}
	
	inline bool 
	compare_and_swap_bool(volatile bool *a, bool b, bool old) {
	    return compare_and_swap_8((uint8_t*)a, (uint8_t)b, (uint8_t)old);
	}
	
	#define MEMORY_BARRIER _ReadWriteBarrier()
	
	#define thread_local __declspec(thread)
	
	#define SHARED_EXPORT __declspec(dllexport)
    #define SHARED_IMPORT __declspec(dllimport)
	
#elif COMPILER_GCC || COMPILER_CLANG
	#define inline __attribute__((always_inline)) inline
	#define alignat(x) __attribute__((aligned(x)))	
    #define noreturn __attribute__((noreturn))
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    
    inline void noreturn
    crash() {
		__builtin_trap();
		volatile int *a = 0;
		*a = 5;
		a = (int*)0xDEADBEEF;
    	*a = 5;
	}
	
    inline u64 
    rdtsc() {
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
    
    inline 
    Cpu_Info_X86 cpuid(u32 function_id) {
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
	
	#define DEPRECATED(proc, msg) __attribute__((deprecated(msg))) proc 
	
	inline bool 
	compare_and_swap_8(volatile uint8_t *a, uint8_t b, uint8_t old) {
	    unsigned char result;
	    __asm__ __volatile__(
	        "lock; cmpxchgb %2, %1"
	        : "=a" (result), "=m" (*a)
	        : "r" (b), "m" (*a), "a" (old)
	        : "memory"
	    );
	    return result == old;
	}
	
	inline bool 
	compare_and_swap_16(volatile uint16_t *a, uint16_t b, uint16_t old) {
	    unsigned short result;
	    __asm__ __volatile__(
	        "lock; cmpxchgw %2, %1"
	        : "=a" (result), "=m" (*a)
	        : "r" (b), "m" (*a), "a" (old)
	        : "memory"
	    );
	    return result == old;
	}
	
	inline bool 
	compare_and_swap_32(volatile uint32_t *a, uint32_t b, uint32_t old) {
	    unsigned int result;
	    __asm__ __volatile__(
	        "lock; cmpxchgl %2, %1"
	        : "=a" (result), "=m" (*a)
	        : "r" (b), "m" (*a), "a" (old)
	        : "memory"
	    );
	    return result == old;
	}
	
	inline bool 
	compare_and_swap_64(volatile uint64_t *a, uint64_t b, uint64_t old) {
	    unsigned long long result;
	    __asm__ __volatile__(
	        "lock; cmpxchgq %2, %1"
	        : "=a" (result), "=m" (*a)
	        : "r" (b), "m" (*a), "a" (old)
	        : "memory"
	    );
	    return result == old;
	}
	
	inline bool 
	compare_and_swap_bool(volatile bool *a, bool b, bool old) {
	    return compare_and_swap_8((uint8_t*)a, (uint8_t)b, (uint8_t)old);
	}
	
	#define MEMORY_BARRIER {__asm__ __volatile__("" ::: "memory");__sync_synchronize();}
	
	#define thread_local __thread
	
#if TARGET_OS == WINDOWS
	#define SHARED_EXPORT __attribute__((visibility("default"))) __declspec(dllexport)
    #define SHARED_IMPORT __declspec(dllimport)
#else
	#define SHARED_EXPORT __attribute__((visibility("default")))
    #define SHARED_IMPORT 
#endif
	
#else
	#define inline inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 0
    
    inline u64 
    rdtsc() { return 0; }
    inline Cpu_Info_X86 cpuid(u32 function_id) {return (Cpu_Info_X86){0};}
    #define COMPILER_CAN_DO_SSE2 0
    #define COMPILER_CAN_DO_AVX 0
    #define COMPILER_CAN_DO_AVX2 0
    #define COMPILER_CAN_DO_AVX512 0
    
    #define DEPRECATED(proc, msg) 
    
    #define MEMORY_BARRIER
    
    #warning "Compiler is not explicitly supported, some things will probably not work as expected"
#endif



Cpu_Capabilities 
query_cpu_capabilities() {
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

