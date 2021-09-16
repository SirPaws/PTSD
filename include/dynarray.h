#pragma once

#ifndef PSTD_DYNARRAY_HEADER
#define PSTD_DYNARRAY_HEADER
#ifndef DYNARRAY_STANDALONE
#   include "general.h"
#else
#error not implemented yet!
#endif

#if defined(_MSC_FULL_VER) && !defined(__clang__)
#error MVSC COMPILER NOT SUPPORTED!
#endif

#ifndef P_DYNARRAY_GROWTH_COUNT
#define P_DYNARRAY_GROWTH_COUNT 2
#endif

#define pda_create_dynarray(name, datatype)      pda_create_dynarray_implementaion(name, datatype)
#define pda_create_static_dynarray(type, value)  pda_create_static_dynarray_implementaion(type, value)

#define pda_size(array)                          pda_size_implementation(array)
#define pda_length(array)                        pda_size_implementation(array)
#define pda_len(array)                           pda_size_implementation(array)
#define pda_sizeof(value)                        pda_sizeof_implementation(value)

#define pda_free(array)
#define pda_set_capacity(array, count)           pda_set_capacity_implementation(array, count)
#define pda_set_cap(array, count)                pda_set_capacity_implementation(array, count)
#define pda_set_count(array, count)              pda_set_capacity_implementation(array, count)
#define pda_set_size(array, count)               pda_set_capacity_implementation(array, count)
#define pda_set_length(array, count)             pda_set_capacity_implementation(array, count)

#define pda_pushback(array, value)               pda_pushback_implementation(array, value)
#define pda_pushbytes(array, value, bytes)       pda_pushbytes_implementation(array, value, bytes)
#define pda_begin(array)                         pda_begin_implementation(array)
#define pda_end(array)                           pda_end_implementation(array)
#define pda_insert(array, position, value)       pda_insert_implementation(array, position, value)
#define pda_makehole(array, position, num_bytes) pda_makehole_implementation(array, position, num_bytes) 
#define pda_popback(array)                       pda_popback_implementation(array)
#define pda_remove(array, position)              pda_remove_implementation(array, position) 
#define pda_copy_dynarray(arr)                   pda_copy_dynarray_implementation(arr)
#define pda_copy_array(type, arr)                pda_copy_array_implementation(type, arr)

















#define pda_create_dynarray_implementaion(name, datatype)   \
    struct name {                                           \
        usize end_of_storage;                               \
        usize size;                                         \
        __typeof(datatype) *data;                           \
    }
#define pda_create_static_dynarray_implementaion(type, value) (type){ sizeof(value), countof(value), value }

#define pda_size_implementation(array)   ((array)->size) 
#define pda_sizeof_implementation(value) (sizeof(__typeof(value)))

#define pda_free_implementation(array) ({   \
        pfree((array)->data);               \
        memset((array), 0, sizeof *(array));\
    })

#define pda_set_capacity_implementation(array, count) ({                                    \
    if (!(array)->data) {                                                                   \
        __typeof(*(array)) pda_set_capacity_array = {                                       \
            .end_of_storage = pda_sizeof((array)->data[0]) * (count),                       \
            .data         = pzero_allocate(pda_sizeof((array)->data[0]) * (count))          \
        };                                                                                  \
        *(array) = pda_set_capacity_array;                                                  \
    } else {                                                                                \
        void *pda_set_capacity_tmp = preallocate((pda_sizeof((array)->data[0]) * (count)),  \
                (array)->data);                                                             \
        (array)->data = pda_set_capacity_tmp;                                               \
    }                                                                                       \
    (array);                                                                                \
})

#define pda_pushback_implementation(array, value) ({                \
    pda_maybe_grow((pdynarray_t *)(array), pda_sizeof(value));      \
    __auto_type pda_pushback_ret = (array)->data + (array)->size++; \
    *pda_pushback_ret = (value);                                    \
    pda_pushback_ret;                                               \
})

