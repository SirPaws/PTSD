#include "general.h"

#define pul_get_meta(list)                        pul_get_meta_implementation(list)
#define pul_get_or_create_meta(list)              pul_get_or_create_meta_implementation(list)
#define pul_default_compare(list_ptr, size, a, b) pul_default_compare_implementation(list_ptr, size, a, b)
#define pul_sizeof(x)                             pul_sizeof_implementation(x)
#define pul_contains(list, value)                 pul_contains_implementation(list, value)
#define pul_insert(list, value)                   pul_insert_implementation(list, value)
#define pul_sort(list)                            pul_sort_implementation(list)
#define pul_size(list)                      pul_size_implementation(list)
#define pul_set_compare_func(list, compare) pul_set_compare_func_implementation(list, compare)
#define pul_free(list)                      pul_free_implementation(list)

enum pul_ordering_t 
#if PTSD_C_VERSION >= PTSD_C23
    : int
#endif
{
    PUL_LESS    = -1,
    PUL_EQUAL   =  0,
    PUL_GREATER =  1
};
typedef enum pul_ordering_t pul_ordering_t;

typedef pul_ordering_t pul_compare_func_t(void *, usize, void *, void *);

typedef struct punique_list_t punique_list_t;
struct punique_list_t {
    pul_compare_func_t *cmp;
    usize size;
    usize storage_size;
};

PTSD_UNUSED static inline pul_ordering_t pul_compare_i(void *list_ptr,  usize size, int          *a, int          *b);
PTSD_UNUSED static inline pul_ordering_t pul_compare_u(void *list_ptr,  usize size, unsigned int *a, unsigned int *b);
PTSD_UNUSED static inline pul_ordering_t pul_compare_f(void *list_ptr,  usize size, float        *a, float        *b);
PTSD_UNUSED static inline pul_ordering_t pul_compare_d(void *list_ptr,  usize size, double       *a, double       *b);
PTSD_UNUSED static inline pul_ordering_t pul_compare_m(void *list_ptr,  usize size, void         *a, void         *b);

PTSD_UNUSED static inline pul_ordering_t pul_compare_i(PTSD_UNUSED void *list_ptr, PTSD_UNUSED usize size, int *a, int *b) {
    return (*a) - (*b);
}
PTSD_UNUSED static inline pul_ordering_t pul_compare_u(PTSD_UNUSED void *list_ptr, PTSD_UNUSED usize size, unsigned int *a, unsigned int *b) {
    return (*a) - (*b);
}
PTSD_UNUSED static inline pul_ordering_t pul_compare_f(PTSD_UNUSED void *list_ptr, PTSD_UNUSED usize size, float *a, float *b) {
    return (*a) - (*b);
}
PTSD_UNUSED static inline pul_ordering_t pul_compare_d(PTSD_UNUSED void *list_ptr, PTSD_UNUSED usize size, double *a, double *b) {
    return (*a) - (*b);
}
PTSD_UNUSED static inline pul_ordering_t pul_compare_m(PTSD_UNUSED void *list_ptr, usize size, void *a, void *b) {
    return memcmp(a, b, size);
}

#define pul_sizeof_implementation(x) sizeof(__typeof(x))

#if PTSD_C_VERSION >= PTSD_C11
#define pul_default_compare_implementation(list_ptr, size, a, b)\
    _Generic(*(list_ptr),                                       \
        int          *: pul_compare_i,                          \
        unsigned int *: pul_compare_u,                          \
        float        *: pul_compare_f,                          \
        double       *: pul_compare_d,                          \
        default       : pul_compare_m                           \
    )((list_ptr), sizeof(**(list_ptr)), (a), (b))
#else
#define pul_default_compare_implementation pul_compare_m
#endif

#define pul_contains_implementation(list, value) ({                                                                                     \
    __auto_type pul_contains_value = (value);                                                                                           \
    bool pul_contains_result = false;                                                                                                   \
    if ((list) && pul_get_meta(list)->cmp) {                                                                                            \
        for (usize pul_contains_index = 0; pul_contains_index < pul_size(list); pul_contains_index++) {                                 \
            if (pul_get_meta(list)->cmp(&(list), pul_sizeof(value), (list) + pul_contains_index, &pul_contains_value) == PUL_EQUAL) {   \
                pul_contains_result = true;                                                                                             \
                break;                                                                                                                  \
            }                                                                                                                           \
        }                                                                                                                               \
    } else {                                                                                                                            \
        for (usize pul_contains_index = 0; pul_contains_index < pul_size(list); pul_contains_index++) {                                 \
            if (pul_default_compare(&(list), pul_sizeof(value), (list) + pul_contains_index, &pul_contains_value) == PUL_EQUAL) {       \
                pul_contains_result = true;                                                                                             \
                break;                                                                                                                  \
            }                                                                                                                           \
        }                                                                                                                               \
    }                                                                                                                                   \
    pul_contains_result;                                                                                                                \
})

