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


// ported from _Xtime_get_ticks: https://github.com/microsoft/STL/blob/master/stl/src/xtime.cpp
static const long long PSTD_EPOCH = 0x19DB1DED53E8000LL; 

pTimePoint pSystemTime(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    return (((s64)(ft.dwHighDateTime)) << 32) + (s64)(ft.dwLowDateTime) - PSTD_EPOCH;
#else
    return 0;
#endif
}
// these clocks are taken from MSVC-STL
// can be found here https://github.com/microsoft/STL
pTimePoint pGetTick(enum pClockType type) {
#if defined(_WIN32) || defined(_WIN64)
    if (__builtin_expect(type != PSTD_SYSTEM_CLOCK, 1)) {
        const s64 _Freq; QueryPerformanceFrequency((void *)&_Freq); // doesn't change after system boot
        const s64 _Ctr;  QueryPerformanceCounter((void*)&_Ctr);
        // Instead of just having "(_Ctr * period::den) / _Freq",
        // the algorithm below prevents overflow when _Ctr is sufficiently large.
        // It assumes that _Freq * period::den does not overflow, which is currently true for nano period.
        // It is not realistic for _Ctr to accumulate to large values from zero with this assumption,
        // but the initial value of _Ctr could be large.
        const s64 _Whole = (_Ctr / _Freq) * 1000000000;
        const s64 _Part = (_Ctr % _Freq) * 1000000000 / _Freq;
        return _Whole + _Part;
    } else {
        return pSystemTime();
    }
#else
    return 0;
#endif
}
