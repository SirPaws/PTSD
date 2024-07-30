// #define STRETCHY_BUFFER_STANDALONE
#include "stretchy_buffer.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *vector = NULL;
    psb_reserve(vector, 4);

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

    // same as 
    psb_foreach(vector) {
        printf("%p: %i\n", it, *it);
    }
   
    int v = psb_remove(vector, insert);
    assert(v == 6);
    
    printf("removed the number %i from array:\n", v);
    for (int *value = psb_begin(vector); value != psb_end(vector); value++) {
        printf("%p: %i\n", value, *value);
    }
    
    // same as 
    psb_foreach(vector, value) {
        printf("%p: %i\n", value, *value);
    }

    psb_free(vector);
}