#define pul_insert_implementation(list, value) ({                                               \
    punique_list_t *pul_insert_meta = pul_get_or_create_meta(list);                             \
    if (!pul_contains(list, value)) {                                                           \
        if ((pul_insert_meta->size + 1) * pul_sizeof (value) > pul_insert_meta->storage_size) { \
            usize pul_insert_new_size = (pul_insert_meta->size ? pul_insert_meta->size : 1) * 2;\
            pul_insert_new_size *= sizeof *(list);                                              \
            pul_insert_new_size += sizeof *pul_insert_meta;                                     \
            void *pul_insert_tmp = realloc(pul_insert_meta, pul_insert_new_size);               \
            assert(pul_insert_tmp); pul_insert_meta = pul_insert_tmp;                           \
            pul_insert_meta->storage_size = pul_insert_new_size - sizeof *pul_insert_meta;      \
            (list) = (__typeof(list))(pul_insert_meta + 1);                                     \
        }                                                                                       \
        (list)[pul_insert_meta->size++] = value;                                                \
    }                                                                                           \
})

#define pul_get_meta_implementation(list) ({\
    ((punique_list_t*)(list)) - 1;          \
})

#define pul_get_or_create_meta_implementation(list) ({                                                              \
    punique_list_t *pul_get_or_create_meta_meta;                                                                    \
    if (list) {                                                                                                     \
        pul_get_or_create_meta_meta = pul_get_meta(list);                                                           \
    } else {                                                                                                        \
        pul_get_or_create_meta_meta = pzero_allocate(sizeof *pul_get_or_create_meta_meta + (sizeof *(list)) * 2);   \
        assert(pul_get_or_create_meta_meta);                                                                        \
        pul_get_or_create_meta_meta->storage_size = (sizeof *(list)) * 2;                                           \
        (list) = (void*)(pul_get_or_create_meta_meta + 1);                                                          \
    }                                                                                                               \
    pul_get_or_create_meta_meta;                                                                                    \
})

#define pul_size_implementation(list) ({    \
    usize length;                           \
    if (!(list))                            \
         length = 0;                        \
    else length = pul_get_meta(list)->size; \
    length;                                 \
})

#define pul_free_implementation(list) ({\
    free(pul_get_meta(list));           \
    (list) = NULL;                      \
})

#define pul_set_compare_func_implementation(list, compare) ({                           \
    pul_compare_func_t *pul_set_compare_func_old = pul_get_or_create_meta(list)->cmp;   \
    pul_get_meta(list)->cmp = (compare);                                                \
    pul_set_compare_func_old;                                                           \
})


#define pul_sort_implementation(list) ({                                                                            \
    punique_list_t *pul_sort_meta = pul_get_meta(list);                                                             \
    if (pul_sort_meta->size > 1) {                                                                                  \
        usize pul_sort_left_index = 0, pul_sort_right_index = 1;                                                    \
        if ((list) && pul_sort_meta->cmp) {                                                                         \
            while (pul_sort_left_index < pul_sort_meta->size - 1) {                                                 \
                __auto_type pul_sort_left  = (list) + pul_sort_left_index;                                          \
                __auto_type pul_sort_right = (list) + pul_sort_right_index;                                         \
                if (pul_sort_meta->cmp(&(list), pul_sizeof(*pul_sort_left), pul_sort_left, pul_sort_right) == PUL_GREATER) { \
                    __auto_type pul_sort_tmp = *pul_sort_left;                                                      \
                    *pul_sort_left = *pul_sort_right;                                                               \
                    *pul_sort_right = pul_sort_tmp;                                                                 \
                }                                                                                                   \
                                                                                                                    \
                pul_sort_right_index++;                                                                             \
                if (pul_sort_right_index == pul_sort_meta->size) {                                                  \
                    pul_sort_left_index++;                                                                          \
                    pul_sort_right_index = pul_sort_left_index + 1;                                                 \
                }                                                                                                   \
            }                                                                                                       \
        } else {                                                                                                    \
            while (pul_sort_left_index < pul_sort_meta->size - 1) {                                                 \
                __auto_type pul_sort_left  = (list) + pul_sort_left_index;                                          \
                __auto_type pul_sort_right = (list) + pul_sort_right_index;                                         \
                if (pul_default_compare(&(list), pul_sizeof(*pul_sort_left), pul_sort_left, pul_sort_right) == PUL_GREATER) {\
                    __auto_type pul_sort_tmp = *pul_sort_left;                                                      \
                    *pul_sort_left = *pul_sort_right;                                                               \
                    *pul_sort_right = pul_sort_tmp;                                                                 \
                }                                                                                                   \
                                                                                                                    \
                pul_sort_right_index++;                                                                             \
                if (pul_sort_right_index == pul_sort_meta->size) {                                                  \
                    pul_sort_left_index++;                                                                          \
                    pul_sort_right_index = pul_sort_left_index + 1;                                                 \
                }                                                                                                   \
            }                                                                                                       \
        }                                                                                                           \
    }                                                                                                               \
})


