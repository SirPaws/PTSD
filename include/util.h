#ifndef PLANG_UTIL_HEADER
#define PLANG_UTIL_HEADER

#include "general.h"
#include "allocator.h"

struct GenericArray {
    u64 count;
    void *data;
};

void array_grow(struct GenericArray *arr, u64 size);

#define pCreateArrayType(name, datatype)    \
    struct name {                           \
        usize count;                        \
        datatype *data;                     \
    }

#define pArrayGenPushBack(name, array_type, datatype)           \
void name(array_type *arr, datatype data){                      \
    array_grow((struct GenericArray  *)arr, sizeof(datatype));  \
    arr->data[arr->count - 1] = data;                           \
} 

struct PointerArray {
    u64 count;
    u8 *data;
};

[[clang::overloadable]] [[maybe_unused]]
static void array_add(struct PointerArray *arr, void *data) {
        array_grow((struct GenericArray *)arr, sizeof(void *));
        memcpy(arr->data + (arr->count - 1), data, sizeof(void *));
}

void *pGetPlatformHandle(void);
#endif // PLANG_UTIL_HEADER

