#pragma once

#ifndef PSTD_STRETCHY_BUFFER_HEADER
#define PSTD_STRETCHY_BUFFER_HEADER
#ifndef STRETCHY_BUFFER_STANDALONE
#   include "general.h"
#else
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

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#define PSTD_GNU_COMPATIBLE
#else
#define PSTD_MSVC
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
#else
#   define PSTD_HAS_VLA 0
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
#   if defined(PSTD_C99)
        enum { false, true };
        typedef _Bool pBool;
#   else
        enum pBool { false, true };
        typedef enum pBool pBool;
#   endif
#else
    using pBool = bool;
#endif

#if defined(PSTD_GNU_COMPATIBLE)
#define PSTD_ASSUME(x) __builtin_assume((x))
#define PSTD_EXPECT(cond, expected_result) __builtin_expect((cond), (expected_result))
#else
#define PSTD_ASSUME(x) __assume((x))
#define PSTD_EXPECT(cond, expected_result) (cond)
#endif

#if defined(PSTD_GNU_COMPATIBLE)
#   ifndef pAllocate
#      define pAllocate(size) malloc(size)
#   endif // pAllocate
#   ifndef pZeroAllocate
#      define pZeroAllocate(size) ({ void *_tmp_ = malloc(size); memset(_tmp_, 0, (size));})
#   endif // pZeroAllocate
#   ifndef pReallocate
#      define pReallocate(size, buffer) realloc(buffer, size)
#   endif // pReallocate
#   ifndef pFree
#      define pFree(buffer) free(buffer)
#   endif // pFree
#   ifndef pSizedFree
#      define pSizedFree(size, buffer) free(buffer)
#   endif // pFree
#else
#   ifndef pAllocate
#      define pAllocate(size) malloc(size)
#   endif // pAllocate
#   ifndef pZeroAllocate
PSTD_UNUSED
static inline void *pZeroAllocateImplementation(usize size) {
    void *tmp = malloc(size);
    memset(tmp, 0, size);
    return tmp;
}
#      define pZeroAllocate(size) pZeroAllocateImplementation
#   endif // pZeroAllocate
#   ifndef pReallocate
#      define pReallocate(size, buffer) realloc(buffer, size)
#   endif // pReallocate
#   ifndef pFree
#      define pFree(buffer) free(buffer)
#   endif // pFree
#   ifndef pSizedFree
#      define pSizedFree(size, buffer) free(buffer)
#   endif // pFree
#endif


#endif // PSTD_GENERAL_HEADER 
#endif

#ifndef PSTD_STRETCHY_BUFFER_GROWTH_COUNT
#define PSTD_STRETCHY_BUFFER_GROWTH_COUNT 2
#endif

// symbol to make it clear that a datatype is a stretchy buffer
// ie.  `int *stretchy data;
#define stretchy

// sets a function that will be called on each element
// when the element is removed, equivalent to a destrutor in c++
#define pSetFreeFunc(array, free_func) \
    pSetFreeFuncImplementation(array, free_func)

#if defined(PSTD_USE_ALLOCATOR)
// creates a new array with a given allocator
#define pCreateStretchyBuffer(data_type, allocator) \
    pCreateStretchyBufferImplementation(data_type, allocator)
#endif

// sets the capacity of the current buffer to hold exactly 'count' elements
#define pResize(array, count)      pSetCapacityImplementation(array, count)
#define pSetCap(array, count)      pSetCapacityImplementation(array, count)
#define pSetCount(array, count)    pSetCapacityImplementation(array, count)
#define pSetSize(array, count)     pSetCapacityImplementation(array, count)
#define pSetLength(array, count)   pSetCapacityImplementation(array, count)
#define pSetCapacity(array, count) pSetCapacityImplementation(array, count)

// sets the capacity of the current buffer to hold at least 'count' elements
// if the current size of the buffer is greater than count this function does nothing
#define pReserve(array, count)  pReserveImplementation(array, count)

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
#define pPushBack(array, value) pPushBackImplementation(array, value)

// same as pPushBack but gives control over how many bytes needs to be inserted into the array
#define pPushBytes(array, value, num_bytes) pPushBytesImplementation(array, value, num_bytes)

// removes last element in array and returns a copy 
#define pPopBack(array) pPopBackImplementation(array)
 
