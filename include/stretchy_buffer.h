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

//takes in a stretchy buffer and removes the meta data from it
#define psb_unstretch(array) psb_unstretch_implementation(array) 

// frees strechy buffer
#define psb_free(array) psb_free_implementation(array)

// attaches a function to the buffer, that will be called on each element
// when the element is removed, equivalent to a destrutor in c++
#define psb_set_free_func(array, free_func) \
    psb_set_free_func_implementation(array, free_func)

// gets meta data for the array
#define psb_get_meta(array) psb_get_meta_implementation(array)

// gets meta data for the array. if the array is equal to NULL
// then it creates a new strechy buffer and assigns array to that
#define psb_get_meta_or_create(array) psb_get_meta_or_create_implementation(array)

// utility function that removes some warning in GNU compatible compilers.
// in MSVC this is equivalent to sizeof(value).
// in GNU compatible this is equivalent to sizeof(__typeof(value))
#define psb_sizeof(value) psb_sizeof_implementation(value)

#define psb_foreach(array, .../*[optiona] name*/)   psb_foreach_implementation(array, __VA_ARGS__)
#define psb_foreach_r(array, .../*[optiona] name*/) psb_foreach_i_implementation(array, __VA_ARGS__)
#define psb_foreach_i(array, .../*[optiona] name*/) psb_foreach_i_implementation(array, __VA_ARGS__)

// get the number of elements currently stored in the array
#define psb_size(array)   psb_size_implementation(array)
#define psb_length(array) psb_size_implementation(array) 
#define psb_count(array)  psb_size_implementation(array) 

// sets the capacity of the current buffer to hold exactly 'count' elements
#define psb_resize(array, count)       psb_set_capacity_implementation(array, count)
#define psb_set_cap(array, count)      psb_set_capacity_implementation(array, count)
#define psb_set_count(array, count)    psb_set_capacity_implementation(array, count)
#define psb_set_size(array, count)     psb_set_capacity_implementation(array, count)
#define psb_set_length(array, count)   psb_set_capacity_implementation(array, count)
#define psb_set_capacity(array, count) psb_set_capacity_implementation(array, count)

// sets the capacity of the current buffer to hold at least 'count' elements
// if the current size of the buffer is greater than count this function does nothing
#define psb_reserve(array, count) psb_reserve_implementation(array, count)

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
#define psb_pushback(array, value) psb_pushback_implementation(array, value)

// appends 'count' elements from 'val_arr' to the end of the array and returns a pointer to the last element pushed
#define psb_pushback_n(array, val_arr, count) psb_pushback_n_implementation(array, val_arr, count)

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
// the difference between psb_pseudo_pushback and psb_pushback is that pseudo_pushback is intended
// for C arrays that do not have meta data, so an example would be
// ```c
//   int cur_length = 0;
//   int *array = NULL;
//   psb_pseudo_pushback(array, length, 1); length++;
//   psb_pseudo_pushback(array, length, 2); length++;
//   psb_pseudo_pushback(array, length, 3); length++;
// ```
// int this example the array pointer, note, however that this function is very naive
// and will reallocate each time it is called
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

// creates a copy of a strechy buffer and returns it
#define psb_copybuffer(array) psb_copybuffer_implementation(array)

// creates a copy of a static array as a strechy buffer and returns it
#define psb_copyarray(type, array) psb_copyarray_implementation(type, array)























#if defined(PSTD_GNU_COMPATIBLE) // not an msvc compiler

// this function frees the buffer, 
// if it has a free elements function it will also free the
// elements
#define psb_free_implementation(array) \
    psb_free_buffer(psb_get_meta(array), psb_sizeof((array)[0]))

// attaches a function to the buffer, that will be called on each element
// when the element is removed, equivalent to a destrutor in c++
#define psb_set_free_func_implementation(array, free_func) ({   \
    psb_get_meta_or_create(array)->free_element = (free_func);  \
})

// checks to see if the buffer has a free_element function attached
// if it does it calls it with the element at array + offset
#define pstd_free_element_implementation(array, offset) ({      \
    if (psb_get_meta(array)->free_element) {                    \
        psb_get_meta(array)->free_element((array) + (offset));  \
    }                                                           \
})

//takes in a stretchy buffer and removes the meta data from it
#define psb_unstretch_implementation(array) ({                                          \
    pstretchy_buffer_t *psb_unstretch_meta = psb_get_meta(array);                       \
    void *psb_unstretch_result = memmove(psb_unstretch_meta, psb_unstretch_meta + 1,    \
            psb_unstretch_meta->size * psb_sizeof(*(array)));                           \
    array = psb_unstretch_result;/*NOLINT*/                                             \
    array;                                                                              \
})


