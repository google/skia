
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkParse.h"
#include "SkParsePath.h"
#include "SkPathEffect.h"
#include "SkRandom.h"
#include "SkReader32.h"
#include "SkSize.h"
#include "SkSurface.h"
#include "SkTypes.h"
#include "SkWriter32.h"

#if defined(WIN32)
    #define SUPPRESS_VISIBILITY_WARNING
#else
    #define SUPPRESS_VISIBILITY_WARNING __attribute__((visibility("hidden")))
#endif

static SkSurface* new_surface(int w, int h) {
    SkImage::Info info = {
        w, h, SkImage::kPMColor_ColorType, SkImage::kPremul_AlphaType
    };
    return SkSurface::NewRaster(info);
}

static void test_path_close_issue1474(skiatest::Reporter* reporter) {
    // This test checks that r{Line,Quad,Conic,Cubic}To following a close()
    // are relative to the point we close to, not relative to the point we close from.
    SkPath path;
    SkPoint last;

    // Test rLineTo().
    path.rLineTo(0, 100);
    path.rLineTo(100, 0);
    path.close();          // Returns us back to 0,0.
    path.rLineTo(50, 50);  // This should go to 50,50.

    path.getLastPt(&last);
    REPORTER_ASSERT(reporter, 50 == last.fX);
    REPORTER_ASSERT(reporter, 50 == last.fY);

    // Test rQuadTo().
    path.rewind();
    path.rLineTo(0, 100);
    path.rLineTo(100, 0);
    path.close();
    path.rQuadTo(50, 50, 75, 75);

    path.getLastPt(&last);
    REPORTER_ASSERT(reporter, 75 == last.fX);
    REPORTER_ASSERT(reporter, 75 == last.fY);

    // Test rConicTo().
    path.rewind();
    path.rLineTo(0, 100);
    path.rLineTo(100, 0);
    path.close();
    path.rConicTo(50, 50, 85, 85, 2);

    path.getLastPt(&last);
    REPORTER_ASSERT(reporter, 85 == last.fX);
    REPORTER_ASSERT(reporter, 85 == last.fY);

    // Test rCubicTo().
    path.rewind();
    path.rLineTo(0, 100);
    path.rLineTo(100, 0);
    path.close();
    path.rCubicTo(50, 50, 85, 85, 95, 95);

    path.getLastPt(&last);
    REPORTER_ASSERT(reporter, 95 == last.fX);
    REPORTER_ASSERT(reporter, 95 == last.fY);
}

static void test_android_specific_behavior(skiatest::Reporter* reporter) {
#ifdef SK_BUILD_FOR_ANDROID
    // Copy constructor should preserve generation ID, but assignment shouldn't.
    SkPath original;
    original.moveTo(0, 0);
    original.lineTo(1, 1);
    REPORTER_ASSERT(reporter, original.getGenerationID() > 0);

    const SkPath copy(original);
    REPORTER_ASSERT(reporter, copy.getGenerationID() == original.getGenerationID());

    SkPath assign;
    assign = original;
    REPORTER_ASSERT(reporter, assign.getGenerationID() != original.getGenerationID());
#endif
}

// This used to assert in the debug build, as the edges did not all line-up.
static void test_bad_cubic_crbug234190() {
    SkPath path;
    path.moveTo(13.8509f, 3.16858f);
    path.cubicTo(-2.35893e+08f, -4.21044e+08f,
                 -2.38991e+08f, -4.26573e+08f,
                 -2.41016e+08f, -4.30188e+08f);

    SkPaint paint;
    paint.setAntiAlias(true);
    SkAutoTUnref<SkSurface> surface(new_surface(84, 88));
    surface->getCanvas()->drawPath(path, paint);
}

static void test_bad_cubic_crbug229478() {
    const SkPoint pts[] = {
        { 4595.91064f,    -11596.9873f },
        { 4597.2168f,    -11595.9414f },
        { 4598.52344f,    -11594.8955f },
        { 4599.83008f,    -11593.8496f },
    };

    SkPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[1], pts[2], pts[3]);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);

    SkPath dst;
    // Before the fix, this would infinite-recurse, and run out of stack
    // because we would keep trying to subdivide a degenerate cubic segment.
    paint.getFillPath(path, &dst, NULL);
}

static void build_path_170666(SkPath& path) {
    path.moveTo(17.9459f, 21.6344f);
    path.lineTo(139.545f, -47.8105f);
    path.lineTo(139.545f, -47.8105f);
    path.lineTo(131.07f, -47.3888f);
    path.lineTo(131.07f, -47.3888f);
    path.lineTo(122.586f, -46.9532f);
    path.lineTo(122.586f, -46.9532f);
    path.lineTo(18076.6f, 31390.9f);
    path.lineTo(18076.6f, 31390.9f);
    path.lineTo(18085.1f, 31390.5f);
    path.lineTo(18085.1f, 31390.5f);
    path.lineTo(18076.6f, 31390.9f);
    path.lineTo(18076.6f, 31390.9f);
    path.lineTo(17955, 31460.3f);
    path.lineTo(17955, 31460.3f);
    path.lineTo(17963.5f, 31459.9f);
    path.lineTo(17963.5f, 31459.9f);
    path.lineTo(17971.9f, 31459.5f);
    path.lineTo(17971.9f, 31459.5f);
    path.lineTo(17.9551f, 21.6205f);
    path.lineTo(17.9551f, 21.6205f);
    path.lineTo(9.47091f, 22.0561f);
    path.lineTo(9.47091f, 22.0561f);
    path.lineTo(17.9459f, 21.6344f);
    path.lineTo(17.9459f, 21.6344f);
    path.close();path.moveTo(0.995934f, 22.4779f);
    path.lineTo(0.986725f, 22.4918f);
    path.lineTo(0.986725f, 22.4918f);
    path.lineTo(17955, 31460.4f);
    path.lineTo(17955, 31460.4f);
    path.lineTo(17971.9f, 31459.5f);
    path.lineTo(17971.9f, 31459.5f);
    path.lineTo(18093.6f, 31390.1f);
    path.lineTo(18093.6f, 31390.1f);
    path.lineTo(18093.6f, 31390);
    path.lineTo(18093.6f, 31390);
    path.lineTo(139.555f, -47.8244f);
    path.lineTo(139.555f, -47.8244f);
    path.lineTo(122.595f, -46.9671f);
    path.lineTo(122.595f, -46.9671f);
    path.lineTo(0.995934f, 22.4779f);
    path.lineTo(0.995934f, 22.4779f);
    path.close();
    path.moveTo(5.43941f, 25.5223f);
    path.lineTo(798267, -28871.1f);
    path.lineTo(798267, -28871.1f);
    path.lineTo(3.12512e+06f, -113102);
    path.lineTo(3.12512e+06f, -113102);
    path.cubicTo(5.16324e+06f, -186882, 8.15247e+06f, -295092, 1.1957e+07f, -432813);
    path.cubicTo(1.95659e+07f, -708257, 3.04359e+07f, -1.10175e+06f, 4.34798e+07f, -1.57394e+06f);
    path.cubicTo(6.95677e+07f, -2.51831e+06f, 1.04352e+08f, -3.77748e+06f, 1.39135e+08f, -5.03666e+06f);
    path.cubicTo(1.73919e+08f, -6.29583e+06f, 2.08703e+08f, -7.555e+06f, 2.34791e+08f, -8.49938e+06f);
    path.cubicTo(2.47835e+08f, -8.97157e+06f, 2.58705e+08f, -9.36506e+06f, 2.66314e+08f, -9.6405e+06f);
    path.cubicTo(2.70118e+08f, -9.77823e+06f, 2.73108e+08f, -9.88644e+06f, 2.75146e+08f, -9.96022e+06f);
    path.cubicTo(2.76165e+08f, -9.99711e+06f, 2.76946e+08f, -1.00254e+07f, 2.77473e+08f, -1.00444e+07f);
    path.lineTo(2.78271e+08f, -1.00733e+07f);
    path.lineTo(2.78271e+08f, -1.00733e+07f);
    path.cubicTo(2.78271e+08f, -1.00733e+07f, 2.08703e+08f, -7.555e+06f, 135.238f, 23.3517f);
    path.cubicTo(131.191f, 23.4981f, 125.995f, 23.7976f, 123.631f, 24.0206f);
    path.cubicTo(121.267f, 24.2436f, 122.631f, 24.3056f, 126.677f, 24.1591f);
    path.cubicTo(2.08703e+08f, -7.555e+06f, 2.78271e+08f, -1.00733e+07f, 2.78271e+08f, -1.00733e+07f);
    path.lineTo(2.77473e+08f, -1.00444e+07f);
    path.lineTo(2.77473e+08f, -1.00444e+07f);
    path.cubicTo(2.76946e+08f, -1.00254e+07f, 2.76165e+08f, -9.99711e+06f, 2.75146e+08f, -9.96022e+06f);
    path.cubicTo(2.73108e+08f, -9.88644e+06f, 2.70118e+08f, -9.77823e+06f, 2.66314e+08f, -9.6405e+06f);
    path.cubicTo(2.58705e+08f, -9.36506e+06f, 2.47835e+08f, -8.97157e+06f, 2.34791e+08f, -8.49938e+06f);
    path.cubicTo(2.08703e+08f, -7.555e+06f, 1.73919e+08f, -6.29583e+06f, 1.39135e+08f, -5.03666e+06f);
    path.cubicTo(1.04352e+08f, -3.77749e+06f, 6.95677e+07f, -2.51831e+06f, 4.34798e+07f, -1.57394e+06f);
    path.cubicTo(3.04359e+07f, -1.10175e+06f, 1.95659e+07f, -708258, 1.1957e+07f, -432814);
    path.cubicTo(8.15248e+06f, -295092, 5.16324e+06f, -186883, 3.12513e+06f, -113103);
    path.lineTo(798284, -28872);
    path.lineTo(798284, -28872);
    path.lineTo(22.4044f, 24.6677f);
    path.lineTo(22.4044f, 24.6677f);
    path.cubicTo(22.5186f, 24.5432f, 18.8134f, 24.6337f, 14.1287f, 24.8697f);
    path.cubicTo(9.4439f, 25.1057f, 5.55359f, 25.3978f, 5.43941f, 25.5223f);
    path.close();
}