#define pda_pushbytes_implementation(array, value, bytes) ({        \
    pda_maybe_byte_grow((pdynarray_t *)(array), (bytes));           \
    memcpy((array)->data + (array)->size, (value), (bytes));        \
    __auto_type pda_pushbytes_ret = (array)->data + ((array)->size);\
    (array)->size += (bytes);                                       \
    pda_pushbytes_ret;                                              \
})

#define pda_begin_implementation(array) ({ (array)->data; })
#define pda_end_implementation(array) ({ (array)->data + (array)->size; })

#define pda_insert_implementation(array, position, value) ({                                                \
    usize pda_insert_size   = pda_sizeof(value);                                                            \
    usize pda_insert_offset = (position) - pda_begin(array);                                                \
    __typeof(value) *pda_insert_result = NULL;                                                              \
    if (pda_insert_offset >= (array)->size) {}                                                              \
    else {                                                                                                  \
        pda_maybe_grow((pdynarray_t *)(array), pda_insert_size);                                            \
        /* first we extract all elements after the place where we want                                    */\
        /* to insert and then we shift them one element forward                                           */\
        /* here is an example we wan't to insert 6 at the place pointed to below                          */\
        /* [1, 2, 3, 4]                                                                                   */\
        /*     ^                                                                                          */\
        /* we make a new array that holds [2, 3, 4]                                                       */\
        /* we insert that into the array                                                                  */\
        /* [1, 2, 2, 3, 4]                                                                                */\
        /* then we insert the value                                                                       */\
        /* [1, 6, 2, 3, 4]                                                                                */\
        usize pda_insert_elems = (array)->size - pda_insert_offset;                                         \
        void *pda_insert_tmp = pallocate(pda_insert_size * pda_insert_elems);                               \
        memcpy(pda_insert_tmp, (array)->data + pda_insert_offset, pda_insert_elems * pda_insert_size);      \
        memcpy((array)->data + pda_insert_offset + 1, pda_insert_tmp, pda_insert_elems * pda_insert_size);  \
        pfree(pda_insert_tmp);                                                                              \
                                                                                                            \
        (array)->size++;                                                                                    \
        (array)->data[pda_insert_offset] = value;                                                           \
        pda_insert_result = (array)->data + pda_insert_offset;                                              \
    }                                                                                                       \
    pda_insert_result;                                                                                      \
})

#define pda_makehole_implementation(array, position, num_bytes) ({                              \
    usize pda_makehole_size = sizeof(__typeof((array)->data[0]));                               \
    usize pda_makehole_offset = (position) - pda_begin(array);                                  \
    __typeof((array)->data[0]) *pda_makehole_result = NULL;                                     \
    if (pda_makehole_offset >= (array)->size) {}                                                \
    else {                                                                                      \
        pda_maybe_byte_grow((pdynarray_t *)(array), num_bytes);                                 \
        /* first we extract all elements after the place where we want                        */\
        /* to insert and then we shift them one element forward                               */\
        /* here is an example we wan't to insert 6 at the place pointed to below              */\
        /* [1, 2, 3, 4]                                                                       */\
        /*     ^                                                                              */\
        /* we make a new array that holds [2, 3, 4]                                           */\
        /* we insert that into the array                                                      */\
        /* [1, 2, 2, 3, 4]                                                                    */\
        /* then we insert the value                                                           */\
        /* [1, 6, 2, 3, 4]                                                                    */\
        usize pda_makehole_elems = (array)->size - pda_makehole_offset;                         \
        void *pda_makehole_tmp = pallocate(pda_makehole_size * pda_makehole_elems);             \
        memcpy(pda_makehole_tmp, (array)->data + pda_makehole_offset,                           \
                pda_makehole_elems * pda_makehole_size);                                        \
        memcpy((array)->data + pda_makehole_offset + (num_bytes), pda_makehole_tmp,             \
                pda_makehole_elems * pda_makehole_size);                                        \
        pfree(pda_makehole_tmp);                                                                \
                                                                                                \
        pda_makehole_result = (array)->data + pda_makehole_offset;                              \
    }                                                                                           \
    pda_makehole_result;                                                                        \
})


