#include "Test.h"
#include "SkPathMeasure.h"

static void TestPathMeasure(skiatest::Reporter* reporter) {
    SkPath  path;

    path.moveTo(0, 0);
    path.lineTo(SK_Scalar1, 0);
    path.lineTo(SK_Scalar1, SK_Scalar1);
    path.lineTo(0, SK_Scalar1);

    SkPathMeasure   meas(path, true);
    SkScalar        length = meas.getLength();
    SkASSERT(length == SK_Scalar1*4);

    path.reset();
    path.moveTo(0, 0);
    path.lineTo(SK_Scalar1*3, SK_Scalar1*4);
    meas.setPath(&path, false);
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1*5);

    path.reset();
    path.addCircle(0, 0, SK_Scalar1);
    meas.setPath(&path, true);
    length = meas.getLength();
//    SkDebugf("circle arc-length = %g\n", length);

    for (int i = 0; i < 8; i++) {
        SkScalar    d = length * i / 8;
        SkPoint     p;
        SkVector    v;
        meas.getPosTan(d, &p, &v);
#if 0
        SkDebugf("circle arc-length=%g, pos[%g %g] tan[%g %g]\n",
                 d, p.fX, p.fY, v.fX, v.fY);
#endif
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathMeasure", PathMeasureTestClass, TestPathMeasure)
