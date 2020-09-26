#pragma once
#include "general.h"

#define pCreateDynArray(name, datatype) \
    struct name {           \
        usize size;         \
        usize endofstorage; \
        datatype *data;     \
    }

#ifndef P_DYNARRAY_GROWTH_COUNT
#define P_DYNARRAY_GROWTH_COUNT 2
#endif

#define MACRO_IF(cond)        (cond) ?
#define MACRO_ELIF(cond)    : (cond) ?
#define MACRO_ELSE          :

#define _MACRO_NAME(name)

#define pCreateStaticDynArray(type, value) (type){ countof(value), countof(value), value }


#define pPushBack(array, value) ({                              \
    pMaybeGrowDynArray((DynArray *)(array), sizeof(value));     \
    __auto_type pPushBack_ret = (array)->data + (array)->size++;\
    *pPushBack_ret = (value);                                   \
    pPushBack_ret;                                              \
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

#define pInsert(array, position, value) ({                                            \
    pMaybeGrowDynArray((DynArray *)(array), sizeof(value));                           \
    usize pInsert_offset = position - pBegin(array);                                          \
    MACRO_IF(pInsert_offset >= (array)->size)                                                 \
        NULL                                                                          \
    MACRO_ELSE ({                                                                     \
        /* first we extract all elements after the place where we want              */\
        /* to insert and then we shift them one element forward                     */\
        /* here is an example we wan't to insert 6 at the place pointed to below    */\
        /* [1, 2, 3, 4]                                                             */\
        /*     ^                                                                    */\
        /* we make a new array that holds [2, 3, 4]                                 */\
        /* we insert that into the array                                            */\
        /* [1, 2, 2, 3, 4]                                                          */\
        /* then we insert the value                                                 */\
        /* [1, 6, 2, 3, 4]                                                          */\
        usize pInsert_elems = (array)->size - pInsert_offset;                                         \
        void *pInsert_tmp = pAllocateBuffer(sizeof(value) * pInsert_elems);                           \
        memcpy(pInsert_tmp, (array)->data + pInsert_offset, pInsert_elems * sizeof(value));                   \
        memcpy((array)->data + pInsert_offset + 1, pInsert_tmp, pInsert_elems * sizeof(value));               \
        pFreeBuffer(pInsert_tmp);                                                             \
                                                                                      \
        (array)->size++;                                                              \
        (array)->data[pInsert_offset] = value;                                                \
        (array)->data + pInsert_offset;                                                       \
    });                                                                               \
})

#define pPopBack(array) ({                                              \
            MACRO_IF((array)->size == 0) (typeof((array)->data[0])){ 0 }\
            MACRO_ELSE ({   (array)->data[(array)->size--];  });        \
        })

#define pRemove(array, position) ({                                                 \
    usize pRemove_offset = position - pBegin(array);                                        \
    MACRO_IF(pRemove_offset >= (array)->size)                                               \
        (__typeof((array)->data[0])){ 0 }                                           \
    MACRO_ELIF(pRemove_offset == (array)->size - 1) ({                                      \
        (array)->size--;                                                            \
        (array)->data[pRemove_offset];                                                      \
    }) MACRO_ELSE ({                                                                \
        __auto_type pRemove_ret = (array)->data[pRemove_offset];                                    \
        usize pRemove_elems = (array)->size - pRemove_offset;                                       \
        void *pRemove_tmp = pAllocateBuffer(sizeof((array)->data[0]) * pRemove_elems);              \
        memcpy(pRemove_tmp, (array)->data + pRemove_offset + 1, pRemove_elems * sizeof((array)->data[0]));  \
        memcpy((array)->data + pRemove_offset, pRemove_tmp, pRemove_elems * sizeof((array)->data[0]));      \
        pFreeBuffer(pRemove_tmp);                                                           \
                                                                                    \
        (array)->size--;                                                            \
        pRemove_ret;                                                                        \
    });                                                                             \
})

typedef struct DynArray DynArray;
struct DynArray {
    usize size;
    usize endofstorage;
    void *data;
};




// how many elements we should add
void pDynArrayByteGrow(DynArray *, usize bytes);
void pDynArrayGrow(DynArray *, usize datasize, usize count);
void pDynArrayFree(DynArray *);

static void pMaybeByteGrowDynArray(DynArray *array, usize bytes) {
    if (array->size + bytes > array->endofstorage) {\
        pDynArrayByteGrow(array, bytes);\
    }
}

static void pMaybeGrowDynArray(DynArray *array, usize datasize) {
    if (array->size + datasize > array->endofstorage) {\
        pDynArrayGrow((DynArray *)array, \
                datasize, P_DYNARRAY_GROWTH_COUNT);\
    }
}




