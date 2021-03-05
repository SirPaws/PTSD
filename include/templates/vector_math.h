
#include "general.h"

#ifndef element_type
#define element_type float
#endif

#ifndef vector_type
#define vector_type tmp_vec
typedef struct tmp_vec tmp_vec;
struct tmp_vec { element_type elements[4]; };
static f32 pSqrt(f32 d);
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
#define pLengthSquared  PSTD_CONCAT(pLengthSquared, vector_type)
#define pNormalize      PSTD_CONCAT(pNormalize,     vector_type)
#define pProject        PSTD_CONCAT(pProject,       vector_type)
#define pReject         PSTD_CONCAT(pReject,        vector_type)

#define pPerpendicular  PSTD_CONCAT(pPerpendicular, vector_type)
#define pCross          PSTD_CONCAT(pCross,         vector_type)


PSTD_UNUSED static inline
vector_type pNegate(vector_type vector) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = -vector.elements[i];
    return result;
}
PSTD_UNUSED static inline
vector_type pAddVector(vector_type a, vector_type b) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] + b.elements[i];
    return result;
}
PSTD_UNUSED static inline
vector_type pSubVector(vector_type a, vector_type b) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] - b.elements[i];
    return result;
}
PSTD_UNUSED static inline
vector_type pMulVector(vector_type a, vector_type b) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] * b.elements[i];
    return result;
}
PSTD_UNUSED static inline
vector_type pDivVector(vector_type a, vector_type b) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] / b.elements[i];
    return result;
}
PSTD_UNUSED static inline
vector_type pAddScalar(vector_type vector, element_type scalar) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = vector.elements[i] + scalar;
    return result;
}
PSTD_UNUSED static inline
vector_type pSubScalar(vector_type vector, element_type scalar) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = vector.elements[i] - scalar;
    return result;
}
PSTD_UNUSED static inline
vector_type pMulScalar(vector_type vector, element_type scalar) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = vector.elements[i] * scalar;
    return result;
}
PSTD_UNUSED static inline
vector_type pDivScalar(vector_type vector, element_type scalar) {
    vector_type result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = vector.elements[i] / scalar;
    return result;
}
PSTD_UNUSED static inline
element_type pDot(vector_type a, vector_type b) {
    element_type sum = 0;
    for (u32 i = 0; i < countof(a.elements); i++)
        sum += a.elements[i] * b.elements[i];
    return sum;
}
PSTD_UNUSED static inline
element_type pLengthSquared(vector_type vector) {
    return pDot(vector, vector);
}
PSTD_UNUSED static inline
element_type pLength(vector_type vector) {
    return pSqrt(pDot(vector, vector));
}
PSTD_UNUSED static inline
vector_type pNormalize(vector_type vector) {
    element_type length = pLength(vector);
    vector_type result;
    for (u32 i = 0; i < countof(vector.elements); i++)
        result.elements[i] = vector.elements[i]/length;
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



#if defined(VEC2)
PSTD_UNUSED static inline
vector_type pPerpendicular(vector_type vector) {
    vector_type result;
    result.x = -vector.y;
    result.y =  vector.x;
    return result;
}
#endif 

#if defined(VEC3)
PSTD_UNUSED static inline
vector_type pCross(vector_type a, vector_type b) {
    vector_type result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}
#endif

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
#undef pLengthSquared
#undef pNormalize
#undef pPerpendicular
#undef pCross
#undef pProject
#undef pReject

