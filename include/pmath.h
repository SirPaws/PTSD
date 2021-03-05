
#include "general.h"
#if !defined(PSTD_WASM)
#include <emmintrin.h>
#include <immintrin.h>
#endif
#include <math.h>

#define pSqrt(x)        \
    _Generic((x),       \
        f32: pSqrtF32,  \
        f64: pSqrtF64,  \
        u32: pSqrtU32,  \
        i32: pSqrtI32   \
    )((x))

#define pSqrtR(x)       \
    _Generic((x),       \
        f32: pSqrtRF32, \
        f64: pSqrtRF64, \
        u32: pSqrtRU32, \
        i32: pSqrtRI32  \
    )((x))

#define pTan(x)         \
    _Generic((x),       \
        f32: pTanF32,   \
        f64: pTanF64,   \
        u32: pTanU32,   \
        i32: pTanI32    \
    )((x))

#define pCos(x)         \
    _Generic((x),       \
        f32: pCosF32,   \
        f64: pCosF64,   \
        u32: pCosU32,   \
        i32: pCosI32    \
    )((x))

#define pSin(x)         \
    _Generic((x),       \
        f32: pSinF32,   \
        f64: pSinF64,   \
        u32: pSinU32,   \
        i32: pSinI32    \
    )((x))
PSTD_UNUSED static inline f32 pTanF32(f32 x) {
    return tanf(x);
}

PSTD_UNUSED static inline f32 pTanF64(f64 x) {
    return tan(x);
}

PSTD_UNUSED static inline u32 pTanU32(u32 x) {
    return (u32)tanf(x);
}

PSTD_UNUSED static inline i32 pTanI32(i32 x) {
    return (i32)tanf(x);
}

PSTD_UNUSED static inline f32 pCosF32(f32 x) {
    return cosf(x);
}

PSTD_UNUSED static inline f32 pCosF64(f64 x) {
    return cos(x);
}

PSTD_UNUSED static inline u32 pCosU32(u32 x) {
    return (u32)cosf(x);
}

PSTD_UNUSED static inline i32 pCosI32(i32 x) {
    return (i32)cosf(x);
}

PSTD_UNUSED static inline f32 pSinF32(f32 x) {
    return sinf(x);
}

PSTD_UNUSED static inline f32 pSinF64(f64 x) {
    return sin(x);
}

PSTD_UNUSED static inline u32 pSinU32(u32 x) {
    return (u32)sinf(x);
}

PSTD_UNUSED static inline i32 pSinI32(i32 x) {
    return (i32)sinf(x);
}


#if defined(PSTD_WASM) || defined(PSTD_NO_SEE)
PSTD_UNUSED static inline f32 pSqrtF32(f32 x) {
    return sqrtf(x);
}

PSTD_UNUSED static inline f64 pSqrtF64(f64 x) {
    return sqrt(x);
}
PSTD_UNUSED static inline u32 pSqrtU32(u32 x) {
    return (u32)sqrtf(x);
}

PSTD_UNUSED static inline i32 pSqrtI32(i32 x) {
    return (u32)sqrtf(x);
}

PSTD_UNUSED static inline f32 pSqrtRF32(f32 x) {
    return (1.0F/sqrtf(x));
}

PSTD_UNUSED static inline f64 pSqrtRF64(f64 x) {
    return 1.0/sqrt(x);
}
PSTD_UNUSED static inline u32 pSqrtRU32(u32 x) {
    return (u32)(1.0F/sqrtf(x));
}

PSTD_UNUSED static inline i32 pSqrtRI32(i32 x) {
    return (u32)(1.0F/sqrtf(x));
}
#else
PSTD_UNUSED static inline f32 pSqrtF32(f32 x) {
   __m128 value  = _mm_set_ss(x); 
   __m128 dest   = _mm_sqrt_ss(value);
   return _mm_cvtss_f32(dest);
}

PSTD_UNUSED static inline f64 pSqrtF64(f64 x) {
   __m128 value  = _mm_set1_pd(x); 
   __m128 dest   = _mm_sqrt_ss(value);
    return _mm_cvtsd_f64(dest);
}
PSTD_UNUSED static inline u32 pSqrtU32(u32 x) {
    return (u32)pSqrtF32(x);
}

