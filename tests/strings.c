#include "pstring.h"
#include "deps/munit.h"
#include <stdlib.h>

static MunitResult
test_plines_single_line(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with a single line");

    pstring_t *stretchy lines = plines(text);
    _Defer psb_free(lines);

    usize line_count = psb_size(lines);
    munit_assert_int(line_count, ==, 1);
    munit_assert_true(pstring_equal(lines[0], text));

    return MUNIT_OK;
}

static MunitResult
test_plines_two_lines(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with two lines\nhere is the second line");

    pstring_t *stretchy lines = plines(text);
    _Defer psb_free(lines);

    usize line_count = psb_size(lines);
    munit_assert_int(line_count, ==, 2);
    munit_assert_true(pstring_equal(lines[0], pcreate_const_string("this is a string with two lines")));
    munit_assert_true(pstring_equal(lines[1], pcreate_const_string("here is the second line")));

    return MUNIT_OK;
}

static MunitResult
test_plines_two_lines_with_crlf(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with two lines\r\nhere is the second line");

    pstring_t *stretchy lines = plines(text);
    _Defer psb_free(lines);

    usize line_count = psb_size(lines);
    munit_assert_int(line_count, ==, 2);
    munit_assert_true(pstring_equal(lines[0], pcreate_const_string("this is a string with two lines")));
    munit_assert_true(pstring_equal(lines[1], pcreate_const_string("here is the second line")));

    return MUNIT_OK;
}

static MunitResult
test_plines_two_lines_with_newline(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with two lines\nhere is the second line\n");

    pstring_t *stretchy lines = plines(text);
    _Defer psb_free(lines);

    usize line_count = psb_size(lines);
    munit_assert_int(line_count, ==, 2);
    munit_assert_true(pstring_equal(lines[0], pcreate_const_string("this is a string with two lines")));
    munit_assert_true(pstring_equal(lines[1], pcreate_const_string("here is the second line")));

    return MUNIT_OK;
}

static MunitResult
test_plines_into_empty(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with two lines\nhere is the second line\n");

    usize line_count = plines_into(0, nullptr, text);
    munit_assert_int(line_count, ==, 2);

    return MUNIT_OK;
}

static MunitResult
test_plines_into_too_small(const MunitParameter[], void*) {
    pstring_t text = pcreate_const_string("this is a string with two lines\nhere is the second line\n");

    pstring_t lines[1];
    usize line_count = plines_into(_Countof(lines), lines, text);
    munit_assert_int(line_count, ==, 2);
    munit_assert_true(pstring_equal(lines[0], pcreate_const_string("this is a string with two lines")));

    return MUNIT_OK;
}

static MunitTest PTSD_PSTRING_TESTS[] = {
    { "/plines_single_line",            test_plines_single_line, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plines_two_lines",              test_plines_two_lines, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plines_two_lines_with_crlf",    test_plines_two_lines_with_crlf, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plines_two_lines_with_lr",      test_plines_two_lines_with_newline, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plines_into_no_buffer",         test_plines_into_empty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plines_into_small_buffer",      test_plines_into_too_small, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    // { "/plines" , test_plines , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite PTSD_PSTRING_SUITE = {
    "/pstrings", 
    PTSD_PSTRING_TESTS, 
    NULL,
    16,
    MUNIT_SUITE_OPTION_NONE
};
