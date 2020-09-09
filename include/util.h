#pragma once
#ifndef PSTD_UTIL_HEADER
#define PSTD_UTIL_HEADER

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

typedef s64 pTimePoint;

enum pClockType {
    PSTD_STEADY_CLOCK,
    PSTD_HIGH_RESOLUTION_CLOCK,
    PSTD_SYSTEM_CLOCK
};

pTimePoint pGetTick(enum pClockType);

#define pGetTimeNanoSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pGetTimeNanod, f32: pGetTimeNanof,\
        s8: pGetTimeNanolls,s16: pGetTimeNanolls,s32: pGetTimeNanolls, s64: pGetTimeNanolls,\
        u8: pGetTimeNanollu, u16: pGetTimeNanollu, u32: pGetTimeNanollu, u64: pGetTimeNanollu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

#define pGetTimeMiliSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f64: pGetTimeMilid, f32: pGetTimeMilif,\
        s8: pGetTimeMilills, s16: pGetTimeMilills, s32: pGetTimeMilills, s64: pGetTimeMilills,\
        u8: pGetTimeMilillu, u16: pGetTimeMilillu, u32: pGetTimeMilillu, u64: pGetTimeMilillu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

#define pGetTimeSeconds(rettype, timepoint) \
    (rettype)_Generic((rettype)0, \
        f128: pGetTimeSecondsld, f64: pGetTimeSecondsd, f32: pGetTimeSecondsf,\
        s8: pGetTimeSecondslls,s16: pGetTimeSecondslls,s32: pGetTimeSecondslls, s64: pGetTimeSecondslls,\
        u8: pGetTimeSecondsllu, u16: pGetTimeSecondsllu, u32: pGetTimeSecondsllu, u64: pGetTimeSecondsllu\
    )(_Generic(timepoint, pTimePoint: pGetTimePointFromTimePoint)(timepoint))

static inline pTimePoint pGetTimePointFromTimePoint(pTimePoint point) { return point; }

static inline f64 pGetTimeNanod(pTimePoint timepoint)   { return (f64) timepoint; }
static inline f32 pGetTimeNanof(pTimePoint timepoint)   { return (f32) timepoint; }
static inline s64 pGetTimeNanolls(pTimePoint timepoint) { return (s64) timepoint; }
static inline u64 pGetTimeNanollu(pTimePoint timepoint) { return (u64) timepoint; }

static inline f64 pGetTimeMilid(pTimePoint timepoint)   { return (f64)timepoint  / (f64) 1000000; }
static inline f32 pGetTimeMilif(pTimePoint timepoint)   { return (f32)timepoint  / (f32) 1000000; }
static inline s64 pGetTimeMilills(pTimePoint timepoint) { return (s64)timepoint  / (s64) 1000000; }
static inline u64 pGetTimeMilillu(pTimePoint timepoint) { return (u64)timepoint  / (u64) 1000000; }

static inline f64 pGetTimeSecondsd(pTimePoint timepoint)   { return (f64)timepoint  / (f64) 1000000000; }
static inline f32 pGetTimeSecondsf(pTimePoint timepoint)   { return (f32)timepoint  / (f32) 1000000000; }
static inline s64 pGetTimeSecondslls(pTimePoint timepoint) { return (s64)timepoint  / (s64) 1000000000; }
static inline u64 pGetTimeSecondsllu(pTimePoint timepoint) { return (u64)timepoint  / (u64) 1000000000; }



#endif // PLANG_UTIL_HEADER

