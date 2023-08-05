// #define STRETCHY_BUFFER_STANDALONE
#include "stretchy_buffer.h"

#include <stdio.h>
#include <stdlib.h>



int main(void) {
    int *vector = NULL;

    psb_pushback(vector, 1);
    psb_pushback(vector, 2);
    psb_pushback(vector, 3);
    psb_pushback(vector, 4);
    
    int *insert = psb_begin(vector);
    insert++;
    insert = psb_insert(vector, insert, 6);

    for (int *it = psb_begin(vector); it != psb_end(vector); it++) {
        printf("%p: %i\n", it, *it);
    }
   
    int v = psb_remove(vector, insert);
    assert(v == 6);
    
    printf("removed the number %i from array:\n", v);
    for (int *it = psb_begin(vector); it != psb_end(vector); it++) {
        printf("%p: %i\n", it, *it);
    }
    psb_free(vector);
}

#if defined(PSTD_USE_ALLOCATOR)
void *debug_allocator(Allocator *, AllocationKind kind, usize size, void *buf) {
    switch (kind) {
    case FREE: {
            printf("freed buffer of size %llu\n", size);
            return malloc(size);
        }
    case ALLOCATE: {
            printf("allocated buffer of size %llu\n", size);
            return malloc(size);
        }
    case REALLOCATE: {
            printf("reallocated buffer %p to be %llu bytes long\n", buf, size);
            return realloc(buf, size);
        }
    case ZERO_ALLOCATE: {
            printf("zero allocated buffer of size %llu\n", size);
            void *data = malloc(size);
            assert(data); memset(data, 0, size);
            return data;
        }
    }
}
#endif 
