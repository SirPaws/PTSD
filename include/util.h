#pragma once
#ifndef PSTD_UTIL_HEADER
#define PSTD_UTIL_HEADER

#ifndef PSTD_UTIL_STANDALONE
#include "general.h"
#else
#define PSTD_GENERAL_VER 1
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

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#define PSTD_GNU_COMPATIBLE
#else
#define PSTD_MSVC
#define __builtin_expect(a, b) (a)
#endif

#if defined(PSTD_MSVC) && (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL)
#pragma message("Warning: the traditional msvc preprocessor does not support 'complicated' macros use /Zc:preprocessor") 
#define PSTD_HAS_SECOND(...) 0
#else
#define PSTD_HAS_SECOND_TEST__(_0, _1, _2, ...) _2
#define PSTD_HAS_SECOND_TRIGGER(...) ,
#define PSTD_HAS_SECOND_TEST_(...) PSTD_HAS_SECOND_TEST__(__VA_ARGS__)
#define PSTD_HAS_SECOND_TEST(...)  PSTD_HAS_SECOND_TEST_(PSTD_HAS_SECOND_TRIGGER __VA_ARGS__ (), 0, 1, 0) 
#define PSTD_HAS_SECOND(a, ...)    PSTD_HAS_SECOND_TEST(__VA_ARGS__)
#endif

#define PSTD_CONCAT_( a, b ) a##b
#define PSTD_CONCAT( a, b ) PSTD_CONCAT_( a, b )

#define PSTD_STRINGIFY_(x) #x
#define PSTD_STRINGIFY(x) PSTD_STRINGIFY_(x)

#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ == 199901
#      define PSTD_C99 1
#      define PSTD_C89 1
#   elif __STDC_VERSION__ == 201112
#      define PSTD_C11 1
#      define PSTD_C99 1
#      define PSTD_C89 1
#   elif __STDC_VERSION__ == 201710
#      define PSTD_C18 1
#      define PSTD_C17 1
#      define PSTD_C11 1
#      define PSTD_C99 1
#      define PSTD_C89 1
#   endif
#elif defined(__STDC__)
#       define PSTD_C89 1
#endif

#if defined(__cplusplus) && !defined(PSTD_I_KNOW_WHAT_IM_DOING)
#if defined(PSTD_MSVC)
#pragma message pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#else
#   warning pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#endif
#endif

#if defined(__STDC_NO_VLA__)
#   define PSTD_HAS_VLA 0
#elif PSTD_C11 && !defined(PSTD_MSVC)
#   define PSTD_HAS_VLA 1
#endif


#if defined(__has_c_attribute)
#   define PSTD_HAS_ATTRIBUTE __has_c_attribute
#   if __STDC_VERSION__ >= 201710 
#       define PSTD_C23 1
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>

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

#ifndef countof
#define countof(x) ((sizeof(x))/(sizeof((x)[0])))
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
#if defined(PSTD_C99)
enum { false, true };
typedef _Bool pBool;
#else
typedef enum { false, true } pBool;
#endif
#else
enum pBool { pFalse, pTrue };
#endif

#ifndef pReallocateBuffer
#    define pReallocateBuffer realloc
#endif
#ifndef pAllocateBuffer
#   define pAllocateBuffer malloc
#endif
#ifndef pFreeBuffer
#   define pFreeBuffer free
#endif
#if defined(PSTD_GNU_COMPATIBLE)
#ifndef pZeroAllocateBuffer
#define pZeroAllocateBuffer(size) ({                \
    void *pZeroAllocateBuffer_tmp = malloc(size);   \
    memset(pZeroAllocateBuffer_tmp, 0, (size));     \
    pZeroAllocateBuffer_tmp;                        \
})
#endif
#else
#ifndef pZeroAllocateBuffer
    static void* pZeroAllocateBuffer(usize size) {
        void* pZeroAllocateBuffer_tmp = pAllocateBuffer(size);
        assert(pZeroAllocateBuffer_tmp);
        memset(pZeroAllocateBuffer_tmp, 0, (size));
        return pZeroAllocateBuffer_tmp;
    }
