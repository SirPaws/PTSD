#pragma once
#ifndef PSTD_STRETCHY_BUFFER_HEADER

#ifndef STRETCHY_BUFFER_STANDALONE
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
typedef enum { false, true } pBool;
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
#define pBeginR(array) pBeginImplementation(array)
// returns a pointer to the element before the first element in the array
#define pEndR(array)   pEndImplementation(array)

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
#define pGetMeta(array)         pGetMetaImplementation(array)

// gets meta data for the array. if the array is equal to NULL
// then it creates a new strechy buffer and assigns array to that
#define pGetMetaOrCreate(array) pGetMetaOrCreateImplementation(array)

// utility function that removes some warning in GNU compatible compilers.
// in MSVC this is equivalent to sizeof(value).
// in GNU compatible this is equivalent to sizeof(__typeof(value))
#define pSizeof(value)          pSizeofImplementation(value)

























#if defined(PSTD_FOR_MACRO)   && defined(PSTD_GNU_COMPATIBLE)
#define pForEach(array, ...)  pForEach_(array,  ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...) pForEachI_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachR(array, ...) pForEachI_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#elif defined(PSTD_FOR_MACRO) && !defined(PSTD_GNU_COMPATIBLE)
#error strechy buffer for each macro relies on a gnu extention that this compiler does not support (__auto_type)
#endif

#ifndef PSTD_STRETCHY_BUFFER_GROWTH_COUNT
#define PSTD_STRETCHY_BUFFER_GROWTH_COUNT 2
#endif

#if defined(PSTD_GNU_COMPATIBLE) // not an msvc compiler

#define pSizeofImplementation(value) (sizeof(__typeof(value)))

#define pGetMetaImplementation(array) ({ pGetArrayMetaData((array), pSizeof((array)[0]), false); })

#define pGetMetaOrCreateImplementation(array) ({                                  \
            StretchyBufferMeta *pGetMeta_meta =                      \
            pGetArrayMetaData((array), pSizeof((array)[0]), true);  \
            (array) = (void*)(pGetMeta_meta + 1);                   \
            pGetMeta_meta;                                          \
        })

#define pSizeImplementation(array)   (pGetMeta(array)->size) 

#define pFreeStretchyBufferImplementation(array) pFreeBuffer(pGetMeta(array))
#define pSetCapacityImplementation(array, count) ({                                               \
    if (!(array)) {                                                                 \
        StretchyBufferMeta *pSetCapacity_meta = pZeroAllocateBuffer(                 \
                    (pSizeof((array)[0]) * (count)) + sizeof(StretchyBufferMeta));   \
        pSetCapacity_meta->endofstorage = (pSizeof((array)[0]) * (count));          \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    } else {                                                                        \
        __auto_type pSetCapacity_meta = pGetMeta(array);                            \
        void *pSetCapacity_tmp = pReallocateBuffer(pSetCapacity_meta,               \
                (pSizeof((array)[0]) * (count)) + sizeof(StretchyBufferMeta));       \
        pSetCapacity_meta = pSetCapacity_tmp;                                       \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    }                                                                               \
    (array);                                                                        \
})

#define pReserveImplementation(array, count) ({                                     \
    if (!(array)) {                                                                 \
        StretchyBufferMeta *pSetCapacity_meta = pZeroAllocateBuffer(                 \
                    (pSizeof((array)[0]) * (count)) + sizeof(StretchyBufferMeta));   \
        pSetCapacity_meta->endofstorage = (pSizeof((array)[0]) * (count));          \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    } else if (pSize(array) < (count)) {                                            \
        __auto_type pSetCapacity_meta = pGetMeta(array);                            \
        void *pSetCapacity_tmp = pReallocateBuffer(pSetCapacity_meta,               \
                (pSizeof((array)[0]) * (count)) + sizeof(StretchyBufferMeta));       \
        assert(pSetCapacity_tmp);                                                   \
        pSetCapacity_meta = pSetCapacity_tmp;                                       \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    }                                                                               \
    (array);                                                                        \
})


#define pPushBackImplementation(array, value) ({                   \
    pGetMetaOrCreate(array);                                       \
    pMaybeGrowStretchyBuffer(&(array), pSizeof((array)[0]));        \
    __auto_type pPushBack_ret = (array) + pSize(array)++;          \
    *pPushBack_ret = (value);                                      \
    pPushBack_ret;                                                 \
})

#define pPushBytesImplementation(array, value, bytes) ({    \
    pGetMetaOrCreate(array);                                \
    pMaybeByteGrowStretchyBuffer(&(array), (bytes));         \
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
        pMaybeGrowStretchyBuffer(&(array), pInsert_size);                                        \
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
                pPopBack_result = (array)[(pSize(array)--) - 1];\
            }                                                   \
            pPopBack_result;                                    \
        })

