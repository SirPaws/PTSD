#include "general.h"
#include "allocator.h"

struct LargeStruct {
    u32 a, b, c, d;
};

int main(void) {
    int arr[10] = { 0 };
    pMemExpand(arr, &(struct LargeStruct){1234, 4321, 1234, 1111}, countof(arr), sizeof(struct LargeStruct));

    for (u32 i = 0; i < 10; i++){
        printf("%i\n", arr[i]);
    }
    return 0;
}
