#include <general.h>

#ifndef element_type
#define element_type f32
#endif

#ifndef prefix
#define prefix Vec
#endif

#ifndef suffix
#define suffix f
#endif

#define PADDING_(type) type PSTD_CONCAT(__pad, __COUNTER__)
#define PADDING2(type) PADDING_(type); PADDING_(type) 
#define PADDING1(type) PADDING_(type)
#define PADDING(num_pads, type) PSTD_CONCAT(PADDING, num_pads)(type) 

#define ALIAS(type, ...) struct { type __VA_ARGS__; }
#define PADDED(type, count,  ...) struct { PADDING(count, type); __VA_ARGS__; }

#define Vec2 PSTD_CONCAT(prefix, PSTD_CONCAT(2, suffix))
#define Vec3 PSTD_CONCAT(prefix, PSTD_CONCAT(3, suffix))
#define Vec4 PSTD_CONCAT(prefix, PSTD_CONCAT(4, suffix))


typedef struct Vec2 Vec2;
struct Vec2 {
    union {
        ALIAS(element_type, x, y);
        ALIAS(element_type, u, v);
        ALIAS(element_type, r, g);
        ALIAS(element_type, X, Y);
        ALIAS(element_type, U, V);
        ALIAS(element_type, R, G);
        element_type elements[2];
    };
};

typedef struct Vec3 Vec3;
struct Vec3 {
    union {
        ALIAS(element_type, x, y, z);
        ALIAS(element_type, u, v, s);
        ALIAS(element_type, r, g, b);
        ALIAS(element_type, X, Y, Z);
        ALIAS(element_type, U, V, S);
        ALIAS(element_type, R, G, B);
        ALIAS(Vec2, xy);
        ALIAS(Vec2, rg);
        ALIAS(Vec2, uv);
        ALIAS(Vec2, XY);
        ALIAS(Vec2, RG);
        ALIAS(Vec2, UV);
        PADDED(element_type, 1, Vec2 yz);
        PADDED(element_type, 1, Vec2 gb);
        PADDED(element_type, 1, Vec2 vs);
        PADDED(element_type, 1, Vec2 YZ);
        PADDED(element_type, 1, Vec2 GB);
        PADDED(element_type, 1, Vec2 VS);
        element_type elements[3];
    };
};

typedef struct Vec4 Vec4;
struct Vec4 {
    union {
        ALIAS(element_type, x, y, z, w);
        ALIAS(element_type, u, v, s, t);
        ALIAS(element_type, r, g, b, a);
        ALIAS(element_type, X, Y, Z, W);
        ALIAS(element_type, U, V, S, T);
        ALIAS(element_type, R, G, B, A);
        ALIAS(Vec2, xy);
        ALIAS(Vec2, rg);
        ALIAS(Vec2, uv);
        ALIAS(Vec2, XY);
        ALIAS(Vec2, RG);
        ALIAS(Vec2, UV);
        PADDED(element_type, 1, Vec2 yz);
        PADDED(element_type, 1, Vec2 gb);
        PADDED(element_type, 1, Vec2 vs);
        PADDED(element_type, 1, Vec2 YZ);
        PADDED(element_type, 1, Vec2 GB);
        PADDED(element_type, 1, Vec2 VS);
        PADDED(element_type, 2, Vec2 zw);
        PADDED(element_type, 2, Vec2 ba);
        PADDED(element_type, 2, Vec2 st);
        PADDED(element_type, 2, Vec2 ZW);
        PADDED(element_type, 2, Vec2 BA);
        PADDED(element_type, 2, Vec2 ST);
        ALIAS(Vec3, xyz);
        ALIAS(Vec3, rgb);
        ALIAS(Vec3, uvs);
        ALIAS(Vec3, XYZ);
        ALIAS(Vec3, RGB);
        ALIAS(Vec3, UVS);
        PADDED(element_type, 1, Vec3 yzw);
        PADDED(element_type, 1, Vec3 gba);
        PADDED(element_type, 1, Vec3 vst);
        PADDED(element_type, 1, Vec3 YZW);
        PADDED(element_type, 1, Vec3 GBA);
        PADDED(element_type, 1, Vec3 VST);
        element_type elements[4];
    };
};

#undef PADDING_
#undef PADDING2
#undef PADDING1
#undef PADDING
#undef ALIAS
#undef PADDED

#define vector_type Vec2
#define VEC2
#include "templates/vector_math.h"
#undef vector_type
#undef VEC2

#define vector_type Vec3
#define VEC3
#include "templates/vector_math.h"
#undef vector_type
#undef VEC3

#define vector_type Vec4
#include "templates/vector_math.h"
#undef vector_type

#undef Vec2
#undef Vec3
#undef Vec4