PSTD_UNUSED static inline i32 pSqrtI32(i32 x) {
    return (i32)pSqrtF32(x);
}

PSTD_UNUSED static inline f32 pSqrtRF32(f32 x) {
   __m128 value  = _mm_set_ss(x); 
   __m128 dest   = _mm_rsqrt_ss(value);
   return _mm_cvtss_f32(dest);
}

PSTD_UNUSED static inline f64 pSqrtRF64(f64 x) {
   __m128 value  = _mm_set1_pd(x); 
   __m128 dest   = _mm_rsqrt_ss(value);
    return _mm_cvtsd_f64(dest);
}
PSTD_UNUSED static inline u32 pSqrtRU32(u32 x) {
    return (u32)pSqrtF32(x);
}

PSTD_UNUSED static inline i32 pSqrtRI32(i32 x) {
    return (i32)pSqrtF32(x);
}
#endif

#define element_type    f32
#define vector_prefix   Vec
#define matrix_prefix   Mat
#define suffix          f
#include "templates/matrix4x4.h"

#undef  element_type
#undef  suffix
#define element_type u32
#define suffix       u
#include "templates/matrix4x4.h"

#undef  element_type
#undef suffix        
#define element_type i32
#define suffix       i
#include "templates/matrix4x4.h"

#define FN(x) ((__typeof(x))NULL)

// negate
#define pNegate(x)              \
    _Generic((x),               \
        Vec2f: pNegateVec2f,    \
        Vec3f: pNegateVec3f,    \
        Vec4f: pNegateVec4f,    \
        Mat4x4f: pNegateMat4x4f,\
        Vec2u: pNegateVec2u,    \
        Vec3u: pNegateVec3u,    \
        Vec4u: pNegateVec4u,    \
        Mat4x4u: pNegateMat4x4u,\
        Vec2i: pNegateVec2i,    \
        Vec3i: pNegateVec3i,    \
        Vec4i: pNegateVec4i,    \
        Mat4x4i: pNegateMat4x4i)\
    ((x))

// add
#define pAdd(a, b)                      \
    _Generic((a),                       \
        Vec2f:                          \
            _Generic((b),               \
                Vec2f: pAddVec2fV,      \
                f32:   pAddVec2fS,      \
                f64:   pAddVec2fS,      \
                i32:   pAddVec2fS,      \
                u32:   pAddVec2fS,      \
                default:                \
                NULL                    \
            ),                          \
        Vec3f:                          \
            _Generic((b),               \
                Vec3f: pAddVec3fV,      \
                f32:   pAddVec3fS,      \
                f64:   pAddVec3fS,      \
                i32:   pAddVec3fS,      \
                u32:   pAddVec3fS,      \
                default: NULL           \
            ),                          \
        Vec4f:                          \
            _Generic((b),               \
                Vec4f: pAddVec4fV,      \
                f32:   pAddVec4fS,      \
                f64:   pAddVec4fS,      \
                i32:   pAddVec4fS,      \
                u32:   pAddVec4fS,      \
                default: NULL           \
            ),                          \
        Mat4x4f:                        \
            _Generic((b),               \
                Mat4x4f: pAddMat4x4fM,  \
                Vec4f:   pAddMat4x4fV,  \
                f32:     pAddMat4x4fS,  \
                f64:     pAddMat4x4fS,  \
                i32:     pAddMat4x4fS,  \
                u32:     pAddMat4x4fS,  \
                default: NULL           \
            ),                          \
        Vec2i:                          \
            _Generic((b),               \
                Vec2i: pAddVec2iV,      \
                f32:   pAddVec2iS,      \
                f64:   pAddVec2iS,      \
                i32:   pAddVec2iS,      \
                u32:   pAddVec2iS,      \
                default: NULL           \
            ),                          \
        Vec3i:                          \
            _Generic((b),               \
                Vec3i: pAddVec3iV,      \
                f32:   pAddVec3iS,      \
                f64:   pAddVec3iS,      \
                i32:   pAddVec3iS,      \
                u32:   pAddVec3iS,      \
                default: NULL           \
            ),                          \
        Vec4i:                          \
            _Generic((b),               \
                Vec4i: pAddVec4iV,      \
                f32:   pAddVec4iS,      \
                f64:   pAddVec4iS,      \
                i32:   pAddVec4iS,      \
                u32:   pAddVec4iS,      \
                default: NULL           \
            ),                          \
        Mat4x4i:                        \
            _Generic((b),               \
                Mat4x4i: pAddMat4x4iM,  \
                Vec4i:   pAddMat4x4iV,  \
                f32:     pAddMat4x4iS,  \
                f64:     pAddMat4x4iS,  \
                i32:     pAddMat4x4iS,  \
                u32:     pAddMat4x4iS,  \
                default: NULL           \
            ),                          \
        Vec2u:                          \
            _Generic((b),               \
                Vec2u: pAddVec2uV,      \
                f32:   pAddVec2uS,      \
                f64:   pAddVec2uS,      \
                i32:   pAddVec2uS,      \
                u32:   pAddVec2uS,      \
                default: NULL           \
            ),                          \
        Vec3u:                          \
            _Generic((b),               \
                Vec3u: pAddVec3uV,      \
                f32:   pAddVec3uS,      \
                f64:   pAddVec3uS,      \
                i32:   pAddVec3uS,      \
                u32:   pAddVec3uS,      \
                default: NULL           \
            ),                          \
        Vec4u:                          \
            _Generic((b),               \
                Vec4u: pAddVec4uV,      \
                f32:   pAddVec4uS,      \
                f64:   pAddVec4uS,      \
                i32:   pAddVec4uS,      \
                u32:   pAddVec4uS,      \
                default: NULL           \
            ),                          \
        Mat4x4u:                        \
            _Generic((b),               \
                Mat4x4u: pAddMat4x4uM,  \
                Vec4u:   pAddMat4x4uV,  \
                f32:     pAddMat4x4uS,  \
                f64:     pAddMat4x4uS,  \
                i32:     pAddMat4x4uS,  \
                u32:     pAddMat4x4uS,  \
                default: NULL           \
            )                           \
        )((a), (b))

