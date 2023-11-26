#pragma once

#ifndef PTSD_TIME_HEADER
#define PTSD_TIME_HEADER
#ifndef PTSD_TIME_STANDALONE
#include "general.h"
#else
#error not implemented yet
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef s64 ptimepoint_t;

enum pclocktype_t {
    PTSD_STEADY_CLOCK,
    PTSD_HIGH_RESOLUTION_CLOCK,
    PTSD_SYSTEM_CLOCK
};

ptimepoint_t pget_tick(enum pclocktype_t);
ptimepoint_t psystem_time(void); // same as pget_tick with PTSD_SYSTEM_CLOCK

#define pget_time_nano_seconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pget_time_nano_d, \
        f32: pget_time_nano_f,\
        s8:  pget_time_nano_lls,\
        s16: pget_time_nano_lls,\
        s32: pget_time_nano_lls, \
        s64: pget_time_nano_lls,\
        u8:  pget_time_nano_llu, \
        u16: pget_time_nano_llu, \
        u32: pget_time_nano_llu, \
        u64: pget_time_nano_llu\
    )(_Generic(timepoint, ptimepoint_t: pget_timepoint_from_timepoint)(timepoint))

#define pget_time_mili_seconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pget_time_mili_d, \
        f32: pget_time_mili_f,\
        s8:  pget_time_mili_lls, \
        s16: pget_time_mili_lls, \
        s32: pget_time_mili_lls, \
        s64: pget_time_mili_lls,\
        u8:  pget_time_mili_llu, \
        u16: pget_time_mili_llu, \
        u32: pget_time_mili_llu, \
        u64: pget_time_mili_llu\
    )(_Generic(timepoint, ptimepoint_t: pget_timepoint_from_timepoint)(timepoint))

#define pget_time_seconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pget_time_seconds_d, \
        f32: pget_time_seconds_f,\
        s8:  pget_time_seconds_lls,\
        s16: pget_time_seconds_lls,\
        s32: pget_time_seconds_lls, \
        s64: pget_time_seconds_lls,\
        u8:  pget_time_seconds_llu, \
        u16: pget_time_seconds_llu, \
        u32: pget_time_seconds_llu, \
        u64: pget_time_seconds_llu\
    )(_Generic(timepoint, ptimepoint_t: pget_timepoint_from_timepoint)(timepoint))

#define PTSD_TIME_MILI    1000000
#define PTSD_TIME_SECONDS 1000000000

PTSD_UNUSED
static inline ptimepoint_t pget_timepoint_from_timepoint(ptimepoint_t point) {
    return point;
}

PTSD_UNUSED
static inline f64 pget_time_nano_d(ptimepoint_t timepoint) { 
    return (f64) timepoint; 
}

PTSD_UNUSED
static inline f32 pget_time_nano_f(ptimepoint_t timepoint) { 
    return (f32) timepoint; 
}

PTSD_UNUSED
static inline s64 pget_time_nano_lls(ptimepoint_t timepoint) { 
    return (s64) timepoint; 
}

PTSD_UNUSED
static inline u64 pget_time_nano_llu(ptimepoint_t timepoint) { 
    return (u64) timepoint; 
}

PTSD_UNUSED
static inline f64 pget_time_mili_d(ptimepoint_t timepoint) { 
    return (f64)timepoint / (f64) PTSD_TIME_MILI; 
}

PTSD_UNUSED
static inline f32 pget_time_mili_f(ptimepoint_t timepoint) { 
    return (f32)timepoint / (f32) PTSD_TIME_MILI; 
}

PTSD_UNUSED
static inline s64 pget_time_mili_lls(ptimepoint_t timepoint) { 
    return (s64)timepoint / (s64) PTSD_TIME_MILI; 
}

PTSD_UNUSED
static inline u64 pget_time_mili_llu(ptimepoint_t timepoint) { 
    return (u64)timepoint / (u64) PTSD_TIME_MILI; 
}

PTSD_UNUSED
static inline f64 pget_time_seconds_d(ptimepoint_t timepoint) { 
    return (f64)timepoint  / (f64) PTSD_TIME_SECONDS; 
}

PTSD_UNUSED
static inline f32 pget_time_seconds_f(ptimepoint_t timepoint) { 
    return (f32)timepoint  / (f32) PTSD_TIME_SECONDS; 
}

PTSD_UNUSED
static inline s64 pget_time_seconds_lls(ptimepoint_t timepoint) { 
    return (s64)timepoint  / (s64) PTSD_TIME_SECONDS; 
}

PTSD_UNUSED
static inline u64 pget_time_seconds_llu(ptimepoint_t timepoint) { 
    return (u64)timepoint  / (u64) PTSD_TIME_SECONDS; 
}

#if defined(__cplusplus)
}
#endif
#endif // PTSD_TIME_HEADER
