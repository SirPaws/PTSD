#include <general.h>

#ifndef value_type
#define value_type f32
#endif

#ifndef pAdd
#define pAdd(a,b) (a) + (b)
#endif
#ifndef pSub
#define pSub(a,b) (a) - (b)
#endif
#ifndef pMul
#define pMul(a,b) (a) * (b)
#endif
#ifndef pMulFloat
#define pMulFloat(a,b) (a) * (b)
#endif
#ifndef pDiv
#define pDiv(a,b) (a) / (b)
#endif
#ifndef pLessThan
#define pLessThan(a,b) (a) < (b)
#endif
#ifndef pGreaterThan
#define pGreaterThan(a,b) (a) > (b)
#endif

#define pRemap  PSTD_CONCAT(pRemap, value_type)      
#define pLerp   PSTD_CONCAT(pLerp,  value_type)
#define pMin    PSTD_CONCAT(pMin,   value_type)
#define pMax    PSTD_CONCAT(pMax,   value_type)
#define pSwap   PSTD_CONCAT(pSwap,  value_type)


PSTD_UNUSED static inline
value_type pRemap(
        value_type value, value_type from1, value_type to1, value_type from2, value_type to2);
PSTD_UNUSED static inline
value_type pLerp(value_type v0, value_type v1, f32 t);

PSTD_UNUSED static inline
value_type pMin(value_type a, value_type b);

PSTD_UNUSED static inline
value_type pMax(value_type a, value_type b);

PSTD_UNUSED static inline
void pSwap(value_type *a, value_type *b);



#if !defined(PIO_EXTERNAL_MATH)
PSTD_UNUSED static inline
value_type pRemap
    (value_type value, value_type from1, value_type to1, value_type from2, value_type to2)
{

    value_type t0 = pSub(value, from1); // (value - from1)
    value_type t1 = pSub(  to1, from1); // (to1   - from1)
    value_type t2 = pDiv(   t0,    t1); // (value - from1) / (to1   - from1)
    value_type t3 = pSub(  to2, from2); // (to2 - from2)
    value_type t4 = pMul(   t2,    t3); // (value - from1) / (to1   - from1) * (to2 - from2)
    value_type t5 = pAdd(   t4, from2);
    // (value - from1) / (to1   - from1) * (to2 - from2) + from2
    return t5;
}

PSTD_UNUSED static inline
value_type pLerp(value_type v0, value_type v1, f32 t) {
    value_type t0 = pSub(v1, v0); // (v1 - v0)
    value_type t1 = pMulFloat( t, t0); // t * (v1 - v0)
    value_type t2 = pAdd(v0, t1); // v0 + t * (v1 - v0)
    return t2;
}


PSTD_UNUSED static inline
value_type pMin(value_type a, value_type b) {
    return pLessThan(a, b) ? a : b;
}
PSTD_UNUSED static inline
value_type pMax(value_type a, value_type b) {
    return pGreaterThan(a, b) ? a : b;
}
PSTD_UNUSED static inline
void pSwap(value_type *a, value_type *b) {
     value_type tmp =  *a; 
     *a             =  *b; 
     *b             = tmp; 
}

#endif

#undef value_type
#undef pAdd
#undef pSub
#undef pMul
#undef pMulFloat
#undef pDiv
#undef pLessThan
#undef pGreaterThan
#undef pRemap
#undef pLerp
#undef pMin
#undef pMax
#undef pSwap


