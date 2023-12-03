#pragma once
#ifndef PTSD_GENERAL_HEADER
#define PTSD_GENERAL_HEADER
#if defined(__EMSCRIPTEN__)
#   define PTSD_WASM (1)
#elif defined(_WIN32) || defined(_WIN64)
#    define PTSD_WINDOWS (1)
#elif defined(__linux__) || defined(__unix__)
#    define PTSD_LINUX (1)
#elif defined(__APPLE__)
#    define PTSD_MACOS (1)
#else
#   error platform not supported
#endif

#if _WIN32 || _WIN64
    #if _WIN64
        #define PTSD_64
    #else
        #define PTSD_32
    #endif
#elif __GNUC__
    #if __x86_64__ || __ppc64__
        #define PTSD_64
    #else
        #define PTSD_32
    #endif
#elif UINTPTR_MAX > UINT_MAX
    #define PTSD_64
#else
    #define PTSD_32
#endif

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#   define PTSD_GNU_COMPATIBLE 1
#else
#   define PTSD_MSVC 1
#endif
#if defined(__cplusplus) && !defined(PTSD_I_KNOW_WHAT_IM_DOING)
#   if defined(PTSD_MSVC)
#       pragma message pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#   else
#       warning pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#   endif
#endif

#define PTSD_C23 5
#define PTSD_C18 4
#define PTSD_C17 4
#define PTSD_C11 3
#define PTSD_C99 2
#define PTSD_C89 1

#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ == 199901
#      define PTSD_C_VERSION PTSD_C99
#   elif __STDC_VERSION__ == 201112
#      define PTSD_C_VERSION PTSD_C11
#   elif __STDC_VERSION__ == 201710
#      define PTSD_C_VERSION PTSD_C18
#   endif
#elif defined(__STDC__)
#      define PTSD_C_VERSION PTSD_C89
#endif

#if defined(__has_c_attribute)
#   define PTSD_HAS_ATTRIBUTE __has_c_attribute
#   if __STDC_VERSION__ >= 201710 
#       undef  PTSD_C_VERSION
#       define PTSD_C_VERSION PTSD_C23
#   endif 
#else
#   define PTSD_HAS_ATTRIBUTE(x) 0
#endif

#if defined(__STDC_NO_VLA__)
#   define PTSD_HAS_VLA 0
#elif PTSD_C_VERSION >= PTSD_C11 && !defined(PTSD_MSVC)
#   define PTSD_HAS_VLA 1
#else
#   define PTSD_HAS_VLA 0
#endif


#if PTSD_HAS_ATTRIBUTE(maybe_unused)
#define PTSD_UNUSED [[maybe_unused]]
#elif defined(PTSD_GNU_COMPATIBLE)
#define PTSD_UNUSED __attribute__((unused))
#else
#define PTSD_UNUSED
#endif

#define PTSD_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define PTSD_HAS_COMMA(...) PTSD_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define PTSD_TRIGGER_PARENTHESIS_(...) ,
#define PTSD_REMOVE_EMPTY(...) ,,,,,,,,,,,,,,,
#define PTSD_INDIRECT1(tuple) PTSD_ARG16 tuple 
#define PTSD_INDIRECT0(...)  PTSD_INDIRECT1((PTSD_REMOVE_EMPTY __VA_ARGS__ () 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
#define PTSD_INDIRECT(...) PTSD_INDIRECT0(__VA_ARGS__)

#define PTSD_NARG16__(args) PTSD_ARG16 args
#define PTSD_NARG16_(...) \
    PTSD_NARG16__((__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))
#define PTSD_NARG16(...) PTSD_NARG16_(__VA_ARGS__)

// based off of an implementation by Jens Gusted https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/ (may, 2022)
#define PTSD_ISEMPTY(...)                                                   \
PTSD_ISEMPTY_(                                                              \
          /* test if there is just one argument, eventually an empty        \
             one */                                                         \
          PTSD_HAS_COMMA(__VA_ARGS__),                                      \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument       \
             adds a comma */                                                \
          PTSD_HAS_COMMA(PTSD_TRIGGER_PARENTHESIS_ __VA_ARGS__),            \
          /* test if the argument together with a parenthesis               \
             adds a comma */                                                \
          PTSD_HAS_COMMA(__VA_ARGS__ (/*empty*/)),                          \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the       \
             parenthesis adds a comma */                                    \
          PTSD_HAS_COMMA(PTSD_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)),\
          PTSD_INDIRECT(__VA_ARGS__)                                        \
          )


#define PTSD_CONCAT_5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define PTSD_ISEMPTY_CONCAT(_0, _1, _2, _3, _4) PTSD_CONCAT_5(_0, _1, _2, _3, _4)
#define PTSD_ISEMPTY_TUPLE2(...)  PTSD_ISEMPTY_CONCAT __VA_ARGS__
#define PTSD_ISEMPTY_TUPLE1(a, b) PTSD_CONCAT(PTSD_IS_EMPTY_CASE_, b)
#define PTSD_ISEMPTY_TUPLE0(...)  PTSD_ISEMPTY_TUPLE1 __VA_ARGS__
#define PTSD_ISEMPTY_(...)  PTSD_ISEMPTY_TUPLE0((a, PTSD_ISEMPTY_TUPLE2 ((__VA_ARGS__))))
#define PTSD_IS_EMPTY_CASE_00001 1
#define PTSD_IS_EMPTY_CASE_00011 1
#define PTSD_IS_EMPTY_CASE_00011 1
#define PTSD_IS_EMPTY_CASE_00000 0
#define PTSD_IS_EMPTY_CASE_11110 0
#define PTSD_IS_EMPTY_CASE_11110 0
#define PTSD_IS_EMPTY_CASE_11110 0
#define PTSD_IS_EMPTY_CASE_11110 0


