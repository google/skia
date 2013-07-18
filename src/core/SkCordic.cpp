/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCordic.h"
#include "SkMathPriv.h"
#include "Sk64.h"

// 0x20000000 equals pi / 4
const int32_t kATanDegrees[] = { 0x20000000,
    0x12E4051D, 0x9FB385B, 0x51111D4, 0x28B0D43, 0x145D7E1, 0xA2F61E, 0x517C55,
    0x28BE53, 0x145F2E, 0xA2F98, 0x517CC, 0x28BE6, 0x145F3, 0xA2F9, 0x517C,
    0x28BE, 0x145F, 0xA2F, 0x517, 0x28B, 0x145, 0xA2, 0x51, 0x28, 0x14,
    0xA, 0x5, 0x2, 0x1 };

const int32_t kFixedInvGain1 = 0x18bde0bb;  // 0.607252935

static void SkCircularRotation(int32_t* x0, int32_t* y0, int32_t* z0)
{
    int32_t t = 0;
    int32_t x = *x0;
    int32_t y = *y0;
    int32_t z = *z0;
    const int32_t* tanPtr = kATanDegrees;
   do {
        int32_t x1 = y >> t;
        int32_t y1 = x >> t;
        int32_t tan = *tanPtr++;
        if (z >= 0) {
            x -= x1;
            y += y1;
            z -= tan;
        } else {
            x += x1;
            y -= y1;
            z += tan;
        }
   } while (++t < 16); // 30);
    *x0 = x;
    *y0 = y;
    *z0 = z;
}

SkFixed SkCordicSinCos(SkFixed radians, SkFixed* cosp)
{
    int32_t scaledRadians = radians * 0x28be;   // scale radians to 65536 / PI()
    int quadrant = scaledRadians >> 30;
    quadrant += 1;
    if (quadrant & 2)
        scaledRadians = -scaledRadians + 0x80000000;
    /* |a| <= 90 degrees as a 1.31 number */
    SkFixed sin = 0;
    SkFixed cos = kFixedInvGain1;
    SkCircularRotation(&cos, &sin, &scaledRadians);
    Sk64 scaled;
    scaled.setMul(sin, 0x6488d);
    sin = scaled.fHi;
    scaled.setMul(cos, 0x6488d);
    if (quadrant & 2)
        scaled.fHi = - scaled.fHi;
    *cosp = scaled.fHi;
    return sin;
}

SkFixed SkCordicTan(SkFixed a)
{
    int32_t cos;
    int32_t sin = SkCordicSinCos(a, &cos);
    return SkFixedDiv(sin, cos);
}

static int32_t SkCircularVector(int32_t* y0, int32_t* x0, int32_t vecMode)
{
    int32_t x = *x0;
    int32_t y = *y0;
    int32_t z = 0;
    int32_t t = 0;
    const int32_t* tanPtr = kATanDegrees;
   do {
        int32_t x1 = y >> t;
        int32_t y1 = x >> t;
        int32_t tan = *tanPtr++;
        if (y < vecMode) {
            x -= x1;
            y += y1;
            z -= tan;
        } else {
            x += x1;
            y -= y1;
            z += tan;
        }
   } while (++t < 16);  // 30
    Sk64 scaled;
    scaled.setMul(z, 0x6488d); // scale back into the SkScalar space (0x100000000/0x28be)
   return scaled.fHi;
}

SkFixed SkCordicASin(SkFixed a) {
    int32_t sign = SkExtractSign(a);
    int32_t z = SkFixedAbs(a);
    if (z >= SK_Fixed1)
        return SkApplySign(SK_FixedPI>>1, sign);
    int32_t x = kFixedInvGain1;
    int32_t y = 0;
    z *= 0x28be;
    z = SkCircularVector(&y, &x, z);
    z = SkApplySign(z, ~sign);
    return z;
}

SkFixed SkCordicACos(SkFixed a) {
    int32_t z = SkCordicASin(a);
    z = (SK_FixedPI>>1) - z;
    return z;
}

SkFixed SkCordicATan2(SkFixed y, SkFixed x) {
    if ((x | y) == 0)
        return 0;
    int32_t xsign = SkExtractSign(x);
    x = SkFixedAbs(x);
    int32_t result = SkCircularVector(&y, &x, 0);
    if (xsign) {
        int32_t rsign = SkExtractSign(result);
        if (y == 0)
            rsign = 0;
        SkFixed pi = SkApplySign(SK_FixedPI, rsign);
        result = pi - result;
    }
    return result;
}

const int32_t kATanHDegrees[] = {
    0x1661788D, 0xA680D61, 0x51EA6FC, 0x28CBFDD, 0x1460E34,
    0xA2FCE8, 0x517D2E, 0x28BE6E, 0x145F32,
    0xA2F98, 0x517CC, 0x28BE6, 0x145F3, 0xA2F9, 0x517C,
    0x28BE, 0x145F, 0xA2F, 0x517, 0x28B, 0x145, 0xA2, 0x51, 0x28, 0x14,
    0xA, 0x5, 0x2, 0x1 };

const int32_t kFixedInvGain2 = 0x31330AAA;  // 1.207534495

