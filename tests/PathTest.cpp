#include "Test.h"
#include "SkPath.h"
#include "SkSize.h"

static void check_convex_bounds(skiatest::Reporter* reporter, const SkPath& p,
                                const SkRect& bounds) {
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, p.getBounds() == bounds);
    
    SkPath p2(p);
    REPORTER_ASSERT(reporter, p2.isConvex());
    REPORTER_ASSERT(reporter, p2.getBounds() == bounds);

    SkPath other;
    other.swap(p2);
    REPORTER_ASSERT(reporter, other.isConvex());
    REPORTER_ASSERT(reporter, other.getBounds() == bounds);
}

static void TestPath(skiatest::Reporter* reporter) {
    {
        SkSize size;
        size.fWidth = 3.4f;
        size.width();
        size = SkSize::Make(3,4);
        SkISize isize = SkISize::Make(3,4);
    }

    SkTSize<SkScalar>::Make(3,4);

    SkPath  p, p2;
    SkRect  bounds, bounds2;

    REPORTER_ASSERT(reporter, p.isEmpty());
    REPORTER_ASSERT(reporter, !p.isConvex());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == p2);
    REPORTER_ASSERT(reporter, !(p != p2));

    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());

    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);

    p.setIsConvex(false);
    p.addRoundRect(bounds, SK_Scalar1, SK_Scalar1);
    check_convex_bounds(reporter, p, bounds);
    
    p.reset();
    p.setIsConvex(false);
    p.addOval(bounds);
    check_convex_bounds(reporter, p, bounds);
    
    p.reset();
    p.setIsConvex(false);
    p.addRect(bounds);
    check_convex_bounds(reporter, p, bounds);

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
    REPORTER_ASSERT(reporter, bounds == p.getBounds());

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
