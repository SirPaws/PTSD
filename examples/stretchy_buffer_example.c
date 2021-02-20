




void *debug_malloc(unsigned long long size);
void *debug_zalloc(unsigned long long size);
void *debug_realloc(void *buf, unsigned long long size);

#include <stdlib.h>
#   define pReallocateBuffer debug_realloc
#   define pAllocateBuffer debug_malloc
#   define pFreeBuffer free
#   define pZeroAllocateBuffer debug_zalloc

#define STRETCHY_BUFFER_STANDALONE
#include "stretchy_buffer.h"

#include <stdio.h>


int main(void) {
    int *vector = NULL;
    // pReserve(vector, 5);

    pPushBack(vector, 1);
    pPushBack(vector, 2);
    pPushBack(vector, 3);
    pPushBack(vector, 4);
    
    int *insert = pBegin(vector);
    insert++;
    insert = pInsert(vector, insert, 6);

    for (int *it = pBegin(vector); it != pEnd(vector); it++) {
        printf("%p: %i\n", it, *it);
    }
   
    int v = pRemove(vector, insert);
    
    printf("removed the number 6 from array:\n");
    for (int *it = pBegin(vector); it != pEnd(vector); it++) {
        printf("%p: %i\n", it, *it);
    }

    pFreeStretchyBuffer(vector);
}

void *debug_malloc(unsigned long long size) {
    printf("allocated buffer of size %llu\n", size);
    return malloc(size);
}

void *debug_zalloc(unsigned long long size) {
    printf("zero allocated buffer of size %llu\n", size);
    void *data = malloc(size);
    assert(data); memset(data, 0, size);
    return data;
}

void *debug_realloc(void *buf, unsigned long long size) {
    printf("reallocated buffer %p to be %llu bytes long\n", buf, size);
    return realloc(buf, size);
}
