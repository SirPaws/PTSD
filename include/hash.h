#pragma once

#ifndef PTSD_HASH_HEADER
#define PTSD_HASH_HEADER
#ifndef PTSD_HASH_STANDALONE
#include "general.h"
#else
#error not implemented yet
#endif

#if defined(PTSD_MSVC) || (__clang__ && _WIN32)

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

// Other compilers
#else	// defined(_MSC_VER)

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

#if defined(__cplusplus)
extern "C" {
#endif

static PTSD_FORCE_INLINE
u32 pfmix32(u32 h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

//----------

static PTSD_FORCE_INLINE
u64 pfmix64(u64 k) {
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;

  return k;
}

PTSD_UNUSED
static inline usize phash(const void *key, isize length, usize seed) {//NOLINT
    assert(length > 0);
    if (sizeof(usize) == 4) {
        // use 32 bit version of murmurhash3
        
        const u8 *data = (const u8*)key;
        const isize nblocks = length / 4;
        
        u32 h1 = seed;

        const u32 c1 = 0xcc9e2d51;
        const u32 c2 = 0x1b873593;
        
        const u32 *blocks = (const u32*)(data + nblocks*4LLU);

        for (isize i = -nblocks; i; i++) {
            u32 k1 = blocks[i];

            k1 *= c1;
            k1 = ROTL32(k1, 15);
            k1 *= c2;

            h1 ^= k1;
            h1 = ROTL32(h1, 13);
            h1 = h1 * 5 + 0xe6546b64;
        }
        
        const u8 *tail = (const u8*)(data + nblocks*4LLU);
        
        u32 k1 = 0;
        
        switch(length & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8; 
        case 1: k1 ^= tail[0] << 0;
                k1 *= c1; 
                k1 = ROTL32(k1, 15);
                k1 *= c2;
                h1 ^=k1;
        }
        h1 ^= length;
        
        h1 = pfmix32(h1);
        
        return h1;
    }
    
    assert(sizeof(usize) == 8);
    const u8 *data = (const u8*)key;
    const isize nblocks = length / 16;

    u64 h1 = seed;
    u64 h2 = seed;

    const u64 c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const u64 c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    const u64 *blocks = (const u64*)(data);

    for (int i = 0; i < nblocks; i++) {
        u64 k1 = blocks[i * 2 + 0];
        u64 k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1 = ROTL64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1 = ROTL64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = ROTL64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2 = ROTL64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    const u8 *tail = (const u8*)(data + nblocks * 16);

    u64 k1 = 0;
    u64 k2 = 0;

    switch (length & 15) {
    case 15: k2 ^= ((u64)tail[14]) << 48;
    case 14: k2 ^= ((u64)tail[13]) << 40;
    case 13: k2 ^= ((u64)tail[12]) << 32;
    case 12: k2 ^= ((u64)tail[11]) << 24;
    case 11: k2 ^= ((u64)tail[10]) << 16;
    case 10: k2 ^= ((u64)tail[9])  <<  8;
    case 9:  k2 ^= ((u64)tail[8])  <<  0;
             k2 *= c2;
             k2 = ROTL64(k2, 33);
             k2 *= c1;
             h2 ^= k2;
    case 8:  k1 ^= ((u64)tail[7]) << 56;
    case 7:  k1 ^= ((u64)tail[6]) << 48;
    case 6:  k1 ^= ((u64)tail[5]) << 40;
    case 5:  k1 ^= ((u64)tail[4]) << 32;
    case 4:  k1 ^= ((u64)tail[3]) << 24;
    case 3:  k1 ^= ((u64)tail[2]) << 16;
    case 2:  k1 ^= ((u64)tail[1]) <<  8;
    case 1:  k1 ^= ((u64)tail[0]) <<  0;
             k1 *= c1;
             k1 = ROTL64(k1, 31);
             k1 *= c2;
             h1 ^= k1;
    };

    h1 ^= length;
    h2 ^= length;

    h1 += h2;
    h2 += h1;

    h1 = pfmix64(h1);
    h2 = pfmix64(h2);

    h1 += h2;
    h2 += h1;

    return h1;
}

#if defined(__cplusplus)
}
#endif
#endif // PTSD_HASH_HEADER
