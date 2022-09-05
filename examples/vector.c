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

void EqualityFloat(pstring_t name, Vec4f *result, Vec4f *expected);
void VectorTestFloat(void) {
    Vec4f vector_a = { .elements = { 1, 2, 3, 4 } };
    Vec4f vector_b = { .elements = { 1, 2, 3, 4 } };

    pprintf("Vector (f32):\n");

    { // ADDITION
        pprintf("    Addition:\n");
        { // v + s
            Vec4f expected = { .elements = {
                2, 3, 4, 5,
            }};
            Vec4f result = pAdd(vector_a, 1);

            EqualityFloat(pcreate_string("Vector + Scalar"), &result, &expected);
        }
        { // v + v
            Vec4f expected = { .elements = {
                2, 4, 6, 8,
            }};
            Vec4f result = pAdd(vector_a, vector_b);

            EqualityFloat(pcreate_string("Vector + Vector"), &result, &expected);
        }
    }
    { // SUBTRACTION
        pprintf("    Subtraction:\n");
        { // v - s
            Vec4f expected = { .elements = {
                0, 1, 2, 3,
            }};
            Vec4f result = pSub(vector_a, 1);

            EqualityFloat(pcreate_string("Vector - Scalar"), &result, &expected);
        }
        { // v - v
            Vec4f expected = { .elements = {
                0, 0, 0, 0,
            }};
            Vec4f result = pSub(vector_a, vector_b);

            EqualityFloat(pcreate_string("Vector - Vector"), &result, &expected);
        }
    }
    { // DIVISION
        pprintf("    Division:\n");
        { // v / s
            Vec4f expected = { .elements = {
                1.0f/4.0f, 2.0f/4.0f, 3.0f/4.0f, 4.0f/4.0f,
            }};
            Vec4f result = pDiv(vector_a, 4);

            EqualityFloat(pcreate_string("Vector / Scalar"), &result, &expected);
        }
        { // v / v
            Vec4f expected = { .elements = {
                1, 1, 1, 1
            }};
            Vec4f result = pDiv(vector_a, vector_b);

            EqualityFloat(pcreate_string("Vector / Vector"), &result, &expected);
        }
    }
    { // MULTIPLICATION 
        pprintf("    Multiplication:\n");
        { // v * v
            Vec4f expected = { .elements = {
                2, 4, 6, 8
            }};
            Vec4f result = pMul(vector_a, 2);

            EqualityFloat(pcreate_string("Vector * Scalar"), &result, &expected);
        }

        { // v * v
            Vec4f expected = {.elements = { 1, 4, 9, 16 }};
            Vec4f result = pMul(vector_a, vector_b);

            EqualityFloat(pcreate_string("Vector * Vector"), &result, &expected);
        }
    }
    { // Dot product 
        pprintf("    Dot product:\n");
        {
            f32 expected = 1*1 + 2*2 + 3*3 + 4*4;
            f32 result = pDot(vector_a, vector_b);

            pprintf("    ");
            pprintf("    ");
            if (isEqual(result, expected))
                 pprintf(TRUE_COLOR);
            else pprintf(FALSE_COLOR);
            pprintf("% 7.3f%Cc\n", result);
        }
    }
    { // Cross Product 
        pprintf("    Cross product:\n");
        {
            Vec4f expected = {.elements = { 
                0, 0, 0, 0 
            }};
            Vec4f result = {0};
            result.xyz = pCross(vector_a.xyz, vector_b.xyz);
            EqualityFloat(pcreate_string(""), &result, &expected);
        }
    }
    { // Perpendicular 2D 
        pprintf("    Perpendicular 2D:\n");
        {
            Vec4f expected = {.elements = { 
                -2, 1, 0, 0 
            }};
            Vec4f result = {0};
            result.xy = pPerpendicular(vector_a.xy);
            EqualityFloat(pcreate_string(""), &result, &expected);
        }
    }
    { // LENGTH SQUARED 
        pprintf("    Length Squared:\n");
        {
            f32 expected = 1*1 + 2*2 + 3*3 + 4*4;
            f32 result = pLengthSquared(vector_a);

            pprintf("    ");
            pprintf("    ");
            if (isEqual(result, expected))
                 pprintf(TRUE_COLOR);
            else pprintf(FALSE_COLOR);
            pprintf("% 7.3f%Cc\n", result);
        }
    }
    { // LENGTH 
        pprintf("    Length:\n");
        { // v * v
            f32 expected = pSqrt(1.0F*1.0F + 2.0F*2.0F + 3.0F*3.0F + 4.0F*4.0F);
            f32 result   = pLength(vector_a);

            pprintf("    ");
            pprintf("    ");
            if (isEqual(result, expected))
                 pprintf(TRUE_COLOR);
            else pprintf(FALSE_COLOR);
            pprintf("% 7.3f%Cc\n", result);
        }
    }
    { // Normalize 
        pprintf("    Normalize:\n");
        {
            f32 length   = pSqrt(1.0F*1.0F + 2.0F*2.0F + 3.0F*3.0F + 4.0F*4.0F);
            Vec4f expected = {.elements = {
                1.0F/length, 2.0F/length, 3.0F/length, 4.0F/length
            }};
            Vec4f result   = pNormalize(vector_a);

            EqualityFloat(pcreate_string(""), &result, &expected);
        }
    }
}

void EqualityFloat(pstring_t name, Vec4f *result, Vec4f *expected) {
    pprintf("    ");
    pprintf("    ");
    if (name.length) {
        pprintf("%S: \n", name);
        pprintf("    ");
        pprintf("    ");
        pprintf("    ");
    }
    for (int i = 0; i < 4; i++) {
        f32 left  = result->elements[i];
        f32 right = expected->elements[i];
        if (isEqual(left, right))
             pprintf(TRUE_COLOR);
        else pprintf(FALSE_COLOR);
        pprintf("% 7.3f ", left);
    }
    pprintf("\n");
    pprintf("%Cc");
}
