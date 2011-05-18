#include "Test.h"
#include "SkPath.h"
#include "SkParse.h"
#include "SkSize.h"

static void check_convexity(skiatest::Reporter* reporter, const SkPath& path,
                            SkPath::Convexity expected) {
    SkPath::Convexity c = SkPath::ComputeConvexity(path);
    REPORTER_ASSERT(reporter, c == expected);
}

static void test_convexity2(skiatest::Reporter* reporter) {
    SkPath pt;
    pt.moveTo(0, 0);
    pt.close();
    check_convexity(reporter, pt, SkPath::kConvex_Convexity);
    
    SkPath line;
    line.moveTo(12, 20);
    line.lineTo(-12, -20);
    line.close();
    check_convexity(reporter, pt, SkPath::kConvex_Convexity);
    
    SkPath triLeft;
    triLeft.moveTo(0, 0);
    triLeft.lineTo(1, 0);
    triLeft.lineTo(1, 1);
    triLeft.close();
    check_convexity(reporter, triLeft, SkPath::kConvex_Convexity);
    
    SkPath triRight;
    triRight.moveTo(0, 0);
    triRight.lineTo(-1, 0);
    triRight.lineTo(1, 1);
    triRight.close();
    check_convexity(reporter, triRight, SkPath::kConvex_Convexity);
    
    SkPath square;
    square.moveTo(0, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 1);
    square.lineTo(0, 1);
    square.close();
    check_convexity(reporter, square, SkPath::kConvex_Convexity);
    
    SkPath redundantSquare;
    redundantSquare.moveTo(0, 0);
    redundantSquare.lineTo(0, 0);
    redundantSquare.lineTo(0, 0);
    redundantSquare.lineTo(1, 0);
    redundantSquare.lineTo(1, 0);
    redundantSquare.lineTo(1, 0);
    redundantSquare.lineTo(1, 1);
    redundantSquare.lineTo(1, 1);
    redundantSquare.lineTo(1, 1);
    redundantSquare.lineTo(0, 1);
    redundantSquare.lineTo(0, 1);
    redundantSquare.lineTo(0, 1);
    redundantSquare.close();
    check_convexity(reporter, redundantSquare, SkPath::kConvex_Convexity);
    
    SkPath bowTie;
    bowTie.moveTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.close();
    check_convexity(reporter, bowTie, SkPath::kConcave_Convexity);
    
    SkPath spiral;
    spiral.moveTo(0, 0);
    spiral.lineTo(100, 0);
    spiral.lineTo(100, 100);
    spiral.lineTo(0, 100);
    spiral.lineTo(0, 50);
    spiral.lineTo(50, 50);
    spiral.lineTo(50, 75);
    spiral.close();
    check_convexity(reporter, spiral, SkPath::kConcave_Convexity);
    
    SkPath dent;
    dent.moveTo(SkIntToScalar(0), SkIntToScalar(0));
    dent.lineTo(SkIntToScalar(100), SkIntToScalar(100));
    dent.lineTo(SkIntToScalar(0), SkIntToScalar(100));
    dent.lineTo(SkIntToScalar(-50), SkIntToScalar(200));
    dent.lineTo(SkIntToScalar(-200), SkIntToScalar(100));
    dent.close();
    check_convexity(reporter, dent, SkPath::kConcave_Convexity);
}

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

static void setFromString(SkPath* path, const char str[]) {
    bool first = true;
    while (str) {
        SkScalar x, y;
        str = SkParse::FindScalar(str, &x);
        if (NULL == str) {
            break;
        }
        str = SkParse::FindScalar(str, &y);
        SkASSERT(str);
        if (first) {
            path->moveTo(x, y);
            first = false;
        } else {
            path->lineTo(x, y);
        }
    }
}

static void test_convexity(skiatest::Reporter* reporter) {
    static const SkPath::Convexity C = SkPath::kConcave_Convexity;
    static const SkPath::Convexity V = SkPath::kConvex_Convexity;

    SkPath path;

    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    path.addCircle(0, 0, 10);
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    path.addCircle(0, 0, 10);   // 2nd circle
    REPORTER_ASSERT(reporter, C == SkPath::ComputeConvexity(path));
    path.reset();
    path.addRect(0, 0, 10, 10, SkPath::kCCW_Direction);
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    path.reset();
    path.addRect(0, 0, 10, 10, SkPath::kCW_Direction);
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    
    static const struct {
        const char*         fPathStr;
        SkPath::Convexity   fExpectedConvexity;
    } gRec[] = {
        { "", SkPath::kConvex_Convexity },
        { "0 0", SkPath::kConvex_Convexity },
        { "0 0 10 10", SkPath::kConvex_Convexity },
        { "0 0 10 10 20 20 0 0 10 10", SkPath::kConcave_Convexity },
        { "0 0 10 10 10 20", SkPath::kConvex_Convexity },
        { "0 0 10 10 10 0", SkPath::kConvex_Convexity },
        { "0 0 10 10 10 0 0 10", SkPath::kConcave_Convexity },
        { "0 0 10 0 0 10 -10 -10", SkPath::kConcave_Convexity },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkPath path;
        setFromString(&path, gRec[i].fPathStr);
        SkPath::Convexity c = SkPath::ComputeConvexity(path);
        REPORTER_ASSERT(reporter, c == gRec[i].fExpectedConvexity);
    }
}

void TestPath(skiatest::Reporter* reporter);
void TestPath(skiatest::Reporter* reporter) {
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
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == p2);
    REPORTER_ASSERT(reporter, !(p != p2));

    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());

    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);

    p.addRoundRect(bounds, SK_Scalar1, SK_Scalar1);
    check_convex_bounds(reporter, p, bounds);

    p.reset();
    p.addOval(bounds);
    check_convex_bounds(reporter, p, bounds);

    p.reset();
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

    test_convexity(reporter);
    test_convexity2(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Path", PathTestClass, TestPath)