static void build_path_simple_170666(SkPath& path) {
    path.moveTo(126.677f, 24.1591f);
    path.cubicTo(2.08703e+08f, -7.555e+06f, 2.78271e+08f, -1.00733e+07f, 2.78271e+08f, -1.00733e+07f);
}

// This used to assert in the SK_DEBUG build, as the clip step would fail with
// too-few interations in our cubic-line intersection code. That code now runs
// 24 interations (instead of 16).
static void test_crbug_170666() {
    SkPath path;
    SkPaint paint;
    paint.setAntiAlias(true);

    SkAutoTUnref<SkSurface> surface(new_surface(1000, 1000));

    build_path_simple_170666(path);
    surface->getCanvas()->drawPath(path, paint);

    build_path_170666(path);
    surface->getCanvas()->drawPath(path, paint);
}

// Make sure we stay non-finite once we get there (unless we reset or rewind).
static void test_addrect_isfinite(skiatest::Reporter* reporter) {
    SkPath path;

    path.addRect(SkRect::MakeWH(50, 100));
    REPORTER_ASSERT(reporter, path.isFinite());

    path.moveTo(0, 0);
    path.lineTo(SK_ScalarInfinity, 42);
    REPORTER_ASSERT(reporter, !path.isFinite());

    path.addRect(SkRect::MakeWH(50, 100));
    REPORTER_ASSERT(reporter, !path.isFinite());

    path.reset();
    REPORTER_ASSERT(reporter, path.isFinite());

    path.addRect(SkRect::MakeWH(50, 100));
    REPORTER_ASSERT(reporter, path.isFinite());
}

static void build_big_path(SkPath* path, bool reducedCase) {
    if (reducedCase) {
        path->moveTo(577330, 1971.72f);
        path->cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
    } else {
        path->moveTo(60.1631f, 7.70567f);
        path->quadTo(60.1631f, 7.70567f, 0.99474f, 0.901199f);
        path->lineTo(577379, 1977.77f);
        path->quadTo(577364, 1979.57f, 577325, 1980.26f);
        path->quadTo(577286, 1980.95f, 577245, 1980.13f);
        path->quadTo(577205, 1979.3f, 577187, 1977.45f);
        path->quadTo(577168, 1975.6f, 577183, 1973.8f);
        path->quadTo(577198, 1972, 577238, 1971.31f);
        path->quadTo(577277, 1970.62f, 577317, 1971.45f);
        path->quadTo(577330, 1971.72f, 577341, 1972.11f);
        path->cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
        path->moveTo(306.718f, -32.912f);
        path->cubicTo(30.531f, 10.0005f, 1502.47f, 13.2804f, 84.3088f, 9.99601f);
    }
}

static void test_clipped_cubic() {
    SkAutoTUnref<SkSurface> surface(new_surface(640, 480));

    // This path used to assert, because our cubic-chopping code incorrectly
    // moved control points after the chop. This test should be run in SK_DEBUG
    // mode to ensure that we no long assert.
    SkPath path;
    for (int doReducedCase = 0; doReducedCase <= 1; ++doReducedCase) {
        build_big_path(&path, SkToBool(doReducedCase));

        SkPaint paint;
        for (int doAA = 0; doAA <= 1; ++doAA) {
            paint.setAntiAlias(SkToBool(doAA));
            surface->getCanvas()->drawPath(path, paint);
        }
    }
}

// Inspired by http://ie.microsoft.com/testdrive/Performance/Chalkboard/
// which triggered an assert, from a tricky cubic. This test replicates that
// example, so we can ensure that we handle it (in SkEdge.cpp), and don't
// assert in the SK_DEBUG build.
static void test_tricky_cubic() {
    const SkPoint pts[] = {
        { SkDoubleToScalar(18.8943768),    SkDoubleToScalar(129.121277) },
        { SkDoubleToScalar(18.8937435),    SkDoubleToScalar(129.121689) },
        { SkDoubleToScalar(18.8950119),    SkDoubleToScalar(129.120422) },
        { SkDoubleToScalar(18.5030727),    SkDoubleToScalar(129.13121)  },
    };

    SkPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[1], pts[2], pts[3]);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkSurface* surface = new_surface(19, 130);
    surface->getCanvas()->drawPath(path, paint);
    surface->unref();
}

// Inspired by http://code.google.com/p/chromium/issues/detail?id=141651
//
static void test_isfinite_after_transform(skiatest::Reporter* reporter) {
    SkPath path;
    path.quadTo(157, 366, 286, 208);
    path.arcTo(37, 442, 315, 163, 957494590897113.0f);

    SkMatrix matrix;
    matrix.setScale(1000*1000, 1000*1000);

    // Be sure that path::transform correctly updates isFinite and the bounds
    // if the transformation overflows. The previous bug was that isFinite was
    // set to true in this case, but the bounds were not set to empty (which
    // they should be).
    while (path.isFinite()) {
        REPORTER_ASSERT(reporter, path.getBounds().isFinite());
        REPORTER_ASSERT(reporter, !path.getBounds().isEmpty());
        path.transform(matrix);
    }
    REPORTER_ASSERT(reporter, path.getBounds().isEmpty());

    matrix.setTranslate(SK_Scalar1, SK_Scalar1);
    path.transform(matrix);
    // we need to still be non-finite
    REPORTER_ASSERT(reporter, !path.isFinite());
    REPORTER_ASSERT(reporter, path.getBounds().isEmpty());
}

static void add_corner_arc(SkPath* path, const SkRect& rect,
                           SkScalar xIn, SkScalar yIn,
                           int startAngle)
{

    SkScalar rx = SkMinScalar(rect.width(), xIn);
    SkScalar ry = SkMinScalar(rect.height(), yIn);

    SkRect arcRect;
    arcRect.set(-rx, -ry, rx, ry);
    switch (startAngle) {
    case 0:
        arcRect.offset(rect.fRight - arcRect.fRight, rect.fBottom - arcRect.fBottom);
        break;
    case 90:
        arcRect.offset(rect.fLeft - arcRect.fLeft, rect.fBottom - arcRect.fBottom);
        break;
    case 180:
        arcRect.offset(rect.fLeft - arcRect.fLeft, rect.fTop - arcRect.fTop);
        break;
    case 270:
        arcRect.offset(rect.fRight - arcRect.fRight, rect.fTop - arcRect.fTop);
        break;
    default:
        break;
    }

    path->arcTo(arcRect, SkIntToScalar(startAngle), SkIntToScalar(90), false);
}

static void make_arb_round_rect(SkPath* path, const SkRect& r,
                                SkScalar xCorner, SkScalar yCorner) {
    // we are lazy here and use the same x & y for each corner
    add_corner_arc(path, r, xCorner, yCorner, 270);
    add_corner_arc(path, r, xCorner, yCorner, 0);
    add_corner_arc(path, r, xCorner, yCorner, 90);
    add_corner_arc(path, r, xCorner, yCorner, 180);
    path->close();
}

