
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkParse.h"
#include "SkParsePath.h"
#include "SkPathEffect.h"
#include "SkRandom.h"
#include "SkReader32.h"
#include "SkSize.h"
#include "SkWriter32.h"

// assert that we always
//  start with a moveTo
//  only have 1 moveTo
//  only have Lines after that
//  end with a single close
//  only have (at most) 1 close
//
static void test_poly(skiatest::Reporter* reporter, const SkPath& path,
                      const SkPoint srcPts[], int count, bool expectClose) {
    SkPath::RawIter iter(path);
    SkPoint         pts[4];

    bool firstTime = true;
    bool foundClose = false;
    for (;;) {
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                REPORTER_ASSERT(reporter, firstTime);
                REPORTER_ASSERT(reporter, pts[0] == srcPts[0]);
                srcPts++;
                firstTime = false;
                break;
            case SkPath::kLine_Verb:
                REPORTER_ASSERT(reporter, !firstTime);
                REPORTER_ASSERT(reporter, pts[1] == srcPts[0]);
                srcPts++;
                break;
            case SkPath::kQuad_Verb:
                REPORTER_ASSERT(reporter, !"unexpected quad verb");
                break;
            case SkPath::kCubic_Verb:
                REPORTER_ASSERT(reporter, !"unexpected cubic verb");
                break;
            case SkPath::kClose_Verb:
                REPORTER_ASSERT(reporter, !firstTime);
                REPORTER_ASSERT(reporter, !foundClose);
                REPORTER_ASSERT(reporter, expectClose);
                foundClose = true;
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    REPORTER_ASSERT(reporter, foundClose == expectClose);
}

static void test_addPoly(skiatest::Reporter* reporter) {
    SkPoint pts[32];
    SkRandom rand;
    
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        pts[i].fX = rand.nextSScalar1();
        pts[i].fY = rand.nextSScalar1();
    }

    for (int doClose = 0; doClose <= 1; ++doClose) {
        for (size_t count = 1; count <= SK_ARRAY_COUNT(pts); ++count) {
            SkPath path;
            path.addPoly(pts, count, SkToBool(doClose));
            test_poly(reporter, path, pts, count, SkToBool(doClose));
        }
    }
}

static void test_strokerec(skiatest::Reporter* reporter) {
    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    REPORTER_ASSERT(reporter, rec.isFillStyle());
    
    rec.setHairlineStyle();
    REPORTER_ASSERT(reporter, rec.isHairlineStyle());
    
    rec.setStrokeStyle(SK_Scalar1, false);
    REPORTER_ASSERT(reporter, SkStrokeRec::kStroke_Style == rec.getStyle());
    
    rec.setStrokeStyle(SK_Scalar1, true);
    REPORTER_ASSERT(reporter, SkStrokeRec::kStrokeAndFill_Style == rec.getStyle());
    
    rec.setStrokeStyle(0, false);
    REPORTER_ASSERT(reporter, SkStrokeRec::kHairline_Style == rec.getStyle());
    
    rec.setStrokeStyle(0, true);
    REPORTER_ASSERT(reporter, SkStrokeRec::kFill_Style == rec.getStyle());
}

/**
 * cheapIsDirection can take a shortcut when a path is marked convex.
 * This function ensures that we always test cheapIsDirection when the path
 * is flagged with unknown convexity status.
 */
static void check_direction(SkPath* path,
                            SkPath::Direction expectedDir,
                            skiatest::Reporter* reporter) {
    if (SkPath::kConvex_Convexity == path->getConvexity()) {
        REPORTER_ASSERT(reporter, path->cheapIsDirection(expectedDir));
        path->setConvexity(SkPath::kUnknown_Convexity);
    }
    REPORTER_ASSERT(reporter, path->cheapIsDirection(expectedDir));
}

static void test_direction(skiatest::Reporter* reporter) {
    size_t i;
    SkPath path;
    REPORTER_ASSERT(reporter, !path.cheapComputeDirection(NULL));
    REPORTER_ASSERT(reporter, !path.cheapIsDirection(SkPath::kCW_Direction));
    REPORTER_ASSERT(reporter, !path.cheapIsDirection(SkPath::kCCW_Direction));

    static const char* gDegen[] = {
        "M 10 10",
        "M 10 10 M 20 20",
        "M 10 10 L 20 20",
        "M 10 10 L 10 10 L 10 10",
        "M 10 10 Q 10 10 10 10",
        "M 10 10 C 10 10 10 10 10 10",
    };
    for (i = 0; i < SK_ARRAY_COUNT(gDegen); ++i) {
        path.reset();
        bool valid = SkParsePath::FromSVGString(gDegen[i], &path);
        REPORTER_ASSERT(reporter, valid);
        REPORTER_ASSERT(reporter, !path.cheapComputeDirection(NULL));
    }
    
    static const char* gCW[] = {
        "M 10 10 L 10 10 Q 20 10 20 20",
        "M 10 10 C 20 10 20 20 20 20",
        "M 20 10 Q 20 20 30 20 L 10 20", // test double-back at y-max
    };
    for (i = 0; i < SK_ARRAY_COUNT(gCW); ++i) {
        path.reset();
        bool valid = SkParsePath::FromSVGString(gCW[i], &path);
        REPORTER_ASSERT(reporter, valid);
        check_direction(&path, SkPath::kCW_Direction, reporter);
    }
    
    static const char* gCCW[] = {
        "M 10 10 L 10 10 Q 20 10 20 -20",
        "M 10 10 C 20 10 20 -20 20 -20",
        "M 20 10 Q 20 20 10 20 L 30 20", // test double-back at y-max
    };
    for (i = 0; i < SK_ARRAY_COUNT(gCCW); ++i) {
        path.reset();
        bool valid = SkParsePath::FromSVGString(gCCW[i], &path);
        REPORTER_ASSERT(reporter, valid);
        check_direction(&path, SkPath::kCCW_Direction, reporter);
    }

    // Test two donuts, each wound a different direction. Only the outer contour
    // determines the cheap direction
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(2), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(1), SkPath::kCCW_Direction);
    check_direction(&path, SkPath::kCW_Direction, reporter);

    path.reset();
    path.addCircle(0, 0, SkIntToScalar(1), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(2), SkPath::kCCW_Direction);
    check_direction(&path, SkPath::kCCW_Direction, reporter);

