

#include "general.h"

#if defined(PSTD_GNU_COMPATIBLE)
#define GNU_VECTOR
#else
#endif

#ifndef element_type
#define element_type f32
#endif

#undef GNU_VECTOR
#ifndef vector_type
#   define vector_type tmp_vec
#   ifdef GNU_VECTOR
#       if defined(__clang__)
            typedef element_type sse_type_ __attribute__((ext_vector_type(4)));
#       else
            typedef element_type sse_type_ __attribute__((vector_size(sizeof(element_type) * 4)));
#       endif
            typedef struct vector_type vector_type;
            struct vector_type {
                sse_type_ sse_elements;
            };
#   else
#       include "generic_sse.h"
            typedef struct vector_type vector_type;
            struct vector_type {
                __m128 sse_elements;
            };
#   endif
f32 pSqrt(f32);
f32 pSqrtR(f32);
#endif

#define pNegate         PSTD_CONCAT(pNegate,        vector_type)
#define pAddVector      PSTD_CONCAT(pAdd,           PSTD_CONCAT(vector_type, V))
#define pSubVector      PSTD_CONCAT(pSub,           PSTD_CONCAT(vector_type, V))
#define pMulVector      PSTD_CONCAT(pMul,           PSTD_CONCAT(vector_type, V))
#define pDivVector      PSTD_CONCAT(pDiv,           PSTD_CONCAT(vector_type, V))
#define pAddScalar      PSTD_CONCAT(pAdd,           PSTD_CONCAT(vector_type, S))
#define pSubScalar      PSTD_CONCAT(pSub,           PSTD_CONCAT(vector_type, S))
#define pMulScalar      PSTD_CONCAT(pMul,           PSTD_CONCAT(vector_type, S))
#define pDivScalar      PSTD_CONCAT(pDiv,           PSTD_CONCAT(vector_type, S))
#define pDot            PSTD_CONCAT(pDot,           vector_type)
#define pLength         PSTD_CONCAT(pLength,        vector_type)
#define pLengthR        PSTD_CONCAT(pLengthR,       vector_type)
#define pLengthSquared  PSTD_CONCAT(pLengthSquared, vector_type)
#define pNormalize      PSTD_CONCAT(pNormalize,     vector_type)
#define pProject        PSTD_CONCAT(pProject,       vector_type)
#define pReject         PSTD_CONCAT(pReject,        vector_type)




PSTD_UNUSED static inline
vector_type pNegate(vector_type vector) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = -vector.sse_elements;
#else
    static vector_type negative = {};
    negative.sse_elements = _mm_set1(-1);
    result.sse_elements = _mm_mul(vector.sse_elements, negative.sse_elements);
#endif
    return result;
}
PSTD_UNUSED static inline
vector_type pAddVector(vector_type a, vector_type b) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = a.sse_elements + b.sse_elements;
#else
#endif
    return result;
}
PSTD_UNUSED static inline
vector_type pSubVector(vector_type a, vector_type b) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = a.sse_elements - b.sse_elements;
#else
#endif
    return result;
}
PSTD_UNUSED static inline
vector_type pMulVector(vector_type a, vector_type b) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = a.sse_elements * b.sse_elements;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pDivVector(vector_type a, vector_type b) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = a.sse_elements / b.sse_elements;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pAddScalar(vector_type vector, element_type scalar) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = vector.sse_elements + scalar;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pSubScalar(vector_type vector, element_type scalar) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = vector.sse_elements - scalar;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pMulScalar(vector_type vector, element_type scalar) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = vector.sse_elements * scalar;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pDivScalar(vector_type vector, element_type scalar) {
    vector_type result;
#ifdef GNU_VECTOR
    result.sse_elements = vector.sse_elements / scalar;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
element_type pDot(vector_type a, vector_type b) {
    element_type result;
#ifdef GNU_VECTOR
    vector_type tmp;
    tmp.sse_elements = a.sse_elements * b.sse_elements;
    result = tmp.sse_elements[0] + tmp.sse_elements[1] + tmp.sse_elements[2] + tmp.sse_elements[3];
#else
#endif
    return result;
}

PSTD_UNUSED static inline
element_type pLengthSquared(vector_type vector) {
    return pDot(vector, vector);
}

PSTD_UNUSED static inline
element_type pLength(vector_type vector) {
    return pSqrt(pLengthSquared(vector));
}

PSTD_UNUSED static inline
element_type pLengthR(vector_type vector) {
    return pSqrtR(pLengthSquared(vector));
}

// normalize doesn't work with int vectors but it doesn't make sense to call this with an 
// int vector anyways but for simplicity it still exists
PSTD_UNUSED static inline
vector_type pNormalize(vector_type vector) {
    vector_type result;
#ifdef GNU_VECTOR
    element_type length = pLengthR(vector);
    result.sse_elements = vector.sse_elements * length;
#else
#endif
    return result;
}

PSTD_UNUSED static inline
vector_type pProject(vector_type a, vector_type b) {
    return pMulScalar(b, pDot(a,b)/pDot(b,b));
}
PSTD_UNUSED static inline
vector_type pReject(vector_type a, vector_type b) {
    return pSubVector(a, pMulScalar(b, pDot(a,b)/pDot(b,b)));
}

#undef pNegate
#undef pAddVector
#undef pSubVector
#undef pMulVector
#undef pDivVector
#undef pAddScalar
#undef pSubScalar
#undef pMulScalar
#undef pDivScalar
#undef pDot
#undef pLength
#undef pLengthR
#undef pLengthSquared
#undef pNormalize
#undef pProject
#undef pReject