// inserts 'value' at 'position' in 'array' and returns a pointer to 'position' in the array
// note that this might invalidate the 'position' parameter so if you need to reuse
// 'position' you should call it thously
// location = pInsert(array, location, 10)
#define pInsert(array, position, value) pInsertImplementation(array, position, value)

// removes element at 'position' in 'array' and returns the data that was at 'position'
#define pRemove(array, position) pRemoveImplementation(array, position)

// returns a pointer to the first element of the array
#define pBegin(array) pBeginImplementation(array)
// returns a pointer to the element after the last element in the array
#define pEnd(array)   pEndImplementation(array)

// returns a pointer to the last element of the array
#define pBeginR(array) _Static_assert(false, "pBeginR not implemented!"), pBeginImplementation(array)
// returns a pointer to the element before the first element in the array
#define pEndR(array)   _Static_assert(false, "pEndR not implemented!"), pEndImplementation(array)

// get the number of elements currently stored in the array
#define pSize(array)   pSizeImplementation(array)
#define pLength(array) pSizeImplementation(array) 
#define pCount(array)  pSizeImplementation(array) 

// creates a copy of a strechy buffer and returns it
#define pCopyStretchyBuffer(array) pCopyStretchyBufferImplementation(array)

// creates a copy of a static array as a strechy buffer and returns it
#define pCopyArray(type, array)   pCopyArrayImplementation(type, array)

// frees strechy buffer
#define pFreeStretchyBuffer(array) pFreeStretchyBufferImplementation(array)

// gets meta data for the array
#define pGetMeta(array)            pGetMetaImplementation(array)

// gets meta data for the array. if the array is equal to NULL
// then it creates a new strechy buffer and assigns array to that
#define pGetMetaOrCreate(array) pGetMetaOrCreateImplementation(array)

// utility function that removes some warning in GNU compatible compilers.
// in MSVC this is equivalent to sizeof(value).
// in GNU compatible this is equivalent to sizeof(__typeof(value))
#define pSizeof(value)          pSizeofImplementation(value)



























#if defined(PSTD_USE_ALLOCATOR) && !defined(PSTD_ALLOCATOR_DEFINED)
#define PSTD_ALLOCATOR_DEFINED
enum AllocationKind {
    ALLOCATE,
    ZERO_ALLOCATE,
    REALLOCATE,
    FREE,
    SIZED_FREE,
};
typedef enum AllocationKind AllocationKind;
typedef struct Allocator Allocator;
struct Allocator {
    void *(*allocator)(Allocator *, AllocationKind kind, usize size, void *buffer); 
    void *user_data;
};

void *pDefaultAllocator(
        Allocator *, AllocationKind kind, usize size, void *buffer) 
{
    switch(kind) {
    case ALLOCATE:       return pAllocate(size);
    case ZERO_ALLOCATE:  return pZeroAllocate(size);
    case REALLOCATE:     return pReallocate(size, buffer);
    case FREE:           return (pFree(buffer), NULL); // NOLINT
    case SIZED_FREE:     return (pSizedFree(size, buffer), NULL);
    }
    return NULL;
}
const static Allocator PSTD_DEFAULT_HASH_MAP_ALLOCATOR = {
    .allocator = pDefaultAllocator,
};
#endif

#if defined(PSTD_FOR_MACRO)   && defined(PSTD_GNU_COMPATIBLE)
#define pForEach(array, ...)  pForEach_(array,  ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...) pForEachI_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachR(array, ...) pForEachI_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#elif defined(PSTD_FOR_MACRO) && !defined(PSTD_GNU_COMPATIBLE)
#error strechy buffer for each macro relies on a gnu extention that this compiler does not support (__auto_type)
#endif

#if defined(PSTD_GNU_COMPATIBLE) // not an msvc compiler

#if defined(PSTD_USE_ALLOCATOR)
#define pCreateStretchyBufferImplementation(data_type, allocator) ({                    \
     __auto_type _buf = pGetArrayMetaData(NULL, pSizeof(data_type), true, (allocator)); \
     (data_type *)(_buf + 1);                                                           \
     })
#endif

#define pFreeStretchyBufferImplementation(array) \
    pStretchyBufferFree(pGetMeta(array), pSizeof((array)[0]))

