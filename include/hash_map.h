#pragma once
#ifndef PTSD_HASH_MAP_HEADER
#define PTSD_HASH_MAP_HEADER
#include "general.h"
#ifndef PTSD_NUM_BUCKETS
#define PTSD_NUM_BUCKETS 32
#endif

#ifndef PTSD_HASH_MAP_GROWTH_COUNT
#define PTSD_HASH_MAP_GROWTH_COUNT 2
#endif

#define phs_freekey_func(map, delete_func) phs_freekey_func_implementation(map, delete_func)
#define phs_free_func(map, delete_func)    phs_free_func_implementation(map, delete_func)
#define phs_sizeof(x)                      phs_sizeof_implementation(x)

#define phs_create_hashmap(data_type, key_type, cmp) \
    phs_create_hashmap_implementation(data_type, key_type, cmp)
#define phs_create_hashmap_adv(data_type, key_type, hash, cmp) \
    phs_create_hashmap_adv_implementation(data_type, key_type, hash, cmp)

#define phs_compare_func(map, fnc) phs_compare_func_implementation(map, fnc)
#define phs_hash_func(map, fnc) phs_hash_func_implementation(map, fnc)

#define phs_get_hashmap(map) phs_get_hashmap_implementation(map)

#define phs_index(map, key) phs_index_implementation(map, key)

#define phs_insert(map, key, value) phs_insert_implementation(map, key, value)

#define phs_begin(map) phs_begin_implementation(map)
#define phs_end(map)   phs_end_implementation(map)

#define phs_get_key(map, iterator) phs_get_key_implementation(map, iterator)

#define phs_contains(map, key) phs_contains_implementation(map, key)

#define phs_remove_key(map, key) phs_remove_key_implementation(map, key)

#define phs_remove_iterator(map, iterator) phs_remove_iterator_implementation(map, iterator)

// frees the hashmap, note that it frees the values then the keys
// so if a free_func is bound to either
// which means, the free_func will be called for each value
// before any of the keys are freed
#define phs_free(map)   phs_free_implementation(map)

// makes the size of the hashmap in memory as small as possible
#define phs_shrink(map) phs_shrink_implementation(map)


struct phashmap_t;
typedef bool phashmap_compare_func_t(struct phashmap_t *, void *, void *);
typedef usize phashmap_hash_func_t(struct phashmap_t*, void *);
typedef struct phashmap_key_t phashmap_key_t;
struct phashmap_key_t {
    usize index; // index in value array
    usize size;
#if PTSD_C99
    u8 key_value[];
#endif
};

typedef void phs_free_func_t(void*);
typedef struct phashmap_t phashmap_t;
struct phashmap_t {
#if defined(PTSD_USE_ALLOCATOR)
    Allocator cb;
#endif
    phashmap_compare_func_t *cmp_fnc;
    phashmap_hash_func_t *hash_fnc;
    phs_free_func_t *free_key, *free_value;
    usize key_size;
    struct {
        usize size;
        phashmap_key_t *keys;
    } key_array[PTSD_NUM_BUCKETS];
    usize size;
    usize end_of_storage;
#if PTSD_C99
    // this is including the parity
    u8 value_buffer[];
#endif
};

#define phs_freekey_func_implementation(map, delete_func) ({\
    phs_get_hashmap(map)->free_key = (delete_func);         \
})
#define phs_free_func_implementation(map, delete_func) ({   \
    phs_get_hashmap(map)->free_value = (delete_func);       \
})

#define phs_sizeof_implementation(x) (sizeof(__typeof(x)))

#define phs_create_hashmap_implementation(data_type, key_type, cmp) \
    ((data_type*)                                                   \
     phs_create_hashmap_implementation_(phs_sizeof(data_type), phs_sizeof(key_type), NULL, cmp))

#if defined(PTSD_USE_ALLOCATOR)
#define phs_create_hashmap_adv_implementation(data_type, key_type, hash, cmp, allocator)\
    ((data_type*)                                                                       \
     phs_create_hashmap_implementation_(phs_sizeof(data_type), phs_sizeof(key_type), hash, cmp, allocator))
#else
#define phs_create_hashmap_adv_implementation(data_type, key_type, hash, cmp)   \
    ((data_type*)                                                               \
     phs_create_hashmap_implementation_(phs_sizeof(data_type), phs_sizeof(key_type), hash, cmp))
