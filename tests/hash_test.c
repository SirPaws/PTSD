#include "phashmap.h"

int main(void) {
    struct HashMapInfo info = {
    .hashfunction = DEFAULT_HASH_FUNCTION,
    .Keycomparisonfunction = DEFAULT_KEY_COMPARE,
    .keysize  = DEFAULT_KEY_SIZE,
    .datasize = sizeof(int),
    
    .numbuckets = DEFAULT_BUCKET_COUNT,
    };
    
    HashMap *map = pInitHashMap(info);
    pHashMapInsert(map, (HashMapGenericKey){ &(HashMapKey){ "hello", sizeof("hello") - 1 }}, &(int){ 55 });
    pHashMapInsert(map, (HashMapGenericKey){ &(HashMapKey){ "sir", sizeof("sir") - 1 }}, &(int){ 33 });
    
    int *value = (int *)(pHashMapFind(map, (HashMapGenericKey){ &(HashMapKey){ "hello", sizeof("hello") - 1 }}));
    printf("key 'hello' holds int with value: %i\n", value ? *value : 0);
    int removed = 0;
    pHashMapRemove(map, (HashMapGenericKey){ &(HashMapKey){ "sir", sizeof("sir") - 1 }}, &removed);
    printf("key 'sir' has been deleted it held an integer with value: %i\n", removed);
    if (pHashMapRemove(map, (HashMapGenericKey){ &(HashMapKey){ "sir", sizeof("sir") - 1 }}, &removed))
         printf("key 'sir' has been deleted it held an integer with value: %i\n", removed);
    else printf("key 'sir' has alread been deleted");

    pFreeHashMap(map);
    return 0;
}

