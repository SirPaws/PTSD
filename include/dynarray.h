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

#define pPushBack(array, value) ({                          \
    pMaybeGrowDynArray((DynArray *)(array), sizeof(value)); \
    __auto_type ret = (array)->data + (array)->size++;      \
    *ret = (value);                                         \
    ret;                                                    \
})

#define pBegin(array) ({ (array)->data; })
#define pEnd(array) ({ (array)->data + (array)->size; })

#define pInsert(array, position, value) ({                                            \
    pMaybeGrowDynArray((DynArray *)(array), sizeof(value));                           \
    usize offset = position - pBegin(array);                                          \
    MACRO_IF(offset >= (array)->size)                                                 \
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
        usize elems = (array)->size - offset;                                         \
        void *tmp = pAllocateBuffer(sizeof(value) * elems);                           \
        memcpy(tmp, (array)->data + offset, elems * sizeof(value));                   \
        memcpy((array)->data + offset + 1, tmp, elems * sizeof(value));               \
        pFreeBuffer(tmp);                                                             \
                                                                                      \
        (array)->size++;                                                              \
        (array)->data[offset] = value;                                                \
        (array)->data + offset;                                                       \
    });                                                                               \
})

#define pRemove(array, position) ({                                                 \
    usize offset = position - pBegin(array);                                        \
    MACRO_IF(offset >= (array)->size)                                               \
        (__typeof((array)->data[0])){ 0 }                                           \
    MACRO_ELIF(offset == (array)->size - 1) ({                                      \
        (array)->size--;                                                            \
        (array)->data[offset];                                                      \
    }) MACRO_ELSE ({                                                                \
        __auto_type ret = (array)->data[offset];                                    \
        usize elems = (array)->size - offset;                                       \
        void *tmp = pAllocateBuffer(sizeof((array)->data[0]) * elems);              \
        memcpy(tmp, (array)->data + offset + 1, elems * sizeof((array)->data[0]));  \
        memcpy((array)->data + offset, tmp, elems * sizeof((array)->data[0]));      \
        pFreeBuffer(tmp);                                                           \
                                                                                    \
        (array)->size--;                                                            \
        ret;                                                                        \
    });                                                                             \
})

typedef struct DynArray DynArray;
struct DynArray {
    usize size;
    usize endofstorage;
    void *data;
};




// how many elements we should add
void pDynArrayGrow(DynArray *, usize datasize, usize count);
void pDynArrayFree(DynArray *);

static void pMaybeGrowDynArray(DynArray *array, usize datasize) {
    if (array->size + 1 > array->endofstorage) {\
        pDynArrayGrow((DynArray *)array, \
                sizeof(array->data[0]), P_DYNARRAY_GROWTH_COUNT);\
    }
}




