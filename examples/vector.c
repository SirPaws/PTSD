#include "pio.h"
#include "pmath.h"
#include <math.h>
#include <float.h>

#define  TRUE_COLOR "%Cfg(44, 176, 79)"
#define FALSE_COLOR "%Cfg(176, 66, 44)"

void VectorTestFloat(void);

int main(void) {
    // no need to test the others
    // as they use the same generated funtions
    // so if one works the rest will follow suit
    VectorTestFloat();
}

pBool isEqual(float a, float b) {
  return fabsf(a - b) < FLT_EPSILON;
}

void EqualityFloat(String name, Vec4f   *result, Vec4f   *expected);
void VectorTestFloat(void) {
    Vec4f vector_a = { .elements = { 1, 2, 3, 4 } };
    Vec4f vector_b = { .elements = { 1, 2, 3, 4 } };

    pPrintf("Vector (f32):\n");

    { // ADDITION
        pPrintf("    Addition:\n");
        { // v + s
            Vec4f expected = { .elements = {
                2, 3, 4, 5,
            }};
            Vec4f result = pAdd(vector_a, 1);

            EqualityFloat(pCreateString("Vector + Scalar"), &result, &expected);
        }
        { // v + v
            Vec4f expected = { .elements = {
                2, 4, 6, 8,
            }};
            Vec4f result = pAdd(vector_a, vector_b);

            EqualityFloat(pCreateString("Vector + Vector"), &result, &expected);
        }
    }
    { // SUBTRACTION
        pPrintf("    Subtraction:\n");
        { // v - s
            Vec4f expected = { .elements = {
                0, 1, 2, 3,
            }};
            Vec4f result = pSub(vector_a, 1);

            EqualityFloat(pCreateString("Vector - Scalar"), &result, &expected);
        }
        { // v - v
            Vec4f expected = { .elements = {
                0, 0, 0, 0,
            }};
            Vec4f result = pSub(vector_a, vector_b);

            EqualityFloat(pCreateString("Vector - Vector"), &result, &expected);
        }
    }
    { // DIVISION
        pPrintf("    Division:\n");
        { // v / s
            Vec4f expected = { .elements = {
                1.0f/4.0f, 2.0f/4.0f, 3.0f/4.0f, 4.0f/4.0f,
            }};
            Vec4f result = pDiv(vector_a, 4);

            EqualityFloat(pCreateString("Vector / Scalar"), &result, &expected);
        }
        { // v / v
            Vec4f expected = { .elements = {
                1, 1, 1, 1
            }};
            Vec4f result = pDiv(vector_a, vector_b);

            EqualityFloat(pCreateString("Vector / Vector"), &result, &expected);
        }
    }
    { // MULTIPLICATION 
        pPrintf("    Multiplication:\n");
        { // v * v
            Vec4f expected = { .elements = {
                2, 4, 6, 8
            }};
            Vec4f result = pMul(vector_a, 2);

            EqualityFloat(pCreateString("Vector * Scalar"), &result, &expected);
        }

        { // v * v
            Vec4f expected = {.elements = { 1, 4, 9, 16 }};
            Vec4f result = pMul(vector_a, vector_b);

            EqualityFloat(pCreateString("Vector * Vector"), &result, &expected);
        }
    }
    { // Dot product 
        pPrintf("    Dot product:\n");
        {
            f32 expected = 1*1 + 2*2 + 3*3 + 4*4;
            f32 result = pDot(vector_a, vector_b);

            pPrintf("    ");
            pPrintf("    ");
            if (isEqual(result, expected))
                 pPrintf(TRUE_COLOR);
            else pPrintf(FALSE_COLOR);
            pPrintf("% 7.3f%Cc\n", result);
        }
    }
    { // Cross Product 
        pPrintf("    Cross product:\n");
        {
            Vec4f expected = {.elements = { 
                0, 0, 0, 0 
            }};
            Vec4f result = {0};
            result.xyz = pCross(vector_a.xyz, vector_b.xyz);
            EqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // Perpendicular 2D 
        pPrintf("    Perpendicular 2D:\n");
        {
            Vec4f expected = {.elements = { 
                -2, 1, 0, 0 
            }};
            Vec4f result = {0};
            result.xy = pPerpendicular(vector_a.xy);
            EqualityFloat(pCreateString(""), &result, &expected);
        }
    }
    { // LENGTH SQUARED 
        pPrintf("    Length Squared:\n");
        {
            f32 expected = 1*1 + 2*2 + 3*3 + 4*4;
            f32 result = pLengthSquared(vector_a);

            pPrintf("    ");
            pPrintf("    ");
            if (isEqual(result, expected))
                 pPrintf(TRUE_COLOR);
            else pPrintf(FALSE_COLOR);
            pPrintf("% 7.3f%Cc\n", result);
        }
    }
    { // LENGTH 
        pPrintf("    Length:\n");
        { // v * v
            f32 expected = pSqrt(1.0F*1.0F + 2.0F*2.0F + 3.0F*3.0F + 4.0F*4.0F);
            f32 result   = pLength(vector_a);

            pPrintf("    ");
            pPrintf("    ");
            if (isEqual(result, expected))
                 pPrintf(TRUE_COLOR);
            else pPrintf(FALSE_COLOR);
            pPrintf("% 7.3f%Cc\n", result);
        }
    }
    { // Normalize 
        pPrintf("    Normalize:\n");
        {
            f32 length   = pSqrt(1.0F*1.0F + 2.0F*2.0F + 3.0F*3.0F + 4.0F*4.0F);
            Vec4f expected = {.elements = {
                1.0F/length, 2.0F/length, 3.0F/length, 4.0F/length
            }};
            Vec4f result   = pNormalize(vector_a);

            EqualityFloat(pCreateString(""), &result, &expected);
        }
    }
}

void EqualityFloat(String name, Vec4f *result, Vec4f *expected) {
    pPrintf("    ");
    pPrintf("    ");
    if (name.length) {
        pPrintf("%S: \n", name);
        pPrintf("    ");
        pPrintf("    ");
        pPrintf("    ");
    }
    for (int i = 0; i < 4; i++) {
        f32 left  = result->elements[i];
        f32 right = expected->elements[i];
        if (isEqual(left, right))
             pPrintf(TRUE_COLOR);
        else pPrintf(FALSE_COLOR);
        pPrintf("% 7.3f ", left);
    }
    pPrintf("\n");
    pPrintf("%Cc");
}