// sub
#define pSub(a, b)                      \
    _Generic((a),                       \
        Vec2f:                          \
            _Generic((b),               \
                Vec2f: pSubVec2fV,      \
                f32:   pSubVec2fS,      \
                f64:   pSubVec2fS,      \
                i32:   pSubVec2fS,      \
                u32:   pSubVec2fS,      \
                default: NULL           \
            ),                          \
        Vec3f:                          \
            _Generic((b),               \
                Vec3f: pSubVec3fV,      \
                f32:   pSubVec3fS,      \
                f64:   pSubVec3fS,      \
                i32:   pSubVec3fS,      \
                u32:   pSubVec3fS,      \
                default: NULL           \
            ),                          \
        Vec4f:                          \
            _Generic((b),               \
                Vec4f: pSubVec4fV,      \
                f32:   pSubVec4fS,      \
                f64:   pSubVec4fS,      \
                i32:   pSubVec4fS,      \
                u32:   pSubVec4fS,      \
                default: NULL           \
            ),                          \
        Mat4x4f:                        \
            _Generic((b),               \
                Mat4x4f: pSubMat4x4fM,  \
                Vec4f:   pSubMat4x4fV,  \
                f32:     pSubMat4x4fS,  \
                f64:     pSubMat4x4fS,  \
                i32:     pSubMat4x4fS,  \
                u32:     pSubMat4x4fS,  \
                default: NULL           \
            ),                          \
        Vec2i:                          \
            _Generic((b),               \
                Vec2i: pSubVec2iV,      \
                f32:   pSubVec2iS,      \
                f64:   pSubVec2iS,      \
                i32:   pSubVec2iS,      \
                u32:   pSubVec2iS,      \
                default: NULL           \
            ),                          \
        Vec3i:                          \
            _Generic((b),               \
                Vec3i: pSubVec3iV,      \
                f32:   pSubVec3iS,      \
                f64:   pSubVec3iS,      \
                i32:   pSubVec3iS,      \
                u32:   pSubVec3iS,      \
                default: NULL           \
            ),                          \
        Vec4i:                          \
            _Generic((b),               \
                Vec4i: pSubVec4iV,      \
                f32:   pSubVec4iS,      \
                f64:   pSubVec4iS,      \
                i32:   pSubVec4iS,      \
                u32:   pSubVec4iS,      \
                default: NULL           \
            ),                          \
        Mat4x4i:                        \
            _Generic((b),               \
                Mat4x4i: pSubMat4x4iM,  \
                Vec4i:   pSubMat4x4iV,  \
                f32:     pSubMat4x4iS,  \
                f64:     pSubMat4x4iS,  \
                i32:     pSubMat4x4iS,  \
                u32:     pSubMat4x4iS,  \
                default: NULL           \
            ),                          \
        Vec2u:                          \
            _Generic((b),               \
                Vec2u: pSubVec2uV,      \
                f32:   pSubVec2uS,      \
                f64:   pSubVec2uS,      \
                i32:   pSubVec2uS,      \
                u32:   pSubVec2uS,      \
                default: NULL           \
            ),                          \
        Vec3u:                          \
            _Generic((b),               \
                Vec3u: pSubVec3uV,      \
                f32:   pSubVec3uS,      \
                f64:   pSubVec3uS,      \
                i32:   pSubVec3uS,      \
                u32:   pSubVec3uS,      \
                default: NULL           \
            ),                          \
        Vec4u:                          \
            _Generic((b),               \
                Vec4u: pSubVec4uV,      \
                f32:   pSubVec4uS,      \
                f64:   pSubVec4uS,      \
                i32:   pSubVec4uS,      \
                u32:   pSubVec4uS,      \
                default: NULL           \
            ),                          \
        Mat4x4u:                        \
            _Generic((b),               \
                Mat4x4u: pSubMat4x4uM,  \
                Vec4u:   pSubMat4x4uV,  \
                f32:     pSubMat4x4uS,  \
                f64:     pSubMat4x4uS,  \
                i32:     pSubMat4x4uS,  \
                u32:     pSubMat4x4uS,  \
                default: NULL           \
            )                           \
        )((a), (b))

