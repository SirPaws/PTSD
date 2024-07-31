#include "general.h"
#include "pio.h"
#include "deps/munit.h"
#include "pplatform.h"

static MunitResult
test_file_stream(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_c_stream(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_string_stream(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_standard_stream(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

void *stream_setup(const MunitParameter params[], void *user_data) {
    (void)params;
    pcd(pcreate_string("tests"));
    return user_data;
}

void *stream_teardown(const MunitParameter params[], void *user_data) {
    (void)params;
    system("rm files/created.txt");
    pcd(pcreate_string(".."));
    return user_data;
}

static MunitTest PTSD_STREAM_TESTS[] = {
    { "/file"    , test_file_stream    , stream_setup, nullptr, MUNIT_TEST_OPTION_TODO, nullptr },
    { "/cfile"   , test_c_stream       , stream_setup, nullptr, MUNIT_TEST_OPTION_TODO, nullptr },
    { "/string"  , test_string_stream  , stream_setup, nullptr, MUNIT_TEST_OPTION_TODO, nullptr },
    { "/standard", test_standard_stream, stream_setup, nullptr, MUNIT_TEST_OPTION_TODO, nullptr },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static constexpr MunitSuite PTSD_STREAM_SUITE = {
    "/stream", PTSD_STREAM_TESTS, nullptr, 16, MUNIT_SUITE_OPTION_NONE
};