#define PSTD_FREE_ELEMENT(array, offset) ({                 \
    if (pGetMeta(array)->free_element) {                    \
        pGetMeta(array)->free_element((array) + (offset));  \
    }                                                       \
})

#define pSetFreeFuncImplementation(array, free_func) ({\
    pGetMetaOrCreate(array)->free_element = (free_func);\
})

#define pSizeofImplementation(value) (sizeof(__typeof(value)))

#if defined(PSTD_USE_ALLOCATOR)
#define pGetMetaImplementation(array) \
    ({ pGetArrayMetaData((array), pSizeof((array)[0]), false, PSTD_DEFAULT_ALLOCATOR); })
#else
#define pGetMetaImplementation(array) ({ pGetArrayMetaData((array), pSizeof((array)[0]), false); })
#endif

#if defined(PSTD_USE_ALLOCATOR)
#define pGetMetaOrCreateImplementation(array) ({                                            \
            pStretchyBufferMeta *pGetMeta_meta =                                            \
            pGetArrayMetaData((array), pSizeof((array)[0]), true, PSTD_DEFAULT_ALLOCATOR);  \
            (array) = (__typeof(array))(pGetMeta_meta + 1);                                 \
            pGetMeta_meta;                                                                  \
        })
#else 
#define pGetMetaOrCreateImplementation(array) ({                    \
            pStretchyBufferMeta *pGetMeta_meta =                    \
            pGetArrayMetaData((array), pSizeof((array)[0]), true);  \
            (array) = (__typeof(array))(pGetMeta_meta + 1);         \
            pGetMeta_meta;                                          \
        })
#endif

#define pSizeImplementation(array)   (pGetMeta(array)->size) 

#define pSetCapacityImplementation(array, count) \
    pSetCapacityImplementation_(&(array), (count), pSizeof((array)[0]))

#define pReserveImplementation(array, count) \
    pReserveImplementation_(&(array), (count), pSizeof((array)[0]))


#define pPushBackImplementation(array, value) ({                   \
    pGetMetaOrCreate(array);                                       \
    pMaybeGrowStretchyBuffer(&(array), pSizeof((array)[0]));       \
    __auto_type pPushBack_ret = (array) + pSize(array)++;          \
    *pPushBack_ret = (value);                                      \
    pPushBack_ret;                                                 \
})

#define pPushBytesImplementation(array, value, bytes) ({    \
    pGetMetaOrCreate(array);                                \
    pMaybeByteGrowStretchyBuffer(&(array), (bytes));        \
    memcpy((array) + pSize((array)), (value), (bytes));     \
    __auto_type pPushBack_ret = (array) + pSize((array));   \
    pSize((array)) += (bytes);                              \
    pPushBack_ret;                                          \
})


#define pBeginImplementation(array) ({ (array); })
#define pEndImplementation(array) ({ (array) + pSize(array); })

#define pBeginRImplementation(array) ({ (array) + pSize(array) - 1; })
#define pEndRImplementation(array) ({ (array) - 1; })

#define pInsertImplementation(array, position, value) ({                                        \
    __auto_type pInsert_array = pGetMetaOrCreate(array);                                        \
    usize pInsert_size = pSizeof( value );                                                      \
    usize pInsert_offset = (position) - pBegin(array);                                          \
    __typeof(value) *pInsert_result = NULL;                                                     \
    if (pInsert_array->size && pInsert_offset >= pInsert_array->size) {}                        \
    else {                                                                                      \
        pMaybeGrowStretchyBuffer(&(array), pInsert_size);                                       \
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
        usize pInsert_elems = pInsert_array->size - pInsert_offset;                             \
        if (pInsert_elems) {                                                                    \
            memmove((array) + pInsert_offset + 1,                                               \
                    (array) + pInsert_offset, pInsert_elems * pInsert_size);                    \
        }                                                                                       \
                                                                                                \
        pInsert_array->size++;                                                                  \
        (array)[pInsert_offset] = value;                                                        \
        pInsert_result = (array) + pInsert_offset;                                              \
    }                                                                                           \
    pInsert_result;                                                                             \
})


