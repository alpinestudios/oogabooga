
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;
typedef f32 float32;
typedef f64 float64;

typedef u8 bool;
#define false 0
#define true 1

#define thread_local _Thread_local

#define local_persist static

#define forward_global extern

// Haters gonna hate
#define If if (
#define then )
// If cond then {}

#ifdef _MSC_VER
	inline void os_break() {
		__debugbreak();
		volatile int *a = 0;
		*a = 5;
	}
#else
	#error "Only msvc compiler supported at the moment";
#endif
	
	
void printf(const char* fmt, ...);
#define ASSERT_STR_HELPER(x) #x
#define ASSERT_STR(x) ASSERT_STR_HELPER(x)
#define assert_line(line, cond, ...) if(!(cond)) { printf("Assertion failed in file " __FILE__ " on line " ASSERT_STR(line) "\nFailed Condition: " #cond ". Message: " __VA_ARGS__); os_break(); }
#define assert(cond, ...) assert_line(__LINE__, cond, __VA_ARGS__);

#define DEFER(start, end) for(int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))

#if CONFIGURATION == RELEASE
#undef assert
#define assert(...)
#endif

#define panic(...) { print(__VA_ARGS__); os_break(); }

#define cast(t) (t)

#define ZERO(t) (t){0}

///
// Compiler specific stuff
// We make inline actually inline.
#ifdef _MSC_VER
    // Microsoft Visual C++
    #define inline __forceinline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    #include <intrin.h>
    #pragma intrinsic(__rdtsc)
    inline u64 rdtsc() {
        return __rdtsc();
    }
#elif defined(__GNUC__) || defined(__GNUG__)
    // GNU GCC/G++
    #define inline __attribute__((always_inline)) inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline u64 rdtsc() {
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
#elif defined(__clang__)
    // Clang/LLVM
    #define inline __attribute__((always_inline)) inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline u64 rdtsc() {
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
#elif defined(__INTEL_COMPILER) || defined(__ICC)
    // Intel C++ Compiler
    #define inline __forceinline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline u64 rdtsc() {
        return __rdtsc();
    }
#elif defined(__BORLANDC__)
    // Borland C++
    #define inline __inline
    inline u64 rdtsc() {
        unsigned int lo, hi;
        __asm {
            rdtsc
            mov lo, eax
            mov hi, edx
        }
        return ((u64)hi << 32) | lo;
    }
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    // Oracle Solaris Studio
    #define inline inline __attribute__((always_inline))
    inline u64 rdtsc() {
        unsigned int lo, hi;
        asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
#elif defined(__IBMC__) || defined(__IBMCPP__)
    // IBM XL C/C++ Compiler
    #define inline __attribute__((always_inline)) inline
    #define COMPILER_HAS_MEMCPY_INTRINSICS 1
    inline u64 rdtsc() {
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
#elif defined(__PGI)
    // Portland Group Compiler
    #define inline inline __attribute__((always_inline))
    inline u64 rdtsc() {
        unsigned int lo, hi;
        asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
        return ((u64)hi << 32) | lo;
    }
#else
    // Fallback for unknown compilers
    #define inline inline
#endif


#define FIRST_ARG(arg1, ...) arg1
#define SECOND_ARG(arg1, arg2, ...) arg2
#define print(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  prints, \
                           default: printf \
                          )(__VA_ARGS__)
#define sprint(...) _Generic((SECOND_ARG(__VA_ARGS__)), \
                           string:  sprints, \
                           default: sprintf \
                          )(__VA_ARGS__)
#define tprint(...) _Generic((FIRST_ARG(__VA_ARGS__)), \
                           string:  tprints, \
                           default: tprintf \
                          )(__VA_ARGS__)

typedef struct Nothing {int nothing;} Nothing;

#ifndef CONTEXT_EXTRA
	#define CONTEXT_EXTRA Nothing
#endif

typedef enum Allocator_Message {
	ALLOCATOR_ALLOCATE,
	ALLOCATOR_DEALLOCATE,
	ALLOCATOR_REALLOCATE,
} Allocator_Message;
typedef void*(*Allocator_Proc)(u64, void*, Allocator_Message, void*);

typedef enum Log_Level {
	LOG_ERROR,
	LOG_INFO,
	LOG_WARNING,
	LOG_VERBOSE
} Log_Level;



typedef struct Allocator {
	Allocator_Proc proc;
	void *data;	
} Allocator;

typedef struct Context {
	void *logger; // void(*Logger_Proc)(Log_Level level, string fmt, ...)
	
	CONTEXT_EXTRA extra;
} Context;


#define CONTEXT_STACK_MAX 512
thread_local Context context;
thread_local Context context_stack[CONTEXT_STACK_MAX];
thread_local u64 num_contexts = 0;

forward_global thread_local Allocator temp;

void* memset(void* dest, int value, size_t amount);
void* alloc(Allocator allocator, u64 size) {
	void *p = allocator.proc(size, 0, ALLOCATOR_ALLOCATE, allocator.data);
#if DO_ZERO_INITIALIZATION
	memset(p, 0, size);
#endif
	return p;
}
void* alloc_uninitialized(Allocator allocator, u64 size) {
	return allocator.proc(size, 0, ALLOCATOR_ALLOCATE, allocator.data);	
}
void dealloc(Allocator allocator, void *p) {
	allocator.proc(0, p, ALLOCATOR_DEALLOCATE, allocator.data);
}

void push_context(Context c) {
	assert(num_contexts < CONTEXT_STACK_MAX, "Context stack overflow");
	
	context_stack[num_contexts] = context;
	context = c;
	num_contexts += 1;
}
void pop_context() {
	assert(num_contexts > 0, "No contexts to pop!");
	num_contexts -= 1;
	context = context_stack[num_contexts];
}