// Chrome creates its own round rects with each corner possibly being different.
// Performance will suffer if they are not convex.
// Note: PathBench::ArbRoundRectBench performs almost exactly
// the same test (but with drawing)
static void test_arb_round_rect_is_convex(skiatest::Reporter* reporter) {
    SkMWCRandom rand;
    SkRect r;

    for (int i = 0; i < 5000; ++i) {

        SkScalar size = rand.nextUScalar1() * 30;
        if (size < SK_Scalar1) {
            continue;
        }
        r.fLeft = rand.nextUScalar1() * 300;
        r.fTop =  rand.nextUScalar1() * 300;
        r.fRight =  r.fLeft + 2 * size;
        r.fBottom = r.fTop + 2 * size;

        SkPath temp;

        make_arb_round_rect(&temp, r, r.width() / 10, r.height() / 15);

        REPORTER_ASSERT(reporter, temp.isConvex());
    }
}

// Chrome will sometimes create a 0 radius round rect. The degenerate
// quads prevent the path from being converted to a rect
// Note: PathBench::ArbRoundRectBench performs almost exactly
// the same test (but with drawing)
static void test_arb_zero_rad_round_rect_is_rect(skiatest::Reporter* reporter) {
    SkMWCRandom rand;
    SkRect r;

    for (int i = 0; i < 5000; ++i) {

        SkScalar size = rand.nextUScalar1() * 30;
        if (size < SK_Scalar1) {
            continue;
        }
        r.fLeft = rand.nextUScalar1() * 300;
        r.fTop =  rand.nextUScalar1() * 300;
        r.fRight =  r.fLeft + 2 * size;
        r.fBottom = r.fTop + 2 * size;

        SkPath temp;

        make_arb_round_rect(&temp, r, 0, 0);

        SkRect result;
        REPORTER_ASSERT(reporter, temp.isRect(&result));
        REPORTER_ASSERT(reporter, r == result);
    }
}

static void test_rect_isfinite(skiatest::Reporter* reporter) {
    const SkScalar inf = SK_ScalarInfinity;
    const SkScalar negInf = SK_ScalarNegativeInfinity;
    const SkScalar nan = SK_ScalarNaN;

    SkRect r;
    r.setEmpty();
    REPORTER_ASSERT(reporter, r.isFinite());
    r.set(0, 0, inf, negInf);
    REPORTER_ASSERT(reporter, !r.isFinite());
    r.set(0, 0, nan, 0);
    REPORTER_ASSERT(reporter, !r.isFinite());

    SkPoint pts[] = {
        { 0, 0 },
        { SK_Scalar1, 0 },
        { 0, SK_Scalar1 },
    };

    bool isFine = r.setBoundsCheck(pts, 3);
    REPORTER_ASSERT(reporter, isFine);
    REPORTER_ASSERT(reporter, !r.isEmpty());

    pts[1].set(inf, 0);
    isFine = r.setBoundsCheck(pts, 3);
    REPORTER_ASSERT(reporter, !isFine);
    REPORTER_ASSERT(reporter, r.isEmpty());

    pts[1].set(nan, 0);
    isFine = r.setBoundsCheck(pts, 3);
    REPORTER_ASSERT(reporter, !isFine);
    REPORTER_ASSERT(reporter, r.isEmpty());
}

static void test_path_isfinite(skiatest::Reporter* reporter) {
    const SkScalar inf = SK_ScalarInfinity;
    const SkScalar negInf = SK_ScalarNegativeInfinity;
    const SkScalar nan = SK_ScalarNaN;

    SkPath path;
    REPORTER_ASSERT(reporter, path.isFinite());

    path.reset();
    REPORTER_ASSERT(reporter, path.isFinite());

    path.reset();
    path.moveTo(SK_Scalar1, 0);
    REPORTER_ASSERT(reporter, path.isFinite());

    path.reset();
    path.moveTo(inf, negInf);
    REPORTER_ASSERT(reporter, !path.isFinite());

    path.reset();
    path.moveTo(nan, 0);
    REPORTER_ASSERT(reporter, !path.isFinite());
}

static void test_isfinite(skiatest::Reporter* reporter) {
    test_rect_isfinite(reporter);
    test_path_isfinite(reporter);
}

// assert that we always
//  start with a moveTo
//  only have 1 moveTo
//  only have Lines after that
//  end with a single close
//  only have (at most) 1 close
//
static void test_poly(skiatest::Reporter* reporter, const SkPath& path,
                      const SkPoint srcPts[], bool expectClose) {
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
            case SkPath::kConic_Verb:
                REPORTER_ASSERT(reporter, !"unexpected conic verb");
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
    SkMWCRandom rand;

    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        pts[i].fX = rand.nextSScalar1();
        pts[i].fY = rand.nextSScalar1();
    }

    for (int doClose = 0; doClose <= 1; ++doClose) {
        for (size_t count = 1; count <= SK_ARRAY_COUNT(pts); ++count) {
            SkPath path;
            path.addPoly(pts, count, SkToBool(doClose));
            test_poly(reporter, path, pts, SkToBool(doClose));
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

// Set this for paths that don't have a consistent direction such as a bowtie.
// (cheapComputeDirection is not expected to catch these.)
static const SkPath::Direction kDontCheckDir = static_cast<SkPath::Direction>(-1);

static void check_direction(skiatest::Reporter* reporter, const SkPath& path,
                            SkPath::Direction expected) {
    if (expected == kDontCheckDir) {
        return;
    }
    SkPath copy(path); // we make a copy so that we don't cache the result on the passed in path.

    SkPath::Direction dir;
    if (copy.cheapComputeDirection(&dir)) {
        REPORTER_ASSERT(reporter, dir == expected);
    } else {
        REPORTER_ASSERT(reporter, SkPath::kUnknown_Direction == expected);
    }
}

static void test_direction(skiatest::Reporter* reporter) {
    size_t i;
    SkPath path;
    REPORTER_ASSERT(reporter, !path.cheapComputeDirection(NULL));
    REPORTER_ASSERT(reporter, !path.cheapIsDirection(SkPath::kCW_Direction));
    REPORTER_ASSERT(reporter, !path.cheapIsDirection(SkPath::kCCW_Direction));
    REPORTER_ASSERT(reporter, path.cheapIsDirection(SkPath::kUnknown_Direction));

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
        // rect with top two corners replaced by cubics with identical middle
        // control points
        "M 10 10 C 10 0 10 0 20 0 L 40 0 C 50 0 50 0 50 10",
        "M 20 10 L 0 10 Q 10 10 20 0",  // left, degenerate serif
    };
    for (i = 0; i < SK_ARRAY_COUNT(gCW); ++i) {
        path.reset();
        bool valid = SkParsePath::FromSVGString(gCW[i], &path);
        REPORTER_ASSERT(reporter, valid);
        check_direction(reporter, path, SkPath::kCW_Direction);
    }

    static const char* gCCW[] = {
        "M 10 10 L 10 10 Q 20 10 20 -20",
        "M 10 10 C 20 10 20 -20 20 -20",
        "M 20 10 Q 20 20 10 20 L 30 20", // test double-back at y-max
        // rect with top two corners replaced by cubics with identical middle
        // control points
        "M 50 10 C 50 0 50 0 40 0 L 20 0 C 10 0 10 0 10 10",
        "M 10 10 L 30 10 Q 20 10 10 0",  // right, degenerate serif
    };
    for (i = 0; i < SK_ARRAY_COUNT(gCCW); ++i) {
        path.reset();
        bool valid = SkParsePath::FromSVGString(gCCW[i], &path);
        REPORTER_ASSERT(reporter, valid);
        check_direction(reporter, path, SkPath::kCCW_Direction);
    }

    // Test two donuts, each wound a different direction. Only the outer contour
    // determines the cheap direction
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(2), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(1), SkPath::kCCW_Direction);
    check_direction(reporter, path, SkPath::kCW_Direction);

    path.reset();
    path.addCircle(0, 0, SkIntToScalar(1), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(2), SkPath::kCCW_Direction);
    check_direction(reporter, path, SkPath::kCCW_Direction);

#ifdef SK_SCALAR_IS_FLOAT
    // triangle with one point really far from the origin.
    path.reset();
    // the first point is roughly 1.05e10, 1.05e10
    path.moveTo(SkFloatToScalar(SkBits2Float(0x501c7652)), SkFloatToScalar(SkBits2Float(0x501c7652)));
    path.lineTo(110 * SK_Scalar1, -10 * SK_Scalar1);
    path.lineTo(-10 * SK_Scalar1, 60 * SK_Scalar1);
    check_direction(reporter, path, SkPath::kCCW_Direction);
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
    SkPath copy(path); // we make a copy so that we don't cache the result on the passed in path.
    SkPath::Convexity c = copy.getConvexity();
    REPORTER_ASSERT(reporter, c == expected);
}

