#pragma once

#ifndef PSTD_STRETCHY_BUFFER_HEADER
#define PSTD_STRETCHY_BUFFER_HEADER
#ifndef STRETCHY_BUFFER_STANDALONE
#   include "general.h"
#   include "plimits.h"
#else
#error not implemented yet
#endif

#ifndef PSTD_STRETCHY_BUFFER_GROWTH_COUNT
#define PSTD_STRETCHY_BUFFER_GROWTH_COUNT 2
#endif

// symbol to make it clear that a datatype is a stretchy buffer
// ie.  `int *stretchy data;
#define stretchy

// sets a function that will be called on each element
// when the element is removed, equivalent to a destrutor in c++
#define psb_set_free_func(array, free_func) \
    psb_set_free_func_implementation(array, free_func)

#if defined(PSTD_USE_ALLOCATOR)
// creates a new array with a given allocator
#define psb_create(data_type, allocator) \
    psb_create_implementation(data_type, allocator)
#endif

// sets the capacity of the current buffer to hold exactly 'count' elements
#define psb_resize(array, count)       psb_set_capacity_implementation(array, count)
#define psb_set_cap(array, count)      psb_set_capacity_implementation(array, count)
#define psb_set_count(array, count)    psb_set_capacity_implementation(array, count)
#define psb_set_size(array, count)     psb_set_capacity_implementation(array, count)
#define psb_set_length(array, count)   psb_set_capacity_implementation(array, count)
#define psb_set_capacity(array, count) psb_set_capacity_implementation(array, count)

// sets the capacity of the current buffer to hold at least 'count' elements
// if the current size of the buffer is greater than count this function does nothing
#define psb_reserve(array, count)  psb_reserve_implementation(array, count)

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
#define psb_pushback(array, value) psb_pushback_implementation(array, value)

// appends 'count' elements from 'val_arr' to the end of the array and returns a pointer to the last element pushed
#define psb_pushback_n(array, val_arr, count) psb_pushback_n_implementation(array, val_arr, count)

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
// the difference between this and normal pushback is that you don't have to pass in a stretchy buffer
#define psb_pseudo_pushback(array, length, value) psb_pseudo_pushback_implementation(array, length, value)

// same as psb_pushback but gives control over how many bytes needs to be inserted into the array
#define psb_pushbytes(array, value, num_bytes) psb_pushbytes_implementation(array, value, num_bytes)

// removes last element in array and returns a copy 
#define psb_popback(array) psb_popback_implementation(array)
 
// inserts 'value' at 'position' in 'array' and returns a pointer to 'position' in the array
// note that this might invalidate the 'position' parameter so if you need to reuse
// 'position' you should call it thously
// location = psb_insert(array, location, 10)
#define psb_insert(array, position, value) psb_insert_implementation(array, position, value)

// removes element at 'position' in 'array' and returns the data that was at 'position'
#define psb_remove(array, position) psb_remove_implementation(array, position)

// returns a pointer to the first element of the array
#define psb_begin(array) psb_begin_implementation(array)
// returns a pointer to the element after the last element in the array
#define psb_end(array)   psb_end_implementation(array)

// returns a pointer to the last element of the array
#define psb_begin_r(array)  psb_begin_r_implementation(array)
// returns a pointer to the element before the first element in the array
#define psb_end_r(array)    psb_end_r_implementation(array)

// get the number of elements currently stored in the array
#define psb_size(array)   psb_size_implementation(array)
#define psb_length(array) psb_size_implementation(array) 
#define psb_count(array)  psb_size_implementation(array) 

// creates a copy of a strechy buffer and returns it
#define psb_copybuffer(array) psb_copybuffer_implementation(array)

// creates a copy of a static array as a strechy buffer and returns it
#define psb_copyarray(type, array)   psb_copyarray_implementation(type, array)

// frees strechy buffer
#define psb_free(array) psb_free_implementation(array)

