#include "dynarray.h"
#include "deps/munit.h"


static MunitResult
test_dynarray_set_capacity(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    pda_set_capacity(&vec, 10);
    munit_assert_uint64(vec.size, ==, 0);
    munit_assert_uint64(vec.end_of_storage, ==, 10);
    pda_free(&vec);

    return MUNIT_OK;
}

static MunitResult
test_dynarray_free(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    pda_set_capacity(&vec, 1);
    
    pda_free(&vec);
    munit_assert_uint64(vec.size, ==, 0);
    munit_assert_uint64(vec.end_of_storage, ==, 0);
    munit_assert_ptr(vec.data, ==, nullptr);

    return MUNIT_OK;
}

static MunitResult
test_dynarray_double_free(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    pda_set_capacity(&vec, 1);
    
    pda_free(&vec);
    munit_assert_uint64(vec.size, ==, 0);
    munit_assert_uint64(vec.end_of_storage, ==, 0);
    munit_assert_ptr(vec.data, ==, nullptr);

    pda_free(&vec);
    munit_assert_uint64(vec.size, ==, 0);
    munit_assert_uint64(vec.end_of_storage, ==, 0);
    munit_assert_ptr(vec.data, ==, nullptr);

    return MUNIT_OK;
}

static MunitResult
test_dynarray_copy(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = pda_copy_array(struct intv, ((int[]){ 1, 2, 3, 4}));
    
    munit_assert_uint64(vec.size, ==, 4);
    munit_assert_uint64(vec.end_of_storage, ==, 4);
    munit_assert_int(vec.data[0], ==, 1);
    munit_assert_int(vec.data[1], ==, 2);
    munit_assert_int(vec.data[2], ==, 3);
    munit_assert_int(vec.data[3], ==, 4);

    struct intv vec2 = pda_copy_dynarray(vec);
    
    munit_assert_uint64(vec2.size, ==, 4);
    munit_assert_uint64(vec2.end_of_storage, ==, 4);
    munit_assert_int(vec2.data[0], ==, 1);
    munit_assert_int(vec2.data[1], ==, 2);
    munit_assert_int(vec2.data[2], ==, 3);
    munit_assert_int(vec2.data[3], ==, 4);

    pda_free(&vec);
    pda_free(&vec2);

    return MUNIT_OK;
}


static MunitResult
test_dynarray_pushback(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    pda_pushback(&vec, 1);
    munit_assert_uint64(vec.size, ==, 1);
    munit_assert_uint64(vec.end_of_storage, ==, 2);
    munit_assert_int(vec.data[0], ==, 1);
    
    pda_pushback(&vec, 2);
    munit_assert_uint64(vec.size, ==, 2);
    munit_assert_uint64(vec.end_of_storage, ==, 2);
    munit_assert_int(vec.data[1], ==, 2);
    
    pda_pushback(&vec, 3);
    munit_assert_uint64(vec.size, ==, 3);
    munit_assert_uint64(vec.end_of_storage, ==, 4);
    munit_assert_int(vec.data[2], ==, 3);

    pda_free(&vec);
    return MUNIT_OK;
}

static MunitResult
test_dynarray_popback(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    pda_pushback(&vec, 1);
    int popped_value = pda_popback(&vec);
    munit_assert_uint64(vec.size, ==, 0);
    munit_assert_uint64(vec.end_of_storage, ==, 2);
    munit_assert_int(popped_value, ==, 1);
    
    pda_free(&vec);
    return MUNIT_OK;
}

static MunitResult
test_dynarray_pushbytes(const MunitParameter[], void*) {
    pda_create_dynarray(stringv, char);

    char example[] = "hi there";
    struct stringv vec = {0};

    pda_pushbytes(&vec, sizeof example - 1, example);
    munit_assert_uint64(vec.size, ==, 8);
    munit_assert_uint64(vec.end_of_storage, ==, 8);
    munit_assert_char(vec.data[0], ==, 'h');
    munit_assert_char(vec.data[1], ==, 'i');
    munit_assert_char(vec.data[2], ==, ' ');
    munit_assert_char(vec.data[3], ==, 't');
    munit_assert_char(vec.data[4], ==, 'h');
    munit_assert_char(vec.data[5], ==, 'e');
    munit_assert_char(vec.data[6], ==, 'r');
    munit_assert_char(vec.data[7], ==, 'e');

    pda_free(&vec);
    return MUNIT_OK;
}


