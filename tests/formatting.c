
#include "general.h"
#include "pio.h"
#include "deps/munit.h"

static MunitResult test_format_printf_char(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult test_format_printf_string(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult test_format_printf_num_characters(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult test_format_printf_pointer(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult test_format_printf_specifiers(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_integer_base8(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_integer_base10(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_integer_base16(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_float(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_pstring(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_colour(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_save(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_restore(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_bold(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_nobold(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_dim(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_nodim(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_italic(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_noitalic(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_line(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_noline(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_blink(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_noblink(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_hide(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_nohide(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_strike(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_nostrike(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_binary(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}
static MunitResult
test_format_generic(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitResult
test_format_bool(const MunitParameter[], void*) {
    return MUNIT_SKIP;
}

static MunitTest PTSD_FORMATTING_PRINTF_TEST[] = {
    { "/char",          test_format_printf_char          , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/string",        test_format_printf_string        , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/numcharacters", test_format_printf_num_characters, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/pointer",       test_format_printf_pointer       , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/specifiers",    test_format_printf_specifiers    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static constexpr MunitSuite PTSD_FORMATTING_PRINTF_SUITE = {
    "/printf", PTSD_FORMATTING_PRINTF_TEST, NULL, 16, MUNIT_SUITE_OPTION_NONE
};

static MunitTest PTSD_FORMATTING_NUMBERS_TEST[] = {
    { "/integer/base8"   , test_format_integer_base8 , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/integer/base10"  , test_format_integer_base10, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/integer/base16"  , test_format_integer_base16, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/float"           , test_format_float         , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static constexpr MunitSuite PTSD_FORMATTING_NUMBERS_SUITE = {
    "/numbers", PTSD_FORMATTING_NUMBERS_TEST, NULL, 16, MUNIT_SUITE_OPTION_NONE
};

static MunitTest PTSD_FORMATTING_TESTS[] = {
    { "/pstring" , test_format_pstring , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/colour"  , test_format_colour  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/save"    , test_format_save    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/restore" , test_format_restore , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/bold"    , test_format_bold    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/nobold"  , test_format_nobold  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/dim"     , test_format_dim     , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/nodim"   , test_format_nodim   , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/italic"  , test_format_italic  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/noitalic", test_format_noitalic, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/line"    , test_format_line    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/noline"  , test_format_noline  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/blink"   , test_format_blink   , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/noblink" , test_format_noblink , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/hide"    , test_format_hide    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/nohide"  , test_format_nohide  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/strike"  , test_format_strike  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/nostrike", test_format_nostrike, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/binary"  , test_format_binary  , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/bool"    , test_format_bool    , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/generic" , test_format_generic , NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static MunitSuite format_internal_suites[] = {
    PTSD_FORMATTING_PRINTF_SUITE,
    PTSD_FORMATTING_NUMBERS_SUITE,
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static const MunitSuite PTSD_FORMATTING_SUITE = {
    "/formatting", 
    PTSD_FORMATTING_TESTS, 
    format_internal_suites,
    16,
    MUNIT_SUITE_OPTION_NONE
};




