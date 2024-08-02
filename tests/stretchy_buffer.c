#include "stretchy_buffer.h"
#include "deps/munit.h"

static MunitResult
test_stretchy_buffer_set_capacity(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_set_capacity(vec, 10);
    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 0);
    munit_assert_uint64(meta->endofstorage, ==, 10 * sizeof(int));

    psb_set_capacity(vec, 5);
    meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 0);
    munit_assert_uint64(meta->endofstorage, ==, 5 * sizeof(int));

    psb_free(vec);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_reserve(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_reserve(vec, 10);
    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 0);
    munit_assert_uint64(meta->endofstorage, ==, 10 * sizeof(int));

    psb_reserve(vec, 5);
    meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 0);
    munit_assert_uint64(meta->endofstorage, ==, 10 * sizeof(int));

    psb_free(vec);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_free(const MunitParameter[], void*) {
    int *stretchy vec = NULL;
    psb_set_capacity(vec, 1);
    
    psb_free(vec);
    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 0);
    munit_assert_uint64(meta->endofstorage, ==, 0 * sizeof(int));
    munit_assert_ptr(vec, ==, nullptr);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_double_free(const MunitParameter[], void*) {
    int *stretchy vec = NULL;
    psb_set_capacity(vec, 1);
    
    psb_free(vec);
    
    auto first_free_meta = psb_get_meta(vec);
    munit_assert_uint64(first_free_meta->size, ==, 0);
    munit_assert_uint64(first_free_meta->endofstorage, ==, 0 * sizeof(int));
    munit_assert_ptr(vec, ==, nullptr);

    psb_free(vec);
    auto second_free_meta = psb_get_meta(vec);
    munit_assert_uint64(second_free_meta->size, ==, 0);
    munit_assert_uint64(second_free_meta->endofstorage, ==, 0 * sizeof(int));
    munit_assert_ptr(vec, ==, nullptr);

    return MUNIT_OK;
}

static int num_freed = 0;
static int freed_values[8];
void test_stretchy_buffer_free_func_count_frees(void *obj) {
    int *value = obj;
    freed_values[num_freed++] = *value;
}