#ifdef SK_SCALAR_IS_FLOAT
    // triangle with one point really far from the origin.
    path.reset();
    // the first point is roughly 1.05e10, 1.05e10
    path.moveTo(SkFloatToScalar(SkBits2Float(0x501c7652)), SkFloatToScalar(SkBits2Float(0x501c7652)));
    path.lineTo(110 * SK_Scalar1, -10 * SK_Scalar1);
    path.lineTo(-10 * SK_Scalar1, 60 * SK_Scalar1);
    check_direction(&path, SkPath::kCCW_Direction, reporter);
#endif
}

static void add_rect(SkPath* path, const SkRect& r) {
    path->moveTo(r.fLeft, r.fTop);
    path->lineTo(r.fRight, r.fTop);
    path->lineTo(r.fRight, r.fBottom);
    path->lineTo(r.fLeft, r.fBottom);
    path->close();
}

static void test_bounds(skiatest::Reporter* reporter) {
    static const SkRect rects[] = {
        { SkIntToScalar(10), SkIntToScalar(160), SkIntToScalar(610), SkIntToScalar(160) },
        { SkIntToScalar(610), SkIntToScalar(160), SkIntToScalar(610), SkIntToScalar(199) },
        { SkIntToScalar(10), SkIntToScalar(198), SkIntToScalar(610), SkIntToScalar(199) },
        { SkIntToScalar(10), SkIntToScalar(160), SkIntToScalar(10), SkIntToScalar(199) },
    };

    SkPath path0, path1;
    for (size_t i = 0; i < SK_ARRAY_COUNT(rects); ++i) {
        path0.addRect(rects[i]);
        add_rect(&path1, rects[i]);
    }

    REPORTER_ASSERT(reporter, path0.getBounds() == path1.getBounds());
}

static void stroke_cubic(const SkPoint pts[4]) {
    SkPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[1], pts[2], pts[3]);
    
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SK_Scalar1 * 2);
    
    SkPath fill;
    paint.getFillPath(path, &fill);
}

// just ensure this can run w/o any SkASSERTS firing in the debug build
// we used to assert due to differences in how we determine a degenerate vector
// but that was fixed with the introduction of SkPoint::CanNormalize
static void stroke_tiny_cubic() {
    SkPoint p0[] = {
        { 372.0f,   92.0f },
        { 372.0f,   92.0f },
        { 372.0f,   92.0f },
        { 372.0f,   92.0f },
    };
    
    stroke_cubic(p0);
    
    SkPoint p1[] = {
        { 372.0f,       92.0f },
        { 372.0007f,    92.000755f },
        { 371.99927f,   92.003922f },
        { 371.99826f,   92.003899f },
    };
    
    stroke_cubic(p1);
}

static void check_close(skiatest::Reporter* reporter, const SkPath& path) {
    for (int i = 0; i < 2; ++i) {
        SkPath::Iter iter(path, SkToBool(i));
        SkPoint mv;
        SkPoint pts[4];
        SkPath::Verb v;
        int nMT = 0;
        int nCL = 0;
        mv.set(0, 0);
        while (SkPath::kDone_Verb != (v = iter.next(pts))) {
            switch (v) {
                case SkPath::kMove_Verb:
                    mv = pts[0];
                    ++nMT;
                    break;
                case SkPath::kClose_Verb:
                    REPORTER_ASSERT(reporter, mv == pts[0]);
                    ++nCL;
                    break;
                default:
                    break;
            }
        }
        // if we force a close on the interator we should have a close
        // for every moveTo
        REPORTER_ASSERT(reporter, !i || nMT == nCL);
    }
}

