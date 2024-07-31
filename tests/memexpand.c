
#include "general.h"
#include "util.h"
#include "deps/munit.h"

static MunitResult
test_expand_int(const MunitParameter params[], void* data) {
    u32 num = munit_rand_uint32();
    u32 arr[10];
    pmemexpand(arr, &num, sizeof arr, sizeof num);

    for (u32 i = 0; i < 10; i++){
        munit_assert_int(arr[i], ==, num);
    }

    return MUNIT_OK;
}

static MunitResult
test_expand_uneven(const MunitParameter params[], void* data) {
    struct LargeStruct {
        u32 a, b, c, d;
    };
    struct LargeStruct s = {
        munit_rand_uint32(),
        munit_rand_uint32(),
        munit_rand_uint32(),
        munit_rand_uint32(),
    };
    u32 arr[10];
    pmemexpand(arr, &s, sizeof arr, sizeof s);

    u32 (*ptr)[4] = (void*)&s.a;
    for (u32 i = 0; i < 10; i++){
        munit_assert_int(arr[i], ==, (*ptr)[i % 4]);
    }

    return MUNIT_OK;
}

static MunitTest PTSD_MEMEXPAND_TESTS[] = {
    { "/int",    test_expand_int,    nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr },
    { "/uneven", test_expand_uneven, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static constexpr MunitSuite PTSD_MEMEXPAND_SUITE = {
    "/memexpand", PTSD_MEMEXPAND_TESTS, nullptr, 16, MUNIT_SUITE_OPTION_NONE
};