// div
#define pDiv(a, b)                      \
    _Generic((a),                       \
        Vec2f:                          \
            _Generic((b),               \
                Vec2f: pDivVec2fV,      \
                f32:   pDivVec2fS,      \
                f64:   pDivVec2fS,      \
                i32:   pDivVec2fS,      \
                u32:   pDivVec2fS,      \
                default: NULL           \
            ),                          \
        Vec3f:                          \
            _Generic((b),               \
                Vec3f: pDivVec3fV,      \
                f32:   pDivVec3fS,      \
                f64:   pDivVec3fS,      \
                i32:   pDivVec3fS,      \
                u32:   pDivVec3fS,      \
                default: NULL           \
            ),                          \
        Vec4f:                          \
            _Generic((b),               \
                Vec4f: pDivVec4fV,      \
                f32:   pDivVec4fS,      \
                f64:   pDivVec4fS,      \
                i32:   pDivVec4fS,      \
                u32:   pDivVec4fS,      \
                default: NULL           \
            ),                          \
        Mat4x4f:                        \
            _Generic((b),               \
                Mat4x4f: pDivMat4x4fM,  \
                Vec4f:   pDivMat4x4fV,  \
                f32:     pDivMat4x4fS,  \
                f64:     pDivMat4x4fS,  \
                i32:     pDivMat4x4fS,  \
                u32:     pDivMat4x4fS,  \
                default: NULL           \
            ),                          \
        Vec2i:                          \
            _Generic((b),               \
                Vec2i: pDivVec2iV,      \
                f32:   pDivVec2iS,      \
                f64:   pDivVec2iS,      \
                i32:   pDivVec2iS,      \
                u32:   pDivVec2iS,      \
                default: NULL           \
            ),                          \
        Vec3i:                          \
            _Generic((b),               \
                Vec3i: pDivVec3iV,      \
                f32:   pDivVec3iS,      \
                f64:   pDivVec3iS,      \
                i32:   pDivVec3iS,      \
                u32:   pDivVec3iS,      \
                default: NULL           \
            ),                          \
        Vec4i:                          \
            _Generic((b),               \
                Vec4i: pDivVec4iV,      \
                f32:   pDivVec4iS,      \
                f64:   pDivVec4iS,      \
                i32:   pDivVec4iS,      \
                u32:   pDivVec4iS,      \
                default: NULL           \
            ),                          \
        Mat4x4i:                        \
            _Generic((b),               \
                Mat4x4i: pDivMat4x4iM,  \
                Vec4i:   pDivMat4x4iV,  \
                f32:     pDivMat4x4iS,  \
                f64:     pDivMat4x4iS,  \
                i32:     pDivMat4x4iS,  \
                u32:     pDivMat4x4iS,  \
                default: NULL           \
            ),                          \
        Vec2u:                          \
            _Generic((b),               \
                Vec2u: pDivVec2uV,      \
                f32:   pDivVec2uS,      \
                f64:   pDivVec2uS,      \
                i32:   pDivVec2uS,      \
                u32:   pDivVec2uS,      \
                default: NULL           \
            ),                          \
        Vec3u:                          \
            _Generic((b),               \
                Vec3u: pDivVec3uV,      \
                f32:   pDivVec3uS,      \
                f64:   pDivVec3uS,      \
                i32:   pDivVec3uS,      \
                u32:   pDivVec3uS,      \
                default: NULL           \
            ),                          \
        Vec4u:                          \
            _Generic((b),               \
                Vec4u: pDivVec4uV,      \
                f32:   pDivVec4uS,      \
                f64:   pDivVec4uS,      \
                i32:   pDivVec4uS,      \
                u32:   pDivVec4uS,      \
                default: NULL           \
            ),                          \
        Mat4x4u:                        \
            _Generic((b),               \
                Mat4x4u: pDivMat4x4uM,  \
                Vec4u:   pDivMat4x4uV,  \
                f32:     pDivMat4x4uS,  \
                f64:     pDivMat4x4uS,  \
                i32:     pDivMat4x4uS,  \
                u32:     pDivMat4x4uS,  \
                default: NULL           \
            )                           \
        )((a), (b))