#endif

#define phs_compare_func_implementation(map, fnc) ({\
    phs_get_hashmap(map)->cmp_fnc = (fnc);          \
})
#define phs_hash_func_implementation(map, fnc) ({   \
    phs_get_hashmap(map)->hash_fnc = (fnc);         \
})



#define phs_get_hashmap_implementation(map) ({  \
    __auto_type parity = (map) - 1;             \
    phashmap_t *hmap = (phashmap_t*)(parity);   \
    hmap-1;                                     \
})

#define phs_index_implementation(map, key) ({           \
    __auto_type hmap     = phs_get_hashmap(map);        \
    __auto_type key_type = key;                         \
    isize index = phs_get_index(hmap, (void*)&key_type);\
    index;                                              \
})

#define phs_insert_implementation(map, key, value) ({                       \
    __auto_type hmap     = phs_get_hashmap(map);                            \
    __auto_type key_type = key;                                             \
    isize index = phs_get_index(hmap, (void*)&key_type);                    \
    if (index == -1) {                                                      \
        index = phs_make_space(&hmap, phs_sizeof(*(map)), (void*)&key_type);\
        (map) = (void*)(hmap->value_buffer + phs_sizeof(*(map)));           \
    }                                                                       \
    (map)[index] = (value);                                                 \
})

#define phs_begin_implementation(map) ({ (map); })
#define phs_end_implementation(map)   ({ ((map) + phs_get_hashmap(map)->size); })

#define phs_get_key_implementation(map, iterator) ({            \
    __auto_type hmap     = phs_get_hashmap(map);                \
    phs_get_key_from_iterator(hmap, sizeof(*(map)), (iterator));\
})

#define phs_contains_implementation(map, key) ({        \
    __auto_type hmap     = phs_get_hashmap(map);        \
    __auto_type key_type = key;                         \
    isize index = phs_get_index(hmap, (void*)&key_type);\
    index != -1;                                        \
})

#define phs_remove_key_implementation(map, key) ({                                  \
    __auto_type hmap     = phs_get_hashmap(map);                                    \
    __auto_type key_type = key;                                                     \
    isize index = phs_get_index(hmap, (void*)&key_type);                            \
    __auto_type value = (map)[index];                                               \
    if (index != -1) {                                                              \
        phs_remove_by_iterator(hmap, (void*)((map) + index), phs_sizeof(*(map)));   \
        (map) = (void*)(hmap->value_buffer + phs_sizeof(*(map)));                   \
    }                                                                               \
    value;                                                                          \
})

#define phs_remove_iterator_implementation(map, iterator) ({                            \
    __auto_type hmap     = phs_get_hashmap(map);                                        \
    __auto_type value    = *(iterator);                                                 \
    isize index = phs_remove_by_iterator(hmap, (void*)(iterator), phs_sizeof(*(map)));  \
    value;                                                                              \
})

#define phs_free_implementation(map) ({                     \
    __auto_type hmap = phs_get_hashmap(map);                \
    phs_free_implementation_(&(hmap), phs_sizeof(*(map)));  \
    (map) = NULL;                                           \
})

#define phs_shrink_implementation(map) ({                   \
    __auto_type hmap = phs_get_hashmap(map);                \
    phs_shrink_implementation_(&(hmap), phs_sizeof(*(map)));\
    (map) = (void*)((u8*)(hmap + 1) + phs_sizeof(*(map)));  \
})


PTSD_UNUSED
static inline usize phs_hash_object(usize value, void *obj_ptr, usize size) {
    static const usize hash_prime = 1697949259063052683ull;
    usize hash = value;
    u8 *obj = obj_ptr;
    for (usize i = 0; i < size; i++) {
        hash ^= (usize)(obj[i]);
        hash *= hash_prime;
    }
    return hash;
}

PTSD_UNUSED
static inline usize phs_default_hash_func(phashmap_t *hmap, void *ptr) {
    return phs_hash_object((usize)ptr, ptr, hmap->key_size); 
}

