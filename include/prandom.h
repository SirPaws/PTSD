

#include "general.h"

#if !defined(PSTD_NO_INTRINSICS)
// due to clang being fucking stupid this header is basically useless
#include <immintrin.h>
PSTD_UNUSED static inline pBool pHardwareSeedS16(s16 *memory);
PSTD_UNUSED static inline pBool pHardwareSeedU16(u16 *memory);

PSTD_UNUSED static inline pBool pHardwareSeedS32(s32 *memory);
PSTD_UNUSED static inline pBool pHardwareSeedU32(u32 *memory);

PSTD_UNUSED static inline pBool pHardwareSeedS64(s64 *memory);
PSTD_UNUSED static inline pBool pHardwareSeedU64(u64 *memory);

PSTD_UNUSED static inline pBool pHardwareRandomS16(s16 *memory);
PSTD_UNUSED static inline pBool pHardwareRandomU16(u16 *memory);

PSTD_UNUSED static inline pBool pHardwareRandomS32(s32 *memory);
PSTD_UNUSED static inline pBool pHardwareRandomU32(u32 *memory);

PSTD_UNUSED static inline pBool pHardwareRandomS64(s64 *memory);
PSTD_UNUSED static inline pBool pHardwareRandomU64(u64 *memory);
#endif

// void pSetGenerator(pRandomNumberGeneratorKind);

void pSeed(u64 value);

u64 pRandomU64(void);
u32 pRandomU32(void);
u16 pRandomU16(void);
u8  pRandomU8 (void);

s64 pRandomS64(void);
s32 pRandomS32(void);
s16 pRandomS16(void);
s8  pRandomS8 (void);


// the bigger PSTD_RNG_SIZE the more 'random' it will be
// for small values 16 seems to be good enough to give 
// random result with a small'ish memory footprint
#ifndef PSTD_RNG_SIZE
#define PSTD_RNG_SIZE 512
#endif

#define PSTD_MAX_VALUE (0XFFFFF00000000000LLU)

typedef struct pRandomDevice pRandomDevice;
struct pRandomDevice {
    // if seed is 0 a new seed will be generated with `rand()`
    u64 seed;
    const u64 size;
    u64 state[PSTD_RNG_SIZE];
};

void pInitializeStateArray(usize count, u64 buffer[count]);

u64 pRDRandomU64(pRandomDevice *);
u32 pRDRandomU32(pRandomDevice *);
u16 pRDRandomU16(pRandomDevice *);
u8  pRDRandomU8 (pRandomDevice *);

s64 pRDRandomS64(pRandomDevice *);
s32 pRDRandomS32(pRandomDevice *);
s16 pRDRandomS16(pRandomDevice *);
s8  pRDRandomS8 (pRandomDevice *);


































#if !defined(PSTD_NO_INTRINSICS)
PSTD_UNUSED 
static inline pBool pHardwareSeedS16(s16 *memory) {
    return (pBool)_rdseed16_step((u16*)memory);
}
PSTD_UNUSED static inline pBool pHardwareSeedU16(u16 *memory) {
    return (pBool)_rdseed16_step(memory);
}

PSTD_UNUSED static inline pBool pHardwareSeedS32(s32 *memory) {
    return (pBool)_rdseed32_step((u32*)memory);
}

PSTD_UNUSED static inline pBool pHardwareSeedU32(u32 *memory) {
    return (pBool)_rdseed32_step(memory);
}

PSTD_UNUSED static inline pBool pHardwareSeedS64(s64 *memory) {
    return (pBool)_rdseed64_step((u64*)memory);
}

PSTD_UNUSED static inline pBool pHardwareSeedU64(u64 *memory) {
    return (pBool)_rdseed64_step(memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomS16(s16 *memory) {
    return (pBool)_rdrand16_step((u16*)memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomU16(u16 *memory) {
    return (pBool)_rdrand16_step(memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomS32(s32 *memory) {
    return (pBool)_rdrand32_step((u32*)memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomU32(u32 *memory) {
    return (pBool)_rdrand32_step(memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomS64(s64 *memory) {
    return (pBool)_rdrand64_step((u64*)memory);
}

PSTD_UNUSED
static inline pBool pHardwareRandomU64(u64 *memory) {
    return (pBool)_rdrand64_step(memory);
}
#endif 



