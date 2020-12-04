#include "dynarray.h"
#include "util.h"
#include "ptest.h"
#include "pio.h"

typedef pCreateDynArray(, int) IntVector;

#define PLATFORM_DEBUG_BREAK __debugbreak()

int main(void) {
    P_TEST_INIT(dynarray);

    IntVector vector = { 0 };
    int *value = pPushBack(&vector, 1);
    P_SUCCESS(vector.data[0] == 1, "pPushBack failed");
    P_SUCCESS(*value == 1,         "pPushBack failed");

    value = pPushBack(&vector, 2);
    P_SUCCESS(vector.data[1] == 2, "pPushBack failed");
    P_SUCCESS(*value == 2,         "pPushBack failed");

    value = pPushBack(&vector, 3);
    P_SUCCESS(vector.data[2] == 3, "pPushBack failed");
    P_SUCCESS(*value == 3,         "pPushBack failed");

    value = pPushBack(&vector, 4);
    P_SUCCESS(vector.data[3] == 4, "pPushBack failed");
    P_SUCCESS(*value == 4,         "pPushBack failed");
    
    int *insert = pBegin(&vector);
    P_SUCCESS(insert == vector.data, "pBegin did not return start of array");

    insert++;
    insert = pInsert(&vector, insert, 6);
    P_SUCCESS(vector.data[1] == 6, "data was not inserted correctly");


    for (int *it = pBegin(&vector); it != pEnd(&vector); it++) {
        printf("%p: %i\n", it, *it);
    }
    
    int v = pRemove(&vector, insert);
    P_FAIL(vector.data[1] == 6, "data was not removed");

    pDynArrayFree((DynArray *)&vector);
    P_SUCCESS(vector.data == NULL, "array was not freed");
}


