#include "memexpand.c"
#include "formatting.c"
#include "streams.c"
#include "dynarray.c"
#include "limits.c"
#include "stretchy_buffer.c"

#define SUITES(x)                   \
    x(PTSD_MEMEXPAND_SUITE)         \
    x(PTSD_STREAM_SUITE)            \
    x(PTSD_FORMATTING_SUITE)        \
    x(PTSD_DYNARRAY_SUITE)          \
    x(PTSD_LIMITS_SUITE)            \
    x(PTSD_STRETCHY_BUFFER_SUITE)   \


