/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// http://metamerist.com/cbrt/CubeRoot.cpp
//

#include <math.h>
#include "CubicUtilities.h"

#define TEST_ALTERNATIVES 0
#if TEST_ALTERNATIVES
typedef float  (*cuberootfnf) (float);
typedef double (*cuberootfnd) (double);

// estimate bits of precision (32-bit float case)
inline int bits_of_precision(float a, float b)
{
    const double kd = 1.0 / log(2.0);

    if (a==b)
        return 23;

    const double kdmin = pow(2.0, -23.0);

    double d = fabs(a-b);
    if (d < kdmin)
        return 23;

    return int(-log(d)*kd);
}

// estiamte bits of precision (64-bit double case)
inline int bits_of_precision(double a, double b)
{
    const double kd = 1.0 / log(2.0);

    if (a==b)
        return 52;

    const double kdmin = pow(2.0, -52.0);

    double d = fabs(a-b);
    if (d < kdmin)
        return 52;

    return int(-log(d)*kd);
}

// cube root via x^(1/3)
static float pow_cbrtf(float x)
{
    return (float) pow(x, 1.0f/3.0f);
}

// cube root via x^(1/3)
static double pow_cbrtd(double x)
{
    return pow(x, 1.0/3.0);
}

// cube root approximation using bit hack for 32-bit float
static  float cbrt_5f(float f)
{
    unsigned int* p = (unsigned int *) &f;
    *p = *p/3 + 709921077;
    return f;
}
#endif

// cube root approximation using bit hack for 64-bit float
// adapted from Kahan's cbrt
static  double cbrt_5d(double d)
{
    const unsigned int B1 = 715094163;
    double t = 0.0;
    unsigned int* pt = (unsigned int*) &t;
    unsigned int* px = (unsigned int*) &d;
    pt[1]=px[1]/3+B1;
    return t;
}

#if TEST_ALTERNATIVES
// cube root approximation using bit hack for 64-bit float
// adapted from Kahan's cbrt
#if 0
static  double quint_5d(double d)
{
    return sqrt(sqrt(d));

    const unsigned int B1 = 71509416*5/3;
    double t = 0.0;
    unsigned int* pt = (unsigned int*) &t;
    unsigned int* px = (unsigned int*) &d;
    pt[1]=px[1]/5+B1;
    return t;
}
#endif

// iterative cube root approximation using Halley's method (float)
static  float cbrta_halleyf(const float a, const float R)
{
    const float a3 = a*a*a;
    const float b= a * (a3 + R + R) / (a3 + a3 + R);
    return b;
}
#endif

// iterative cube root approximation using Halley's method (double)
static  double cbrta_halleyd(const double a, const double R)
{
    const double a3 = a*a*a;
    const double b= a * (a3 + R + R) / (a3 + a3 + R);
    return b;
}

#if TEST_ALTERNATIVES
// iterative cube root approximation using Newton's method (float)
static  float cbrta_newtonf(const float a, const float x)
{
//    return (1.0 / 3.0) * ((a + a) + x / (a * a));
    return a - (1.0f / 3.0f) * (a - x / (a*a));
}

// iterative cube root approximation using Newton's method (double)
static  double cbrta_newtond(const double a, const double x)
{
    return (1.0/3.0) * (x / (a*a) + 2*a);
}

// cube root approximation using 1 iteration of Halley's method (double)
static double halley_cbrt1d(double d)
{
    double a = cbrt_5d(d);
    return cbrta_halleyd(a, d);
}

// cube root approximation using 1 iteration of Halley's method (float)
static float halley_cbrt1f(float d)
{
    float a = cbrt_5f(d);
    return cbrta_halleyf(a, d);
}

// cube root approximation using 2 iterations of Halley's method (double)
static double halley_cbrt2d(double d)
{
    double a = cbrt_5d(d);
    a = cbrta_halleyd(a, d);
    return cbrta_halleyd(a, d);
}
#endif

// cube root approximation using 3 iterations of Halley's method (double)
static double halley_cbrt3d(double d)
{
    double a = cbrt_5d(d);
    a = cbrta_halleyd(a, d);
    a = cbrta_halleyd(a, d);
    return cbrta_halleyd(a, d);
}

#if TEST_ALTERNATIVES
// cube root approximation using 2 iterations of Halley's method (float)
static float halley_cbrt2f(float d)
{
    float a = cbrt_5f(d);
    a = cbrta_halleyf(a, d);
    return cbrta_halleyf(a, d);
}

// cube root approximation using 1 iteration of Newton's method (double)
static double newton_cbrt1d(double d)
{
    double a = cbrt_5d(d);
    return cbrta_newtond(a, d);
}

// cube root approximation using 2 iterations of Newton's method (double)
static double newton_cbrt2d(double d)
{
    double a = cbrt_5d(d);
    a = cbrta_newtond(a, d);
    return cbrta_newtond(a, d);
}

// cube root approximation using 3 iterations of Newton's method (double)
static double newton_cbrt3d(double d)
{
    double a = cbrt_5d(d);
    a = cbrta_newtond(a, d);
    a = cbrta_newtond(a, d);
    return cbrta_newtond(a, d);
}

// cube root approximation using 4 iterations of Newton's method (double)
static double newton_cbrt4d(double d)
{
    double a = cbrt_5d(d);
    a = cbrta_newtond(a, d);
    a = cbrta_newtond(a, d);
    a = cbrta_newtond(a, d);
    return cbrta_newtond(a, d);
}