static MunitResult
test_stretchy_buffer_free_func(const MunitParameter[], void*) {
    int *stretchy vec = NULL;
    psb_set_free_func(vec, test_stretchy_buffer_free_func_count_frees); 
    memset(freed_values, 0, sizeof freed_values);

    psb_pushback(vec, 1);
    psb_pushback(vec, 2);
    psb_pushback(vec, 3);
    psb_pushback(vec, 4);

    psb_free(vec);

    munit_assert_int(num_freed, ==, 4);
    munit_assert_ptr_equal(vec, NULL);
    if (num_freed >= 4) {
        munit_assert_int(freed_values[0], ==, 4);
        munit_assert_int(freed_values[1], ==, 3);
        munit_assert_int(freed_values[2], ==, 2);
        munit_assert_int(freed_values[3], ==, 1);
    }
    
    psb_free(vec);
    munit_assert_int(num_freed, ==, 4);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_copy(const MunitParameter[], void*) {
    int *copied_array = psb_copyarray(int *, ((int[]){ 1, 2, 3, 4}));
    auto copied_array_meta = psb_get_meta(copied_array);
    
    munit_assert_uint64(copied_array_meta->size, ==, 4);
    munit_assert_uint64(copied_array_meta->endofstorage, ==, 4 * sizeof(int));
    munit_assert_int(copied_array[0], ==, 1);
    munit_assert_int(copied_array[1], ==, 2);
    munit_assert_int(copied_array[2], ==, 3);
    munit_assert_int(copied_array[3], ==, 4);

    int *copied_buffer = psb_copybuffer(copied_array);
    auto copied_buffer_meta = psb_get_meta(copied_buffer);
    
    munit_assert_uint64(copied_buffer_meta->size, ==, 4);
    munit_assert_uint64(copied_buffer_meta->endofstorage, ==, 4 * sizeof(int));
    munit_assert_int(copied_buffer[0], ==, 1);
    munit_assert_int(copied_buffer[1], ==, 2);
    munit_assert_int(copied_buffer[2], ==, 3);
    munit_assert_int(copied_buffer[3], ==, 4);

    psb_free(copied_array);
    psb_free(copied_buffer);

    return MUNIT_OK;
}


static MunitResult
test_stretchy_buffer_pushback(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_pushback(vec, 1);
    pstretchy_buffer_t *pushback_once = psb_get_meta(vec);
    munit_assert_uint64(pushback_once->size, ==, 1);
    munit_assert_uint64(pushback_once->endofstorage, ==, 1 * sizeof(int));
    munit_assert_int(vec[0], ==, 1);
    
    psb_pushback(vec, 2);
    pstretchy_buffer_t *pushback_twice = psb_get_meta(vec);
    munit_assert_uint64(pushback_twice->size, ==, 2);
    munit_assert_uint64(pushback_twice->endofstorage, ==, 2 * sizeof(int));
    munit_assert_int(vec[1], ==, 2);
    
    psb_pushback(vec, 3);
    pstretchy_buffer_t *pushback_thrice = psb_get_meta(vec);
    munit_assert_uint64(pushback_thrice->size, ==, 3);
    munit_assert_uint64(pushback_thrice->endofstorage, ==, 4 * sizeof(int));
    munit_assert_int(vec[2], ==, 3);
    
    psb_pushback(vec, 4);
    psb_pushback(vec, 5);
    pstretchy_buffer_t *pushback_five_times = psb_get_meta(vec);
    munit_assert_uint64(pushback_five_times->size, ==, 5);
    munit_assert_uint64(pushback_five_times->endofstorage, ==, 8 * sizeof(int));
    munit_assert_int(vec[3], ==, 4);
    munit_assert_int(vec[4], ==, 5);

    psb_free(vec);
    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_push_back_n(const MunitParameter[], void*) {

    {
        int *stretchy from_empty = NULL;
        int arr[] = { 1, 2, 3, 4};

        psb_pushback_n(from_empty, countof(arr), arr);
        auto from_empty_meta = psb_get_meta(from_empty);
        munit_assert_uint64(from_empty_meta->size, ==, 4);
        munit_assert_uint64(from_empty_meta->endofstorage, ==, 4 * sizeof(int));
        munit_assert_int(from_empty[0], ==, 1);
        munit_assert_int(from_empty[1], ==, 2);
        munit_assert_int(from_empty[2], ==, 3);
        munit_assert_int(from_empty[3], ==, 4);

        psb_free(from_empty);
    }
    
    {
        int *stretchy vec = NULL;
        int arr[] = { 2, 3, 4 };

        psb_pushback(vec, 1);
        psb_pushback_n(vec, countof(arr), arr);
        auto meta = psb_get_meta(vec);
        munit_assert_uint64(meta->size, ==, 4);
        munit_assert_uint64(meta->endofstorage, ==, 4 * sizeof(int));
        munit_assert_int(vec[0], ==, 1);
        munit_assert_int(vec[1], ==, 2);
        munit_assert_int(vec[2], ==, 3);
        munit_assert_int(vec[3], ==, 4);
        
        psb_free(vec);
    }

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_pseudo_pushback(const MunitParameter[], void*) {

    int length = 0;
    int *array = NULL;
    psb_pseudo_pushback(array, length, 1);
    munit_assert_not_null(array);
    munit_assert_int(length  , ==, 1);
    munit_assert_int(array[0], ==, 1);

    psb_pseudo_pushback(array, length, 2);
    munit_assert_int(length  , ==, 2);
    munit_assert_int(array[1], ==, 2);

    psb_pseudo_pushback(array, length, 3);
    munit_assert_int(length  , ==, 3);
    munit_assert_int(array[2], ==, 3);

    pfree(array);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_popback(const MunitParameter[], void*) {
    int *stretchy vec = NULL;
    psb_pushback(vec, 1);
    int popped_value = psb_popback(vec);
    auto popback_meta = psb_get_meta(vec);
    munit_assert_uint64(popback_meta->size, ==, 0);
    munit_assert_uint64(popback_meta->endofstorage, ==, 1 * sizeof(int));
    munit_assert_int(popped_value, ==, 1);
    
    psb_free(vec);
    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_pushbytes(const MunitParameter[], void*) {
    char example[] = "hi there";
    char *vec = NULL;

    psb_pushbytes(vec, sizeof example - 1, example);
    auto pushbytes_meta = psb_get_meta(vec);
    munit_assert_uint64(pushbytes_meta->size, ==, 8);
    munit_assert_uint64(pushbytes_meta->endofstorage, ==, 8);
    munit_assert_char(vec[0], ==, 'h');
    munit_assert_char(vec[1], ==, 'i');
    munit_assert_char(vec[2], ==, ' ');
    munit_assert_char(vec[3], ==, 't');
    munit_assert_char(vec[4], ==, 'h');
    munit_assert_char(vec[5], ==, 'e');
    munit_assert_char(vec[6], ==, 'r');
    munit_assert_char(vec[7], ==, 'e');

    psb_free(vec);
    return MUNIT_OK;
}


static MunitResult
test_stretchy_buffer_front_insertion(const MunitParameter[], void*) {
    {
        int *stretchy insert_while_empty = NULL;
        int *stretchy empty_position = NULL;
        psb_insert(insert_while_empty, empty_position, 1);
        auto insert_while_empty_meta = psb_get_meta(insert_while_empty);
        munit_assert_uint64(insert_while_empty_meta->size, ==, 1);
        munit_assert_uint64(insert_while_empty_meta->endofstorage, ==, 1 * sizeof(int));
        munit_assert_int(insert_while_empty[0], ==, 1);
        
        psb_free(insert_while_empty);
    }

    {
        int *stretchy vec = NULL;
        psb_pushback(vec, 2);
        
        int *start = psb_begin(vec);
        psb_insert(vec, start, 1);
        auto meta = psb_get_meta(vec);
        munit_assert_uint64(meta->size, ==, 2);
        munit_assert_uint64(meta->endofstorage, ==, 2 * sizeof(int));
        munit_assert_int(vec[0], ==, 1);
        munit_assert_int(vec[1], ==, 2);
        
        psb_free(vec);
    }

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_back_insertion(const MunitParameter[], void*) {
    {
        int *stretchy insert_while_empty = NULL;
        int *end_ptr = psb_end(insert_while_empty);
        int *empty_position = end_ptr - 1;

        psb_insert(insert_while_empty, empty_position, 1);
        auto insert_while_empty_meta = psb_get_meta(insert_while_empty);
        munit_assert_uint64(insert_while_empty_meta->size, ==, 1);
        munit_assert_uint64(insert_while_empty_meta->endofstorage, ==, 1 * sizeof(int));
        munit_assert_int(insert_while_empty[0], ==, 1);

        psb_free(insert_while_empty);
    }

    {
        
        int *stretchy insert_before_end = NULL;
        psb_pushback(insert_before_end, 2);
        
        int *before_end = psb_end(insert_before_end) - 1;
        psb_insert(insert_before_end, before_end, 1);
        auto insert_before_end_meta = psb_get_meta(insert_before_end);
        munit_assert_uint64(insert_before_end_meta->size, ==, 2);
        munit_assert_uint64(insert_before_end_meta->endofstorage, ==, 2 * sizeof(int));
        munit_assert_int(insert_before_end[0], ==, 1);
        munit_assert_int(insert_before_end[1], ==, 2);
        
        psb_free(insert_before_end);
    }
    
    {
        int *stretchy insert_at_end = NULL;
        psb_pushback(insert_at_end, 1);
        
        int *at_end = psb_end(insert_at_end);
        psb_insert(insert_at_end, at_end, 2);
        auto meta = psb_get_meta(insert_at_end);
        munit_assert_uint64(meta->size, ==, 2);
        munit_assert_uint64(meta->endofstorage, ==, 2 * sizeof(int));
        munit_assert_int(insert_at_end[0], ==, 1);
        munit_assert_int(insert_at_end[1], ==, 2);
        
        psb_free(insert_at_end);
    }
    
    {
        int *stretchy insert_after_end = NULL;
        psb_pushback(insert_after_end, 1);
        
        int *after_end = psb_end(insert_after_end) + 1;
        int *invalid_insert = psb_insert(insert_after_end, after_end, 2);
        auto insert_after_end_meta = psb_get_meta(insert_after_end);
        munit_assert_uint64(insert_after_end_meta->size, ==, 1);
        munit_assert_uint64(insert_after_end_meta->endofstorage, ==, 1 * sizeof(int));
        munit_assert_ptr_equal(invalid_insert, NULL);
        munit_assert_int(insert_after_end[0], ==, 1);
        
        psb_free(insert_after_end);
    }

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_middle_insertion(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_pushback(vec, 1);
    psb_pushback(vec, 2);
    psb_pushback(vec, 4);

    int *position = psb_begin(vec) + 2;
    psb_insert(vec, position, 3);
    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 4);
    munit_assert_uint64(meta->endofstorage, ==, 4 * sizeof(int));
    munit_assert_int(vec[0], ==, 1);
    munit_assert_int(vec[1], ==, 2);
    munit_assert_int(vec[2], ==, 3);
    munit_assert_int(vec[3], ==, 4);
    psb_free(vec);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_remove(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_pushback(vec,  1);
    psb_pushback(vec, 90);
    psb_pushback(vec,  2);

    int removed = psb_remove(vec, &vec[1]);
    
    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 2);
    munit_assert_uint64(meta->endofstorage, ==, 4 * sizeof(int));
    munit_assert_int(vec[0],  ==, 1);
    munit_assert_int(vec[1],  ==, 2);
    munit_assert_int(removed, ==, 90);
    
    psb_free(vec);
    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_unstretch(const MunitParameter[], void*) {
    int *stretchy vec = NULL;

    psb_pushback(vec, 1);
    psb_pushback(vec, 2);
    psb_pushback(vec, 3);
    psb_pushback(vec, 4);

    auto meta = psb_get_meta(vec);
    psb_unstretch(vec);

    munit_assert_ptr_equal(vec, meta);
    munit_assert_int(vec[0], ==, 1);
    munit_assert_int(vec[1], ==, 2);
    munit_assert_int(vec[2], ==, 3);
    munit_assert_int(vec[3], ==, 4);

    return MUNIT_OK;
}

static MunitResult
test_stretchy_buffer_user_type(const MunitParameter[], void*) {
    struct vec3 { int x, y, z; };
        
    struct vec3 *stretchy vec = NULL;
    psb_pushback(vec, ((struct vec3){ 1, 1, 1 }));
    psb_pushback(vec, ((struct vec3){ 2, 2, 2 }));
    psb_pushback(vec, ((struct vec3){ 4, 4, 4 }));

    struct vec3 *stretchy insert_after_two = psb_begin(vec) + 2;
    psb_insert(vec, insert_after_two, ((struct vec3){ 3, 3, 3 }));

    auto meta = psb_get_meta(vec);
    munit_assert_uint64(meta->size, ==, 4);
    munit_assert_uint64(meta->endofstorage, ==, 4 * sizeof(struct vec3));

#define assert_vec3(lhs, op, rhs)           \
    munit_assert_int((lhs).x, op, (rhs).x); \
    munit_assert_int((lhs).y, op, (rhs).y); \
    munit_assert_int((lhs).z, op, (rhs).z)

    assert_vec3(vec[0], ==, ((struct vec3){ 1, 1, 1 }));
    assert_vec3(vec[1], ==, ((struct vec3){ 2, 2, 2 }));
    assert_vec3(vec[2], ==, ((struct vec3){ 3, 3, 3 }));
    assert_vec3(vec[3], ==, ((struct vec3){ 4, 4, 4 }));
#undef assert_vec3
    psb_free(vec);

    return MUNIT_OK;
}

static MunitTest PTSD_STRETCHY_BUFFER_TESTS[] = {
    { "/set-capacity"    , test_stretchy_buffer_set_capacity    , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/reserve"         , test_stretchy_buffer_reserve         , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/free"            , test_stretchy_buffer_free            , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/double-free"     , test_stretchy_buffer_double_free     , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/free_func"       , test_stretchy_buffer_free_func       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/copy"            , test_stretchy_buffer_copy            , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/pushback"        , test_stretchy_buffer_pushback        , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/push_back_n"     , test_stretchy_buffer_push_back_n     , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/pseudo_pushback" , test_stretchy_buffer_pseudo_pushback , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/popback"         , test_stretchy_buffer_popback         , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/pushbytes"       , test_stretchy_buffer_pushbytes       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/front" , test_stretchy_buffer_front_insertion , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/middle", test_stretchy_buffer_middle_insertion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/back"  , test_stretchy_buffer_back_insertion  , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/remove"          , test_stretchy_buffer_remove          , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/unstretch"       , test_stretchy_buffer_unstretch       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/user-type"       , test_stretchy_buffer_user_type       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_STRETCHY_BUFFER_SUITE = {
    "/stretchybuffer", 
    PTSD_STRETCHY_BUFFER_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};