static void test_close(skiatest::Reporter* reporter) {
    SkPath closePt;
    closePt.moveTo(0, 0);
    closePt.close();
    check_close(reporter, closePt);

    SkPath openPt;
    openPt.moveTo(0, 0);
    check_close(reporter, openPt);

    SkPath empty;
    check_close(reporter, empty);
    empty.close();
    check_close(reporter, empty);

    SkPath rect;
    rect.addRect(SK_Scalar1, SK_Scalar1, 10 * SK_Scalar1, 10*SK_Scalar1);
    check_close(reporter, rect);
    rect.close();
    check_close(reporter, rect);

    SkPath quad;
    quad.quadTo(SK_Scalar1, SK_Scalar1, 10 * SK_Scalar1, 10*SK_Scalar1);
    check_close(reporter, quad);
    quad.close();
    check_close(reporter, quad);

    SkPath cubic;
    quad.cubicTo(SK_Scalar1, SK_Scalar1, 10 * SK_Scalar1, 
                 10*SK_Scalar1, 20 * SK_Scalar1, 20*SK_Scalar1);
    check_close(reporter, cubic);
    cubic.close();
    check_close(reporter, cubic);

    SkPath line;
    line.moveTo(SK_Scalar1, SK_Scalar1);
    line.lineTo(10 * SK_Scalar1, 10*SK_Scalar1);
    check_close(reporter, line);
    line.close();
    check_close(reporter, line);

    SkPath rect2;
    rect2.addRect(SK_Scalar1, SK_Scalar1, 10 * SK_Scalar1, 10*SK_Scalar1);
    rect2.close();
    rect2.addRect(SK_Scalar1, SK_Scalar1, 10 * SK_Scalar1, 10*SK_Scalar1);
    check_close(reporter, rect2);
    rect2.close();
    check_close(reporter, rect2);

    SkPath oval3;
    oval3.addOval(SkRect::MakeWH(SK_Scalar1*100,SK_Scalar1*100));
    oval3.close();
    oval3.addOval(SkRect::MakeWH(SK_Scalar1*200,SK_Scalar1*200));
    check_close(reporter, oval3);
    oval3.close();
    check_close(reporter, oval3);

    SkPath moves;
    moves.moveTo(SK_Scalar1, SK_Scalar1);
    moves.moveTo(5 * SK_Scalar1, SK_Scalar1);
    moves.moveTo(SK_Scalar1, 10 * SK_Scalar1);
    moves.moveTo(10 *SK_Scalar1, SK_Scalar1);
    check_close(reporter, moves);

    stroke_tiny_cubic();
}

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
    line.moveTo(12*SK_Scalar1, 20*SK_Scalar1);
    line.lineTo(-12*SK_Scalar1, -20*SK_Scalar1);
    line.close();
    check_convexity(reporter, pt, SkPath::kConvex_Convexity);
    
    SkPath triLeft;
    triLeft.moveTo(0, 0);
    triLeft.lineTo(SK_Scalar1, 0);
    triLeft.lineTo(SK_Scalar1, SK_Scalar1);
    triLeft.close();
    check_convexity(reporter, triLeft, SkPath::kConvex_Convexity);
    
    SkPath triRight;
    triRight.moveTo(0, 0);
    triRight.lineTo(-SK_Scalar1, 0);
    triRight.lineTo(SK_Scalar1, SK_Scalar1);
    triRight.close();
    check_convexity(reporter, triRight, SkPath::kConvex_Convexity);
    
    SkPath square;
    square.moveTo(0, 0);
    square.lineTo(SK_Scalar1, 0);
    square.lineTo(SK_Scalar1, SK_Scalar1);
    square.lineTo(0, SK_Scalar1);
    square.close();
    check_convexity(reporter, square, SkPath::kConvex_Convexity);
    
    SkPath redundantSquare;
    redundantSquare.moveTo(0, 0);
    redundantSquare.lineTo(0, 0);
    redundantSquare.lineTo(0, 0);
    redundantSquare.lineTo(SK_Scalar1, 0);
    redundantSquare.lineTo(SK_Scalar1, 0);
    redundantSquare.lineTo(SK_Scalar1, 0);
    redundantSquare.lineTo(SK_Scalar1, SK_Scalar1);
    redundantSquare.lineTo(SK_Scalar1, SK_Scalar1);
    redundantSquare.lineTo(SK_Scalar1, SK_Scalar1);
    redundantSquare.lineTo(0, SK_Scalar1);
    redundantSquare.lineTo(0, SK_Scalar1);
    redundantSquare.lineTo(0, SK_Scalar1);
    redundantSquare.close();
    check_convexity(reporter, redundantSquare, SkPath::kConvex_Convexity);
    
    SkPath bowTie;
    bowTie.moveTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(SK_Scalar1, SK_Scalar1);
    bowTie.lineTo(SK_Scalar1, SK_Scalar1);
    bowTie.lineTo(SK_Scalar1, SK_Scalar1);
    bowTie.lineTo(SK_Scalar1, 0);
    bowTie.lineTo(SK_Scalar1, 0);
    bowTie.lineTo(SK_Scalar1, 0);
    bowTie.lineTo(0, SK_Scalar1);
    bowTie.lineTo(0, SK_Scalar1);
    bowTie.lineTo(0, SK_Scalar1);
    bowTie.close();
    check_convexity(reporter, bowTie, SkPath::kConcave_Convexity);
    
    SkPath spiral;
    spiral.moveTo(0, 0);
    spiral.lineTo(100*SK_Scalar1, 0);
    spiral.lineTo(100*SK_Scalar1, 100*SK_Scalar1);
    spiral.lineTo(0, 100*SK_Scalar1);
    spiral.lineTo(0, 50*SK_Scalar1);
    spiral.lineTo(50*SK_Scalar1, 50*SK_Scalar1);
    spiral.lineTo(50*SK_Scalar1, 75*SK_Scalar1);
    spiral.close();
    check_convexity(reporter, spiral, SkPath::kConcave_Convexity);
    
    SkPath dent;
    dent.moveTo(0, 0);
    dent.lineTo(100*SK_Scalar1, 100*SK_Scalar1);
    dent.lineTo(0, 100*SK_Scalar1);
    dent.lineTo(-50*SK_Scalar1, 200*SK_Scalar1);
    dent.lineTo(-200*SK_Scalar1, 100*SK_Scalar1);
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
    path.addCircle(0, 0, SkIntToScalar(10));
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    path.addCircle(0, 0, SkIntToScalar(10));   // 2nd circle
    REPORTER_ASSERT(reporter, C == SkPath::ComputeConvexity(path));
    path.reset();
    path.addRect(0, 0, SkIntToScalar(10), SkIntToScalar(10), SkPath::kCCW_Direction);
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    REPORTER_ASSERT(reporter, path.cheapIsDirection(SkPath::kCCW_Direction));
    path.reset();
    path.addRect(0, 0, SkIntToScalar(10), SkIntToScalar(10), SkPath::kCW_Direction);
    REPORTER_ASSERT(reporter, V == SkPath::ComputeConvexity(path));
    REPORTER_ASSERT(reporter, path.cheapIsDirection(SkPath::kCW_Direction));
    
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

static void test_isLine(skiatest::Reporter* reporter) {
    SkPath path;
    SkPoint pts[2];
    const SkScalar value = SkIntToScalar(5);

    REPORTER_ASSERT(reporter, !path.isLine(NULL));
    
    // set some non-zero values
    pts[0].set(value, value);
    pts[1].set(value, value);
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    // check that pts was untouched
    REPORTER_ASSERT(reporter, pts[0].equals(value, value));
    REPORTER_ASSERT(reporter, pts[1].equals(value, value));

    const SkScalar moveX = SkIntToScalar(1);
    const SkScalar moveY = SkIntToScalar(2);
    SkASSERT(value != moveX && value != moveY);

    path.moveTo(moveX, moveY);
    REPORTER_ASSERT(reporter, !path.isLine(NULL));
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    // check that pts was untouched
    REPORTER_ASSERT(reporter, pts[0].equals(value, value));
    REPORTER_ASSERT(reporter, pts[1].equals(value, value));

    const SkScalar lineX = SkIntToScalar(2);
    const SkScalar lineY = SkIntToScalar(2);
    SkASSERT(value != lineX && value != lineY);

    path.lineTo(lineX, lineY);
    REPORTER_ASSERT(reporter, path.isLine(NULL));

    REPORTER_ASSERT(reporter, !pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, !pts[1].equals(lineX, lineY));
    REPORTER_ASSERT(reporter, path.isLine(pts));
    REPORTER_ASSERT(reporter, pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, pts[1].equals(lineX, lineY));

    path.lineTo(0, 0);  // too many points/verbs
    REPORTER_ASSERT(reporter, !path.isLine(NULL));
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    REPORTER_ASSERT(reporter, pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, pts[1].equals(lineX, lineY));
}

