#pragma once
#ifndef PSTD_DYNARRAY_HEADER
#ifndef DYNARRAY_STANDALONE
#   include "general.h"
#else
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

#if defined(_MSC_FULL_VER) && !defined(__clang__)
#error MVSC COMPILER NOT SUPPORTED!
#endif



#ifndef P_DYNARRAY_GROWTH_COUNT
#define P_DYNARRAY_GROWTH_COUNT 2
#endif

#define pCreateDynArray(name, datatype) \
    struct name {                       \
        usize endofstorage;             \
        usize size;                     \
        __typeof(datatype) *data;       \
    }
#define pCreateStaticDynArray(type, value) (type){ sizeof(value), countof(value), value }

#define pSize(array)   ((array)->size) 
#define pLength(array) ((array)->size)
#define pLen(array)    ((array)->size)
#define pSizeof(value) (sizeof(__typeof(value)))

#define pFreeDynArray(array) ({             \
        pFreeBuffer((array)->data);         \
        memset((array), 0, sizeof *(array));\
    })

#define pSetCapacity(array, count) ({                                               \
    if (!(array)->data) {                                                           \
        __typeof(*(array)) pSetCapacity_array = {                                   \
            .endofstorage = pSizeof((array)->data[0]) * (count),                    \
            .data         = pZeroAllocateBuffer(pSizeof((array)->data[0]) * (count))\
        };                                                                          \
        *(array) = pSetCapacity_array;                                              \
    } else {                                                                        \
        void *pSetCapacity_tmp = pReallocateBuffer((array)->data,                   \
                (pSizeof((array)->data[0]) * (count)));                             \
        (array)->data = pSetCapacity_tmp;                                           \
    }                                                                               \
    (array);                                                                        \
})
#define pSetCap     pSetCapacity
#define pSetCount   pSetCapacity
#define pSetSize    pSetCapacity
#define pSetLength  pSetCapacity

#define pPushBack(array, value) ({                                      \
    pMaybeGrowDynArray((DynArray *)(array), sizeof(__typeof(value)));   \
    __auto_type pPushBack_ret = (array)->data + (array)->size++;        \
    *pPushBack_ret = (value);                                           \
    pPushBack_ret;                                                      \
})

#define pPushBytes(array, value, bytes) ({                      \
    pMaybeByteGrowDynArray((DynArray *)(array), (bytes));       \
    memcpy((array)->data + (array)->size, (value), (bytes));    \
    __auto_type pPushBack_ret = (array)->data + ((array)->size);\
    (array)->size += (bytes);                                   \
    pPushBack_ret;                                              \
})

#define pBegin(array) ({ (array)->data; })
#define pEnd(array) ({ (array)->data + (array)->size; })

#define pInsert(array, position, value) ({                                                      \
    usize pInsert_size = sizeof(__typeof(value));                                               \
    usize pInsert_offset = (position) - pBegin(array);                                          \
    __typeof(value) *result = NULL;                                                             \
    if (pInsert_offset >= (array)->size) {}                                                     \
    else {                                                                                      \
        pMaybeGrowDynArray((DynArray *)(array), pInsert_size);                                  \
        /* first we extract all elements after the place where we want                        */\
        /* to insert and then we shift them one element forward                               */\
        /* here is an example we wan't to insert 6 at the place pointed to below              */\
        /* [1, 2, 3, 4]                                                                       */\
        /*     ^                                                                              */\
        /* we make a new array that holds [2, 3, 4]                                           */\
        /* we insert that into the array                                                      */\
        /* [1, 2, 2, 3, 4]                                                                    */\
        /* then we insert the value                                                           */\
        /* [1, 6, 2, 3, 4]                                                                    */\
        usize pInsert_elems = (array)->size - pInsert_offset;                                   \
        void *pInsert_tmp = pAllocateBuffer(pInsert_size * pInsert_elems);                      \
        memcpy(pInsert_tmp, (array)->data + pInsert_offset, pInsert_elems * pInsert_size);      \
        memcpy((array)->data + pInsert_offset + 1, pInsert_tmp, pInsert_elems * pInsert_size);  \
        pFreeBuffer(pInsert_tmp);                                                               \
                                                                                                \
        (array)->size++;                                                                        \
        (array)->data[pInsert_offset] = value;                                                  \
        result = (array)->data + pInsert_offset;                                                \
    }                                                                                           \
    result;                                                                                     \
})

