#include "pio.h"
#include "pmath.h"
#include <math.h>
#include <float.h>


void MatrixTestFloat(void);

int main(void) {
    // no need to test the others
    // as they use the same generated funtions
    // so if one works the rest will follow suit
    MatrixTestFloat();
}


void MatrixEqualityFloat(String name, Mat4x4f *result, Mat4x4f *expected);
void VectorEqualityFloat(String name, Vec4f   *result, Vec4f   *expected);
void MatrixTestFloat(void) {
    Mat4x4f identity = { .elements = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    }};
    Vec4f vector = { .elements = { 1, 2, 3, 4 } };

    pPrintf("Matrix (f32):\n");

    { // ADDITION
        pPrintf("    Addition:\n");
        { // m + s
            Mat4x4f expected = { .elements = {
                2, 1, 1, 1,
                1, 2, 1, 1,
                1, 1, 2, 1,
                1, 1, 1, 2,
            }};
            Mat4x4f result = pAdd(identity, 1);

            MatrixEqualityFloat(pCreateString("Matrix + Scalar"), &result, &expected);
        }
        { // m + v
            Mat4x4f expected = { .elements = {
                2, 2, 3, 4,
                1, 3, 3, 4,
                1, 2, 4, 4,
                1, 2, 3, 5,
            }};
            Mat4x4f result = pAdd(identity, vector);

            MatrixEqualityFloat(pCreateString("Matrix + Vector"), &result, &expected);
        }
        { // m + m
            Mat4x4f expected = { .elements = {
                2, 0, 0, 0,
                0, 2, 0, 0,
                0, 0, 2, 0,
                0, 0, 0, 2,
            }};
            Mat4x4f result = pAdd(identity, identity);

            MatrixEqualityFloat(pCreateString("Matrix + Matrix"), &result, &expected);
        }
    }
    { // SUBTRACTION
        pPrintf("    Subtraction:\n");
        { // m - s
            Mat4x4f expected = { .elements = {
                 0,-1,-1,-1,
                -1, 0,-1,-1,
                -1,-1, 0,-1,
                -1,-1,-1, 0,
            }};
            Mat4x4f result = pSub(identity, 1);

            MatrixEqualityFloat(pCreateString("Matrix - Scalar"), &result, &expected);
        }
        { // m - v
            Mat4x4f expected = { .elements = {
                0,-2,-3,-4,
                -1,-1,-3,-4,
                -1,-2,-2,-4,
                -1,-2,-3,-3,
            }};
            Mat4x4f result = pSub(identity, vector);

            MatrixEqualityFloat(pCreateString("Matrix - Vector"), &result, &expected);
        }
        { // m - m
            Mat4x4f expected = { .elements = {
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
            }};
            Mat4x4f result = pSub(identity, identity);

            MatrixEqualityFloat(pCreateString("Matrix - Matrix"), &result, &expected);
        }
    }
    { // DIVISION
        pPrintf("    Division:\n");
        { // m / s
            Mat4x4f expected = { .elements = {
                1.0F/4.0F,      0.0F,      0.0F,      0.0F,
                0.0F,      1.0F/4.0F,      0.0F,      0.0F,
                0.0F,           0.0F, 1.0F/4.0F,      0.0F,
                0.0F,           0.0F,      0.0F, 1.0F/4.0F,
            }};
            Mat4x4f result = pDiv(identity, 4);

            MatrixEqualityFloat(pCreateString("Matrix / Scalar"), &result, &expected);
        }
        { // m / v
            Mat4x4f expected = { .elements = {
                1.0F/1.0F,      0.0F,      0.0F,      0.0F,
                0.0F,      1.0F/2.0F,      0.0F,      0.0F,
                0.0F,           0.0F, 1.0F/3.0F,      0.0F,
                0.0F,           0.0F,      0.0F, 1.0F/4.0F,
            }};
            Mat4x4f result = pDiv(identity, vector);

            MatrixEqualityFloat(pCreateString("Matrix / Vector"), &result, &expected);
        }
        { // m / v
            Mat4x4f expected = { .elements = {
                1.0F/2.0F,      0.0F,      0.0F,      0.0F,
                0.0F,      1.0F/2.0F,      0.0F,      0.0F,
                0.0F,           0.0F, 1.0F/2.0F,      0.0F,
                0.0F,           0.0F,      0.0F, 1.0F/2.0F,
            }};

            Mat4x4f other  = { .elements = {
                2.0F, 2.0F, 2.0F, 2.0F,
                2.0F, 2.0F, 2.0F, 2.0F,
                2.0F, 2.0F, 2.0F, 2.0F,
                2.0F, 2.0F, 2.0F, 2.0F,
            }};
            Mat4x4f result = pDiv(identity, other);

            MatrixEqualityFloat(pCreateString("Matrix / Matrix"), &result, &expected);
        }
    }
    { // ORTHOGRAPHIC PROJECTION 
        pPrintf("    Orthographic projection:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x =  2.0f/(960.0f);
            expected.columns[0].w = -1.0f;
            expected.columns[1].y =  2.0f/(540.0f);
            expected.columns[1].w = -1.0f;
            expected.columns[2].z = -1.0f;
            expected.columns[2].w =  0.0f;
            expected.columns[3].w =  1.0f;
            Mat4x4f result = pOrtho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);

            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // FRUSTUM 
        pPrintf("    Frustum:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x = -2.0f / (960.0f);
            expected.columns[0].z =  1.0f;
            expected.columns[1].y = -2.0f / (540.0f);
            expected.columns[1].z =  1.0f;
            expected.columns[2].w =  1.0f;
            expected.columns[3].z = -1.0f;
            Mat4x4f result = pFrustum(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);

            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // PERSPECTIVE 
        pPrintf("    Perspective:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x =  0.91f;
            expected.columns[1].y =  1.62f;
            expected.columns[2].w = -1.00f;
            expected.columns[3].z = -1.00f;
            Mat4x4f result = pPerspective(90.0f, 960.0f/540.0f, -1.0f, 1.0f);

            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // LOOK AT 
        pPrintf("    Look At:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x =  0.89f;
            expected.columns[0].w = -0.89f;

            expected.columns[1].y =  0.80f;
            expected.columns[1].z = -0.40f;
            expected.columns[1].w = -0.40f;

            expected.columns[2].y =  0.44f;
            expected.columns[2].z =  0.89f;
            expected.columns[2].w = -3.57f;
            expected.columns[3].w =  1;

            Vec3f eye    = {.elements = {1, 2, 3}};
            Vec3f center = {.elements = {1, 1, 1}};
            Vec3f up     = {.elements = {0, 1, 0}};
            Mat4x4f result = pLookAt(eye, center, up);

            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // TRANSLATE 
        pPrintf("    Translate:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x =  1.000000f;
            expected.columns[0].w =  1.000000f;
            expected.columns[1].y =  1.000000f;
            expected.columns[1].w =  1.000000f;
            expected.columns[2].z =  1.000000f;
            expected.columns[2].w =  1.000000f;
            expected.columns[3].w =  1.000000f;

            Mat4x4f result = pTranslate(1.0f, 1.0f, 1.0f);
            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // SCALE 
        pPrintf("    Translate:\n");
        { // m * v
            Mat4x4f expected = {0};
            expected.columns[0].x =  1.000000f;
            expected.columns[1].y =  1.000000f;
            expected.columns[2].z =  1.000000f;
            expected.columns[3].w =  1.000000f;

            Mat4x4f result = pScale(1.0f, 1.0f, 1.0f);
            MatrixEqualityFloat(pCreateString(""), &result, &expected);
        }
    }
}


pBool isEqual(float a, float b) {
  return fabsf(a - b) < 0.01f;
}

void MatrixEqualityFloat(String name, Mat4x4f *result, Mat4x4f *expected) {
    if (name.length) {
        pPrintf("    ");
        pPrintf("    ");
        pPrintf("%S: \n", name);
    }
    for (int i = 0; i < 4; i++) {
        if (name.length) pPrintf("    ");
        pPrintf("    ");
        pPrintf("    ");
        for (int j = 0; j < 4; j++) {
            f32 left  = result->elements[j + i*4];
            f32 right = expected->elements[j + i*4];
            if (isEqual(left, right))
                 pPrintf("%Cfg(44, 176, 79)");
            else 
                pPrintf("%Cfg(176, 66, 44)");
            pPrintf("% 7.3f ", left);
        }
        pPrintf("\n");
    }
    pPrintf("%Cc");
}

void VectorEqualityFloat(String name, Vec4f *result, Vec4f *expected) {
    pPrintf("    ");
    pPrintf("    ");
    pPrintf("%S: \n", name);
    pPrintf("    ");
    pPrintf("    ");
    pPrintf("    ");
    for (int i = 0; i < 4; i++) {
        f32 left  = result->elements[i];
        f32 right = expected->elements[i];
        if (isEqual(left, right))
             pPrintf("%Cfg(44, 176, 79)");
        else pPrintf("%Cfg(176, 66, 44)");
        pPrintf("% 7.3f ", left);
    }
    pPrintf("\n");
    pPrintf("%Cc");
}