static void test_convexity2(skiatest::Reporter* reporter) {
    SkPath pt;
    pt.moveTo(0, 0);
    pt.close();
    check_convexity(reporter, pt, SkPath::kConvex_Convexity);
    check_direction(reporter, pt, SkPath::kUnknown_Direction);

    SkPath line;
    line.moveTo(12*SK_Scalar1, 20*SK_Scalar1);
    line.lineTo(-12*SK_Scalar1, -20*SK_Scalar1);
    line.close();
    check_convexity(reporter, line, SkPath::kConvex_Convexity);
    check_direction(reporter, line, SkPath::kUnknown_Direction);

    SkPath triLeft;
    triLeft.moveTo(0, 0);
    triLeft.lineTo(SK_Scalar1, 0);
    triLeft.lineTo(SK_Scalar1, SK_Scalar1);
    triLeft.close();
    check_convexity(reporter, triLeft, SkPath::kConvex_Convexity);
    check_direction(reporter, triLeft, SkPath::kCW_Direction);

    SkPath triRight;
    triRight.moveTo(0, 0);
    triRight.lineTo(-SK_Scalar1, 0);
    triRight.lineTo(SK_Scalar1, SK_Scalar1);
    triRight.close();
    check_convexity(reporter, triRight, SkPath::kConvex_Convexity);
    check_direction(reporter, triRight, SkPath::kCCW_Direction);

    SkPath square;
    square.moveTo(0, 0);
    square.lineTo(SK_Scalar1, 0);
    square.lineTo(SK_Scalar1, SK_Scalar1);
    square.lineTo(0, SK_Scalar1);
    square.close();
    check_convexity(reporter, square, SkPath::kConvex_Convexity);
    check_direction(reporter, square, SkPath::kCW_Direction);

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
    check_direction(reporter, redundantSquare, SkPath::kCW_Direction);

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
    check_direction(reporter, bowTie, kDontCheckDir);

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
    check_direction(reporter, spiral, kDontCheckDir);

    SkPath dent;
    dent.moveTo(0, 0);
    dent.lineTo(100*SK_Scalar1, 100*SK_Scalar1);
    dent.lineTo(0, 100*SK_Scalar1);
    dent.lineTo(-50*SK_Scalar1, 200*SK_Scalar1);
    dent.lineTo(-200*SK_Scalar1, 100*SK_Scalar1);
    dent.close();
    check_convexity(reporter, dent, SkPath::kConcave_Convexity);
    check_direction(reporter, dent, SkPath::kCW_Direction);
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
    SkPath path;

    check_convexity(reporter, path, SkPath::kConvex_Convexity);
    path.addCircle(0, 0, SkIntToScalar(10));
    check_convexity(reporter, path, SkPath::kConvex_Convexity);
    path.addCircle(0, 0, SkIntToScalar(10));   // 2nd circle
    check_convexity(reporter, path, SkPath::kConcave_Convexity);

    path.reset();
    path.addRect(0, 0, SkIntToScalar(10), SkIntToScalar(10), SkPath::kCCW_Direction);
    check_convexity(reporter, path, SkPath::kConvex_Convexity);
    REPORTER_ASSERT(reporter, path.cheapIsDirection(SkPath::kCCW_Direction));

    path.reset();
    path.addRect(0, 0, SkIntToScalar(10), SkIntToScalar(10), SkPath::kCW_Direction);
    check_convexity(reporter, path, SkPath::kConvex_Convexity);
    REPORTER_ASSERT(reporter, path.cheapIsDirection(SkPath::kCW_Direction));

    static const struct {
        const char*         fPathStr;
        SkPath::Convexity   fExpectedConvexity;
        SkPath::Direction   fExpectedDirection;
    } gRec[] = {
        { "", SkPath::kConvex_Convexity, SkPath::kUnknown_Direction },
        { "0 0", SkPath::kConvex_Convexity, SkPath::kUnknown_Direction },
        { "0 0 10 10", SkPath::kConvex_Convexity, SkPath::kUnknown_Direction },
        { "0 0 10 10 20 20 0 0 10 10", SkPath::kConcave_Convexity, SkPath::kUnknown_Direction },
        { "0 0 10 10 10 20", SkPath::kConvex_Convexity, SkPath::kCW_Direction },
        { "0 0 10 10 10 0", SkPath::kConvex_Convexity, SkPath::kCCW_Direction },
        { "0 0 10 10 10 0 0 10", SkPath::kConcave_Convexity, kDontCheckDir },
        { "0 0 10 0 0 10 -10 -10", SkPath::kConcave_Convexity, SkPath::kCW_Direction },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkPath path;
        setFromString(&path, gRec[i].fPathStr);
        check_convexity(reporter, path, gRec[i].fExpectedConvexity);
        check_direction(reporter, path, gRec[i].fExpectedDirection);
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

static void test_conservativelyContains(skiatest::Reporter* reporter) {
    SkPath path;

    // kBaseRect is used to construct most our test paths: a rect, a circle, and a round-rect.
    static const SkRect kBaseRect = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));

    // A circle that bounds kBaseRect (with a significant amount of slop)
    SkScalar circleR = SkMaxScalar(kBaseRect.width(), kBaseRect.height());
    circleR = SkScalarMul(circleR, SkFloatToScalar(1.75f)) / 2;
    static const SkPoint kCircleC = {kBaseRect.centerX(), kBaseRect.centerY()};

    // round-rect radii
    static const SkScalar kRRRadii[] = {SkIntToScalar(5), SkIntToScalar(3)};

    static const struct SUPPRESS_VISIBILITY_WARNING {
        SkRect fQueryRect;
        bool   fInRect;
        bool   fInCircle;
        bool   fInRR;
    } kQueries[] = {
        {kBaseRect, true, true, false},

        // rect well inside of kBaseRect
        {SkRect::MakeLTRB(kBaseRect.fLeft + SkFloatToScalar(0.25f)*kBaseRect.width(),
                          kBaseRect.fTop + SkFloatToScalar(0.25f)*kBaseRect.height(),
                          kBaseRect.fRight - SkFloatToScalar(0.25f)*kBaseRect.width(),
                          kBaseRect.fBottom - SkFloatToScalar(0.25f)*kBaseRect.height()),
                          true, true, true},

        // rects with edges off by one from kBaseRect's edges
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width(), kBaseRect.height() + 1),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width() + 1, kBaseRect.height()),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width() + 1, kBaseRect.height() + 1),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft - 1, kBaseRect.fTop,
                          kBaseRect.width(), kBaseRect.height()),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop - 1,
                          kBaseRect.width(), kBaseRect.height()),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft - 1, kBaseRect.fTop,
                          kBaseRect.width() + 2, kBaseRect.height()),
         false, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop - 1,
                          kBaseRect.width() + 2, kBaseRect.height()),
         false, true, false},

        // zero-w/h rects at each corner of kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop, 0, 0), true, true, false},
        {SkRect::MakeXYWH(kBaseRect.fRight, kBaseRect.fTop, 0, 0), true, true, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fBottom, 0, 0), true, true, false},
        {SkRect::MakeXYWH(kBaseRect.fRight, kBaseRect.fBottom, 0, 0), true, true, false},

        // far away rect
        {SkRect::MakeXYWH(10 * kBaseRect.fRight, 10 * kBaseRect.fBottom,
                          SkIntToScalar(10), SkIntToScalar(10)),
         false, false, false},

        // very large rect containing kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft - 5 * kBaseRect.width(),
                          kBaseRect.fTop - 5 * kBaseRect.height(),
                          11 * kBaseRect.width(), 11 * kBaseRect.height()),
         false, false, false},

        // skinny rect that spans same y-range as kBaseRect
        {SkRect::MakeXYWH(kBaseRect.centerX(), kBaseRect.fTop,
                          SkIntToScalar(1), kBaseRect.height()),
         true, true, true},

        // short rect that spans same x-range as kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.centerY(), kBaseRect.width(), SkScalar(1)),
         true, true, true},

        // skinny rect that spans slightly larger y-range than kBaseRect
        {SkRect::MakeXYWH(kBaseRect.centerX(), kBaseRect.fTop,
                          SkIntToScalar(1), kBaseRect.height() + 1),
         false, true, false},

        // short rect that spans slightly larger x-range than kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.centerY(),
                          kBaseRect.width() + 1, SkScalar(1)),
         false, true, false},
    };

    for (int inv = 0; inv < 4; ++inv) {
        for (size_t q = 0; q < SK_ARRAY_COUNT(kQueries); ++q) {
            SkRect qRect = kQueries[q].fQueryRect;
            if (inv & 0x1) {
                SkTSwap(qRect.fLeft, qRect.fRight);
            }
            if (inv & 0x2) {
                SkTSwap(qRect.fTop, qRect.fBottom);
            }
            for (int d = 0; d < 2; ++d) {
                SkPath::Direction dir = d ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
                path.reset();
                path.addRect(kBaseRect, dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInRect ==
                                          path.conservativelyContainsRect(qRect));

                path.reset();
                path.addCircle(kCircleC.fX, kCircleC.fY, circleR, dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInCircle ==
                                          path.conservativelyContainsRect(qRect));

                path.reset();
                path.addRoundRect(kBaseRect, kRRRadii[0], kRRRadii[1], dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInRR ==
                                          path.conservativelyContainsRect(qRect));
            }
            // Slightly non-convex shape, shouldn't contain any rects.
            path.reset();
            path.moveTo(0, 0);
            path.lineTo(SkIntToScalar(50), SkFloatToScalar(0.05f));
            path.lineTo(SkIntToScalar(100), 0);
            path.lineTo(SkIntToScalar(100), SkIntToScalar(100));
            path.lineTo(0, SkIntToScalar(100));
            path.close();
            REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(qRect));
        }
    }

    // make sure a minimal convex shape works, a right tri with edges along pos x and y axes.
    path.reset();
    path.moveTo(0, 0);
    path.lineTo(SkIntToScalar(100), 0);
    path.lineTo(0, SkIntToScalar(100));

    // inside, on along top edge
    REPORTER_ASSERT(reporter, path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(50), 0,
                                                                               SkIntToScalar(10),
                                                                               SkIntToScalar(10))));
    // above
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(
        SkRect::MakeXYWH(SkIntToScalar(50),
                         SkIntToScalar(-10),
                         SkIntToScalar(10),
                         SkIntToScalar(10))));
    // to the left
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(-10),
                                                                                SkIntToScalar(5),
                                                                                SkIntToScalar(5),
                                                                                SkIntToScalar(5))));

    // outside the diagonal edge
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(10),
                                                                                SkIntToScalar(200),
                                                                                SkIntToScalar(20),
                                                                                SkIntToScalar(5))));

    // same as above path and first test but with an extra moveTo.
    path.reset();
    path.moveTo(100, 100);
    path.moveTo(0, 0);
    path.lineTo(SkIntToScalar(100), 0);
    path.lineTo(0, SkIntToScalar(100));

    REPORTER_ASSERT(reporter, path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(50), 0,
                                                                               SkIntToScalar(10),
                                                                               SkIntToScalar(10))));

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
    SkPoint rf[] = {{1, 0}, {8, 0}, {8, 8}, {0, 8}, {0, 0}};

    // failing tests
    SkPoint f1[] = {{0, 0}, {1, 0}, {1, 1}}; // too few points
    SkPoint f2[] = {{0, 0}, {1, 1}, {0, 1}, {1, 0}}; // diagonal
    SkPoint f3[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}}; // wraps
    SkPoint f4[] = {{0, 0}, {1, 0}, {0, 0}, {1, 0}, {1, 1}, {0, 1}}; // backs up
    SkPoint f5[] = {{0, 0}, {1, 0}, {1, 1}, {2, 0}}; // end overshoots
    SkPoint f6[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 2}}; // end overshoots
    SkPoint f7[] = {{0, 0}, {1, 0}, {1, 1}, {0, 2}}; // end overshoots
    SkPoint f8[] = {{0, 0}, {1, 0}, {1, 1}, {1, 0}}; // 'L'
    SkPoint f9[] = {{1, 0}, {8, 0}, {8, 8}, {0, 8}, {0, 0}, {2, 0}}; // overlaps
    SkPoint fa[] = {{1, 0}, {8, 0}, {8, 8}, {0, 8}, {0, -1}, {1, -1}}; // non colinear gap
    SkPoint fb[] = {{1, 0}, {8, 0}, {8, 8}, {0, 8}, {0, 1}}; // falls short

    // failing, no close
    SkPoint c1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}}; // close doesn't match
    SkPoint c2[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}}; // ditto

    size_t testLen[] = {
        sizeof(r1), sizeof(r2), sizeof(r3), sizeof(r4), sizeof(r5), sizeof(r6),
        sizeof(r7), sizeof(r8), sizeof(r9), sizeof(ra), sizeof(rb), sizeof(rc),
        sizeof(rd), sizeof(re), sizeof(rf),
        sizeof(f1), sizeof(f2), sizeof(f3), sizeof(f4), sizeof(f5), sizeof(f6),
        sizeof(f7), sizeof(f8), sizeof(f9), sizeof(fa), sizeof(fb),
        sizeof(c1), sizeof(c2)
    };
    SkPoint* tests[] = {
        r1, r2, r3, r4, r5, r6, r7, r8, r9, ra, rb, rc, rd, re, rf,
        f1, f2, f3, f4, f5, f6, f7, f8, f9, fa, fb,
        c1, c2
    };
    SkPoint* lastPass = rf;
    SkPoint* lastClose = fb;
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
        REPORTER_ASSERT(reporter, fail ^ path.isRect(NULL, NULL));

        if (!fail) {
            SkRect computed, expected;
            expected.set(tests[testIndex], testLen[testIndex] / sizeof(SkPoint));
            REPORTER_ASSERT(reporter, path.isRect(&computed));
            REPORTER_ASSERT(reporter, expected == computed);

            bool isClosed;
            SkPath::Direction direction, cheapDirection;
            REPORTER_ASSERT(reporter, path.cheapComputeDirection(&cheapDirection));
            REPORTER_ASSERT(reporter, path.isRect(&isClosed, &direction));
            REPORTER_ASSERT(reporter, isClosed == close);
            REPORTER_ASSERT(reporter, direction == cheapDirection);
        } else {
            SkRect computed;
            computed.set(123, 456, 789, 1011);
            REPORTER_ASSERT(reporter, !path.isRect(&computed));
            REPORTER_ASSERT(reporter, computed.fLeft == 123 && computed.fTop == 456);
            REPORTER_ASSERT(reporter, computed.fRight == 789 && computed.fBottom == 1011);

            bool isClosed = (bool) -1;
            SkPath::Direction direction = (SkPath::Direction) -1;
            REPORTER_ASSERT(reporter, !path.isRect(&isClosed, &direction));
            REPORTER_ASSERT(reporter, isClosed == (bool) -1);
            REPORTER_ASSERT(reporter, direction == (SkPath::Direction) -1);
        }

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

