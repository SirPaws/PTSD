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
typedef bool KeyCompareFunc(HashMapKey a, HashMapKey b);
typedef struct HashMapInfo HashMapInfo;
typedef struct HashMap HashMap;

struct HashMapKey {
    const void *key;
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
bool pHashMapHasKey(HashMap *map, HashMapKey key);
void *pHashMapFind(HashMap *map, HashMapKey key);

// both HashMapRemove and HashMapInsert returns the value at key
void *pHashMapRemove(HashMap *map, HashMapKey key);
void *pHashMapInsert(HashMap *map, HashMapKey key, void *value);

usize pMurmurHash2A(HashMapKey key,  usize seed);
usize pMurmurHash64A(HashMapKey key, usize seed);

// checks if both keys have the same size
// if they do it then does calls memcmp
bool pDataCompare(HashMapKey a, HashMapKey b);










