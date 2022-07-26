#pragma once

#ifndef PSTD_GENERAL_HEADER
#define PSTD_GENERAL_HEADER
#if defined(__EMSCRIPTEN__)
#   define PSTD_WASM
#elif defined(_WIN32) || defined(_WIN64)
#    define PSTD_WINDOWS
#elif defined(__linux__) || defined(__unix__)
#    define PSTD_LINUX
#elif defined(__APPLE__)
#    define PSTD_MACOS
#else
#   error platform not supported
#endif

#if _WIN32 || _WIN64
    #if _WIN64
        #define PSTD_64
    #else
        #define PSTD_32
    #endif
#elif __GNUC__
    #if __x86_64__ || __ppc64__
        #define PSTD_64
    #else
        #define PSTD_32
    #endif
#elif UINTPTR_MAX > UINT_MAX
    #define PSTD_64
#else
    #define PSTD_32
#endif

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#   define PSTD_GNU_COMPATIBLE 1
#else
#   define PSTD_MSVC 1
#endif
#if defined(__cplusplus) && !defined(PSTD_I_KNOW_WHAT_IM_DOING)
#   if defined(PSTD_MSVC)
#       pragma message pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#   else
#       warning pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#   endif
#endif

#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ == 199901
#      define PSTD_C99 2
#      define PSTD_C89 1
#      define PSTD_C_VERSION PSTD_C99
#   elif __STDC_VERSION__ == 201112
#      define PSTD_C11 3
#      define PSTD_C99 2
#      define PSTD_C89 1
#      define PSTD_C_VERSION PSTD_C11
#   elif __STDC_VERSION__ == 201710
#      define PSTD_C18 4
#      define PSTD_C17 4
#      define PSTD_C11 3
#      define PSTD_C99 2
#      define PSTD_C89 1
#      define PSTD_C_VERSION PSTD_C18
#   endif
#elif defined(__STDC__)
#       define PSTD_C89 1
#      define PSTD_C_VERSION PSTD_C89
#endif

#if defined(__STDC_NO_VLA__)
#   define PSTD_HAS_VLA 0
#elif PSTD_C11 && !defined(PSTD_MSVC)
#   define PSTD_HAS_VLA 1
#else
#   define PSTD_HAS_VLA 0
#endif

#if defined(__has_c_attribute)
#   define PSTD_HAS_ATTRIBUTE __has_c_attribute
#   if __STDC_VERSION__ >= 201710 
#       define PSTD_C23 5
#       undef  PSTD_C_VERSION
#       define PSTD_C_VERSION PSTD_C23
#   endif 
#else
#   define PSTD_HAS_ATTRIBUTE(x) 0
#endif


#if PSTD_HAS_ATTRIBUTE(maybe_unused)
#define PSTD_UNUSED [[maybe_unused]]
#elif defined(PSTD_GNU_COMPATIBLE)
#define PSTD_UNUSED __attribute__((unused))
#else
#define PSTD_UNUSED
#endif

#define PSTD_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define PSTD_HAS_COMMA(...) PSTD_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define PSTD_TRIGGER_PARENTHESIS_(...) ,
#define PSTD_REMOVE_EMPTY(...) ,,,,,,,,,,,,,,,
#define PSTD_INDIRECT1(tuple) PSTD_ARG16 tuple 
#define PSTD_INDIRECT0(...)  PSTD_INDIRECT1((PSTD_REMOVE_EMPTY __VA_ARGS__ () 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
#define PSTD_INDIRECT(...) PSTD_INDIRECT0(__VA_ARGS__)

// based off of an implementation by Jens Gusted https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/ (may, 2022)
#define PSTD_ISEMPTY(...)                                                   \
PSTD_ISEMPTY_(                                                              \
          /* test if there is just one argument, eventually an empty        \
             one */                                                         \
          PSTD_HAS_COMMA(__VA_ARGS__),                                      \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument       \
             adds a comma */                                                \
          PSTD_HAS_COMMA(PSTD_TRIGGER_PARENTHESIS_ __VA_ARGS__),            \
          /* test if the argument together with a parenthesis               \
             adds a comma */                                                \
          PSTD_HAS_COMMA(__VA_ARGS__ (/*empty*/)),                          \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the       \
             parenthesis adds a comma */                                    \
          PSTD_HAS_COMMA(PSTD_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)),\
          PSTD_INDIRECT(__VA_ARGS__)                                        \
          )


