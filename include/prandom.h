

#include "general.h"

#if !defined(PSTD_NO_INTRINSICS)
// due to clang being fucking stupid this header is basically useless
#include <immintrin.h>
PSTD_UNUSED static inline bool phardware_seed_s16(s16 *memory);
PSTD_UNUSED static inline bool phardware_seed_u16(u16 *memory);

PSTD_UNUSED static inline bool phardware_seed_s32(s32 *memory);
PSTD_UNUSED static inline bool phardware_seed_u32(u32 *memory);

PSTD_UNUSED static inline bool phardware_seed_s64(s64 *memory);
PSTD_UNUSED static inline bool phardware_seed_u64(u64 *memory);

PSTD_UNUSED static inline bool phardware_random_s16(s16 *memory);
PSTD_UNUSED static inline bool phardware_random_u16(u16 *memory);

PSTD_UNUSED static inline bool phardware_random_s32(s32 *memory);
PSTD_UNUSED static inline bool phardware_random_u32(u32 *memory);

PSTD_UNUSED static inline bool phardware_random_s64(s64 *memory);
PSTD_UNUSED static inline bool phardware_random_u64(u64 *memory);
#endif

// void pSetGenerator(pRandomNumberGeneratorKind);

void pseed(u64 value);

u64 prandom_u64(void);
u32 prandom_u32(void);
u16 prandom_u16(void);
u8  prandom_u8 (void);

s64 prandom_s64(void);
s32 prandom_s32(void);
s16 prandom_s16(void);
s8  prandom_s8 (void);


// the bigger PSTD_RNG_SIZE the more 'random' it will be
// for small values 16 seems to be good enough to give 
// random result with a small'ish memory footprint
#ifndef PSTD_RNG_SIZE
#define PSTD_RNG_SIZE 512
#endif

#define PSTD_MAX_VALUE (0XFFFFF00000000000LLU)

typedef struct prandom_device_t prandom_device_t;
struct prandom_device_t {
    // if seed is 0 a new seed will be generated with `rand()`
    u64 seed;
    const u64 size;
    u64 state[PSTD_RNG_SIZE];
};

void pinitialize_state_array(usize count, u64 buffer[count]);

u64 prd_random_u64(prandom_device_t *);
u32 prd_random_u32(prandom_device_t *);
u16 prd_random_u16(prandom_device_t *);
u8  prd_random_u8 (prandom_device_t *);

s64 prd_random_s64(prandom_device_t *);
s32 prd_random_s32(prandom_device_t *);
s16 prd_random_s16(prandom_device_t *);
s8  prd_random_s8 (prandom_device_t *);


































#if !defined(PSTD_NO_INTRINSICS)
PSTD_UNUSED 
static inline bool pHardwareSeedS16(s16 *memory) {
    return (bool)_rdseed16_step((u16*)memory);
}
PSTD_UNUSED static inline bool pHardwareSeedU16(u16 *memory) {
    return (bool)_rdseed16_step(memory);
}

PSTD_UNUSED static inline bool pHardwareSeedS32(s32 *memory) {
    return (bool)_rdseed32_step((u32*)memory);
}

PSTD_UNUSED static inline bool pHardwareSeedU32(u32 *memory) {
    return (bool)_rdseed32_step(memory);
}

PSTD_UNUSED static inline bool pHardwareSeedS64(s64 *memory) {
    return (bool)_rdseed64_step((u64*)memory);
}

PSTD_UNUSED static inline bool pHardwareSeedU64(u64 *memory) {
    return (bool)_rdseed64_step(memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomS16(s16 *memory) {
    return (bool)_rdrand16_step((u16*)memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomU16(u16 *memory) {
    return (bool)_rdrand16_step(memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomS32(s32 *memory) {
    return (bool)_rdrand32_step((u32*)memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomU32(u32 *memory) {
    return (bool)_rdrand32_step(memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomS64(s64 *memory) {
    return (bool)_rdrand64_step((u64*)memory);
}

PSTD_UNUSED
static inline bool pHardwareRandomU64(u64 *memory) {
    return (bool)_rdrand64_step(memory);
}
#endif 