// Simple isRect test is inline TestPath, below.
// test_isRect provides more extensive testing.
static void test_isRect(skiatest::Reporter* reporter) {
    // passing tests (all moveTo / lineTo...
    SkPoint r1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    SkPoint r2[] = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
    SkPoint r3[] = {{1, 1}, {0, 1}, {0, 0}, {1, 0}};
    SkPoint r4[] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
    SkPoint r5[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    SkPoint r6[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    SkPoint r7[] = {{1, 1}, {1, 0}, {0, 0}, {0, 1}};
    SkPoint r8[] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};
    SkPoint r9[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    SkPoint ra[] = {{0, 0}, {0, .5f}, {0, 1}, {.5f, 1}, {1, 1}, {1, .5f},
        {1, 0}, {.5f, 0}};
    SkPoint rb[] = {{0, 0}, {.5f, 0}, {1, 0}, {1, .5f}, {1, 1}, {.5f, 1},
        {0, 1}, {0, .5f}};
    SkPoint rc[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}};
    SkPoint rd[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}};
    SkPoint re[] = {{0, 0}, {1, 0}, {1, 0}, {1, 1}, {0, 1}};
    
    // failing tests
    SkPoint f1[] = {{0, 0}, {1, 0}, {1, 1}}; // too few points
    SkPoint f2[] = {{0, 0}, {1, 1}, {0, 1}, {1, 0}}; // diagonal
    SkPoint f3[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}}; // wraps
    SkPoint f4[] = {{0, 0}, {1, 0}, {0, 0}, {1, 0}, {1, 1}, {0, 1}}; // backs up
    SkPoint f5[] = {{0, 0}, {1, 0}, {1, 1}, {2, 0}}; // end overshoots
    SkPoint f6[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 2}}; // end overshoots
    SkPoint f7[] = {{0, 0}, {1, 0}, {1, 1}, {0, 2}}; // end overshoots
    SkPoint f8[] = {{0, 0}, {1, 0}, {1, 1}, {1, 0}}; // 'L'
    
    // failing, no close
    SkPoint c1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}}; // close doesn't match
    SkPoint c2[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}}; // ditto

    size_t testLen[] = {
        sizeof(r1), sizeof(r2), sizeof(r3), sizeof(r4), sizeof(r5), sizeof(r6),
        sizeof(r7), sizeof(r8), sizeof(r9), sizeof(ra), sizeof(rb), sizeof(rc),
        sizeof(rd), sizeof(re),
        sizeof(f1), sizeof(f2), sizeof(f3), sizeof(f4), sizeof(f5), sizeof(f6),
        sizeof(f7), sizeof(f8),
        sizeof(c1), sizeof(c2) 
    };
    SkPoint* tests[] = {
        r1, r2, r3, r4, r5, r6, r7, r8, r9, ra, rb, rc, rd, re,
        f1, f2, f3, f4, f5, f6, f7, f8,
        c1, c2 
    };
    SkPoint* lastPass = re;
    SkPoint* lastClose = f8;
    bool fail = false;
    bool close = true;
    const size_t testCount = sizeof(tests) / sizeof(tests[0]);
    size_t index;
    for (size_t testIndex = 0; testIndex < testCount; ++testIndex) {
        SkPath path;
        path.moveTo(tests[testIndex][0].fX, tests[testIndex][0].fY);
        for (index = 1; index < testLen[testIndex] / sizeof(SkPoint); ++index) {
            path.lineTo(tests[testIndex][index].fX, tests[testIndex][index].fY);
        }
        if (close) {
            path.close();
        }
        REPORTER_ASSERT(reporter, fail ^ path.isRect(0));
        if (tests[testIndex] == lastPass) {
            fail = true;
        }
        if (tests[testIndex] == lastClose) {
            close = false;
        }
    }
    
    // fail, close then line
    SkPath path1;
    path1.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
        path1.lineTo(r1[index].fX, r1[index].fY);
    }
    path1.close();
    path1.lineTo(1, 0);
    REPORTER_ASSERT(reporter, fail ^ path1.isRect(0));
    
    // fail, move in the middle
    path1.reset();
    path1.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
        if (index == 2) {
            path1.moveTo(1, .5f);
        }
        path1.lineTo(r1[index].fX, r1[index].fY);
    }
    path1.close();
    REPORTER_ASSERT(reporter, fail ^ path1.isRect(0));

    // fail, move on the edge
    path1.reset();
    for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
        path1.moveTo(r1[index - 1].fX, r1[index - 1].fY);
        path1.lineTo(r1[index].fX, r1[index].fY);
    }
    path1.close();
    REPORTER_ASSERT(reporter, fail ^ path1.isRect(0));
    
    // fail, quad
    path1.reset();
    path1.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
        if (index == 2) {
            path1.quadTo(1, .5f, 1, .5f);
        }
        path1.lineTo(r1[index].fX, r1[index].fY);
    }
    path1.close();
    REPORTER_ASSERT(reporter, fail ^ path1.isRect(0));
    
    // fail, cubic
    path1.reset();
    path1.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
        if (index == 2) {
            path1.cubicTo(1, .5f, 1, .5f, 1, .5f);
        }
        path1.lineTo(r1[index].fX, r1[index].fY);
    }
    path1.close();
    REPORTER_ASSERT(reporter, fail ^ path1.isRect(0));
}