#define PSTD_CONCAT_5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define PSTD_ISEMPTY_CONCAT(_0, _1, _2, _3, _4) PSTD_CONCAT_5(_0, _1, _2, _3, _4)
#define PSTD_ISEMPTY_TUPLE2(...)  PSTD_ISEMPTY_CONCAT __VA_ARGS__
#define PSTD_ISEMPTY_TUPLE1(a, b) PSTD_CONCAT(PSTD_IS_EMPTY_CASE_, b)
#define PSTD_ISEMPTY_TUPLE0(...)  PSTD_ISEMPTY_TUPLE1 __VA_ARGS__
#define PSTD_ISEMPTY_(...)  PSTD_ISEMPTY_TUPLE0((a, PSTD_ISEMPTY_TUPLE2 ((__VA_ARGS__))))
#define PSTD_IS_EMPTY_CASE_00001 1
#define PSTD_IS_EMPTY_CASE_00011 1
#define PSTD_IS_EMPTY_CASE_00011 1
#define PSTD_IS_EMPTY_CASE_00000 0
#define PSTD_IS_EMPTY_CASE_11110 0
#define PSTD_IS_EMPTY_CASE_11110 0
#define PSTD_IS_EMPTY_CASE_11110 0
#define PSTD_IS_EMPTY_CASE_11110 0


#define PSTD_CONCAT_( a, b ) a##b
#define PSTD_CONCAT( a, b ) PSTD_CONCAT_( a, b )

#define PSTD_STRINGIFY_(x) #x
#define PSTD_STRINGIFY(x) PSTD_STRINGIFY_(x)

#define PSTD_DEFAULT_1(actual, default) default
#define PSTD_DEFAULT_0(actual, default) actual

#define PSTD_DEFAULT_(...) PSTD_CONCAT __VA_ARGS__
#define PSTD_DEFAULT(value, default) PSTD_DEFAULT_((PSTD_DEFAULT_, PSTD_ISEMPTY(value)))(value, default)

_Static_assert(PSTD_DEFAULT(, 5)  == 5, "");
_Static_assert(PSTD_DEFAULT(2, 5) == 2, "");



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

#if !defined(__cplusplus)
#   if PSTD_C_VERSION >= PSTD_C99
        typedef _Bool pbool_t;
        static const pbool_t false = 0;
        static const pbool_t true  = 1;
#   else
        enum pbool_t { false, true };
        typedef enum pbool_t pbool_t;
#   endif
#else
    using p_bool = pbool_t;
#endif

#if defined(PSTD_MSVC) || (__clang__ && _WIN32)
#   define PSTD_FORCE_INLINE __forceinline
#else
#   define PSTD_FORCE_INLINE inline __attribute__((always_inline))
#endif

#if defined(PSTD_GNU_COMPATIBLE)
#   define PSTD_ASSUME(x) __builtin_assume((x))
#   define PSTD_EXPECT(cond, expected_result) __builtin_expect((cond), (expected_result))
#   if PSTD_C23
#       define PSTD_NONNULL [[gnu::nonnull]]
#   else
#       define PSTD_NONNULL __attribute__((nonnull))
#   endif
#else
#define PSTD_ASSUME(x) __assume((x))
#define PSTD_EXPECT(cond, expected_result) (cond)
#define register __register
#   if PSTD_C23
#       define PSTD_NONNULL [[non_null]]
#   else
#       define PSTD_NONNULL 
#   endif
#endif

#if defined(PSTD_GNU_COMPATIBLE)
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
PSTD_UNUSED
static inline void *pzero_allocate_implementation(usize size) {
    void *tmp = malloc(size);
    memset(tmp, 0, size);
    return tmp;
}
#      define pzero_allocate(size) pzero_allocate_implementation
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



#endif // PSTD_GENERAL_HEADER 

