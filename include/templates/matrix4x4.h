
#include "general.h"

#ifndef element_type
#define element_type f32
#include <math.h>
element_type pCos(element_type x) {
    return cos(x);
}
element_type pSin(element_type x) {
    return sin(x);
}
element_type pTan(element_type x) {
    return tan(x);
}

#endif

#ifndef vector_prefix
#define vector_prefix Vec
#endif

#ifndef matrix_prefix
#define matrix_prefix Mat
#endif

#ifndef suffix
#define suffix f
#endif


#include "vector.h"

#define ALIAS(type, ...) struct { type __VA_ARGS__; }
#define Vec2   PSTD_CONCAT(vector_prefix, PSTD_CONCAT(2, suffix))
#define Vec3   PSTD_CONCAT(vector_prefix, PSTD_CONCAT(3, suffix))
#define Vec4   PSTD_CONCAT(vector_prefix, PSTD_CONCAT(4, suffix))
#define Mat4x4 PSTD_CONCAT(matrix_prefix, PSTD_CONCAT(4x4, suffix))


typedef struct Mat4x4 Mat4x4;
struct Mat4x4 {
    union {
        ALIAS(element_type, 
                 _0,  _1,  _2,  _3,
                 _4,  _5,  _6,  _7,
                 _8,  _9, _10, _11,
                _12, _13, _14, _15);
        element_type elements[4*4];
        Vec4 columns[4];
        Vec2 split_columns[4*2];
    };
};

// element wise
#define pNegate         PSTD_CONCAT(pNegate,        Mat4x4)
#define pAddVector      PSTD_CONCAT(pAdd,           PSTD_CONCAT(Mat4x4, V))
#define pSubVector      PSTD_CONCAT(pSub,           PSTD_CONCAT(Mat4x4, V))
#define pDivVector      PSTD_CONCAT(pDiv,           PSTD_CONCAT(Mat4x4, V))

#define pAddScalar      PSTD_CONCAT(pAdd,           PSTD_CONCAT(Mat4x4, S))
#define pSubScalar      PSTD_CONCAT(pSub,           PSTD_CONCAT(Mat4x4, S))
#define pDivScalar      PSTD_CONCAT(pDiv,           PSTD_CONCAT(Mat4x4, S))

#define pAddMatrix      PSTD_CONCAT(pAdd,           PSTD_CONCAT(Mat4x4, M))
#define pSubMatrix      PSTD_CONCAT(pSub,           PSTD_CONCAT(Mat4x4, M))
#define pMulMatrix      PSTD_CONCAT(pMul,           PSTD_CONCAT(Mat4x4, M))
#define pDivMatrix      PSTD_CONCAT(pDiv,           PSTD_CONCAT(Mat4x4, M))


#define pMulScalar      PSTD_CONCAT(pMul,           PSTD_CONCAT(Mat4x4, S))
#define pMulVector      PSTD_CONCAT(pMul,           PSTD_CONCAT(Mat4x4, V))
#define pVecMulMatrix   PSTD_CONCAT(pMul,           PSTD_CONCAT(Vec4, M))

#define pVecMulScalar   PSTD_CONCAT(pMul,           PSTD_CONCAT(Vec4, S))
#define pVecDot         PSTD_CONCAT(pDot,           Vec4)
#define pVecAdd         PSTD_CONCAT(pAdd,           PSTD_CONCAT(Vec4, V))
#define pVecSub         PSTD_CONCAT(pSub,           PSTD_CONCAT(Vec4, V))
#define pVecMul         PSTD_CONCAT(pMul,           PSTD_CONCAT(Vec4, V))
#define pVecDiv         PSTD_CONCAT(pDiv,           PSTD_CONCAT(Vec4, V))

