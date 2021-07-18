
#include "general.h"

#if defined(PSTD_USE_ALLOCATOR) && !defined(PSTD_ALLOCATOR_DEFINED)
#define PSTD_ALLOCATOR_DEFINED
enum AllocationKind {
    ALLOCATE,
    ZERO_ALLOCATE,
    REALLOCATE,
    FREE,
    SIZED_FREE,
};
typedef enum AllocationKind AllocationKind;
typedef struct Allocator Allocator;
struct Allocator {
    void *(*allocator)(Allocator *, AllocationKind kind, usize size, void *buffer); 
    void *user_data;
};

void *pDefaultAllocator(
        Allocator *, AllocationKind kind, usize size, void *buffer) 
{
    switch(kind) {
    case ALLOCATE:       return pAllocate(size);
    case ZERO_ALLOCATE:  return pZeroAllocate(size);
    case REALLOCATE:     return pReallocate(size, buffer);
    case FREE:           return (pFree(buffer), NULL); // NOLINT
    case SIZED_FREE:     return (pSizedFree(size, buffer), NULL);
    }
    return NULL;
}
const static Allocator PSTD_DEFAULT_HASH_MAP_ALLOCATOR = {
    .allocator = pDefaultAllocator,
};
#endif

#ifndef PSTD_NUM_BUCKETS
#define PSTD_NUM_BUCKETS 32
#endif

#ifndef PSTD_HASH_MAP_GROWTH_COUNT
#define PSTD_HASH_MAP_GROWTH_COUNT 2
#endif

struct HashMap;
typedef pBool HashMapCompareFunc(struct HashMap *, void *, void *);
typedef usize HashMapHash(struct HashMap*, void *);
typedef struct HashMapKey HashMapKey;
struct HashMapKey {
    usize index; // index in value array
    usize size;
#if PSTD_C99
    u8 key_value[];
#endif
};

typedef void pFreeFunction(void*);
typedef struct HashMap HashMap;
struct HashMap {
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb;
#endif
    HashMapCompareFunc *cmp_fnc;
    HashMapHash *hash_fnc;
    pFreeFunction *free_key, *free_value;
    usize key_size;
    struct {
        usize size;
        HashMapKey *keys;
    } key_array[PSTD_NUM_BUCKETS];
    usize size;
    usize end_of_storage;
#if PSTD_C99
    // this is including the parity
    u8 value_buffer[];
#endif
};

#define pSetKeyFreeFunc(map, delete_func) ({   \
    pGetHashMap(map)->free_key = (delete_func);\
})
#define pSetFreeFunc(map, delete_func)({         \
    pGetHashMap(map)->free_value = (delete_func);\
})

#define pSizeof(x) (sizeof(__typeof(x)))

#define pCreateHashMap(data_type, key_type, cmp)\
    ((data_type*)\
     pCreateHashMapImplementation(pSizeof(data_type), pSizeof(key_type), NULL, cmp))

#if defined(PSTD_USE_ALLOCATOR)
#define pCreateHashMapAdv(data_type, key_type, hash, cmp, allocator) \
    ((data_type*)\
     pCreateHashMapImplementation(pSizeof(data_type), pSizeof(key_type), hash, cmp, allocator))
#else
#define pCreateHashMapAdv(data_type, key_type, hash, cmp) \
    ((data_type*)\
     pCreateHashMapImplementation(pSizeof(data_type), pSizeof(key_type), hash, cmp))
#endif

#define pSetKeyCompareFunc(map, fnc) ({ \
    pGetHashMap(map)->cmp_fnc = (fnc);  \
})
#define pSetHashFunc(map, fnc) ({       \
    pGetHashMap(map)->hash_fnc = (fnc); \
})



#define pGetHashMap(map) ({             \
    __auto_type parity = (map) - 1;     \
    HashMap *hmap = (HashMap*)(parity); \
    hmap-1;                             \
})

#define pIndex(map, key) ({\
    __auto_type hmap     = pGetHashMap(map);            \
    __auto_type key_type = key;                         \
    isize index = pHashGetIndex(hmap, (void*)&key_type);\
    index;                                              \
})