// mul
#define pMul(a, b)                      \
    _Generic((a),                       \
        Vec2f:                          \
            _Generic((b),               \
                Vec2f: pMulVec2fV,      \
                f32:   pMulVec2fS,      \
                f64:   pMulVec2fS,      \
                i32:   pMulVec2fS,      \
                u32:   pMulVec2fS,      \
                default: NULL           \
            ),                          \
        Vec3f:                          \
            _Generic((b),               \
                Vec3f: pMulVec3fV,      \
                f32:   pMulVec3fS,      \
                f64:   pMulVec3fS,      \
                i32:   pMulVec3fS,      \
                u32:   pMulVec3fS,      \
                default: NULL           \
            ),                          \
        Vec4f:                          \
            _Generic((b),               \
                Mat4x4f: pMulVec4fM,    \
                Vec4f:   pMulVec4fV,    \
                f32:     pMulVec4fS,    \
                f64:     pMulVec4fS,    \
                i32:     pMulVec4fS,    \
                u32:     pMulVec4fS,    \
                default: NULL           \
            ),                          \
        Mat4x4f:                        \
            _Generic((b),               \
                Mat4x4f: pMulMat4x4fM,  \
                Vec4f:   pMulMat4x4fV,  \
                f32:     pMulMat4x4fS,  \
                f64:     pMulMat4x4fS,  \
                i32:     pMulMat4x4fS,  \
                u32:     pMulMat4x4fS,  \
                default: NULL           \
            ),                          \
        Vec2i:                          \
            _Generic((b),               \
                Vec2i: pMulVec2iV,      \
                f32:   pMulVec2iS,      \
                f64:   pMulVec2iS,      \
                i32:   pMulVec2iS,      \
                u32:   pMulVec2iS,      \
                default: NULL           \
            ),                          \
        Vec3i:                          \
            _Generic((b),               \
                Vec3i: pMulVec3iV,      \
                f32:   pMulVec3iS,      \
                f64:   pMulVec3iS,      \
                i32:   pMulVec3iS,      \
                u32:   pMulVec3iS,      \
                default: NULL           \
            ),                          \
        Vec4i:                          \
            _Generic((b),               \
                Mat4x4i: pMulVec4iM,    \
                Vec4i:   pMulVec4iV,    \
                f32:     pMulVec4iS,    \
                f64:     pMulVec4iS,    \
                i32:     pMulVec4iS,    \
                u32:     pMulVec4iS,    \
                default: NULL           \
            ),                          \
        Mat4x4i:                        \
            _Generic((b),               \
                Mat4x4i: pMulMat4x4iM,  \
                Vec4i:   pMulMat4x4iV,  \
                f32:     pMulMat4x4iS,  \
                f64:     pMulMat4x4iS,  \
                i32:     pMulMat4x4iS,  \
                u32:     pMulMat4x4iS,  \
                default: NULL           \
            ),                          \
        Vec2u:                          \
            _Generic((b),               \
                Vec2u: pMulVec2uV,      \
                f32:   pMulVec2uS,      \
                f64:   pMulVec2uS,      \
                i32:   pMulVec2uS,      \
                u32:   pMulVec2uS,      \
                default: NULL           \
            ),                          \
        Vec3u:                          \
            _Generic((b),               \
                Vec3u: pMulVec3uV,      \
                f32:   pMulVec3uS,      \
                f64:   pMulVec3uS,      \
                i32:   pMulVec3uS,      \
                u32:   pMulVec3uS,      \
                default: NULL           \
            ),                          \
        Vec4u:                          \
            _Generic((b),               \
                Mat4x4u: pMulVec4uM,    \
                Vec4u:   pMulVec4uV,    \
                f32:     pMulVec4uS,    \
                f64:     pMulVec4uS,    \
                i32:     pMulVec4uS,    \
                u32:     pMulVec4uS,    \
                default: NULL           \
            ),                          \
        Mat4x4u:                        \
            _Generic((b),               \
                Mat4x4u: pMulMat4x4uM,  \
                Vec4u:   pMulMat4x4uV,  \
                f32:     pMulMat4x4uS,  \
                f64:     pMulMat4x4uS,  \
                i32:     pMulMat4x4uS,  \
                u32:     pMulMat4x4uS,  \
                default: NULL           \
            )                           \
        )((a), (b))

