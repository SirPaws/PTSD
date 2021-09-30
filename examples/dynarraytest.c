#include "dynarray.h"
#include "util.h"

typedef pda_create_dynarray(, int) IntVector;

int main(void) {
    IntVector vector = { 0 };
    pda_pushback(&vector, 1);
    pda_pushback(&vector, 2);
    pda_pushback(&vector, 3);
    pda_pushback(&vector, 4);
    
    printf("foreach loop (unnamed): \n");
    pda_foreach(&vector) {
        printf("    %p: %i\n", it, *it);
        // should print 4 pointers and the values 
        // 1, 2, 3, 4
    }
    
    printf("reversed foreach loop (unnamed): \n");
    pda_foreach_r(&vector) {
        printf("    %p: %i\n", it, *it);
        // should print 4 pointers and the values 
        // 4, 3, 2, 1
    }
    
    printf("foreach loop (named): \n");
    pda_foreach(&vector, i) {
        printf("    %p: %i\n", i, *i);
        // should print 4 pointers and the values 
        // 1, 2, 3, 4
    }
    
    printf("reversed foreach loop (named): \n");
    pda_foreach_r(&vector, i) {
        printf("    %p: %i\n", i, *i);
        // should print 4 pointers and the values 
        // 4, 3, 2, 1
    }
    
    int *insert = pda_begin(&vector);
    insert++;
    insert = pda_insert(&vector, insert, 6);

    printf("with iterator loop: \n");
    for (int *it = pda_begin(&vector); it != pda_end(&vector); it++) {
        printf("    %p: %i\n", it, *it);
        // should print 5 pointers and the values 
        // 1, 6, 2, 3, 4
    }
    
    int v = pda_remove(&vector, insert);
    printf("removed value: %i\n", v);

    pda_free((pdynarray_t *)&vector);
}