#define pPopBackImplementation(array) ({                        \
            __typeof((array)[0]) pPopBack_result = {0};         \
            if (pSize(array) == 0) {}                           \
            else {                                              \
                PSTD_FREE_ELEMENT(array, pSize(array));         \
                pPopBack_result = (array)[(pSize(array)--) - 1];\
            }                                                   \
            pPopBack_result;                                    \
        })

#define pRemoveImplementation(array, position) ({                                   \
    __typeof((array)[0]) pRemove_result = (__typeof((array)[0])){0};                \
    usize pRemove_offset = (position) - pBegin(array);                              \
    if (pRemove_offset >= pSize(array)) {}                                          \
    else if (pRemove_offset == pSize(array) - 1) {                                  \
        PSTD_FREE_ELEMENT(array, pRemove_offset);                                   \
        pSize(array)--;                                                             \
        pRemove_result = (array)[pRemove_offset];                                   \
    } else {                                                                        \
        PSTD_FREE_ELEMENT(array, pRemove_offset);                                   \
        pRemove_result = (array)[pRemove_offset];                                   \
        usize pRemove_elems = (pSize(array) - pRemove_offset) - 1;                  \
        memmove((array) + pRemove_offset, (array) + pRemove_offset + 1,             \
                pRemove_elems * pSizeof((array)[0]));                               \
        pSize(array)--;                                                             \
    }                                                                               \
    pRemove_result;                                                                 \
})

// arr:  another dynamic array
#define pCopyStretchyBufferImplementation(arr) ({\
    __auto_type pCopyStretchyBuffer_array  = pGetMeta(array);                             \
    usize pCopyStretchyBuffer_size = pSizeof((arr)[0]) * pCopyStretchyBuffer_array->size; \
    pStretchyBufferMeta *pCopyStretchyBuffer_copy =                                       \
        pAllocate(sizeof(pStretchyBufferMeta) * pCopyStretchyBuffer_size)           \
    pCopyStretchyBuffer_copy->endofstorage = pCopyStretchyBuffer_size;                    \
    pCopyStretchyBuffer_copy->size = pCopyStretchyBuffer_array->size;                     \
    memcpy((arr), pCopyStretchyBuffer_array + 1, pCopyStretchyBuffer_size;                \
    (__typeof((arr)[0])*)( pCopyStretchyBuffer_copy + 1);                                 \
})

// type: the type of array we want
// arr:  a static array
#define pCopyArrayImplementation(type, arr) ({      \
    type pCopyArray_tmp = NULL;                     \
    pReserve(pCopyArray_tmp, countof(arr));         \
    memcpy(pCopyArray_tmp, (arr), sizeof(arr));     \
    pGetMeta(pCopyArray_tmp)->size = countof(arr);  \
    pCopyArray_tmp;                                 \
})

typedef void pFreeFunction(void*);
typedef struct pStretchyBufferMeta pStretchyBufferMeta;
struct pStretchyBufferMeta {
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb;
#endif
    pFreeFunction *free_element;
    usize size;
    usize endofstorage;
#if PSTD_C99
    u8    buffer_data[];
#endif
};

// how many elements we should add
static void pStretchyBufferByteGrow(void* array, usize bytes);
static void pStretchyBufferGrow(void* array, usize datasize, usize count);

PSTD_UNUSED
static void pMaybeByteGrowStretchyBuffer(void* array, usize bytes) {
    pStretchyBufferMeta** meta_ptr = (pStretchyBufferMeta**)array;
    pStretchyBufferMeta* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        pStretchyBufferByteGrow(array, bytes);
    }
}

