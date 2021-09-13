#include "general.h"
#include "hash_map.h"

#define PSTD_PSTRING_IMPLEMENTATION
#include "pstring.h"




usize stringhash(phashmap_t *map, void *ptr) {
    (void)map;
    pstring_t *str = ptr;
    usize hash = phs_hash_object((usize)str, str, sizeof(pstring_t));
    return hash;
}

pbool_t KeyCmp(phashmap_t *map, void *a, void *b) {
    (void)map;
    return pcmp_string(*(pstring_t*)a, *(pstring_t*)b);
}

void KeyDeleteFunc(void *key) {
    pstring_t *str = key;
    printf("Deleting key '%s'\n", str->c_str);
}
void ValueDeleteFunc(void *key) {
    int *i = key;
    printf("Deleting value '%i'\n", *i);
}

int main(void) {
    int *map = phs_create_hashmap(int, pstring_t, KeyCmp);  
    // int *map = pCreateHashMapAdv(int, pstring_t, pstring_tHash, KeyCmp, PSTD_DEFAULT_ALLOCATOR);
    
    phs_compare_func(map, KeyCmp);
    phs_hash_func(map, phs_default_hash_func);

    phs_free_func(map, ValueDeleteFunc);
    phs_freekey_func(map, KeyDeleteFunc);

    pstring_t hello    = pcreate_string("hello");
    pstring_t equal    = pcreate_string("equal");
    pstring_t test_key = pcreate_string("test key");
    pstring_t uwu      = pcreate_string("uwu herrow");

    phs_insert(map, hello, 55);
    phs_insert(map, equal, 43);

    map[phs_index(map, uwu)] = 99;

    printf("\"hello\": %lli: %i\n", phs_index(map, hello), map[phs_index(map, hello)]);
    printf("\"equal\": %lli: %i\n", phs_index(map, equal), map[phs_index(map, equal)]);

    for (int *i = phs_begin(map); i != phs_end(map); i++) {
        pstring_t *key = phs_get_key(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }

    int value = phs_remove_key(map, equal);
    printf("removed value: %i\n", value);
    phs_shrink(map);

    for (int *i = phs_begin(map); i != phs_end(map); i++) {
        pstring_t *key = phs_get_key(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }
    printf("\n");

    
    phs_insert(map, equal, 55);
    phs_insert(map, test_key, 55);
    for (int *i = phs_begin(map); i != phs_end(map); i++) {
        pstring_t *key = phs_get_key(map, i);
        printf("[%s]: %i\n", key->c_str, *i);
    }
    
    if (phs_contains(map, uwu))
        printf("this is an error the hashmap contains the key 'uwu herrow'\n");
    if (phs_contains(map, test_key))
        printf("the map contains 'test key'!\n");

    phs_free(map);
}