// utility function that gets the meta data for the current buffer
// this will never allocate a new buffer
#define psb_get_meta_implementation(array) ({                       \
    psb_get_metadata((void*)(array), psb_sizeof((array)[0]), false);\
})


// utility function that gets the meta data for the current buffer.
// if the buffer is uninitialised (i.e. NULL) this will create a new buffer
// with meta data
#define psb_get_meta_or_create_implementation(array) ({         \
    pstretchy_buffer_t *psb_get_meta_meta =                     \
    psb_get_metadata((array), psb_sizeof((array)[0]), true);    \
    (array) = (__typeof(array))(psb_get_meta_meta + 1);         \
    psb_get_meta_meta;                                          \
})

// utility function that removes some warning in GNU compatible compilers.
// in MSVC this is equivalent to sizeof(value).
// in GNU compatible this is equivalent to sizeof(__typeof(value))
#define psb_sizeof_implementation(value) (sizeof(__typeof(value)))

#define psb_foreach_implementation_(array, name) \
    for( __auto_type name = psb_begin(array); name != psb_end(array); name++) //NOLINT
#define psb_foreach_implementation(array, ...)   \
    psb_foreach_implementation_(array, PSTD_DEFAULT(__VA_ARGS__, it))

#define psb_foreach_i_implementation_(array, name) \
    for( __auto_type name = psb_end(array) - 1; name != psb_begin(array) - 1; name--) //NOLINT
#define psb_foreach_i_implementation(array, ...)   \
    psb_foreach_i_implementation_(array, PSTD_DEFAULT(__VA_ARGS__, it))

// gets the size of the buffer
#define psb_size_implementation(array)   (psb_get_meta(array)->size) 

// sets the capacity of the current buffer to hold exactly 'count' elements
#define psb_set_capacity_implementation(array, count) \
    psb_set_capacity_implementation_(&(array), (count), psb_sizeof((array)[0]))

// sets the capacity of the current buffer to hold at least 'count' elements
// if the current size of the buffer is greater than count this function does nothing
#define psb_reserve_implementation(array, count) \
    psb_reserve_implementation_(&(array), (count), psb_sizeof((array)[0]))


// appends 'value' to the end of the array and returns a pointer to the location of 'value'
#define psb_pushback_implementation(array, value) ({            \
    psb_get_meta_or_create(array);                              \
    psb_maybe_grow(&(array), psb_sizeof((array)[0]));           \
    __auto_type psb_pushback_ret = (array) + psb_size(array)++; \
    *psb_pushback_ret = (value);                                \
    psb_pushback_ret;                                           \
})


// appends 'count' elements from 'val_arr' to the end of the array and returns a pointer to the last element pushed
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

// appends 'value' to the end of the array and returns a pointer to the location of 'value'
// the difference between pseudo_pushback and pushback is that pseudo_pushback is intended
// for C arrays that do not have meta data, so an example would be
// ```c
//   int cur_length = 0;
//   int *array = NULL;
//   psb_pseudo_pushback(array, length, 1); length++;
//   psb_pseudo_pushback(array, length, 2); length++;
//   psb_pseudo_pushback(array, length, 3); length++;
// ```
// int this example the array pointer, note, however that this function is very naive
// and will reallocate each time it is called
#define psb_pseudo_pushback_implementation(array, length, value) ({   \
    psb_pseudo_grow(&(array), psb_sizeof((array)[0]), length);        \
    __auto_type psb_pushback_ret = (array) + (length)++;              \
    *psb_pushback_ret = (value);                                      \
    psb_pushback_ret;                                                 \
})

// same as psb_pushback but gives control over how many bytes needs to be inserted into the array
#define psb_pushbytes_implementation(array, value, bytes) ({    \
    psb_get_meta_or_create(array);                              \
    psb_maybe_byte_grow(&(array), (bytes));                     \
    memcpy((array) + psb_size((array)), (value), (bytes));      \
    __auto_type psb_pushback_ret = (array) + psb_size((array)); \
    psb_size((array)) += (bytes);                               \
    psb_pushback_ret;                                           \
})

// removes last element in array and returns a copy 
#define psb_popback_implementation(array) ({                        \
    __typeof((array)[0]) psb_popback_result = {0};                  \
    if (psb_size(array) == 0) {}                                    \
    else {                                                          \
        pstd_free_element_implementation(array, psb_size(array));   \
        psb_popback_result = (array)[(psb_size(array)--) - 1];      \
    }                                                               \
    psb_popback_result;                                             \
})

