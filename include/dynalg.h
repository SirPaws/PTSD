#pragma once
#include "dynarray.h"


#define pForEach(array, ...)    pForEach_(array, (__VA_ARGS__))(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, (__VA_ARGS__))(array, ## __VA_ARGS__)









































// DETAIL

#define _ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define HAS_COMMA(...) _ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...) ,
 
#define ISEMPTY(...)                                                    \
_ISEMPTY(                                                               \
          /* test if there is just one argument, eventually an empty    \
             one */                                                     \
          HAS_COMMA(__VA_ARGS__),                                       \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
             adds a comma */                                            \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
          /* test if the argument together with a parenthesis           \
             adds a comma */                                            \
          HAS_COMMA(__VA_ARGS__ (/*empty*/)),                           \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the   \
             parenthesis adds a comma */                                \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))      \
          )
#define PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define _ISEMPTY(_0, _1, _2, _3) HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,
#define CONCAT_( a, b ) a##b
#define CONCAT( a, b ) CONCAT_( a, b )


#define pForEach0(array, name)  for( __auto_type name = pBegin(array); name != pEnd(array); name++)
#define pForEach1(array)        pForEach0(array, it)
#define pForEach__(array, args) CONCAT(pForEach, args)
#define pForEach_(array, args)  pForEach__(array, ISEMPTY args)

#define pForEachR0(array, name)  for( __auto_type name = pEnd(array) - 1; name != pBegin(array) - 1; name++)
#define pForEachR1(array)        pForEachR0(array, it)
#define pForEachR__(array, args) CONCAT(pForEachR, args)
#define pForEachR_(array, args)  pForEachR__(array, ISEMPTY args)

