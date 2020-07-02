#include "phashmap.h"
#include "allocator.h"
#include "util.h"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

typedef struct HashMapData HashMapData;
struct HashMapData {
    HashMapKey key;
    void *data;
};

typedef struct HashMapBucket HashMapBucket;
struct HashMapBucket {
    usize count;
    HashMapData *data;
};

static pArrayGenPushBack(HashMapBucketPush, HashMapBucket, HashMapData)

struct HashMap {
    HashFunc *hash;
    KeyCompareFunc *cmp;
    usize datasize;
    usize numbuckets;
    struct HashMapBucket *buckets;
};

HashMap *pInitHashMap(HashMapInfo info) {
    HashMap *map = pCurrentAllocatorFunc(NULL, sizeof *map, 0, MALLOC, pCurrentAllocatorUserData);
    map->hash       = info.hashfunction;
    map->cmp        = info.Keycomparisonfunction;
    map->datasize   = info.datasize;
    map->numbuckets = info.numbuckets;
    map->buckets    = pCurrentAllocatorFunc(NULL, 
            (sizeof *map->buckets), map->numbuckets, ARRAYALLOC, pCurrentAllocatorUserData); 
    memset(map->buckets, 0, (sizeof *map->buckets) * map->numbuckets);
    return map;
}

static void pFreeBucket(HashMapBucket *bucket) {
    pCurrentAllocatorFunc(bucket->data, 0, 0, FREE, pCurrentAllocatorUserData);
}

void pFreeHashMap(HashMap *map) {
    assert(map);
    for (usize i = 0; i < map->numbuckets; i++) {
        if (map->buckets[i].count != 0) pFreeBucket(map->buckets + i); 
    }
    pCurrentAllocatorFunc(map->buckets, 0, 0, FREE, pCurrentAllocatorUserData);
    pCurrentAllocatorFunc(map, 0, 0, FREE, pCurrentAllocatorUserData);
}

void *pHashMapFind(HashMap *map, const HashMapKey key) {
    assert(map);
    usize index = map->hash(key, map->numbuckets);
    HashMapBucket *bucket = map->buckets + (index % map->numbuckets);
    if (bucket->count == 0) return NULL;
    for (u32 i = 0; i < bucket->count; i++) {
        if (map->cmp(bucket->data[i].key, key)) {
            return bucket->data[i].data;
        }
    }
    return NULL;
    
}

bool pHashMapHasKey(HashMap *map, const HashMapKey key) {
    assert(map);
    usize index = map->hash(key, map->numbuckets);
    HashMapBucket *bucket = map->buckets + (index % map->numbuckets);
    if (bucket->count == 0) return false;
    for (u32 i = 0; i < bucket->count; i++) {
        if (map->cmp(bucket->data[i].key, key)) return true;
    }
    return false;
}

void pBucketRemoveFirst(HashMapBucket *bucket);
void pBucketRemoveMiddle(HashMapBucket *bucket, usize index);
// both HashMapRemove and HashMapInsert returns the value at key
void *pHashMapRemove(HashMap *map, const HashMapKey key) {
    assert(map);
    usize index = map->hash(key, map->numbuckets);
    HashMapBucket *bucket = map->buckets + (index % map->numbuckets);
    if (bucket->count == 0) return NULL;
    for (u32 i = 0; i < bucket->count; i++) {
        if (map->cmp(bucket->data[i].key, key)) {
            void *data = bucket->data[i].data;
            if (i == 0) {
                if (bucket->count > 1) pBucketRemoveFirst(bucket);
                else { pFreeBucket(bucket); bucket->count = 0; }
            }
            else if (i < (bucket->count - 1) ){
                pBucketRemoveMiddle(bucket, i);
            }
            else {
                void *tmp = pCurrentAllocatorFunc(bucket->data, 
                        sizeof(*bucket->data) * (bucket->count - 1), 
                        0, REALLOC, pCurrentAllocatorUserData);
                bucket->data = tmp;
            }
            return data;
        }
    }
    return NULL;
}

void *pHashMapInsert(HashMap *map, const HashMapKey key, void *value) {
    assert(map);
    usize index = map->hash(key, map->numbuckets);
    HashMapBucket *bucket = map->buckets + (index % map->numbuckets);
    HashMapBucketPush(bucket, (HashMapData){ key, value });
    return (bucket->data + (bucket->count - 1))->data;
}

#if PLANG64
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wcast-align"
#endif

#define mmix(h,k) do { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; } while(0)
usize pMurmurHash2A(HashMapKey key, usize seed) {
  const u32 m = 0x5bd1e995;
  const int r = 24;
  u32 l = (u32)key.length;

  const unsigned char * data = (const unsigned char *)key.key;

  u32 h = (u32)seed;

  while(key.length >= 4)
  {
    uint32_t k = *(u32*)data;

    mmix(h,k);

    data += 4;
    key.length -= 4;
  }

  uint32_t t = 0;

  switch(key.length)
  {
  case 3: t ^= (unsigned)data[2] << 16; [[fallthrough]]; 
  case 2: t ^= (unsigned)data[1] << 8;  [[fallthrough]];
  case 1: t ^= data[0];
  }

  mmix(h,t);
  mmix(h,l);

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}
#if PLANG64
#pragma clang diagnostic pop
#endif

usize pMurmurHash64A(HashMapKey key, usize seed) {
  const uint64_t m = 0xc6a4a7935bd1e995LLU;
  const int r = 47;

  uint64_t h = seed ^ (key.length * m);

  const uint64_t * data = (const uint64_t *)key.key;
  const uint64_t * end = data + (key.length/8);

  while(data != end)
  {
    uint64_t k = *data++;

    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h ^= k;
    h *= m; 
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch(key.length & 7)
  {
  case 7: h ^= (u64)(data2[6]) << 48; [[fallthrough]];
  case 6: h ^= (u64)(data2[5]) << 40; [[fallthrough]];
  case 5: h ^= (u64)(data2[4]) << 32; [[fallthrough]];
  case 4: h ^= (u64)(data2[3]) << 24; [[fallthrough]];
  case 3: h ^= (u64)(data2[2]) << 16; [[fallthrough]];
  case 2: h ^= (u64)(data2[1]) << 8;  [[fallthrough]];
  case 1: h ^= (u64)(data2[0]);
          h *= m;
  }
 
  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

bool pDataCompare(const HashMapKey a, const HashMapKey b) {
    if (a.length != b.length) return false;
    return memcmp(a.key, b.key, b.length) == 0;
}




void pBucketRemoveFirst(HashMapBucket *bucket) {
    void *tmp = pCurrentAllocatorFunc(NULL, 
            sizeof(*bucket->data) * bucket->count - 1, 0, MALLOC, pCurrentAllocatorUserData);
    memcpy(tmp, bucket->data + 1, sizeof(*bucket->data) * bucket->count - 1);
    memcpy(bucket->data, tmp,  sizeof(*bucket->data) * bucket->count - 1);
    bucket->count--;
}

void pBucketRemoveMiddle(HashMapBucket *bucket, usize index) {
    usize size = bucket->count - (index + 1);

    void *tmp = pCurrentAllocatorFunc(NULL, 
            sizeof(*bucket->data) * size, 0, MALLOC, pCurrentAllocatorUserData);
    memcpy(tmp, bucket->data + (index + 1), sizeof(*bucket->data) * size);
    memcpy(bucket->data + index, tmp, sizeof(*bucket->data) * size);
    bucket->count--;
}

