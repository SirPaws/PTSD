#pragma once
#ifndef PSTD_SMART_MEMORY_HEADER

#ifndef SMART_MEMORY_STANDALONE
#   include "general.h"
#else
#define PSTD_GENERAL_VER 1
#if defined(_WIN32) || defined(_WIN64)
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

#if !defined(PSTD_GNU_COMPATIBLE)
#error pstd smart memory is not supported in MSVC!
#endif

typedef void pFreeFunction(void*);

#define smart __attribute__((cleanup(pCleanupFunc)))

// if `dtor` is *NULL* no function is called

void *pMove(void**); 
#define pMove(ptr) pMove((void*)&(ptr))
void *pRef(void**);
#define pRef(ptr) pRef((void*)&(ptr))
void *pCopy(void **);
#define pCopy(ptr) pCopy((void*)&(ptr))

void *pSmartAllocateBuffer(usize size, pFreeFunction *dtor);
void *pSmartZeroAllocateBuffer(usize size, pFreeFunction *dtor);

void *pSmartReallocateBuffer(void *, usize size);

void pSmartFreeBuffer(void *);

PSTD_UNUSED
static void pCleanupFunc(void *mem) {
    pSmartFreeBuffer(*(void**)mem);
}

#define SMART_MEMORY_IMPLEMENTATION
#if defined(SMART_MEMORY_IMPLEMENTATION)

typedef struct pSmartMemoryHeader pSmartMemoryHeader;
struct pSmartMemoryHeader {
    usize use_count;
    pFreeFunction *dtor;
    usize size;
};

void *pSmartAllocateBuffer(usize size, pFreeFunction *dtor) {
    pSmartMemoryHeader *sptr = pAllocateBuffer(sizeof(pSmartMemoryHeader) + size);
    sptr->use_count = 0;
    sptr->dtor      = dtor;
    sptr->size      = size;
    return (void*)(sptr + 1);
}

void *pSmartZeroAllocateBuffer(usize size, pFreeFunction *dtor) {
    pSmartMemoryHeader *sptr = pZeroAllocateBuffer(sizeof(pSmartMemoryHeader) + size);
    sptr->dtor      = dtor;
    sptr->size      = size;
    return sptr + 1;
}

void *pSmartReallocateBuffer(void *mem, usize size) {
    if (!mem) return NULL;
    pSmartMemoryHeader *sptr = ((pSmartMemoryHeader *)mem) - 1;
    if (sptr->use_count != 0) return NULL;
    void *new_mem = pReallocateBuffer(sptr, sizeof(pSmartMemoryHeader) + size);
    if (!new_mem) return NULL;
    sptr = new_mem;
    sptr->size      = size;
    return sptr + 1;
}

void pSmartFreeBuffer(void *mem) {
    if (!mem) return;
    pSmartMemoryHeader *sptr = ((pSmartMemoryHeader *)mem) - 1;
    if (sptr->use_count != 0) return (void)--sptr->use_count;
    if (sptr->dtor) sptr->dtor(mem);
    pFreeBuffer(sptr);
}

#pragma push_macro("pMove")
#pragma push_macro("pRef")
#pragma push_macro("pCopy")
#define pMove pMove
#define pRef pRef
#define pCopy pCopy

void *pMove(void** mem_ptr) {
    if (!*mem_ptr) return NULL;
    void *result = *mem_ptr;
    *mem_ptr = NULL;
    return result;
}

void *pRef(void **mem_ptr) {
    if (!*mem_ptr) return NULL;
    pSmartMemoryHeader *sptr = ((pSmartMemoryHeader *)*mem_ptr) - 1;
    sptr->use_count++;
    return *mem_ptr;
}

void *pCopy(void **mem_ptr) {
    if (!*mem_ptr) return NULL;
    pSmartMemoryHeader *sptr = ((pSmartMemoryHeader *)*mem_ptr) - 1;
    void *copy = pSmartAllocateBuffer(sptr->size, sptr->dtor);
    return copy;
}

#pragma pop_macro("pCopy")
#pragma pop_macro("pRef")
#pragma pop_macro("pMove")

#endif
#endif // PSTD_STRETCHY_BUFFER_HEADER