static void test_isNestedRects(skiatest::Reporter* reporter) {
    // passing tests (all moveTo / lineTo...
    SkPoint r1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}}; // CW
    SkPoint r2[] = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
    SkPoint r3[] = {{1, 1}, {0, 1}, {0, 0}, {1, 0}};
    SkPoint r4[] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
    SkPoint r5[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}}; // CCW
    SkPoint r6[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    SkPoint r7[] = {{1, 1}, {1, 0}, {0, 0}, {0, 1}};
    SkPoint r8[] = {{1, 0}, {0, 0}, {0, 1}, {1, 1}};
    SkPoint r9[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
    SkPoint ra[] = {{0, 0}, {0, .5f}, {0, 1}, {.5f, 1}, {1, 1}, {1, .5f}, // CCW
        {1, 0}, {.5f, 0}};
    SkPoint rb[] = {{0, 0}, {.5f, 0}, {1, 0}, {1, .5f}, {1, 1}, {.5f, 1}, // CW
        {0, 1}, {0, .5f}};
    SkPoint rc[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}}; // CW
    SkPoint rd[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}}; // CCW
    SkPoint re[] = {{0, 0}, {1, 0}, {1, 0}, {1, 1}, {0, 1}}; // CW

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
    SkPath::Direction dirs[] = {
        SkPath::kCW_Direction, SkPath::kCW_Direction, SkPath::kCW_Direction,
        SkPath::kCW_Direction, SkPath::kCCW_Direction, SkPath::kCCW_Direction,
        SkPath::kCCW_Direction, SkPath::kCCW_Direction, SkPath::kCCW_Direction,
        SkPath::kCCW_Direction, SkPath::kCW_Direction, SkPath::kCW_Direction,
        SkPath::kCCW_Direction, SkPath::kCW_Direction, SkPath::kUnknown_Direction,
        SkPath::kUnknown_Direction, SkPath::kUnknown_Direction, SkPath::kUnknown_Direction,
        SkPath::kUnknown_Direction, SkPath::kUnknown_Direction, SkPath::kUnknown_Direction,
        SkPath::kUnknown_Direction, SkPath::kUnknown_Direction, SkPath::kUnknown_Direction,
    };
    SkASSERT(SK_ARRAY_COUNT(tests) == SK_ARRAY_COUNT(dirs));

    const SkPoint* lastPass = re;
    const SkPoint* lastClose = f8;
    const size_t testCount = sizeof(tests) / sizeof(tests[0]);
    size_t index;
    for (int rectFirst = 0; rectFirst <= 1; ++rectFirst) {
        bool fail = false;
        bool close = true;
        for (size_t testIndex = 0; testIndex < testCount; ++testIndex) {
            SkPath path;
            if (rectFirst) {
                path.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
            }
            path.moveTo(tests[testIndex][0].fX, tests[testIndex][0].fY);
            for (index = 1; index < testLen[testIndex] / sizeof(SkPoint); ++index) {
                path.lineTo(tests[testIndex][index].fX, tests[testIndex][index].fY);
            }
            if (close) {
                path.close();
            }
            if (!rectFirst) {
                path.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
            }
            REPORTER_ASSERT(reporter, fail ^ path.isNestedRects(0));
            if (!fail) {
                SkRect expected[2], computed[2];
                SkPath::Direction expectedDirs[2], computedDirs[2];
                SkRect testBounds;
                testBounds.set(tests[testIndex], testLen[testIndex] / sizeof(SkPoint));
                expected[0] = SkRect::MakeLTRB(-1, -1, 2, 2);
                expected[1] = testBounds;
                if (rectFirst) {
                    expectedDirs[0] = SkPath::kCW_Direction;
                } else {
                    expectedDirs[0] = SkPath::kCCW_Direction;
                }
                expectedDirs[1] = dirs[testIndex];
                REPORTER_ASSERT(reporter, path.isNestedRects(computed, computedDirs));
                REPORTER_ASSERT(reporter, expected[0] == computed[0]);
                REPORTER_ASSERT(reporter, expected[1] == computed[1]);
                REPORTER_ASSERT(reporter, expectedDirs[0] == computedDirs[0]);
                REPORTER_ASSERT(reporter, expectedDirs[1] == computedDirs[1]);
            }
            if (tests[testIndex] == lastPass) {
                fail = true;
            }
            if (tests[testIndex] == lastClose) {
                close = false;
            }
        }

        // fail, close then line
        SkPath path1;
        if (rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
        }
        path1.moveTo(r1[0].fX, r1[0].fY);
        for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
            path1.lineTo(r1[index].fX, r1[index].fY);
        }
        path1.close();
        path1.lineTo(1, 0);
        if (!rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
        }
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));

        // fail, move in the middle
        path1.reset();
        if (rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
        }
        path1.moveTo(r1[0].fX, r1[0].fY);
        for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
            if (index == 2) {
                path1.moveTo(1, .5f);
            }
            path1.lineTo(r1[index].fX, r1[index].fY);
        }
        path1.close();
        if (!rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
        }
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));

        // fail, move on the edge
        path1.reset();
        if (rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
        }
        for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
            path1.moveTo(r1[index - 1].fX, r1[index - 1].fY);
            path1.lineTo(r1[index].fX, r1[index].fY);
        }
        path1.close();
        if (!rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
        }
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));

        // fail, quad
        path1.reset();
        if (rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
        }
        path1.moveTo(r1[0].fX, r1[0].fY);
        for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
            if (index == 2) {
                path1.quadTo(1, .5f, 1, .5f);
            }
            path1.lineTo(r1[index].fX, r1[index].fY);
        }
        path1.close();
        if (!rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
        }
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));

        // fail, cubic
        path1.reset();
        if (rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCW_Direction);
        }
        path1.moveTo(r1[0].fX, r1[0].fY);
        for (index = 1; index < testLen[0] / sizeof(SkPoint); ++index) {
            if (index == 2) {
                path1.cubicTo(1, .5f, 1, .5f, 1, .5f);
            }
            path1.lineTo(r1[index].fX, r1[index].fY);
        }
        path1.close();
        if (!rectFirst) {
            path1.addRect(-1, -1, 2, 2, SkPath::kCCW_Direction);
        }
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));

        // fail,  not nested
        path1.reset();
        path1.addRect(1, 1, 3, 3, SkPath::kCW_Direction);
        path1.addRect(2, 2, 4, 4, SkPath::kCW_Direction);
        REPORTER_ASSERT(reporter, fail ^ path1.isNestedRects(0));
    }

    // pass, stroke rect
    SkPath src, dst;
    src.addRect(1, 1, 7, 7, SkPath::kCW_Direction);
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(2);
    strokePaint.getFillPath(src, &dst);
    REPORTER_ASSERT(reporter, dst.isNestedRects(0));
}

