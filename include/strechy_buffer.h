#pragma once
#ifndef PSTD_STRECHY_BUFFER_HEADER
#ifndef STRECHY_BUFFER_STANDALONE
#   include "general.h"
#   include "allocator.h"
#else
#   include <stdlib.h>
#   include <string.h>
#   include <assert.h>
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
#   if !defined(STRECHY_BUFFER_NO_TYPES)
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

#       if !defined(STRECHY_BUFFER_NO_BOOL)
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



#ifndef P_STRECHY_BUFFER_GROWTH_COUNT
#define P_STRECHY_BUFFER_GROWTH_COUNT 2
#endif

#define pSizeof(value) (sizeof(__typeof(value)))

#define pGetMeta(array) ({                                              \
            StrechyBufferMeta *pGetMeta_meta =                          \
                pGetArrayMetaData((array), pSizeof((array)[0]), false); \
            (array) = (void*)(pGetMeta_meta + 1);                       \
            pGetMeta_meta;                                              \
        })

#define pGetMetaOrCreate(array) ({                                  \
            StrechyBufferMeta *pGetMeta_meta =                      \
            pGetArrayMetaData((array), pSizeof((array)[0]), true);  \
            (array) = (void*)(pGetMeta_meta + 1);                   \
            pGetMeta_meta;                                          \
        })

#define pSize(array)   (pGetMeta(array)->size) 
#define pLength(array) (pGetMeta(array)->size)
#define pLen(array)    (pGetMeta(array)->size)

#define pFreeStrechyBuffer(array) pFreeBuffer(pGetMeta(array))
#define pSetCapacity(array, count) ({                                               \
    if (!(array)) {                                                                 \
        StrechyBufferMeta *pSetCapacity_meta = pZeroAllocateBuffer(                 \
                    (pSizeof((array)[0]) * (count)) + sizeof(StrechyBufferMeta));   \
        pSetCapacity_meta->endofstorage = (pSizeof((array)[0]) * (count));          \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    } else {                                                                        \
        __auto_type pSetCapacity_meta = pGetMeta(array);                            \
        void *pSetCapacity_tmp = pReallocateBuffer(pSetCapacity_meta,               \
                (pSizeof((array)[0]) * (count)) + sizeof(StrechyBufferMeta));       \
        pSetCapacity_meta = pSetCapacity_tmp;                                       \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    }                                                                               \
    (array);                                                                        \
})


#define pSetCap     pSetCapacity
#define pSetCount   pSetCapacity
#define pSetSize    pSetCapacity
#define pSetLength  pSetCapacity
#define pReserve    pSetCapacity

#define pPushBack(array, value) ({                                 \
    pGetMetaOrCreate(array);                                       \
    pMaybeGrowStrechyBuffer(&(array), pSizeof((array)[0]));        \
    __auto_type pPushBack_ret = (array) + pSize(array)++;          \
    *pPushBack_ret = (value);                                      \
    pPushBack_ret;                                                 \
})

#define pPushBytes(array, value, bytes) ({                          \
    __auto_type pPushBytes_array = pGetMetaOrCreate(array);         \
    pMaybeByteGrowStrechyBuffer(&(array), (bytes));                 \
    memcpy((array) + (array)->size, (value), (bytes));              \
    __auto_type pPushBack_ret = (array) + ((array)->size);          \
    (array)->size += (bytes);                                       \
    pPushBack_ret;                                                  \
})

#define pBegin(array) ({ (array); })
#define pEnd(array) ({ (array) + pSize(array); })

#define pInsert(array, position, value) ({                                                      \
    __auto_type pInsert_array = pGetMetaOrCreate(array);                                        \
    usize pInsert_size = pSizeof( value );                                                      \
    usize pInsert_offset = (position) - pBegin(array);                                          \
    __typeof(value) *pInsert_result = NULL;                                                     \
    if (pInsert_offset >= pInsert_array->size) {}                                               \
    else {                                                                                      \
        pMaybeGrowStrechyBuffer(&(array), pInsert_size);                                        \
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
        void *pInsert_tmp = pAllocateBuffer(pInsert_size * pInsert_elems);                      \
        memcpy(pInsert_tmp, (array) + pInsert_offset,                                           \
                pInsert_elems * pInsert_size);                                                  \
        memcpy((array) + pInsert_offset + 1, pInsert_tmp,                                       \
                pInsert_elems * pInsert_size);                                                  \
        pFreeBuffer(pInsert_tmp);                                                               \
                                                                                                \
        pInsert_array->size++;                                                                  \
        (array)[pInsert_offset] = value;                                                        \
        pInsert_result = (array) + pInsert_offset;                                              \
    }                                                                                           \
    pInsert_result;                                                                             \
})


