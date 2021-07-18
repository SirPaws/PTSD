#include "general.h"
#include "hash_map.h"

#define PSTD_PSTRING_IMPLEMENTATION
#include "pstring.h"




usize StringHash(HashMap *, void *ptr) {
    String *str = ptr;
    usize hash = pHashObject((usize)str, str, sizeof(String));
    return hash;
}

pBool KeyCmp(HashMap *, void *a, void *b) {
    return pStringCmp(*(String*)a, *(String*)b);
}

void KeyDeleteFunc(void *key) {
    String *str = key;
    printf("Deleting key '%s'\n", str->c_str);
}
void ValueDeleteFunc(void *key) {
    int *i = key;
    printf("Deleting value '%i'\n", *i);
}

int main(void) {
    int *map = pCreateHashMap(int, String, KeyCmp);  
    // int *map = pCreateHashMapAdv(int, String, StringHash, KeyCmp, PSTD_DEFAULT_ALLOCATOR);
    
    pSetKeyCompareFunc(map, KeyCmp);
    pSetHashFunc(map, pDefaultHashFunc);

    pSetFreeFunc(map, ValueDeleteFunc);
    pSetKeyFreeFunc(map, KeyDeleteFunc);

    String hello    = pCreateString("hello");
    String equal    = pCreateString("equal");
    String test_key = pCreateString("test key");
    String uwu      = pCreateString("uwu herrow");

    pInsert(map, hello, 55);
    pInsert(map, equal, 43);

    map[pIndex(map, uwu)] = 99;

    printf("\"hello\": %lli: %i\n", pIndex(map, hello), map[pIndex(map, hello)]);
    printf("\"equal\": %lli: %i\n", pIndex(map, equal), map[pIndex(map, equal)]);

    for (int *i = pBegin(map); i != pEnd(map); i++) {
        String *key = pGetKey(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }

    int value = pRemoveKey(map, equal);
    printf("removed value: %i\n", value);
    pShrinkHashMap(map);

    for (int *i = pBegin(map); i != pEnd(map); i++) {
        String *key = pGetKey(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }
    printf("\n");

    
    pInsert(map, equal, 55);
    pInsert(map, test_key, 55);
    for (int *i = pBegin(map); i != pEnd(map); i++) {
        String *key = pGetKey(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }
    
    if (pContains(map, uwu))
        printf("this is an error the hashmap contains the key 'uwu herrow'\n");
    if (pContains(map, test_key))
        printf("the map contains 'test key'!\n");

    pFreeHashMap(map);
}