static void write_and_read_back(skiatest::Reporter* reporter,
                                const SkPath& p) {
    SkWriter32 writer(100);
    writer.writePath(p);
    size_t size = writer.size();
    SkAutoMalloc storage(size);
    writer.flatten(storage.get());
    SkReader32 reader(storage.get(), size);

    SkPath readBack;
    REPORTER_ASSERT(reporter, readBack != p);
    reader.readPath(&readBack);
    REPORTER_ASSERT(reporter, readBack == p);

    REPORTER_ASSERT(reporter, readBack.getConvexityOrUnknown() ==
                              p.getConvexityOrUnknown());

    REPORTER_ASSERT(reporter, readBack.isOval(NULL) == p.isOval(NULL));

    const SkRect& origBounds = p.getBounds();
    const SkRect& readBackBounds = readBack.getBounds();

    REPORTER_ASSERT(reporter, origBounds == readBackBounds);
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

    write_and_read_back(reporter, p);

    // create a buffer that should be much larger than the path so we don't
    // kill our stack if writer goes too far.
    char buffer[1024];
    uint32_t size1 = p.writeToMemory(NULL);
    uint32_t size2 = p.writeToMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);

    SkPath p2;
    uint32_t size3 = p2.readFromMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, p == p2);

    char buffer2[1024];
    size3 = p2.writeToMemory(buffer2);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);

    // test persistence of the oval flag & convexity
    {
        SkPath oval;
        SkRect rect = SkRect::MakeWH(10, 10);
        oval.addOval(rect);

        write_and_read_back(reporter, oval);
    }
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
    uint8_t verbs[32];

    struct SUPPRESS_VISIBILITY_WARNING zeroPathTestData {
        const char* testPath;
        const size_t numResultPts;
        const SkRect resultBound;
        const SkPath::Verb* resultVerbs;
        const size_t numResultVerbs;
    };

    static const SkPath::Verb resultVerbs1[] = { SkPath::kMove_Verb };
    static const SkPath::Verb resultVerbs2[] = { SkPath::kMove_Verb, SkPath::kMove_Verb };
    static const SkPath::Verb resultVerbs3[] = { SkPath::kMove_Verb, SkPath::kClose_Verb };
    static const SkPath::Verb resultVerbs4[] = { SkPath::kMove_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kClose_Verb };
    static const SkPath::Verb resultVerbs5[] = { SkPath::kMove_Verb, SkPath::kLine_Verb };
    static const SkPath::Verb resultVerbs6[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb, SkPath::kLine_Verb };
    static const SkPath::Verb resultVerbs7[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb };
    static const SkPath::Verb resultVerbs8[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb
    };
    static const SkPath::Verb resultVerbs9[] = { SkPath::kMove_Verb, SkPath::kQuad_Verb };
    static const SkPath::Verb resultVerbs10[] = { SkPath::kMove_Verb, SkPath::kQuad_Verb, SkPath::kMove_Verb, SkPath::kQuad_Verb };
    static const SkPath::Verb resultVerbs11[] = { SkPath::kMove_Verb, SkPath::kQuad_Verb, SkPath::kClose_Verb };
    static const SkPath::Verb resultVerbs12[] = {
        SkPath::kMove_Verb, SkPath::kQuad_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kQuad_Verb, SkPath::kClose_Verb
    };
    static const SkPath::Verb resultVerbs13[] = { SkPath::kMove_Verb, SkPath::kCubic_Verb };
    static const SkPath::Verb resultVerbs14[] = { SkPath::kMove_Verb, SkPath::kCubic_Verb, SkPath::kMove_Verb, SkPath::kCubic_Verb };
    static const SkPath::Verb resultVerbs15[] = { SkPath::kMove_Verb, SkPath::kCubic_Verb, SkPath::kClose_Verb };
    static const SkPath::Verb resultVerbs16[] = {
        SkPath::kMove_Verb, SkPath::kCubic_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kCubic_Verb, SkPath::kClose_Verb
    };
    static const struct zeroPathTestData gZeroLengthTests[] = {
        { "M 1 1", 1, {0, 0, 0, 0}, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 1 M 2 1", 2, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs2, SK_ARRAY_COUNT(resultVerbs2) },
        { "M 1 1 z", 1, {0, 0, 0, 0}, resultVerbs3, SK_ARRAY_COUNT(resultVerbs3) },
        { "M 1 1 z M 2 1 z", 2, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs4, SK_ARRAY_COUNT(resultVerbs4) },
        { "M 1 1 L 1 1", 2, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs5, SK_ARRAY_COUNT(resultVerbs5) },
        { "M 1 1 L 1 1 M 2 1 L 2 1", 4, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs6, SK_ARRAY_COUNT(resultVerbs6) },
        { "M 1 1 L 1 1 z", 2, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs7, SK_ARRAY_COUNT(resultVerbs7) },
        { "M 1 1 L 1 1 z M 2 1 L 2 1 z", 4, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs8, SK_ARRAY_COUNT(resultVerbs8) },
        { "M 1 1 Q 1 1 1 1", 3, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs9, SK_ARRAY_COUNT(resultVerbs9) },
        { "M 1 1 Q 1 1 1 1 M 2 1 Q 2 1 2 1", 6, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs10, SK_ARRAY_COUNT(resultVerbs10) },
        { "M 1 1 Q 1 1 1 1 z", 3, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs11, SK_ARRAY_COUNT(resultVerbs11) },
        { "M 1 1 Q 1 1 1 1 z M 2 1 Q 2 1 2 1 z", 6, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs12, SK_ARRAY_COUNT(resultVerbs12) },
        { "M 1 1 C 1 1 1 1 1 1", 4, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs13, SK_ARRAY_COUNT(resultVerbs13) },
        { "M 1 1 C 1 1 1 1 1 1 M 2 1 C 2 1 2 1 2 1", 8, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs14,
            SK_ARRAY_COUNT(resultVerbs14)
        },
        { "M 1 1 C 1 1 1 1 1 1 z", 4, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs15, SK_ARRAY_COUNT(resultVerbs15) },
        { "M 1 1 C 1 1 1 1 1 1 z M 2 1 C 2 1 2 1 2 1 z", 8, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs16,
            SK_ARRAY_COUNT(resultVerbs16)
        }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gZeroLengthTests); ++i) {
        p.reset();
        bool valid = SkParsePath::FromSVGString(gZeroLengthTests[i].testPath, &p);
        REPORTER_ASSERT(reporter, valid);
        REPORTER_ASSERT(reporter, !p.isEmpty());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].numResultPts == (size_t)p.countPoints());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].resultBound == p.getBounds());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].numResultVerbs == (size_t)p.getVerbs(verbs, SK_ARRAY_COUNT(verbs)));
        for (size_t j = 0; j < gZeroLengthTests[i].numResultVerbs; ++j) {
            REPORTER_ASSERT(reporter, gZeroLengthTests[i].resultVerbs[j] == verbs[j]);
        }
    }
}