static void test_flattening(skiatest::Reporter* reporter) {
    SkPath p;

    static const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), SkIntToScalar(10) },
        { SkIntToScalar(20), SkIntToScalar(10) }, { SkIntToScalar(20), 0 },
        { 0, 0 }, { 0, SkIntToScalar(10) }, { SkIntToScalar(1), SkIntToScalar(10) }
    };
    p.moveTo(pts[0]);
    p.lineTo(pts[1]);
    p.quadTo(pts[2], pts[3]);
    p.cubicTo(pts[4], pts[5], pts[6]);

    SkWriter32 writer(100);
    p.flatten(writer);
    size_t size = writer.size();
    SkAutoMalloc storage(size);
    writer.flatten(storage.get());
    SkReader32 reader(storage.get(), size);

    SkPath p1;
    REPORTER_ASSERT(reporter, p1 != p);
    p1.unflatten(reader);
    REPORTER_ASSERT(reporter, p1 == p);
}

static void test_transform(skiatest::Reporter* reporter) {
    SkPath p, p1;
    
    static const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), SkIntToScalar(10) },
        { SkIntToScalar(20), SkIntToScalar(10) }, { SkIntToScalar(20), 0 },
        { 0, 0 }, { 0, SkIntToScalar(10) }, { SkIntToScalar(1), SkIntToScalar(10) }
    };
    p.moveTo(pts[0]);
    p.lineTo(pts[1]);
    p.quadTo(pts[2], pts[3]);
    p.cubicTo(pts[4], pts[5], pts[6]);
    
    SkMatrix matrix;
    matrix.reset();
    p.transform(matrix, &p1);
    REPORTER_ASSERT(reporter, p == p1);
    
    matrix.setScale(SK_Scalar1 * 2, SK_Scalar1 * 3);
    p.transform(matrix, &p1);
    SkPoint pts1[7];
    int count = p1.getPoints(pts1, 7);
    REPORTER_ASSERT(reporter, 7 == count);
    for (int i = 0; i < count; ++i) {
        SkPoint newPt = SkPoint::Make(pts[i].fX * 2, pts[i].fY * 3);
        REPORTER_ASSERT(reporter, newPt == pts1[i]);
    }
}

static void test_zero_length_paths(skiatest::Reporter* reporter) {
    SkPath  p;
    SkPoint pt;
    SkRect  bounds;

    // Lone moveTo case
    p.moveTo(SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 1 == p.countPoints());
    p.getLastPt(&pt);
    REPORTER_ASSERT(reporter, pt.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, pt.fY == SK_Scalar1);
    bounds.set(0, 0, 0, 0);
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[1];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    }

    // MoveTo-MoveTo case
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 2 == p.countPoints());
    REPORTER_ASSERT(reporter, 2 == p.countVerbs());
    p.getLastPt(&pt);
    REPORTER_ASSERT(reporter, pt.fX == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, pt.fY == SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, bounds == p.getBounds()); 
    {
    uint8_t verbs[2];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[1]);
    }

    // moveTo-close case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.close();
    bounds.set(0, 0, 0, 0);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 1 == p.countPoints());
    REPORTER_ASSERT(reporter, 2 == p.countVerbs());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[2];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[1]);
    }

    // moveTo-close-moveTo-close case
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.close();
    bounds.set(SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 2 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[4];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[1]);
    }

    // moveTo-line case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.lineTo(SK_Scalar1, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 2 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[2];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[1]);
    }

    // moveTo-lineTo-moveTo-lineTo case
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.lineTo(SK_Scalar1*2, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1*2, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 4 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[4];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[2]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[3]);
    }

    // moveTo-line-close case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.lineTo(SK_Scalar1, SK_Scalar1);
    p.close();
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 2 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[3];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[2]);
    }

    // moveTo-line-close-moveTo-line-close case
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.lineTo(SK_Scalar1*2, SK_Scalar1);
    p.close();
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1*2, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 4 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[6];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[2]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[3]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[4]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[5]);
    }

    // moveTo-quadTo case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.quadTo(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 3 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[2];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kQuad_Verb == verbs[1]);
    }

    // moveTo-quadTo-close case
    p.close();
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 3 == p.countPoints());
    REPORTER_ASSERT(reporter, 3 == p.countVerbs());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[3];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kQuad_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[2]);
    }

    // moveTo-quadTo-moveTo-quadTo case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.quadTo(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.quadTo(SK_Scalar1*2, SK_Scalar1, SK_Scalar1*2, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1*2, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 6 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[4];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kQuad_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kQuad_Verb == verbs[1]);
    }

    // moveTo-cubicTo case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.cubicTo(SK_Scalar1, SK_Scalar1,
              SK_Scalar1, SK_Scalar1,
              SK_Scalar1, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 4 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[2];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb  == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb == verbs[1]);
    }

    // moveTo-cubicTo-close case
    p.close();
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 4 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[3];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb   == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb  == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb  == verbs[2]);
    }

    // moveTo-cubicTo-moveTo-cubicTo case
    p.reset();
    p.moveTo(SK_Scalar1, SK_Scalar1);
    p.cubicTo(SK_Scalar1, SK_Scalar1,
              SK_Scalar1, SK_Scalar1,
              SK_Scalar1, SK_Scalar1);
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.cubicTo(SK_Scalar1*2, SK_Scalar1,
              SK_Scalar1*2, SK_Scalar1,
              SK_Scalar1*2, SK_Scalar1);
    bounds.set(SK_Scalar1, SK_Scalar1, SK_Scalar1*2, SK_Scalar1);
    REPORTER_ASSERT(reporter, !p.isEmpty());
    REPORTER_ASSERT(reporter, 8 == p.countPoints());
    REPORTER_ASSERT(reporter, bounds == p.getBounds());
    {
    uint8_t verbs[4];
    REPORTER_ASSERT(reporter, SK_ARRAY_COUNT(verbs) == p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb  == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb  == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[2]);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb == verbs[3]);
    }
}

struct SegmentInfo {
    SkPath fPath;
    int    fPointCount;
};

