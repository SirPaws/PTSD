#include "dynarray.h"
#include "allocator.h"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void pDynArrayGrow(DynArray *vector, usize datasize, usize count) {
    if (!count || !vector || !datasize) return;
    void *tmp = pCurrentAllocatorFunc(vector->data, datasize * (vector->endofstorage + count), 0, REALLOC, pCurrentAllocatorUserData);
    assert(tmp);
    vector->data = tmp;
    vector->endofstorage += count;
}

void pDynArrayFree(DynArray *vector) {
    pCurrentAllocatorFunc(vector->data, 0, 0, FREE, pCurrentAllocatorUserData);
    memset(vector, 0, sizeof *vector);
}