#define pInsert(map, key, value) ({                                         \
    __auto_type hmap     = pGetHashMap(map);                                \
    __auto_type key_type = key;                                             \
    isize index = pHashGetIndex(hmap, (void*)&key_type);                    \
    if (index == -1) {                                                      \
        index = pHashMapMakeSpace(&hmap, pSizeof(*(map)), (void*)&key_type);\
        (map) = (void*)(hmap->value_buffer + pSizeof(*(map)));              \
    }                                                                       \
    (map)[index] = (value);                                                 \
})

#define pBegin(map) ({ (map); })
#define pEnd(map)   ({ ((map) + pGetHashMap(map)->size); })

#define pGetKey(map, iterator) ({                           \
    __auto_type hmap     = pGetHashMap(map);                \
    pGetKeyFromIterator(hmap, sizeof(*(map)), (iterator));  \
})

#define pContains(map, key) ({                          \
    __auto_type hmap     = pGetHashMap(map);            \
    __auto_type key_type = key;                         \
    isize index = pHashGetIndex(hmap, (void*)&key_type);\
    index != -1;                                        \
})

#define pRemoveKey(map, key) ({\
    __auto_type hmap     = pGetHashMap(map);                                \
    __auto_type key_type = key;                                             \
    isize index = pHashGetIndex(hmap, (void*)&key_type);                    \
    __auto_type value = (map)[index];                                       \
    if (index != -1) {                                                      \
        pRemoveByIterator(hmap, (void*)((map) + index), pSizeof(*(map)));   \
        (map) = (void*)(hmap->value_buffer + pSizeof(*(map)));              \
    }                                                                       \
    value;                                                                  \
})

#define pRemoveIterator(map, iterator) ({                                       \
    __auto_type hmap     = pGetHashMap(map);                                    \
    __auto_type value    = *(iterator);                                         \
    isize index = pRemoveByIterator(hmap, (void*)(iterator), pSizeof(*(map)));  \
    value;                                                                      \
})

#define pFreeHashMap(map) ({                                \
    __auto_type hmap = pGetHashMap(map);                    \
    pFreeHashMapImplementation(&(hmap), pSizeof(*(map)));   \
    (map) = NULL;\
})

#define pShrinkHashMap(map) ({                              \
    __auto_type hmap = pGetHashMap(map);                    \
    pShrinkHashMapImplementation(&(hmap), pSizeof(*(map))); \
    (map) = (void*)((u8*)(hmap + 1) + pSizeof(*(map)));     \
})


PSTD_UNUSED
static inline usize pHashObject(usize value, void *obj_ptr, usize size) {
    static const usize hash_prime = 1697949259063052683ull;
    usize hash = value;
    u8 *obj = obj_ptr;
    for (usize i = 0; i < size; i++) {
        hash ^= (usize)(obj[i]);
        hash *= hash_prime;
    }
    return hash;
}

PSTD_UNUSED
static inline usize pDefaultHashFunc(HashMap *hmap, void *ptr) {
    return pHashObject((usize)ptr, ptr, hmap->key_size); 
}

