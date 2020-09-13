#include "dynarray.h"
#include "allocator.h"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void pDynArrayByteGrow(DynArray *array, usize bytes) {
    if (!bytes || !array) return;
    void *tmp = pCurrentAllocatorFunc(array->data, array->endofstorage + bytes, 0, REALLOC, pCurrentAllocatorUserData);
    assert(tmp);
    array->data = tmp;
    array->endofstorage += bytes;
}

void pDynArrayGrow(DynArray *array, usize datasize, usize count) {
    if (!count || !array || !datasize) return;
    void *tmp = pCurrentAllocatorFunc(array->data, datasize * (array->endofstorage + count), 0, REALLOC, pCurrentAllocatorUserData);
    assert(tmp);
    array->data = tmp;
    array->endofstorage += count;
}

void pDynArrayFree(DynArray *array) {
    pCurrentAllocatorFunc(array->data, 0, 0, FREE, pCurrentAllocatorUserData);
    memset(array, 0, sizeof *array);
}