#define pZeroAllocateBuffer pZeroAllocateBuffer
#endif
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void *pGetPlatformHandle(void);

typedef s64 pTimePoint;

enum pClockType {
    PSTD_STEADY_CLOCK,
    PSTD_HIGH_RESOLUTION_CLOCK,
    PSTD_SYSTEM_CLOCK
};

pTimePoint pGetTick(enum pClockType);

#define pGetTimeNanoSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pGetTimeNanod, f32: pGetTimeNanof,\
        s8: pGetTimeNanolls,s16: pGetTimeNanolls,s32: pGetTimeNanolls, s64: pGetTimeNanolls,\
        u8: pGetTimeNanollu, u16: pGetTimeNanollu, u32: pGetTimeNanollu, u64: pGetTimeNanollu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

#define pGetTimeMiliSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pGetTimeMilid, f32: pGetTimeMilif,\
        s8: pGetTimeMilills, s16: pGetTimeMilills, s32: pGetTimeMilills, s64: pGetTimeMilills,\
        u8: pGetTimeMilillu, u16: pGetTimeMilillu, u32: pGetTimeMilillu, u64: pGetTimeMilillu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

#define pGetTimeSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f128: pGetTimeSecondsld, f64: pGetTimeSecondsd, f32: pGetTimeSecondsf,\
        s8: pGetTimeSecondslls,s16: pGetTimeSecondslls,s32: pGetTimeSecondslls, s64: pGetTimeSecondslls,\
        u8: pGetTimeSecondsllu, u16: pGetTimeSecondsllu, u32: pGetTimeSecondsllu, u64: pGetTimeSecondsllu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

#define PSTD_TIME_MILI    1000000
#define PSTD_TIME_SECONDS 1000000000

PSTD_UNUSED
static inline pTimePoint pGetTimePointFromTimePoint(pTimePoint point) { return point; }

PSTD_UNUSED
static inline f64 pGetTimeNanod(pTimePoint timepoint)   { return (f64) timepoint; }
PSTD_UNUSED
static inline f32 pGetTimeNanof(pTimePoint timepoint)   { return (f32) timepoint; }
PSTD_UNUSED
static inline s64 pGetTimeNanolls(pTimePoint timepoint) { return (s64) timepoint; }
PSTD_UNUSED
static inline u64 pGetTimeNanollu(pTimePoint timepoint) { return (u64) timepoint; }

PSTD_UNUSED
static inline f64 pGetTimeMilid(pTimePoint timepoint)   { return (f64)timepoint / (f64) PSTD_TIME_MILI; }
PSTD_UNUSED
static inline f32 pGetTimeMilif(pTimePoint timepoint)   { return (f32)timepoint / (f32) PSTD_TIME_MILI; }
PSTD_UNUSED
static inline s64 pGetTimeMilills(pTimePoint timepoint) { return (s64)timepoint / (s64) PSTD_TIME_MILI; }
PSTD_UNUSED
static inline u64 pGetTimeMilillu(pTimePoint timepoint) { return (u64)timepoint / (u64) PSTD_TIME_MILI; }

PSTD_UNUSED
static inline f64 pGetTimeSecondsd(pTimePoint timepoint)   { return (f64)timepoint  / (f64) PSTD_TIME_SECONDS; }
PSTD_UNUSED
static inline f32 pGetTimeSecondsf(pTimePoint timepoint)   { return (f32)timepoint  / (f32) PSTD_TIME_SECONDS; }
PSTD_UNUSED
static inline s64 pGetTimeSecondslls(pTimePoint timepoint) { return (s64)timepoint  / (s64) PSTD_TIME_SECONDS; }
PSTD_UNUSED
static inline u64 pGetTimeSecondsllu(pTimePoint timepoint) { return (u64)timepoint  / (u64) PSTD_TIME_SECONDS; }

// a version of memset but for bigger data types see tests for more info
void *pMemExpand(void *dst, void *src, usize dstsize, usize srcsize);

#if defined(__cplusplus)
}
#endif

#endif // PLANG_UTIL_HEADER


