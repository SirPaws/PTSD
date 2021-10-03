#include "ptime.h"

#if defined(PSTD_WINDOWS)
#include <Windows.h>
#elif defined(PSTD_LINUX) || defined(PSTD_WASM)
#include <time.h>
#endif

// ported from _Xtime_get_ticks: https://github.com/microsoft/STL/blob/master/stl/src/xtime.cpp
ptimepoint_t psystem_time(void) {
#if defined(PSTD_WINDOWS)
#define COMBINE(high, low) (((s64)(high) << 32) | (s64)(low))
    static const long long PSTD_EPOCH = 0x19DB1DED53E8000LL; 
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    return COMBINE(ft.dwHighDateTime,ft.dwLowDateTime) - PSTD_EPOCH;
#undef COMBINE
#else
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec * PSTD_TIME_SECONDS + tp.tv_nsec;
#endif
}
// these clocks are taken from MSVC-STL
// the only edit is the names of the variables have been changed to all lowercase
// because the _Ugly syntax is reserved.
// Can be found here https://github.com/microsoft/STL  (2021)
ptimepoint_t pget_tick(enum pclocktype_t type) {
#if defined(PSTD_WINDOWS)
    if (PSTD_EXPECT(type != PSTD_SYSTEM_CLOCK, 1)) {
        const s64 _freq; QueryPerformanceFrequency((void *)&_freq); // doesn't change after system boot
        const s64 _ctr;  QueryPerformanceCounter((void*)&_ctr);
        // Instead of just having "(_Ctr * period::den) / _Freq",
        // the algorithm below prevents overflow when _Ctr is sufficiently large.
        // It assumes that _Freq * period::den does not overflow, which is currently true for nano period.
        // It is not realistic for _Ctr to accumulate to large values from zero with this assumption,
        // but the initial value of _Ctr could be large.
        const s64 _whole = (_ctr / _freq) * 1000000000;
        const s64 _part = (_ctr % _freq) * 1000000000 / _freq;
        return _whole + _part;
    } else {
        return psystem_time();
    }
#elif defined(PSTD_LINUX) || defined(PSTD_WASM)
    struct timespec tp; 
    switch (type) {
    case PSTD_STEADY_CLOCK: {
            clock_gettime(CLOCK_MONOTONIC, &tp);
            return tp.tv_sec * PSTD_TIME_SECONDS + tp.tv_nsec;
        }
    case PSTD_SYSTEM_CLOCK:
    case PSTD_HIGH_RESOLUTION_CLOCK: return psystem_time();
    }
#else
    return 0;
#endif
}
