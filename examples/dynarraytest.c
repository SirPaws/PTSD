#include "dynarray.h"
#include "util.h"

typedef pda_create_dynarray(, int) IntVector;

int main(void) {
    IntVector vector = { 0 };
    pda_pushback(&vector, 1);
    pda_pushback(&vector, 2);
    pda_pushback(&vector, 3);
    pda_pushback(&vector, 4);
    
    int *insert = pda_begin(&vector);
    insert++;
    insert = pda_insert(&vector, insert, 6);

    for (int *it = pda_begin(&vector); it != pda_end(&vector); it++) {
        printf("%p: %i\n", it, *it);
    }
    
    int v = pda_remove(&vector, insert);
    printf("removed value: %i\n", v);

    pda_free((DynArray *)&vector);
}


