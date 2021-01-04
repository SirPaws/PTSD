#pragma once
#ifndef PSTD_STRECHY_BUFFER_HEADER

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#define PSTD_GNU_COMPATIBLE
#else
#define PSTD_MSVC
#endif

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
struct String { usize length; u8* c_str; };

#       if !defined(STRECHY_BUFFER_NO_BOOL)
typedef enum bool{ false, true } bool;
#       endif
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
#   if defined(PSTD_GNU_COMPATIBLE)
#   ifndef pZeroAllocateBuffer
#       define pZeroAllocateBuffer(size) ({                 \
            void *pZeroAllocateBuffer_tmp = malloc(size);   \
            memset(pZeroAllocateBuffer_tmp, 0, (size));     \
            pZeroAllocateBuffer_tmp;                        \
        })
#   endif
#   else
        static void* pZeroAllocateBuffer(usize size) {
            void* pZeroAllocateBuffer_tmp = pAllocateBuffer(size);
            memset(pZeroAllocateBuffer_tmp, 0, (size));
            return pZeroAllocateBuffer_tmp;
        }
#   endif
#endif


#ifndef P_STRECHY_BUFFER_GROWTH_COUNT
#define P_STRECHY_BUFFER_GROWTH_COUNT 2
#endif

#if defined(PSTD_GNU_COMPATIBLE) // not an msvc compiler

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
    if (pInsert_array->size && pInsert_offset >= pInsert_array->size) {}                        \
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
        memmove((array) + pRemove_offset, (array) + pRemove_offset + 1,             \
                pRemove_elems * pSizeof((array)[0]));                               \
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
static void pStrechyBufferByteGrow(void* array, usize bytes);
static void pStrechyBufferGrow(void* array, usize datasize, usize count);

PSTD_MAYBE_UNUSED
static void pMaybeByteGrowStrechyBuffer(void* array, usize bytes) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        pStrechyBufferByteGrow(array, bytes);
    }
}

PSTD_MAYBE_UNUSED
static void pMaybeGrowStrechyBuffer(void* array, usize datasize) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStrechyBufferGrow(array, datasize, P_STRECHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStrechyBufferByteGrow(void* array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;


    void* tmp = pReallocateBuffer(meta, sizeof(StrechyBufferMeta) + meta->endofstorage + bytes);
    assert(tmp); meta = tmp;
    meta->endofstorage += bytes;
    *(u8**)array_ptr = (void*)(meta + 1);
}

static void pStrechyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StrechyBufferMeta) + array_size;
    void* tmp = pReallocateBuffer(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1);
}
StrechyBufferMeta* pGetArrayMetaData(void* array, usize data_size, bool create) {
    if (!array) {
        if (!create) { // should probably just return NULL
            static StrechyBufferMeta nil = { 0 };
            nil = (StrechyBufferMeta){ 0 };
            return &nil;
        }
        StrechyBufferMeta* meta = pZeroAllocateBuffer(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

#if !defined(NDEBUG)
StrechyBufferMeta* _debug_GetMeta(void* array) { //NOLINT
    StrechyBufferMeta* address = array;
    return address - 1;
}
#endif
#else // using mvsc compiler
#define pSizeof(value) (sizeof(value))

#define pGetMetaOrCreate(array)                                                         \
    ((array) = (void*) pGetArrayMetaData((array), pSizeof((array)[0]),true),            \
            (array) = ((StrechyBufferMeta*)array) + 1, ((StrechyBufferMeta*)array) - 1)

#define pGetMeta(array) (pGetArrayMetaData((array), pSizeof((array)[0]), false))


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
    void* tmp = pReallocateBuffer(meta, size);
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
        StrechyBufferMeta* meta = pZeroAllocateBuffer(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

static StrechyBufferMeta* pSetArraySize(StrechyBufferMeta* meta, void* array_ptr, usize new_size) {
    void* tmp = pReallocateBuffer(meta, (new_size)+sizeof(StrechyBufferMeta));
    assert(tmp); meta = tmp;
    meta->endofstorage = new_size;
    *((void**)array_ptr) = meta + 1;
    return meta;
}

static void* pCopyBuffer(StrechyBufferMeta* src, usize data_size) {
    if (src->size != 0) {
        StrechyBufferMeta* meta = 
            pZeroAllocateBuffer((data_size * src->size)+ sizeof(StrechyBufferMeta));
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
            pZeroAllocateBuffer((data_size * count) + sizeof(StrechyBufferMeta));
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

    u8* array_ = *(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(array_ + ((offset + 1) * data_size), (array_)+(offset * data_size), elems * data_size);
    meta->size++;
    return array_ + (offset * data_size);
}

static usize pSwapAndPop(StrechyBufferMeta* meta, void* array, void* location, usize data_size) {
    u8* buffer = malloc(data_size);
    u8* data_start = *(void**)location;
    memmove(buffer, data_start, data_size);

    u8* array_ = *(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(data_start, data_start + data_size, elems * data_size);
    memmove(array_ + (--meta->size) * data_size, buffer, data_size);
    free(buffer);
    return meta->size;
}
#endif
#endif // PSTD_STRECHY_BUFFER_HEADER
