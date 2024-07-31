#include "plimits.h"
#include "deps/munit.h"

static MunitResult
test_LIMITS_NAME_HERE(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitTest PTSD_LIMITS_TESTS[] = {
    { "/NAME" , test_LIMITS_NAME_HERE , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_LIMITS_SUITE = {
    "/limits", 
    PTSD_LIMITS_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};