#define pVec3Dot         PSTD_CONCAT(pDot,           Vec3)
#define pVec3Add         PSTD_CONCAT(pAdd,           PSTD_CONCAT(Vec3, V))
#define pVec3Sub         PSTD_CONCAT(pSub,           PSTD_CONCAT(Vec3, V))
#define pVec3Mul         PSTD_CONCAT(pMul,           PSTD_CONCAT(Vec3, V))
#define pVec3MulScalar   PSTD_CONCAT(pMul,           PSTD_CONCAT(Vec3, S))
#define pVec3Div         PSTD_CONCAT(pDiv,           PSTD_CONCAT(Vec3, V))
#define pVec3Normalize   PSTD_CONCAT(pNormalize,     Vec3)
#define pVec3Cross       PSTD_CONCAT(pCross,         Vec3)

#define pOrtho          PSTD_CONCAT(pOrtho,         Mat4x4)
#define pPerspective    PSTD_CONCAT(pPerspective,   Mat4x4)
#define pLookAt         PSTD_CONCAT(pLookAt,        Mat4x4)
#define pFrustum        PSTD_CONCAT(pFrustum,       Mat4x4)
#define pTranslate      PSTD_CONCAT(pTranslate,     Mat4x4)
#define pRotate         PSTD_CONCAT(pRotate,        Mat4x4)
#define pRotateX        PSTD_CONCAT(pRotateX,       Mat4x4)
#define pRotateY        PSTD_CONCAT(pRotateY,       Mat4x4)
#define pRotateZ        PSTD_CONCAT(pRotateZ,       Mat4x4)
#define pScale          PSTD_CONCAT(pScale,         Mat4x4)


PSTD_UNUSED static inline
Mat4x4 pNegate(Mat4x4 matrix) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = -matrix.elements[i];
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pAddMatrix(Mat4x4 a, Mat4x4 b) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] + b.elements[i];
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pSubMatrix(Mat4x4 a, Mat4x4 b) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] - b.elements[i];
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pDivMatrix(Mat4x4 a, Mat4x4 b) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = a.elements[i] / b.elements[i];
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pAddScalar(Mat4x4 matrix, element_type scalar) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = matrix.elements[i] + scalar;
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pSubScalar(Mat4x4 matrix, element_type scalar) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = matrix.elements[i] - scalar;
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pMulScalar(Mat4x4 matrix, element_type scalar) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = matrix.elements[i] * scalar;
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pDivScalar(Mat4x4 matrix, element_type scalar) {
    Mat4x4 result;
    for (u32 i = 0; i < countof(result.elements); i++)
        result.elements[i] = matrix.elements[i] / scalar;
    return result;
}

