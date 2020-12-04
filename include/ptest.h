#include "pmacroutil.h"

#ifndef P_TEST_NO_IO
#include "pio.h"
#endif

#ifndef P_SUCCESS_COLOR
#define P_SUCCESS_COLOR "%Cfg(0, 127, 0)"
#endif

#ifndef P_FAILURE_COLOR
#define P_FAILURE_COLOR "%Cfg(127, 0, 0)"
#endif

#ifndef P_COLOR_CLEAR
#define P_COLOR_CLEAR "%Cc"
#endif

#ifndef P_TEST_PRINT_ERROR
#define P_TEST_PRINT_ERROR(message)                                     \
    ({                                                                  \
     pPrintf(P_FAILURE_COLOR "("__FILE__ ":" PSTD_STRINGIFY(__LINE__)")"\
             "TEST FAILED: " message "%Cc\n");})
#endif

#ifndef P_PRINT
#define P_PRINT(fmt, ...) pPrintf(fmt, __VA_ARGS__)
#endif

struct pTestState {
    int count, succeded, failed;
};
void pTestShutdown(struct pTestState *state) {
    if (state->count) {
        P_PRINT("------|RAN ");
        usize count = P_PRINT("%u", state->count);
        P_PRINT(" TESTS|------\n");
        if (!state->failed)
            P_PRINT(P_SUCCESS_COLOR "ALL TEST SUCCEDED" P_COLOR_CLEAR);
        else if (!state->succeded)
            P_PRINT(P_FAILURE_COLOR "ALL TEST FAILED" P_COLOR_CLEAR);
        else {
            P_PRINT(P_SUCCESS_COLOR"%*i TESTS SUCCEDED\n" P_COLOR_CLEAR, count, state->count);
            P_PRINT(P_FAILURE_COLOR"%*i TESTS FAILED\n" P_COLOR_CLEAR,   count, state->failed);
        }
    } else {
        P_PRINT("------|RAN 0 TESTS|------\n");
    }
}

#define P_TEST_INIT( name ) __attribute__((cleanup(pTestShutdown)))\
    struct pTestState name = {0}; struct pTestState *P_TEST_LOCAL = &(name);\
    P_PRINT("------|RUNNING " #name " TEST|------")
             


#define P_SUCCESS0(assert, message) /* message given */\
        ({                                      \
            P_TEST_LOCAL->count++;              \
            if (!(assert)) {                    \
                P_TEST_PRINT_ERROR(#message);   \
                P_TEST_LOCAL->failed++;         \
            }                                   \
            else P_TEST_LOCAL->succeded++;      \
        })
#define P_SUCCESS1(assert, message) /* no message given */ \
        ({                                      \
            P_TEST_LOCAL->count++;              \
            if (!(assert)) {                    \
                P_TEST_PRINT_ERROR(#assert);    \
                P_TEST_LOCAL->failed++;         \
            }                                   \
            else P_TEST_LOCAL->succeded++;      \
        })

#define P_SUCCESS_(assert, is_empty) PSTD_CONCAT(P_SUCCESS, is_empty) 
#define P_SUCCESS( assert, ... ) \
    P_SUCCESS_( assert, PSTD_ISEMPTY(__VA_ARGS__))(assert, __VA_ARGS__)

#define P_FAIL( assert, ... ) P_SUCCESS(!(assert), __VA_ARGS__)