#define pda_popback_implementation(array) ({                        \
    __typeof((array)->data[0]) pda_popback_result = {0};            \
    if ((array)->size == 0) { }                                     \
    else {                                                          \
        pda_popback_result = (array)->data[((array)->size--) - 1];  \
    }                                                               \
    pda_popback_result;                                             \
})

#define pda_remove_implementation(array, position) ({                                       \
    __typeof((array)->data[0]) pda_remove_result = {0};                                     \
    usize pda_remove_offset = (position) - pda_begin(array);                                \
    if (pda_remove_offset >= (array)->size) {}                                              \
    else if (pda_remove_offset == (array)->size - 1) {                                      \
        (array)->size--;                                                                    \
        pda_remove_result = (array)->data[pda_remove_offset];                               \
    } else {                                                                                \
        pda_remove_result = (array)->data[pda_remove_offset];                               \
        usize pda_remove_elems = (array)->size - pda_remove_offset;                         \
        void *pda_remove_tmp = pallocate(pda_sizeof((array)->data[0]) * pda_remove_elems);  \
        memcpy(pda_remove_tmp, (array)->data + pda_remove_offset + 1,                       \
                pda_remove_elems * pda_sizeof((array)->data[0]));                           \
        memcpy((array)->data + pda_remove_offset, pda_remove_tmp,                           \
                pda_remove_elems * pda_sizeof((array)->data[0]));                           \
        pfree(pda_remove_tmp);                                                              \
        (array)->size--;                                                                    \
    }                                                                                       \
    pda_remove_result;                                                                      \
})

// arr:  another dynamic array
//TODO: this code is unreadable consider changing it
#define pda_copy_dynarray_implementation(arr) ({                                        \
    __typeof(arr) pda_copy_dynarray_tmp = { 0 };                                        \
    pda_copy_dynarray_tmp.data = pallocate( (arr).size );                               \
    memcpy(pda_copy_dynarray_tmp.data, (arr).data, sizeof(*(arr).data) * (arr).size);   \
    pda_copy_dynarray_tmp.size = pCopypdynarray_t_tmp.end_of_storage = (arr).size;      \
    pda_copy_dynarray_tmp;                                                              \
})

// type: the type of array we want
// arr:  a static array
#define pda_copy_array_implementation(type, arr) ({         \
        type pda_copy_array_tmp = { 0 };                    \
        pda_copy_array_tmp.data = pallocate( sizeof(arr) ); \
        memcpy(pda_copy_array_tmp.data, (arr), sizeof(arr));\
        pda_copy_array_tmp.size = countof(arr);             \
        pda_copy_array_tmp.end_of_storage = sizeof(arr);    \
        pda_copy_array_tmp;                                 \
})

typedef struct pdynarray_t pdynarray_t;
struct pdynarray_t {
    usize end_of_storage;
    usize size;
    void *data;
};

// how many elements we should add
static void pda_bytegrow(pdynarray_t *array, usize bytes);
static void pda_grow(pdynarray_t *array, usize datasize, usize count);

PSTD_UNUSED
static void pda_maybe_bytegrow(pdynarray_t *array, usize bytes) {
    if (array->size + bytes > array->end_of_storage) {
        pda_bytegrow(array, bytes);
    }
}

PSTD_UNUSED
static void pda_maybe_grow(pdynarray_t *array, usize datasize) {
    if ((array->size + 1) * datasize > array->end_of_storage) {
        pda_grow((pdynarray_t *)array,
                datasize, P_DYNARRAY_GROWTH_COUNT);
    }
}

PSTD_UNUSED
static void pda_bytegrow(pdynarray_t *array, usize bytes) {
    if (!bytes || !array) return;
    void *tmp = preallocate(array->end_of_storage + bytes, array->data);
    // assert(tmp);
    array->data = tmp;
    array->end_of_storage += bytes;
}

PSTD_UNUSED
static void pda_grow(pdynarray_t *array, usize datasize, usize count) {
    if (!count || !array || !datasize) return;
    void *tmp = preallocate(datasize * (array->end_of_storage + count), array->data);
    // assert(tmp);
    array->data = tmp;
    array->end_of_storage += count;
}
#endif // PSTD_DYNARRAY_HEADER
