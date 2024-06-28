
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

#ifdef _MSC_VER
	#define os_debug_break __debugbreak
#else
	#error "Only msvc compiler supported at the moment";
#endif
	
#define assert(cond, ...) if (!(cond)) { printf("Assertion failed for condition: " #cond ". Message: " __VA_ARGS__); os_debug_break(); }

#define cast(t) (t)

///
// inline
// (Credit to chatgpt, so it might not be 100% correct)
#ifdef _MSC_VER
    // Microsoft Visual C++
    #define inline __forceinline
#elif defined(__GNUC__) || defined(__GNUG__)
    // GNU GCC/G++
    #define inline __attribute__((always_inline)) inline
#elif defined(__clang__)
    // Clang/LLVM
    #define inline __attribute__((always_inline)) inline
#elif defined(__INTEL_COMPILER) || defined(__ICC)
    // Intel C++ Compiler
    #define inline __forceinline
#elif defined(__BORLANDC__)
    // Borland C++
    #define inline __inline
#elif defined(__MINGW32__) || defined(__MINGW64__)
    // MinGW (Minimalist GNU for Windows)
    #define inline __attribute__((always_inline)) inline
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    // Oracle Solaris Studio
    #define inline inline __attribute__((always_inline))
#elif defined(__IBMC__) || defined(__IBMCPP__)
    // IBM XL C/C++ Compiler
    #define inline __attribute__((always_inline)) inline
#elif defined(__PGI)
    // Portland Group Compiler
    #define inline inline __attribute__((always_inline))
#else
    // Fallback for unknown compilers
    #define inline inline
#endif

typedef struct Nothing {int nothing;} Nothing;

#ifndef CONTEXT_EXTRA
	#define CONTEXT_EXTRA Nothing
#endif

typedef enum Allocator_Message {
	ALLOCATOR_ALLOCATE,
	ALLOCATOR_DEALLOCATE,
} Allocator_Message;
typedef void*(*Allocator_Proc)(u64, void*, Allocator_Message);

typedef struct Allocator {
	Allocator_Proc proc;
	void *data;	
} Allocator;

typedef struct Context {
	Allocator allocator;
	
	CONTEXT_EXTRA extra;
} Context;


#define CONTEXT_STACK_MAX 512
thread_local Context context;
thread_local Context context_stack[CONTEXT_STACK_MAX];
thread_local u64 num_contexts = 0;


void* alloc(u64 size) { return context.allocator.proc(size, NULL, ALLOCATOR_ALLOCATE); }
void dealloc(void *p) { context.allocator.proc(0, p, ALLOCATOR_DEALLOCATE); }



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

void push_allocator(Allocator a) {
	Context c = context;
	c.allocator = a;
	push_context(c);
}
void pop_allocator() { pop_context(); }