PSTD_UNUSED static inline
Mat4x4 pAddVector(Mat4x4 matrix, Vec4 vector) {
    Mat4x4 result;
    result.columns[0] = pVecAdd(matrix.columns[0], vector);
    result.columns[1] = pVecAdd(matrix.columns[1], vector);
    result.columns[2] = pVecAdd(matrix.columns[2], vector);
    result.columns[3] = pVecAdd(matrix.columns[3], vector);
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pSubVector(Mat4x4 matrix, Vec4 vector) {
    Mat4x4 result;
    result.columns[0] = pVecSub(matrix.columns[0], vector);
    result.columns[1] = pVecSub(matrix.columns[1], vector);
    result.columns[2] = pVecSub(matrix.columns[2], vector);
    result.columns[3] = pVecSub(matrix.columns[3], vector);
    return result;
}
PSTD_UNUSED static inline
Mat4x4 pDivVector(Mat4x4 matrix, Vec4 vector) {
    Mat4x4 result;
    result.columns[0] = pVecDiv(matrix.columns[0], vector);
    result.columns[1] = pVecDiv(matrix.columns[1], vector);
    result.columns[2] = pVecDiv(matrix.columns[2], vector);
    result.columns[3] = pVecDiv(matrix.columns[3], vector);
    return result;
}


PSTD_UNUSED static inline
Mat4x4 pMulMatrix(Mat4x4 m, Mat4x4 n) {
    Mat4x4 result;
#define result result.columns
#define m      m.columns
#define n      n.columns
    result[0].x = m[0].x * n[0].x + m[1].x * n[0].y + m[2].x * n[0].z + m[3].x * n[0].w;
    result[1].x = m[0].x * n[1].x + m[1].x * n[1].y + m[2].x * n[1].z + m[3].x * n[1].w;
    result[2].x = m[0].x * n[2].x + m[1].x * n[2].y + m[2].x * n[2].z + m[3].x * n[2].w;
    result[3].x = m[0].x * n[3].x + m[1].x * n[3].y + m[2].x * n[3].z + m[3].x * n[3].w;

    result[0].y = m[0].y * n[0].x + m[1].y * n[0].y + m[2].y * n[0].z + m[3].y * n[0].w;
    result[1].y = m[0].y * n[1].x + m[1].y * n[1].y + m[2].y * n[1].z + m[3].y * n[1].w;
    result[2].y = m[0].y * n[2].x + m[1].y * n[2].y + m[2].y * n[2].z + m[3].y * n[2].w;
    result[3].y = m[0].y * n[3].x + m[1].y * n[3].y + m[2].y * n[3].z + m[3].y * n[3].w;

    result[0].z = m[0].z * n[0].x + m[1].z * n[0].y + m[2].z * n[0].z + m[3].z * n[0].w;
    result[1].z = m[0].z * n[1].x + m[1].z * n[1].y + m[2].z * n[1].z + m[3].z * n[1].w;
    result[2].z = m[0].z * n[2].x + m[1].z * n[2].y + m[2].z * n[2].z + m[3].z * n[2].w;
    result[3].z = m[0].z * n[3].x + m[1].z * n[3].y + m[2].z * n[3].z + m[3].z * n[3].w;
    
    result[0].w = m[0].w * n[0].x + m[1].w * n[0].y + m[2].w * n[0].z + m[3].w * n[0].w;
    result[1].w = m[0].w * n[1].x + m[1].w * n[1].y + m[2].w * n[1].z + m[3].w * n[1].w;
    result[2].w = m[0].w * n[2].x + m[1].w * n[2].y + m[2].w * n[2].z + m[3].w * n[2].w;
    result[3].w = m[0].w * n[3].x + m[1].w * n[3].y + m[2].w * n[3].z + m[3].w * n[3].w;
#undef result
#undef m
#undef n
    return result;
}

PSTD_UNUSED static inline
Vec4 pMulVector(Mat4x4 matrix, Vec4 vector) {
    Vec4 result;

    Vec4 c0 = pVecMulScalar(matrix.columns[0], vector.x);
    Vec4 c1 = pVecMulScalar(matrix.columns[1], vector.y);
    Vec4 c2 = pVecMulScalar(matrix.columns[2], vector.z);
    Vec4 c3 = pVecMulScalar(matrix.columns[3], vector.w);

    result.x = c0.x + c1.x + c2.x + c3.x;
    result.y = c0.y + c1.y + c2.y + c3.y;
    result.z = c0.z + c1.z + c2.z + c3.z;
    result.w = c0.w + c1.w + c2.w + c3.w;

    return result;
}

PSTD_UNUSED static inline
Vec4 pVecMulMatrix(Vec4 vector, Mat4x4 matrix) {
    Vec4 result;

    result.x = pVecDot(vector, matrix.columns[0]);
    result.y = pVecDot(vector, matrix.columns[1]);
    result.z = pVecDot(vector, matrix.columns[2]);
    result.w = pVecDot(vector, matrix.columns[3]);

    return result;
}




Mat4x4 pOrtho(element_type left, element_type right, 
        element_type bottom, element_type top, element_type near, element_type far) 
{
#define M result.columns
#define F far
#define N near
#define L left
#define R right
#define T top
#define B bottom
    Mat4x4 result = {0};
    static const element_type two = 2;
    M[0].x = two/(R - L); M[0].y =           0; M[0].z =            0; M[0].w = -((R + L)/(R - L));
    M[1].x =           0; M[1].y = two/(T - B); M[1].z =            0; M[1].w = -((T + B)/(T - B));
    M[2].x =           0; M[2].y =           0; M[2].z = -two/(F - N); M[2].w = -((F + N)/(F - N));
    M[3].x =           0; M[3].y =           0; M[3].z =            0; M[3].w =                  1;
    return result;                   
#undef M
#undef F
#undef N
#undef L
#undef R
#undef T
#undef B
}

Mat4x4 pFrustum(element_type left, element_type right, 
        element_type bottom, element_type top, element_type near, element_type far) 
{
#define M result.columns
#define F far
#define N near
#define L left
#define R right
#define T top
#define B bottom
    Mat4x4 result = {0};
    M[0].x = (2*N)/(R-L); M[0].y =           0; M[0].z =  (R+L)/(R-L); M[0].w =              0;
    M[1].x =           0; M[1].y = (2*N)/(T-B); M[1].z =  (T+B)/(T-B); M[1].w =              0;
    M[2].x =           0; M[2].y =           0; M[2].z = -(F+N)/(F-N); M[2].w = -(2*F*N)/(F-N);
    M[3].x =           0; M[3].y =           0; M[3].z =           -1; M[3].w =              0;
    return result;
#undef M
#undef F
#undef N
#undef L
#undef R
#undef T
#undef B
}

Mat4x4 pPerspective(element_type fov, element_type aspect, element_type near, element_type far) {
#define M result.columns
#define F far
#define N near
    Mat4x4 result = {0};
    fov = pTan(fov/2);
    M[0].x = (fov)/(aspect); M[0].y =   0; M[0].z =           0; M[0].w =             0;
    M[1].x =              0; M[1].y = fov; M[1].z =           0; M[1].w =             0;
    M[2].x =              0; M[2].y =   0; M[2].z = (F+N)/(F-N); M[2].w = (2*F*N)/(F-N);
    M[3].x =              0; M[3].y =   0; M[3].z =          -1; M[3].w =             0;
    return result;
#undef M
#undef F
#undef N
}

Mat4x4 pLookAt(Vec3 eye, Vec3 center, Vec3 up) {
#define M result.columns
#define F far
#define N near
    Mat4x4 result = {0};

    up = pVec3Normalize(up);
    Vec3 forward = pVec3Normalize(pVec3Sub(center, eye));
    Vec3 right   = pVec3Cross(forward, up);
    up           = pVec3Cross(right,  forward);

    M[0].x =    right.x; M[0].y =    right.y; M[0].z =    right.z; M[0].w =-pVec3Dot(right,   eye);
    M[1].x =       up.x; M[1].y =       up.y; M[1].z =       up.z; M[1].w =-pVec3Dot(up,      eye);
    M[2].x = -forward.x; M[2].y = -forward.y; M[2].z = -forward.z; M[2].w = pVec3Dot(forward, eye);
    M[3].x =          0; M[3].y =          0; M[3].z =          0; M[3].w =                      1;
    return result;
#undef M
#undef F
#undef N
}

Mat4x4 pTranslate(element_type x, element_type y, element_type z) {
#define M result.columns
    Mat4x4 result = {0};

    M[0].x = 1; M[0].y = 0; M[0].z = 0; M[0].w = x;
    M[1].x = 0; M[1].y = 1; M[1].z = 0; M[1].w = y;
    M[2].x = 0; M[2].y = 0; M[2].z = 1; M[2].w = z;
    M[3].x = 0; M[3].y = 0; M[3].z = 0; M[3].w = 1;
    return result;
#undef M
}
Mat4x4 pScale(element_type x, element_type y, element_type z) {
#define M result.columns
    Mat4x4 result = {0};

    M[0].x = x; M[0].y = 0; M[0].z = 0; M[0].w = 0;
    M[1].x = 0; M[1].y = y; M[1].z = 0; M[1].w = 0;
    M[2].x = 0; M[2].y = 0; M[2].z = z; M[2].w = 0;
    M[3].x = 0; M[3].y = 0; M[3].z = 0; M[3].w = 1;
    return result;
#undef M
}

Mat4x4 pRotateX(element_type radians) {
#define M result.columns
    Mat4x4 result = {0};

    M[0].x = 1; M[0].y =             0; M[0].z =             0; M[0].w = 0;
    M[1].x = 0; M[1].y = pCos(radians); M[1].z =-pSin(radians); M[1].w = 0;
    M[2].x = 0; M[2].y = pSin(radians); M[2].z = pCos(radians); M[2].w = 0;
    M[3].x = 0; M[3].y =             0; M[3].z =             0; M[3].w = 1;
    return result;
#undef M
}
Mat4x4 pRotateY(element_type radians) {
#define M result.columns
    Mat4x4 result = {0};

    M[0].x = pCos(radians); M[0].y =             0; M[0].z = pSin(radians); M[0].w = 0;
    M[1].x =             0; M[1].y =             1; M[1].z =             0; M[1].w = 0;
    M[2].x =-pSin(radians); M[2].y =             0; M[2].z = pCos(radians); M[2].w = 0;
    M[3].x =             0; M[3].y =             0; M[3].z =             0; M[3].w = 1;
    return result;
#undef M
}
Mat4x4 pRotateZ(element_type radians) {
#define M result.columns
    Mat4x4 result = {0};

    M[0].x = pCos(radians); M[0].y = pSin(radians); M[0].z =             0; M[0].w = 0;
    M[1].x = pSin(radians); M[1].y = pCos(radians); M[1].z =             0; M[1].w = 0;
    M[2].x =             0; M[2].y =             0; M[2].z =             1; M[2].w = 0;
    M[3].x =             0; M[3].y =             0; M[3].z =             0; M[3].w = 1;
    return result;
#undef M
}

Mat4x4 pRotate (element_type radians, Vec3 axis) {
    
    Vec3 unit_axis = pVec3Normalize(axis);
    element_type m00 = pCos(radians) + unit_axis.x * (1 - pCos(radians));
    element_type m01 = axis.x*axis.y*(1 - pCos(radians)) - axis.z*pSin(radians);
    element_type m02 = axis.z*axis.z*(1 - pCos(radians)) + axis.y*pSin(radians);

    element_type m10 = axis.y*axis.x*(1 - pCos(radians)) + axis.z*pSin(radians);
    element_type m11 = pCos(radians) + unit_axis.y * (1 - pCos(radians));
    element_type m12 = axis.y*axis.z*(1 - pCos(radians)) - axis.x*pSin(radians);
    
    element_type m20 = axis.z*axis.x*(1 - pCos(radians)) - axis.y*pSin(radians);
    element_type m21 = axis.z*axis.y*(1 - pCos(radians)) + axis.x*pSin(radians);
    element_type m22 = pCos(radians) + unit_axis.z * (1 - pCos(radians));

#define M result.columns
    Mat4x4 result = {0};

    M[0].x = m00; M[0].y = m01; M[0].z = m02; M[0].w = 0;
    M[1].x = m10; M[1].y = m11; M[1].z = m12; M[1].w = 0;
    M[2].x = m20; M[2].y = m21; M[2].z = m22; M[2].w = 0;
    M[3].x =   0; M[3].y =   0; M[3].z =   0; M[3].w = 1;
    return result;
#undef M
}

#undef ALIAS
#undef Vec2
#undef Vec3
#undef Vec4
#undef Mat4x4
#undef pNegate
#undef pAddVector
#undef pSubVector
#undef pDivVector

#undef pAddScalar
#undef pSubScalar
#undef pMulScalar
#undef pDivScalar

#undef pAddMatrix
#undef pSubMatrix
#undef pMulMatrix
#undef pDivMatrix

// linear algebra
#undef pMulVector
#undef pVecMulMatrix
#undef pVecMulScalar
#undef pVecDot
#undef pVecAdd
#undef pVecSub
#undef pVecMul
#undef pVecDiv

#undef pOrtho
#undef pFrustum
#undef pPerspective
#undef pLookAt
#undef pTranslate
#undef pRotate
#undef pRotateX
#undef pRotateY
#undef pRotateZ
#undef pScale


