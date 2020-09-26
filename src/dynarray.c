#include "dynarray.h"
#include "allocator.h"

void pDynArrayByteGrow(DynArray *array, usize bytes) {
    if (!bytes || !array) return;
    void *tmp = pReallocateBuffer(array->data, array->endofstorage + bytes);
    assert(tmp);
    array->data = tmp;
    array->endofstorage += bytes;
}

void pDynArrayGrow(DynArray *array, usize datasize, usize count) {
    if (!count || !array || !datasize) return;
    void *tmp = pReallocateBuffer(array->data, datasize * (array->endofstorage + count));
    assert(tmp);
    array->data = tmp;
    array->endofstorage += count;
}

void pDynArrayFree(DynArray *array) {
    pFreeBuffer(array->data);
    memset(array, 0, sizeof *array);
}

