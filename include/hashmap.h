#pragma once
#include "general.h"
#include "pstring.h"




#define pCreateHashmap(name, TYPE)  \
struct name {                       \
    usize count;                    \
    struct name##Bucket {           \
        usize count;                \
        struct name##KeyValuePair { \
            String key;             \
            TYPE   data;            \
        } *data;                    \
    } *buckets;                     \
}

typedef pCreateHashmap(GenericHashMap, u8) GenericHashMap;

void pHashMapBucketGrow(struct GenericHashMapBucket *bucket, usize datasize);
void pHashMapPushBucket(GenericHashMap *map, usize datasize);

#define pHashMapInsert(map, value) ({\
        if ((map)->count == )\
    })

/*
 * pCreateHashmap(name, hashtype, valuetype)
 *
 *
 *
 * pCreateHashmap(,String, int) hs;
 * 
 * 
 * 
 * 
 */


