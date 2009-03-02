#include "Test.h"
#include "SkPath.h"

static void TestPath(skiatest::Reporter* reporter) {
    SkPath  p, p2;
    SkRect  bounds, bounds2;
    
    REPORTER_ASSERT(reporter, p.isEmpty());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == p2);
    REPORTER_ASSERT(reporter, !(p != p2));

    // initialize bounds to not-empty
    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);
    p.computeBounds(&bounds, SkPath::kFast_BoundsType);
    REPORTER_ASSERT(reporter, bounds.isEmpty());
    
    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);
    p.addRect(bounds);
    bounds2.setEmpty();
    p.computeBounds(&bounds2, SkPath::kFast_BoundsType);
    REPORTER_ASSERT(reporter, bounds == bounds2);

    REPORTER_ASSERT(reporter, p != p2);
    REPORTER_ASSERT(reporter, !(p == p2));

    // does getPoints return the right result
    REPORTER_ASSERT(reporter, p.getPoints(NULL, 5) == 4);
    SkPoint pts[4];
    int count = p.getPoints(pts, 4);
    REPORTER_ASSERT(reporter, count == 4);
    bounds2.set(pts, 4);
    REPORTER_ASSERT(reporter, bounds == bounds2);
    
    bounds.offset(SK_Scalar1*3, SK_Scalar1*4);
    p.offset(SK_Scalar1*3, SK_Scalar1*4);
    p.computeBounds(&bounds2, SkPath::kFast_BoundsType);
    REPORTER_ASSERT(reporter, bounds == bounds2);

#if 0 // isRect needs to be implemented
    REPORTER_ASSERT(reporter, p.isRect(NULL));
    bounds.setEmpty();
    REPORTER_ASSERT(reporter, p.isRect(&bounds2));
    REPORTER_ASSERT(reporter, bounds == bounds2);
    
    // now force p to not be a rect
    bounds.set(0, 0, SK_Scalar1/2, SK_Scalar1/2);
    p.addRect(bounds);
    REPORTER_ASSERT(reporter, !p.isRect(NULL));
#endif

    SkPoint pt;

    p.moveTo(SK_Scalar1, 0);
    p.getLastPt(&pt);
    REPORTER_ASSERT(reporter, pt.fX == SK_Scalar1);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Path", PathTestClass, TestPath)