#define pMakeHole(array, position, num_bytes) ({                                                \
    usize pMakeHole_size = sizeof(__typeof((array)->data[0]));                                  \
    usize pMakeHole_offset = (position) - pBegin(array);                                        \
    __typeof((array)->data[0]) *pMakeHole_result = NULL;                                        \
    if (pMakeHole_offset >= (array)->size) {}                                                   \
    else {                                                                                      \
        pMaybeByteGrowDynArray((DynArray *)(array), num_bytes);                                 \
        /* first we extract all elements after the place where we want                        */\
        /* to insert and then we shift them one element forward                               */\
        /* here is an example we wan't to insert 6 at the place pointed to below              */\
        /* [1, 2, 3, 4]                                                                       */\
        /*     ^                                                                              */\
        /* we make a new array that holds [2, 3, 4]                                           */\
        /* we insert that into the array                                                      */\
        /* [1, 2, 2, 3, 4]                                                                    */\
        /* then we insert the value                                                           */\
        /* [1, 6, 2, 3, 4]                                                                    */\
        usize pMakeHole_elems = (array)->size - pMakeHole_offset;                               \
        void *pMakeHole_tmp = pAllocateBuffer(pMakeHole_size * pMakeHole_elems);                \
        memcpy(pMakeHole_tmp, (array)->data + pMakeHole_offset,                                 \
                pMakeHole_elems * pMakeHole_size);                                              \
        memcpy((array)->data + pMakeHole_offset + (num_bytes), pMakeHole_tmp,                   \
                pMakeHole_elems * pMakeHole_size);                                              \
        pFreeBuffer(pMakeHole_tmp);                                                             \
                                                                                                \
        pMakeHole_result = (array)->data + pMakeHole_offset;                                    \
    }                                                                                           \
    pMakeHole_result;                                                                           \
})


#define pPopBack(array) ({                                      \
    __typeof((array)->data[0]) pPopBack_result = {0};           \
    if ((array)->size == 0) { }                                 \
    else {                                                      \
        pPopBack_result = (array)->data[((array)->size--) - 1]; \
    }                                                           \
    pPopBack_result;                                            \
})

#define pRemove(array, position) ({                                                     \
    __typeof((array)->data[0]) pRemove_result = {0};                                    \
    usize pRemove_offset = (position) - pBegin(array);                                  \
    if (pRemove_offset >= (array)->size) {}                                             \
    else if (pRemove_offset == (array)->size - 1) {                                     \
        (array)->size--;                                                                \
        pRemove_result = (array)->data[pRemove_offset];                                 \
    } else {                                                                            \
        pRemove_result = (array)->data[pRemove_offset];                                 \
        usize pRemove_elems = (array)->size - pRemove_offset;                           \
        void *pRemove_tmp = pAllocateBuffer(pSizeof((array)->data[0]) * pRemove_elems); \
        memcpy(pRemove_tmp, (array)->data + pRemove_offset + 1,                         \
                pRemove_elems * pSizeof((array)->data[0]));                             \
        memcpy((array)->data + pRemove_offset, pRemove_tmp,                             \
                pRemove_elems * pSizeof((array)->data[0]));                             \
        pFreeBuffer(pRemove_tmp);                                                       \
        (array)->size--;                                                                \
    }                                                                                   \
    pRemove_result;                                                                     \
})

// arr:  another dynamic array
#define pCopyDynArray(arr) ({                                                           \
        __typeof(arr) pCopyDynArray_tmp = { 0 };                                        \
        pCopyDynArray_tmp.data = pAllocateBuffer( (arr).size );                         \
        memcpy(pCopyDynArray_tmp.data, (arr).data, sizeof(*(arr).data) * (arr).size);   \
        pCopyDynArray_tmp.size = pCopyDynArray_tmp.endofstorage = (arr).size;           \
        pCopyDynArray_tmp;})

// type: the type of array we want
// arr:  a static array
#define pCopyArray(type, arr) ({                                \
        type pCopyArray_tmp = { 0 };                            \
        pCopyArray_tmp.data = pAllocateBuffer( sizeof(arr) );   \
        memcpy(pCopyArray_tmp.data, (arr), sizeof(arr));        \
        pCopyArray_tmp.size = countof(arr);                     \
        pCopyArray_tmp.endofstorage = sizeof(arr);              \
        pCopyArray_tmp;                                         \
})

typedef struct DynArray DynArray;
struct DynArray {
    usize endofstorage;
    usize size;
    void *data;
};

// how many elements we should add
static void pDynArrayByteGrow(DynArray *array, usize bytes);
static void pDynArrayGrow(DynArray *array, usize datasize, usize count);

PSTD_UNUSED
static void pMaybeByteGrowDynArray(DynArray *array, usize bytes) {
    if (array->size + bytes > array->endofstorage) {             \
        pDynArrayByteGrow(array, bytes);                         \
    }
}

PSTD_UNUSED
static void pMaybeGrowDynArray(DynArray *array, usize datasize) {
    if ((array->size + 1) * datasize > array->endofstorage) {   \
        pDynArrayGrow((DynArray *)array,                        \
                datasize, P_DYNARRAY_GROWTH_COUNT);             \
    }
}

PSTD_UNUSED
static void pDynArrayByteGrow(DynArray *array, usize bytes) {
    if (!bytes || !array) return;
    void *tmp = pReallocateBuffer(array->data, array->endofstorage + bytes);
    // assert(tmp);
    array->data = tmp;
    array->endofstorage += bytes;
}

PSTD_UNUSED
static void pDynArrayGrow(DynArray *array, usize datasize, usize count) {
    if (!count || !array || !datasize) return;
    void *tmp = pReallocateBuffer(array->data, datasize * (array->endofstorage + count));
    // assert(tmp);
    array->data = tmp;
    array->endofstorage += count;
}

PSTD_UNUSED
static void pDynArrayFree(DynArray *array) {
    pFreeBuffer(array->data);
    memset(array, 0, sizeof *array);
}
#endif // PSTD_DYNARRAY_HEADER