// dot
#define pDot(a, b)          \
    _Generic((a),           \
        Vec2f: pDotVec2f,   \
        Vec3f: pDotVec3f,   \
        Vec4f: pDotVec4f,   \
        Vec2u: pDotVec2u,   \
        Vec3u: pDotVec3u,   \
        Vec4u: pDotVec4u,   \
        Vec2i: pDotVec2i,   \
        Vec3i: pDotVec3i,   \
        Vec4i: pDotVec4i    \
    )((a), (b))

// length squared
#define pLengthSquared(a)           \
    _Generic((a),                   \
        Vec2f: pLengthSquaredVec2f, \
        Vec3f: pLengthSquaredVec3f, \
        Vec4f: pLengthSquaredVec4f, \
        Vec2u: pLengthSquaredVec2u, \
        Vec3u: pLengthSquaredVec3u, \
        Vec4u: pLengthSquaredVec4u, \
        Vec2i: pLengthSquaredVec2i, \
        Vec3i: pLengthSquaredVec3i, \
        Vec4i: pLengthSquaredVec4i  \
    )((a))

// length
#define pLength(a)          \
    _Generic((a),           \
        Vec2f: pLengthVec2f,\
        Vec3f: pLengthVec3f,\
        Vec4f: pLengthVec4f,\
        Vec2u: pLengthVec2u,\
        Vec3u: pLengthVec3u,\
        Vec4u: pLengthVec4u,\
        Vec2i: pLengthVec2i,\
        Vec3i: pLengthVec3i,\
        Vec4i: pLengthVec4i \
    )((a))

// normalize
#define pNormalize(a)           \
    _Generic((a),               \
        Vec2f: pNormalizeVec2f, \
        Vec3f: pNormalizeVec3f, \
        Vec4f: pNormalizeVec4f, \
        Vec2u: pNormalizeVec2u, \
        Vec3u: pNormalizeVec3u, \
        Vec4u: pNormalizeVec4u, \
        Vec2i: pNormalizeVec2i, \
        Vec3i: pNormalizeVec3i, \
        Vec4i: pNormalizeVec4i  \
    )((a))