static MunitResult
test_dynarray_front_insertion(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    
    {
        auto insert_while_empty = &vec;
        int *empty_position = vec.data;
        pda_insert(insert_while_empty, empty_position, 1);
        munit_assert_uint64(insert_while_empty->size, ==, 1);
        munit_assert_uint64(insert_while_empty->end_of_storage, ==, 2);
        munit_assert_int(insert_while_empty->data[0], ==, 1);
        
        pda_free(&vec);
    }

    {
        pda_pushback(&vec, 2);
        
        int *start = pda_begin(&vec);
        pda_insert(&vec, start, 1);
        munit_assert_uint64(vec.size, ==, 2);
        munit_assert_uint64(vec.end_of_storage, ==, 2);
        munit_assert_int(vec.data[0], ==, 1);
        munit_assert_int(vec.data[1], ==, 2);
        
        pda_free(&vec);
    }

    return MUNIT_OK;
}

static MunitResult
test_dynarray_back_insertion(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);

    struct intv vec = {0};
    {
        auto insert_while_empty = &vec;
        int *end_ptr = pda_end(&vec);
        int *empty_position = end_ptr - 1;

        pda_insert(insert_while_empty, empty_position, 1);
        munit_assert_uint64(insert_while_empty->size, ==, 1);
        munit_assert_uint64(insert_while_empty->end_of_storage, ==, 2);
        munit_assert_int(insert_while_empty->data[0], ==, 1);

        pda_free(&vec);
    }

    {
        pda_pushback(&vec, 2);
        
        auto insert_before_end = &vec;
        int *before_end = pda_end(&vec) - 1;
        pda_insert(insert_before_end, before_end, 1);
        munit_assert_uint64(insert_before_end->size, ==, 2);
        munit_assert_uint64(insert_before_end->end_of_storage, ==, 2);
        munit_assert_int(insert_before_end->data[0], ==, 1);
        munit_assert_int(insert_before_end->data[1], ==, 2);
        
        pda_free(&vec);
    }
    
    {
        pda_pushback(&vec, 1);
        
        auto insert_at_end = &vec;
        int *at_end = pda_end(&vec);
        pda_insert(insert_at_end, at_end, 2);
        munit_assert_uint64(insert_at_end->size, ==, 2);
        munit_assert_uint64(insert_at_end->end_of_storage, ==, 2);
        munit_assert_int(insert_at_end->data[0], ==, 1);
        munit_assert_int(insert_at_end->data[1], ==, 2);
        
        pda_free(&vec);
    }
    
    {
        pda_pushback(&vec, 1);
        
        auto insert_after_end = &vec;
        int *after_end = pda_end(&vec) + 1;
        pda_insert(insert_after_end, after_end, 2);
        munit_assert_uint64(insert_after_end->size, ==, 1);
        munit_assert_uint64(insert_after_end->end_of_storage, ==, 2);
        munit_assert_int(insert_after_end->data[0], ==, 1);
        
        pda_free(&vec);
    }

    return MUNIT_OK;
}

static MunitResult
test_dynarray_middle_insertion(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);
    
    struct intv vec = {0};

    pda_pushback(&vec, 1);
    pda_pushback(&vec, 2);
    pda_pushback(&vec, 4);

    int *insert_after_two = pda_begin(&vec) + 2;
    pda_insert(&vec, insert_after_two, 3);
    munit_assert_uint64(vec.size, ==, 4);
    munit_assert_uint64(vec.end_of_storage, ==, 4);
    munit_assert_int(vec.data[0], ==, 1);
    munit_assert_int(vec.data[1], ==, 2);
    munit_assert_int(vec.data[2], ==, 3);
    munit_assert_int(vec.data[3], ==, 4);
    pda_free(&vec);

    return MUNIT_OK;
}

