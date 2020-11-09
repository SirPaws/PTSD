#pragma once
#ifndef PSTD_DYNARRAY_HEADER
#include "general.h"

#define pCreateDynArray(name, datatype) \
    struct name {           \
        usize endofstorage; \
        usize size;         \
        datatype *data;     \
    }

#ifndef P_DYNARRAY_GROWTH_COUNT
#define P_DYNARRAY_GROWTH_COUNT 2
#endif

#define MACRO_IF(cond)        (cond) ?
#define MACRO_ELIF(cond)    : (cond) ?
#define MACRO_ELSE          :

#define pCreateStaticDynArray(type, value) (type){ countof(value), countof(value), value }


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
        pMaybeByteGrowDynArray((DynArray *)(array), num_bytes);                                      \
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
        void *pMakeHole_tmp = pAllocateBuffer(pMakeHole_size * pMakeHole_elems);                  \
        memcpy(pMakeHole_tmp, (array)->data + pMakeHole_offset,                                 \
                pMakeHole_elems * pMakeHole_size);                                              \
        memcpy((array)->data + pMakeHole_offset + num_bytes, pMakeHole_tmp,                          \
                pMakeHole_elems * pMakeHole_size);                                              \
        pFreeBuffer(pMakeHole_tmp);                                                             \
                                                                                                \
        pMakeHole_result = (array)->data + pMakeHole_offset;                                    \
    }                                                                                           \
    pMakeHole_result;                                                                           \
})


#define pPopBack(array) ({                                              \
            MACRO_IF((array)->size == 0) (typeof((array)->data[0])){ 0 }\
            MACRO_ELSE ({   (array)->data[((array)->size--) - 1];  });        \
        })

#define pRemove(array, position) ({                                                                         \
    usize pRemove_offset = position - pBegin(array);                                                        \
    MACRO_IF(pRemove_offset >= (array)->size)                                                               \
        (__typeof((array)->data[0])){ 0 }                                                                   \
    MACRO_ELIF(pRemove_offset == (array)->size - 1) ({                                                      \
        (array)->size--;                                                                                    \
        (array)->data[pRemove_offset];                                                                      \
    }) MACRO_ELSE ({                                                                                        \
        __auto_type pRemove_ret = (array)->data[pRemove_offset];                                            \
        usize pRemove_elems = (array)->size - pRemove_offset;                                               \
        void *pRemove_tmp = pAllocateBuffer(sizeof((array)->data[0]) * pRemove_elems);                      \
        memcpy(pRemove_tmp, (array)->data + pRemove_offset + 1, pRemove_elems * sizeof((array)->data[0]));  \
        memcpy((array)->data + pRemove_offset, pRemove_tmp, pRemove_elems * sizeof((array)->data[0]));      \
        pFreeBuffer(pRemove_tmp);                                                                           \
                                                                                                            \
        (array)->size--;                                                                                    \
        pRemove_ret;                                                                                        \
    });                                                                                                     \
})

// arr:  another dynamic array
#define pCopyDynArray(arr) ({                                                           \
        type pCopyDynArray_tmp = { 0 };                                                 \
        pCopyDynArray_tmp.data = pAllocateBuffer( (arr).size );                         \
        memcpy(pCopyDynArray_tmp.data, (arr).data, sizeof(*(arr).data) * (arr).size);   \
        pCopyDynArray_tmp.size = pCopyDynArray_tmp.endofstorage = (arr).size;           \
        pCopyDynArray_tmp;})

// type: the type of array we want
// arr:  a static array
#define pCopyArray(type, arr) ({                                            \
        type pCopyArray_tmp = { 0 };                                        \
        pCopyArray_tmp.data = pAllocateBuffer( sizeof(arr) );               \
        memcpy(pCopyArray_tmp.data, (arr), sizeof(arr));                    \
        pCopyArray_tmp.size = pCopyArray_tmp.endofstorage = countof(arr);   \
        pCopyArray_tmp;})

typedef struct DynArray DynArray;
struct DynArray {
    usize endofstorage;
    usize size;
    void *data;
};

// how many elements we should add
void pDynArrayByteGrow(DynArray *, usize bytes);
void pDynArrayGrow(DynArray *, usize datasize, usize count);
void pDynArrayFree(DynArray *);

PSTD_MAYBE_UNUSED
static void pMaybeByteGrowDynArray(DynArray *array, usize bytes) {
    if (array->size + bytes > array->endofstorage) {\
        pDynArrayByteGrow(array, bytes);\
    }
}

PSTD_MAYBE_UNUSED
static void pMaybeGrowDynArray(DynArray *array, usize datasize) {
    if (array->size + datasize > array->endofstorage) {\
        pDynArrayGrow((DynArray *)array, \
                datasize, P_DYNARRAY_GROWTH_COUNT);\
    }
}
#endif // PSTD_DYNARRAY_HEADER