PTSD_UNUSED
static inline bool phs_default_compare_func(phashmap_t *hmap, void *_a, void *_b) {
    u8 *a = _a, *b = _b;
    for (usize i = 0; i < hmap->key_size; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}


#if defined(PTSD_USE_ALLOCATOR)
PTSD_UNUSED
static inline void *phs_create_hashmap_implementation_(
        usize data_size, usize key_size, 
        phashmap_hash_func_t *hs, phashmap_compare_func_t *cmp, pallocator_t cb) 
#else
PTSD_UNUSED
static inline void *phs_create_hashmap_implementation_(
        usize data_size, usize key_size, phashmap_hash_func_t *hs, phashmap_compare_func_t *cmp) 
#endif
{
#if defined(PTSD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    if (!cb.allocator) cb.allocator = pDefaultAllocator;
    allocator = cb.allocator;
#endif

    usize size = (data_size*2) + sizeof(phashmap_t);

    if (!hs)  hs  = phs_default_hash_func;
    if (!cmp) cmp = phs_default_compare_func;

#if defined(PTSD_USE_ALLOCATOR)
    void *tmp = allocator(&cb, ZERO_ALLOCATE, size, NULL);
#else
    void *tmp = pzero_allocate(size);
#endif 
    assert(tmp);
    phashmap_t *hmap = tmp;
    *hmap = (phashmap_t){
#if defined(PTSD_USE_ALLOCATOR)
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

PTSD_UNUSED
static inline isize phs_get_index(phashmap_t *hmap, void *key) {
    if (!hmap || !key) return -1;

    usize hash_key_size = (sizeof(phashmap_key_t) + hmap->key_size);
    usize hash = hmap->hash_fnc(hmap, key);
    __auto_type bucket = hmap->key_array[hash % PTSD_NUM_BUCKETS];
    for (usize i = 0; i < bucket.size; i++) {
        u8 *bucket_keys = (void*)bucket.keys;
        phashmap_key_t *key_data = (void*)(bucket_keys + (i * hash_key_size));
        if (hmap->cmp_fnc(hmap, key_data->key_value, key)) 
            return key_data->index;
    }
    return -1;
}

PTSD_UNUSED
static inline isize phs_make_space(phashmap_t **hmap_ptr, usize data_size, void *key) {
    phashmap_t *hmap = *hmap_ptr;
#if defined(PTSD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    usize remaining_elements = (hmap->end_of_storage/data_size) - hmap->size;
    if (remaining_elements == 0) {
        usize storage_size = hmap->end_of_storage + (data_size * PTSD_HASH_MAP_GROWTH_COUNT);
        usize new_size = sizeof(phashmap_t) + storage_size + data_size*2;
#if defined(PTSD_USE_ALLOCATOR)
        void *tmp = allocator(&hmap->cb, REALLOCATE, new_size, hmap);
#else
        void *tmp = preallocate(new_size, hmap);
#endif
        assert(tmp);
        hmap = *hmap_ptr = tmp;
        hmap->end_of_storage = storage_size;
    }

    usize hash = hmap->hash_fnc(hmap, key);
    __auto_type bucket = &hmap->key_array[hash % PTSD_NUM_BUCKETS];

    usize hash_key_size = (sizeof(phashmap_key_t) + hmap->key_size);
    usize size = (bucket->size + 1) * hash_key_size;
#if defined(PTSD_USE_ALLOCATOR)
    void *tmp = allocator(&hmap->cb, REALLOCATE, size, bucket->keys);
#else
    void *tmp = preallocate(size, bucket->keys);
#endif
    assert(tmp); bucket->keys = tmp;

    u8 *bucket_keys = (void*)bucket->keys;
    phashmap_key_t *key_data = (void*)(bucket_keys + (bucket->size * hash_key_size));
    key_data->size = hmap->key_size;
    memcpy(key_data->key_value, key, key_data->size);
    bucket->size++;

    isize index = hmap->size++;
    key_data->index = index;
    return index;
}

PTSD_UNUSED
static inline void *phs_get_key_from_iterator(phashmap_t *hmap, usize data_size, void *iterator) {
    u8 *array_start = (void*)(hmap + 1);
    array_start += data_size; // skip parity

    u8 *it = iterator;
    usize index = (it - array_start)/data_size;
    usize hash_key_size = (sizeof(phashmap_key_t) + hmap->key_size);

    for (int i = 0; i < PTSD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type bucket = hmap->key_array[i];
        for (usize i = 0; i < bucket.size; i++) {
            u8 *bucket_keys = (void*)bucket.keys;
            phashmap_key_t *key_data = (void*)(bucket_keys + (i * hash_key_size));
            if (key_data->index == index) return key_data->key_value;
        }
    }
    return NULL;
}

PTSD_UNUSED
static inline void phs_remove_by_iterator(phashmap_t *hmap, void *it, usize data_size) {
#if defined(PTSD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    u8 *iterator = it;
    usize index = (iterator - (hmap->value_buffer + data_size))/data_size;

    __typeof(*hmap->key_array) *bucket = NULL;
    usize hash_key_size = (sizeof(phashmap_key_t) + hmap->key_size);
    isize key_index = -1;

    for (int i = 0; i < PTSD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type key_bucket = hmap->key_array[i];
        for (usize j = 0; j < key_bucket.size; j++) {
            u8 *bucket_keys = (void*)key_bucket.keys;
            phashmap_key_t *key_data = (void*)(bucket_keys + (j * hash_key_size));
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
            hmap->free_key(((phashmap_key_t*)key_array)->key_value);
        }

        if (key_index == (isize)(bucket->size - 1)) goto skip_copy;
        
        usize count = bucket->size - key_index;
        phashmap_key_t *key  = (void*)(bucket->keys + (key_index * hash_key_size));
        phashmap_key_t *rest = (void*)(bucket->keys + ((key_index + 1) * hash_key_size));
        memcpy(key, rest, count * hash_key_size);

skip_copy:
        usize size = (bucket->size - 1) * hash_key_size;
#if defined(PTSD_USE_ALLOCATOR)
        void *tmp = allocator(&hmap->cb, REALLOCATE, size, bucket->keys);
#else
        void *tmp = preallocate(size, bucket->keys);
#endif
        assert(tmp); bucket->keys = tmp;
        bucket->size--;
    }
}

PTSD_UNUSED
static inline void phs_free_implementation_(phashmap_t **map_ptr, usize data_size) {
    phashmap_t *hmap = *map_ptr;
#if defined(PTSD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif

    if (hmap->free_value) {
        for (usize i = 0; i < hmap->size; i++) {
            hmap->free_value(hmap->value_buffer + (i*data_size));
        }
    }

    usize hash_key_size = (sizeof(phashmap_key_t) + hmap->key_size);
    for (int i = 0; i < PTSD_NUM_BUCKETS; i++) {
        if (!hmap->key_array[i].size) continue;
        __auto_type key_bucket = hmap->key_array[i];
        if (hmap->free_key) {
            for (usize j = 0; j < key_bucket.size; j++) {
                u8 *bucket_keys = (void*)key_bucket.keys;
                phashmap_key_t *key_data = (void*)(bucket_keys + (j * hash_key_size));
                hmap->free_key(key_data->key_value);
            }
        }
#if defined(PTSD_USE_ALLOCATOR)
        allocator(&hmap->cb, SIZED_FREE, hash_key_size, key_bucket.keys);
#else
        psized_free(hash_key_size, key_bucket.keys);
#endif
    }
#if defined(PTSD_USE_ALLOCATOR)
    allocator(&hmap->cb, SIZED_FREE, sizeof(*hmap) + (hmap->size*data_size), hmap);
#else
    psized_free(sizeof(*hmap) + (hmap->size*data_size), hmap);
#endif
    *map_ptr = NULL;
}

PTSD_UNUSED
static inline void phs_shrink_implementation_(phashmap_t **hmap_ptr, usize data_size) {
    phashmap_t *hmap = *hmap_ptr;
#if defined(PTSD_USE_ALLOCATOR)
    void *(*allocator)(Allocator *self, AllocationKind kind, usize size, void *block); 
    allocator = hmap->cb.allocator;
#endif
    
    usize storage_size = hmap->size * data_size;
    usize new_size = sizeof(*hmap) + data_size + storage_size;
#if defined(PTSD_USE_ALLOCATOR)
    void *tmp = allocator(&hmap->cb, REALLOCATE, new_size, hmap);
#else
    void *tmp = preallocate(new_size, hmap);
#endif
    assert(tmp); hmap = tmp;
    hmap->end_of_storage = storage_size;
    *hmap_ptr = hmap;
}
#endif // PTSD_HASH_MAP_HEADER

