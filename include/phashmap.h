#pragma once
#ifndef PSTD_PHASHMAP_HEADER
#define PSTD_PHASHMAP_HEADER
#include "general.h"

#if PLANG64 
#define DEFAULT_HASH_FUNCTION pMurmurHash64A
#elif PLANG32 
#define DEFAULT_HASH_FUNCTION pMurmurHash2A
#endif

#define DEFAULT_KEY_COMPARE pDataCompare 
#define DEFAULT_BUCKET_COUNT 32
#define DEFAULT_KEY_SIZE (sizeof(HashMapKey))

typedef struct HashMapGenericKey HashMapGenericKey;
typedef usize HashFunc(const HashMapGenericKey key, usize seed);
typedef bool KeyCompareFunc(const HashMapGenericKey a, const HashMapGenericKey b);
typedef struct HashMapInfo HashMapInfo;
typedef struct HashMap HashMap;
typedef struct HashMapKey HashMapKey;

struct HashMapGenericKey {
    void *key;
};

struct HashMapKey {
    void *key;
    usize length;
};

struct HashMapInfo {
    HashFunc *hashfunction;
    KeyCompareFunc *Keycomparisonfunction;
    usize keysize;
    usize datasize;
    
    usize numbuckets;
};


HashMap *pInitHashMap(HashMapInfo info);
void pFreeHashMap(HashMap *map);
bool pHashMapHasKey(HashMap *map, const HashMapGenericKey key);
void *pHashMapFind(HashMap *map, const HashMapGenericKey key);

// returns the value at key
void *pHashMapInsert(HashMap *map, const HashMapGenericKey key, void *value);
bool pHashMapRemove(HashMap *map, const HashMapGenericKey key, void *output);

usize pMurmurHash2A(HashMapGenericKey key,  usize seed);
usize pMurmurHash64A(HashMapGenericKey key, usize seed);

// checks if both keys have the same size
// if they do it then does calls memcmp
bool pDataCompare(const HashMapGenericKey a, const HashMapGenericKey b);

#endif // PSTD_PHASHMAP_HEADER
