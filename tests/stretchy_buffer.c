#include "stretchy_buffer.h"
#include "deps/munit.h"

static MunitResult
test_STRETCHY_BUFFER_NAME_HERE(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitTest PTSD_STRETCHY_BUFFER_TESTS[] = {
    { "/NAME" , test_STRETCHY_BUFFER_NAME_HERE , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_STRETCHY_BUFFER_SUITE = {
    "/stretchybuffer", 
    PTSD_STRETCHY_BUFFER_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};