#define kCurveSegmentMask   (SkPath::kQuad_SegmentMask | SkPath::kCubic_SegmentMask)

static void test_segment_masks(skiatest::Reporter* reporter) {
    SkPath p;
    p.moveTo(0, 0);
    p.quadTo(100, 100, 200, 200);
    REPORTER_ASSERT(reporter, SkPath::kQuad_SegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());
    p.cubicTo(100, 100, 200, 200, 300, 300);
    REPORTER_ASSERT(reporter, kCurveSegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());
    p.reset();
    p.moveTo(0, 0);
    p.cubicTo(100, 100, 200, 200, 300, 300);
    REPORTER_ASSERT(reporter, SkPath::kCubic_SegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());
}

static void test_iter(skiatest::Reporter* reporter) {
    SkPath p;
    SkPoint pts[4];

    // Test an iterator with no path
    SkPath::Iter noPathIter;
    REPORTER_ASSERT(reporter, noPathIter.next(pts) == SkPath::kDone_Verb);
    // Test that setting an empty path works
    noPathIter.setPath(p, false);
    REPORTER_ASSERT(reporter, noPathIter.next(pts) == SkPath::kDone_Verb);
    // Test that close path makes no difference for an empty path
    noPathIter.setPath(p, true);
    REPORTER_ASSERT(reporter, noPathIter.next(pts) == SkPath::kDone_Verb);
    
    // Test an iterator with an initial empty path
    SkPath::Iter iter(p, false);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Test that close path makes no difference
    SkPath::Iter forceCloseIter(p, true);
    REPORTER_ASSERT(reporter, forceCloseIter.next(pts) == SkPath::kDone_Verb);

    // Test that a move-only path produces nothing when iterated.
    p.moveTo(SK_Scalar1, 0);
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // No matter how many moves we add, we should still get nothing back.
    p.moveTo(SK_Scalar1*2, 0);
    p.moveTo(SK_Scalar1*3, 0);
    p.moveTo(SK_Scalar1*4, 0);
    p.moveTo(SK_Scalar1*5, 0);
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Nor should force closing
    forceCloseIter.setPath(p, true);
    REPORTER_ASSERT(reporter, forceCloseIter.next(pts) == SkPath::kDone_Verb);

    // Initial closes should be ignored
    p.reset();
    p.close();
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);
    // Even if force closed
    forceCloseIter.setPath(p, true);
    REPORTER_ASSERT(reporter, forceCloseIter.next(pts) == SkPath::kDone_Verb);

    // Move/close sequences should also be ignored
    p.reset();
    p.close();
    p.moveTo(SK_Scalar1, 0);
    p.close();
    p.close();
    p.moveTo(SK_Scalar1*2, 0);
    p.close();
    p.moveTo(SK_Scalar1*3, 0);
    p.moveTo(SK_Scalar1*4, 0);
    p.close();
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);
    // Even if force closed
    forceCloseIter.setPath(p, true);
    REPORTER_ASSERT(reporter, forceCloseIter.next(pts) == SkPath::kDone_Verb);

    // The GM degeneratesegments.cpp test is more extensive
}

static void test_raw_iter(skiatest::Reporter* reporter) {
    SkPath p;
    SkPoint pts[4];

    // Test an iterator with no path
    SkPath::RawIter noPathIter;
    REPORTER_ASSERT(reporter, noPathIter.next(pts) == SkPath::kDone_Verb);
    // Test that setting an empty path works
    noPathIter.setPath(p);
    REPORTER_ASSERT(reporter, noPathIter.next(pts) == SkPath::kDone_Verb);
    
    // Test an iterator with an initial empty path
    SkPath::RawIter iter(p);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Test that a move-only path returns the move.
    p.moveTo(SK_Scalar1, 0);
    iter.setPath(p);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, pts[0].fY == 0);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // No matter how many moves we add, we should get them all back
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.moveTo(SK_Scalar1*3, SK_Scalar1*2);
    iter.setPath(p);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, pts[0].fY == 0);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*3);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Initial close is never ever stored
    p.reset();
    p.close();
    iter.setPath(p);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Move/close sequences
    p.reset();
    p.close(); // Not stored, no purpose
    p.moveTo(SK_Scalar1, 0);
    p.close();
    p.close(); // Not stored, no purpose
    p.moveTo(SK_Scalar1*2, SK_Scalar1);
    p.close();
    p.moveTo(SK_Scalar1*3, SK_Scalar1*2);
    p.moveTo(SK_Scalar1*4, SK_Scalar1*3);
    p.close();
    iter.setPath(p);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, pts[0].fY == 0);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kClose_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, pts[0].fY == 0);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kClose_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*3);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1*2);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kMove_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*4);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1*3);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kClose_Verb);
    REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*4);
    REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1*3);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);

    // Generate random paths and verify
    SkPoint randomPts[25];
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            randomPts[i*5+j].set(SK_Scalar1*i, SK_Scalar1*j);
        }
    }

    // Max of 10 segments, max 3 points per segment
    SkRandom rand(9876543);
    SkPoint          expectedPts[31]; // May have leading moveTo
    SkPath::Verb     expectedVerbs[22]; // May have leading moveTo
    SkPath::Verb     nextVerb;

    for (int i = 0; i < 500; ++i) {
        p.reset();
        bool lastWasClose = true;
        bool haveMoveTo = false;
        SkPoint lastMoveToPt = { 0, 0 };
        int numPoints = 0;
        int numVerbs = (rand.nextU() >> 16) % 10;
        int numIterVerbs = 0;
        for (int j = 0; j < numVerbs; ++j) {
            do {
                nextVerb = static_cast<SkPath::Verb>((rand.nextU() >> 16) % SkPath::kDone_Verb);
            } while (lastWasClose && nextVerb == SkPath::kClose_Verb);
            switch (nextVerb) {
                case SkPath::kMove_Verb:
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    p.moveTo(expectedPts[numPoints]);
                    lastMoveToPt = expectedPts[numPoints];
                    numPoints += 1;
                    lastWasClose = false;
                    haveMoveTo = true;
                    break;
                case SkPath::kLine_Verb:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPath::kMove_Verb;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    p.lineTo(expectedPts[numPoints]);
                    numPoints += 1;
                    lastWasClose = false;
                    break;
                case SkPath::kQuad_Verb:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPath::kMove_Verb;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    p.quadTo(expectedPts[numPoints], expectedPts[numPoints + 1]);
                    numPoints += 2;
                    lastWasClose = false;
                    break;
                case SkPath::kCubic_Verb:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPath::kMove_Verb;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 2] = randomPts[(rand.nextU() >> 16) % 25];
                    p.cubicTo(expectedPts[numPoints], expectedPts[numPoints + 1],
                              expectedPts[numPoints + 2]);
                    numPoints += 3;
                    lastWasClose = false;
                    break;
                case SkPath::kClose_Verb:
                    p.close();
                    haveMoveTo = false;
                    lastWasClose = true;
                    break;
                default:;
            }
            expectedVerbs[numIterVerbs++] = nextVerb;
        }
        
        iter.setPath(p);
        numVerbs = numIterVerbs;
        numIterVerbs = 0;
        int numIterPts = 0;
        SkPoint lastMoveTo;
        SkPoint lastPt;
        lastMoveTo.set(0, 0);
        lastPt.set(0, 0);
        while ((nextVerb = iter.next(pts)) != SkPath::kDone_Verb) {
            REPORTER_ASSERT(reporter, nextVerb == expectedVerbs[numIterVerbs]);
            numIterVerbs++;
            switch (nextVerb) {
                case SkPath::kMove_Verb:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints);
                    REPORTER_ASSERT(reporter, pts[0] == expectedPts[numIterPts]);
                    lastPt = lastMoveTo = pts[0];
                    numIterPts += 1;
                    break;
                case SkPath::kLine_Verb:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 1);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    lastPt = pts[1];
                    numIterPts += 1;
                    break;
                case SkPath::kQuad_Verb:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 2);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    REPORTER_ASSERT(reporter, pts[2] == expectedPts[numIterPts + 1]);
                    lastPt = pts[2];
                    numIterPts += 2;
                    break;
                case SkPath::kCubic_Verb:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 3);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    REPORTER_ASSERT(reporter, pts[2] == expectedPts[numIterPts + 1]);
                    REPORTER_ASSERT(reporter, pts[3] == expectedPts[numIterPts + 2]);
                    lastPt = pts[3];
                    numIterPts += 3;
                    break;
                case SkPath::kClose_Verb:
                    REPORTER_ASSERT(reporter, pts[0] == lastMoveTo);
                    lastPt = lastMoveTo;
                    break;
                default:;
            }
        }
        REPORTER_ASSERT(reporter, numIterPts == numPoints);
        REPORTER_ASSERT(reporter, numIterVerbs == numVerbs);
    }
}