static MunitResult
test_dynarray_makehole(const MunitParameter[], void*) {
    pda_create_dynarray(stringv, char);

    char example[] = "i ther";
    struct stringv vec = {0};

    pda_pushback(&vec, 'h');
    pda_pushback(&vec, 'e');
    pda_makehole(&vec, &vec.data[1], sizeof example - 1);
    
    munit_assert_uint64(vec.size, ==, 2);
    munit_assert_uint64(vec.end_of_storage, ==, (sizeof example - 1 ) + 2);
    munit_assert_int(vec.data[0], ==, 'h');
    munit_assert_int(vec.data[7], ==, 'e');

    pda_free(&vec);
    return MUNIT_OK;
}

static MunitResult
test_dynarray_remove(const MunitParameter[], void*) {
    pda_create_dynarray(intv, int);
    
    struct intv vec = {0};

    pda_pushback(&vec,  1);
    pda_pushback(&vec, 90);
    pda_pushback(&vec,  2);

    pda_remove(&vec, &vec.data[1]);
    
    munit_assert_uint64(vec.size, ==, 2);
    munit_assert_uint64(vec.end_of_storage, ==, 4);
    munit_assert_int(vec.data[0], ==, 1);
    munit_assert_int(vec.data[1], ==, 2);
    
    pda_free(&vec);
    return MUNIT_OK;
}

static MunitResult
test_dynarray_user_type(const MunitParameter[], void*) {
    struct vec3 { int x, y, z; };
    pda_create_dynarray(v3vec, struct vec3);
    
    struct v3vec vec = {0};
    pda_pushback(&vec, ((struct vec3){ 1, 1, 1 }));
    pda_pushback(&vec, ((struct vec3){ 2, 2, 2 }));
    pda_pushback(&vec, ((struct vec3){ 4, 4, 4 }));

    struct vec3 *insert_after_two = pda_begin(&vec) + 2;
    pda_insert(&vec, insert_after_two, ((struct vec3){ 3, 3, 3 }));

    munit_assert_uint64(vec.size, ==, 4);
    munit_assert_uint64(vec.end_of_storage, ==, 4);

#define assert_vec3(lhs, op, rhs)           \
    munit_assert_int((lhs).x, op, (rhs).x); \
    munit_assert_int((lhs).y, op, (rhs).y); \
    munit_assert_int((lhs).z, op, (rhs).z)

    assert_vec3(vec.data[0], ==, ((struct vec3){ 1, 1, 1 }));
    assert_vec3(vec.data[1], ==, ((struct vec3){ 2, 2, 2 }));
    assert_vec3(vec.data[2], ==, ((struct vec3){ 3, 3, 3 }));
    assert_vec3(vec.data[3], ==, ((struct vec3){ 4, 4, 4 }));
#undef assert_vec3
    pda_free(&vec);

    return MUNIT_OK;
}

static MunitTest PTSD_DYNARRAY_TESTS[] = {
    { "/set-capacity"    , test_dynarray_set_capacity    , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/free"            , test_dynarray_free            , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/double-free"     , test_dynarray_double_free     , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/copy"            , test_dynarray_copy            , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/pushback"        , test_dynarray_pushback        , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/popback"         , test_dynarray_popback         , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/pushbytes"       , test_dynarray_pushbytes       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/front" , test_dynarray_front_insertion , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/middle", test_dynarray_middle_insertion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/insertion/back"  , test_dynarray_back_insertion  , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/makehole"        , test_dynarray_makehole        , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/remove"          , test_dynarray_remove          , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/user-type"       , test_dynarray_user_type       , NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_DYNARRAY_SUITE = {
    "/dynarray", 
    PTSD_DYNARRAY_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};