// gets meta data for the array
#define psb_get_meta(array)            psb_get_meta_implementation(array)

// gets meta data for the array. if the array is equal to NULL
// then it creates a new strechy buffer and assigns array to that
#define psb_get_meta_or_create(array) psb_get_meta_or_create_implementation(array)

// utility function that removes some warning in GNU compatible compilers.
// in MSVC this is equivalent to sizeof(value).
// in GNU compatible this is equivalent to sizeof(__typeof(value))
#define psb_sizeof(value)          psb_sizeof_implementation(value)

//takes in a stretchy buffer and removes the meta data from it
#define psb_unstretch(array)       psb_unstretch_implementation(array) 

#define psb_foreach(array, .../*[optiona] name*/)   psb_foreach_(array, __VA_ARGS__)
#define psb_foreach_r(array, .../*[optiona] name*/) psb_foreach_r_(array, __VA_ARGS__)
#define psb_foreach_i(array, .../*[optiona] name*/) psb_foreach_i_(array, __VA_ARGS__)


























#if defined(PSTD_USE_ALLOCATOR) && !defined(PSTD_ALLOCATOR_DEFINED)
#define PSTD_ALLOCATOR_DEFINED
enum pallocation_kind_t {
    ALLOCATE,
    ZERO_ALLOCATE,
    REALLOCATE,
    FREE,
    SIZED_FREE,
};
typedef enum pallocation_kind_t pallocation_kind_t;
typedef struct pallocator_t pallocator_t;
struct pallocator_t {
    void *(*allocator)(pallocator_t *, pallocation_kind_t kind, usize size, void *buffer); 
    void *user_data;
};

void *pdefault_allocator(
        pallocator_t *, pallocation_kind_t kind, usize size, void *buffer) 
{
    switch(kind) {
    case ALLOCATE:       return pallocate(size);
    case ZERO_ALLOCATE:  return pzero_allocate(size);
    case REALLOCATE:     return preallocate(size, buffer);
    case FREE:           return (pfree(buffer), NULL); // NOLINT
    case SIZED_FREE:     return (psized_free(size, buffer), NULL);
    }
    return NULL;
}
const static pallocator_t PSTD_DEFAULT_HASH_MAP_ALLOCATOR = {
    .allocator = pdefault_allocator,
};
#endif

#if defined(PSTD_GNU_COMPATIBLE) // not an msvc compiler

#if defined(PSTD_USE_ALLOCATOR)
#define psb_create_implementation(data_type, allocator) ({                  \
     __auto_type _buf = psb_get_metadata(NULL, psb_sizeof(data_type), true, (allocator));  \
     (data_type *)(_buf + 1);                                                               \
     })
#endif

#define psb_free_implementation(array) \
    psb_free_buffer(psb_get_meta(array), psb_sizeof((array)[0]))

#define PSTD_FREE_ELEMENT(array, offset) ({                     \
    if (psb_get_meta(array)->free_element) {                    \
        psb_get_meta(array)->free_element((array) + (offset));  \
    }                                                           \
})

#define psb_set_free_func_implementation(array, free_func) ({   \
    psb_get_meta_or_create(array)->free_element = (free_func);  \
})

#define psb_sizeof_implementation(value) (sizeof(__typeof(value)))

#if defined(PSTD_USE_ALLOCATOR)
#define psb_get_meta_implementation(array) \
    ({ psb_get_metadata((array), psb_sizeof((array)[0]), false, PSTD_DEFAULT_ALLOCATOR); })
#else
#define psb_get_meta_implementation(array) \
    ({ psb_get_metadata((void*)(array), psb_sizeof((array)[0]), false); })
#endif

#if defined(PSTD_USE_ALLOCATOR)
#define psb_get_meta_or_create_implementation(array) ({                                         \
            pstretchy_buffer_t *psb_get_meta_meta =                                             \
            psb_get_metadata((array), psb_sizeof((array)[0]), true, PSTD_DEFAULT_ALLOCATOR);   \
            (array) = (__typeof(array))(psb_get_meta_meta + 1);                                 \
            psb_get_meta_meta;                                                                  \
        })