static void check_for_circle(skiatest::Reporter* reporter,
                             const SkPath& path, bool expected) {
    SkRect rect;
    REPORTER_ASSERT(reporter, path.isOval(&rect) == expected);
    if (expected) {
        REPORTER_ASSERT(reporter, rect.height() == rect.width());
    }
}

static void test_circle_skew(skiatest::Reporter* reporter,
                             const SkPath& path) {
    SkPath tmp;

    SkMatrix m;
    m.setSkew(SkIntToScalar(3), SkIntToScalar(5));
    path.transform(m, &tmp);
    check_for_circle(reporter, tmp, false);
}

static void test_circle_translate(skiatest::Reporter* reporter,
                                  const SkPath& path) {
    SkPath tmp;

    // translate at small offset
    SkMatrix m;
    m.setTranslate(SkIntToScalar(15), SkIntToScalar(15));
    path.transform(m, &tmp);
    check_for_circle(reporter, tmp, true);

    tmp.reset();
    m.reset();

    // translate at a relatively big offset
    m.setTranslate(SkIntToScalar(1000), SkIntToScalar(1000));
    path.transform(m, &tmp);
    check_for_circle(reporter, tmp, true);
}

static void test_circle_rotate(skiatest::Reporter* reporter,
                               const SkPath& path) {
    for (int angle = 0; angle < 360; ++angle) {
        SkPath tmp;
        SkMatrix m;
        m.setRotate(SkIntToScalar(angle));
        path.transform(m, &tmp);

        // TODO: a rotated circle whose rotated angle is not a mutiple of 90
        // degrees is not an oval anymore, this can be improved.  we made this
        // for the simplicity of our implementation.
        if (angle % 90 == 0) {
            check_for_circle(reporter, tmp, true);
        } else {
            check_for_circle(reporter, tmp, false);
        }
    }
}

static void test_circle_with_direction(skiatest::Reporter* reporter,
                                       SkPath::Direction dir) {
    SkPath path;

    // circle at origin
    path.addCircle(0, 0, SkIntToScalar(20), dir);
    check_for_circle(reporter, path, true);
    test_circle_rotate(reporter, path);
    test_circle_translate(reporter, path);
    test_circle_skew(reporter, path);

    // circle at an offset at (10, 10)
    path.reset();
    path.addCircle(SkIntToScalar(10), SkIntToScalar(10),
                   SkIntToScalar(20), dir);
    check_for_circle(reporter, path, true);
    test_circle_rotate(reporter, path);
    test_circle_translate(reporter, path);
    test_circle_skew(reporter, path);
}

static void test_circle_with_add_paths(skiatest::Reporter* reporter) {
    SkPath path;
    SkPath circle;
    SkPath rect;
    SkPath empty;

    circle.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    rect.addRect(SkIntToScalar(5), SkIntToScalar(5),
                 SkIntToScalar(20), SkIntToScalar(20), SkPath::kCW_Direction);

    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(12), SkIntToScalar(12));

    // For simplicity, all the path concatenation related operations
    // would mark it non-circle, though in theory it's still a circle.

    // empty + circle (translate)
    path = empty;
    path.addPath(circle, translate);
    check_for_circle(reporter, path, false);

    // circle + empty (translate)
    path = circle;
    path.addPath(empty, translate);
    check_for_circle(reporter, path, false);

    // test reverseAddPath
    path = circle;
    path.reverseAddPath(rect);
    check_for_circle(reporter, path, false);
}

