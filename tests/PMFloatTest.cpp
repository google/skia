#include "SkPMFloat.h"
#include "Test.h"

DEF_TEST(SkPMFloat, r) {
    SkPMColor c = SkPreMultiplyColor(0xFFCC9933);

    SkPMFloat pmf;
    pmf.set(c);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(1.0f, pmf.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.8f, pmf.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.6f, pmf.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.2f, pmf.b()));

    REPORTER_ASSERT(r, c == pmf.get());

    SkPMFloat unclamped;
    unclamped.setA(+2.0f);
    unclamped.setR(+0.2f);
    unclamped.setG(-0.2f);
    unclamped.setB(-5.0f);

    SkPMFloat clamped;
    clamped.set(unclamped.clamped());

    REPORTER_ASSERT(r, SkScalarNearlyEqual(1.0f, clamped.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.2f, clamped.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.0f, clamped.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(0.0f, clamped.b()));
}
