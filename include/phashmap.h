#pragma once
#include "general.h"

#if PLANG64 
#define DEFAULT_HASH_FUNCTION pMurmurHash64A
#elif PLANG32 
#define DEFAULT_HASH_FUNCTION pMurmurHash2A
#endif

#define DEFAULT_KEY_COMPARE pDataCompare 
#define DEFAULT_BUCKET_COUNT 32

typedef struct HashMapKey HashMapKey;
typedef usize HashFunc(HashMapKey key, usize seed);
typedef bool KeyCompareFunc(const HashMapKey a, const HashMapKey b);
typedef struct HashMapInfo HashMapInfo;
typedef struct HashMap HashMap;

struct HashMapKey {
    void *key;
    usize length;
};

struct HashMapInfo {
    HashFunc *hashfunction;
    KeyCompareFunc *Keycomparisonfunction;
    usize datasize;
    
    usize numbuckets;
};


HashMap *pInitHashMap(HashMapInfo info);
void pFreeHashMap(HashMap *map);
bool pHashMapHasKey(HashMap *map, const HashMapKey key);
void *pHashMapFind(HashMap *map, const HashMapKey key);

// returns the value at key
void *pHashMapInsert(HashMap *map, const HashMapKey key, void *value);
bool pHashMapRemove(HashMap *map, const HashMapKey key, void *output);

usize pMurmurHash2A(HashMapKey key,  usize seed);
usize pMurmurHash64A(HashMapKey key, usize seed);

// checks if both keys have the same size
// if they do it then does calls memcmp
bool pDataCompare(const HashMapKey a, const HashMapKey b);

