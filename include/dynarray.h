#pragma once
#ifndef PSTD_DYNARRAY_HEADER
#ifndef DYNARRAY_STANDALONE
#   include "general.h"
#   include "allocator.h"
#else
#   if defined(_WIN32) || defined(_WIN64)
#       define PSTD_WINDOWS
#   elif defined(__linux__) || defined(__unix__)
#       define PSTD_LINUX
#   elif defined(__APPLE__)
#       define PSTD_MACOS
#   endif
#   if __STDC_VERSION__ == 202000L  // this will probably break
#       define PSTD_MAYBE_UNUSED [[maybe_unused]]
#   else
#       define PSTD_MAYBE_UNUSED __attribute__((unused))
#   endif
#   if !defined(DYNARRAY_NO_TYPES)
#       include <stddef.h>
#       include <stdint.h>
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

        typedef struct String String;
        struct String { usize length; u8 *c_str; };

#       if !defined(DYNARRAY_NO_BOOL)
            typedef enum bool { false, true } bool;
#       endif
#   endif 
#   ifndef pZeroAllocateBuffer
#       define pZeroAllocateBuffer(size) ({                 \
            void *pZeroAllocateBuffer_tmp = malloc(size);   \
            memset(pZeroAllocateBuffer_tmp, 0, (size));     \
            pZeroAllocateBuffer_tmp;                        \
        })
#   endif
#   ifndef pReallocateBuffer
#       define pReallocateBuffer realloc
#   endif
#   ifndef pAllocateBuffer
#      define pAllocateBuffer malloc
#   endif
#   ifndef pFreeBuffer
#      define pFreeBuffer free
#   endif
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

PSTD_MAYBE_UNUSED
static void pMaybeByteGrowDynArray(DynArray *array, usize bytes) {
    if (array->size + bytes > array->endofstorage) {             \
        pDynArrayByteGrow(array, bytes);                         \
    }
}

PSTD_MAYBE_UNUSED
static void pMaybeGrowDynArray(DynArray *array, usize datasize) {
    if ((array->size + 1) * datasize > array->endofstorage) {   \
        pDynArrayGrow((DynArray *)array,                        \
                datasize, P_DYNARRAY_GROWTH_COUNT);             \
    }
}

PSTD_MAYBE_UNUSED
static void pDynArrayByteGrow(DynArray *array, usize bytes) {
    if (!bytes || !array) return;
    void *tmp = pReallocateBuffer(array->data, array->endofstorage + bytes);
    // assert(tmp);
    array->data = tmp;
    array->endofstorage += bytes;
}

PSTD_MAYBE_UNUSED
static void pDynArrayGrow(DynArray *array, usize datasize, usize count) {
    if (!count || !array || !datasize) return;
    void *tmp = pReallocateBuffer(array->data, datasize * (array->endofstorage + count));
    // assert(tmp);
    array->data = tmp;
    array->endofstorage += count;
}

PSTD_MAYBE_UNUSED
static void pDynArrayFree(DynArray *array) {
    pFreeBuffer(array->data);
    memset(array, 0, sizeof *array);
}
#endif // PSTD_DYNARRAY_HEADER