struct SegmentInfo {
    SkPath fPath;
    int    fPointCount;
};

#define kCurveSegmentMask   (SkPath::kQuad_SegmentMask | SkPath::kCubic_SegmentMask)

static void test_segment_masks(skiatest::Reporter* reporter) {
    SkPath p, p2;

    p.moveTo(0, 0);
    p.quadTo(100, 100, 200, 200);
    REPORTER_ASSERT(reporter, SkPath::kQuad_SegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());
    p2 = p;
    REPORTER_ASSERT(reporter, p2.getSegmentMasks() == p.getSegmentMasks());
    p.cubicTo(100, 100, 200, 200, 300, 300);
    REPORTER_ASSERT(reporter, kCurveSegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());
    p2 = p;
    REPORTER_ASSERT(reporter, p2.getSegmentMasks() == p.getSegmentMasks());

    p.reset();
    p.moveTo(0, 0);
    p.cubicTo(100, 100, 200, 200, 300, 300);
    REPORTER_ASSERT(reporter, SkPath::kCubic_SegmentMask == p.getSegmentMasks());
    p2 = p;
    REPORTER_ASSERT(reporter, p2.getSegmentMasks() == p.getSegmentMasks());

    REPORTER_ASSERT(reporter, !p.isEmpty());
}

static void test_iter(skiatest::Reporter* reporter) {
    SkPath  p;
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
    iter.setPath(p, true);
    REPORTER_ASSERT(reporter, iter.next(pts) == SkPath::kDone_Verb);


    struct iterTestData {
        const char* testPath;
        const bool forceClose;
        const bool consumeDegenerates;
        const size_t* numResultPtsPerVerb;
        const SkPoint* resultPts;
        const SkPath::Verb* resultVerbs;
        const size_t numResultVerbs;
    };

    static const SkPath::Verb resultVerbs1[] = { SkPath::kDone_Verb };
    static const SkPath::Verb resultVerbs2[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kDone_Verb
    };
    static const SkPath::Verb resultVerbs3[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb, SkPath::kDone_Verb
    };
    static const SkPath::Verb resultVerbs4[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb, SkPath::kClose_Verb, SkPath::kDone_Verb
    };
    static const SkPath::Verb resultVerbs5[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kClose_Verb, SkPath::kDone_Verb
    };
    static const size_t resultPtsSizes1[] = { 0 };
    static const size_t resultPtsSizes2[] = { 1, 2, 2, 0 };
    static const size_t resultPtsSizes3[] = { 1, 2, 2, 2, 1, 0 };
    static const size_t resultPtsSizes4[] = { 1, 2, 1, 1, 0 };
    static const size_t resultPtsSizes5[] = { 1, 2, 1, 1, 1, 0 };
    static const SkPoint* resultPts1 = 0;
    static const SkPoint resultPts2[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, SK_Scalar1 }, { SK_Scalar1, SK_Scalar1 }, { 0, SK_Scalar1 }
    };
    static const SkPoint resultPts3[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, SK_Scalar1 }, { SK_Scalar1, SK_Scalar1 }, { 0, SK_Scalar1 },
        { 0, SK_Scalar1 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }
    };
    static const SkPoint resultPts4[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { 0, 0 }, { 0, 0 }
    };
    static const SkPoint resultPts5[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { 0, 0 }, { 0, 0 }
    };
    static const struct iterTestData gIterTests[] = {
        { "M 1 0", false, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 0 M 2 0 M 3 0 M 4 0 M 5 0", false, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 0 M 1 0 M 3 0 M 4 0 M 5 0", true, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "z", false, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "z", true, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "z M 1 0 z z M 2 0 z M 3 0 M 4 0 z", false, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "z M 1 0 z z M 2 0 z M 3 0 M 4 0 z", true, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 0 L 1 1 L 0 1 M 0 0 z", false, true, resultPtsSizes2, resultPts2, resultVerbs2, SK_ARRAY_COUNT(resultVerbs2) },
        { "M 1 0 L 1 1 L 0 1 M 0 0 z", true, true, resultPtsSizes3, resultPts3, resultVerbs3, SK_ARRAY_COUNT(resultVerbs3) },
        { "M 1 0 L 1 0 M 0 0 z", false, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 0 L 1 0 M 0 0 z", true, true, resultPtsSizes1, resultPts1, resultVerbs1, SK_ARRAY_COUNT(resultVerbs1) },
        { "M 1 0 L 1 0 M 0 0 z", false, false, resultPtsSizes4, resultPts4, resultVerbs4, SK_ARRAY_COUNT(resultVerbs4) },
        { "M 1 0 L 1 0 M 0 0 z", true, false, resultPtsSizes5, resultPts5, resultVerbs5, SK_ARRAY_COUNT(resultVerbs5) }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gIterTests); ++i) {
        p.reset();
        bool valid = SkParsePath::FromSVGString(gIterTests[i].testPath, &p);
        REPORTER_ASSERT(reporter, valid);
        iter.setPath(p, gIterTests[i].forceClose);
        int j = 0, l = 0;
        do {
            REPORTER_ASSERT(reporter, iter.next(pts, gIterTests[i].consumeDegenerates) == gIterTests[i].resultVerbs[j]);
            for (int k = 0; k < (int)gIterTests[i].numResultPtsPerVerb[j]; ++k) {
                REPORTER_ASSERT(reporter, pts[k] == gIterTests[i].resultPts[l++]);
            }
        } while (gIterTests[i].resultVerbs[j++] != SkPath::kDone_Verb);
        REPORTER_ASSERT(reporter, j == (int)gIterTests[i].numResultVerbs);
    }

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
    SkMWCRandom rand(9876543);
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
                case SkPath::kConic_Verb:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPath::kMove_Verb;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    p.conicTo(expectedPts[numPoints], expectedPts[numPoints + 1],
                              rand.nextUScalar1() * 4);
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
                default:
                    SkASSERT(!"unexpected verb");
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
                case SkPath::kConic_Verb:
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
                default:
                    SkASSERT(!"unexpected verb");
            }
        }
        REPORTER_ASSERT(reporter, numIterPts == numPoints);
        REPORTER_ASSERT(reporter, numIterVerbs == numVerbs);
    }
}

static void check_for_circle(skiatest::Reporter* reporter,
                             const SkPath& path,
                             bool expectedCircle,
                             SkPath::Direction expectedDir) {
    SkRect rect;
    REPORTER_ASSERT(reporter, path.isOval(&rect) == expectedCircle);
    REPORTER_ASSERT(reporter, path.cheapIsDirection(expectedDir));

    if (expectedCircle) {
        REPORTER_ASSERT(reporter, rect.height() == rect.width());
    }
}

