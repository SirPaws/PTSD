#include "dynarray.h"
#include "util.h"

typedef pCreateDynArray(, int) IntVector;

int main(void) {
    IntVector vector = { 0 };
    pPushBack(&vector, 1);
    pPushBack(&vector, 2);
    pPushBack(&vector, 3);
    pPushBack(&vector, 4);
    
    int *insert = pBegin(&vector);
    insert++;
    insert = pInsert(&vector, insert, 6);

    for (int *it = pBegin(&vector); it != pEnd(&vector); it++) {
        printf("%p: %i\n", it, *it);
    }
    
    int v = pRemove(&vector, insert);

    pDynArrayFree((DynArray *)&vector);
}