#define PTSD_CONCAT_( a, b ) a##b
#define PTSD_CONCAT( a, b ) PTSD_CONCAT_( a, b )

#define PTSD_STRINGIFY_(x) #x
#define PTSD_STRINGIFY(x) PTSD_STRINGIFY_(x)

#define PTSD_DEFAULT_1(actual, default) default
#define PTSD_DEFAULT_0(actual, default) actual

#define PTSD_DEFAULT_(...) PTSD_CONCAT __VA_ARGS__
#define PTSD_DEFAULT(value, default) PTSD_DEFAULT_((PTSD_DEFAULT_, PTSD_ISEMPTY(value)))(value, default)

#if PTSD_C_VERSION >= PTSD_C11
_Static_assert(PTSD_DEFAULT( , 5) == 5, "");
_Static_assert(PTSD_DEFAULT(2, 5) == 2, "");
#elif defined(PTSD_MSVC)
static_assert(PTSD_DEFAULT( , 5) == 5, "");
static_assert(PTSD_DEFAULT(2, 5) == 2, "");
#else
int static_assertPTSD_DEFAULT0[PTSD_DEFAULT( , 5) == 5 ? 1 : 0];
int static_assertPTSD_DEFAULT1[PTSD_DEFAULT(2, 5) == 2 ? 1 : 0];
#endif

#if defined(PTSD_MSVC)
#define PTSD_PRAGMA_MESSAGE_(msg) PTSD_STRINGIFY( message (__FILE__"("PTSD_STRINGIFY(__LINE__)")"PTSD_STRINGIFY(: Warning PTSD)": "msg))
#define PTSD_PRAGMA_MESSAGE(msg) _Pragma(PTSD_PRAGMA_MESSAGE_(msg))
#else
#define PTSD_PRAGMA_MESSAGE(msg) _Pragma(PTSD_STRINGIFY(message msg))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef countof
#define countof(x) (sizeof((x))/sizeof((x)[0]))
#endif

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef ptrdiff_t isize;
typedef ptrdiff_t ssize;

typedef size_t    usize;

typedef float     f32;
typedef double    f64;

#if PTSD_C_VERSION >= PTSD_C99
     typedef _Bool pbool_t;
     static const pbool_t pfalse = 0;
     static const pbool_t ptrue  = 1;
#    ifndef bool
#        define bool pbool_t
#    endif
#    ifndef false
#        define false pfalse
#    endif
#    ifndef true
#        define true ptrue
#    endif
#else
     enum pbool_t { pfalse, ptrue };
     typedef enum pbool_t pbool_t;
#    ifndef bool
#        define bool pbool_t
#    endif
#    ifndef false
#        define false pfalse
#    endif
#    ifndef true
#        define true ptrue
#    endif
#endif

#if defined(PTSD_MSVC) || (__clang__ && _WIN32)
#   define PTSD_FORCE_INLINE __forceinline
#else
#   define PTSD_FORCE_INLINE inline __attribute__((always_inline))
#endif

#if defined(PTSD_GNU_COMPATIBLE)
#   define PTSD_ASSUME(x) __builtin_assume((x))
#   define PTSD_EXPECT(cond, expected_result) __builtin_expect((cond), (expected_result))
#   if PTSD_C23
#       define PTSD_NONNULL [[gnu::nonnull]]
#   else
#       define PTSD_NONNULL __attribute__((nonnull))
#   endif
#else
#define PTSD_ASSUME(x) __assume((x))
#define PTSD_EXPECT(cond, expected_result) (cond)
#define register __register
#   if PTSD_C23
#       define PTSD_NONNULL [[non_null]]
#   else
#       define PTSD_NONNULL 
#   endif
#endif

#if defined(PTSD_GNU_COMPATIBLE)
#   ifndef pallocate
#      define pallocate(size) malloc(size)
#   endif // pAllocate
#   ifndef pzero_allocate
#      define pzero_allocate(size) ({ void *_tmp_ = malloc(size); memset(_tmp_, 0, (size));})
#   endif // pzeroallocate
#   ifndef preallocate
#      define preallocate(size, buffer) realloc(buffer, size)
#   endif // pReallocate
#   ifndef pfree
#      define pfree(buffer) free(buffer)
#   endif // pfree
#   ifndef psized_free
#      define psized_free(size, buffer) free(buffer)
#   endif // psizedfree
#else
#   ifndef pallocate
#      define pallocate(size) malloc(size)
#   endif // pAllocate
#   ifndef pzero_allocate
PTSD_UNUSED
static inline void *pzero_allocate_implementation(usize size) {
    void *tmp = malloc(size);
    memset(tmp, 0, size);
    return tmp;
}
#      define pzero_allocate(size) pzero_allocate_implementation(size)
#   endif // pZeroAllocate
#   ifndef pReallocate
#      define preallocate(size, buffer) realloc(buffer, size)
#   endif // pReallocate
#   ifndef pFree
#      define pfree(buffer) free(buffer)
#   endif // pFree
#   ifndef pSized_Free
#      define psized_free(size, buffer) free(buffer)
#   endif // pFree
#endif
#endif // PTSD_GENERAL_HEADER 