static void test_circle_skew(skiatest::Reporter* reporter,
                             const SkPath& path,
                             SkPath::Direction dir) {
    SkPath tmp;

    SkMatrix m;
    m.setSkew(SkIntToScalar(3), SkIntToScalar(5));
    path.transform(m, &tmp);
    // this matrix reverses the direction.
    if (SkPath::kCCW_Direction == dir) {
        dir = SkPath::kCW_Direction;
    } else {
        SkASSERT(SkPath::kCW_Direction == dir);
        dir = SkPath::kCCW_Direction;
    }
    check_for_circle(reporter, tmp, false, dir);
}

static void test_circle_translate(skiatest::Reporter* reporter,
                                  const SkPath& path,
                                  SkPath::Direction dir) {
    SkPath tmp;

    // translate at small offset
    SkMatrix m;
    m.setTranslate(SkIntToScalar(15), SkIntToScalar(15));
    path.transform(m, &tmp);
    check_for_circle(reporter, tmp, true, dir);

    tmp.reset();
    m.reset();

    // translate at a relatively big offset
    m.setTranslate(SkIntToScalar(1000), SkIntToScalar(1000));
    path.transform(m, &tmp);
    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_rotate(skiatest::Reporter* reporter,
                               const SkPath& path,
                               SkPath::Direction dir) {
    for (int angle = 0; angle < 360; ++angle) {
        SkPath tmp;
        SkMatrix m;
        m.setRotate(SkIntToScalar(angle));
        path.transform(m, &tmp);

        // TODO: a rotated circle whose rotated angle is not a multiple of 90
        // degrees is not an oval anymore, this can be improved.  we made this
        // for the simplicity of our implementation.
        if (angle % 90 == 0) {
            check_for_circle(reporter, tmp, true, dir);
        } else {
            check_for_circle(reporter, tmp, false, dir);
        }
    }
}

static void test_circle_mirror_x(skiatest::Reporter* reporter,
                                 const SkPath& path,
                                 SkPath::Direction dir) {
    SkPath tmp;
    SkMatrix m;
    m.reset();
    m.setScaleX(-SK_Scalar1);
    path.transform(m, &tmp);

    if (SkPath::kCW_Direction == dir) {
        dir = SkPath::kCCW_Direction;
    } else {
        SkASSERT(SkPath::kCCW_Direction == dir);
        dir = SkPath::kCW_Direction;
    }

    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_mirror_y(skiatest::Reporter* reporter,
                                 const SkPath& path,
                                 SkPath::Direction dir) {
    SkPath tmp;
    SkMatrix m;
    m.reset();
    m.setScaleY(-SK_Scalar1);
    path.transform(m, &tmp);

    if (SkPath::kCW_Direction == dir) {
        dir = SkPath::kCCW_Direction;
    } else {
        SkASSERT(SkPath::kCCW_Direction == dir);
        dir = SkPath::kCW_Direction;
    }

    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_mirror_xy(skiatest::Reporter* reporter,
                                 const SkPath& path,
                                 SkPath::Direction dir) {
    SkPath tmp;
    SkMatrix m;
    m.reset();
    m.setScaleX(-SK_Scalar1);
    m.setScaleY(-SK_Scalar1);
    path.transform(m, &tmp);

    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_with_direction(skiatest::Reporter* reporter,
                                       SkPath::Direction dir) {
    SkPath path;

    // circle at origin
    path.addCircle(0, 0, SkIntToScalar(20), dir);
    check_for_circle(reporter, path, true, dir);
    test_circle_rotate(reporter, path, dir);
    test_circle_translate(reporter, path, dir);
    test_circle_skew(reporter, path, dir);

    // circle at an offset at (10, 10)
    path.reset();
    path.addCircle(SkIntToScalar(10), SkIntToScalar(10),
                   SkIntToScalar(20), dir);
    check_for_circle(reporter, path, true, dir);
    test_circle_rotate(reporter, path, dir);
    test_circle_translate(reporter, path, dir);
    test_circle_skew(reporter, path, dir);
    test_circle_mirror_x(reporter, path, dir);
    test_circle_mirror_y(reporter, path, dir);
    test_circle_mirror_xy(reporter, path, dir);
}

static void test_circle_with_add_paths(skiatest::Reporter* reporter) {
    SkPath path;
    SkPath circle;
    SkPath rect;
    SkPath empty;

    static const SkPath::Direction kCircleDir = SkPath::kCW_Direction;
    static const SkPath::Direction kCircleDirOpposite = SkPath::kCCW_Direction;

    circle.addCircle(0, 0, SkIntToScalar(10), kCircleDir);
    rect.addRect(SkIntToScalar(5), SkIntToScalar(5),
                 SkIntToScalar(20), SkIntToScalar(20), SkPath::kCW_Direction);

    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(12), SkIntToScalar(12));

    // For simplicity, all the path concatenation related operations
    // would mark it non-circle, though in theory it's still a circle.

    // empty + circle (translate)
    path = empty;
    path.addPath(circle, translate);
    check_for_circle(reporter, path, false, kCircleDir);

    // circle + empty (translate)
    path = circle;
    path.addPath(empty, translate);
    check_for_circle(reporter, path, false, kCircleDir);

    // test reverseAddPath
    path = circle;
    path.reverseAddPath(rect);
    check_for_circle(reporter, path, false, kCircleDirOpposite);
}

static void test_circle(skiatest::Reporter* reporter) {
    test_circle_with_direction(reporter, SkPath::kCW_Direction);
    test_circle_with_direction(reporter, SkPath::kCCW_Direction);

    // multiple addCircle()
    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.addCircle(0, 0, SkIntToScalar(20), SkPath::kCW_Direction);
    check_for_circle(reporter, path, false, SkPath::kCW_Direction);

    // some extra lineTo() would make isOval() fail
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.lineTo(0, 0);
    check_for_circle(reporter, path, false, SkPath::kCW_Direction);

    // not back to the original point
    path.reset();
    path.addCircle(0, 0, SkIntToScalar(10), SkPath::kCW_Direction);
    path.setLastPt(SkIntToScalar(5), SkIntToScalar(5));
    check_for_circle(reporter, path, false, SkPath::kCW_Direction);

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

static void test_empty(skiatest::Reporter* reporter, const SkPath& p) {
    SkPath  empty;

    REPORTER_ASSERT(reporter, p.isEmpty());
    REPORTER_ASSERT(reporter, 0 == p.countPoints());
    REPORTER_ASSERT(reporter, 0 == p.countVerbs());
    REPORTER_ASSERT(reporter, 0 == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == empty);
    REPORTER_ASSERT(reporter, !(p != empty));
}

static void TestPath(skiatest::Reporter* reporter) {
    SkTSize<SkScalar>::Make(3,4);

    SkPath  p, empty;
    SkRect  bounds, bounds2;
    test_empty(reporter, p);

    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());

    bounds.set(0, 0, SK_Scalar1, SK_Scalar1);

    p.addRoundRect(bounds, SK_Scalar1, SK_Scalar1);
    check_convex_bounds(reporter, p, bounds);
    // we have quads or cubics
    REPORTER_ASSERT(reporter, p.getSegmentMasks() & kCurveSegmentMask);
    REPORTER_ASSERT(reporter, !p.isEmpty());

    p.reset();
    test_empty(reporter, p);

    p.addOval(bounds);
    check_convex_bounds(reporter, p, bounds);
    REPORTER_ASSERT(reporter, !p.isEmpty());

    p.rewind();
    test_empty(reporter, p);

    p.addRect(bounds);
    check_convex_bounds(reporter, p, bounds);
    // we have only lines
    REPORTER_ASSERT(reporter, SkPath::kLine_SegmentMask == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, !p.isEmpty());

    REPORTER_ASSERT(reporter, p != empty);
    REPORTER_ASSERT(reporter, !(p == empty));

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
    test_isNestedRects(reporter);
    test_zero_length_paths(reporter);
    test_direction(reporter);
    test_convexity(reporter);
    test_convexity2(reporter);
    test_conservativelyContains(reporter);
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
    test_isfinite(reporter);
    test_isfinite_after_transform(reporter);
    test_arb_round_rect_is_convex(reporter);
    test_arb_zero_rad_round_rect_is_rect(reporter);
    test_addrect_isfinite(reporter);
    test_tricky_cubic();
    test_clipped_cubic();
    test_crbug_170666();
    test_bad_cubic_crbug229478();
    test_bad_cubic_crbug234190();
    test_android_specific_behavior(reporter);
    test_path_close_issue1474(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Path", PathTestClass, TestPath)
