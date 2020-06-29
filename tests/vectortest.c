
#include "vector.h"

typedef pCreateVectorStruct(iVector, int) iVector;

int main(void) {
    iVector *vector = (void *)pInitVector((VectorInfo){ .datasize = sizeof(int), .initialsize = 2}); 
    int *pos = pPushBack((void *)&vector, &(int){32});
    int *second = pInsert((void *)&vector, pos, &(int){55});

    usize count = pSize((void *)vector);
    printf("vector holds %llu elements\n", count);

    for (int *it = pBegin((void *)vector); it != pEnd((void *)vector); it++){
        printf("%p holds %i\n", (void *)it, *it);
    }

    pErase((void *)vector, second);
    count = pSize((void *)vector);
    printf("vector holds %llu elements\n", count);

    pFreeVector((void *)vector);

    int *test = malloc(sizeof(int));
    *test = 0xbad;
    printf("%p holds 0x%x", (void *)test, *test);
    free(test);

    return 0;
}



