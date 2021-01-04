#pragma once
#ifndef PSTD_GENERAL_HEADER
#define PSTD_GENERAL_HEADER

#include <stdint.h> 
#include <assert.h>
#include <wchar.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define countof(array) sizeof(array)/sizeof(array[0])
#define typeof __typeof
#define auto __auto_type
#define pointer_cast(ToType, ptr) ((ToType *)(void *)(ptr))

// maybe have this
// typedef const void *const readonly_ptr;

#if defined(_WIN32) || defined(_WIN64)
#define PSTD_WINDOWS
#elif defined(__linux__) || defined(__unix__)
#define PSTD_LINUX
#elif defined(__APPLE__)
#define PSTD_MACOS
#warning this has been written with support for linux and windows! It might not work on mac
#endif

#if defined(_MSC_FULL_VER) && !defined(__clang__)
#error MVSC COMPILER NOT SUPPORTED!
#endif


#if _WIN32 || _WIN64
#   if _WIN64
#       define PSTD_64 1
#       define PSTD_32 0
#   else
#       define PSTD_64 0
#       define PSTD_32 1
#   endif
#elif __GNUC__
#   if __x86_64__ || __ppc64__
#       define PSTD_64 1
#       define PSTD_32 0
#   else
#       define PSTD_64 0
#       define PSTD_32 1
#   endif
#elif UINTPTR_MAX > UINT_MAX
#   define PSTD_64 1
#   define PSTD_32 0
#else
#   define PSTD_64 0
#   define PSTD_32 1
#endif



#if defined(__cplusplus)
#	define PSTD_CPP 1
#	define PSTD_C   0
#else
#	define PSTD_C   1
#	define PSTD_CPP 0
#endif

#if PSTD_C 
typedef enum bool{ false, true } bool;

#if __STDC_VERSION__ == 202000L  // this will probably break
#define PSTD_MAYBE_UNUSED [[maybe_unused]]
#else
#define PSTD_MAYBE_UNUSED __attribute__((unused))
#endif

#define EXTERNAL extern
#else
#define EXTERNAL extern "C"
#define PSTD_MAYBE_UNUSED [[maybe_unused]]
#endif


typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float       f32;
typedef double      f64;

typedef ptrdiff_t isize;
typedef size_t    usize;

// compare number to range
PSTD_MAYBE_UNUSED
static bool pCmpRange(s32 num, s32 start, s32 end) {
    return num > start && num < end;
}

// compare number to range inclusive
PSTD_MAYBE_UNUSED
static bool pCmpRangeI(s32 num, s32 start, s32 end) {
    return (num >= start) && (num <= end);
}

#endif // PSTD_GENERAL_HEADER 