static void SkHyperbolic(int32_t* x0, int32_t* y0, int32_t* z0, int mode)
{
    int32_t t = 1;
    int32_t x = *x0;
    int32_t y = *y0;
    int32_t z = *z0;
    const int32_t* tanPtr = kATanHDegrees;
    int k = -3;
    do {
        int32_t x1 = y >> t;
        int32_t y1 = x >> t;
        int32_t tan = *tanPtr++;
        int count = 2 + (k >> 31);
        if (++k == 1)
            k = -2;
        do {
            if (((y >> 31) & mode) | ~((z >> 31) | mode)) {
                x += x1;
                y += y1;
                z -= tan;
            } else {
                x -= x1;
                y -= y1;
                z += tan;
            }
        } while (--count);
    } while (++t < 30);
    *x0 = x;
    *y0 = y;
    *z0 = z;
}

SkFixed SkCordicLog(SkFixed a) {
    a *= 0x28be;
    int32_t x = a + 0x28BE60DB; // 1.0
    int32_t y = a - 0x28BE60DB;
    int32_t z = 0;
    SkHyperbolic(&x, &y, &z, -1);
    Sk64 scaled;
    scaled.setMul(z, 0x6488d);
    z = scaled.fHi;
    return z << 1;
}

SkFixed SkCordicExp(SkFixed a) {
    int32_t cosh = kFixedInvGain2;
    int32_t sinh = 0;
    SkHyperbolic(&cosh, &sinh, &a, 0);
    return cosh + sinh;
}

#ifdef SK_DEBUG

#include "SkFloatingPoint.h"

void SkCordic_UnitTest()
{
#if defined(SK_SUPPORT_UNITTEST)
    float val;
    for (float angle = -720; angle < 720; angle += 30) {
        float radian = angle * 3.1415925358f / 180.0f;
        SkFixed f_angle = SkFloatToFixed(radian);
    // sincos
        float sine = sinf(radian);
        float cosine = cosf(radian);
        SkFixed f_cosine;
        SkFixed f_sine = SkCordicSinCos(f_angle, &f_cosine);
        float sine2 = (float) f_sine / 65536.0f;
        float cosine2 = (float) f_cosine / 65536.0f;
        float error = fabsf(sine - sine2);
        if (error > 0.001)
            SkDebugf("sin error : angle = %g ; sin = %g ; cordic = %g\n", angle, sine, sine2);
        error = fabsf(cosine - cosine2);
        if (error > 0.001)
            SkDebugf("cos error : angle = %g ; cos = %g ; cordic = %g\n", angle, cosine, cosine2);
    // tan
        float _tan = tanf(radian);
        SkFixed f_tan = SkCordicTan(f_angle);
        float tan2 = (float) f_tan / 65536.0f;
        error = fabsf(_tan - tan2);
        if (error > 0.05 && fabsf(_tan) < 1e6)
            SkDebugf("tan error : angle = %g ; tan = %g ; cordic = %g\n", angle, _tan, tan2);
    }
    for (val = -1; val <= 1; val += .1f) {
        SkFixed f_val = SkFloatToFixed(val);
    // asin
        float arcsine = asinf(val);
        SkFixed f_arcsine = SkCordicASin(f_val);
        float arcsine2 = (float) f_arcsine / 65536.0f;
        float error = fabsf(arcsine - arcsine2);
        if (error > 0.001)
            SkDebugf("asin error : val = %g ; asin = %g ; cordic = %g\n", val, arcsine, arcsine2);
    }
#if 1
    for (val = -1; val <= 1; val += .1f) {
#else
    val = .5; {
#endif
        SkFixed f_val = SkFloatToFixed(val);
    // acos
        float arccos = acosf(val);
        SkFixed f_arccos = SkCordicACos(f_val);
        float arccos2 = (float) f_arccos / 65536.0f;
        float error = fabsf(arccos - arccos2);
        if (error > 0.001)
            SkDebugf("acos error : val = %g ; acos = %g ; cordic = %g\n", val, arccos, arccos2);
    }
    // atan2
#if 1
    for (val = -1000; val <= 1000; val += 500.f) {
        for (float val2 = -1000; val2 <= 1000; val2 += 500.f) {
#else
            val = 0; {
            float val2 = -1000; {
#endif
            SkFixed f_val = SkFloatToFixed(val);
            SkFixed f_val2 = SkFloatToFixed(val2);
            float arctan = atan2f(val, val2);
            SkFixed f_arctan = SkCordicATan2(f_val, f_val2);
            float arctan2 = (float) f_arctan / 65536.0f;
            float error = fabsf(arctan - arctan2);
            if (error > 0.001)
                SkDebugf("atan2 error : val = %g ; val2 = %g ; atan2 = %g ; cordic = %g\n", val, val2, arctan, arctan2);
        }
    }
    // log
#if 1
    for (val = 0.125f; val <= 8.f; val *= 2.0f) {
#else
    val = .5; {
#endif
        SkFixed f_val = SkFloatToFixed(val);
    // acos
        float log = logf(val);
        SkFixed f_log = SkCordicLog(f_val);
        float log2 = (float) f_log / 65536.0f;
        float error = fabsf(log - log2);
        if (error > 0.001)
            SkDebugf("log error : val = %g ; log = %g ; cordic = %g\n", val, log, log2);
    }
    // exp
#endif
}

#endif