#else 
#define psb_get_meta_or_create_implementation(array) ({                 \
            pstretchy_buffer_t *psb_get_meta_meta =                     \
            psb_get_metadata((array), psb_sizeof((array)[0]), true);   \
            (array) = (__typeof(array))(psb_get_meta_meta + 1);         \
            psb_get_meta_meta;                                          \
        })
#endif

#define psb_size_implementation(array)   (psb_get_meta(array)->size) 

#define psb_set_capacity_implementation(array, count) \
    psb_set_capacity_implementation_(&(array), (count), psb_sizeof((array)[0]))

#define psb_reserve_implementation(array, count) \
    psb_reserve_implementation_(&(array), (count), psb_sizeof((array)[0]))


#define psb_pushback_implementation(array, value) ({                  \
    psb_get_meta_or_create(array);                                    \
    pMaybeGrowStretchyBuffer(&(array), psb_sizeof((array)[0]));       \
    __auto_type psb_pushback_ret = (array) + psb_size(array)++;       \
    *psb_pushback_ret = (value);                                      \
    psb_pushback_ret;                                                 \
})

#define psb_pushback_n_implementation(array, val_arr, count) ({                         \
    __auto_type psb_pushback_ret = (array);                                             \
    pmaybe_grow_n_elems(&(array), psb_sizeof((array)[0]), (count));                     \
    for (usize psb_pushback_n_i = 0; psb_pushback_n_i < (count); psb_pushback_n_i++) {  \
        psb_pushback_ret = &(array)[psb_size(array) + psb_pushback_n_i];                \
        *psb_pushback_ret = (val_arr)[psb_pushback_n_i];                                \
        psb_size(array)++;                                                              \
    }                                                                                   \
    psb_pushback_ret;                                                                   \
})

#define psb_pseudo_pushback_implementation(array, length, value) ({   \
    psb_pseudo_grow(&(array), psb_sizeof((array)[0]), length);        \
    __auto_type psb_pushback_ret = (array) + (length)++;              \
    *psb_pushback_ret = (value);                                      \
    psb_pushback_ret;                                                 \
})

#define psb_pushbytes_implementation(array, value, bytes) ({    \
    psb_get_meta_or_create(array);                              \
    pMaybeByteGrowStretchyBuffer(&(array), (bytes));            \
    memcpy((array) + psb_size((array)), (value), (bytes));      \
    __auto_type psb_pushback_ret = (array) + psb_size((array)); \
    psb_size((array)) += (bytes);                               \
    psb_pushback_ret;                                           \
})


#define psb_begin_implementation(array) ({ (array); })
#define psb_end_implementation(array) ({ (array) + psb_size(array); })

#define psb_begin_r_implementation(array) ({ (array) + psb_size(array) - 1; })
#define psb_end_r_implementation(array) ({ (array) - 1; })

#define psb_insert_implementation(array, position, value) ({                                    \
    __auto_type psb_insert_array = psb_get_meta_or_create(array);                               \
    usize psb_insert_size = psb_sizeof( value );                                                \
    usize psb_insert_offset = (position) - psb_begin(array);                                    \
    __typeof(value) *psb_insert_result = NULL;                                                  \
    if (psb_insert_array->size && psb_insert_offset >= psb_insert_array->size) {                \
        /*TODO: handle this properly*/                                                          \
        psb_insert_result = psb_pushback(array, value);                                         \
    }                                                                                           \
    else {                                                                                      \
        pMaybeGrowStretchyBuffer(&(array), psb_insert_size);                                    \
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
        usize psb_insert_elems = psb_size(array) - psb_insert_offset;                           \
        if (psb_insert_elems) {                                                                 \
            memmove((array) + psb_insert_offset + 1,                                            \
                    (array) + psb_insert_offset, psb_insert_elems * psb_insert_size);           \
        }                                                                                       \
                                                                                                \
        psb_get_meta(array)->size++;                                                            \
        (array)[psb_insert_offset] = value;                                                     \
        psb_insert_result = (array) + psb_insert_offset;                                        \
    }                                                                                           \
    psb_insert_result;                                                                          \
})


