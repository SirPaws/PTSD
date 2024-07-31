#include "dynarray.h"
#include "deps/munit.h"

static MunitResult
test_DYNARRAY_NAME_HERE(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitTest PTSD_DYNARRAY_TESTS[] = {
    { "/NAME" , test_DYNARRAY_NAME_HERE , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_DYNARRAY_SUITE = {
    "/dynarray", 
    PTSD_DYNARRAY_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};


