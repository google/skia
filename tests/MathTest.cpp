#include "Test.h"
#include "SkPoint.h"
#include "SkRandom.h"

#if defined(SkLONGLONG)
static int symmetric_fixmul(int a, int b) {
    int sa = SkExtractSign(a);
    int sb = SkExtractSign(b);
    
    a = SkApplySign(a, sa);
    b = SkApplySign(b, sb);
    
#if 1
    int c = (int)(((SkLONGLONG)a * b) >> 16);
    
    return SkApplySign(c, sa ^ sb);
#else
    SkLONGLONG ab = (SkLONGLONG)a * b;
    if (sa ^ sb) {
        ab = -ab;
    }
    return ab >> 16;
#endif
}
#endif

static void check_length(skiatest::Reporter* reporter,
                         const SkPoint& p, SkScalar targetLen) {
#ifdef SK_CAN_USE_FLOAT
    float x = SkScalarToFloat(p.fX);
    float y = SkScalarToFloat(p.fY);
    float len = sk_float_sqrt(x*x + y*y);
    
    len /= SkScalarToFloat(targetLen);
    
    REPORTER_ASSERT(reporter, len > 0.999f && len < 1.001f);
#endif
}

#if defined(SK_CAN_USE_FLOAT)

static float nextFloat(SkRandom& rand) {
    SkFloatIntUnion data;
    data.fSignBitInt = rand.nextU();
    return data.fFloat;
}

/*  returns true if a == b as resulting from (int)x. Since it is undefined
 what to do if the float exceeds 2^32-1, we check for that explicitly.
 */
static bool equal_float_native_skia(float x, uint32_t ni, uint32_t si) {
    if (!(x == x)) {    // NAN
        return si == SK_MaxS32 || si == SK_MinS32;
    }
    // for out of range, C is undefined, but skia always should return NaN32
    if (x > SK_MaxS32) {
        return si == SK_MaxS32;
    }
    if (x < -SK_MaxS32) {
        return si == SK_MinS32;
    }
    return si == ni;
}

static void assert_float_equal(skiatest::Reporter* reporter, const char op[],
                               float x, uint32_t ni, uint32_t si) {
    if (!equal_float_native_skia(x, ni, si)) {
        SkString desc;
        desc.printf("%s float %g bits %x native %x skia %x\n", op, x, ni, si);
        reporter->reportFailed(desc);
    }
}

static void test_float_cast(skiatest::Reporter* reporter, float x) {
    int ix = (int)x;
    int iix = SkFloatToIntCast(x);
    assert_float_equal(reporter, "cast", x, ix, iix);
}

static void test_float_floor(skiatest::Reporter* reporter, float x) {
    int ix = (int)floor(x);
    int iix = SkFloatToIntFloor(x);
    assert_float_equal(reporter, "floor", x, ix, iix);
}

static void test_float_round(skiatest::Reporter* reporter, float x) {
    double xx = x + 0.5;    // need intermediate double to avoid temp loss
    int ix = (int)floor(xx);
    int iix = SkFloatToIntRound(x);
    assert_float_equal(reporter, "round", x, ix, iix);
}

static void test_float_ceil(skiatest::Reporter* reporter, float x) {
    int ix = (int)ceil(x);
    int iix = SkFloatToIntCeil(x);
    assert_float_equal(reporter, "ceil", x, ix, iix);
}

static void test_float_conversions(skiatest::Reporter* reporter, float x) {
    test_float_cast(reporter, x);
    test_float_floor(reporter, x);
    test_float_round(reporter, x);
    test_float_ceil(reporter, x);
}

static void test_int2float(skiatest::Reporter* reporter, int ival) {
    float x0 = (float)ival;
    float x1 = SkIntToFloatCast(ival);
    float x2 = SkIntToFloatCast_NoOverflowCheck(ival);
    REPORTER_ASSERT(reporter, x0 == x1);
    REPORTER_ASSERT(reporter, x0 == x2);
}

static void unittest_fastfloat(skiatest::Reporter* reporter) {
    SkRandom rand;
    size_t i;
    
    static const float gFloats[] = {
        0.f, 1.f, 0.5f, 0.499999f, 0.5000001f, 1.f/3,
        0.000000001f, 1000000000.f,     // doesn't overflow
        0.0000000001f, 10000000000.f    // does overflow
    };
    for (i = 0; i < SK_ARRAY_COUNT(gFloats); i++) {
        //        SkDebugf("---- test floats %g %d\n", gFloats[i], (int)gFloats[i]);
        test_float_conversions(reporter, gFloats[i]);
        test_float_conversions(reporter, -gFloats[i]);
    }
    
    for (int outer = 0; outer < 100; outer++) {
        rand.setSeed(outer);
        for (i = 0; i < 100000; i++) {
            float x = nextFloat(rand);
            test_float_conversions(reporter, x);
        }
        
        test_int2float(reporter, 0);
        test_int2float(reporter, 1);
        test_int2float(reporter, -1);
        for (i = 0; i < 100000; i++) {
            // for now only test ints that are 24bits or less, since we don't
            // round (down) large ints the same as IEEE...
            int ival = rand.nextU() & 0xFFFFFF;
            test_int2float(reporter, ival);
            test_int2float(reporter, -ival);
        }
    }
}