static void test_circle(skiatest::Reporter* reporter) {
    test_circle_with_direction(reporter, SkPath::kCW_Direction);
    test_circle_with_direction(reporter, SkPath::kCCW_Direction);

    // multiple addCircle()
    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(20), SkPath::kCW_Direction);
    check_for_circle(reporter, path, false);

    // some extra lineTo() would make isOval() fail
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.lineTo(0, 0);
    check_for_circle(reporter, path, false);

    // not back to the original point
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.setLastPt(SkIntToScalar(5), SkIntToScalar(5));
    check_for_circle(reporter, path, false);

    test_circle_with_add_paths(reporter);
}

static void test_oval(skiatest::Reporter* reporter) {
    SkRect rect;
    SkMatrix m;
    SkPath path;

    rect = SkRect::MakeWH(SkIntToScalar(30), SkIntToScalar(50));
    path.addOval(rect);

    REPORTER_ASSERT(reporter, path.isOval(NULL));

    m.setRotate(SkIntToScalar(90));
    SkPath tmp;
    path.transform(m, &tmp);
    // an oval rotated 90 degrees is still an oval.
    REPORTER_ASSERT(reporter, tmp.isOval(NULL));

    m.reset();
    m.setRotate(SkIntToScalar(30));
    tmp.reset();
    path.transform(m, &tmp);
    // an oval rotated 30 degrees is not an oval anymore.
    REPORTER_ASSERT(reporter, !tmp.isOval(NULL));

    // since empty path being transformed.
    path.reset();
    tmp.reset();
    m.reset();
    path.transform(m, &tmp);
    REPORTER_ASSERT(reporter, !tmp.isOval(NULL));

    // empty path is not an oval
    tmp.reset();
    REPORTER_ASSERT(reporter, !tmp.isOval(NULL));

    // only has moveTo()s
    tmp.reset();
    tmp.moveTo(0, 0);
    tmp.moveTo(SkIntToScalar(10), SkIntToScalar(10));
    REPORTER_ASSERT(reporter, !tmp.isOval(NULL));

    // mimic WebKit's calling convention,
    // call moveTo() first and then call addOval()
    path.reset();
    path.moveTo(0, 0);
    path.addOval(rect);
    REPORTER_ASSERT(reporter, path.isOval(NULL));

    // copy path
    path.reset();
    tmp.reset();
    tmp.addOval(rect);
    path = tmp;
    REPORTER_ASSERT(reporter, path.isOval(NULL));
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
    REPORTER_ASSERT(reporter, 0 == p.countPoints());
    REPORTER_ASSERT(reporter, 0 == p.countVerbs());
    REPORTER_ASSERT(reporter, 0 == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == p2);
    REPORTER_ASSERT(reporter, !(p != p2));

    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());

    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);

    p.addRoundRect(bounds, SK_Scalar1, SK_Scalar1);
    check_convex_bounds(reporter, p, bounds);
    // we have quads or cubics
    REPORTER_ASSERT(reporter, p.getSegmentMasks() & kCurveSegmentMask);
    REPORTER_ASSERT(reporter, !p.isEmpty());

    p.reset();
    REPORTER_ASSERT(reporter, 0 == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, p.isEmpty());

    p.addOval(bounds);
    check_convex_bounds(reporter, p, bounds);
    REPORTER_ASSERT(reporter, !p.isEmpty());

    p.reset();
    p.addRect(bounds);
    check_convex_bounds(reporter, p, bounds);
    // we have only lines
    REPORTER_ASSERT(reporter, SkPath::kLine_SegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());

    REPORTER_ASSERT(reporter, p != p2);
    REPORTER_ASSERT(reporter, !(p == p2));

    // do getPoints and getVerbs return the right result
    REPORTER_ASSERT(reporter, p.getPoints(NULL, 0) == 4);
    REPORTER_ASSERT(reporter, p.getVerbs(NULL, 0) == 5);
    SkPoint pts[4];
    int count = p.getPoints(pts, 4);
    REPORTER_ASSERT(reporter, count == 4);
    uint8_t verbs[6];
    verbs[5] = 0xff;
    p.getVerbs(verbs, 5);
    REPORTER_ASSERT(reporter, SkPath::kMove_Verb == verbs[0]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[1]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[2]);
    REPORTER_ASSERT(reporter, SkPath::kLine_Verb == verbs[3]);
    REPORTER_ASSERT(reporter, SkPath::kClose_Verb == verbs[4]);
    REPORTER_ASSERT(reporter, 0xff == verbs[5]);
    bounds2.set(pts, 4);
    REPORTER_ASSERT(reporter, bounds == bounds2);

    bounds.offset(SK_Scalar1*3, SK_Scalar1*4);
    p.offset(SK_Scalar1*3, SK_Scalar1*4);
    REPORTER_ASSERT(reporter, bounds == p.getBounds());

    REPORTER_ASSERT(reporter, p.isRect(NULL));
    bounds2.setEmpty();
    REPORTER_ASSERT(reporter, p.isRect(&bounds2));
    REPORTER_ASSERT(reporter, bounds == bounds2);

    // now force p to not be a rect
    bounds.set(0, 0, SK_Scalar1/2, SK_Scalar1/2);
    p.addRect(bounds);
    REPORTER_ASSERT(reporter, !p.isRect(NULL));

    test_isLine(reporter);
    test_isRect(reporter);
    test_zero_length_paths(reporter);
    test_direction(reporter);
    test_convexity(reporter);
    test_convexity2(reporter);
    test_close(reporter);
    test_segment_masks(reporter);
    test_flattening(reporter);
    test_transform(reporter);
    test_bounds(reporter);
    test_iter(reporter);
    test_raw_iter(reporter);
    test_circle(reporter);
    test_oval(reporter);
    test_strokerec(reporter);
    test_addPoly(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Path", PathTestClass, TestPath)
