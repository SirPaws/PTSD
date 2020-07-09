#include "general.h"
#include "allocator.h"

int main(void) {
    int arr[10] = { 0 };
    memexpand(arr, &(struct LargeStruct){1234, 4321, 1234, 1111}, sizeof(int) * 10, sizeof(struct LargeStruct));

    for (u32 i = 0; i < 10; i++){
        printf("%i??/n", arr[i]);
    }
    return 0;
}
