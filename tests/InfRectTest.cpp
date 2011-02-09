#include "Test.h"
#include "SkRect.h"

#ifdef SK_SCALAR_IS_FLOAT
static float make_zero() {
    return sk_float_sin(0);
}
#endif

static void check_invalid(skiatest::Reporter* reporter,
                          SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect;
    rect.set(l, t, r, b);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());
}

// Tests that hasValidCoordinates() will reject any rect with +/-inf values
// as one of its coordinates.
static void TestInfRect(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    float invalid = 1 / make_zero();    // infinity
#else
    SkFixed invalid = SK_FixedNaN;
#endif
    SkScalar small = SkIntToScalar(10);
    SkScalar big = SkIntToScalar(100);

    SkRect rect = SkRect::MakeXYWH(small, small, big, big);
    REPORTER_ASSERT(reporter, rect.hasValidCoordinates());

    check_invalid(reporter, small, small, big, invalid);
    check_invalid(reporter, small, small, invalid, big);
    check_invalid(reporter, small, invalid, big, big);
    check_invalid(reporter, invalid, small, big, big);
    check_invalid(reporter, small, small, big, -invalid);
    check_invalid(reporter, small, small, -invalid, big);
    check_invalid(reporter, small, -invalid, big, big);
    check_invalid(reporter, -invalid, small, big, big);
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("InfRect", InfRectTestClass, TestInfRect)