#define pRemoveImplementation(array, position) ({                                   \
    __typeof((array)[0]) pRemove_result = (__typeof((array)[0])){0};                \
    usize pRemove_offset = (position) - pBegin(array);                              \
    if (pRemove_offset >= pSize(array)) {}                                          \
    else if (pRemove_offset == pSize(array) - 1) {                                  \
        pSize(array)--;                                                             \
        pRemove_result = (array)[pRemove_offset];                                   \
    } else {                                                                        \
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
    __auto_type pCopyStretchyBuffer_array  = pGetMeta(array);                            \
    usize pCopyStretchyBuffer_size = pSizeof((arr)[0]) * pCopyStretchyBuffer_array->size; \
    StretchyBufferMeta *pCopyStretchyBuffer_copy =                                        \
        pAllocateBuffer(sizeof(StretchyBufferMeta) * pCopyStretchyBuffer_size)            \
    pCopyStretchyBuffer_copy->endofstorage = pCopyStretchyBuffer_size;                    \
    pCopyStretchyBuffer_copy->size = pCopyStretchyBuffer_array->size;                     \
    memcpy((arr), pCopyStretchyBuffer_array + 1, pCopyStretchyBuffer_size;                \
    (__typeof((arr)[0])*)( pCopyStretchyBuffer_copy + 1);                                \
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

typedef struct StretchyBufferMetaa StretchyBufferMeta;
struct StretchyBufferMetaa {
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
    StretchyBufferMeta** meta_ptr = array;
    StretchyBufferMeta* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        pStretchyBufferByteGrow(array, bytes);
    }
}

PSTD_UNUSED
static void pMaybeGrowStretchyBuffer(void* array, usize datasize) {
    StretchyBufferMeta** meta_ptr = array;
    StretchyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStretchyBufferGrow(array, datasize, PSTD_STRETCHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStretchyBufferByteGrow(void* array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8* array = *(u8**)array_ptr;
    StretchyBufferMeta* meta = ((StretchyBufferMeta*)array) - 1;


    void* tmp = pReallocateBuffer(meta, sizeof(StretchyBufferMeta) + meta->endofstorage + bytes);
    assert(tmp); meta = tmp;
    meta->endofstorage += bytes;
    *(u8**)array_ptr = (void*)(meta + 1);
}

static void pStretchyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    StretchyBufferMeta* meta = ((StretchyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StretchyBufferMeta) + array_size;
    void* tmp = pReallocateBuffer(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1);
}
StretchyBufferMeta* pGetArrayMetaData(void* array, usize data_size, pBool create) {
    if (!array) {
        if (!create) { // should probably just return NULL
            static StretchyBufferMeta nil = { 0 };
            nil = (StretchyBufferMeta){ 0 };
            return &nil;
        }
        StretchyBufferMeta* meta = pZeroAllocateBuffer(sizeof(StretchyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StretchyBufferMeta*)array) - 1;
}

#if !defined(NDEBUG)
StretchyBufferMeta* _debug_GetMeta(void* array) { //NOLINT
    StretchyBufferMeta* address = array;
    return address - 1;
}
#endif
#else // using mvsc compiler
int msvc_not_implemented[-1];
#endif

#if defined(PSTD_FOR_MACRO)

#define pHas2Args_(_0, a, b, _3, answer, _5, ...) answer
#define pHas2Args(a, ...) pHas2Args_(0, a, ## __VA_ARGS__, 1, 1, 0, 1) 
#define PSTD_CONCAT_( a, b ) a##b
#define PSTD_CONCAT( a, b ) PSTD_CONCAT_( a, b )
#define PSTD_STRINGIFY_(x) #x
#define PSTD_STRINGIFY(x) PSTD_STRINGIFY_(x)

#define pForEach1(array, name)  for( __auto_type name = pBegin(array); name != pEnd(array); name++)
#define pForEach0(array)        pForEach1(array, it)
#define pForEach__(array, args) PSTD_CONCAT(pForEach, args)
#define pForEach_(array, ...)    pForEach__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEachI1(array, name)  for( __auto_type name = pEnd(array) - 1; name != pBegin(array) - 1; name++)
#define pForEachI0(array)        pForEachI1(array, it)
#define pForEachI__(array, args) PSTD_CONCAT(pForEachR, args)
#define pForEachI_(array, ...)   pForEachI__(array, pHas2Args( array, ## __VA_ARGS__ ))

#elif defined(PSTD_FOR_MACRO) && !defined(PSTD_GNU_COMPATIBLE)
#endif



#endif // PSTD_STRETCHY_BUFFER_HEADER