#define psb_popback_implementation(array) ({                            \
            __typeof((array)[0]) psb_popback_result = {0};              \
            if (psb_size(array) == 0) {}                                \
            else {                                                      \
                PSTD_FREE_ELEMENT(array, psb_size(array));              \
                psb_popback_result = (array)[(psb_size(array)--) - 1];  \
            }                                                           \
            psb_popback_result;                                         \
        })

#define psb_remove_implementation(array, position) ({                           \
    __typeof((array)[0]) psb_remove_result = (__typeof((array)[0])){0};         \
    usize psb_remove_offset = (position) - psb_begin(array);                    \
    if (psb_remove_offset >= psb_size(array)) {}                                \
    else if (psb_remove_offset == psb_size(array) - 1) {                        \
        PSTD_FREE_ELEMENT(array, psb_remove_offset);                            \
        psb_size(array)--;                                                      \
        psb_remove_result = (array)[psb_remove_offset];                         \
    } else {                                                                    \
        PSTD_FREE_ELEMENT(array, psb_remove_offset);                            \
        psb_remove_result = (array)[psb_remove_offset];                         \
        usize psb_remove_elems = (psb_size(array) - psb_remove_offset) - 1;     \
        memmove((array) + psb_remove_offset, (array) + psb_remove_offset + 1,   \
                psb_remove_elems * psb_sizeof((array)[0]));                     \
        psb_size(array)--;                                                      \
    }                                                                           \
    psb_remove_result;                                                          \
})

// arr:  another dynamic array
#define psb_copybuffer_implementation(arr) ({                                       \
    __auto_type psb_copybuffer_array  = psb_get_meta(array);                        \
    usize psb_copybuffer_size = psb_sizeof((arr)[0]) * psb_copybuffer_array->size;  \
    pstretchy_buffer_t *psb_copybuffer_copy =                                       \
        pallocate(sizeof(pstretchy_buffer_t) * psb_copybuffer_size)                 \
    psb_copybuffer_copy->endofstorage = psb_copybuffer_size;                        \
    psb_copybuffer_copy->size = psb_copybuffer_array->size;                         \
    memcpy((arr), psb_copybuffer_array + 1, psb_copybuffer_size;                    \
    (__typeof((arr)[0])*)( psb_copybuffer_copy + 1);                                \
})

// type: the type of array we want
// arr:  a static array
#define psb_copyarray_implementation(type, arr) ({          \
    type psb_copyarray_tmp = NULL;                          \
    psb_reserve(psb_copyarray_tmp, countof(arr));           \
    memcpy(psb_copyarray_tmp, (arr), sizeof(arr));          \
    psb_get_meta(psb_copyarray_tmp)->size = countof(arr);   \
    psb_copyarray_tmp;                                      \
})

#define psb_unstretch_implementation(array) ({                                          \
    pstretchy_buffer_t *psb_unstretch_meta = psb_get_meta(array);                       \
    void *psb_unstretch_result = memmove(psb_unstretch_meta, psb_unstretch_meta + 1,    \
            psb_unstretch_meta->size * psb_sizeof(*(array)));                           \
    array = psb_unstretch_result;/*NOLINT*/                                             \
    array;                                                                              \
})

#define psb_foreach_impl(array, name) \
    for( __auto_type name = psb_begin(array); name != psb_end(array); name++) //NOLINT
#define psb_foreach_(array, ...)   \
    psb_foreach_impl(array, PSTD_DEFAULT(__VA_ARGS__, it))