// cube root approximation using 2 iterations of Newton's method (float)
static float newton_cbrt1f(float d)
{
    float a = cbrt_5f(d);
    return cbrta_newtonf(a, d);
}

// cube root approximation using 2 iterations of Newton's method (float)
static float newton_cbrt2f(float d)
{
    float a = cbrt_5f(d);
    a = cbrta_newtonf(a, d);
    return cbrta_newtonf(a, d);
}

// cube root approximation using 3 iterations of Newton's method (float)
static float newton_cbrt3f(float d)
{
    float a = cbrt_5f(d);
    a = cbrta_newtonf(a, d);
    a = cbrta_newtonf(a, d);
    return cbrta_newtonf(a, d);
}

// cube root approximation using 4 iterations of Newton's method (float)
static float newton_cbrt4f(float d)
{
    float a = cbrt_5f(d);
    a = cbrta_newtonf(a, d);
    a = cbrta_newtonf(a, d);
    a = cbrta_newtonf(a, d);
    return cbrta_newtonf(a, d);
}

static double TestCubeRootf(const char* szName, cuberootfnf cbrt, double rA, double rB, int rN)
{
    const int N = rN;

    float dd = float((rB-rA) / N);

    // calculate 1M numbers
    int i=0;
    float d = (float) rA;

    double s = 0.0;

    for(d=(float) rA, i=0; i<N; i++, d += dd)
    {
        s += cbrt(d);
    }

    double bits = 0.0;
    double worstx=0.0;
    double worsty=0.0;
    int minbits=64;

    for(d=(float) rA, i=0; i<N; i++, d += dd)
    {
        float a = cbrt((float) d);
        float b = (float) pow((double) d, 1.0/3.0);

        int bc = bits_of_precision(a, b);
        bits += bc;

        if (b > 1.0e-6)
        {
            if (bc < minbits)
            {
                minbits = bc;
                worstx = d;
                worsty = a;
            }
        }
    }

    bits /= N;

    printf(" %3d mbp  %6.3f abp\n", minbits, bits);

    return s;
}


static double TestCubeRootd(const char* szName, cuberootfnd cbrt, double rA, double rB, int rN)
{
    const int N = rN;

    double dd = (rB-rA) / N;

    int i=0;

    double s = 0.0;
    double d = 0.0;

    for(d=rA, i=0; i<N; i++, d += dd)
    {
        s += cbrt(d);
    }


    double bits = 0.0;
    double worstx = 0.0;
    double worsty = 0.0;
    int minbits = 64;
    for(d=rA, i=0; i<N; i++, d += dd)
    {
        double a = cbrt(d);
        double b = pow(d, 1.0/3.0);

        int bc = bits_of_precision(a, b); // min(53, count_matching_bitsd(a, b) - 12);
        bits += bc;

        if (b > 1.0e-6)
        {
            if (bc < minbits)
            {
                bits_of_precision(a, b);
                minbits = bc;
                worstx = d;
                worsty = a;
            }
        }
    }

    bits /= N;

    printf(" %3d mbp  %6.3f abp\n", minbits, bits);

    return s;
}

static int _tmain()
{
    // a million uniform steps through the range from 0.0 to 1.0
    // (doing uniform steps in the log scale would be better)
    double a = 0.0;
    double b = 1.0;
    int n = 1000000;

    printf("32-bit float tests\n");
    printf("----------------------------------------\n");
    TestCubeRootf("cbrt_5f", cbrt_5f, a, b, n);
    TestCubeRootf("pow", pow_cbrtf, a, b, n);
    TestCubeRootf("halley x 1", halley_cbrt1f, a, b, n);
    TestCubeRootf("halley x 2", halley_cbrt2f, a, b, n);
    TestCubeRootf("newton x 1", newton_cbrt1f, a, b, n);
    TestCubeRootf("newton x 2", newton_cbrt2f, a, b, n);
    TestCubeRootf("newton x 3", newton_cbrt3f, a, b, n);
    TestCubeRootf("newton x 4", newton_cbrt4f, a, b, n);
    printf("\n\n");

    printf("64-bit double tests\n");
    printf("----------------------------------------\n");
    TestCubeRootd("cbrt_5d", cbrt_5d, a, b, n);
    TestCubeRootd("pow", pow_cbrtd, a, b, n);
    TestCubeRootd("halley x 1", halley_cbrt1d, a, b, n);
    TestCubeRootd("halley x 2", halley_cbrt2d, a, b, n);
    TestCubeRootd("halley x 3", halley_cbrt3d, a, b, n);
    TestCubeRootd("newton x 1", newton_cbrt1d, a, b, n);
    TestCubeRootd("newton x 2", newton_cbrt2d, a, b, n);
    TestCubeRootd("newton x 3", newton_cbrt3d, a, b, n);
    TestCubeRootd("newton x 4", newton_cbrt4d, a, b, n);
    printf("\n\n");

    return 0;
}
#endif

double cube_root(double x) {
    return halley_cbrt3d(x);
}

#if TEST_ALTERNATIVES
// http://bytes.com/topic/c/answers/754588-tips-find-cube-root-program-using-c
/* cube root */
int icbrt(int n) {
    int t=0, x=(n+2)/3; /* works for n=0 and n>=1 */
    for(; t!=x;) {
        int x3=x*x*x;
        t=x;
        x*=(2*n + x3);
        x/=(2*x3 + n);
    }
    return x ; /* always(?) equal to floor(n^(1/3)) */
}
#endif