// project
#define pProject(a, b)          \
    _Generic((a),               \
        Vec2f: pProjectVec2f,   \
        Vec3f: pProjectVec3f,   \
        Vec4f: pProjectVec4f,   \
        Vec2u: pProjectVec2u,   \
        Vec3u: pProjectVec3u,   \
        Vec4u: pProjectVec4u,   \
        Vec2i: pProjectVec2i,   \
        Vec3i: pProjectVec3i,   \
        Vec4i: pProjectVec4i    \
    )((a), (b))

// reject
#define pReject(a, b)       \
    _Generic((a),           \
        Vec2f: pRejectVec2f,\
        Vec3f: pRejectVec3f,\
        Vec4f: pRejectVec4f,\
        Vec2u: pRejectVec2u,\
        Vec3u: pRejectVec3u,\
        Vec4u: pRejectVec4u,\
        Vec2i: pRejectVec2i,\
        Vec3i: pRejectVec3i,\
        Vec4i: pRejectVec4i \
    )((a), (b))

// perpendicular
#define pPerpendicular(a)           \
    _Generic((a),                   \
        Vec2f: pPerpendicularVec2f, \
        Vec2u: pPerpendicularVec2u, \
        Vec2i: pPerpendicularVec2i  \
    )((a))

// cross
#define pCross(a, b)        \
    _Generic((a),           \
        Vec3f: pCrossVec3f, \
        Vec3u: pCrossVec3u, \
        Vec3i: pCrossVec3i  \
    )((a), (b))

// ortho
#define pOrtho(left, right, bottom, top, near, far) \
    _Generic((left),                                \
        f32: pOrthoMat4x4f,                         \
        u32: pOrthoMat4x4u,                         \
        i32: pOrthoMat4x4i                          \
    )((left), (right), (bottom), (top), (near), (far))

// frustum
#define pFrustum(left, right, bottom, top, near, far)   \
    _Generic((left),                                    \
        f32: pFrustumMat4x4f,                           \
        u32: pFrustumMat4x4u,                           \
        i32: pFrustumMat4x4i                            \
    )((left), (right), (bottom), (top), (near), (far))

// perspective
#define pPerspective(fov, aspect, near, far)\
    _Generic((fov),                         \
        f32: pPerspectiveMat4x4f,           \
        u32: pPerspectiveMat4x4u,           \
        i32: pPerspectiveMat4x4i            \
    )((fov), (aspect), (near), (far))

// look at
#define pLookAt(eye, center, up)\
    _Generic((eye),             \
        Vec3f: pLookAtMat4x4f,  \
        Vec3u: pLookAtMat4x4u,  \
        Vec3i: pLookAtMat4x4i   \
    )((eye), (center), (up))

// translate
#define pTranslate(x, y, z)     \
    _Generic((x),               \
        f32: pTranslateMat4x4f, \
        u32: pTranslateMat4x4u, \
        i32: pTranslateMat4x4i  \
    )((x), (y), (z))

// scale
#define pScale(x, y, z)     \
    _Generic((x),           \
        f32: pScaleMat4x4f, \
        u32: pScaleMat4x4u, \
        i32: pScaleMat4x4i  \
    )((x), (y), (z))

// rotate x
#define pRotateX(radians)    \
    _Generic((radians),      \
        f32: pRotateXMat4x4f,\
        u32: pRotateXMat4x4u,\
        i32: pRotateXMat4x4i \
    )((radians))

// rotate y
#define pRotateY(radians)       \
    _Generic((radians),         \
        f32: pRotateYMat4x4f,   \
        u32: pRotateYMat4x4u,   \
        i32: pRotateYMat4x4i    \
    )((radians))

// rotate z
#define pRotateZ(radians)       \
    _Generic((radians),         \
        f32: pRotateZMat4x4f,   \
        u32: pRotateZMat4x4u,   \
        i32: pRotateZMat4x4i    \
    )((radians))

// rotate
#define pRotate (radians, axis) \
    _Generic((radians),         \
        f32: pRotateMat4x4f,    \
        u32: pRotateMat4x4u,    \
        i32: pRotateMat4x4i     \
    )((radians), (axis))