#endif

static void test_muldiv255(skiatest::Reporter* reporter) {
#ifdef SK_CAN_USE_FLOAT
    for (int a = 0; a <= 255; a++) {
        for (int b = 0; b <= 255; b++) {
            int ab = a * b;
            float s = ab / 255.0f;
            int round = (int)floorf(s + 0.5f);
            int trunc = (int)floorf(s);
            
            int iround = SkMulDiv255Round(a, b);
            int itrunc = SkMulDiv255Trunc(a, b);
            
            REPORTER_ASSERT(reporter, iround == round);
            REPORTER_ASSERT(reporter, itrunc == trunc);
            
            REPORTER_ASSERT(reporter, itrunc <= iround);
            REPORTER_ASSERT(reporter, iround <= a);
            REPORTER_ASSERT(reporter, iround <= b);
        }
    }
#endif
}

static void TestMath(skiatest::Reporter* reporter) {    
    int         i;
    int32_t     x;
    SkRandom    rand;
    
    // these should not assert
    SkToS8(127);    SkToS8(-128);       SkToU8(255);
    SkToS16(32767); SkToS16(-32768);    SkToU16(65535);
    SkToS32(2*1024*1024);   SkToS32(-2*1024*1024);  SkToU32(4*1024*1024);
    
    // these should assert
#if 0
    SkToS8(128);
    SkToS8(-129);
    SkToU8(256);
    SkToU8(-5);
    
    SkToS16(32768);
    SkToS16(-32769);
    SkToU16(65536);
    SkToU16(-5);
    
    if (sizeof(size_t) > 4) {
        SkToS32(4*1024*1024);
        SkToS32(-4*1024*1024);
        SkToU32(5*1024*1024);
        SkToU32(-5);
    }
#endif
    
    test_muldiv255(reporter);
    
    {
        SkScalar x = SK_ScalarNaN;
        REPORTER_ASSERT(reporter, SkScalarIsNaN(x));
    }
    
    for (i = 1; i <= 10; i++) {
        x = SkCubeRootBits(i*i*i, 11);
        REPORTER_ASSERT(reporter, x == i);
    }
    
    REPORTER_ASSERT(reporter, !"test the reporter");
    
    x = SkFixedSqrt(SK_Fixed1);
    REPORTER_ASSERT(reporter, x == SK_Fixed1);
    x = SkFixedSqrt(SK_Fixed1/4);
    REPORTER_ASSERT(reporter, x == SK_Fixed1/2);
    x = SkFixedSqrt(SK_Fixed1*4);
    REPORTER_ASSERT(reporter, x == SK_Fixed1*2);
    
    x = SkFractSqrt(SK_Fract1);
    REPORTER_ASSERT(reporter, x == SK_Fract1);
    x = SkFractSqrt(SK_Fract1/4);
    REPORTER_ASSERT(reporter, x == SK_Fract1/2);
    x = SkFractSqrt(SK_Fract1/16);
    REPORTER_ASSERT(reporter, x == SK_Fract1/4);
    
    for (i = 1; i < 100; i++) {
        x = SkFixedSqrt(SK_Fixed1 * i * i);
        REPORTER_ASSERT(reporter, x == SK_Fixed1 * i);
    }
    
    for (i = 0; i < 1000; i++) {
        int value = rand.nextS16();
        int max = rand.nextU16();
        
        int clamp = SkClampMax(value, max);
        int clamp2 = value < 0 ? 0 : (value > max ? max : value);
        REPORTER_ASSERT(reporter, clamp == clamp2);
    }
    
    for (i = 0; i < 100000; i++) {
        SkPoint p;
        
        p.setLength(rand.nextS(), rand.nextS(), SK_Scalar1);
        check_length(reporter, p, SK_Scalar1);
        p.setLength(rand.nextS() >> 13, rand.nextS() >> 13, SK_Scalar1);
        check_length(reporter, p, SK_Scalar1);
    }
    
    {
        SkFixed result = SkFixedDiv(100, 100);
        REPORTER_ASSERT(reporter, result == SK_Fixed1);
        result = SkFixedDiv(1, SK_Fixed1);
        REPORTER_ASSERT(reporter, result == 1);
    }
    
#ifdef SK_CAN_USE_FLOAT
    unittest_fastfloat(reporter);
#endif
    
#ifdef SkLONGLONG
    for (i = 0; i < 100000; i++) {
        SkFixed numer = rand.nextS();
        SkFixed denom = rand.nextS();
        SkFixed result = SkFixedDiv(numer, denom);
        SkLONGLONG check = ((SkLONGLONG)numer << 16) / denom;
        
        (void)SkCLZ(numer);
        (void)SkCLZ(denom);
        
        REPORTER_ASSERT(reporter, result != (SkFixed)SK_NaN32);
        if (check > SK_MaxS32) {
            check = SK_MaxS32;
        } else if (check < -SK_MaxS32) {
            check = SK_MinS32;
        }
        REPORTER_ASSERT(reporter, result == (int32_t)check);
        
        result = SkFractDiv(numer, denom);
        check = ((SkLONGLONG)numer << 30) / denom;
        
        REPORTER_ASSERT(reporter, result != (SkFixed)SK_NaN32);
        if (check > SK_MaxS32) {
            check = SK_MaxS32;
        } else if (check < -SK_MaxS32) {
            check = SK_MinS32;
        }
        REPORTER_ASSERT(reporter, result == (int32_t)check);
        
        // make them <= 2^24, so we don't overflow in fixmul
        numer = numer << 8 >> 8;
        denom = denom << 8 >> 8;
        
        result = SkFixedMul(numer, denom);
        SkFixed r2 = symmetric_fixmul(numer, denom);
        //        SkASSERT(result == r2);
        
        result = SkFixedMul(numer, numer);
        r2 = SkFixedSquare(numer);
        REPORTER_ASSERT(reporter, result == r2);
        
#ifdef SK_CAN_USE_FLOAT
        if (numer >= 0 && denom >= 0) {
            SkFixed mean = SkFixedMean(numer, denom);
            float fm = sk_float_sqrt(sk_float_abs(SkFixedToFloat(numer) * SkFixedToFloat(denom)));
            SkFixed mean2 = SkFloatToFixed(fm);
            int diff = SkAbs32(mean - mean2);
            REPORTER_ASSERT(reporter, diff <= 1);
        }
        
        {
            SkFixed mod = SkFixedMod(numer, denom);
            float n = SkFixedToFloat(numer);
            float d = SkFixedToFloat(denom);
            float m = sk_float_mod(n, d);
            REPORTER_ASSERT(reporter, mod == 0 || (mod < 0) == (m < 0)); // ensure the same sign
            int diff = SkAbs32(mod - SkFloatToFixed(m));
            REPORTER_ASSERT(reporter, (diff >> 7) == 0);
        }
#endif
    }
#endif
    
#ifdef SK_CAN_USE_FLOAT
    for (i = 0; i < 100000; i++) {
        SkFract x = rand.nextU() >> 1;
        double xx = (double)x / SK_Fract1;
        SkFract xr = SkFractSqrt(x);
        SkFract check = SkFloatToFract(sqrt(xx));
        REPORTER_ASSERT(reporter, xr == check || xr == check-1 || xr == check+1);
        
        xr = SkFixedSqrt(x);
        xx = (double)x / SK_Fixed1;
        check = SkFloatToFixed(sqrt(xx));
        REPORTER_ASSERT(reporter, xr == check || xr == check-1);
        
        xr = SkSqrt32(x);
        xx = (double)x;
        check = (int32_t)sqrt(xx);
        REPORTER_ASSERT(reporter, xr == check || xr == check-1);
    }
#endif
    
#if !defined(SK_SCALAR_IS_FLOAT) && defined(SK_CAN_USE_FLOAT)
    {
        SkFixed s, c;
        s = SkFixedSinCos(0, &c);
        REPORTER_ASSERT(reporter, s == 0);
        REPORTER_ASSERT(reporter, c == SK_Fixed1);
    }
    
    int maxDiff = 0;
    for (i = 0; i < 10000; i++) {
        SkFixed rads = rand.nextS() >> 10;
        double frads = SkFixedToFloat(rads);
        
        SkFixed s, c;
        s = SkScalarSinCos(rads, &c);
        
        double fs = sin(frads);
        double fc = cos(frads);
        
        SkFixed is = SkFloatToFixed(fs);
        SkFixed ic = SkFloatToFixed(fc);
        
        maxDiff = SkMax32(maxDiff, SkAbs32(is - s));
        maxDiff = SkMax32(maxDiff, SkAbs32(ic - c));
    }
    SkDebugf("SinCos: maximum error = %d\n", maxDiff);
#endif
}

///////////////////////////////////////////////////////////////////////////////

namespace skiatest {

    class MathTest : public Test {
    public:
        static Test* Factory(void*) {
            return SkNEW(MathTest);
        }

    protected:
        virtual void onGetName(SkString* name) {
            name->set("Math");
        }
        
        virtual void onRun(Reporter* reporter) {
            TestMath(reporter);
        }
    };

    static TestRegistry gReg(MathTest::Factory);
}