#define psb_foreach_i_impl(array, name) \
    for( __auto_type name = psb_end(array) - 1; name != psb_begin(array) - 1; name++) //NOLINT
#define psb_foreach_i_(array, ...)   \
    psb_foreach_i_impl(array, PSTD_DEFAULT(__VA_ARGS__, it))

typedef void pfree_func_t(void*);
typedef struct pstretchy_buffer_t pstretchy_buffer_t;
struct pstretchy_buffer_t {
#if defined(PSTD_USE_ALLOCATOR)
    pallocator_t cb;
#endif
    pfree_func_t *free_element;
    usize size;
    usize endofstorage;
#if PSTD_C99
    u8    buffer_data[];
#endif
};

// how many elements we should add
static void psb_byte_grow(void* array, usize bytes);
static void psb_grow(void* array, usize datasize, usize count);

PSTD_UNUSED
static void pMaybeByteGrowStretchyBuffer(void* array, usize bytes) {
    pstretchy_buffer_t** meta_ptr = (pstretchy_buffer_t**)array;
    pstretchy_buffer_t* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        psb_byte_grow(array, bytes);
    }
}

PSTD_UNUSED
static void pMaybeGrowStretchyBuffer(void* array, usize datasize) {
    pstretchy_buffer_t** meta_ptr = (pstretchy_buffer_t**)array;
    pstretchy_buffer_t* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        psb_grow(array, datasize, PSTD_STRETCHY_BUFFER_GROWTH_COUNT);
    }
}

PSTD_UNUSED
static void pmaybe_grow_n_elems(void *array, usize datasize, usize count) {
    pstretchy_buffer_t** meta_ptr = (pstretchy_buffer_t**)array;
    pstretchy_buffer_t* meta = (*meta_ptr) - 1;

    if ((meta->size + count) * datasize >= meta->endofstorage) {
        psb_grow(array, datasize, (meta->size + count));
    }
}

static void psb_byte_grow(void* array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8* array = *(u8**)array_ptr;
    pstretchy_buffer_t* meta = ((pstretchy_buffer_t*)array) - 1;
    usize size = sizeof(pstretchy_buffer_t) + meta->endofstorage + bytes;

#if defined(PSTD_USE_ALLOCATOR)
    assert(meta->cb.allocator);

    void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
    void *tmp = preallocate(size, meta);
#endif
    assert(tmp); meta = (pstretchy_buffer_t*)tmp;

    meta->endofstorage += bytes;
    *(u8**)array_ptr = (u8*)(meta + 1);
}

PSTD_UNUSED 
static void psb_pseudo_grow(void* array_ptr, usize datasize, usize count) {
    if (!array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    usize size = datasize * (count + 1);

    void *tmp = preallocate(size, array);
    assert(tmp);

    *(u8**)array_ptr = tmp;
}

static void psb_grow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    pstretchy_buffer_t* meta = ((pstretchy_buffer_t*)array) - 1;

#if defined(PSTD_USE_ALLOCATOR)
    assert(meta->cb.allocator);
#endif 

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(pstretchy_buffer_t) + array_size;

#if defined(PSTD_USE_ALLOCATOR)
    void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
    void *tmp = preallocate(size, meta);
#endif
    assert(tmp); meta = (pstretchy_buffer_t*)tmp;

    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (u8*)(meta + 1);
}