#define pPopBack(array) ({                                      \
            __typeof((array)[0]) pPopBack_result = {0};         \
            if (pSize(array) == 0) {}                           \
            else {                                              \
                pPopBack_result = (array)[(pSize(array)--) - 1];\
            }                                                   \
            pPopBack_result;                                    \
        })

#define pRemove(array, position) ({                                                 \
    __typeof((array)[0]) pRemove_result = (__typeof((array)[0])){0};                \
    usize pRemove_offset = (position) - pBegin(array);                              \
    if (pRemove_offset >= pSize(array)) {}                                          \
    else if (pRemove_offset == pSize(array) - 1) {                                  \
        pSize(array)--;                                                             \
        pRemove_result = (array)[pRemove_offset];                                   \
    } else {                                                                        \
        pRemove_result = (array)[pRemove_offset];                                   \
        usize pRemove_elems = pSize(array) - pRemove_offset;                        \
        void *pRemove_tmp = pAllocateBuffer(pSizeof((array)[0]) * pRemove_elems);   \
        memcpy(pRemove_tmp, (array) + pRemove_offset + 1,                           \
                pRemove_elems * pSizeof((array)[0]));                               \
        memcpy((array) + pRemove_offset, pRemove_tmp,                               \
                pRemove_elems * pSizeof((array)[0]));                               \
        pFreeBuffer(pRemove_tmp);                                                   \
        pSize(array)--;                                                             \
    }                                                                               \
    pRemove_result;                                                                 \
})

// arr:  another dynamic array
#define pCopyStrechyBuffer(arr) ({\
    __auto_type pCopyStrechyBuffer_array  = pGetMeta(array);                            \
    usize pCopyStrechyBuffer_size = pSizeof((arr)[0]) * pCopyStrechyBuffer_array->size; \
    StrechyBufferMeta *pCopyStrechyBuffer_copy =                                        \
        pAllocateBuffer(sizeof(StrechyBufferMeta) * pCopyStrechyBuffer_size)            \
    pCopyStrechyBuffer_copy->endofstorage = pCopyStrechyBuffer_size;                    \
    pCopyStrechyBuffer_copy->size = pCopyStrechyBuffer_array->size;                     \
    memcpy((arr), pCopyStrechyBuffer_array + 1, pCopyStrechyBuffer_size;                \
    (__typeof((arr)[0])*)( pCopyStrechyBuffer_copy + 1);                                \
})

// type: the type of array we want
// arr:  a static array
#define pCopyArray(type, arr) ({                    \
    type pCopyArray_tmp = NULL;                     \
    pReserve(pCopyArray_tmp, countof(arr));         \
    memcpy(pCopyArray_tmp, (arr), sizeof(arr));     \
    pGetMeta(pCopyArray_tmp)->size = countof(arr);  \
    pCopyArray_tmp;                                 \
})



typedef struct StrechyBufferMeta StrechyBufferMeta;
struct StrechyBufferMeta {
    usize size;
    usize endofstorage;
};

// how many elements we should add
static void pStrechyBufferByteGrow(void *array, usize bytes);
static void pStrechyBufferGrow(void *array, usize datasize, usize count);

PSTD_MAYBE_UNUSED
static void pMaybeByteGrowStrechyBuffer(void *array, usize bytes) {
    StrechyBufferMeta **meta_ptr = array;
    StrechyBufferMeta *meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        pStrechyBufferByteGrow(array, bytes);
    }
}

PSTD_MAYBE_UNUSED
static void pMaybeGrowStrechyBuffer(void *array, usize datasize) {
    StrechyBufferMeta **meta_ptr = array;
    StrechyBufferMeta *meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStrechyBufferGrow(array, datasize, P_STRECHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStrechyBufferByteGrow(void *array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8 *array = *(u8**)array_ptr;
    StrechyBufferMeta *meta = ((StrechyBufferMeta*)array) - 1;


    void *tmp = pReallocateBuffer(meta, sizeof(StrechyBufferMeta) + meta->endofstorage + bytes);
    assert(tmp); meta = tmp;
    meta->endofstorage += bytes;
    *(u8**)array_ptr = (void*)(meta + 1); 
}

static void pStrechyBufferGrow(void *array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8 *array = *(u8**)array_ptr;
    StrechyBufferMeta *meta = ((StrechyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StrechyBufferMeta) + array_size;
    void *tmp = pReallocateBuffer(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1); 
}
StrechyBufferMeta *pGetArrayMetaData(void *array, usize data_size, bool create) {
    if (!array) {
        if (!create){ // should probably just return NULL
            static StrechyBufferMeta nil = {0};
            nil = (StrechyBufferMeta){0};
            return &nil;
        }
        StrechyBufferMeta *meta = pZeroAllocateBuffer(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

#if !defined(NDEBUG)
StrechyBufferMeta *_debug_GetMeta(void *array) { //NOLINT
    StrechyBufferMeta *address = array;
    return address - 1;
}
#endif

#endif // PSTD_STRECHY_BUFFER_HEADER
