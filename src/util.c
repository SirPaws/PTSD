#include "util.h"
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void array_grow(struct GenericArray *arr, u64 size){
    void *tmp = pCurrentAllocatorFunc(arr->data, 
            (arr->count + 1) * size, 0, REALLOC, pCurrentAllocatorUserData);
    if (!tmp) { assert(true); return; }
    arr->data = tmp;
    ++arr->count;
}

void *pGetPlatformHandle(void) {
#if defined(_WIN32) || defined(_WIN64)
    void *handle = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned int mode;
    GetConsoleMode(handle, (unsigned long *)&mode);
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return handle; 
#else
    //TODO: other platforms
    return NULL;
#endif
}