PSTD_UNUSED
static inline pBool pDefaultCompareFunc(HashMap *hmap, void *_a, void *_b) {
    u8 *a = _a, *b = _b;
    for (usize i = 0; i < hmap->key_size; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}


#if defined(PSTD_USE_ALLOCATOR)
PSTD_UNUSED
static inline void *pCreateHashMapImplementation(
        usize data_size, usize key_size, HashMapHash *hs, HashMapCompareFunc *cmp, Allocator cb) 
#else
PSTD_UNUSED
static inline void *pCreateHashMapImplementation(
        usize data_size, usize key_size, HashMapHash *hs, HashMapCompareFunc *cmp) 
#endif
{
#if defined(PSTD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    if (!cb.allocator) cb.allocator = pDefaultAllocator;
    allocator = cb.allocator;
#endif

    usize size = (data_size*2) + sizeof(HashMap);

    if (!hs)  hs  = pDefaultHashFunc;
    if (!cmp) cmp = pDefaultCompareFunc;

#if defined(PSTD_USE_ALLOCATOR)
    void *tmp = allocator(&cb, ZERO_ALLOCATE, size, NULL);
#else
    void *tmp = pZeroAllocate(size);
#endif 
    assert(tmp);
    HashMap *hmap = tmp;
    *hmap = (HashMap){
#if defined(PSTD_USE_ALLOCATOR)
        .cb             = cb,
#endif
        .hash_fnc       = hs,
        .cmp_fnc        = cmp,
        .key_size       = key_size,
        .size           = 0,
        .end_of_storage = data_size,
    };
    u8 *parity = hmap->value_buffer;
    return parity + data_size;
}

PSTD_UNUSED
static inline isize pHashGetIndex(HashMap *hmap, void *key) {
    if (!hmap || !key) return -1;

    usize hash_key_size = (sizeof(HashMapKey) + hmap->key_size);
    usize hash = hmap->hash_fnc(hmap, key);
    __auto_type bucket = hmap->key_array[hash % PSTD_NUM_BUCKETS];
    for (usize i = 0; i < bucket.size; i++) {
        u8 *bucket_keys = (void*)bucket.keys;
        HashMapKey *key_data = (void*)(bucket_keys + (i * hash_key_size));
        if (hmap->cmp_fnc(hmap, key_data->key_value, key)) 
            return key_data->index;
    }
    return -1;
}

PSTD_UNUSED
static inline isize pHashMapMakeSpace(HashMap **hmap_ptr, usize data_size, void *key) {
    HashMap *hmap = *hmap_ptr;
#if defined(PSTD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    usize remaining_elements = (hmap->end_of_storage/data_size) - hmap->size;
    if (remaining_elements == 0) {
        usize storage_size = hmap->end_of_storage + (data_size * PSTD_HASH_MAP_GROWTH_COUNT);
        usize new_size = sizeof(HashMap) + storage_size + data_size*2;
#if defined(PSTD_USE_ALLOCATOR)
        void *tmp = allocator(&hmap->cb, REALLOCATE, new_size, hmap);
#else
        void *tmp = pReallocate(new_size, hmap);
#endif
        assert(tmp);
        hmap = *hmap_ptr = tmp;
        hmap->end_of_storage = storage_size;
    }

    usize hash = hmap->hash_fnc(hmap, key);
    __auto_type bucket = &hmap->key_array[hash % PSTD_NUM_BUCKETS];

    usize hash_key_size = (sizeof(HashMapKey) + hmap->key_size);
    usize size = (bucket->size + 1) * hash_key_size;
#if defined(PSTD_USE_ALLOCATOR)
    void *tmp = allocator(&hmap->cb, REALLOCATE, size, bucket->keys);
#else
    void *tmp = pReallocate(size, bucket->keys);
#endif
    assert(tmp); bucket->keys = tmp;

    u8 *bucket_keys = (void*)bucket->keys;
    HashMapKey *key_data = (void*)(bucket_keys + (bucket->size * hash_key_size));
    key_data->size = hmap->key_size;
    memcpy(key_data->key_value, key, key_data->size);
    bucket->size++;

    isize index = hmap->size++;
    key_data->index = index;
    return index;
}

PSTD_UNUSED
static inline void *pGetKeyFromIterator(HashMap *hmap, usize data_size, void *iterator) {
    u8 *array_start = (void*)(hmap + 1);
    array_start += data_size; // skip parity

    u8 *it = iterator;
    usize index = (it - array_start)/data_size;
    usize hash_key_size = (sizeof(HashMapKey) + hmap->key_size);

    for (int i = 0; i < PSTD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type bucket = hmap->key_array[i];
        for (usize i = 0; i < bucket.size; i++) {
            u8 *bucket_keys = (void*)bucket.keys;
            HashMapKey *key_data = (void*)(bucket_keys + (i * hash_key_size));
            if (key_data->index == index) return key_data->key_value;
        }
    }
    return NULL;
}

PSTD_UNUSED
static inline void pRemoveByIterator(HashMap *hmap, void *it, usize data_size) {
#if defined(PSTD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    u8 *iterator = it;
    usize index = (iterator - (hmap->value_buffer + data_size))/data_size;

    __typeof(*hmap->key_array) *bucket = NULL;
    usize hash_key_size = (sizeof(HashMapKey) + hmap->key_size);
    isize key_index = -1;

    for (int i = 0; i < PSTD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type key_bucket = hmap->key_array[i];
        for (usize j = 0; j < key_bucket.size; j++) {
            u8 *bucket_keys = (void*)key_bucket.keys;
            HashMapKey *key_data = (void*)(bucket_keys + (j * hash_key_size));
            if (key_data->index == index) {
                key_index = j;
                bucket = hmap->key_array + i;
                goto key_found;
            }
        }
    }
key_found:
    if (!bucket) return;

    usize count = hmap->size - index;
    usize next = (index + 1) % hmap->size;
    
    u8* buffer = hmap->value_buffer + data_size;
    if (hmap->free_value)
        hmap->free_value(buffer + (index * data_size));

    memcpy(hmap->value_buffer, buffer + (index * data_size), data_size);
    memcpy(buffer + (index*data_size),
           buffer + (next*data_size), data_size * count);
    hmap->size--;

    if (bucket->size == 1) {
        free(bucket->keys);
        bucket->size = 0;
        bucket->keys = NULL;
    } else {
        if (hmap->free_key) { // now it's getting freekey
            u8 *key_array = (void*)bucket->keys;
            key_array += (key_index * hash_key_size);
            hmap->free_key(((HashMapKey*)key_array)->key_value);
        }

        if (key_index == (isize)(bucket->size - 1)) goto skip_copy;
        
        usize count = bucket->size - key_index;
        HashMapKey *key  = (void*)(bucket->keys + (key_index * hash_key_size));
        HashMapKey *rest = (void*)(bucket->keys + ((key_index + 1) * hash_key_size));
        memcpy(key, rest, count * hash_key_size);

skip_copy:
        usize size = (bucket->size - 1) * hash_key_size;
#if defined(PSTD_USE_ALLOCATOR)
        void *tmp = allocator(&hmap->cb, REALLOCATE, size, bucket->keys);
#else
        void *tmp = pReallocate(size, bucket->keys);
#endif
        assert(tmp); bucket->keys = tmp;
        bucket->size--;
    }
}

PSTD_UNUSED
static inline void pFreeHashMapImplementation(HashMap **map_ptr, usize data_size) {
    HashMap *hmap = *map_ptr;
#if defined(PSTD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    if (hmap->free_value) {
        for (usize i = 0; i < hmap->size; i++) {
            hmap->free_value(hmap->value_buffer + (i*data_size));
        }
    }

    usize hash_key_size = (sizeof(HashMapKey) + hmap->key_size);
    for (int i = 0; i < PSTD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type key_bucket = hmap->key_array[i];
        if (hmap->free_key) {
            for (usize j = 0; j < key_bucket.size; j++) {
                u8 *bucket_keys = (void*)key_bucket.keys;
                HashMapKey *key_data = (void*)(bucket_keys + (j * hash_key_size));
                hmap->free_key(key_data->key_value);
            }
        }
#if defined(PSTD_USE_ALLOCATOR)
        allocator(&hmap->cb, SIZED_FREE, hash_key_size, key_bucket.keys);
#else
        pSizedFree(hash_key_size, key_bucket.keys);
#endif
    }
#if defined(PSTD_USE_ALLOCATOR)
    allocator(&hmap->cb, SIZED_FREE, sizeof(*hmap) + (hmap->size*data_size), hmap);
#else
    pSizedFree(sizeof(*hmap) + (hmap->size*data_size), hmap);
#endif
    *map_ptr = NULL;
}

PSTD_UNUSED
static inline void pShrinkHashMapImplementation(HashMap **hmap_ptr, usize data_size) {
    HashMap *hmap = *hmap_ptr;
#if defined(PSTD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif
    
    usize storage_size = hmap->size * data_size;
    usize new_size = sizeof(*hmap) + data_size + storage_size;
#if defined(PSTD_USE_ALLOCATOR)
    void *tmp = allocator(&hmap->cb, REALLOCATE, new_size, hmap);
#else
    void *tmp = pReallocate(new_size, hmap);
#endif
    assert(tmp); hmap = tmp;
    hmap->end_of_storage = storage_size;
    *hmap_ptr = hmap;
}


