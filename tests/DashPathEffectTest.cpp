#include "Test.h"

#include "SkDashPathEffect.h"
#include "SkWriteBuffer.h"

// crbug.com/348821 was rooted in SkDashPathEffect refusing to flatten and unflatten itself when
// fInitialDashLength < 0 (a signal the effect is nonsense).  Here we test that it flattens.

DEF_TEST(DashPathEffectTest_crbug_348821, r) {
    SkScalar intervals[] = { 1.76934361e+36f, 2.80259693e-45f };  // Values from bug.
    const int count = 2;
    SkScalar phase = SK_ScalarInfinity;  // Used to force the bad fInitialDashLength = -1 path.
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, count, phase));

    // NULL -> refuses to work with flattening framework.
    REPORTER_ASSERT(r, dash->getFactory() != NULL);

    SkWriteBuffer buffer;
    buffer.writeFlattenable(dash);
    REPORTER_ASSERT(r, buffer.bytesWritten() > 12);  // We'd write 12 if broken, >=40 if not.
}