PSTD_UNUSED
#if defined(PSTD_USE_ALLOCATOR)
static pstretchy_buffer_t* psb_get_metadata(void* array, usize data_size, pbool_t create, pallocator_t cb) {
#else
static pstretchy_buffer_t* psb_get_metadata(void* array, usize data_size, pbool_t create) {//NOLINT
#endif
#if defined(PSTD_USE_ALLOCATOR)
    if (!cb.allocator) {
        cb.allocator = pdefault_allocator;
    }
#endif

    if (!array) {
        if (!create) { // should probably just return NULL
            static pstretchy_buffer_t nil = {0};
            nil = (pstretchy_buffer_t){0};
            return &nil;
        }
        pstretchy_buffer_t *meta;
#if defined(PSTD_USE_ALLOCATOR)
        meta = cb.allocator(&cb, ZERO_ALLOCATE, sizeof(pstretchy_buffer_t) + data_size, NULL);
        meta->cb = cb;
#else
        meta = pzero_allocate(sizeof(pstretchy_buffer_t) + data_size);
#endif
        meta->endofstorage = data_size;
        meta->size = 0;
        return meta;
    }

    return ((pstretchy_buffer_t*)array) - 1;
}

PSTD_UNUSED
static usize psb_set_capacity_implementation_(void *mem, usize count, usize data_size) {
    void **array_ptr = mem;
    if (!(*array_ptr)) {
        usize array_size = ((data_size) * (count));
        usize size = array_size + sizeof(pstretchy_buffer_t);
#if defined(PSTD_USE_ALLOCATOR)
        pallocator_t cb = PSTD_DEFAULT_ALLOCATOR;
        pstretchy_buffer_t *meta = cb.allocator(&cb, ZERO_ALLOCATE, size, NULL);
        meta->cb = cb;
#else
        pstretchy_buffer_t *meta = pzero_allocate(size);
#endif
        meta->endofstorage = array_size;
        (*array_ptr) = (void*)(meta + 1);
    } else {
        __auto_type meta = psb_get_meta(*array_ptr);

        usize size = ((data_size) * (count)) + sizeof(pstretchy_buffer_t);
#if defined(PSTD_USE_ALLOCATOR)
        assert(meta->cb.allocator);
        void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
        void *tmp = preallocate(size, meta);
#endif
        assert(tmp);
        meta = tmp;
        (*array_ptr) = (void*)(meta + 1);
    }
    return count;
}

PSTD_UNUSED
static usize psb_reserve_implementation_(void *mem, usize count, usize data_size) {
    void **array_ptr = mem;
    if (!(*array_ptr)) {
        usize array_size = ((data_size) * (count));
        usize size = array_size + sizeof(pstretchy_buffer_t);
#if defined(PSTD_USE_ALLOCATOR)
        pallocator_t cb = PSTD_DEFAULT_ALLOCATOR;
        pstretchy_buffer_t *meta = cb.allocator(&cb, ZERO_ALLOCATE, size, NULL);
#else
        pstretchy_buffer_t *meta = pzero_allocate(size);
#endif
        assert(meta);
        meta->endofstorage = array_size;
#if defined(PSTD_USE_ALLOCATOR)
        meta->cb           = cb;
#endif
        (*array_ptr)       = (void*)(meta + 1);
    } else if ((data_size) < (count)) {
        __auto_type meta = psb_get_meta(*array_ptr);
        usize size = ((data_size) * (count)) + sizeof(pstretchy_buffer_t);
#if defined(PSTD_USE_ALLOCATOR)
        void *tmp = meta->cb.allocator(&meta->cb, REALLOCATE, size, meta);
#else
        void *tmp = preallocate(size, meta);
#endif
        assert(tmp);
        meta = tmp;
        (*array_ptr) = (void*)(meta + 1);
    }
    return count;
}

PSTD_UNUSED
static inline void psb_free_buffer(pstretchy_buffer_t *meta, usize data_size) {
    if (meta->free_element) {
        u8 *data = (void*)(meta + 1);
        
        usize index = meta->size;
        do {
            index--;
            meta->free_element(data + data_size * index);
        } while(index != 0);
    }
#if defined(PSTD_USE_ALLOCATOR)
    meta->cb.allocator(&meta->cb, SIZED_FREE, sizeof(*meta) + meta->endofstorage, meta);
#else
    psized_free(sizeof(*meta) + meta->endofstorage, meta);
#endif
}

#else // using mvsc compiler
#endif
#endif // PSTD_STRETCHY_BUFFER_HEADER