PSTD_UNUSED
static void pMaybeGrowStretchyBuffer(void* array, usize datasize) {
    pStretchyBufferMeta** meta_ptr = (pStretchyBufferMeta**)array;
    pStretchyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStretchyBufferGrow(array, datasize, PSTD_STRETCHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStretchyBufferByteGrow(void* array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8* array = *(u8**)array_ptr;
    pStretchyBufferMeta* meta = ((pStretchyBufferMeta*)array) - 1;
    usize size = sizeof(pStretchyBufferMeta) + meta->endofstorage + bytes;

#if defined(PSTD_USE_ALLOCATOR)
    assert(meta->cb.allocator);

    void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
    void *tmp = pReallocate(size, meta);
#endif
    assert(tmp); meta = (pStretchyBufferMeta*)tmp;

    meta->endofstorage += bytes;
    *(u8**)array_ptr = (u8*)(meta + 1);
}

static void pStretchyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    pStretchyBufferMeta* meta = ((pStretchyBufferMeta*)array) - 1;

#if defined(PSTD_USE_ALLOCATOR)
    assert(meta->cb.allocator);
#endif 

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(pStretchyBufferMeta) + array_size;

#if defined(PSTD_USE_ALLOCATOR)
    void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
    void *tmp = pReallocate(size, meta);
#endif
    assert(tmp); meta = (pStretchyBufferMeta*)tmp;

    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (u8*)(meta + 1);
}

PSTD_UNUSED
#if defined(PSTD_USE_ALLOCATOR)
static pStretchyBufferMeta* pGetArrayMetaData(void* array, usize data_size, pBool create, Allocator cb) {
#else
static pStretchyBufferMeta* pGetArrayMetaData(void* array, usize data_size, pBool create) {
#endif
#if defined(PSTD_USE_ALLOCATOR)
    if (!cb.allocator) {
        cb.allocator = pDefaultAllocator;
    }
#endif

    if (!array) {
        if (!create) { // should probably just return NULL
            static pStretchyBufferMeta nil = {0};
            nil = (pStretchyBufferMeta){0};
            return &nil;
        }
        pStretchyBufferMeta *meta;
#if defined(PSTD_USE_ALLOCATOR)
        meta = cb.allocator(&cb, ZERO_ALLOCATE, sizeof(pStretchyBufferMeta) + data_size, NULL);
        meta->cb = cb;
#else
        meta = pZeroAllocate(sizeof(pStretchyBufferMeta) + data_size);
#endif
        meta->endofstorage = data_size;
        meta->size = 0;
        return meta;
    }

    return ((pStretchyBufferMeta*)array) - 1;
}

PSTD_UNUSED
static usize pSetCapacityImplementation_(void *mem, usize count, usize data_size) {
    void **array_ptr = mem;
    if (!(*array_ptr)) {
        usize array_size = ((data_size) * (count));
        usize size = array_size + sizeof(pStretchyBufferMeta);
#if defined(PSTD_USE_ALLOCATOR)
        Allocator cb = PSTD_DEFAULT_ALLOCATOR;
        pStretchyBufferMeta *meta = cb.allocator(&cb, ZERO_ALLOCATE, size, NULL);
        meta->cb = cb;
#else
        pStretchyBufferMeta *meta = pZeroAllocate(size);
#endif
        meta->endofstorage = array_size;
        (*array_ptr) = (void*)(meta + 1);
    } else {
        __auto_type meta = pGetMeta(*array_ptr);

        usize size = ((data_size) * (count)) + sizeof(pStretchyBufferMeta);
#if defined(PSTD_USE_ALLOCATOR)
        assert(meta->cb.allocator);
        void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
        void *tmp = pReallocate(size, meta);
#endif
        assert(tmp);
        meta = tmp;
        (*array_ptr) = (void*)(meta + 1);
    }
    return count;
}

PSTD_UNUSED
static usize pReserveImplementation_(void *mem, usize count, usize data_size) {
    void **array_ptr = mem;
    if (!(*array_ptr)) {
        usize array_size = ((data_size) * (count));
        usize size = array_size + sizeof(pStretchyBufferMeta);
#if defined(PSTD_USE_ALLOCATOR)
        Allocator cb = PSTD_DEFAULT_ALLOCATOR;
        pStretchyBufferMeta *meta = cb.allocator(&cb, ZERO_ALLOCATE, size, NULL);
#else
        pStretchyBufferMeta *meta = pZeroAllocate(size);
#endif
        assert(meta);
        meta->endofstorage = array_size;
#if defined(PSTD_USE_ALLOCATOR)
        meta->cb           = cb;
#endif
        (*array_ptr)       = (void*)(meta + 1);
    } else if ((data_size) < (count)) {
        __auto_type meta = pGetMeta(*array_ptr);
        usize size = ((data_size) * (count)) + sizeof(pStretchyBufferMeta);
#if defined(PSTD_USE_ALLOCATOR)
        void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
        void *tmp = pReallocate(size, meta);
#endif
        assert(tmp);
        meta = tmp;
        (*array_ptr) = (void*)(meta + 1);
    }
    return count;
}

PSTD_UNUSED
static inline void pStretchyBufferFree(pStretchyBufferMeta *meta, usize data_size) {
    if (meta->free_element) {
        u8 *data = (void*)(meta + 1);
        for (isize i = meta->size - 1; i >= 0; i--) {
            meta->free_element(data + data_size * i);
        }
    }
#if defined(PSTD_USE_ALLOCATOR)
    meta->cb.allocator(&meta->cb, SIZED_FREE, sizeof(*meta) + meta->endofstorage, meta);
#else
    pSizedFree(sizeof(*meta) + meta->endofstorage, meta);
#endif
}

#else // using mvsc compiler
#define pSizeof(value) (sizeof(value))

#define pGetMetaOrCreate(array)                                                         \
    ((array) = (void*) pGetArrayMetaData((array), pSizeof((array)[0]),true),            \
            (array) = ((StrechyBufferMeta*)array) + 1, ((StrechyBufferMeta*)array) - 1)

#define pGetMeta(array) (pGetArrayMetaData((array), pSizeof((array)[0]), false))

#define pSetBufferAllocatorImplementation(array, allocator) \
    ({ pGetMetaOrCreate(array)->allocator = allocator; })


#define pSize(array)   (pGetMeta(array)->size) 
#define pLength(array) (pGetMeta(array)->size)
#define pLen(array)    (pGetMeta(array)->size)

#define pFreeStrechyBuffer(array) pFreeBuffer(pGetMeta(array))

#define pSetCapacity(array, count)   \
    (pSetArraySize(pGetMetaOrCreate(array), &(array), (pSizeof((array)[0]) * (count))), array) 


#define pSetCap     pSetCapacity
#define pSetCount   pSetCapacity
#define pSetSize    pSetCapacity
#define pSetLength  pSetCapacity
#define pReserve    pSetCapacity

#define pPushBack(array, value) \
    (pGetMetaOrCreate(array), pMaybeGrowStrechyBuffer(&(array), pSizeof((array)[0])),\
        *((array)+pSize(array)) = (value), pGetMeta(array)->size++, ((array)+(pSize(array)-1)))

#define pBegin(array) (array)
#define pEnd(array) ((array) + pSize(array))

#define pInsert(array, position, value)\
    (pGetMetaOrCreate(array), (((position) - pBegin(array)) >= pSize(array) ? NULL :              \
        ((position) = pInsertAtLocation(pGetMeta(array), &(array), &(position), pSizeof(value)))),\
        (*(position) = value), position)



#define pPopBack(array) /*due to not having any typeof we just return msvc (trash) on size 0*/\
    (pSize(array) == 0 ? (array)[-9034] : (pGetMeta(array)->size--, (array)[pSize(array)]))   \

#define pRemove(array, position)                                            \
    (((position) - pBegin(array)) >= pSize(array) ? *position :             \
    ((((position) - pBegin(array)) == pSize(array) - 1) ? pPopBack(array) : \
                (array)[pSwapAndPop(pGetMeta(array), &(array), &(position), pSizeof((array)[0]))]))

#define pCopyStrechyBuffer(array)    \
    (pCopyBuffer(pGetMeta(array), pSizeof((array)[0]))) 

#define pCopyArray(array) \
    (pCopyStaticArray((array), pSizeof((array)[0]), sizeof((array)))


typedef struct StrechyBufferMeta StrechyBufferMeta;
struct StrechyBufferMeta {
    Allocator cb;
    usize size;
    usize endofstorage;
};

// how many elements we should add
static void pStrechyBufferGrow(void* array, usize datasize, usize count);

static void pMaybeGrowStrechyBuffer(void* array, usize datasize) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStrechyBufferGrow(array, datasize, P_STRECHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStrechyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StrechyBufferMeta) + array_size;
    void* tmp = pReallocate(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1);
}

static StrechyBufferMeta p_strechy_buffer_nil = { 0 };
static StrechyBufferMeta* pGetArrayMetaData(void* array, usize data_size, bool should_create) {
    if (!array) {
        if (!should_create) {
            p_strechy_buffer_nil = (StrechyBufferMeta){ 0 };
            return &p_strechy_buffer_nil;
        }
        StrechyBufferMeta* meta = pZeroAllocate(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

static StrechyBufferMeta* pSetArraySize(StrechyBufferMeta* meta, void* array_ptr, usize new_size) {
    void* tmp = pReallocate(meta, (new_size)+sizeof(StrechyBufferMeta));
    assert(tmp); meta = tmp;
    meta->endofstorage = new_size;
    *((void**)array_ptr) = meta + 1;
    return meta;
}

static void* pCopyBuffer(StrechyBufferMeta* src, usize data_size) {
    if (src->size != 0) {
        StrechyBufferMeta* meta =
            pZeroAllocate((data_size * src->size) + sizeof(StrechyBufferMeta));
        meta->endofstorage = src->size * data_size;
        meta->size = src->size;
        memcpy(meta + 1, src + 1, src->size * data_size);
        return meta + 1;
    }
    else {
        void* arr;
        StrechyBufferMeta* meta = pNewArray(&arr, data_size);
        return arr;
    }
}

static void* pCopyStaticArray(void* array, usize data_size, usize count) {
    if (count != 0) {
        StrechyBufferMeta* meta =
            pZeroAllocate((data_size * count) + sizeof(StrechyBufferMeta));
        meta->endofstorage = count * data_size;
        meta->size = count;
        memcpy(meta + 1, array, count * data_size);
        return meta + 1;
    }
    else {
        void* arr;
        StrechyBufferMeta* meta = pNewArray(&arr, data_size);
        return arr;
    }
}

static void* pInsertAtLocation(StrechyBufferMeta* meta, void* array, void* location, usize data_size) {
    pMaybeGrowStrechyBuffer(array, data_size);

    /* first we extract all elements after the place where we want
     * to insert and then we shift them one element forward
     * here is an example we wan't to insert 6 at the place pointed to below
     * [1, 2, 3, 4]
     *     ^
     * we make a new array that holds [2, 3, 4]
     * we insert that into the array
     * [1, 2, 2, 3, 4]
     * then we insert the value
     * [1, 6, 2, 3, 4]
     */

    u8* array_ = (u8*)*(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(array_ + ((offset + 1) * data_size), (array_)+(offset * data_size), elems * data_size);
    meta->size++;
    return array_ + (offset * data_size);
}

static usize pSwapAndPop(StrechyBufferMeta* meta, void* array, void* location, usize data_size) {
    u8* buffer = (u8*)malloc(data_size);
    u8* data_start = (u8*)*(void**)location;
    memmove(buffer, data_start, data_size);

    u8* array_ = (u8*)*(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(data_start, data_start + data_size, elems * data_size);
    memmove(array_ + (--meta->size) * data_size, buffer, data_size);
    free(buffer);
    return meta->size;
}
#endif


#if defined(PSTD_FOR_MACRO) && ( defined(PSTD_GNU_COMPATIBLE) \
        || (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL == 0))
#define pHas2Args_(_0, a, b, _3, answer, _5, ...) answer
#define pHas2Args(a, ...)                         pHas2Args_(0, a, ## __VA_ARGS__, 1, 1, 0, 1) 
#define PSTD_CONCAT_( a, b )                      a##b
#define PSTD_CONCAT( a, b )                       PSTD_CONCAT_( a, b )
#define PSTD_STRINGIFY_(x)                        #x
#define PSTD_STRINGIFY(x)                         PSTD_STRINGIFY_(x)

#define pForEach1(array, name) \
    for( __auto_type name = pBegin(array); name != pEnd(array); name++)
#define pForEach0(array)        pForEach1(array, it)
#define pForEach__(array, args) PSTD_CONCAT(pForEach, args)
#define pForEach_(array, ...)   pForEach__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEachI1(array, name) \
    for( __auto_type name = pEnd(array) - 1; name != pBegin(array) - 1; name++)
#define pForEachI0(array)        pForEachI1(array, it)
#define pForEachI__(array, args) PSTD_CONCAT(pForEachR, args)
#define pForEachI_(array, ...)   pForEachI__(array, pHas2Args( array, ## __VA_ARGS__ ))
#elif defined(PSTD_FOR_MACRO) 
#error pstd For Macro Does not work with msvc traditional preprocessor
#endif

#endif // PSTD_STRETCHY_BUFFER_HEADER