// inserts 'value' at 'position' in 'array' and returns a pointer to 'position' in the array
// note that this might invalidate the 'position' parameter so if you need to reuse
// 'position' you should call it thously
// location = psb_insert(array, location, 10)
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
        psb_maybe_grow(&(array), psb_insert_size);                                              \
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

// removes element at 'position' in 'array' and returns the data that was at 'position'
#define psb_remove_implementation(array, position) ({                           \
    __typeof((array)[0]) psb_remove_result = (__typeof((array)[0])){0};         \
    usize psb_remove_offset = (position) - psb_begin(array);                    \
    if (psb_remove_offset >= psb_size(array)) {}                                \
    else if (psb_remove_offset == psb_size(array) - 1) {                        \
        pstd_free_element_implementation(array, psb_remove_offset);             \
        psb_size(array)--;                                                      \
        psb_remove_result = (array)[psb_remove_offset];                         \
    } else {                                                                    \
        pstd_free_element_implementation(array, psb_remove_offset);             \
        psb_remove_result = (array)[psb_remove_offset];                         \
        usize psb_remove_elems = (psb_size(array) - psb_remove_offset) - 1;     \
        memmove((array) + psb_remove_offset, (array) + psb_remove_offset + 1,   \
                psb_remove_elems * psb_sizeof((array)[0]));                     \
        psb_size(array)--;                                                      \
    }                                                                           \
    psb_remove_result;                                                          \
})


// returns a pointer to the first element of the array
#define psb_begin_implementation(array) ({ (array); })
// returns a pointer to the element after the last element in the array
#define psb_end_implementation(array) ({ (array) + psb_size(array); })

// returns a pointer to the last element of the array
#define psb_begin_r_implementation(array) ({ (array) + psb_size(array) - 1; })
// returns a pointer to the element before the first element in the array
#define psb_end_r_implementation(array) ({ (array) - 1; })


// creates a copy of a strechy buffer and returns it
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

// creates a copy of a static array as a strechy buffer and returns it
#define psb_copyarray_implementation(type, arr) ({          \
    type psb_copyarray_tmp = NULL;                          \
    psb_reserve(psb_copyarray_tmp, countof(arr));           \
    memcpy(psb_copyarray_tmp, (arr), sizeof(arr));          \
    psb_get_meta(psb_copyarray_tmp)->size = countof(arr);   \
    psb_copyarray_tmp;                                      \
})

typedef void pfree_func_t(void*);
typedef struct pstretchy_buffer_t pstretchy_buffer_t;
struct pstretchy_buffer_t {
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
static void psb_maybe_byte_grow(void* array, usize bytes) {
    pstretchy_buffer_t** meta_ptr = (pstretchy_buffer_t**)array;
    pstretchy_buffer_t* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        psb_byte_grow(array, bytes);
    }
}

PSTD_UNUSED
static void psb_maybe_grow(void* array, usize datasize) {
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

    void *tmp = preallocate(size, meta);
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

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(pstretchy_buffer_t) + array_size;

    void *tmp = preallocate(size, meta);
    assert(tmp); meta = (pstretchy_buffer_t*)tmp;

    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (u8*)(meta + 1);
}

PSTD_UNUSED
static pstretchy_buffer_t* psb_get_metadata(void* array, usize data_size, bool create) {//NOLINT
    if (!array) {
        if (!create) { // should probably just return NULL
            static pstretchy_buffer_t nil = {0};
            nil = (pstretchy_buffer_t){0};
            return &nil;
        }
        pstretchy_buffer_t *meta;
        meta = pzero_allocate(sizeof(pstretchy_buffer_t) + data_size);
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
        pstretchy_buffer_t *meta = pzero_allocate(size);
        meta->endofstorage = array_size;
        (*array_ptr) = (void*)(meta + 1);
    } else {
        __auto_type meta = psb_get_meta(*array_ptr);

        usize size = ((data_size) * (count)) + sizeof(pstretchy_buffer_t);
        void *tmp = preallocate(size, meta);
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
        pstretchy_buffer_t *meta = pzero_allocate(size);
        assert(meta);
        meta->endofstorage = array_size;
        (*array_ptr)       = (void*)(meta + 1);
    } else if ((data_size) < (count)) {
        __auto_type meta = psb_get_meta(*array_ptr);
        usize size = ((data_size) * (count)) + sizeof(pstretchy_buffer_t);
        void *tmp = preallocate(size, meta);
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
    psized_free(sizeof(*meta) + meta->endofstorage, meta);
}

#else // using mvsc compiler
#endif
#endif // PSTD_STRETCHY_BUFFER_HEADER
