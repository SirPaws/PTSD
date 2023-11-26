#pragma once
#ifndef PTSD_RANDOM_HEADER
#define PTSD_RANDOM_HEADER

#include "general.h"
void pseed(u64 value);

u64 prandom_u64(void);
u32 prandom_u32(void);
u16 prandom_u16(void);
u8  prandom_u8 (void);

s64 prandom_s64(void);
s32 prandom_s32(void);
s16 prandom_s16(void);
s8  prandom_s8 (void);


// the bigger PTSD_RNG_SIZE the more 'random' it will be
// for small values 16 seems to be good enough to give 
// random result with a small'ish memory footprint
#ifndef PTSD_RNG_SIZE
#define PTSD_RNG_SIZE 512
#endif

#define PTSD_MAX_VALUE (0XFFFFF00000000000LLU)

typedef struct prandom_device_t prandom_device_t;
struct prandom_device_t {
    // if seed is 0 a new seed will be generated with `rand()`
    u64 seed;
    const u64 size;
    u64 state[PTSD_RNG_SIZE];
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
#endif // PTSD_RANDOM_HEADER

