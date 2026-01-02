/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkNullCanvas.h"
#include "include/utils/SkParse.h"
#include "include/utils/SkParsePath.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <vector>

static void set_radii(SkVector radii[4], int index, float rad) {
    sk_bzero(radii, sizeof(SkVector) * 4);
    radii[index].set(rad, rad);
}

static void test_add_rrect(skiatest::Reporter* reporter, const SkRect& bounds,
                           const SkVector radii[4]) {
    SkRRect rrect;
    rrect.setRectRadii(bounds, radii);
    REPORTER_ASSERT(reporter, bounds == rrect.rect());

    // this line should not assert in the debug build (from validate)
    SkPath path = SkPath::RRect(rrect);
    REPORTER_ASSERT(reporter, bounds == path.getBounds());
}

static void test_skbug_3469(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(20, 20)
                  .quadTo(20, 50, 80, 50)
                  .quadTo(20, 50, 20, 80)
                  .detach();
    REPORTER_ASSERT(reporter, !path.isConvex());
}

static void test_skbug_3239(skiatest::Reporter* reporter) {
    const float min = SkBits2Float(0xcb7f16c8); /* -16717512.000000 */
    const float max = SkBits2Float(0x4b7f1c1d); /*  16718877.000000 */
    const float big = SkBits2Float(0x4b7f1bd7); /*  16718807.000000 */

    const float rad = 33436320;

    const SkRect rectx = SkRect::MakeLTRB(min, min, max, big);
    const SkRect recty = SkRect::MakeLTRB(min, min, big, max);

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        set_radii(radii, i, rad);
        test_add_rrect(reporter, rectx, radii);
        test_add_rrect(reporter, recty, radii);
    }
}

static SkPath make_path_crbug364224() {
    return SkPathBuilder()
           .moveTo(3.747501373f, 2.724499941f)
           .lineTo(3.747501373f, 3.75f)
           .cubicTo(3.747501373f, 3.88774991f, 3.635501385f, 4.0f, 3.497501373f, 4.0f)
           .lineTo(0.7475013733f, 4.0f)
           .cubicTo(0.6095013618f, 4.0f, 0.4975013733f, 3.88774991f, 0.4975013733f, 3.75f)
           .lineTo(0.4975013733f, 1.0f)
           .cubicTo(0.4975013733f, 0.8622499704f, 0.6095013618f, 0.75f, 0.7475013733f,0.75f)
           .lineTo(3.497501373f, 0.75f)
           .cubicTo(3.50275135f, 0.75f, 3.5070014f, 0.7527500391f, 3.513001442f, 0.753000021f)
           .lineTo(3.715001345f, 0.5512499809f)
           .cubicTo(3.648251295f, 0.5194999576f, 3.575501442f, 0.4999999702f, 3.497501373f, 0.4999999702f)
           .lineTo(0.7475013733f, 0.4999999702f)
           .cubicTo(0.4715013802f, 0.4999999702f, 0.2475013733f, 0.7239999771f, 0.2475013733f, 1.0f)
           .lineTo(0.2475013733f, 3.75f)
           .cubicTo(0.2475013733f, 4.026000023f, 0.4715013504f, 4.25f, 0.7475013733f, 4.25f)
           .lineTo(3.497501373f, 4.25f)
           .cubicTo(3.773501396f, 4.25f, 3.997501373f, 4.026000023f, 3.997501373f, 3.75f)
           .lineTo(3.997501373f, 2.474750042f)
           .lineTo(3.747501373f, 2.724499941f)
           .close()
           .detach();
}

static SkPath make_path_crbug364224_simplified() {
    return SkPathBuilder()
           .moveTo(3.747501373f, 2.724499941f)
           .cubicTo(3.648251295f, 0.5194999576f, 3.575501442f, 0.4999999702f, 3.497501373f, 0.4999999702f)
           .close()
           .detach();
}

static void test_sect_with_horizontal_needs_pinning() {
    // Test that sect_with_horizontal in SkLineClipper.cpp needs to pin after computing the
    // intersection.
    SkPath path = SkPathBuilder()
                  .moveTo(-540000, -720000)
                  .lineTo(-9.10000017e-05f, 9.99999996e-13f)
                  .lineTo(1, 1)
                  .detach();

    // Without the pinning code in sect_with_horizontal(), this would assert in the lineclipper
    SkPaint paint;
    SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10))->getCanvas()->drawPath(path, paint);
}

static void test_iterative_intersect_line() {
    // crbug.com/1320467
    // SkLineClipper::IntersectLine used to clip against the horizontal segment. Then, if it still
    // needed clipping, would clip against the vertical segment, but start over from the un-clipped
    // endpoints. With that version, this draw would trigger an assert.
    // With the fix (iteratively clipping the intermediate results after the first operation),
    // this shouldn't assert:
    SkPath path = SkPathBuilder()
                  .moveTo(-478.805145f, 153.862549f)
                  .lineTo(6.27216804e+19f, 6.27216804e+19f)
                  .lineTo(-666.754272f, 155.086304f)
                  .close()
                  .detach();

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    SkSurfaces::Raster(SkImageInfo::MakeN32Premul(256, 256))->getCanvas()->drawPath(path, paint);
}

static void test_path_crbug364224() {
    SkPaint paint;
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(84, 88)));
    SkCanvas* canvas = surface->getCanvas();

    SkPath path = make_path_crbug364224_simplified();
    canvas->drawPath(path, paint);

    path = make_path_crbug364224();
    canvas->drawPath(path, paint);
}

static void test_draw_AA_path(int width, int height, const SkPath& path) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
}

// this is a unit test instead of a GM because it doesn't draw anything
static void test_fuzz_crbug_638223() {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x47452a00), SkBits2Float(0x43211d01))  // 50474, 161.113f
                  .conicTo(SkBits2Float(0x401c0000), SkBits2Float(0x40680000),
                           SkBits2Float(0x02c25a81), SkBits2Float(0x981a1fa0),
                           SkBits2Float(0x6bf9abea))  // 2.4375f, 3.625f, 2.85577e-37f, -1.992e-24f, 6.03669e+26f
                  .detach();
    test_draw_AA_path(250, 250, path);
}

static void test_fuzz_crbug_643933() {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .conicTo(SkBits2Float(0x002001f2), SkBits2Float(0x4161ffff),  // 2.93943e-39f, 14.125f
                           SkBits2Float(0x49f7224d), SkBits2Float(0x45eec8df), // 2.02452e+06f, 7641.11f
                           SkBits2Float(0x721aee0c))  // 3.0687e+30f
                  .detach();
    test_draw_AA_path(250, 250, path);
    path = SkPathBuilder()
           .moveTo(0, 0)
           .conicTo(SkBits2Float(0x00007ff2), SkBits2Float(0x4169ffff),  // 4.58981e-41f, 14.625f
                    SkBits2Float(0x43ff2261), SkBits2Float(0x41eeea04),  // 510.269f, 29.8643f
                    SkBits2Float(0x5d06eff8))  // 6.07704e+17f
           .detach();
    test_draw_AA_path(250, 250, path);
}

static void test_fuzz_crbug_647922() {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .conicTo(SkBits2Float(0x00003939), SkBits2Float(0x42487fff),  // 2.05276e-41f, 50.125f
                           SkBits2Float(0x48082361), SkBits2Float(0x4408e8e9),  // 139406, 547.639f
                           SkBits2Float(0x4d1ade0f))  // 1.6239e+08f
                  .detach();
    test_draw_AA_path(250, 250, path);
}

static void test_fuzz_crbug_662780() {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(250, 250)));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x41000000), SkBits2Float(0x431e0000))  // 8, 158
                  .lineTo(SkBits2Float(0x41000000), SkBits2Float(0x42f00000))  // 8, 120
                    // 8, 8, 8.00002f, 8, 0.707107f
                  .conicTo(SkBits2Float(0x41000000), SkBits2Float(0x41000000),
                           SkBits2Float(0x41000010), SkBits2Float(0x41000000), SkBits2Float(0x3f3504f3))
                  .lineTo(SkBits2Float(0x439a0000), SkBits2Float(0x41000000))  // 308, 8
                    // 308, 8, 308, 8, 0.707107f
                  .conicTo(SkBits2Float(0x439a0000), SkBits2Float(0x41000000),
                           SkBits2Float(0x439a0000), SkBits2Float(0x41000000), SkBits2Float(0x3f3504f3))
                  .lineTo(SkBits2Float(0x439a0000), SkBits2Float(0x431e0000))  // 308, 158
                    // 308, 158, 308, 158, 0.707107f
                  .conicTo(SkBits2Float(0x439a0000), SkBits2Float(0x431e0000),
                           SkBits2Float(0x439a0000), SkBits2Float(0x431e0000), SkBits2Float(0x3f3504f3))
                  .lineTo(SkBits2Float(0x41000000), SkBits2Float(0x431e0000))  // 8, 158
                    // 8, 158, 8, 158, 0.707107f
                  .conicTo(SkBits2Float(0x41000000), SkBits2Float(0x431e0000),
                           SkBits2Float(0x41000000), SkBits2Float(0x431e0000), SkBits2Float(0x3f3504f3))
                  .close()
                  .detach();
    canvas->clipPath(path, true);
    canvas->drawRect(SkRect::MakeWH(250, 250), paint);
}

static void test_mask_overflow() {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x43e28000), SkBits2Float(0x43aa8000))  // 453, 341
                  .lineTo(SkBits2Float(0x43de6000), SkBits2Float(0x43aa8000))  // 444.75f, 341
                    // 440.47f, 341, 437, 344.47f, 437, 348.75f
                  .cubicTo(SkBits2Float(0x43dc3c29), SkBits2Float(0x43aa8000),
                           SkBits2Float(0x43da8000), SkBits2Float(0x43ac3c29),
                           SkBits2Float(0x43da8000), SkBits2Float(0x43ae6000))
                  .lineTo(SkBits2Float(0x43da8000), SkBits2Float(0x43b18000))  // 437, 355
                  .lineTo(SkBits2Float(0x43e28000), SkBits2Float(0x43b18000))  // 453, 355
                  .lineTo(SkBits2Float(0x43e28000), SkBits2Float(0x43aa8000))  // 453, 341
                  .detach();
    test_draw_AA_path(500, 500, path);
}

static void test_fuzz_crbug_668907() {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x46313741), SkBits2Float(0x3b00e540))  // 11341.8f, 0.00196679f
                  .quadTo(SkBits2Float(0x41410041), SkBits2Float(0xc1414141), SkBits2Float(0x41414141),
                          SkBits2Float(0x414100ff))  // 12.0626f, -12.0784f, 12.0784f, 12.0627f
                  .lineTo(SkBits2Float(0x46313741), SkBits2Float(0x3b00e540))  // 11341.8f, 0.00196679f
                  .close()
                  .detach();
    test_draw_AA_path(400, 500, path);
}

/**
 * In debug mode, this path was causing an assertion to fail in
 * SkPathStroker::preJoinTo() and, in Release, the use of an unitialized value.
 */
static SkPath make_path_crbugskia2820() {
    SkPoint orig, p1, p2, p3;
    orig = SkPoint::Make(1.f, 1.f);
    p1 = SkPoint::Make(1.f - SK_ScalarNearlyZero, 1.f);
    p2 = SkPoint::Make(1.f, 1.f + SK_ScalarNearlyZero);
    p3 = SkPoint::Make(2.f, 2.f);

    return SkPathBuilder()
           .moveTo(orig)
           .cubicTo(p1, p2, p3)
           .close()
           .detach();
}

static void test_path_crbugskia2820(skiatest::Reporter* reporter) {
    SkPath path = make_path_crbugskia2820();

    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
    stroke.setStrokeStyle(2 * SK_Scalar1);

    SkPathBuilder bulider;
    stroke.applyToPath(&bulider, path);
}

static void test_path_crbugskia5995() {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x40303030), SkBits2Float(0x3e303030))  // 2.75294f, 0.172059f
                  .quadTo(SkBits2Float(0x41d63030), SkBits2Float(0x30303030), SkBits2Float(0x41013030),
                          SkBits2Float(0x00000000))  // 26.7735f, 6.40969e-10f, 8.07426f, 0
                  .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))  // 0, 0
                  .detach();
    test_draw_AA_path(500, 500, path);
}

static SkPath make_path0() {
    // from  *  https://code.google.com/p/skia/issues/detail?id=1706

    return SkPathBuilder()
           .moveTo(146.939f, 1012.84f)
           .lineTo(181.747f, 1009.18f)
           .lineTo(182.165f, 1013.16f)
           .lineTo(147.357f, 1016.82f)
           .lineTo(146.939f, 1012.84f)
           .close()
           .detach();
}

static SkPath make_path1() {
    return SkPath::Rect(SkRect::MakeXYWH(10, 10, 10, 1));
}

typedef SkPath (*PathProc)();

/*
 *  Regression test: we used to crash (overwrite internal storage) during
 *  construction of the region when the path was INVERSE. That is now fixed,
 *  so test these regions (which used to assert/crash).
 *
 *  https://code.google.com/p/skia/issues/detail?id=1706
 */
static void test_path_to_region(skiatest::Reporter* reporter) {
    PathProc procs[] = {
        make_path0,
        make_path1,
    };

    SkRegion clip;
    clip.setRect({0, 0, 1255, 1925});

    for (size_t i = 0; i < std::size(procs); ++i) {
        SkPath path = procs[i]();

        SkRegion rgn;
        rgn.setPath(path, clip);
        path.toggleInverseFillType();
        rgn.setPath(path, clip);
    }
}

#ifdef SK_BUILD_FOR_WIN
    #define SUPPRESS_VISIBILITY_WARNING
#else
    #define SUPPRESS_VISIBILITY_WARNING __attribute__((visibility("hidden")))
#endif

static void test_path_close_issue1474(skiatest::Reporter* reporter) {
    // This test checks that r{Line,Quad,Conic,Cubic}To following a close()
    // are relative to the point we close to, not relative to the point we close from.

    // Test rLineTo().
    SkPath path = SkPathBuilder()
                  .rLineTo(0, 100)
                  .rLineTo(100, 0)
                  .close()          // Returns us back to 0,0.
                  .rLineTo(50, 50)  // This should go to 50,50.
                  .detach();
    auto last = path.getLastPt();
    REPORTER_ASSERT(reporter, last.has_value());
    REPORTER_ASSERT(reporter, 50 == last->fX);
    REPORTER_ASSERT(reporter, 50 == last->fY);

    // Test rQuadTo().
    path = SkPathBuilder()
           .rLineTo(0, 100)
           .rLineTo(100, 0)
           .close()
           .rQuadTo(50, 50, 75, 75)
           .detach();

    last = path.getLastPt();
    REPORTER_ASSERT(reporter, last.has_value());
    REPORTER_ASSERT(reporter, 75 == last->fX);
    REPORTER_ASSERT(reporter, 75 == last->fY);

    // Test rConicTo().
    path = SkPathBuilder()
           .rLineTo(0, 100)
           .rLineTo(100, 0)
           .close()
           .rConicTo(50, 50, 85, 85, 2)
           .detach();

    last = path.getLastPt();
    REPORTER_ASSERT(reporter, last.has_value());
    REPORTER_ASSERT(reporter, 85 == last->fX);
    REPORTER_ASSERT(reporter, 85 == last->fY);

    // Test rCubicTo().
    path = SkPathBuilder()
           .rLineTo(0, 100)
           .rLineTo(100, 0)
           .close()
           .rCubicTo(50, 50, 85, 85, 95, 95)
           .detach();

    last = path.getLastPt();
    REPORTER_ASSERT(reporter, last.has_value());
    REPORTER_ASSERT(reporter, 95 == last->fX);
    REPORTER_ASSERT(reporter, 95 == last->fY);
}

// This used to assert in the debug build, as the edges did not all line-up.
static void test_bad_cubic_crbug234190() {
    SkPath path = SkPathBuilder()
                  .moveTo(13.8509f, 3.16858f)
                  .cubicTo(-2.35893e+08f, -4.21044e+08f,
                           -2.38991e+08f, -4.26573e+08f,
                           -2.41016e+08f, -4.30188e+08f)
                  .detach();
    test_draw_AA_path(84, 88, path);
}

static void test_bad_cubic_crbug229478() {
    const SkPoint pts[] = {
        { 4595.91064f,    -11596.9873f },
        { 4597.2168f,    -11595.9414f },
        { 4598.52344f,    -11594.8955f },
        { 4599.83008f,    -11593.8496f },
    };

    SkPath path = SkPathBuilder()
                  .moveTo(pts[0])
                  .cubicTo(pts[1], pts[2], pts[3])
                  .detach();

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);

    // Before the fix, this would infinite-recurse, and run out of stack
    // because we would keep trying to subdivide a degenerate cubic segment.
    (void)skpathutils::FillPathWithPaint(path, paint);
}

static SkPath build_path_170666() {
    return SkPathBuilder()
           .moveTo(17.9459f, 21.6344f)
           .lineTo(139.545f, -47.8105f)
           .lineTo(139.545f, -47.8105f)
           .lineTo(131.07f, -47.3888f)
           .lineTo(131.07f, -47.3888f)
           .lineTo(122.586f, -46.9532f)
           .lineTo(122.586f, -46.9532f)
           .lineTo(18076.6f, 31390.9f)
           .lineTo(18076.6f, 31390.9f)
           .lineTo(18085.1f, 31390.5f)
           .lineTo(18085.1f, 31390.5f)
           .lineTo(18076.6f, 31390.9f)
           .lineTo(18076.6f, 31390.9f)
           .lineTo(17955, 31460.3f)
           .lineTo(17955, 31460.3f)
           .lineTo(17963.5f, 31459.9f)
           .lineTo(17963.5f, 31459.9f)
           .lineTo(17971.9f, 31459.5f)
           .lineTo(17971.9f, 31459.5f)
           .lineTo(17.9551f, 21.6205f)
           .lineTo(17.9551f, 21.6205f)
           .lineTo(9.47091f, 22.0561f)
           .lineTo(9.47091f, 22.0561f)
           .lineTo(17.9459f, 21.6344f)
           .lineTo(17.9459f, 21.6344f)
           .close()
           .moveTo(0.995934f, 22.4779f)
           .lineTo(0.986725f, 22.4918f)
           .lineTo(0.986725f, 22.4918f)
           .lineTo(17955, 31460.4f)
           .lineTo(17955, 31460.4f)
           .lineTo(17971.9f, 31459.5f)
           .lineTo(17971.9f, 31459.5f)
           .lineTo(18093.6f, 31390.1f)
           .lineTo(18093.6f, 31390.1f)
           .lineTo(18093.6f, 31390)
           .lineTo(18093.6f, 31390)
           .lineTo(139.555f, -47.8244f)
           .lineTo(139.555f, -47.8244f)
           .lineTo(122.595f, -46.9671f)
           .lineTo(122.595f, -46.9671f)
           .lineTo(0.995934f, 22.4779f)
           .lineTo(0.995934f, 22.4779f)
           .close()
           .moveTo(5.43941f, 25.5223f)
           .lineTo(798267, -28871.1f)
           .lineTo(798267, -28871.1f)
           .lineTo(3.12512e+06f, -113102)
           .lineTo(3.12512e+06f, -113102)
           .cubicTo(5.16324e+06f, -186882, 8.15247e+06f, -295092, 1.1957e+07f, -432813)
           .cubicTo(1.95659e+07f, -708257, 3.04359e+07f, -1.10175e+06f, 4.34798e+07f, -1.57394e+06f)
           .cubicTo(6.95677e+07f, -2.51831e+06f, 1.04352e+08f, -3.77748e+06f, 1.39135e+08f, -5.03666e+06f)
           .cubicTo(1.73919e+08f, -6.29583e+06f, 2.08703e+08f, -7.555e+06f, 2.34791e+08f, -8.49938e+06f)
           .cubicTo(2.47835e+08f, -8.97157e+06f, 2.58705e+08f, -9.36506e+06f, 2.66314e+08f, -9.6405e+06f)
           .cubicTo(2.70118e+08f, -9.77823e+06f, 2.73108e+08f, -9.88644e+06f, 2.75146e+08f, -9.96022e+06f)
           .cubicTo(2.76165e+08f, -9.99711e+06f, 2.76946e+08f, -1.00254e+07f, 2.77473e+08f, -1.00444e+07f)
           .lineTo(2.78271e+08f, -1.00733e+07f)
           .lineTo(2.78271e+08f, -1.00733e+07f)
           .cubicTo(2.78271e+08f, -1.00733e+07f, 2.08703e+08f, -7.555e+06f, 135.238f, 23.3517f)
           .cubicTo(131.191f, 23.4981f, 125.995f, 23.7976f, 123.631f, 24.0206f)
           .cubicTo(121.267f, 24.2436f, 122.631f, 24.3056f, 126.677f, 24.1591f)
           .cubicTo(2.08703e+08f, -7.555e+06f, 2.78271e+08f, -1.00733e+07f, 2.78271e+08f, -1.00733e+07f)
           .lineTo(2.77473e+08f, -1.00444e+07f)
           .lineTo(2.77473e+08f, -1.00444e+07f)
           .cubicTo(2.76946e+08f, -1.00254e+07f, 2.76165e+08f, -9.99711e+06f, 2.75146e+08f, -9.96022e+06f)
           .cubicTo(2.73108e+08f, -9.88644e+06f, 2.70118e+08f, -9.77823e+06f, 2.66314e+08f, -9.6405e+06f)
           .cubicTo(2.58705e+08f, -9.36506e+06f, 2.47835e+08f, -8.97157e+06f, 2.34791e+08f, -8.49938e+06f)
           .cubicTo(2.08703e+08f, -7.555e+06f, 1.73919e+08f, -6.29583e+06f, 1.39135e+08f, -5.03666e+06f)
           .cubicTo(1.04352e+08f, -3.77749e+06f, 6.95677e+07f, -2.51831e+06f, 4.34798e+07f, -1.57394e+06f)
           .cubicTo(3.04359e+07f, -1.10175e+06f, 1.95659e+07f, -708258, 1.1957e+07f, -432814)
           .cubicTo(8.15248e+06f, -295092, 5.16324e+06f, -186883, 3.12513e+06f, -113103)
           .lineTo(798284, -28872)
           .lineTo(798284, -28872)
           .lineTo(22.4044f, 24.6677f)
           .lineTo(22.4044f, 24.6677f)
           .cubicTo(22.5186f, 24.5432f, 18.8134f, 24.6337f, 14.1287f, 24.8697f)
           .cubicTo(9.4439f, 25.1057f, 5.55359f, 25.3978f, 5.43941f, 25.5223f)
           .close()
           .detach();
}

static SkPath build_path_simple_170666() {
    return SkPathBuilder()
           .moveTo(126.677f, 24.1591f)
           .cubicTo(2.08703e+08f, -7.555e+06f, 2.78271e+08f, -1.00733e+07f, 2.78271e+08f, -1.00733e+07f)
           .detach();
}

// This used to assert in the SK_DEBUG build, as the clip step would fail with
// too-few interations in our cubic-line intersection code. That code now runs
// 24 interations (instead of 16).
static void test_crbug_170666() {
    SkPath path = build_path_simple_170666();
    test_draw_AA_path(1000, 1000, path);

    path = build_path_170666();
    test_draw_AA_path(1000, 1000, path);
}


static void test_tiny_path_convexity(skiatest::Reporter* reporter, const char* pathBug,
        SkScalar tx, SkScalar ty, SkScalar scale) {
    auto smallPath = SkParsePath::FromSVGString(pathBug);
    SkAssertResult(smallPath.has_value());
    bool smallConvex = smallPath->isConvex();
    auto largePath = SkParsePath::FromSVGString(pathBug);
    SkAssertResult(largePath.has_value());
    SkMatrix matrix;
    matrix.reset();
    matrix.preTranslate(100, 100);
    matrix.preScale(scale, scale);
    largePath = largePath->makeTransform(matrix);
    bool largeConvex = largePath->isConvex();
    REPORTER_ASSERT(reporter, smallConvex == largeConvex);
}

static void test_crbug_493450(skiatest::Reporter* reporter) {
    const char reducedCase[] =
        "M0,0"
        "L0.0002, 0"
        "L0.0002, 0.0002"
        "L0.0001, 0.0001"
        "L0,0.0002"
        "Z";
    test_tiny_path_convexity(reporter, reducedCase, 100, 100, 100000);
    const char originalFiddleData[] =
        "M-0.3383152268862998,-0.11217565719203619L-0.33846085183212765,-0.11212264406895281"
        "L-0.338509393480737,-0.11210607966681395L-0.33857792286700894,-0.1121889121487573"
        "L-0.3383866116636664,-0.11228834570924921L-0.33842087635680235,-0.11246078673250548"
        "L-0.33809536177201055,-0.11245415228342878L-0.33797257995493996,-0.11216571641452182"
        "L-0.33802112160354925,-0.11201996164188659L-0.33819815585141844,-0.11218559834671019Z";
    test_tiny_path_convexity(reporter, originalFiddleData, 280081.4116670522f, 93268.04618493588f,
            826357.3384828606f);
}

static void test_crbug_495894(skiatest::Reporter* reporter) {
    const char originalFiddleData[] =
        "M-0.34004273849857214,-0.11332803232216355L-0.34008271397389744,-0.11324483772714951"
        "L-0.3401940742265893,-0.11324483772714951L-0.34017694188002134,-0.11329807920275889"
        "L-0.3402026403998733,-0.11333468903941245L-0.34029972369709194,-0.11334134592705701"
        "L-0.3403054344792813,-0.11344121970007795L-0.3403140006525653,-0.11351115418399343"
        "L-0.34024261587519866,-0.11353446986281181L-0.3402197727464413,-0.11360442946144192"
        "L-0.34013696640469604,-0.11359110237029302L-0.34009128014718143,-0.1135877707043939"
        "L-0.3400598708451401,-0.11360776134112742L-0.34004273849857214,-0.11355112520064405"
        "L-0.3400113291965308,-0.11355112520064405L-0.3399970522410575,-0.11359110237029302"
        "L-0.33997135372120546,-0.11355112520064405L-0.3399627875479215,-0.11353780084493197"
        "L-0.3399485105924481,-0.11350782354357004L-0.3400027630232468,-0.11346452910331437"
        "L-0.3399485105924481,-0.11340126558629839L-0.33993994441916414,-0.11340126558629839"
        "L-0.33988283659727087,-0.11331804756574679L-0.33989140277055485,-0.11324483772714951"
        "L-0.33997991989448945,-0.11324483772714951L-0.3399856306766788,-0.11324483772714951"
        "L-0.34002560615200417,-0.11334467443478255ZM-0.3400684370184241,-0.11338461985124307"
        "L-0.340154098751264,-0.11341791238732665L-0.340162664924548,-0.1134378899559977"
        "L-0.34017979727111597,-0.11340126558629839L-0.3401655203156427,-0.11338129083212668"
        "L-0.34012268944922275,-0.11332137577529414L-0.34007414780061346,-0.11334467443478255Z"
        "M-0.3400027630232468,-0.11290567901106024L-0.3400113291965308,-0.11298876531245433"
        "L-0.33997991989448945,-0.11301535852306784L-0.33990282433493346,-0.11296217481488612"
        "L-0.33993994441916414,-0.11288906492739594Z";
    test_tiny_path_convexity(reporter, originalFiddleData, 22682.240000000005f,7819.72220766405f,
            65536);
}

static void test_crbug_613918() {
    SkPath path = SkPathBuilder()
                  .conicTo(-6.62478e-08f, 4.13885e-08f, -6.36935e-08f, 3.97927e-08f, 0.729058f)
                  .quadTo(2.28206e-09f, -1.42572e-09f, 3.91919e-09f, -2.44852e-09f)
                  .cubicTo(-16752.2f, -26792.9f, -21.4673f, 10.9347f, -8.57322f, -7.22739f)
                  .detach();

    // This call could lead to an assert or uninitialized read due to a failure
    // to check the return value from SkCubicClipper::ChopMonoAtY.
    path.contains(-1.84817e-08f, 1.15465e-08f);
}

static void test_addrect(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .lineTo(0, 0)
                  .addRect(SkRect::MakeWH(50, 100))
                  .detach();
    REPORTER_ASSERT(reporter, path.isRect(nullptr));

    path = SkPathBuilder()
           .lineTo(FLT_EPSILON, FLT_EPSILON)
           .addRect(SkRect::MakeWH(50, 100))
           .detach();
    REPORTER_ASSERT(reporter, !path.isRect(nullptr));

    path = SkPathBuilder()
           .quadTo(0, 0, 0, 0)
           .addRect(SkRect::MakeWH(50, 100))
           .detach();
    REPORTER_ASSERT(reporter, !path.isRect(nullptr));

    path = SkPathBuilder()
           .conicTo(0, 0, 0, 0, 0.5f)
           .addRect(SkRect::MakeWH(50, 100))
           .detach();
    REPORTER_ASSERT(reporter, !path.isRect(nullptr));

    path = SkPathBuilder()
           .cubicTo(0, 0, 0, 0, 0, 0)
           .addRect(SkRect::MakeWH(50, 100))
           .detach();
    REPORTER_ASSERT(reporter, !path.isRect(nullptr));
}

// Make sure we stay non-finite once we get there (unless we reset or rewind).
static void test_addrect_isfinite(skiatest::Reporter* reporter) {
    SkPathBuilder builder;
    builder.addRect(SkRect::MakeWH(50, 100));

    SkPath path = builder.snapshot();
    REPORTER_ASSERT(reporter, path.isFinite());

    builder.moveTo(0, 0);
    builder.lineTo(SK_ScalarInfinity, 42);
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, !path.isFinite());

    builder.addRect(SkRect::MakeWH(50, 100));
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, !path.isFinite());

    builder.reset();
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, path.isFinite());

    builder.addRect(SkRect::MakeWH(50, 100));
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, path.isFinite());
}

static SkPath build_big_path(bool reducedCase) {
    SkPathBuilder builder;
    if (reducedCase) {
        builder.moveTo(577330, 1971.72f);
        builder.cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
    } else {
        builder.moveTo(60.1631f, 7.70567f);
        builder.quadTo(60.1631f, 7.70567f, 0.99474f, 0.901199f);
        builder.lineTo(577379, 1977.77f);
        builder.quadTo(577364, 1979.57f, 577325, 1980.26f);
        builder.quadTo(577286, 1980.95f, 577245, 1980.13f);
        builder.quadTo(577205, 1979.3f, 577187, 1977.45f);
        builder.quadTo(577168, 1975.6f, 577183, 1973.8f);
        builder.quadTo(577198, 1972, 577238, 1971.31f);
        builder.quadTo(577277, 1970.62f, 577317, 1971.45f);
        builder.quadTo(577330, 1971.72f, 577341, 1972.11f);
        builder.cubicTo(10.7082f, -116.596f, 262.057f, 45.6468f, 294.694f, 1.96237f);
        builder.moveTo(306.718f, -32.912f);
        builder.cubicTo(30.531f, 10.0005f, 1502.47f, 13.2804f, 84.3088f, 9.99601f);
    }
    return builder.detach();
}

static void test_clipped_cubic() {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(640, 480)));

    // This path used to assert, because our cubic-chopping code incorrectly
    // moved control points after the chop. This test should be run in SK_DEBUG
    // mode to ensure that we no long assert.

    for (int doReducedCase = 0; doReducedCase <= 1; ++doReducedCase) {
        SkPath path = build_big_path(SkToBool(doReducedCase));

        SkPaint paint;
        for (int doAA = 0; doAA <= 1; ++doAA) {
            paint.setAntiAlias(SkToBool(doAA));
            surface->getCanvas()->drawPath(path, paint);
        }
    }
}

static void dump_if_ne(skiatest::Reporter* reporter, const SkRect& expected, const SkRect& bounds) {
    if (expected != bounds) {
        ERRORF(reporter, "path.getBounds() returned [%g %g %g %g], but expected [%g %g %g %g]",
               bounds.left(), bounds.top(), bounds.right(), bounds.bottom(),
               expected.left(), expected.top(), expected.right(), expected.bottom());
    }
}

static void test_bounds_crbug_513799(skiatest::Reporter* reporter) {
    SkPath path;
    dump_if_ne(reporter, SkRect::MakeLTRB(0, 0, 0, 0), path.getBounds());

    SkPathBuilder builder;
    builder.moveTo(-5, -8);    // should generate bounds
    path = builder.snapshot();
    dump_if_ne(reporter, SkRect::MakeLTRB(-5, -8, -5, -8), path.getBounds());

    builder.addRect(SkRect::MakeLTRB(1, 2, 3, 4)); // should extend the bounds
    path = builder.snapshot();
    dump_if_ne(reporter, SkRect::MakeLTRB(1, 2, 3, 4), path.getBounds());

    builder.moveTo(2, 3);  // don't expect this to have changed the bounds
    path = builder.snapshot();
    dump_if_ne(reporter, SkRect::MakeLTRB(1, 2, 3, 4), path.getBounds());
}

static void test_fuzz_crbug_627414(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .conicTo(3.58732e-43f, 2.72084f, 3.00392f, 3.00392f, 8.46e+37f)
                  .detach();
    test_draw_AA_path(100, 100, path);
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

    SkPath path = SkPathBuilder()
                  .moveTo(pts[0])
                  .cubicTo(pts[1], pts[2], pts[3])
                  .detach();
    test_draw_AA_path(19, 130, path);
}

// Inspired by http://code.google.com/p/chromium/issues/detail?id=141651
//
static void test_isfinite_after_transform(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .quadTo(157, 366, 286, 208)
                  .arcTo({37, 442}, {315, 163}, 957494590897113.0f)
                  .detach();

    SkMatrix matrix;
    matrix.setScale(1000*1000, 1000*1000);

    // Be sure that path::transform correctly updates isFinite and the bounds
    // if the transformation overflows. The previous bug was that isFinite was
    // set to true in this case, but the bounds were not set to empty (which
    // they should be).
    while (path.isFinite()) {
        REPORTER_ASSERT(reporter, path.getBounds().isFinite());
        REPORTER_ASSERT(reporter, !path.getBounds().isEmpty());
        path = path.makeTransform(matrix);
    }
    REPORTER_ASSERT(reporter, path.getBounds().isEmpty());

    matrix.setTranslate(SK_Scalar1, SK_Scalar1);
    path = path.makeTransform(matrix);
    // we need to still be non-finite
    REPORTER_ASSERT(reporter, !path.isFinite());
    REPORTER_ASSERT(reporter, path.getBounds().isEmpty());
}

static void add_corner_arc(SkPathBuilder* builder, const SkRect& rect,
                           SkScalar xIn, SkScalar yIn,
                           int startAngle)
{

    SkScalar rx = std::min(rect.width(), xIn);
    SkScalar ry = std::min(rect.height(), yIn);

    SkRect arcRect;
    arcRect.setLTRB(-rx, -ry, rx, ry);
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

    builder->arcTo(arcRect, SkIntToScalar(startAngle), SkIntToScalar(90), false);
}

static SkPath make_arb_round_rect(const SkRect& r, SkScalar xCorner, SkScalar yCorner) {
    SkPathBuilder builder;
    // we are lazy here and use the same x & y for each corner
    add_corner_arc(&builder, r, xCorner, yCorner, 270);
    add_corner_arc(&builder, r, xCorner, yCorner, 0);
    add_corner_arc(&builder, r, xCorner, yCorner, 90);
    add_corner_arc(&builder, r, xCorner, yCorner, 180);
    builder.close();
    return builder.detach();
}

// Chrome creates its own round rects with each corner possibly being different.
// Performance will suffer if they are not convex.
// Note: PathBench::ArbRoundRectBench performs almost exactly
// the same test (but with drawing)
static void test_arb_round_rect_is_convex(skiatest::Reporter* reporter) {
    SkRandom rand;
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

        SkPath temp = make_arb_round_rect(r, r.width() / 10, r.height() / 15);

        REPORTER_ASSERT(reporter, temp.isConvex());
    }
}

// Chrome will sometimes create a 0 radius round rect. The degenerate
// quads prevent the path from being converted to a rect
// Note: PathBench::ArbRoundRectBench performs almost exactly
// the same test (but with drawing)
static void test_arb_zero_rad_round_rect_is_rect(skiatest::Reporter* reporter) {
    SkRandom rand;
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

        SkPath temp = make_arb_round_rect(r, 0, 0);

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
    r.setLTRB(0, 0, inf, negInf);
    REPORTER_ASSERT(reporter, !r.isFinite());
    r.setLTRB(0, 0, nan, 0);
    REPORTER_ASSERT(reporter, !r.isFinite());

    SkPoint pts[] = {
        { 0, 0 },
        { SK_Scalar1, 0 },
        { 0, SK_Scalar1 },
    };

    bool isFine = r.setBoundsCheck(pts);
    REPORTER_ASSERT(reporter, isFine);
    REPORTER_ASSERT(reporter, !r.isEmpty());

    pts[1].set(inf, 0);
    isFine = r.setBoundsCheck(pts);
    REPORTER_ASSERT(reporter, !isFine);
    REPORTER_ASSERT(reporter, r.isEmpty());

    pts[1].set(nan, 0);
    isFine = r.setBoundsCheck(pts);
    REPORTER_ASSERT(reporter, !isFine);
    REPORTER_ASSERT(reporter, r.isEmpty());
}

static void test_path_isfinite(skiatest::Reporter* reporter) {
    const SkScalar inf = SK_ScalarInfinity;
    const SkScalar negInf = SK_ScalarNegativeInfinity;
    const SkScalar nan = SK_ScalarNaN;

    SkPath path;
    REPORTER_ASSERT(reporter, path.isFinite());

    path = SkPathBuilder().moveTo(1, 0).detach();
    REPORTER_ASSERT(reporter, path.isFinite());

    path = SkPathBuilder().moveTo(inf, negInf).detach();
    REPORTER_ASSERT(reporter, !path.isFinite());

    path = SkPathBuilder().moveTo(nan, 0).detach();
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
    bool firstTime = true;
    bool foundClose = false;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                REPORTER_ASSERT(reporter, firstTime);
                REPORTER_ASSERT(reporter, pts[0] == srcPts[0]);
                srcPts++;
                firstTime = false;
                break;
            case SkPathVerb::kLine:
                REPORTER_ASSERT(reporter, !firstTime);
                REPORTER_ASSERT(reporter, pts[1] == srcPts[0]);
                srcPts++;
                break;
            case SkPathVerb::kQuad:
                REPORTER_ASSERT(reporter, false, "unexpected quad verb");
                break;
            case SkPathVerb::kConic:
                REPORTER_ASSERT(reporter, false, "unexpected conic verb");
                break;
            case SkPathVerb::kCubic:
                REPORTER_ASSERT(reporter, false, "unexpected cubic verb");
                break;
            case SkPathVerb::kClose:
                REPORTER_ASSERT(reporter, !firstTime);
                REPORTER_ASSERT(reporter, !foundClose);
                REPORTER_ASSERT(reporter, expectClose);
                foundClose = true;
                break;
        }
    }
    REPORTER_ASSERT(reporter, foundClose == expectClose);
}

static void test_addPoly(skiatest::Reporter* reporter) {
    SkPoint pts[32];
    SkRandom rand;

    for (size_t i = 0; i < std::size(pts); ++i) {
        pts[i].fX = rand.nextSScalar1();
        pts[i].fY = rand.nextSScalar1();
    }

    for (int doClose = 0; doClose <= 1; ++doClose) {
        for (size_t count = 1; count <= std::size(pts); ++count) {
            SkPath path = SkPath::Polygon({pts, count}, SkToBool(doClose));
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
// Legal values are CW (0), CCW (1) and Unknown (2), leaving 3 as a convenient sentinel.
const SkPathFirstDirection kDontCheckDir = static_cast<SkPathFirstDirection>(3);

static void check_direction(skiatest::Reporter* reporter, const SkPath& path,
                            SkPathFirstDirection expected) {
    if (expected == kDontCheckDir) {
        return;
    }
    // We make a copy so that we don't cache the result on the passed in path.
    SkPath copy(path);  // NOLINT(performance-unnecessary-copy-initialization)

    SkPathFirstDirection dir = SkPathPriv::ComputeFirstDirection(copy);
    if (dir != SkPathFirstDirection::kUnknown) {
        REPORTER_ASSERT(reporter, dir == expected);
    }
}

static void test_direction(skiatest::Reporter* reporter) {
    size_t i;
    REPORTER_ASSERT(reporter,
                    SkPathPriv::ComputeFirstDirection(SkPath()) == SkPathFirstDirection::kUnknown);

    static const char* gDegen[] = {
        "M 10 10",
        "M 10 10 M 20 20",
        "M 10 10 L 20 20",
        "M 10 10 L 10 10 L 10 10",
        "M 10 10 Q 10 10 10 10",
        "M 10 10 C 10 10 10 10 10 10",
    };
    for (i = 0; i < std::size(gDegen); ++i) {
        auto path = SkParsePath::FromSVGString(gDegen[i]);
        REPORTER_ASSERT(reporter, path.has_value());
        REPORTER_ASSERT(reporter,
                        SkPathPriv::ComputeFirstDirection(*path) == SkPathFirstDirection::kUnknown);
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
    for (i = 0; i < std::size(gCW); ++i) {
        auto path = SkParsePath::FromSVGString(gCW[i]);
        REPORTER_ASSERT(reporter, path.has_value());
        check_direction(reporter, *path, SkPathFirstDirection::kCW);
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
    for (i = 0; i < std::size(gCCW); ++i) {
        auto path = SkParsePath::FromSVGString(gCCW[i]);
        REPORTER_ASSERT(reporter, path.has_value());
        check_direction(reporter, *path, SkPathFirstDirection::kCCW);
    }

    // Test two donuts, each wound a different direction. Only the outer contour
    // determines the cheap direction
    SkPath path = SkPathBuilder()
                  .addCircle(0, 0, 2, SkPathDirection::kCW)
                  .addCircle(0, 0, 1, SkPathDirection::kCCW)
                  .detach();
    check_direction(reporter, path, SkPathFirstDirection::kCW);

    path = SkPathBuilder()
           .addCircle(0, 0, 1, SkPathDirection::kCW)
           .addCircle(0, 0, 2, SkPathDirection::kCCW)
           .detach();
    check_direction(reporter, path, SkPathFirstDirection::kCCW);

    // triangle with one point really far from the origin.
    // the first point is roughly 1.05e10, 1.05e10
    path = SkPathBuilder()
           .moveTo(SkBits2Float(0x501c7652), SkBits2Float(0x501c7652))
           .lineTo(110, -10)
           .lineTo(-10, 60)
           .detach();
    check_direction(reporter, path, SkPathFirstDirection::kCCW);

    path = SkPathBuilder()
           .conicTo(20, 0, 20, 20, 0.5f)
           .close()
           .detach();
    check_direction(reporter, path, SkPathFirstDirection::kCW);

    path = SkPathBuilder()
           .lineTo(1, 1e7f)
           .lineTo(1e7f, 2e7f)
           .close()
           .detach();
    REPORTER_ASSERT(reporter, path.isConvex());
    check_direction(reporter, path, SkPathFirstDirection::kCCW);
}

static void add_rect(SkPathBuilder* builder, const SkRect& r) {
    builder->moveTo(r.fLeft, r.fTop);
    builder->lineTo(r.fRight, r.fTop);
    builder->lineTo(r.fRight, r.fBottom);
    builder->lineTo(r.fLeft, r.fBottom);
    builder->close();
}

static void test_bounds(skiatest::Reporter* reporter) {
    static const SkRect rects[] = {
        { SkIntToScalar(10), SkIntToScalar(160), SkIntToScalar(610), SkIntToScalar(160) },
        { SkIntToScalar(610), SkIntToScalar(160), SkIntToScalar(610), SkIntToScalar(199) },
        { SkIntToScalar(10), SkIntToScalar(198), SkIntToScalar(610), SkIntToScalar(199) },
        { SkIntToScalar(10), SkIntToScalar(160), SkIntToScalar(10), SkIntToScalar(199) },
    };

    SkPathBuilder builder0, builder1;
    for (size_t i = 0; i < std::size(rects); ++i) {
        builder0.addRect(rects[i]);
        add_rect(&builder1, rects[i]);
    }

    SkPath path0 = builder0.detach(),
           path1 = builder1.detach();
    REPORTER_ASSERT(reporter, path0.getBounds() == path1.getBounds());
}

static void stroke_cubic(const SkPoint pts[4]) {
    SkPath path = SkPathBuilder()
                  .moveTo(pts[0])
                  .cubicTo(pts[1], pts[2], pts[3])
                  .detach();

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);

    (void)skpathutils::FillPathWithPaint(path, paint);
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
    SkPath closePt = SkPathBuilder().moveTo(0, 0).close().detach();
    check_close(reporter, closePt);

    SkPath openPt = SkPathBuilder().moveTo(0, 0).detach();
    check_close(reporter, openPt);

    SkPath empty;
    check_close(reporter, empty);
    empty = SkPathBuilder().close().detach();
    check_close(reporter, empty);

    SkPathBuilder builder;
    builder.addRect({1, 1, 10, 10});
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.quadTo(1, 1, 10, 10);
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.cubicTo(1, 1, 10, 10, 20, 20);
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.moveTo(1, 1);
    builder.lineTo(10, 10);
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.addRect({1, 1, 10, 10});
    builder.close();
    builder.addRect({1, 1, 10, 10});
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.addOval(SkRect::MakeWH(100, 100));
    builder.close();
    builder.addOval(SkRect::MakeWH(200, 200));
    check_close(reporter, builder.snapshot());
    builder.close();
    check_close(reporter, builder.detach());

    builder.moveTo(1, 1);
    builder.moveTo(5, 1);
    builder.moveTo(1, 10);
    builder.moveTo(10, 1);
    check_close(reporter, builder.detach());

    stroke_tiny_cubic();
}

static void check_convexity(skiatest::Reporter* reporter, const SkPath& path,
                            bool expectedConvexity) {
    // We make a copy so that we don't cache the result on the passed in path.
    SkPath copy(path);  // NOLINT(performance-unnecessary-copy-initialization)
    bool convexity = copy.isConvex();
    REPORTER_ASSERT(reporter, convexity == expectedConvexity);
}

static void test_path_crbug389050(skiatest::Reporter* reporter) {
    SkPathBuilder tinyConvexPolygon;
    tinyConvexPolygon.moveTo(600.131559f, 800.112512f);
    tinyConvexPolygon.lineTo(600.161735f, 800.118627f);
    tinyConvexPolygon.lineTo(600.148962f, 800.142338f);
    tinyConvexPolygon.lineTo(600.134891f, 800.137724f);
    tinyConvexPolygon.close();
    check_direction(reporter, tinyConvexPolygon.detach(), SkPathFirstDirection::kCW);

    SkPathBuilder platTriangle;
    platTriangle.moveTo(0, 0);
    platTriangle.lineTo(200, 0);
    platTriangle.lineTo(100, 0.04f);
    platTriangle.close();
    check_direction(reporter, platTriangle.detach(), SkPathFirstDirection::kCW);

    platTriangle.moveTo(0, 0);
    platTriangle.lineTo(200, 0);
    platTriangle.lineTo(100, 0.03f);
    platTriangle.close();
    check_direction(reporter, platTriangle.detach(), SkPathFirstDirection::kCW);
}

static void test_convexity2(skiatest::Reporter* reporter) {
    SkPath pt = SkPathBuilder()
                .moveTo(0, 0)
                .close()
                .detach();
    check_convexity(reporter, pt, true);
    check_direction(reporter, pt, SkPathFirstDirection::kUnknown);

    SkPath line = SkPathBuilder()
                  .moveTo(12, 20)
                  .lineTo(-12, -20)
                  .close()
                  .detach();
    check_convexity(reporter, line, true);
    check_direction(reporter, line, SkPathFirstDirection::kUnknown);

    SkPath triLeft = SkPathBuilder()
                     .moveTo(0, 0)
                     .lineTo(1, 0)
                     .lineTo(1, 1)
                     .close()
                     .detach();
    check_convexity(reporter, triLeft, true);
    check_direction(reporter, triLeft, SkPathFirstDirection::kCW);

    SkPath triRight = SkPathBuilder()
                      .moveTo(0, 0)
                      .lineTo(-1, 0)
                      .lineTo(1, 1)
                      .close()
                      .detach();
    check_convexity(reporter, triRight, true);
    check_direction(reporter, triRight, SkPathFirstDirection::kCCW);

    SkPath square = SkPathBuilder()
                    .moveTo(0, 0)
                    .lineTo(1, 0)
                    .lineTo(1, 1)
                    .lineTo(0, 1)
                    .close()
                    .detach();
    check_convexity(reporter, square, true);
    check_direction(reporter, square, SkPathFirstDirection::kCW);

    SkPath redundantSquare = SkPathBuilder()
                             .moveTo(0, 0)
                             .lineTo(0, 0)
                             .lineTo(0, 0)
                             .lineTo(1, 0)
                             .lineTo(1, 0)
                             .lineTo(1, 0)
                             .lineTo(1, 1)
                             .lineTo(1, 1)
                             .lineTo(1, 1)
                             .lineTo(0, 1)
                             .lineTo(0, 1)
                             .lineTo(0, 1)
                             .close()
                             .detach();
    check_convexity(reporter, redundantSquare, true);
    check_direction(reporter, redundantSquare, SkPathFirstDirection::kCW);

    SkPath bowTie = SkPathBuilder()
                    .moveTo(0, 0)
                    .lineTo(0, 0)
                    .lineTo(0, 0)
                    .lineTo(1, 1)
                    .lineTo(1, 1)
                    .lineTo(1, 1)
                    .lineTo(1, 0)
                    .lineTo(1, 0)
                    .lineTo(1, 0)
                    .lineTo(0, 1)
                    .lineTo(0, 1)
                    .lineTo(0, 1)
                    .close()
                    .detach();
    check_convexity(reporter, bowTie, false);
    check_direction(reporter, bowTie, kDontCheckDir);

    SkPath spiral = SkPathBuilder()
                    .moveTo(0, 0)
                    .lineTo(100, 0)
                    .lineTo(100, 100)
                    .lineTo(0, 100)
                    .lineTo(0, 50)
                    .lineTo(50, 50)
                    .lineTo(50, 75)
                    .close()
                    .detach();
    check_convexity(reporter, spiral, false);
    check_direction(reporter, spiral, kDontCheckDir);

    SkPath dent = SkPathBuilder()
                  .moveTo(0, 0)
                  .lineTo(100, 100)
                  .lineTo(0, 100)
                  .lineTo(-50, 200)
                  .lineTo(-200, 100)
                  .close()
                  .detach();
    check_convexity(reporter, dent, false);
    check_direction(reporter, dent, SkPathFirstDirection::kCW);

    // skbug.com/40033336
    SkPathBuilder strokedSin;
    for (int i = 0; i < 2000; i++) {
        SkScalar x = SkIntToScalar(i) / 2;
        SkScalar y = 500 - (x + SkScalarSin(x / 100) * 40) / 3;
        if (0 == i) {
            strokedSin.moveTo(x, y);
        } else {
            strokedSin.lineTo(x, y);
        }
    }
    SkStrokeRec stroke(SkStrokeRec::kFill_InitStyle);
    stroke.setStrokeStyle(2);
    SkPathBuilder builder;
    stroke.applyToPath(&builder, strokedSin.detach());
    SkPath newpath = builder.detach();
    check_convexity(reporter, newpath, false);
    check_direction(reporter, newpath, kDontCheckDir);

    // http://crbug.com/412640
    SkPath degenerateConcave = SkPathBuilder()
                               .moveTo(148.67912f, 191.875f)
                               .lineTo(470.37695f, 7.5f)
                               .lineTo(148.67912f, 191.875f)
                               .lineTo(41.446522f, 376.25f)
                               .lineTo(-55.971577f, 460.0f)
                               .lineTo(41.446522f, 376.25f)
                               .detach();
    check_convexity(reporter, degenerateConcave, false);
    check_direction(reporter, degenerateConcave, SkPathFirstDirection::kUnknown);

    // http://crbug.com/433683
    SkPath badFirstVector = SkPathBuilder()
                            .moveTo(501.087708f, 319.610352f)
                            .lineTo(501.087708f, 319.610352f)
                            .cubicTo(501.087677f, 319.610321f, 449.271606f, 258.078674f, 395.084564f, 198.711182f)
                            .cubicTo(358.967072f, 159.140717f, 321.910553f, 120.650436f, 298.442322f, 101.955399f)
                            .lineTo(301.557678f, 98.044601f)
                            .cubicTo(325.283844f, 116.945084f, 362.615204f, 155.720825f, 398.777557f, 195.340454f)
                            .cubicTo(453.031860f, 254.781662f, 504.912262f, 316.389618f, 504.912292f, 316.389648f)
                            .lineTo(504.912292f, 316.389648f)
                            .lineTo(501.087708f, 319.610352f)
                            .close()
                            .detach();
    check_convexity(reporter, badFirstVector, false);

    // http://crbug.com/993330
    SkPath falseBackEdge = SkPathBuilder()
                           .moveTo(-217.83430557928145f,      -382.14948768484857f)
                           .lineTo(-227.73867866614847f,      -399.52485512718323f)
                           .cubicTo(-158.3541047666846f,      -439.0757140459542f,
                                     -79.8654464485281f,      -459.875f,
                                      -1.1368683772161603e-13f, -459.875f)
                           .lineTo(-8.08037266162413e-14f,    -439.875f)
                           .lineTo(-8.526512829121202e-14f,   -439.87499999999994f)
                           .cubicTo( -76.39209188702645f,     -439.87499999999994f,
                                    -151.46727226799754f,     -419.98027663161537f,
                                    -217.83430557928145f,     -382.14948768484857f)
                           .close()
                           .detach();
    check_convexity(reporter, falseBackEdge, false);
}

static void test_convexity_doubleback(skiatest::Reporter* reporter) {
    SkPathBuilder doubleback;
    doubleback.lineTo(1, 1);
    check_convexity(reporter, doubleback.snapshot(), true);
    doubleback.lineTo(2, 2);
    check_convexity(reporter, doubleback.detach(), true);

    doubleback.lineTo(1, 0);
    check_convexity(reporter, doubleback.snapshot(), true);
    doubleback.lineTo(2, 0);
    check_convexity(reporter, doubleback.snapshot(), true);
    doubleback.lineTo(1, 0);
    check_convexity(reporter, doubleback.detach(), true);

    doubleback.quadTo(1, 1, 2, 2);
    check_convexity(reporter, doubleback.detach(), true);

    doubleback.quadTo(1, 0, 2, 0);
    check_convexity(reporter, doubleback.snapshot(), true);
    doubleback.quadTo(1, 0, 0, 0);
    check_convexity(reporter, doubleback.detach(), true);

    doubleback.lineTo(1, 0);
    doubleback.lineTo(1, 0);
    doubleback.lineTo(1, 1);
    doubleback.lineTo(1, 1);
    doubleback.lineTo(1, 0);
    check_convexity(reporter, doubleback.detach(), false);

    doubleback.lineTo(-1, 0);
    doubleback.lineTo(-1, 1);
    doubleback.lineTo(-1, 0);
    check_convexity(reporter, doubleback.detach(), false);

    for (int i = 0; i < 4; ++i) {
        doubleback.moveTo(0, 0);
        if (i == 0) {
            doubleback.lineTo(-1, -1);
            doubleback.lineTo(0, 0);
        }
        doubleback.lineTo(0, 1);
        if (i == 1) {
            doubleback.lineTo(0, 2);
            doubleback.lineTo(0, 1);
        }
        doubleback.lineTo(1, 1);
        if (i == 2) {
            doubleback.lineTo(2, 2);
            doubleback.lineTo(1, 1);
        }
        doubleback.lineTo(0, 0);
        if (i == 3) {
            doubleback.lineTo(-1, -1);
            doubleback.lineTo(0, 0);
        }
        check_convexity(reporter, doubleback.detach(), false);
    }
}

static SkPath setFromString(const char str[]) {
    SkPathBuilder builder;
    bool first = true;
    while (str) {
        SkScalar x, y;
        str = SkParse::FindScalar(str, &x);
        if (nullptr == str) {
            break;
        }
        str = SkParse::FindScalar(str, &y);
        SkASSERT(str);
        if (first) {
            builder.moveTo(x, y);
            first = false;
        } else {
            builder.lineTo(x, y);
        }
    }
    return builder.detach();
}

static void test_convexity(skiatest::Reporter* reporter) {
    SkPathBuilder bulider;

    check_convexity(reporter, bulider.snapshot(), true);
    bulider.addCircle(0, 0, SkIntToScalar(10));
    check_convexity(reporter, bulider.snapshot(), true);
    bulider.addCircle(0, 0, SkIntToScalar(10));   // 2nd circle
    check_convexity(reporter, bulider.detach(), false);

    SkPath path = SkPath::Rect({0, 0, 10, 10}, SkPathDirection::kCCW);
    check_convexity(reporter, path, true);
    REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(path) == SkPathFirstDirection::kCCW);

    path = SkPath::Rect({0, 0, 10, 10}, SkPathDirection::kCW);
    check_convexity(reporter, path, true);
    REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(path) == SkPathFirstDirection::kCW);

    path = SkPathBuilder()
           .quadTo(100, 100, 50, 50) // This from GM:convexpaths
           .detach();
    check_convexity(reporter, path, true);

    static const struct {
        const char*           fPathStr;
        bool                  fExpectedIsConvex;
        SkPathFirstDirection  fExpectedDirection;
    } gRec[] = {
        { "", true, SkPathFirstDirection::kUnknown },
        { "0 0", true, SkPathFirstDirection::kUnknown },
        { "0 0 10 10", true, SkPathFirstDirection::kUnknown },
        { "0 0 10 10 20 20 0 0 10 10", false, SkPathFirstDirection::kUnknown },
        { "0 0 10 10 10 20", true, SkPathFirstDirection::kCW },
        { "0 0 10 10 10 0", true, SkPathFirstDirection::kCCW },
        { "0 0 10 10 10 0 0 10", false, kDontCheckDir },
        { "0 0 10 0 0 10 -10 -10", false, SkPathFirstDirection::kCW },
    };

    for (size_t i = 0; i < std::size(gRec); ++i) {
        path = setFromString(gRec[i].fPathStr);
        check_convexity(reporter, path, gRec[i].fExpectedIsConvex);
        check_direction(reporter, path, gRec[i].fExpectedDirection);
        // check after setting the initial convex and direction
        if (kDontCheckDir != gRec[i].fExpectedDirection) {
            // We make a copy so that we don't cache the result on the passed in path.
            SkPath copy(path);  // NOLINT(performance-unnecessary-copy-initialization)
            SkPathFirstDirection dir = SkPathPriv::ComputeFirstDirection(copy);
            bool foundDir = dir != SkPathFirstDirection::kUnknown;
            REPORTER_ASSERT(reporter, (gRec[i].fExpectedDirection == SkPathFirstDirection::kUnknown)
                    ^ foundDir);
            REPORTER_ASSERT(reporter, !foundDir || gRec[i].fExpectedDirection == dir);
            check_convexity(reporter, copy, gRec[i].fExpectedIsConvex);
        }
        REPORTER_ASSERT(reporter, gRec[i].fExpectedIsConvex == path.isConvex());
        check_direction(reporter, path, gRec[i].fExpectedDirection);
    }

    static const SkPoint axisAlignedPts[] = {
        { SK_ScalarMax, 0 },
        { 0, SK_ScalarMax },
        { SK_ScalarMin, 0 },
        { 0, SK_ScalarMin },
    };

    const size_t axisAlignedPtsCount = sizeof(axisAlignedPts) / sizeof(axisAlignedPts[0]);

    for (int index = 0; index < (int) (11 * axisAlignedPtsCount); ++index) {
        int f = (int) (index % axisAlignedPtsCount);
        int g = (int) ((f + 1) % axisAlignedPtsCount);

        SkPathBuilder builder;
        int curveSelect = index % 11;
        switch (curveSelect) {
            case 0: builder.moveTo(axisAlignedPts[f]); break;
            case 1: builder.lineTo(axisAlignedPts[f]); break;
            case 2: builder.quadTo(axisAlignedPts[f], axisAlignedPts[f]); break;
            case 3: builder.quadTo(axisAlignedPts[f], axisAlignedPts[g]); break;
            case 4: builder.quadTo(axisAlignedPts[g], axisAlignedPts[f]); break;
            case 5: builder.cubicTo(axisAlignedPts[f], axisAlignedPts[f], axisAlignedPts[f]); break;
            case 6: builder.cubicTo(axisAlignedPts[f], axisAlignedPts[f], axisAlignedPts[g]); break;
            case 7: builder.cubicTo(axisAlignedPts[f], axisAlignedPts[g], axisAlignedPts[f]); break;
            case 8: builder.cubicTo(axisAlignedPts[f], axisAlignedPts[g], axisAlignedPts[g]); break;
            case 9: builder.cubicTo(axisAlignedPts[g], axisAlignedPts[f], axisAlignedPts[f]); break;
            case 10: builder.cubicTo(axisAlignedPts[g], axisAlignedPts[f], axisAlignedPts[g]); break;
        }
        path = builder.detach();
        if (curveSelect == 0 || curveSelect == 1 || curveSelect == 2 || curveSelect == 5) {
            check_convexity(reporter, path, true);
        } else {
            // We make a copy so that we don't cache the result on the passed in path.
            SkPath copy(path);  // NOLINT(performance-unnecessary-copy-initialization)
            REPORTER_ASSERT(reporter, !copy.isConvex());
        }
    }

    static const SkPoint diagonalPts[] = {
        { SK_ScalarMax, SK_ScalarMax },
        { SK_ScalarMin, SK_ScalarMin },
    };

    const size_t diagonalPtsCount = sizeof(diagonalPts) / sizeof(diagonalPts[0]);

    for (int index = 0; index < (int) (7 * diagonalPtsCount); ++index) {
        int f = (int) (index % diagonalPtsCount);
        int g = (int) ((f + 1) % diagonalPtsCount);
        SkPathBuilder builder;
        int curveSelect = index % 11;
        switch (curveSelect) {
            case 0: builder.moveTo(diagonalPts[f]); break;
            case 1: builder.lineTo(diagonalPts[f]); break;
            case 2: builder.quadTo(diagonalPts[f], diagonalPts[f]); break;
            case 3: builder.quadTo(axisAlignedPts[f], diagonalPts[g]); break;
            case 4: builder.quadTo(diagonalPts[g], axisAlignedPts[f]); break;
            case 5: builder.cubicTo(diagonalPts[f], diagonalPts[f], diagonalPts[f]); break;
            case 6: builder.cubicTo(diagonalPts[f], diagonalPts[f], axisAlignedPts[g]); break;
            case 7: builder.cubicTo(diagonalPts[f], axisAlignedPts[g], diagonalPts[f]); break;
            case 8: builder.cubicTo(axisAlignedPts[f], diagonalPts[g], diagonalPts[g]); break;
            case 9: builder.cubicTo(diagonalPts[g], diagonalPts[f], axisAlignedPts[f]); break;
            case 10: builder.cubicTo(diagonalPts[g], axisAlignedPts[f], diagonalPts[g]); break;
        }
        path = builder.detach();
        if (curveSelect == 0) {
            check_convexity(reporter, path, true);
        } else {
            // We make a copy so that we don't cache the result on the passed in path.
            SkPath copy(path);  // NOLINT(performance-unnecessary-copy-initialization)
            REPORTER_ASSERT(reporter, !copy.isConvex());
        }
    }


    path = SkPathBuilder()
           .moveTo(SkBits2Float(0xbe9171db), SkBits2Float(0xbd7eeb5d))  // -0.284072f, -0.0622362f
           .lineTo(SkBits2Float(0xbe9171db), SkBits2Float(0xbd7eea38))  // -0.284072f, -0.0622351f
           .lineTo(SkBits2Float(0xbe9171a0), SkBits2Float(0xbd7ee5a7))  // -0.28407f, -0.0622307f
           .lineTo(SkBits2Float(0xbe917147), SkBits2Float(0xbd7ed886))  // -0.284067f, -0.0622182f
           .lineTo(SkBits2Float(0xbe917378), SkBits2Float(0xbd7ee1a9))  // -0.284084f, -0.0622269f
           .lineTo(SkBits2Float(0xbe9171db), SkBits2Float(0xbd7eeb5d))  // -0.284072f, -0.0622362f
           .close()
           .detach();
    check_convexity(reporter, path, false);

}

static void test_isLine(skiatest::Reporter* reporter) {
    SkPoint pts[2];
    const SkScalar value = SkIntToScalar(5);

    SkPath path;
    REPORTER_ASSERT(reporter, !path.isLine(nullptr));

    // set some non-zero values
    pts[0].set(value, value);
    pts[1].set(value, value);
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    // check that pts was untouched
    REPORTER_ASSERT(reporter, pts[0].equals(value, value));
    REPORTER_ASSERT(reporter, pts[1].equals(value, value));

    const SkScalar moveX = SkIntToScalar(1);
    const SkScalar moveY = SkIntToScalar(2);
    REPORTER_ASSERT(reporter, value != moveX && value != moveY);

    SkPathBuilder builder;
    builder.moveTo(moveX, moveY);
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, !path.isLine(nullptr));
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    // check that pts was untouched
    REPORTER_ASSERT(reporter, pts[0].equals(value, value));
    REPORTER_ASSERT(reporter, pts[1].equals(value, value));

    const SkScalar lineX = SkIntToScalar(2);
    const SkScalar lineY = SkIntToScalar(2);
    REPORTER_ASSERT(reporter, value != lineX && value != lineY);

    builder.lineTo(lineX, lineY);
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, path.isLine(nullptr));

    REPORTER_ASSERT(reporter, !pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, !pts[1].equals(lineX, lineY));
    REPORTER_ASSERT(reporter, path.isLine(pts));
    REPORTER_ASSERT(reporter, pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, pts[1].equals(lineX, lineY));

    builder.lineTo(0, 0);  // too many points/verbs
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, !path.isLine(nullptr));
    REPORTER_ASSERT(reporter, !path.isLine(pts));
    REPORTER_ASSERT(reporter, pts[0].equals(moveX, moveY));
    REPORTER_ASSERT(reporter, pts[1].equals(lineX, lineY));

    builder.reset();
    builder.quadTo(1, 1, 2, 2);
    path = builder.snapshot();
    REPORTER_ASSERT(reporter, !path.isLine(nullptr));
}

static void test_conservativelyContains(skiatest::Reporter* reporter) {
    // kBaseRect is used to construct most our test paths: a rect, a circle, and a round-rect.
    static const SkRect kBaseRect = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));

    // A circle that bounds kBaseRect (with a significant amount of slop)
    SkScalar circleR = std::max(kBaseRect.width(), kBaseRect.height());
    circleR *= 1.75f / 2;
    static const SkPoint kCircleC = {kBaseRect.centerX(), kBaseRect.centerY()};

    // round-rect radii
    static const SkScalar kRRRadii[] = {SkIntToScalar(5), SkIntToScalar(3)};

    static const struct SUPPRESS_VISIBILITY_WARNING {
        SkRect fQueryRect;
        bool   fInRect;
        bool   fInCircle;
        bool   fInRR;
        bool   fInCubicRR;
    } kQueries[] = {
        {kBaseRect, true, true, false, false},

        // rect well inside of kBaseRect
        {SkRect::MakeLTRB(kBaseRect.fLeft + 0.25f*kBaseRect.width(),
                          kBaseRect.fTop + 0.25f*kBaseRect.height(),
                          kBaseRect.fRight - 0.25f*kBaseRect.width(),
                          kBaseRect.fBottom - 0.25f*kBaseRect.height()),
                          true, true, true, true},

        // rects with edges off by one from kBaseRect's edges
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width(), kBaseRect.height() + 1),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width() + 1, kBaseRect.height()),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop,
                          kBaseRect.width() + 1, kBaseRect.height() + 1),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft - 1, kBaseRect.fTop,
                          kBaseRect.width(), kBaseRect.height()),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop - 1,
                          kBaseRect.width(), kBaseRect.height()),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft - 1, kBaseRect.fTop,
                          kBaseRect.width() + 2, kBaseRect.height()),
         false, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop - 1,
                          kBaseRect.width() + 2, kBaseRect.height()),
         false, true, false, false},

        // zero-w/h rects at each corner of kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fTop, 0, 0), true, true, false, false},
        {SkRect::MakeXYWH(kBaseRect.fRight, kBaseRect.fTop, 0, 0), true, true, false, true},
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.fBottom, 0, 0), true, true, false, true},
        {SkRect::MakeXYWH(kBaseRect.fRight, kBaseRect.fBottom, 0, 0), true, true, false, true},

        // far away rect
        {SkRect::MakeXYWH(10 * kBaseRect.fRight, 10 * kBaseRect.fBottom,
                          SkIntToScalar(10), SkIntToScalar(10)),
         false, false, false, false},

        // very large rect containing kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft - 5 * kBaseRect.width(),
                          kBaseRect.fTop - 5 * kBaseRect.height(),
                          11 * kBaseRect.width(), 11 * kBaseRect.height()),
         false, false, false, false},

        // skinny rect that spans same y-range as kBaseRect
        {SkRect::MakeXYWH(kBaseRect.centerX(), kBaseRect.fTop,
                          SkIntToScalar(1), kBaseRect.height()),
         true, true, true, true},

        // short rect that spans same x-range as kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.centerY(), kBaseRect.width(), SkScalar(1)),
         true, true, true, true},

        // skinny rect that spans slightly larger y-range than kBaseRect
        {SkRect::MakeXYWH(kBaseRect.centerX(), kBaseRect.fTop,
                          SkIntToScalar(1), kBaseRect.height() + 1),
         false, true, false, false},

        // short rect that spans slightly larger x-range than kBaseRect
        {SkRect::MakeXYWH(kBaseRect.fLeft, kBaseRect.centerY(),
                          kBaseRect.width() + 1, SkScalar(1)),
         false, true, false, false},
    };

    for (int inv = 0; inv < 4; ++inv) {
        for (size_t q = 0; q < std::size(kQueries); ++q) {
            SkRect qRect = kQueries[q].fQueryRect;
            if (inv & 0x1) {
                using std::swap;
                swap(qRect.fLeft, qRect.fRight);
            }
            if (inv & 0x2) {
                using std::swap;
                swap(qRect.fTop, qRect.fBottom);
            }
            for (int d = 0; d < 2; ++d) {
                SkPathDirection dir = d ? SkPathDirection::kCCW : SkPathDirection::kCW;
                SkPath path = SkPath::Rect(kBaseRect, dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInRect ==
                                          path.conservativelyContainsRect(qRect));

                path = SkPath::Circle(kCircleC.fX, kCircleC.fY, circleR, dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInCircle ==
                                          path.conservativelyContainsRect(qRect));

                path = SkPath::RRect(kBaseRect, kRRRadii[0], kRRRadii[1], dir);
                REPORTER_ASSERT(reporter, kQueries[q].fInRR ==
                                          path.conservativelyContainsRect(qRect));

                path = SkPathBuilder()
                       .moveTo(kBaseRect.fLeft + kRRRadii[0], kBaseRect.fTop)
                       .cubicTo(kBaseRect.fLeft + kRRRadii[0] / 2, kBaseRect.fTop,
                                kBaseRect.fLeft, kBaseRect.fTop + kRRRadii[1] / 2,
                                kBaseRect.fLeft, kBaseRect.fTop + kRRRadii[1])
                       .lineTo(kBaseRect.fLeft, kBaseRect.fBottom)
                       .lineTo(kBaseRect.fRight, kBaseRect.fBottom)
                       .lineTo(kBaseRect.fRight, kBaseRect.fTop)
                       .close()
                       .detach();
                REPORTER_ASSERT(reporter, kQueries[q].fInCubicRR ==
                                          path.conservativelyContainsRect(qRect));

            }
            // Slightly non-convex shape, shouldn't contain any rects.
            SkPath path = SkPathBuilder()
                          .moveTo(0, 0)
                          .lineTo(SkIntToScalar(50), 0.05f)
                          .lineTo(SkIntToScalar(100), 0)
                          .lineTo(SkIntToScalar(100), SkIntToScalar(100))
                          .lineTo(0, SkIntToScalar(100))
                          .close()
                          .detach();
            REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(qRect));
        }
    }

    // make sure a minimal convex shape works, a right tri with edges along pos x and y axes.
    SkPathBuilder builder;
    builder.moveTo(0, 0);
    builder.lineTo(SkIntToScalar(100), 0);
    builder.lineTo(0, SkIntToScalar(100));
    SkPath path = builder.snapshot();

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


    // Test that multiple move commands do not cause asserts.
    builder.moveTo(SkIntToScalar(100), SkIntToScalar(100));
    path = builder.detach();
    REPORTER_ASSERT(reporter, path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(50), 0,
                                                                               SkIntToScalar(10),
                                                                               SkIntToScalar(10))));

    // Same as above path and first test but with an extra moveTo.
    path = SkPathBuilder()
           .moveTo(100, 100)
           .moveTo(0, 0)
           .lineTo(SkIntToScalar(100), 0)
           .lineTo(0, SkIntToScalar(100))
           .detach();
    // Convexity logic treats a path as filled and closed, so that multiple (non-trailing) moveTos
    // have no effect on convexity
    REPORTER_ASSERT(reporter, path.conservativelyContainsRect(
        SkRect::MakeXYWH(SkIntToScalar(50), 0,
                         SkIntToScalar(10),
                         SkIntToScalar(10))));

    // Same as above path and first test but with the extra moveTo making a degenerate sub-path
    // following the non-empty sub-path. Verifies that this does not trigger assertions.
    path = SkPathBuilder()
           .moveTo(0, 0)
           .lineTo(SkIntToScalar(100), 0)
           .lineTo(0, SkIntToScalar(100))
           .moveTo(100, 100)
           .detach();

    REPORTER_ASSERT(reporter, path.conservativelyContainsRect(SkRect::MakeXYWH(SkIntToScalar(50), 0,
                                                                               SkIntToScalar(10),
                                                                               SkIntToScalar(10))));

    // Test that multiple move commands do not cause asserts and that the function
    // is not confused by the multiple moves.
    path = SkPathBuilder()
           .moveTo(0, 0)
           .lineTo(SkIntToScalar(100), 0)
           .lineTo(0, SkIntToScalar(100))
           .moveTo(0, SkIntToScalar(200))
           .lineTo(SkIntToScalar(100), SkIntToScalar(200))
           .lineTo(0, SkIntToScalar(300))
           .detach();

    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(
                                                            SkRect::MakeXYWH(SkIntToScalar(50), 0,
                                                                             SkIntToScalar(10),
                                                                             SkIntToScalar(10))));

    path = SkPathBuilder().lineTo(100, 100).detach();
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeXYWH(0, 0, 1, 1)));

    // An empty path should not contain any rectangle. It's questionable whether an empty path
    // contains an empty rectangle. However, since it is a conservative test it is ok to
    // return false.
    path = SkPath();
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(1,1)));
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(0,0)));

    path = SkPathBuilder()
           .moveTo(50, 50)
           .cubicTo(0, 0, 100, 0, 50, 50)
           .detach();
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(100, 100)));
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(30, 30)));
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(1,1)));
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(0,0)));

    path = SkPathBuilder()
           .moveTo(50, 50)
           .quadTo(100, 100, 50, 50)
           .detach();
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(1,1)));
    REPORTER_ASSERT(reporter, !path.conservativelyContainsRect(SkRect::MakeWH(0,0)));
}

static void test_isRect_open_close(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0).lineTo(1, 0).lineTo(1, 1).lineTo(0, 1)
                  .close()
                  .detach();

    bool isClosed;
    REPORTER_ASSERT(reporter, path.isRect(nullptr, &isClosed, nullptr));
    REPORTER_ASSERT(reporter, isClosed);
}

// Simple isRect test is inline TestPath, below.
// test_isRect provides more extensive testing.
static void test_isRect(skiatest::Reporter* reporter) {
    test_isRect_open_close(reporter);

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
    SkPoint ra[] = {{0, 0}, {0, .5f}, {0, 1}, {.5f, 1}, {1, 1}, {1, .5f}, {1, 0}, {.5f, 0}};
    SkPoint rb[] = {{0, 0}, {.5f, 0}, {1, 0}, {1, .5f}, {1, 1}, {.5f, 1}, {0, 1}, {0, .5f}};
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

    // no close, but we should detect them as fillably the same as a rect
    SkPoint c1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    SkPoint c2[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}};
    SkPoint c3[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}, {0, 0}}; // hit the start

    // like c2, but we double-back on ourselves
    SkPoint d1[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}, {0, 2}};
    // like c2, but we overshoot the start point
    SkPoint d2[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, -1}};
    SkPoint d3[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, -1}, {0, 0}};

    struct IsRectTest {
        SkPoint *fPoints;
        int fPointCount;
        bool fClose;
        bool fIsRect;
    } tests[] = {
        { r1, std::size(r1), true, true },
        { r2, std::size(r2), true, true },
        { r3, std::size(r3), true, true },
        { r4, std::size(r4), true, true },
        { r5, std::size(r5), true, true },
        { r6, std::size(r6), true, true },
        { r7, std::size(r7), true, true },
        { r8, std::size(r8), true, true },
        { r9, std::size(r9), true, true },
        { ra, std::size(ra), true, true },
        { rb, std::size(rb), true, true },
        { rc, std::size(rc), true, true },
        { rd, std::size(rd), true, true },
        { re, std::size(re), true, true },
        { rf, std::size(rf), true, true },

        { f1, std::size(f1), true, false },
        { f2, std::size(f2), true, false },
        { f3, std::size(f3), true, false },
        { f4, std::size(f4), true, false },
        { f5, std::size(f5), true, false },
        { f6, std::size(f6), true, false },
        { f7, std::size(f7), true, false },
        { f8, std::size(f8), true, false },
        { f9, std::size(f9), true, false },
        { fa, std::size(fa), true, false },
        { fb, std::size(fb), true, false },

        { c1, std::size(c1), false, true },
        { c2, std::size(c2), false, true },
        { c3, std::size(c3), false, true },

        { d1, std::size(d1), false, false },
        { d2, std::size(d2), false, true },
        { d3, std::size(d3), false, false },
    };

    const size_t testCount = std::size(tests);
    int index;
    for (size_t testIndex = 0; testIndex < testCount; ++testIndex) {
        SkPathBuilder builder;
        builder.moveTo(tests[testIndex].fPoints[0].fX, tests[testIndex].fPoints[0].fY);
        for (index = 1; index < tests[testIndex].fPointCount; ++index) {
            builder.lineTo(tests[testIndex].fPoints[index].fX, tests[testIndex].fPoints[index].fY);
        }
        if (tests[testIndex].fClose) {
            builder.close();
        }
        SkPath path = builder.detach();
        REPORTER_ASSERT(reporter, tests[testIndex].fIsRect == path.isRect(nullptr));

        if (tests[testIndex].fIsRect) {
            SkRect computed, expected;
            bool isClosed;
            SkPathDirection direction;
            size_t pointCount = tests[testIndex].fPointCount - (d2 == tests[testIndex].fPoints);
            expected.setBounds({tests[testIndex].fPoints, pointCount});
            SkPathFirstDirection cheapDirection = SkPathPriv::ComputeFirstDirection(path);
            REPORTER_ASSERT(reporter, cheapDirection != SkPathFirstDirection::kUnknown);
            REPORTER_ASSERT(reporter, path.isRect(&computed, &isClosed, &direction));
            REPORTER_ASSERT(reporter, expected == computed);
            REPORTER_ASSERT(reporter, isClosed == tests[testIndex].fClose);
            REPORTER_ASSERT(reporter, SkPathPriv::AsFirstDirection(direction) == cheapDirection);
        } else {
            SkRect computed;
            computed.setLTRB(123, 456, 789, 1011);
            for (auto c : {true, false})
            for (auto d : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
              bool isClosed = c;
              SkPathDirection direction = d;
              REPORTER_ASSERT(reporter, !path.isRect(&computed, &isClosed, &direction));
              REPORTER_ASSERT(reporter, computed.fLeft == 123 && computed.fTop == 456);
              REPORTER_ASSERT(reporter, computed.fRight == 789 && computed.fBottom == 1011);
              REPORTER_ASSERT(reporter, isClosed == c);
              REPORTER_ASSERT(reporter, direction == d);
            }
        }
    }

    // fail, close then line
    SkPathBuilder builder;
    builder.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < SkToInt(std::size(r1)); ++index) {
        builder.lineTo(r1[index].fX, r1[index].fY);
    }
    builder.close();
    builder.lineTo(1, 0);
    SkPath path1 = builder.detach();
    REPORTER_ASSERT(reporter, !path1.isRect(nullptr));

    // fail, move in the middle
    builder.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < SkToInt(std::size(r1)); ++index) {
        if (index == 2) {
            builder.moveTo(1, .5f);
        }
        builder.lineTo(r1[index].fX, r1[index].fY);
    }
    builder.close();
    path1 = builder.detach();
    REPORTER_ASSERT(reporter, !path1.isRect(nullptr));

    // fail, move on the edge
    for (index = 1; index < SkToInt(std::size(r1)); ++index) {
        builder.moveTo(r1[index - 1].fX, r1[index - 1].fY);
        builder.lineTo(r1[index].fX, r1[index].fY);
    }
    builder.close();
    path1 = builder.detach();
    REPORTER_ASSERT(reporter, !path1.isRect(nullptr));

    // fail, quad
    builder.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < SkToInt(std::size(r1)); ++index) {
        if (index == 2) {
            builder.quadTo(1, .5f, 1, .5f);
        }
        builder.lineTo(r1[index].fX, r1[index].fY);
    }
    builder.close();
    path1 = builder.detach();
    REPORTER_ASSERT(reporter, !path1.isRect(nullptr));

    // fail, cubic
    builder.moveTo(r1[0].fX, r1[0].fY);
    for (index = 1; index < SkToInt(std::size(r1)); ++index) {
        if (index == 2) {
            builder.cubicTo(1, .5f, 1, .5f, 1, .5f);
        }
        builder.lineTo(r1[index].fX, r1[index].fY);
    }
    builder.close();
    path1 = builder.detach();
    REPORTER_ASSERT(reporter, !path1.isRect(nullptr));
}

static void check_simple_rect(skiatest::Reporter* reporter, const SkPath& path, bool isClosed,
                              const SkRect& rect, SkPathDirection dir, unsigned start) {
    auto info = SkPathPriv::IsSimpleRect(path, false);
    REPORTER_ASSERT(reporter, info.has_value() == isClosed);

    info = SkPathPriv::IsSimpleRect(path, true);
    REPORTER_ASSERT(reporter, info.has_value());
    REPORTER_ASSERT(reporter, info->fRect       == rect);
    REPORTER_ASSERT(reporter, info->fDirection  == dir);
    REPORTER_ASSERT(reporter, info->fStartIndex == start);
}

static void test_is_closed_rect(skiatest::Reporter* reporter) {
    using std::swap;
    const SkRect testRect = SkRect::MakeXYWH(10, 10, 50, 70);
    const SkRect emptyRect = SkRect::MakeEmpty();
    for (int start = 0; start < 4; ++start) {
        for (auto dir : {SkPathDirection::kCCW, SkPathDirection::kCW}) {
            SkPathBuilder builder;
            builder.addRect(testRect, dir, start);
            check_simple_rect(reporter, builder.snapshot(), true, testRect, dir, start);
            builder.close();
            check_simple_rect(reporter, builder.snapshot(), true, testRect, dir, start);

            SkPathBuilder builder2 = builder;
            builder2.lineTo(10, 10);
            SkPath path2 = builder2.detach();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));

            builder2 = builder;
            builder2.moveTo(10, 10);
            path2 = builder2.detach();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));

            builder2 = builder;
            builder2.addRect(testRect, dir, start);
            path2 = builder2.detach();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));

            // Make the path by hand, manually closing it.
            builder2.reset();
            path2 = builder.detach();
            SkPoint firstPt = {0.f, 0.f};
            for (auto [v, verbPts, w] : SkPathPriv::Iterate(path2)) {
                switch(v) {
                    case SkPathVerb::kMove:
                        firstPt = verbPts[0];
                        builder2.moveTo(verbPts[0]);
                        break;
                    case SkPathVerb::kLine:
                        builder2.lineTo(verbPts[1]);
                        break;
                    default:
                        break;
                }
            }
            path2 = builder2.snapshot();
            // We haven't closed it yet...
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));
            // ... now we do and test again.
            builder2.lineTo(firstPt);
            check_simple_rect(reporter, builder2.snapshot(), false, testRect, dir, start);
            // A redundant close shouldn't cause a failure.
            builder2.close();
            check_simple_rect(reporter, builder2.snapshot(), true, testRect, dir, start);

            // Degenerate point and line rects are not allowed
            builder2.reset();
            builder2.addRect(emptyRect, dir, start);
            path2 = builder2.snapshot();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));
            SkRect degenRect = testRect;
            degenRect.fLeft = degenRect.fRight;

            builder2.reset();
            builder2.addRect(degenRect, dir, start);
            path2 = builder2.snapshot();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));
            degenRect = testRect;
            degenRect.fTop = degenRect.fBottom;

            builder2.reset();
            builder2.addRect(degenRect, dir, start);
            path2 = builder2.snapshot();
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, false));
            REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path2, true));

            // An inverted rect makes a rect path, but changes the winding dir and start point.
            SkPathDirection swapDir = (dir == SkPathDirection::kCW)
                                            ? SkPathDirection::kCCW
                                            : SkPathDirection::kCW;
            static constexpr unsigned kXSwapStarts[] = { 1, 0, 3, 2 };
            static constexpr unsigned kYSwapStarts[] = { 3, 2, 1, 0 };
            SkRect swapRect = testRect;
            swap(swapRect.fLeft, swapRect.fRight);
            builder2.reset();
            builder2.addRect(swapRect, dir, start);
            path2 = builder2.snapshot();
            check_simple_rect(reporter, path2, true, testRect, swapDir, kXSwapStarts[start]);
            swapRect = testRect;
            swap(swapRect.fTop, swapRect.fBottom);

            builder2.reset();
            builder2.addRect(swapRect, dir, start);
            path2 = builder2.snapshot();
            check_simple_rect(reporter, path2, true, testRect, swapDir, kYSwapStarts[start]);
        }
    }
    // down, up, left, close
    SkPath path = SkPathBuilder()
                  .moveTo(1, 1)
                  .lineTo(1, 2)
                  .lineTo(1, 1)
                  .lineTo(0, 1)
                  .close()
                  .detach();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, false));
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, true));
    // right, left, up, close
    path = SkPathBuilder()
           .moveTo(1, 1)
           .lineTo(2, 1)
           .lineTo(1, 1)
           .lineTo(1, 0)
           .close()
           .detach();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, false));
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, true));
    // parallelogram with horizontal edges
    path = SkPathBuilder()
           .moveTo(1, 0)
           .lineTo(3, 0)
           .lineTo(2, 1)
           .lineTo(0, 1)
           .close()
           .detach();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, false));
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, true));
    // parallelogram with vertical edges
    path = SkPathBuilder()
           .moveTo(0, 1)
           .lineTo(0, 3)
           .lineTo(1, 2)
           .lineTo(1, 0)
           .close()
           .detach();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, false));
    REPORTER_ASSERT(reporter, !SkPathPriv::IsSimpleRect(path, true));

}

static void test_isNestedFillRects(skiatest::Reporter* reporter) {
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
    SkPoint ra[] = {{0, 0}, {0, .5f}, {0, 1}, {.5f, 1}, {1, 1}, {1, .5f}, {1, 0}, {.5f, 0}}; // CCW
    SkPoint rb[] = {{0, 0}, {.5f, 0}, {1, 0}, {1, .5f}, {1, 1}, {.5f, 1}, {0, 1}, {0, .5f}}; // CW
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

    // success, no close is OK
    SkPoint c1[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}}; // close doesn't match
    SkPoint c2[] = {{0, 0}, {1, 0}, {1, 2}, {0, 2}, {0, 1}}; // ditto

    struct IsNestedRectTest {
        SkPoint *fPoints;
        size_t fPointCount;
        SkPathFirstDirection fDirection;
        bool fClose;
        bool fIsNestedRect; // nests with path.addRect(-1, -1, 2, 2);
    } tests[] = {
        { r1, std::size(r1), SkPathFirstDirection::kCW , true, true },
        { r2, std::size(r2), SkPathFirstDirection::kCW , true, true },
        { r3, std::size(r3), SkPathFirstDirection::kCW , true, true },
        { r4, std::size(r4), SkPathFirstDirection::kCW , true, true },
        { r5, std::size(r5), SkPathFirstDirection::kCCW, true, true },
        { r6, std::size(r6), SkPathFirstDirection::kCCW, true, true },
        { r7, std::size(r7), SkPathFirstDirection::kCCW, true, true },
        { r8, std::size(r8), SkPathFirstDirection::kCCW, true, true },
        { r9, std::size(r9), SkPathFirstDirection::kCCW, true, true },
        { ra, std::size(ra), SkPathFirstDirection::kCCW, true, true },
        { rb, std::size(rb), SkPathFirstDirection::kCW,  true, true },
        { rc, std::size(rc), SkPathFirstDirection::kCW,  true, true },
        { rd, std::size(rd), SkPathFirstDirection::kCCW, true, true },
        { re, std::size(re), SkPathFirstDirection::kCW,  true, true },

        { f1, std::size(f1), SkPathFirstDirection::kUnknown, true, false },
        { f2, std::size(f2), SkPathFirstDirection::kUnknown, true, false },
        { f3, std::size(f3), SkPathFirstDirection::kUnknown, true, false },
        { f4, std::size(f4), SkPathFirstDirection::kUnknown, true, false },
        { f5, std::size(f5), SkPathFirstDirection::kUnknown, true, false },
        { f6, std::size(f6), SkPathFirstDirection::kUnknown, true, false },
        { f7, std::size(f7), SkPathFirstDirection::kUnknown, true, false },
        { f8, std::size(f8), SkPathFirstDirection::kUnknown, true, false },

        { c1, std::size(c1), SkPathFirstDirection::kCW, false, true },
        { c2, std::size(c2), SkPathFirstDirection::kCW, false, true },
    };

    const size_t testCount = std::size(tests);
    for (int rectFirst = 0; rectFirst <= 1; ++rectFirst) {
        for (size_t testIndex = 0; testIndex < testCount; ++testIndex) {
            SkPathBuilder builder;
            if (rectFirst) {
                builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
            }
            builder.moveTo(tests[testIndex].fPoints[0].fX, tests[testIndex].fPoints[0].fY);
            for (size_t index = 1; index < tests[testIndex].fPointCount; ++index) {
                builder.lineTo(tests[testIndex].fPoints[index].fX, tests[testIndex].fPoints[index].fY);
            }
            if (tests[testIndex].fClose) {
                builder.close();
            }
            if (!rectFirst) {
                builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
            }
            SkPath path = builder.detach();
            REPORTER_ASSERT(reporter,
                            tests[testIndex].fIsNestedRect == SkPathPriv::IsNestedFillRects(path, nullptr));
            if (tests[testIndex].fIsNestedRect) {
                SkRect expected[2], computed[2];
                SkPathFirstDirection expectedDirs[2];
                SkPathDirection computedDirs[2];
                SkRect testBounds = SkRect::BoundsOrEmpty({tests[testIndex].fPoints,
                                                           tests[testIndex].fPointCount});
                expected[0] = SkRect::MakeLTRB(-1, -1, 2, 2);
                expected[1] = testBounds;
                if (rectFirst) {
                    expectedDirs[0] = SkPathFirstDirection::kCW;
                } else {
                    expectedDirs[0] = SkPathFirstDirection::kCCW;
                }
                expectedDirs[1] = tests[testIndex].fDirection;
                REPORTER_ASSERT(reporter, SkPathPriv::IsNestedFillRects(path, computed, computedDirs));
                REPORTER_ASSERT(reporter, expected[0] == computed[0]);
                REPORTER_ASSERT(reporter, expected[1] == computed[1]);
                REPORTER_ASSERT(reporter, expectedDirs[0] == SkPathPriv::AsFirstDirection(computedDirs[0]));
                REPORTER_ASSERT(reporter, expectedDirs[1] == SkPathPriv::AsFirstDirection(computedDirs[1]));
            }
        }

        // fail, close then line
        SkPathBuilder builder;
        if (rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
        }
        builder.moveTo(r1[0].fX, r1[0].fY);
        for (size_t index = 1; index < std::size(r1); ++index) {
            builder.lineTo(r1[index].fX, r1[index].fY);
        }
        builder.close();
        builder.lineTo(1, 0);
        if (!rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
        }
        SkPath path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));

        // fail, move in the middle
        if (rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
        }
        builder.moveTo(r1[0].fX, r1[0].fY);
        for (size_t index = 1; index < std::size(r1); ++index) {
            if (index == 2) {
                builder.moveTo(1, .5f);
            }
            builder.lineTo(r1[index].fX, r1[index].fY);
        }
        builder.close();
        if (!rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
        }
        path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));

        // fail, move on the edge
        if (rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
        }
        for (size_t index = 1; index < std::size(r1); ++index) {
            builder.moveTo(r1[index - 1].fX, r1[index - 1].fY);
            builder.lineTo(r1[index].fX, r1[index].fY);
        }
        builder.close();
        if (!rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
        }
        path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));

        // fail, quad
        if (rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
        }
        builder.moveTo(r1[0].fX, r1[0].fY);
        for (size_t index = 1; index < std::size(r1); ++index) {
            if (index == 2) {
                builder.quadTo(1, .5f, 1, .5f);
            }
            builder.lineTo(r1[index].fX, r1[index].fY);
        }
        builder.close();
        if (!rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
        }
        path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));

        // fail, cubic
        if (rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCW);
        }
        builder.moveTo(r1[0].fX, r1[0].fY);
        for (size_t index = 1; index < std::size(r1); ++index) {
            if (index == 2) {
                builder.cubicTo(1, .5f, 1, .5f, 1, .5f);
            }
            builder.lineTo(r1[index].fX, r1[index].fY);
        }
        builder.close();
        if (!rectFirst) {
            builder.addRect({-1, -1, 2, 2}, SkPathDirection::kCCW);
        }
        path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));

        // fail,  not nested
        builder.addRect({1, 1, 3, 3}, SkPathDirection::kCW);
        builder.addRect({2, 2, 4, 4}, SkPathDirection::kCW);
        path1 = builder.detach();
        REPORTER_ASSERT(reporter, !SkPathPriv::IsNestedFillRects(path1, nullptr));
    }

    //  pass, constructed explicitly from manually closed rects specified as moves/lines.
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .lineTo(10, 0)
                  .lineTo(10, 10)
                  .lineTo(0, 10)
                  .lineTo(0, 0)
                  .moveTo(1, 1)
                  .lineTo(9, 1)
                  .lineTo(9, 9)
                  .lineTo(1, 9)
                  .lineTo(1, 1)
                  .detach();
    REPORTER_ASSERT(reporter, SkPathPriv::IsNestedFillRects(path, nullptr));

    // pass, stroke rect
    SkPath src = SkPath::Rect({1, 1, 7, 7}, SkPathDirection::kCW);
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(2);
    SkPath dst = skpathutils::FillPathWithPaint(src, strokePaint);
    REPORTER_ASSERT(reporter, SkPathPriv::IsNestedFillRects(dst, nullptr));
}

static void write_and_read_back(skiatest::Reporter* reporter,
                                const SkPath& p) {
    SkBinaryWriteBuffer writer({});
    writer.writePath(p);
    size_t size = writer.bytesWritten();
    SkAutoMalloc storage(size);
    writer.writeToMemory(storage.get());
    SkReadBuffer reader(storage.get(), size);

    auto readBack = reader.readPath();
    REPORTER_ASSERT(reporter, readBack.has_value() && *readBack == p);

    REPORTER_ASSERT(reporter, SkPathPriv::GetConvexity(*readBack) ==
                              SkPathPriv::GetConvexity(p));

    REPORTER_ASSERT(reporter, readBack->isOval(nullptr) == p.isOval(nullptr));
    std::optional<SkPathOvalInfo> oval0 = SkPathPriv::IsOval(p),
                                  oval1 = SkPathPriv::IsOval(*readBack);
    if (oval0 && oval1) {
        REPORTER_ASSERT(reporter, oval0->fBounds     == oval1->fBounds);
        REPORTER_ASSERT(reporter, oval0->fDirection  == oval1->fDirection);
        REPORTER_ASSERT(reporter, oval0->fStartIndex == oval1->fStartIndex);
    }

    REPORTER_ASSERT(reporter, readBack->isRRect(nullptr) == p.isRRect(nullptr));
    std::optional<SkPathRRectInfo> rrect0 = SkPathPriv::IsRRect(p),
                                   rrect1 = SkPathPriv::IsRRect(*readBack);
    if (rrect0 && rrect1) {
        REPORTER_ASSERT(reporter, rrect0->fRRect      == rrect1->fRRect);
        REPORTER_ASSERT(reporter, rrect0->fDirection  == rrect1->fDirection);
        REPORTER_ASSERT(reporter, rrect0->fStartIndex == rrect1->fStartIndex);
    }

    const SkRect& origBounds = p.getBounds();
    const SkRect& readBackBounds = readBack->getBounds();
    REPORTER_ASSERT(reporter, origBounds == readBackBounds);
}

static void test_flattening(skiatest::Reporter* reporter) {
    static const SkPoint pts[] = {
        { 0, 0 },
        { SkIntToScalar(10), SkIntToScalar(10) },
        { SkIntToScalar(20), SkIntToScalar(10) }, { SkIntToScalar(20), 0 },
        { 0, 0 }, { 0, SkIntToScalar(10) }, { SkIntToScalar(1), SkIntToScalar(10) }
    };
    SkPath p = SkPathBuilder()
               .moveTo(pts[0])
               .lineTo(pts[1])
               .quadTo(pts[2], pts[3])
               .cubicTo(pts[4], pts[5], pts[6])
               .detach();

    write_and_read_back(reporter, p);

    // create a buffer that should be much larger than the path so we don't
    // kill our stack if writer goes too far.
    char buffer[1024];
    size_t size1 = p.writeToMemory(nullptr);
    size_t size2 = p.writeToMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);

    size_t size3 = 0;
    auto p2 = SkPath::ReadFromMemory(buffer, 1024, &size3);
    REPORTER_ASSERT(reporter, p2.has_value());
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, p == *p2);

    auto missing = SkPath::ReadFromMemory(buffer, 0, &size3);
    REPORTER_ASSERT(reporter, !missing.has_value());

    missing = SkPath::ReadFromMemory(buffer, size1 - 1, &size3);
    REPORTER_ASSERT(reporter, !missing.has_value());

    char buffer2[1024];
    size3 = p2->writeToMemory(buffer2);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);

    // test persistence of the oval flag & convexity
    {
        SkPath oval = SkPath::Oval(SkRect::MakeWH(10, 10));
        write_and_read_back(reporter, oval);
    }
}

static void test_transform(skiatest::Reporter* reporter) {
#define CONIC_PERSPECTIVE_BUG_FIXED 0
    static const SkPoint pts[] = {
        { 0, 0 },  // move
        { SkIntToScalar(10), SkIntToScalar(10) },  // line
        { SkIntToScalar(20), SkIntToScalar(10) }, { SkIntToScalar(20), 0 },  // quad
        { 0, 0 }, { 0, SkIntToScalar(10) }, { SkIntToScalar(1), SkIntToScalar(10) },  // cubic
#if CONIC_PERSPECTIVE_BUG_FIXED
        { 0, 0 }, { SkIntToScalar(20), SkIntToScalar(10) },  // conic
#endif
    };
    const int kPtCount = std::size(pts);

    SkPath p = SkPathBuilder()
               .moveTo(pts[0])
               .lineTo(pts[1])
               .quadTo(pts[2], pts[3])
               .cubicTo(pts[4], pts[5], pts[6])
#if CONIC_PERSPECTIVE_BUG_FIXED
               .conicTo(pts[4], pts[5], 0.5f)
#endif
               .close()
               .detach();

    {
        SkPath p1 = p.makeTransform(SkMatrix::I());
        REPORTER_ASSERT(reporter, p == p1);
    }


    {
        SkMatrix matrix;
        matrix.setScale(SK_Scalar1 * 2, SK_Scalar1 * 3);

        SkPath p1 = p.makeTransform(matrix);
        SkSpan<const SkPoint> pts1 = p1.points();
        REPORTER_ASSERT(reporter, kPtCount == pts1.size());
        for (size_t i = 0; i < pts1.size(); ++i) {
            SkPoint newPt = SkPoint::Make(pts[i].fX * 2, pts[i].fY * 3);
            REPORTER_ASSERT(reporter, newPt == pts1[i]);
        }
    }

    {
        SkMatrix matrix;
        matrix.reset();
        matrix.setPerspX(4);

        SkPath p1 = p.makeTransform(matrix);

        REPORTER_ASSERT(reporter, matrix.invert(&matrix));
        p1 = p1.makeTransform(matrix);
        SkRect pBounds = p.getBounds();
        SkRect p1Bounds = p1.getBounds();
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fLeft, p1Bounds.fLeft));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fTop, p1Bounds.fTop));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fRight, p1Bounds.fRight));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fBottom, p1Bounds.fBottom));
    }

    p = SkPath::Circle(0, 0, 1, SkPathDirection::kCW);

    {
        SkPath p1 = p.makeTransform(SkMatrix::I());
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(p1) == SkPathFirstDirection::kCW);
    }

    {
        SkMatrix matrix;
        matrix.setScaleX(-1);
        SkPath p1 = p.makeTransform(matrix);
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(p1) == SkPathFirstDirection::kCCW);
    }

    {
        SkMatrix matrix;
        matrix.setAll(1, 1, 0, 1, 1, 0, 0, 0, 1);
        SkPath p1 = p.makeTransform(matrix);
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(p1) == SkPathFirstDirection::kUnknown);
    }

    {
        SkPath p1 = SkPath::Rect({ 10, 20, 30, 40 });
        SkPath p2 = SkPath::Rect({ 10, 20, 30, 40 });
        uint32_t id1 = p1.getGenerationID();
        uint32_t id2 = p2.getGenerationID();
        REPORTER_ASSERT(reporter, id1 != id2);
        SkMatrix matrix;
        matrix.setScale(2, 2);
        p2 = p1.makeTransform(matrix);
        REPORTER_ASSERT(reporter, id1 == p1.getGenerationID());
        REPORTER_ASSERT(reporter, id2 != p2.getGenerationID());
        p1 = p1.makeTransform(matrix);
        REPORTER_ASSERT(reporter, id1 != p1.getGenerationID());
    }
}

static void test_zero_length_paths(skiatest::Reporter* reporter) {
    struct SUPPRESS_VISIBILITY_WARNING zeroPathTestData {
        const char* testPath;
        const size_t numResultPts;
        const SkRect resultBound;
        const SkPath::Verb* resultVerbs;
        const size_t numResultVerbs;
    };

    static const SkPath::Verb resultVerbs1[] = { SkPath::kMove_Verb };
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
        { "M 1 1", 1, {1, 1, 1, 1}, resultVerbs1, std::size(resultVerbs1) },
        { "M 1 1 z", 1, {1, 1, 1, 1}, resultVerbs3, std::size(resultVerbs3) },
        { "M 1 1 z M 2 1 z", 2, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs4, std::size(resultVerbs4) },
        { "M 1 1 L 1 1", 2, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs5, std::size(resultVerbs5) },
        { "M 1 1 L 1 1 M 2 1 L 2 1", 4, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs6, std::size(resultVerbs6) },
        { "M 1 1 L 1 1 z", 2, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs7, std::size(resultVerbs7) },
        { "M 1 1 L 1 1 z M 2 1 L 2 1 z", 4, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs8, std::size(resultVerbs8) },
        { "M 1 1 Q 1 1 1 1", 3, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs9, std::size(resultVerbs9) },
        { "M 1 1 Q 1 1 1 1 M 2 1 Q 2 1 2 1", 6, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs10, std::size(resultVerbs10) },
        { "M 1 1 Q 1 1 1 1 z", 3, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs11, std::size(resultVerbs11) },
        { "M 1 1 Q 1 1 1 1 z M 2 1 Q 2 1 2 1 z", 6, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs12, std::size(resultVerbs12) },
        { "M 1 1 C 1 1 1 1 1 1", 4, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs13, std::size(resultVerbs13) },
        { "M 1 1 C 1 1 1 1 1 1 M 2 1 C 2 1 2 1 2 1", 8, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs14,
            std::size(resultVerbs14)
        },
        { "M 1 1 C 1 1 1 1 1 1 z", 4, {SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1}, resultVerbs15, std::size(resultVerbs15) },
        { "M 1 1 C 1 1 1 1 1 1 z M 2 1 C 2 1 2 1 2 1 z", 8, {SK_Scalar1, SK_Scalar1, 2*SK_Scalar1, SK_Scalar1}, resultVerbs16,
            std::size(resultVerbs16)
        }
    };

    for (size_t i = 0; i < std::size(gZeroLengthTests); ++i) {
        auto p = SkParsePath::FromSVGString(gZeroLengthTests[i].testPath);
        SkSpan<const SkPathVerb> verbs = p->verbs();
        REPORTER_ASSERT(reporter, p.has_value());
        REPORTER_ASSERT(reporter, !p->isEmpty());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].numResultPts == (size_t)p->countPoints());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].resultBound == p->getBounds());
        REPORTER_ASSERT(reporter, gZeroLengthTests[i].numResultVerbs == verbs.size());
        for (size_t j = 0; j < gZeroLengthTests[i].numResultVerbs; ++j) {
            REPORTER_ASSERT(reporter, gZeroLengthTests[i].resultVerbs[j] == (uint8_t)verbs[j]);
        }
    }
}

struct SegmentInfo {
    SkPath fPath;
    int    fPointCount;
};

static void test_segment_masks(skiatest::Reporter* reporter) {
    auto check_mask = [reporter](const char* desc,
                                 uint32_t expectedMask,
                                 const std::function<void(SkPathBuilder&)>& build) {
        skiatest::ReporterContext context(reporter, desc);
        SkPathBuilder builder;
        build(builder);
        SkPath path = builder.detach();
        REPORTER_ASSERT(reporter, path.getSegmentMasks() == expectedMask);
    };

    check_mask("empty", 0, [](SkPathBuilder& builder) {
        // empty
    });

    check_mask("move-only", 0, [](SkPathBuilder& builder) { builder.moveTo(0, 0); });

    check_mask("line", SkPath::kLine_SegmentMask, [](SkPathBuilder& builder) {
        builder.moveTo(0, 0);
        builder.lineTo(1, 1);
    });

    check_mask("quad", SkPath::kQuad_SegmentMask, [](SkPathBuilder& builder) {
        builder.moveTo(0, 0);
        builder.quadTo(1, 1, 2, 2);
    });

    check_mask("conic", SkPath::kConic_SegmentMask, [](SkPathBuilder& builder) {
        builder.moveTo(0, 0);
        builder.conicTo(1, 1, 2, 2, 0.5f);
    });

    check_mask("cubic", SkPath::kCubic_SegmentMask, [](SkPathBuilder& builder) {
        builder.moveTo(0, 0);
        builder.cubicTo(1, 1, 2, 2, 3, 3);
    });

    check_mask("quad-cubic",
               SkPath::kQuad_SegmentMask | SkPath::kCubic_SegmentMask,
               [](SkPathBuilder& builder) {
                   builder.moveTo(0, 0);
                   builder.quadTo(1, 1, 2, 2);
                   builder.cubicTo(3, 3, 4, 4, 5, 5);
               });

    check_mask("all",
               SkPath::kLine_SegmentMask | SkPath::kQuad_SegmentMask | SkPath::kConic_SegmentMask |
                       SkPath::kCubic_SegmentMask,
               [](SkPathBuilder& builder) {
                   builder.moveTo(0, 0);
                   builder.lineTo(1, 1);
                   builder.quadTo(2, 2, 3, 3);
                   builder.conicTo(4, 4, 5, 5, 0.5f);
                   builder.cubicTo(6, 6, 7, 7, 8, 8);
                   builder.close();
               });
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
        const size_t* numResultPtsPerVerb;
        const SkPoint* resultPts;
        const SkPath::Verb* resultVerbs;
        const size_t numResultVerbs;
    };

    static const SkPath::Verb resultVerbs1[] = { SkPath::kDone_Verb };
    static const SkPath::Verb resultVerbs2[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb, SkPath::kClose_Verb, SkPath::kDone_Verb
    };
    static const SkPath::Verb resultVerbs3[] = {
        SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb, SkPath::kClose_Verb, SkPath::kDone_Verb
    };
    static const size_t resultPtsSizes1[] = { 0 };
    static const size_t resultPtsSizes2[] = { 1, 2, 1, 1, 0 };
    static const size_t resultPtsSizes3[] = { 1, 2, 1, 1, 1, 0 };
    static const SkPoint* resultPts1 = nullptr;
    static const SkPoint resultPts2[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { 0, 0 }, { 0, 0 }
    };
    static const SkPoint resultPts3[] = {
        { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { SK_Scalar1, 0 }, { 0, 0 }, { 0, 0 }
    };
    static const struct iterTestData gIterTests[] = {
        { "M 1 0", false, resultPtsSizes1, resultPts1, resultVerbs1, std::size(resultVerbs1) },
        { "z", false, resultPtsSizes1, resultPts1, resultVerbs1, std::size(resultVerbs1) },
        { "z", true, resultPtsSizes1, resultPts1, resultVerbs1, std::size(resultVerbs1) },
        { "M 1 0 L 1 0 M 0 0 z", false, resultPtsSizes2, resultPts2, resultVerbs2, std::size(resultVerbs2) },
        { "M 1 0 L 1 0 M 0 0 z", true, resultPtsSizes3, resultPts3, resultVerbs3, std::size(resultVerbs3) }
    };

    for (size_t i = 0; i < std::size(gIterTests); ++i) {
        auto path = SkParsePath::FromSVGString(gIterTests[i].testPath);
        REPORTER_ASSERT(reporter, path.has_value());
        iter.setPath(*path, gIterTests[i].forceClose);
        int j = 0, l = 0;
        do {
            REPORTER_ASSERT(reporter, iter.next(pts) == gIterTests[i].resultVerbs[j]);
            for (int k = 0; k < (int)gIterTests[i].numResultPtsPerVerb[j]; ++k) {
                REPORTER_ASSERT(reporter, pts[k] == gIterTests[i].resultPts[l++]);
            }
        } while (gIterTests[i].resultVerbs[j++] != SkPath::kDone_Verb);
        REPORTER_ASSERT(reporter, j == (int)gIterTests[i].numResultVerbs);
    }

    p.reset();
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, !iter.isClosedContour());
    p = SkPathBuilder().lineTo(1, 1).close().detach();
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, iter.isClosedContour());
    p.reset();
    iter.setPath(p, true);
    REPORTER_ASSERT(reporter, !iter.isClosedContour());
    p = SkPathBuilder().lineTo(1, 1).detach();
    iter.setPath(p, true);
    REPORTER_ASSERT(reporter, iter.isClosedContour());
    p = SkPathBuilder().lineTo(1, 1).moveTo(0, 0).lineTo(2, 2).detach();
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, !iter.isClosedContour());

    p = SkPathBuilder().quadTo(0, 0, 0, 0).detach();
    iter.setPath(p, false);
    iter.next(pts);
    REPORTER_ASSERT(reporter, SkPath::kQuad_Verb == iter.next(pts));

    p = SkPathBuilder().conicTo(0, 0, 0, 0, 0.5f).detach();
    iter.setPath(p, false);
    iter.next(pts);
    REPORTER_ASSERT(reporter, SkPath::kConic_Verb == iter.next(pts));

    SkPathBuilder builder;
    builder.cubicTo(0, 0, 0, 0, 0, 0);
    p = builder.snapshot();
    iter.setPath(p, false);
    iter.next(pts);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb == iter.next(pts));

    builder.moveTo(1, 1);  // add a trailing moveto
    p = builder.detach();
    iter.setPath(p, false);
    iter.next(pts);
    REPORTER_ASSERT(reporter, SkPath::kCubic_Verb == iter.next(pts));

    // The GM degeneratesegments.cpp test is more extensive

    // Test out mixed degenerate and non-degenerate geometry with Conics
    const SkVector radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 100, 100 } };
    SkRect r = SkRect::MakeWH(100, 100);
    SkRRect rr;
    rr.setRectRadii(r, radii);
    p = SkPath::RRect(rr);
    iter.setPath(p, false);
    REPORTER_ASSERT(reporter, SkPathVerb::kMove == iter.next()->fVerb);
    REPORTER_ASSERT(reporter, SkPathVerb::kLine == iter.next()->fVerb);
}

static void test_range_iter(skiatest::Reporter* reporter) {
    SkPath path;

    // Test an iterator with an initial empty path
    SkPathPriv::Iterate iterate(path);
    REPORTER_ASSERT(reporter, iterate.begin() == iterate.end());

    // Test that a move-only path returns the move.
    SkPathBuilder builder;
    builder.moveTo(1, 0);
    path = builder.snapshot();
    iterate = SkPathPriv::Iterate(path);
    SkPathPriv::RangeIter iter = iterate.begin();
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == 1);
        REPORTER_ASSERT(reporter, pts[0].fY == 0);
    }
    REPORTER_ASSERT(reporter, iter == iterate.end());

    // another moveTo just replace the previous position
    builder.moveTo(2, 3);
    path = builder.snapshot();
    iterate = SkPathPriv::Iterate(path);
    iter = iterate.begin();
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == 2);
        REPORTER_ASSERT(reporter, pts[0].fY == 3);
    }
    REPORTER_ASSERT(reporter, iter == iterate.end());

    // Initial close is never ever stored
    path = SkPathBuilder().close().detach();
    iterate = SkPathPriv::Iterate(path);
    REPORTER_ASSERT(reporter, iterate.begin() == iterate.end());

    // Move/close sequences
    path = SkPathBuilder()
           .close() // Not stored, no purpose
           .moveTo(1, 0)
           .close()
           .close() // Not stored, no purpose
           .moveTo(2, 1)
           .close()
           .moveTo(3, 2)
           .moveTo(4, 3) // replaces previous moveTo
           .close()
           .detach();
    iterate = SkPathPriv::Iterate(path);
    iter = iterate.begin();
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1);
        REPORTER_ASSERT(reporter, pts[0].fY == 0);
    }
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kClose);
    }
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*2);
        REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1);
    }
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kClose);
    }
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == SK_Scalar1*4);
        REPORTER_ASSERT(reporter, pts[0].fY == SK_Scalar1*3);
    }
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kClose);
    }
    REPORTER_ASSERT(reporter, iter == iterate.end());

    // Generate random paths and verify
    SkPoint randomPts[25];
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            randomPts[i*5+j].set(SK_Scalar1*i, SK_Scalar1*j);
        }
    }

    // Max of 10 segments, max 3 points per segment
    SkRandom rand(9876543);
    SkPoint expectedPts[31]; // May have leading moveTo
    SkPathVerb expectedVerbs[22]; // May have leading moveTo
    SkPathVerb nextVerb;

    SkPathVerb prevVerb = static_cast<SkPathVerb>(0xFF); // need something illegal to start twith
    for (int i = 0; i < 500; ++i) {
        builder.reset();
        bool lastWasClose = true;
        bool haveMoveTo = false;
        SkPoint lastMoveToPt = { 0, 0 };
        int numPoints = 0;
        int numVerbs = (rand.nextU() >> 16) % 10;
        int numIterVerbs = 0;
        for (int j = 0; j < numVerbs; ++j) {
            do {
                nextVerb = static_cast<SkPathVerb>((rand.nextU() >> 16) % SkPath::kDone_Verb);
            } while ((lastWasClose && nextVerb == SkPathVerb::kClose) ||
                     (prevVerb == SkPathVerb::kMove && nextVerb == SkPathVerb::kMove));
            switch (nextVerb) {
                case SkPathVerb::kMove:
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    builder.moveTo(expectedPts[numPoints]);
                    lastMoveToPt = expectedPts[numPoints];
                    numPoints += 1;
                    lastWasClose = false;
                    haveMoveTo = true;
                    break;
                case SkPathVerb::kLine:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPathVerb::kMove;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    builder.lineTo(expectedPts[numPoints]);
                    numPoints += 1;
                    lastWasClose = false;
                    break;
                case SkPathVerb::kQuad:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPathVerb::kMove;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    builder.quadTo(expectedPts[numPoints], expectedPts[numPoints + 1]);
                    numPoints += 2;
                    lastWasClose = false;
                    break;
                case SkPathVerb::kConic:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPathVerb::kMove;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    builder.conicTo(expectedPts[numPoints], expectedPts[numPoints + 1],
                                    rand.nextUScalar1() * 4);
                    numPoints += 2;
                    lastWasClose = false;
                    break;
                case SkPathVerb::kCubic:
                    if (!haveMoveTo) {
                        expectedPts[numPoints++] = lastMoveToPt;
                        expectedVerbs[numIterVerbs++] = SkPathVerb::kMove;
                        haveMoveTo = true;
                    }
                    expectedPts[numPoints] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 1] = randomPts[(rand.nextU() >> 16) % 25];
                    expectedPts[numPoints + 2] = randomPts[(rand.nextU() >> 16) % 25];
                    builder.cubicTo(expectedPts[numPoints], expectedPts[numPoints + 1],
                                    expectedPts[numPoints + 2]);
                    numPoints += 3;
                    lastWasClose = false;
                    break;
                case SkPathVerb::kClose:
                    builder.close();
                    haveMoveTo = false;
                    lastWasClose = true;
                    break;
                default:
                    SkDEBUGFAIL("unexpected verb");
            }
            expectedVerbs[numIterVerbs++] = nextVerb;
            prevVerb = nextVerb;
        }

        numVerbs = numIterVerbs;
        numIterVerbs = 0;
        int numIterPts = 0;
        SkPoint lastMoveTo;
        SkPoint lastPt;
        lastMoveTo.set(0, 0);
        lastPt.set(0, 0);
        path = builder.detach();
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            REPORTER_ASSERT(reporter, verb == expectedVerbs[numIterVerbs]);
            numIterVerbs++;
            switch (verb) {
                case SkPathVerb::kMove:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints);
                    REPORTER_ASSERT(reporter, pts[0] == expectedPts[numIterPts]);
                    lastPt = lastMoveTo = pts[0];
                    numIterPts += 1;
                    break;
                case SkPathVerb::kLine:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 1);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    lastPt = pts[1];
                    numIterPts += 1;
                    break;
                case SkPathVerb::kQuad:
                case SkPathVerb::kConic:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 2);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    REPORTER_ASSERT(reporter, pts[2] == expectedPts[numIterPts + 1]);
                    lastPt = pts[2];
                    numIterPts += 2;
                    break;
                case SkPathVerb::kCubic:
                    REPORTER_ASSERT(reporter, numIterPts < numPoints + 3);
                    REPORTER_ASSERT(reporter, pts[0] == lastPt);
                    REPORTER_ASSERT(reporter, pts[1] == expectedPts[numIterPts]);
                    REPORTER_ASSERT(reporter, pts[2] == expectedPts[numIterPts + 1]);
                    REPORTER_ASSERT(reporter, pts[3] == expectedPts[numIterPts + 2]);
                    lastPt = pts[3];
                    numIterPts += 3;
                    break;
                case SkPathVerb::kClose:
                    lastPt = lastMoveTo;
                    break;
                default:
                    SkDEBUGFAIL("unexpected verb");
            }
        }
        REPORTER_ASSERT(reporter, numIterPts == numPoints);
        REPORTER_ASSERT(reporter, numIterVerbs == numVerbs);
    }
}

static void check_for_circle(skiatest::Reporter* reporter,
                             const SkPath& path,
                             bool expectedCircle,
                             SkPathFirstDirection expectedDir) {
    SkRect rect = SkRect::MakeEmpty();
    REPORTER_ASSERT(reporter, path.isOval(&rect) == expectedCircle);
    if (auto info = SkPathPriv::IsOval(path)) {
        REPORTER_ASSERT(reporter, info->fBounds.height() == info->fBounds.width());
        REPORTER_ASSERT(reporter, SkPathPriv::AsFirstDirection(info->fDirection) == expectedDir);
        SkPath tmpPath = SkPath::Oval(rect, info->fDirection, info->fStartIndex);
        REPORTER_ASSERT(reporter, path == tmpPath);
    }
    REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(path) == expectedDir);
}

static void test_circle_skew(skiatest::Reporter* reporter,
                             const SkPath& path,
                             SkPathFirstDirection dir) {
    SkMatrix m;
    m.setSkew(SkIntToScalar(3), SkIntToScalar(5));
    SkPath tmp = path.makeTransform(m);
    // this matrix reverses the direction.
    if (SkPathFirstDirection::kCCW == dir) {
        dir = SkPathFirstDirection::kCW;
    } else {
        REPORTER_ASSERT(reporter, SkPathFirstDirection::kCW == dir);
        dir = SkPathFirstDirection::kCCW;
    }
    check_for_circle(reporter, tmp, false, dir);
}

static void test_circle_translate(skiatest::Reporter* reporter,
                                  const SkPath& path,
                                  SkPathFirstDirection dir) {
    // translate at small offset
    SkMatrix m;
    m.setTranslate(SkIntToScalar(15), SkIntToScalar(15));
    SkPath tmp = path.makeTransform(m);
    check_for_circle(reporter, tmp, true, dir);

    // translate at a relatively big offset
    m.setTranslate(SkIntToScalar(1000), SkIntToScalar(1000));
    tmp = path.makeTransform(m);
    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_rotate(skiatest::Reporter* reporter,
                               const SkPath& path,
                               SkPathFirstDirection dir) {
    for (int angle = 0; angle < 360; ++angle) {
        SkMatrix m;
        m.setRotate(SkIntToScalar(angle));
        SkPath tmp = path.makeTransform(m);

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
                                 SkPathFirstDirection dir) {
    SkMatrix m;
    m.reset();
    m.setScaleX(-1);
    SkPath tmp = path.makeTransform(m);
    if (SkPathFirstDirection::kCW == dir) {
        dir = SkPathFirstDirection::kCCW;
    } else {
        REPORTER_ASSERT(reporter, SkPathFirstDirection::kCCW == dir);
        dir = SkPathFirstDirection::kCW;
    }
    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_mirror_y(skiatest::Reporter* reporter,
                                 const SkPath& path,
                                 SkPathFirstDirection dir) {
    SkMatrix m;
    m.reset();
    m.setScaleY(-1);
    SkPath tmp = path.makeTransform(m);

    if (SkPathFirstDirection::kCW == dir) {
        dir = SkPathFirstDirection::kCCW;
    } else {
        REPORTER_ASSERT(reporter, SkPathFirstDirection::kCCW == dir);
        dir = SkPathFirstDirection::kCW;
    }

    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_mirror_xy(skiatest::Reporter* reporter,
                                 const SkPath& path,
                                 SkPathFirstDirection dir) {
    SkMatrix m;
    m.reset();
    m.setScaleX(-1);
    m.setScaleY(-1);
    SkPath tmp = path.makeTransform(m);

    check_for_circle(reporter, tmp, true, dir);
}

static void test_circle_with_direction(skiatest::Reporter* reporter,
                                       SkPathDirection inDir) {
    const SkPathFirstDirection dir = SkPathPriv::AsFirstDirection(inDir);
    // circle at origin
    SkPath path = SkPath::Circle(0, 0, SkIntToScalar(20), inDir);

    check_for_circle(reporter, path, true, dir);
    test_circle_rotate(reporter, path, dir);
    test_circle_translate(reporter, path, dir);
    test_circle_skew(reporter, path, dir);
    test_circle_mirror_x(reporter, path, dir);
    test_circle_mirror_y(reporter, path, dir);
    test_circle_mirror_xy(reporter, path, dir);

    // circle at an offset at (10, 10)
    path = SkPath::Circle(SkIntToScalar(10), SkIntToScalar(10),
                          SkIntToScalar(20), inDir);

    check_for_circle(reporter, path, true, dir);
    test_circle_rotate(reporter, path, dir);
    test_circle_translate(reporter, path, dir);
    test_circle_skew(reporter, path, dir);
    test_circle_mirror_x(reporter, path, dir);
    test_circle_mirror_y(reporter, path, dir);
    test_circle_mirror_xy(reporter, path, dir);

    // Try different starting points for the contour.
    for (unsigned start = 0; start < 4; ++start) {
        path = SkPath::Oval(SkRect::MakeXYWH(20, 10, 5, 5), inDir, start);
        test_circle_rotate(reporter, path, dir);
        test_circle_translate(reporter, path, dir);
        test_circle_skew(reporter, path, dir);
        test_circle_mirror_x(reporter, path, dir);
        test_circle_mirror_y(reporter, path, dir);
        test_circle_mirror_xy(reporter, path, dir);
    }
}

static void test_circle_with_add_paths(skiatest::Reporter* reporter) {
    SkPath path;
    SkPath empty;

    const SkPathDirection kCircleDir = SkPathDirection::kCW;
    const SkPathDirection kCircleDirOpposite = SkPathDirection::kCCW;

    SkPath circle = SkPath::Circle(0, 0, 10, kCircleDir);
    SkPath rect = SkPath::Rect({5, 5, 20, 20}, SkPathDirection::kCW);

    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(12), SkIntToScalar(12));

    // Although all the path concatenation related operations leave
    // the path a circle, most mark it as a non-circle for simplicity

    // empty + circle (translate)
    SkPathBuilder builder(empty);
    builder.addPath(circle, translate);
    check_for_circle(reporter, builder.detach(), false, SkPathPriv::AsFirstDirection(kCircleDir));

    // circle + empty (translate)
    builder = circle;
    builder.addPath(empty, translate);
    check_for_circle(reporter, builder.detach(), true, SkPathPriv::AsFirstDirection(kCircleDir));

    // test reverseAddPath
    builder = circle;
    SkPathPriv::ReverseAddPath(&builder, rect);
    check_for_circle(reporter, builder.detach(), false,
                     SkPathPriv::AsFirstDirection(kCircleDirOpposite));
}

static void test_circle(skiatest::Reporter* reporter) {
    test_circle_with_direction(reporter, SkPathDirection::kCW);
    test_circle_with_direction(reporter, SkPathDirection::kCCW);

    // multiple addCircle()
    SkPath path = SkPathBuilder()
                  .addCircle(0, 0, SkIntToScalar(10), SkPathDirection::kCW)
                  .addCircle(0, 0, SkIntToScalar(20), SkPathDirection::kCW)
                  .detach();
    check_for_circle(reporter, path, false, SkPathFirstDirection::kCW);

    // some extra lineTo() would make isOval() fail
    path = SkPathBuilder()
           .addCircle(0, 0, SkIntToScalar(10), SkPathDirection::kCW)
           .lineTo(0, 0)
           .detach();
    check_for_circle(reporter, path, false, SkPathFirstDirection::kCW);

    // not back to the original point
    SkPathBuilder builder;
    builder.addCircle(0, 0, SkIntToScalar(10), SkPathDirection::kCW);
    builder.setLastPoint({5, 5});
    check_for_circle(reporter, builder.detach(), false, SkPathFirstDirection::kCW);

    test_circle_with_add_paths(reporter);

    // test negative radius
    path = SkPath::Circle(0, 0, -1, SkPathDirection::kCW);
    REPORTER_ASSERT(reporter, path.isEmpty());
}

static void test_oval(skiatest::Reporter* reporter) {
    SkRect rect;
    SkMatrix m;

    rect = SkRect::MakeWH(SkIntToScalar(30), SkIntToScalar(50));
    SkPath path = SkPath::Oval(rect);

    // Defaults to dir = CW and start = 1
    REPORTER_ASSERT(reporter, path.isOval(nullptr));

    m.setRotate(SkIntToScalar(90));
    SkPath tmp = path.makeTransform(m);
    // an oval rotated 90 degrees is still an oval. The start index changes from 1 to 2. Direction
    // is unchanged.
    std::optional<SkPathOvalInfo> info = SkPathPriv::IsOval(tmp);
    REPORTER_ASSERT(reporter, info.has_value());
    REPORTER_ASSERT(reporter, 2 == info->fStartIndex);
    REPORTER_ASSERT(reporter, SkPathDirection::kCW == info->fDirection);

    m.setRotate(30);
    tmp = path.makeTransform(m);
    // an oval rotated 30 degrees is not an oval anymore.
    REPORTER_ASSERT(reporter, !tmp.isOval(nullptr));

    // since empty path being transformed.
    path = SkPath();
    m.reset();
    tmp = path.makeTransform(m);
    REPORTER_ASSERT(reporter, !tmp.isOval(nullptr));

    // empty path is not an oval
    tmp = SkPath();
    REPORTER_ASSERT(reporter, !tmp.isOval(nullptr));

    // only has moveTo()s
    tmp = SkPathBuilder()
          .moveTo(0, 0)
          .moveTo(SkIntToScalar(10), SkIntToScalar(10))
          .detach();
    REPORTER_ASSERT(reporter, !tmp.isOval(nullptr));

    // mimic WebKit's calling convention,
    // call moveTo() first and then call addOval()
    path = SkPathBuilder()
           .moveTo(0, 0)
           .addOval(rect)
           .detach();
    REPORTER_ASSERT(reporter, path.isOval(nullptr));

    // copy path
    tmp = SkPath::Oval(rect);
    path = tmp;
    info = SkPathPriv::IsOval(path);
    REPORTER_ASSERT(reporter, info.has_value());
    REPORTER_ASSERT(reporter, SkPathDirection::kCW == info->fDirection);
    REPORTER_ASSERT(reporter, 1 == info->fStartIndex);
}

static void test_empty(skiatest::Reporter* reporter, const SkPath& p) {
    SkPath  empty;

    REPORTER_ASSERT(reporter, p.isEmpty());
    REPORTER_ASSERT(reporter, 0 == p.countPoints());
    REPORTER_ASSERT(reporter, 0 == p.countVerbs());
    REPORTER_ASSERT(reporter, 0 == p.getSegmentMasks());
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, p.getFillType() == SkPathFillType::kWinding);
    REPORTER_ASSERT(reporter, !p.isInverseFillType());
    REPORTER_ASSERT(reporter, p == empty);
    REPORTER_ASSERT(reporter, !(p != empty));
}

static void test_rrect_is_convex(skiatest::Reporter* reporter, const SkPath& path,
                                 SkPathDirection dir) {
    REPORTER_ASSERT(reporter, path.isConvex());
    REPORTER_ASSERT(reporter,
                    SkPathPriv::ComputeFirstDirection(path) == SkPathPriv::AsFirstDirection(dir));
    SkPathPriv::ForceComputeConvexity(path);
    REPORTER_ASSERT(reporter, path.isConvex());
}

static void test_rrect_convexity_is_unknown(skiatest::Reporter* reporter, const SkPath& path,
                                 SkPathDirection dir) {
    REPORTER_ASSERT(reporter, path.isConvex());
    REPORTER_ASSERT(reporter,
                    SkPathPriv::ComputeFirstDirection(path) == SkPathPriv::AsFirstDirection(dir));
    SkPathPriv::ForceComputeConvexity(path);
    REPORTER_ASSERT(reporter, !path.isConvex());
}

static void test_rrect(skiatest::Reporter* reporter) {
    SkRRect rr;
    SkVector radii[] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}};
    SkRect r = {10, 20, 30, 40};
    rr.setRectRadii(r, radii);
    test_rrect_is_convex(reporter, SkPath::RRect(rr), SkPathDirection::kCW);
    test_rrect_is_convex(reporter, SkPath::RRect(rr, SkPathDirection::kCCW), SkPathDirection::kCCW);
    test_rrect_is_convex(reporter, SkPath::RRect(r, radii[1].fX, radii[1].fY),
                                                 SkPathDirection::kCW);
    test_rrect_is_convex(reporter, SkPath::RRect(r, radii[1].fX, radii[1].fY,
                                                 SkPathDirection::kCCW), SkPathDirection::kCCW);

    for (size_t i = 0; i < std::size(radii); ++i) {
        SkVector save = radii[i];
        radii[i].set(0, 0);
        rr.setRectRadii(r, radii);
        test_rrect_is_convex(reporter, SkPath::RRect(rr), SkPathDirection::kCW);
        radii[i] = save;
    }

    SkPath p = SkPath::RRect(r, 0, 0);
    SkRect returnedRect;
    REPORTER_ASSERT(reporter, p.isRect(&returnedRect));
    REPORTER_ASSERT(reporter, returnedRect == r);
    test_rrect_is_convex(reporter, p, SkPathDirection::kCW);

    SkVector zeroRadii[] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    rr.setRectRadii(r, zeroRadii);
    p = SkPath::RRect(rr);
    bool closed;
    SkPathDirection dir;
    REPORTER_ASSERT(reporter, p.isRect(nullptr, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, SkPathDirection::kCW == dir);
    test_rrect_is_convex(reporter, p, SkPathDirection::kCW);

    p = SkPathBuilder()
        .addRRect(rr, SkPathDirection::kCW)
        .addRRect(rr, SkPathDirection::kCW)
        .detach();
    REPORTER_ASSERT(reporter, !p.isConvex());
    p = SkPathBuilder()
        .addRRect(rr, SkPathDirection::kCCW)
        .addRRect(rr, SkPathDirection::kCCW)
        .detach();
    REPORTER_ASSERT(reporter, !p.isConvex());

    SkRect emptyR = {10, 20, 10, 30};
    rr.setRectRadii(emptyR, radii);
    p = SkPath::RRect(rr);
    // The round rect is "empty" in that it has no fill area. However,
    // the path isn't "empty" in that it should have verbs and points.
    REPORTER_ASSERT(reporter, !p.isEmpty());

    SkRect largeR = {0, 0, SK_ScalarMax, SK_ScalarMax};
    rr.setRectRadii(largeR, radii);
    test_rrect_convexity_is_unknown(reporter, SkPath::RRect(rr), SkPathDirection::kCW);

    // we check for non-finites
    SkRect infR = {0, 0, SK_ScalarMax, SK_ScalarInfinity};
    rr.setRectRadii(infR, radii);
    REPORTER_ASSERT(reporter, rr.isEmpty());
}

static void test_arc(skiatest::Reporter* reporter) {
    SkPath p;
    SkRect emptyOval = {10, 20, 30, 20};
    REPORTER_ASSERT(reporter, emptyOval.isEmpty());
    p = SkPathBuilder().addArc(emptyOval, 1, 2).detach();
    REPORTER_ASSERT(reporter, p.isEmpty());
    SkRect oval = {10, 20, 30, 40};
    p = SkPathBuilder().addArc(oval, 1, 0).detach();
    REPORTER_ASSERT(reporter, p.isEmpty());
    SkPath cwOval = SkPath::Oval(oval);
    p = SkPathBuilder().addArc(oval, 0, 360).detach();
    REPORTER_ASSERT(reporter, p == cwOval);
    SkPath ccwOval = SkPath::Oval(oval, SkPathDirection::kCCW);
    p = SkPathBuilder().addArc(oval, 0, -360).detach();
    REPORTER_ASSERT(reporter, p == ccwOval);
    p = SkPathBuilder().addArc(oval, 1, 180).detach();
    // diagonal colinear points make arc convex
    // TODO: one way to keep it concave would be to introduce interpolated on curve points
    // between control points and computing the on curve point at scan conversion time
    REPORTER_ASSERT(reporter, p.isConvex());
    REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(p) == SkPathFirstDirection::kCW);
    SkPathPriv::ForceComputeConvexity(p);
    REPORTER_ASSERT(reporter, p.isConvex());
}

static inline SkScalar oval_start_index_to_angle(unsigned start) {
    switch (start) {
        case 0:
            return 270.f;
        case 1:
            return 0.f;
        case 2:
            return 90.f;
        case 3:
            return 180.f;
        default:
            return -1.f;
    }
}

static inline SkScalar canonical_start_angle(float angle) {
    while (angle < 0.f) {
        angle += 360.f;
    }
    while (angle >= 360.f) {
        angle -= 360.f;
    }
    return angle;
}

static void check_oval_arc(skiatest::Reporter* reporter, SkScalar start, SkScalar sweep,
                           const SkPath& path) {
    std::optional<SkPathOvalInfo> info = SkPathPriv::IsOval(path);
    REPORTER_ASSERT(reporter, info.has_value());
    SkPath recreatedPath = SkPath::Oval(info->fBounds, info->fDirection, info->fStartIndex);
    REPORTER_ASSERT(reporter, path == recreatedPath);
    REPORTER_ASSERT(reporter,
                    oval_start_index_to_angle(info->fStartIndex) == canonical_start_angle(start));
    REPORTER_ASSERT(reporter, (SkPathDirection::kCW == info->fDirection) == (sweep > 0.f));
}

static void test_arc_ovals(skiatest::Reporter* reporter) {
    SkRect oval = SkRect::MakeWH(10, 20);
    for (SkScalar sweep : {-720.f, -540.f, -360.f, 360.f, 432.f, 720.f}) {
        for (SkScalar start = -360.f; start <= 360.f; start += 1.f) {
            SkPath path = SkPathBuilder().addArc(oval, start, sweep).detach();
            // SkPath's interfaces for inserting and extracting ovals only allow contours
            // to start at multiples of 90 degrees.
            if (std::fmod(start, 90.f) == 0) {
                check_oval_arc(reporter, start, sweep, path);
            } else {
                REPORTER_ASSERT(reporter, !path.isOval(nullptr));
            }
        }
        // Test start angles that are nearly at valid oval start angles.
        for (float start : {-180.f, -90.f, 90.f, 180.f}) {
            for (float delta : {-SK_ScalarNearlyZero, SK_ScalarNearlyZero}) {
                SkPath path = SkPathBuilder().addArc(oval, start + delta, sweep).detach();
                check_oval_arc(reporter, start, sweep, path);
            }
        }
    }
}

static void check_move(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter,
                       SkScalar x0, SkScalar y0) {
    auto [v, pts, w] = *(*iter)++;
    REPORTER_ASSERT(reporter, v == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, pts[0].fX == x0);
    REPORTER_ASSERT(reporter, pts[0].fY == y0);
}

static void check_line(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter,
                       SkScalar x1, SkScalar y1) {
    auto [v, pts, w] = *(*iter)++;
    REPORTER_ASSERT(reporter, v == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, pts[1].fX == x1);
    REPORTER_ASSERT(reporter, pts[1].fY == y1);
}

static void check_close(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter) {
    auto [v, pts, w] = *(*iter)++;
    REPORTER_ASSERT(reporter, v == SkPathVerb::kClose);
}

static void check_done(skiatest::Reporter* reporter, const SkPath& p, SkPathPriv::RangeIter* iter) {
    REPORTER_ASSERT(reporter, *iter == SkPathPriv::Iterate(p).end());
}

static void check_path_is_move(skiatest::Reporter* reporter, const SkPath& p,
                               SkScalar x0, SkScalar y0) {
    SkPathPriv::RangeIter iter = SkPathPriv::Iterate(p).begin();
    check_move(reporter, &iter, x0, y0);
    check_done(reporter, p, &iter);
}

static void check_path_is_line(skiatest::Reporter* reporter, const SkPath& p,
                               SkScalar x1, SkScalar y1) {
    SkPathPriv::RangeIter iter = SkPathPriv::Iterate(p).begin();
    check_move(reporter, &iter, 0, 0);
    check_line(reporter, &iter, x1, y1);
    check_done(reporter, p, &iter);
}

static bool nearly_equal(const SkRect& a, const SkRect& b) {
    return  SkScalarNearlyEqual(a.fLeft, b.fLeft) &&
            SkScalarNearlyEqual(a.fTop, b.fTop) &&
            SkScalarNearlyEqual(a.fRight, b.fRight) &&
            SkScalarNearlyEqual(a.fBottom, b.fBottom);
}

static void test_rMoveTo(skiatest::Reporter* reporter) {
    SkPath p = SkPathBuilder()
               .moveTo(10, 11)
               .lineTo(20, 21)
               .close()
               .rMoveTo({30, 31})
               .detach();
    SkPathPriv::RangeIter iter = SkPathPriv::Iterate(p).begin();
    check_move(reporter, &iter, 10, 11);
    check_line(reporter, &iter, 20, 21);
    check_close(reporter, &iter);
    check_move(reporter, &iter, 10 + 30, 11 + 31);
    check_done(reporter, p, &iter);

    p = SkPathBuilder()
        .moveTo(10, 11)
        .lineTo(20, 21)
        .rMoveTo({30, 31})
        .detach();
    iter = SkPathPriv::Iterate(p).begin();
    check_move(reporter, &iter, 10, 11);
    check_line(reporter, &iter, 20, 21);
    check_move(reporter, &iter, 20 + 30, 21 + 31);
    check_done(reporter, p, &iter);

    p = SkPathBuilder().rMoveTo({30, 31}).detach();
    iter = SkPathPriv::Iterate(p).begin();
    check_move(reporter, &iter, 30, 31);
    check_done(reporter, p, &iter);
}

static void test_arcTo(skiatest::Reporter* reporter) {
    SkPath p = SkPathBuilder().arcTo({0, 0}, {1, 2}, 1).detach();
    check_path_is_line(reporter, p, 0, 0);
    p = SkPathBuilder().arcTo({1, 2}, {1, 2}, 1).detach();
    check_path_is_line(reporter, p, 1, 2);
    p = SkPathBuilder().arcTo({1, 2}, {3, 4}, 0).detach();
    check_path_is_line(reporter, p, 1, 2);
    p = SkPathBuilder().arcTo({1, 2}, {0, 0}, 1).detach();
    check_path_is_line(reporter, p, 1, 2);
    p = SkPathBuilder().arcTo({1, 0}, {1, 1}, 1).detach();
    SkPoint pt = p.points().back();
    REPORTER_ASSERT(reporter, pt.fX == 1 && pt.fY == 1);
    p = SkPathBuilder().arcTo({1, 0}, {1, -1}, 1).detach();
    pt = p.points().back();
    REPORTER_ASSERT(reporter, pt.fX == 1 && pt.fY == -1);
    SkRect oval = {1, 2, 3, 4};
    p = SkPathBuilder().arcTo(oval, 0, 0, true).detach();
    check_path_is_move(reporter, p, oval.fRight, oval.centerY());
    p = SkPathBuilder().arcTo(oval, 0, 0, false).detach();
    check_path_is_move(reporter, p, oval.fRight, oval.centerY());
    p = SkPathBuilder().arcTo(oval, 360, 0, true).detach();
    check_path_is_move(reporter, p, oval.fRight, oval.centerY());
    p = SkPathBuilder().arcTo(oval, 360, 0, false).detach();
    check_path_is_move(reporter, p, oval.fRight, oval.centerY());

    SkPathBuilder builder;
    for (float sweep = 359, delta = 0.5f; sweep != (float) (sweep + delta); ) {
        builder.arcTo(oval, 0, sweep, false);
        REPORTER_ASSERT(reporter, nearly_equal(builder.computeBounds(), oval));
        sweep += delta;
        delta /= 2;
    }
    for (float sweep = 361, delta = 0.5f; sweep != (float) (sweep - delta);) {
        builder.arcTo(oval, 0, sweep, false);
        REPORTER_ASSERT(reporter, nearly_equal(builder.computeBounds(), oval));
        sweep -= delta;
        delta /= 2;
    }

    SkRect noOvalWidth = {1, 2, 0, 3};
    p = SkPathBuilder().arcTo(noOvalWidth, 0, 360, false).detach();
    REPORTER_ASSERT(reporter, p.isEmpty());

    SkRect noOvalHeight = {1, 2, 3, 1};
    p = SkPathBuilder().arcTo(noOvalHeight, 0, 360, false).detach();
    REPORTER_ASSERT(reporter, p.isEmpty());

    // Inspired by http://code.google.com/p/chromium/issues/detail?id=1001768
    {
      p = SkPathBuilder()
          .moveTo(216, 216)
          .arcTo({216, 108}, 0, SkPathBuilder::kLarge_ArcSize, SkPathDirection::kCW, {216, 0})
          .arcTo({270, 135}, 0, SkPathBuilder::kLarge_ArcSize, SkPathDirection::kCCW, {216, 216})
          .detach();

      // The 'arcTo' call should end up exactly at the starting location.
      REPORTER_ASSERT(reporter, p.points().front() == p.points().back());
    }

    // This test, if improperly handled, can create an infinite loop in angles_to_unit_vectors
    std::ignore = SkPathBuilder()
                  .arcTo(SkRect::MakeXYWH(0, 0, 10, 10), -2.61488527e+33f, 359.992157f, false)
                  .detach();
}

static void test_addPath(skiatest::Reporter* reporter) {
    SkPath q = SkPathBuilder()
               .moveTo(4, 4)
               .lineTo(7, 8)
               .conicTo(8, 7, 6, 5, 0.5f)
               .quadTo(6, 7, 8, 6)
               .cubicTo(5, 6, 7, 8, 7, 5)
               .close()
               .detach();
    SkPath p = SkPathBuilder()
               .lineTo(1, 2)
               .addPath(q, -4, -4)
               .detach();
    SkRect expected = {0, 0, 4, 4};
    REPORTER_ASSERT(reporter, p.getBounds() == expected);
    p = SkPathPriv::ReversePath(q);
    SkRect reverseExpected = {4, 4, 8, 8};
    REPORTER_ASSERT(reporter, p.getBounds() == reverseExpected);
}

static void test_addPathMode(skiatest::Reporter* reporter, bool explicitMoveTo, bool extend) {
    SkPathBuilder p, q;
    if (explicitMoveTo) {
        p.moveTo(1, 1);
    }
    p.lineTo(1, 2);
    if (explicitMoveTo) {
        q.moveTo(2, 1);
    }
    q.lineTo(2, 2);
    p.addPath(q.detach(), extend ? SkPath::kExtend_AddPathMode : SkPath::kAppend_AddPathMode);

    SkSpan<const SkPathVerb> verbs = p.verbs();
    REPORTER_ASSERT(reporter, verbs.size() == 4);
    REPORTER_ASSERT(reporter, verbs[0] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[1] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[2] == (extend ? SkPathVerb::kLine : SkPathVerb::kMove));
    REPORTER_ASSERT(reporter, verbs[3] == SkPathVerb::kLine);
}

static void test_extendClosedPath(skiatest::Reporter* reporter) {
    SkPath q = SkPathBuilder().moveTo(2, 1).lineTo(2, 3).detach();

    SkPathBuilder p;
    p.moveTo(1, 1)
     .lineTo(1, 2)
     .lineTo(2, 2)
     .close()
     .addPath(q, SkPath::kExtend_AddPathMode);

    SkSpan<const SkPathVerb> verbs = p.verbs();
    REPORTER_ASSERT(reporter, verbs.size() == 7);
    REPORTER_ASSERT(reporter, verbs[0] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[1] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[2] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[3] == SkPathVerb::kClose);
    REPORTER_ASSERT(reporter, verbs[4] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[5] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[6] == SkPathVerb::kLine);

    auto pt = p.getLastPt();
    REPORTER_ASSERT(reporter, pt.has_value());
    REPORTER_ASSERT(reporter, pt.value() == SkPoint::Make(2, 3));
    REPORTER_ASSERT(reporter, p.points()[3] == SkPoint::Make(1, 1));
}

static void test_addEmptyPath(skiatest::Reporter* reporter, SkPath::AddPathMode mode) {
    // case 1: dst is empty
    SkPathBuilder p;
    p.moveTo(2, 1);
    p.lineTo(2, 3);
    SkPathBuilder q;
    q.addPath(p.snapshot(), mode);
    REPORTER_ASSERT(reporter, q == p);

    // case 2: src is empty
    SkPath r;   // empty
    p.addPath(r, mode);
    REPORTER_ASSERT(reporter, q == p);

    // case 3: src and dst are empty
    q.reset();
    q.addPath(r, mode);
    REPORTER_ASSERT(reporter, q.isEmpty());
}

static void test_contains(skiatest::Reporter* reporter,
                          const SkPathBuilder& bu, SkPoint pt, bool expectedContains) {
    SkPath path = bu.snapshot();

    auto raw = SkPathPriv::Raw(bu, SkResolveConvexity::kNo);
    REPORTER_ASSERT(reporter, raw.has_value());
    auto pdata = SkPathData::Make(raw->points(), raw->verbs(), raw->conics());

    REPORTER_ASSERT(reporter, bu.contains(pt) == expectedContains);
    REPORTER_ASSERT(reporter, path.contains(pt) == expectedContains);
    REPORTER_ASSERT(reporter, pdata->contains(pt, raw->fillType()) == expectedContains);
}

static void test_contains(skiatest::Reporter* reporter) {
    SkPathBuilder bu = SkPathBuilder()
               .moveTo(SkBits2Float(0xe085e7b1), SkBits2Float(0x5f512c00))  // -7.7191e+19f, 1.50724e+19f
               .conicTo(SkBits2Float(0xdfdaa221), SkBits2Float(0x5eaac338), SkBits2Float(0x60342f13), SkBits2Float(0xdf0cbb58), SkBits2Float(0x3f3504f3))  // -3.15084e+19f, 6.15237e+18f, 5.19345e+19f, -1.01408e+19f, 0.707107f
               .conicTo(SkBits2Float(0x60ead799), SkBits2Float(0xdfb76c24), SkBits2Float(0x609b9872), SkBits2Float(0xdf730de8), SkBits2Float(0x3f3504f4))  // 1.35377e+20f, -2.6434e+19f, 8.96947e+19f, -1.75139e+19f, 0.707107f
               .lineTo(SkBits2Float(0x609b9872), SkBits2Float(0xdf730de8))  // 8.96947e+19f, -1.75139e+19f
               .conicTo(SkBits2Float(0x6018b296), SkBits2Float(0xdeee870d), SkBits2Float(0xe008cd8e), SkBits2Float(0x5ed5b2db), SkBits2Float(0x3f3504f3))  // 4.40121e+19f, -8.59386e+18f, -3.94308e+19f, 7.69931e+18f, 0.707107f
               .conicTo(SkBits2Float(0xe0d526d9), SkBits2Float(0x5fa67b31), SkBits2Float(0xe085e7b2), SkBits2Float(0x5f512c01), SkBits2Float(0x3f3504f3)); // -1.22874e+20f, 2.39925e+19f, -7.7191e+19f, 1.50724e+19f, 0.707107
    // this may return true or false, depending on the platform's numerics, but it should not crash
    (void) bu.contains({-77.2027664f, 15.3066053f});

    auto check = [&](SkPoint p, bool expected) {
        test_contains(reporter, bu, p, expected);
    };

    bu.reset();
    bu.setFillType(SkPathFillType::kInverseWinding);
    check({0, 0}, true);
    bu.setFillType(SkPathFillType::kWinding);
    check({0, 0}, false);
    bu.setFillType(SkPathFillType::kWinding)
        .moveTo(4, 4)
        .lineTo(6, 8)
        .lineTo(8, 4);
    // test on edge
    check({6, 4}, true);
    check({5, 6}, true);
    check({7, 6}, true);
    // test quick reject
    check({4, 0}, false);
    check({0, 4}, false);
    check({4, 10}, false);
    check({10, 4}, false);
    // test various crossings in x
    check({5, 7}, false);
    check({6, 7}, true);
    check({7, 7}, false);
    bu = SkPathBuilder()
        .moveTo(4, 4)
        .lineTo(8, 6)
        .lineTo(4, 8);
    // test on edge
    check({4, 6}, true);
    check({6, 5}, true);
    check({6, 7}, true);
    // test various crossings in y
    check({7, 5}, false);
    check({7, 6}, true);
    check({7, 7}, false);
    bu = SkPathBuilder()
        .moveTo(4, 4)
        .lineTo(8, 4)
        .lineTo(8, 8)
        .lineTo(4, 8);
    // test on vertices
    check({4, 4}, true);
    check({8, 4}, true);
    check({8, 8}, true);
    check({4, 8}, true);
    bu = SkPathBuilder()
        .moveTo(4, 4)
        .lineTo(6, 8)
        .lineTo(2, 8);
    // test on edge
    check({5, 6}, true);
    check({4, 8}, true);
    check({3, 6}, true);
    bu = SkPathBuilder()
        .moveTo(4, 4)
        .lineTo(0, 6)
        .lineTo(4, 8);
    // test on edge
    check({2, 5}, true);
    check({2, 7}, true);
    check({4, 6}, true);
    // test canceling coincident edge (a smaller triangle is coincident with a larger one)
    bu = SkPathBuilder()
        .moveTo(4, 0)
        .lineTo(6, 4)
        .lineTo(2, 4)
        .moveTo(4, 0)
        .lineTo(0, 8)
        .lineTo(8, 8);
    check({1, 2}, false);
    check({3, 2}, false);
    check({4, 0}, false);
    check({4, 4}, true);

    // test quads
    bu = SkPathBuilder()
        .moveTo(4, 4)
        .quadTo(6, 6, 8, 8)
        .quadTo(6, 8, 4, 8)
        .quadTo(4, 6, 4, 4);
    check({5, 6}, true);
    check({6, 5}, false);
    // test quad edge
    check({5, 5}, true);
    check({5, 8}, true);
    check({4, 5}, true);
    // test quad endpoints
    check({4, 4}, true);
    check({8, 8}, true);
    check({4, 8}, true);

    bu.reset();
    const SkPoint qPts[] = {{6, 6}, {8, 8}, {6, 8}, {4, 8}, {4, 6}, {4, 4}, {6, 6}};
    bu.moveTo(qPts[0]);
    for (int index = 1; index < (int) std::size(qPts); index += 2) {
        bu.quadTo(qPts[index], qPts[index + 1]);
    }
    check({5, 6}, true);
    check({6, 5}, false);
    // test quad edge
    SkPoint halfway;
    for (int index = 0; index < (int) std::size(qPts) - 2; index += 2) {
        SkEvalQuadAt(&qPts[index], 0.5f, &halfway, nullptr);
        check(halfway, true);
    }

    // test conics
    bu.reset();
    const SkPoint kPts[] = {{4, 4}, {6, 6}, {8, 8}, {6, 8}, {4, 8}, {4, 6}, {4, 4}};
    bu.moveTo(kPts[0]);
    for (int index = 1; index < (int) std::size(kPts); index += 2) {
        bu.conicTo(kPts[index], kPts[index + 1], 0.5f);
    }
    check({5, 6}, true);
    check({6, 5}, false);
    // test conic edge
    for (int index = 0; index < (int) std::size(kPts) - 2; index += 2) {
        SkConic conic(&kPts[index], 0.5f);
        halfway = conic.evalAt(0.5f);
        check(halfway, true);
    }
    // test conic end points
    check({4, 4}, true);
    check({8, 8}, true);
    check({4, 8}, true);

    // test cubics
    SkPoint pts[] = {{5, 4}, {6, 5}, {7, 6}, {6, 6}, {4, 6}, {5, 7}, {5, 5}, {5, 4}, {6, 5}, {7, 6}};
    for (int i = 0; i < 3; ++i) {
        bu = SkPathBuilder(SkPathFillType::kEvenOdd)
            .moveTo(pts[i].fX, pts[i].fY)
            .cubicTo(pts[i + 1], pts[i + 2], pts[i + 3])
            .cubicTo(pts[i + 4], pts[i + 5], pts[i + 6])
            .close();
        check({5.5f, 5.5f}, true);
        check({4.5f, 5.5f}, false);
        // test cubic edge
        SkEvalCubicAt(&pts[i], 0.5f, &halfway, nullptr, nullptr);
        check(halfway, true);
        SkEvalCubicAt(&pts[i + 3], 0.5f, &halfway, nullptr, nullptr);
        check(halfway, true);
        // test cubic end points
        check(pts[i], true);
        check(pts[i + 3], true);
        check(pts[i + 6], true);
    }
}

static void test_operatorEqual(skiatest::Reporter* reporter) {
    SkPath a;
    SkPath b;
    REPORTER_ASSERT(reporter, a == a);
    REPORTER_ASSERT(reporter, a == b);
    a.setFillType(SkPathFillType::kInverseWinding);
    REPORTER_ASSERT(reporter, a != b);
    a.reset();
    REPORTER_ASSERT(reporter, a == b);
    a = SkPathBuilder().lineTo(1, 1).detach();
    REPORTER_ASSERT(reporter, a != b);
    a.reset();
    REPORTER_ASSERT(reporter, a == b);
    a = SkPathBuilder().lineTo(1, 1).detach();
    b = SkPathBuilder().lineTo(1, 2).detach();
    REPORTER_ASSERT(reporter, a != b);
    a = SkPathBuilder().lineTo(1, 2).detach();
    REPORTER_ASSERT(reporter, a == b);
}

static void compare_dump(skiatest::Reporter* reporter, const SkPath& path, bool dumpAsHex,
                         const char* str) {
    SkDynamicMemoryWStream wStream;
    path.dump(&wStream, dumpAsHex);
    sk_sp<SkData> data = wStream.detachAsData();
    REPORTER_ASSERT(reporter, data->size() == strlen(str));
    if (strlen(str) > 0) {
        REPORTER_ASSERT(reporter, !memcmp(data->data(), str, strlen(str)));
    } else {
        REPORTER_ASSERT(reporter, data->data() == nullptr || !memcmp(data->data(), str, strlen(str)));
    }
}

static void test_dump(skiatest::Reporter* reporter) {
    SkPath p;
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kWinding);\n");
    p = SkPathBuilder()
        .moveTo(1, 2)
        .lineTo(3, 4)
        .detach();
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kWinding);\n"
                                            "path.moveTo(1, 2);\n"
                                            "path.lineTo(3, 4);\n");
    p = SkPathBuilder(SkPathFillType::kEvenOdd)
        .moveTo(1, 2)
        .quadTo(3, 4, 5, 6)
        .detach();
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kEvenOdd);\n"
                                            "path.moveTo(1, 2);\n"
                                            "path.quadTo(3, 4, 5, 6);\n");
    p = SkPathBuilder(SkPathFillType::kInverseWinding)
        .moveTo(1, 2)
        .conicTo(3, 4, 5, 6, 0.5f)
        .detach();
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kInverseWinding);\n"
                                            "path.moveTo(1, 2);\n"
                                            "path.conicTo(3, 4, 5, 6, 0.5f);\n");
    p = SkPathBuilder(SkPathFillType::kInverseEvenOdd)
        .moveTo(1, 2)
        .cubicTo(3, 4, 5, 6, 7, 8)
        .detach();
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kInverseEvenOdd);\n"
                                            "path.moveTo(1, 2);\n"
                                            "path.cubicTo(3, 4, 5, 6, 7, 8);\n");
    p = SkPathBuilder(SkPathFillType::kWinding)
        .moveTo(1, 2)
        .lineTo(3, 4)
        .detach();
    compare_dump(reporter, p, true,
                 "path.setFillType(SkPathFillType::kWinding);\n"
                 "path.moveTo(SkBits2Float(0x3f800000), SkBits2Float(0x40000000));  // 1, 2\n"
                 "path.lineTo(SkBits2Float(0x40400000), SkBits2Float(0x40800000));  // 3, 4\n");
    p = SkPathBuilder(SkPathFillType::kWinding)
        .moveTo(SkBits2Float(0x3f800000), SkBits2Float(0x40000000))
        .lineTo(SkBits2Float(0x40400000), SkBits2Float(0x40800000))
        .detach();
    compare_dump(reporter, p, false, "path.setFillType(SkPathFillType::kWinding);\n"
                                            "path.moveTo(1, 2);\n"
                                            "path.lineTo(3, 4);\n");
}

namespace {

class ChangeListener : public SkIDChangeListener {
public:
    ChangeListener(bool *changed) : fChanged(changed) { *fChanged = false; }
    ~ChangeListener() override {}
    void changed() override { *fChanged = true; }

private:
    bool* fChanged;
};

}  // namespace

static void test_crbug_629455(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
//  AKA: cubicTo(-4.31596e+08f, -4.31602e+08f, -4.31602e+08f, -4.31602e+08f, 47.951f, 7.42423f);
                  .cubicTo(SkBits2Float(0xcdcdcd00), SkBits2Float(0xcdcdcdcd),
                           SkBits2Float(0xcdcdcdcd), SkBits2Float(0xcdcdcdcd),
                           SkBits2Float(0x423fcdcd), SkBits2Float(0x40ed9341))
                  .lineTo(0, 0)
                  .detach();
    test_draw_AA_path(100, 100, path);
}

static void test_fuzz_crbug_662952(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x4109999a), SkBits2Float(0x411c0000))  // 8.6f, 9.75f
                  .lineTo(SkBits2Float(0x410a6666), SkBits2Float(0x411c0000))  // 8.65f, 9.75f
                  .lineTo(SkBits2Float(0x410a6666), SkBits2Float(0x411e6666))  // 8.65f, 9.9f
                  .lineTo(SkBits2Float(0x4109999a), SkBits2Float(0x411e6666))  // 8.6f, 9.9f
                  .lineTo(SkBits2Float(0x4109999a), SkBits2Float(0x411c0000))  // 8.6f, 9.75f
                  .close()
                  .detach();

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    SkPaint paint;
    paint.setAntiAlias(true);
    surface->getCanvas()->clipPath(path, true);
    surface->getCanvas()->drawRect(SkRect::MakeWH(100, 100), paint);
}

static void test_path_crbugskia6003() {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(500, 500)));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x4325e666), SkBits2Float(0x42a1999a))  // 165.9f, 80.8f
                  .lineTo(SkBits2Float(0x4325e666), SkBits2Float(0x42a2999a))  // 165.9f, 81.3f
                  .lineTo(SkBits2Float(0x4325b333), SkBits2Float(0x42a2999a))  // 165.7f, 81.3f
                  .lineTo(SkBits2Float(0x4325b333), SkBits2Float(0x42a16666))  // 165.7f, 80.7f
                  .lineTo(SkBits2Float(0x4325b333), SkBits2Float(0x429f6666))  // 165.7f, 79.7f
                  // 165.7f, 79.7f, 165.8f, 79.7f, 165.8f, 79.7f
                  .cubicTo(SkBits2Float(0x4325b333), SkBits2Float(0x429f6666), SkBits2Float(0x4325cccc),
                            SkBits2Float(0x429f6666), SkBits2Float(0x4325cccc), SkBits2Float(0x429f6666))
                  // 165.8f, 79.7f, 165.8f, 79.7f, 165.9f, 79.7f
                  .cubicTo(SkBits2Float(0x4325cccc), SkBits2Float(0x429f6666), SkBits2Float(0x4325cccc),
                            SkBits2Float(0x429f6666), SkBits2Float(0x4325e666), SkBits2Float(0x429f6666))
                  .lineTo(SkBits2Float(0x4325e666), SkBits2Float(0x42a1999a))  // 165.9f, 80.8f
                  .close()
                  .detach();
    canvas->clipPath(path, true);
    canvas->drawRect(SkRect::MakeWH(500, 500), paint);
}

static void test_fuzz_crbug_662730(skiatest::Reporter* reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))  // 0, 0
                  .lineTo(SkBits2Float(0xd5394437), SkBits2Float(0x37373737))  // -1.2731e+13f, 1.09205e-05f
                  .lineTo(SkBits2Float(0x37373737), SkBits2Float(0x37373737))  // 1.09205e-05f, 1.09205e-05f
                  .lineTo(SkBits2Float(0x37373745), SkBits2Float(0x0001b800))  // 1.09205e-05f, 1.57842e-40f
                  .close()
                  .detach();
    test_draw_AA_path(100, 100, path);
}

static void test_skbug_6947() {
    const SkPoint points[] =
        {{125.126022f, -0.499872506f}, {125.288895f, -0.499338806f},
         {125.299316f, -0.499290764f}, {126.294594f, 0.505449712f},
         {125.999992f, 62.5047531f}, {124.0f, 62.4980202f},
         {124.122749f, 0.498142242f}, {125.126022f, -0.499872506f},
         {125.119476f, 1.50011659f}, {125.122749f, 0.50012207f},
         {126.122749f, 0.502101898f}, {126.0f, 62.5019798f},
         {125.0f, 62.5f}, {124.000008f, 62.4952469f},
         {124.294609f, 0.495946467f}, {125.294601f, 0.50069809f},
         {125.289886f, 1.50068688f}, {125.282349f, 1.50065041f},
         {125.119476f, 1.50011659f}};
    constexpr SkPathVerb kMove = SkPathVerb::kMove;
    constexpr SkPathVerb kLine = SkPathVerb::kLine;
    constexpr SkPathVerb kClose = SkPathVerb::kClose;
    const SkPathVerb verbs[] = {kMove, kLine, kLine, kLine, kLine, kLine, kLine, kLine, kClose,
            kMove, kLine, kLine, kLine, kLine, kLine, kLine, kLine, kLine, kLine, kLine, kClose};
    int pointIndex = 0;
    SkPathBuilder builder;
    for(auto verb : verbs) {
        switch (verb) {
            case kMove:
                builder.moveTo(points[pointIndex++]);
                break;
            case kLine:
                builder.lineTo(points[pointIndex++]);
                break;
            case kClose:
            default:
                builder.close();
                break;
        }
    }
    test_draw_AA_path(250, 125, builder.detach());
}

static void test_skbug_7015() {
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(SkBits2Float(0x4388c000), SkBits2Float(0x43947c08))  // 273.5f, 296.969f
                  .lineTo(SkBits2Float(0x4386c000), SkBits2Float(0x43947c08))  // 269.5f, 296.969f
                  // 269.297f, 292.172f, 273.695f, 292.172f, 273.5f, 296.969f
                  .cubicTo(SkBits2Float(0x4386a604), SkBits2Float(0x43921604),
                           SkBits2Float(0x4388d8f6), SkBits2Float(0x43921604),
                           SkBits2Float(0x4388c000), SkBits2Float(0x43947c08))
                  .close()
                  .detach();
    test_draw_AA_path(500, 500, path);
}

static void test_skbug_7051() {
    SkPath path = SkPathBuilder()
                  .moveTo(10, 10)
                  .cubicTo(10, 20, 10, 30, 30, 30)
                  .lineTo(50, 20)
                  .lineTo(50, 10)
                  .close()
                  .detach();
    test_draw_AA_path(100, 100, path);
}

static void test_skbug_7435() {
    SkPaint paint;
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
        .moveTo(SkBits2Float(0x7f07a5af), SkBits2Float(0xff07ff1d))  // 1.80306e+38f, -1.8077e+38f
        .lineTo(SkBits2Float(0x7edf4b2d), SkBits2Float(0xfedffe0a))  // 1.48404e+38f, -1.48868e+38f
        .lineTo(SkBits2Float(0x7edf4585), SkBits2Float(0xfee003b2))  // 1.48389e+38f, -1.48883e+38f
        .lineTo(SkBits2Float(0x7ef348e9), SkBits2Float(0xfef403c6))  // 1.6169e+38f, -1.62176e+38f
        .lineTo(SkBits2Float(0x7ef74c4e), SkBits2Float(0xfef803cb))  // 1.64358e+38f, -1.64834e+38f
        .conicTo(SkBits2Float(0x7ef74f23), SkBits2Float(0xfef8069e), SkBits2Float(0x7ef751f6), SkBits2Float(0xfef803c9), SkBits2Float(0x3f3504f3))  // 1.64365e+38f, -1.64841e+38f, 1.64372e+38f, -1.64834e+38f, 0.707107f
        .conicTo(SkBits2Float(0x7ef754c8), SkBits2Float(0xfef800f5), SkBits2Float(0x7ef751f5), SkBits2Float(0xfef7fe22), SkBits2Float(0x3f353472))  // 1.6438e+38f, -1.64827e+38f, 1.64372e+38f, -1.64819e+38f, 0.707832f
        .lineTo(SkBits2Float(0x7edb57a9), SkBits2Float(0xfedbfe06))  // 1.45778e+38f, -1.4621e+38f
        .lineTo(SkBits2Float(0x7e875976), SkBits2Float(0xfe87fdb3))  // 8.99551e+37f, -9.03815e+37f
        .lineTo(SkBits2Float(0x7ded5c2b), SkBits2Float(0xfdeff59e))  // 3.94382e+37f, -3.98701e+37f
        .lineTo(SkBits2Float(0x7d7a78a7), SkBits2Float(0xfd7fda0f))  // 2.08083e+37f, -2.12553e+37f
        .lineTo(SkBits2Float(0x7d7a6403), SkBits2Float(0xfd7fe461))  // 2.08016e+37f, -2.12587e+37f
        .conicTo(SkBits2Float(0x7d7a4764), SkBits2Float(0xfd7ff2b0), SkBits2Float(0x7d7a55b4), SkBits2Float(0xfd8007a8), SkBits2Float(0x3f3504f3))  // 2.07924e+37f, -2.12633e+37f, 2.0797e+37f, -2.12726e+37f, 0.707107f
        .conicTo(SkBits2Float(0x7d7a5803), SkBits2Float(0xfd8009f7), SkBits2Float(0x7d7a5ba9), SkBits2Float(0xfd800bcc), SkBits2Float(0x3f7cba66))  // 2.07977e+37f, -2.12741e+37f, 2.07989e+37f, -2.12753e+37f, 0.987219f
        .lineTo(SkBits2Float(0x7d8d2067), SkBits2Float(0xfd900bdb))  // 2.34487e+37f, -2.39338e+37f
        .lineTo(SkBits2Float(0x7ddd137a), SkBits2Float(0xfde00c2d))  // 3.67326e+37f, -3.72263e+37f
        .lineTo(SkBits2Float(0x7ddd2a1b), SkBits2Float(0xfddff58e))  // 3.67473e+37f, -3.72116e+37f
        .lineTo(SkBits2Float(0x7c694ae5), SkBits2Float(0xfc7fa67c))  // 4.8453e+36f, -5.30965e+36f
        .lineTo(SkBits2Float(0xfc164a8b), SkBits2Float(0x7c005af5))  // -3.12143e+36f, 2.66584e+36f
        .lineTo(SkBits2Float(0xfc8ae983), SkBits2Float(0x7c802da7))  // -5.77019e+36f, 5.32432e+36f
        .lineTo(SkBits2Float(0xfc8b16d9), SkBits2Float(0x7c80007b))  // -5.77754e+36f, 5.31699e+36f
        .lineTo(SkBits2Float(0xfc8b029c), SkBits2Float(0x7c7f8788))  // -5.77426e+36f, 5.30714e+36f
        .lineTo(SkBits2Float(0xfc8b0290), SkBits2Float(0x7c7f8790))  // -5.77425e+36f, 5.30714e+36f
        .lineTo(SkBits2Float(0xfc8b16cd), SkBits2Float(0x7c80007f))  // -5.77753e+36f, 5.31699e+36f
        .lineTo(SkBits2Float(0xfc8b4409), SkBits2Float(0x7c7fa672))  // -5.78487e+36f, 5.30965e+36f
        .lineTo(SkBits2Float(0x7d7aa2ba), SkBits2Float(0xfd800bd1))  // 2.0822e+37f, -2.12753e+37f
        .lineTo(SkBits2Float(0x7e8757ee), SkBits2Float(0xfe88035b))  // 8.99512e+37f, -9.03962e+37f
        .lineTo(SkBits2Float(0x7ef7552d), SkBits2Float(0xfef803ca))  // 1.64381e+38f, -1.64834e+38f
        .lineTo(SkBits2Float(0x7f0fa653), SkBits2Float(0xff1001f9))  // 1.90943e+38f, -1.91419e+38f
        .lineTo(SkBits2Float(0x7f0fa926), SkBits2Float(0xff0fff24))  // 1.90958e+38f, -1.91404e+38f
        .lineTo(SkBits2Float(0x7f0da75c), SkBits2Float(0xff0dff22))  // 1.8829e+38f, -1.88746e+38f
        .lineTo(SkBits2Float(0x7f07a5af), SkBits2Float(0xff07ff1d))  // 1.80306e+38f, -1.8077e+38f
        .close()
        .moveTo(SkBits2Float(0x7f07a2db), SkBits2Float(0xff0801f1))  // 1.80291e+38f, -1.80785e+38f
        .lineTo(SkBits2Float(0x7f0da48a), SkBits2Float(0xff0e01f8))  // 1.88275e+38f, -1.88761e+38f
        .lineTo(SkBits2Float(0x7f0fa654), SkBits2Float(0xff1001fa))  // 1.90943e+38f, -1.91419e+38f
        .lineTo(SkBits2Float(0x7f0fa7bd), SkBits2Float(0xff10008f))  // 1.90951e+38f, -1.91412e+38f
        .lineTo(SkBits2Float(0x7f0fa927), SkBits2Float(0xff0fff25))  // 1.90958e+38f, -1.91404e+38f
        .lineTo(SkBits2Float(0x7ef75ad5), SkBits2Float(0xfef7fe22))  // 1.64395e+38f, -1.64819e+38f
        .lineTo(SkBits2Float(0x7e875d96), SkBits2Float(0xfe87fdb3))  // 8.99659e+37f, -9.03815e+37f
        .lineTo(SkBits2Float(0x7d7acff6), SkBits2Float(0xfd7fea5b))  // 2.08367e+37f, -2.12606e+37f
        .lineTo(SkBits2Float(0xfc8b0588), SkBits2Float(0x7c8049b7))  // -5.77473e+36f, 5.32887e+36f
        .lineTo(SkBits2Float(0xfc8b2b16), SkBits2Float(0x7c803d32))  // -5.78083e+36f, 5.32684e+36f
        .conicTo(SkBits2Float(0xfc8b395c), SkBits2Float(0x7c803870), SkBits2Float(0xfc8b4405), SkBits2Float(0x7c802dd1), SkBits2Float(0x3f79349d))  // -5.78314e+36f, 5.32607e+36f, -5.78487e+36f, 5.32435e+36f, 0.973459f
        .conicTo(SkBits2Float(0xfc8b715b), SkBits2Float(0x7c8000a5), SkBits2Float(0xfc8b442f), SkBits2Float(0x7c7fa69e), SkBits2Float(0x3f3504f3))  // -5.79223e+36f, 5.31702e+36f, -5.7849e+36f, 5.30966e+36f, 0.707107f
        .lineTo(SkBits2Float(0xfc16ffaa), SkBits2Float(0x7bff4c12))  // -3.13612e+36f, 2.65116e+36f
        .lineTo(SkBits2Float(0x7c6895e0), SkBits2Float(0xfc802dc0))  // 4.83061e+36f, -5.32434e+36f
        .lineTo(SkBits2Float(0x7ddd137b), SkBits2Float(0xfde00c2e))  // 3.67326e+37f, -3.72263e+37f
        .lineTo(SkBits2Float(0x7ddd1ecb), SkBits2Float(0xfde000de))  // 3.67399e+37f, -3.72189e+37f
        .lineTo(SkBits2Float(0x7ddd2a1c), SkBits2Float(0xfddff58f))  // 3.67473e+37f, -3.72116e+37f
        .lineTo(SkBits2Float(0x7d8d3711), SkBits2Float(0xfd8ff543))  // 2.34634e+37f, -2.39191e+37f
        .lineTo(SkBits2Float(0x7d7a88fe), SkBits2Float(0xfd7fea69))  // 2.08136e+37f, -2.12606e+37f
        .lineTo(SkBits2Float(0x7d7a7254), SkBits2Float(0xfd800080))  // 2.08063e+37f, -2.1268e+37f
        .lineTo(SkBits2Float(0x7d7a80a4), SkBits2Float(0xfd800ed0))  // 2.08109e+37f, -2.12773e+37f
        .lineTo(SkBits2Float(0x7d7a80a8), SkBits2Float(0xfd800ecf))  // 2.08109e+37f, -2.12773e+37f
        .lineTo(SkBits2Float(0x7d7a7258), SkBits2Float(0xfd80007f))  // 2.08063e+37f, -2.1268e+37f
        .lineTo(SkBits2Float(0x7d7a5bb9), SkBits2Float(0xfd800bd0))  // 2.0799e+37f, -2.12753e+37f
        .lineTo(SkBits2Float(0x7ded458b), SkBits2Float(0xfdf00c3e))  // 3.94235e+37f, -3.98848e+37f
        .lineTo(SkBits2Float(0x7e8753ce), SkBits2Float(0xfe88035b))  // 8.99405e+37f, -9.03962e+37f
        .lineTo(SkBits2Float(0x7edb5201), SkBits2Float(0xfedc03ae))  // 1.45763e+38f, -1.46225e+38f
        .lineTo(SkBits2Float(0x7ef74c4d), SkBits2Float(0xfef803ca))  // 1.64358e+38f, -1.64834e+38f
        .lineTo(SkBits2Float(0x7ef74f21), SkBits2Float(0xfef800f6))  // 1.64365e+38f, -1.64827e+38f
        .lineTo(SkBits2Float(0x7ef751f4), SkBits2Float(0xfef7fe21))  // 1.64372e+38f, -1.64819e+38f
        .lineTo(SkBits2Float(0x7ef34e91), SkBits2Float(0xfef3fe1e))  // 1.61705e+38f, -1.62161e+38f
        .lineTo(SkBits2Float(0x7edf4b2d), SkBits2Float(0xfedffe0a))  // 1.48404e+38f, -1.48868e+38f
        .lineTo(SkBits2Float(0x7edf4859), SkBits2Float(0xfee000de))  // 1.48397e+38f, -1.48876e+38f
        .lineTo(SkBits2Float(0x7edf4585), SkBits2Float(0xfee003b2))  // 1.48389e+38f, -1.48883e+38f
        .lineTo(SkBits2Float(0x7f07a2db), SkBits2Float(0xff0801f1))  // 1.80291e+38f, -1.80785e+38f
        .close()
        .moveTo(SkBits2Float(0xfab120db), SkBits2Float(0x77b50b4f))  // -4.59851e+35f, 7.34402e+33f
        .lineTo(SkBits2Float(0xfd6597e5), SkBits2Float(0x7d60177f))  // -1.90739e+37f, 1.86168e+37f
        .lineTo(SkBits2Float(0xfde2cea1), SkBits2Float(0x7de00c2e))  // -3.76848e+37f, 3.72263e+37f
        .lineTo(SkBits2Float(0xfe316511), SkBits2Float(0x7e300657))  // -5.89495e+37f, 5.84943e+37f
        .lineTo(SkBits2Float(0xfe415da1), SkBits2Float(0x7e400666))  // -6.42568e+37f, 6.38112e+37f
        .lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e4000be))  // -6.42641e+37f, 6.38039e+37f
        .lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e3ff8be))  // -6.42641e+37f, 6.37935e+37f
        .lineTo(SkBits2Float(0xfe416349), SkBits2Float(0x7e3ff8be))  // -6.42641e+37f, 6.37935e+37f
        .lineTo(SkBits2Float(0xfe415f69), SkBits2Float(0x7e3ff8be))  // -6.42591e+37f, 6.37935e+37f
        .lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e3ff8be))  // -6.42544e+37f, 6.37935e+37f
        .lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e4000be))  // -6.42544e+37f, 6.38039e+37f
        .lineTo(SkBits2Float(0xfe416171), SkBits2Float(0x7e3ffb16))  // -6.42617e+37f, 6.37966e+37f
        .lineTo(SkBits2Float(0xfe016131), SkBits2Float(0x7dfff5ae))  // -4.29938e+37f, 4.25286e+37f
        .lineTo(SkBits2Float(0xfe0155e2), SkBits2Float(0x7e000628))  // -4.29791e+37f, 4.25433e+37f
        .lineTo(SkBits2Float(0xfe0958ea), SkBits2Float(0x7e080630))  // -4.56415e+37f, 4.52018e+37f
        .lineTo(SkBits2Float(0xfe115c92), SkBits2Float(0x7e100638))  // -4.83047e+37f, 4.78603e+37f
        .conicTo(SkBits2Float(0xfe11623c), SkBits2Float(0x7e100bdf), SkBits2Float(0xfe1167e2), SkBits2Float(0x7e100636), SkBits2Float(0x3f3504f3))  // -4.8312e+37f, 4.78676e+37f, -4.83194e+37f, 4.78603e+37f, 0.707107f
        .conicTo(SkBits2Float(0xfe116d87), SkBits2Float(0x7e10008e), SkBits2Float(0xfe1167e2), SkBits2Float(0x7e0ffae8), SkBits2Float(0x3f35240a))  // -4.83267e+37f, 4.78529e+37f, -4.83194e+37f, 4.78456e+37f, 0.707581f
        .lineTo(SkBits2Float(0xfe016b92), SkBits2Float(0x7dfff5af))  // -4.30072e+37f, 4.25286e+37f
        .lineTo(SkBits2Float(0xfdc2d963), SkBits2Float(0x7dbff56e))  // -3.23749e+37f, 3.18946e+37f
        .lineTo(SkBits2Float(0xfd65ae25), SkBits2Float(0x7d5fea3d))  // -1.90811e+37f, 1.86021e+37f
        .lineTo(SkBits2Float(0xfab448de), SkBits2Float(0xf7b50a19))  // -4.68046e+35f, -7.34383e+33f
        .lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x43480000))  // -4.60703e+35f, 200
        .lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x7800007f))  // -4.60703e+35f, 1.03848e+34f
        .lineTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x7800007f))  // -4.67194e+35f, 1.03848e+34f
        .lineTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000))  // -4.67194e+35f, 200
        .lineTo(SkBits2Float(0xfab120db), SkBits2Float(0x77b50b4f))  // -4.59851e+35f, 7.34402e+33f
        .close()
        .moveTo(SkBits2Float(0xfab59cf2), SkBits2Float(0xf800007e))  // -4.71494e+35f, -1.03847e+34f
        .lineTo(SkBits2Float(0xfaa7cc52), SkBits2Float(0xf800007f))  // -4.35629e+35f, -1.03848e+34f
        .lineTo(SkBits2Float(0xfd6580e5), SkBits2Float(0x7d60177f))  // -1.90664e+37f, 1.86168e+37f
        .lineTo(SkBits2Float(0xfdc2c2c1), SkBits2Float(0x7dc00c0f))  // -3.23602e+37f, 3.19093e+37f
        .lineTo(SkBits2Float(0xfe016040), SkBits2Float(0x7e000626))  // -4.29925e+37f, 4.25433e+37f
        .lineTo(SkBits2Float(0xfe115c90), SkBits2Float(0x7e100636))  // -4.83047e+37f, 4.78603e+37f
        .lineTo(SkBits2Float(0xfe116239), SkBits2Float(0x7e10008f))  // -4.8312e+37f, 4.78529e+37f
        .lineTo(SkBits2Float(0xfe1167e0), SkBits2Float(0x7e0ffae6))  // -4.83194e+37f, 4.78456e+37f
        .lineTo(SkBits2Float(0xfe096438), SkBits2Float(0x7e07fade))  // -4.56562e+37f, 4.51871e+37f
        .lineTo(SkBits2Float(0xfe016130), SkBits2Float(0x7dfff5ac))  // -4.29938e+37f, 4.25286e+37f
        .lineTo(SkBits2Float(0xfe015b89), SkBits2Float(0x7e00007f))  // -4.29864e+37f, 4.25359e+37f
        .lineTo(SkBits2Float(0xfe0155e1), SkBits2Float(0x7e000627))  // -4.29791e+37f, 4.25433e+37f
        .lineTo(SkBits2Float(0xfe415879), SkBits2Float(0x7e4008bf))  // -6.42501e+37f, 6.38143e+37f
        .lineTo(SkBits2Float(0xfe415f69), SkBits2Float(0x7e4008bf))  // -6.42591e+37f, 6.38143e+37f
        .lineTo(SkBits2Float(0xfe416349), SkBits2Float(0x7e4008bf))  // -6.42641e+37f, 6.38143e+37f
        .lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e4008bf))  // -6.42641e+37f, 6.38143e+37f
        .conicTo(SkBits2Float(0xfe416699), SkBits2Float(0x7e4008bf), SkBits2Float(0xfe4168f1), SkBits2Float(0x7e400668), SkBits2Float(0x3f6c8ed9))  // -6.42684e+37f, 6.38143e+37f, -6.42715e+37f, 6.38113e+37f, 0.924055f
        .conicTo(SkBits2Float(0xfe416e9a), SkBits2Float(0x7e4000c2), SkBits2Float(0xfe4168f3), SkBits2Float(0x7e3ffb17), SkBits2Float(0x3f3504f3))  // -6.42788e+37f, 6.38039e+37f, -6.42715e+37f, 6.37966e+37f, 0.707107f
        .lineTo(SkBits2Float(0xfe317061), SkBits2Float(0x7e2ffb07))  // -5.89642e+37f, 5.84796e+37f
        .lineTo(SkBits2Float(0xfde2e542), SkBits2Float(0x7ddff58e))  // -3.76995e+37f, 3.72116e+37f
        .lineTo(SkBits2Float(0xfd65c525), SkBits2Float(0x7d5fea3d))  // -1.90886e+37f, 1.86021e+37f
        .lineTo(SkBits2Float(0xfab6c8db), SkBits2Float(0xf7b50b4f))  // -4.74536e+35f, -7.34402e+33f
        .lineTo(SkBits2Float(0xfab59cf2), SkBits2Float(0xf800007e))  // -4.71494e+35f, -1.03847e+34f
        .close()
        .moveTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000))  // -4.67194e+35f, 200
        .lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x43480000))  // -4.60703e+35f, 200
        .quadTo(SkBits2Float(0xfd0593a5), SkBits2Float(0x7d00007f), SkBits2Float(0xfd659785), SkBits2Float(0x7d6000de))  // -1.10971e+37f, 1.0634e+37f, -1.90737e+37f, 1.86095e+37f
        .quadTo(SkBits2Float(0xfda2cdf2), SkBits2Float(0x7da0009f), SkBits2Float(0xfdc2ce12), SkBits2Float(0x7dc000be))  // -2.70505e+37f, 2.6585e+37f, -3.23675e+37f, 3.1902e+37f
        .quadTo(SkBits2Float(0xfde2ce31), SkBits2Float(0x7de000de), SkBits2Float(0xfe0165e9), SkBits2Float(0x7e00007f))  // -3.76845e+37f, 3.72189e+37f, -4.29999e+37f, 4.25359e+37f
        .quadTo(SkBits2Float(0xfe1164b9), SkBits2Float(0x7e10008f), SkBits2Float(0xfe116239), SkBits2Float(0x7e10008f))  // -4.83153e+37f, 4.78529e+37f, -4.8312e+37f, 4.78529e+37f
        .quadTo(SkBits2Float(0xfe116039), SkBits2Float(0x7e10008f), SkBits2Float(0xfe095e91), SkBits2Float(0x7e080087))  // -4.83094e+37f, 4.78529e+37f, -4.56488e+37f, 4.51944e+37f
        .quadTo(SkBits2Float(0xfe015d09), SkBits2Float(0x7e00007f), SkBits2Float(0xfe015b89), SkBits2Float(0x7e00007f))  // -4.29884e+37f, 4.25359e+37f, -4.29864e+37f, 4.25359e+37f
        .lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e4000be))  // -6.42544e+37f, 6.38039e+37f
        .quadTo(SkBits2Float(0xfe415da9), SkBits2Float(0x7e4000be), SkBits2Float(0xfe415f69), SkBits2Float(0x7e4000be))  // -6.42568e+37f, 6.38039e+37f, -6.42591e+37f, 6.38039e+37f
        .quadTo(SkBits2Float(0xfe416149), SkBits2Float(0x7e4000be), SkBits2Float(0xfe416349), SkBits2Float(0x7e4000be))  // -6.42615e+37f, 6.38039e+37f, -6.42641e+37f, 6.38039e+37f
        .quadTo(SkBits2Float(0xfe416849), SkBits2Float(0x7e4000be), SkBits2Float(0xfe316ab9), SkBits2Float(0x7e3000af))  // -6.42706e+37f, 6.38039e+37f, -5.89569e+37f, 5.84869e+37f
        .quadTo(SkBits2Float(0xfe216d29), SkBits2Float(0x7e20009f), SkBits2Float(0xfde2d9f2), SkBits2Float(0x7de000de))  // -5.36431e+37f, 5.31699e+37f, -3.76921e+37f, 3.72189e+37f
        .quadTo(SkBits2Float(0xfda2d9b2), SkBits2Float(0x7da0009f), SkBits2Float(0xfd65ae85), SkBits2Float(0x7d6000de))  // -2.70582e+37f, 2.6585e+37f, -1.90812e+37f, 1.86095e+37f
        .quadTo(SkBits2Float(0xfd05a9a6), SkBits2Float(0x7d00007f), SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000))  // -1.11043e+37f, 1.0634e+37f, -4.67194e+35f, 200
        .close()
        .moveTo(SkBits2Float(0x7f07a445), SkBits2Float(0xff080087))  // 1.80299e+38f, -1.80778e+38f
        .quadTo(SkBits2Float(0x7f0ba519), SkBits2Float(0xff0c008b), SkBits2Float(0x7f0da5f3), SkBits2Float(0xff0e008d))  // 1.8562e+38f, -1.86095e+38f, 1.88283e+38f, -1.88753e+38f
        .quadTo(SkBits2Float(0x7f0fa6d5), SkBits2Float(0xff10008f), SkBits2Float(0x7f0fa7bd), SkBits2Float(0xff10008f))  // 1.90946e+38f, -1.91412e+38f, 1.90951e+38f, -1.91412e+38f
        .quadTo(SkBits2Float(0x7f0faa7d), SkBits2Float(0xff10008f), SkBits2Float(0x7ef75801), SkBits2Float(0xfef800f6))  // 1.90965e+38f, -1.91412e+38f, 1.64388e+38f, -1.64827e+38f
        .quadTo(SkBits2Float(0x7ecf5b09), SkBits2Float(0xfed000ce), SkBits2Float(0x7e875ac2), SkBits2Float(0xfe880087))  // 1.37811e+38f, -1.38242e+38f, 8.99585e+37f, -9.03889e+37f
        .quadTo(SkBits2Float(0x7e0eb505), SkBits2Float(0xfe10008f), SkBits2Float(0x7d7ab958), SkBits2Float(0xfd80007f))  // 4.74226e+37f, -4.78529e+37f, 2.08293e+37f, -2.1268e+37f
        .quadTo(SkBits2Float(0xfc8ac1cd), SkBits2Float(0x7c80007f), SkBits2Float(0xfc8b16cd), SkBits2Float(0x7c80007f))  // -5.76374e+36f, 5.31699e+36f, -5.77753e+36f, 5.31699e+36f
        .quadTo(SkBits2Float(0xfc8b36cd), SkBits2Float(0x7c80007f), SkBits2Float(0xfc16a51a), SkBits2Float(0x7c00007f))  // -5.78273e+36f, 5.31699e+36f, -3.12877e+36f, 2.6585e+36f
        .quadTo(SkBits2Float(0xfab6e4de), SkBits2Float(0x43480000), SkBits2Float(0x7c68f062), SkBits2Float(0xfc80007f))  // -4.7482e+35f, 200, 4.83795e+36f, -5.31699e+36f
        .lineTo(SkBits2Float(0x7ddd1ecb), SkBits2Float(0xfde000de))  // 3.67399e+37f, -3.72189e+37f
        .quadTo(SkBits2Float(0x7d9d254b), SkBits2Float(0xfda0009f), SkBits2Float(0x7d8d2bbc), SkBits2Float(0xfd90008f))  // 2.61103e+37f, -2.6585e+37f, 2.3456e+37f, -2.39265e+37f
        .quadTo(SkBits2Float(0x7d7a64d8), SkBits2Float(0xfd80007f), SkBits2Float(0x7d7a7258), SkBits2Float(0xfd80007f))  // 2.08019e+37f, -2.1268e+37f, 2.08063e+37f, -2.1268e+37f
        .quadTo(SkBits2Float(0x7d7a9058), SkBits2Float(0xfd80007f), SkBits2Float(0x7ded50db), SkBits2Float(0xfdf000ee))  // 2.0816e+37f, -2.1268e+37f, 3.94309e+37f, -3.98774e+37f
        .quadTo(SkBits2Float(0x7e2eace5), SkBits2Float(0xfe3000af), SkBits2Float(0x7e8756a2), SkBits2Float(0xfe880087))  // 5.80458e+37f, -5.84869e+37f, 8.99478e+37f, -9.03889e+37f
        .quadTo(SkBits2Float(0x7ebf56d9), SkBits2Float(0xfec000be), SkBits2Float(0x7edb54d5), SkBits2Float(0xfedc00da))  // 1.27167e+38f, -1.27608e+38f, 1.45771e+38f, -1.46217e+38f
        .quadTo(SkBits2Float(0x7ef752e1), SkBits2Float(0xfef800f6), SkBits2Float(0x7ef74f21), SkBits2Float(0xfef800f6))  // 1.64375e+38f, -1.64827e+38f, 1.64365e+38f, -1.64827e+38f
        .quadTo(SkBits2Float(0x7ef74d71), SkBits2Float(0xfef800f6), SkBits2Float(0x7ef34bbd), SkBits2Float(0xfef400f2))  // 1.64361e+38f, -1.64827e+38f, 1.61698e+38f, -1.62168e+38f
        .quadTo(SkBits2Float(0x7eef4a19), SkBits2Float(0xfef000ee), SkBits2Float(0x7edf4859), SkBits2Float(0xfee000de))  // 1.59035e+38f, -1.5951e+38f, 1.48397e+38f, -1.48876e+38f
        .lineTo(SkBits2Float(0x7f07a445), SkBits2Float(0xff080087))  // 1.80299e+38f, -1.80778e+38f
        .close()
        .detach();
    SkSurfaces::Raster(SkImageInfo::MakeN32Premul(250, 250), nullptr)
            ->getCanvas()
            ->drawPath(path, paint);
}

static void test_interp(skiatest::Reporter* reporter) {
    SkPath p1, p2, out;
    REPORTER_ASSERT(reporter, p1.isInterpolatable(p2));
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 0, &out));
    REPORTER_ASSERT(reporter, p1 == out);
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 1, &out));
    REPORTER_ASSERT(reporter, p1 == out);
    p1 = SkPathBuilder().moveTo(0, 2).lineTo(0, 4).detach();
    REPORTER_ASSERT(reporter, !p1.isInterpolatable(p2));
    REPORTER_ASSERT(reporter, !p1.interpolate(p2, 1, &out));
    p2 = SkPathBuilder().moveTo(6, 0).lineTo(8, 0).detach();
    REPORTER_ASSERT(reporter, p1.isInterpolatable(p2));
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 0, &out));
    REPORTER_ASSERT(reporter, p2 == out);
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 1, &out));
    REPORTER_ASSERT(reporter, p1 == out);
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 0.5f, &out));
    REPORTER_ASSERT(reporter, out.getBounds() == SkRect::MakeLTRB(3, 1, 4, 2));
    p1 = SkPathBuilder()
         .moveTo(4, 4)
         .conicTo(5, 4, 5, 5, 1 / SkScalarSqrt(2))
         .detach();
    p2 = SkPathBuilder()
         .moveTo(4, 2)
         .conicTo(7, 2, 7, 5, 1 / SkScalarSqrt(2))
         .detach();
    REPORTER_ASSERT(reporter, p1.isInterpolatable(p2));
    REPORTER_ASSERT(reporter, p1.interpolate(p2, 0.5f, &out));
    REPORTER_ASSERT(reporter, out.getBounds() == SkRect::MakeLTRB(4, 3, 6, 5));
    p2 = SkPathBuilder()
         .moveTo(4, 2)
         .conicTo(6, 3, 6, 5, 1)
         .detach();
    REPORTER_ASSERT(reporter, !p1.isInterpolatable(p2));
    p2 = SkPathBuilder()
         .moveTo(4, 4)
         .conicTo(5, 4, 5, 5, 0.5f)
         .detach();
    REPORTER_ASSERT(reporter, !p1.isInterpolatable(p2));
}

DEF_TEST(PathInterp, reporter) {
    test_interp(reporter);
}

DEF_TEST(PathBigCubic, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000))  // 0, 0
                  .moveTo(SkBits2Float(0x44000000), SkBits2Float(0x373938b8))  // 512, 1.10401e-05f
                  .cubicTo(SkBits2Float(0x00000001), SkBits2Float(0xdf000052),
                           SkBits2Float(0x00000100), SkBits2Float(0x00000000),
                           SkBits2Float(0x00000100), SkBits2Float(0x00000000))  // 1.4013e-45f, -9.22346e+18f, 3.58732e-43f, 0, 3.58732e-43f, 0
                  .moveTo(0, 512)
                  .detach();

    // this call should not assert
    SkSurfaces::Raster(SkImageInfo::MakeN32Premul(255, 255), nullptr)
            ->getCanvas()
            ->drawPath(path, SkPaint());
}

DEF_TEST(PathContains, reporter) {
    test_contains(reporter);
}

DEF_TEST(Paths, reporter) {
    test_fuzz_crbug_647922();
    test_fuzz_crbug_643933();
    test_sect_with_horizontal_needs_pinning();
    test_iterative_intersect_line();
    test_crbug_629455(reporter);
    test_fuzz_crbug_627414(reporter);
    test_path_crbug364224();
    test_fuzz_crbug_662952(reporter);
    test_fuzz_crbug_662730(reporter);
    test_fuzz_crbug_662780();
    test_mask_overflow();
    test_path_crbugskia6003();
    test_fuzz_crbug_668907();
    test_skbug_6947();
    test_skbug_7015();
    test_skbug_7051();
    test_skbug_7435();

    SkSize::Make(3, 4);

    SkPath  p, empty;
    test_empty(reporter, p);

    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());

    // this triggers a code path in SkPath::operator= which is otherwise unexercised
    SkPath& self = p;
    p = self;

    // this triggers a code path in SkPath::swap which is otherwise unexercised
    p.swap(self);

    test_operatorEqual(reporter);
    test_isLine(reporter);
    test_isRect(reporter);
    test_is_closed_rect(reporter);
    test_isNestedFillRects(reporter);
    test_zero_length_paths(reporter);
    test_direction(reporter);
    test_convexity(reporter);
    test_convexity2(reporter);
    test_convexity_doubleback(reporter);
    test_conservativelyContains(reporter);
    test_close(reporter);
    test_segment_masks(reporter);
    test_flattening(reporter);
    test_transform(reporter);
    test_bounds(reporter);
    test_iter(reporter);
    test_range_iter(reporter);
    test_circle(reporter);
    test_oval(reporter);
    test_strokerec(reporter);
    test_addPoly(reporter);
    test_isfinite(reporter);
    test_isfinite_after_transform(reporter);
    test_arb_round_rect_is_convex(reporter);
    test_arb_zero_rad_round_rect_is_rect(reporter);
    test_addrect(reporter);
    test_addrect_isfinite(reporter);
    test_tricky_cubic();
    test_clipped_cubic();
    test_crbug_170666();
    test_crbug_493450(reporter);
    test_crbug_495894(reporter);
    test_crbug_613918();
    test_bad_cubic_crbug229478();
    test_bad_cubic_crbug234190();
    test_path_close_issue1474(reporter);
    test_path_to_region(reporter);
    test_rrect(reporter);
    test_rMoveTo(reporter);
    test_arc(reporter);
    test_arc_ovals(reporter);
    test_arcTo(reporter);
    test_addPath(reporter);
    test_addPathMode(reporter, false, false);
    test_addPathMode(reporter, true, false);
    test_addPathMode(reporter, false, true);
    test_addPathMode(reporter, true, true);
    test_extendClosedPath(reporter);
    test_addEmptyPath(reporter, SkPath::kExtend_AddPathMode);
    test_addEmptyPath(reporter, SkPath::kAppend_AddPathMode);
    test_contains(reporter);
    test_dump(reporter);
    test_path_crbug389050(reporter);
    test_path_crbugskia2820(reporter);
    test_path_crbugskia5995();
    test_skbug_3469(reporter);
    test_skbug_3239(reporter);
    test_bounds_crbug_513799(reporter);
    test_fuzz_crbug_638223();
}

DEF_TEST(conservatively_contains_rect, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(SkBits2Float(0x44000000), SkBits2Float(0x373938b8))  // 512, 1.10401e-05f
                  // 1.4013e-45f, -9.22346e+18f, 3.58732e-43f, 0, 3.58732e-43f, 0
                  .cubicTo(SkBits2Float(0x00000001), SkBits2Float(0xdf000052),
                           SkBits2Float(0x00000100), SkBits2Float(0x00000000),
                           SkBits2Float(0x00000100), SkBits2Float(0x00000000))
                  .moveTo(0, 0)
                  .detach();

    // this should not assert
    path.conservativelyContainsRect({ -211747, 12.1115f, -197893, 25.0321f });
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_TEST(skbug_6450, r) {
    SkRect ri = { 0.18554693f, 195.26283f, 0.185784385f, 752.644409f };
    SkVector rdi[4] = {
        { 1.81159976e-09f, 7.58768801e-05f },
        { 0.000118725002f, 0.000118725002f },
        { 0.000118725002f, 0.000118725002f },
        { 0.000118725002f, 0.486297607f }
    };
    SkRRect irr;
    irr.setRectRadii(ri, rdi);
    SkRect ro = { 9.18354821e-39f, 2.1710848e+9f, 2.16945843e+9f, 3.47808128e+9f };
    SkVector rdo[4] = {
        { 0, 0 },
        { 0.0103298295f, 0.185887396f },
        { 2.52999727e-29f, 169.001938f },
        { 195.262741f, 195.161255f }
    };
    SkRRect orr;
    orr.setRectRadii(ro, rdo);
    SkMakeNullCanvas()->drawDRRect(orr, irr, SkPaint());
}


DEF_TEST(NonFinitePathIteration, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(SK_ScalarInfinity, SK_ScalarInfinity)
                  .detach();
    SkPathPriv::Iterate iterate(path);
    REPORTER_ASSERT(reporter, iterate.begin() == iterate.end());
}

DEF_TEST(AndroidArc, reporter) {
    const char* tests[] = {
         "M50,0A50,50,0,0 1 100,50 L100,85 A15,15,0,0 1 85,100 L50,100 A50,50,0,0 1 50,0z",
        ("M50,0L92,0 A8,8,0,0 1 100,8 L100,92 A8,8,0,0 1 92,100 L8,100"
            " A8,8,0,0 1 0,92 L 0,8 A8,8,0,0 1 8,0z"),
         "M50 0A50 50,0,1,1,50 100A50 50,0,1,1,50 0"
    };
    for (auto test : tests) {
        const auto aPath = SkParsePath::FromSVGString(test);
        SkAssertResult(aPath.has_value());
        SkASSERT(aPath->isConvex());
        for (SkScalar scale = 1; scale < 1000; scale *= 1.1f) {
            auto scalePath = aPath->makeTransform(SkMatrix::Scale(scale, scale));
            SkASSERT(scalePath.isConvex());
        }
        for (SkScalar scale = 1; scale < .001; scale /= 1.1f) {
            auto scalePath = aPath->makeTransform(SkMatrix::Scale(scale, scale));
            SkASSERT(scalePath.isConvex());
        }
    }
}

/*
 *  Try a range of crazy values, just to ensure that we don't assert/crash.
 */
DEF_TEST(HugeGeometry, reporter) {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    auto canvas = surf->getCanvas();

    const bool aas[] = { false, true };
    const SkPaint::Style styles[] = {
        SkPaint::kFill_Style, SkPaint::kStroke_Style, SkPaint::kStrokeAndFill_Style
    };
    const SkScalar values[] = {
        0, 1, 1000, 1000 * 1000, 1000.f * 1000 * 10000, SK_ScalarMax / 2, SK_ScalarMax,
        SK_ScalarInfinity
    };

    SkPaint paint;
    for (auto x : values) {
        SkRect r = { -x, -x, x, x };
        for (auto width : values) {
            paint.setStrokeWidth(width);
            for (auto aa : aas) {
                paint.setAntiAlias(aa);
                for (auto style : styles) {
                    paint.setStyle(style);
                    canvas->drawRect(r, paint);
                    canvas->drawOval(r, paint);
                }
            }
        }
    }

}

// Treat nonfinite paths as "empty" or "full", depending on inverse-filltype
DEF_TEST(ClipPath_nonfinite, reporter) {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10));
    SkCanvas* canvas = surf->getCanvas();

    REPORTER_ASSERT(reporter, !canvas->isClipEmpty());
    for (bool aa : {false, true}) {
        for (auto ft : {SkPathFillType::kWinding, SkPathFillType::kInverseWinding}) {
            for (SkScalar bad : {SK_ScalarInfinity, SK_ScalarNaN}) {
                for (int bits = 1; bits <= 15; ++bits) {
                    SkPoint p0 = { 0, 0 };
                    SkPoint p1 = { 0, 0 };
                    if (bits & 1) p0.fX = -bad;
                    if (bits & 2) p0.fY = -bad;
                    if (bits & 4) p1.fX = bad;
                    if (bits & 8) p1.fY = bad;

                    SkPath path = SkPath::Line(p0, p1).makeFillType(ft);
                    canvas->save();
                    canvas->clipPath(path, aa);
                    REPORTER_ASSERT(reporter, canvas->isClipEmpty() == !path.isInverseFillType());
                    canvas->restore();
                }
            }
        }
    }
    REPORTER_ASSERT(reporter, !canvas->isClipEmpty());
}

// skbug.com/40039046
DEF_TEST(Path_isRect, reporter) {
    auto makePath = [](const SkPoint* points, size_t count, bool close) -> SkPath {
        SkPathBuilder builder;
        for (size_t index = 0; index < count; ++index) {
            index < 2 ? builder.moveTo(points[index]) : builder.lineTo(points[index]);
        }
        if (close) {
            builder.close();
        }
        return builder.detach();
    };
    auto makePath2 = [](const SkPoint* points, const SkPath::Verb* verbs, size_t count) -> SkPath {
        SkPathBuilder builder;
        for (size_t index = 0; index < count; ++index) {
            switch (verbs[index]) {
                case SkPath::kMove_Verb:
                    builder.moveTo(*points++);
                    break;
                case SkPath::kLine_Verb:
                    builder.lineTo(*points++);
                    break;
                case SkPath::kClose_Verb:
                    builder.close();
                    break;
                default:
                    SkASSERT(0);
            }
        }
        return builder.detach();
    };
    // isolated from skbug.com/40039046 (bug description)
    SkRect rect;
    SkPoint points[] = { {10, 10}, {75, 75}, {150, 75}, {150, 150}, {75, 150} };
    SkPath path = makePath(points, std::size(points), false);
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    SkRect compare = SkRect::BoundsOrEmpty({&points[1], std::size(points) - 1});
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c3
    SkPoint points3[] = { {75, 50}, {100, 75}, {150, 75}, {150, 150}, {75, 150}, {75, 50} };
    path = makePath(points3, std::size(points3), true);
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c9
    SkPoint points9[] = { {10, 10}, {75, 75}, {150, 75}, {150, 150}, {75, 150} };
    path = makePath(points9, std::size(points9), true);
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    compare.setBounds({&points9[1], std::size(points9) - 1});
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c11
    SkPath::Verb verbs11[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb };
    SkPoint points11[] = { {75, 150}, {75, 75}, {150, 75}, {150, 150}, {75, 150}, {75, 150} };
    path = makePath2(points11, verbs11, std::size(verbs11));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    compare.setBounds(points11);
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c14
    SkPath::Verb verbs14[] = { SkPath::kMove_Verb, SkPath::kMove_Verb, SkPath::kMove_Verb,
                               SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kClose_Verb,
                               SkPath::kLine_Verb, SkPath::kClose_Verb };
    SkPoint points14[] = { {250, 75}, {250, 75}, {250, 75}, {100, 75},
                           {150, 75}, {150, 150}, {75, 150}, {75, 75}, {0, 0} };
    path = makePath2(points14, verbs14, std::size(verbs14));
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c15
    SkPath::Verb verbs15[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kMove_Verb };
    SkPoint points15[] = { {75, 75}, {150, 75}, {150, 150}, {75, 150}, {250, 75} };
    path = makePath2(points15, verbs15, std::size(verbs15));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    compare.setBounds({&points15[0], std::size(points15) - 1});
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c17
    SkPoint points17[] = { {75, 10}, {75, 75}, {150, 75}, {150, 150}, {75, 150}, {75, 10} };
    path = makePath(points17, std::size(points17), true);
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c19
    SkPath::Verb verbs19[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kClose_Verb, SkPath::kMove_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb };
    SkPoint points19[] = { {75, 75}, {75, 75}, {75, 75}, {75, 75}, {150, 75}, {150, 150},
                           {75, 150}, {10, 10}, {30, 10}, {10, 30} };
    path = makePath2(points19, verbs19, std::size(verbs19));
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c23
    SkPath::Verb verbs23[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kClose_Verb };
    SkPoint points23[] = { {75, 75}, {75, 75}, {75, 75}, {75, 75}, {150, 75}, {150, 150},
                           {75, 150} };
    path = makePath2(points23, verbs23, std::size(verbs23));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    compare.setBounds(points23);
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c29
    SkPath::Verb verbs29[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb,
                               SkPath::kClose_Verb };
    SkPoint points29[] = { {75, 75}, {150, 75}, {150, 150}, {75, 150}, {75, 250}, {75, 75} };
    path = makePath2(points29, verbs29, std::size(verbs29));
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c31
    SkPath::Verb verbs31[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb,
                               SkPath::kClose_Verb };
    SkPoint points31[] = { {75, 75}, {150, 75}, {150, 150}, {75, 150}, {75, 10}, {75, 75} };
    path = makePath2(points31, verbs31, std::size(verbs31));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
        compare.setBounds({&points31[0], 4});
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c36
    SkPath::Verb verbs36[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kMove_Verb, SkPath::kLine_Verb  };
    SkPoint points36[] = { {75, 75}, {150, 75}, {150, 150}, {10, 150}, {75, 75}, {75, 75} };
    path = makePath2(points36, verbs36, std::size(verbs36));
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from skbug.com/40039046#c39
    SkPath::Verb verbs39[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb };
    SkPoint points39[] = { {150, 75}, {150, 150}, {75, 150}, {75, 100} };
    path = makePath2(points39, verbs39, std::size(verbs39));
    REPORTER_ASSERT(reporter, !path.isRect(&rect));
    // isolated from zero_length_paths_aa
    SkPath::Verb verbsAA[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kClose_Verb };
    SkPoint pointsAA[] = { {32, 9.5f}, {32, 9.5f}, {32, 17}, {17, 17}, {17, 9.5f}, {17, 2},
                           {32, 2} };
    path = makePath2(pointsAA, verbsAA, std::size(verbsAA));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
    compare.setBounds(pointsAA);
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c41
    SkPath::Verb verbs41[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb,
                               SkPath::kClose_Verb };
    SkPoint points41[] = { {75, 75}, {150, 75}, {150, 150}, {140, 150}, {140, 75}, {75, 75} };
    path = makePath2(points41, verbs41, std::size(verbs41));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
        compare.setBounds({&points41[1], 4});
    REPORTER_ASSERT(reporter, rect == compare);
    // isolated from skbug.com/40039046#c53
    SkPath::Verb verbs53[] = { SkPath::kMove_Verb, SkPath::kLine_Verb, SkPath::kLine_Verb,
                               SkPath::kLine_Verb, SkPath::kLine_Verb, SkPath::kMove_Verb,
                               SkPath::kClose_Verb };
    SkPoint points53[] = { {75, 75}, {150, 75}, {150, 150}, {140, 150}, {140, 75}, {75, 75} };
    path = makePath2(points53, verbs53, std::size(verbs53));
    REPORTER_ASSERT(reporter, path.isRect(&rect));
        compare.setBounds({&points53[1], 4});
    REPORTER_ASSERT(reporter, rect == compare);
}

static void draw_triangle(SkCanvas* canvas, const SkPoint pts[]) {
    // draw in different ways, looking for an assert

    {
        SkPath path = SkPath::Polygon({pts, 3}, false);
        canvas->drawPath(path, SkPaint());
    }

    const SkColor colors[] = { SK_ColorBLACK, SK_ColorBLACK, SK_ColorBLACK };
    auto v = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 3, pts, nullptr, colors);
    canvas->drawVertices(v, SkBlendMode::kSrcOver, SkPaint());
}

DEF_TEST(triangle_onehalf, reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)));

    const SkPoint pts[] = {
        {  0.499069244f, 9.63295173f },
        {  0.499402374f, 7.88207579f },
        { 10.2363272f,   0.49999997f }
    };
    draw_triangle(surface->getCanvas(), pts);
}

DEF_TEST(triangle_big, reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(4, 4304)));

    // The first two points, when sent through our fixed-point SkEdge, can walk negative beyond
    // -0.5 due to accumulated += error of the slope. We have since make the bounds calculation
    // be conservative, so we invoke clipping if we get in this situation.
    // This test was added to demonstrate the need for this conservative bounds calc.
    // (found by a fuzzer)
    const SkPoint pts[] = {
        { 0.327190518f, -114.945152f },
        { -0.5f, 1.00003874f },
        { 0.666425824f, 4304.26172f },
    };
    draw_triangle(surface->getCanvas(), pts);
}

DEF_TEST(Path_increserve_handle_neg_crbug_883666, r) {
    SkPathBuilder builder;

    builder.conicTo({0, 0}, {1, 1}, SK_FloatNegativeInfinity);

    // <== use a copy path object to force SkPathRef::copy() and SkPathRef::resetToSize()
    SkPathBuilder shallowBuilder = builder;

    // make sure we don't assert/crash on this.
    shallowBuilder.incReserve(0xffffffff);
}

////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *  For speed, we tried to preserve useful/expensive attributes about paths,
 *      - convexity, isrect, isoval, ...
 *  Axis-aligned shapes (rect, oval, rrect) should survive, including convexity if the matrix
 *  is axis-aligned (e.g. scale+translate)
 */

struct Xforms {
    SkMatrix    fIM, fTM, fSM, fRM;

    Xforms() {
        fIM.reset();
        fTM.setTranslate(10, 20);
        fSM.setScale(2, 3);
        fRM.setRotate(30);
    }
};

static bool nocompute_isconvex(const SkPath& path) {
    SkPathConvexity convexity = SkPathPriv::GetConvexityOrUnknown(path);
    return SkPathConvexity_IsConvex(convexity);
}

// expect axis-aligned shape to survive assignment, identity and scale/translate matrices
template <typename ISA>
void survive(SkPath* path, const Xforms& x, bool isAxisAligned, skiatest::Reporter* reporter,
             ISA isa_proc) {
    REPORTER_ASSERT(reporter, isa_proc(*path));
    // force the issue (computing convexity) the first time.
    REPORTER_ASSERT(reporter, path->isConvex());

    // a path's isa and convexity should survive assignment
    {
        SkPath path2 = *path;
        REPORTER_ASSERT(reporter, isa_proc(path2));
        REPORTER_ASSERT(reporter, nocompute_isconvex(path2));
    }

    // a path's isa and convexity should identity transform
    *path = path->makeTransform(x.fIM);
    REPORTER_ASSERT(reporter, isa_proc(*path));
    REPORTER_ASSERT(reporter, nocompute_isconvex(*path));

    // a path's isa should survive translation, convexity depends on axis alignment
    *path = path->makeTransform(x.fTM);
    REPORTER_ASSERT(reporter, isa_proc(*path));
    REPORTER_ASSERT(reporter, nocompute_isconvex(*path) == isAxisAligned);

    // a path's isa should survive scaling, convexity depends on axis alignment
    *path = path->makeTransform(x.fSM);
    REPORTER_ASSERT(reporter, isa_proc(*path));
    REPORTER_ASSERT(reporter, nocompute_isconvex(*path) == isAxisAligned);

    // For security, post-rotation, we can't assume we're still convex. It might prove to be,
    // in fact, still be convex, be we can't have cached that setting, hence the call to
    // getConvexityOrUnknown() instead of getConvexity().
    *path = path->makeTransform(x.fRM);
    REPORTER_ASSERT(reporter, !nocompute_isconvex(*path));

    if (isAxisAligned) {
        REPORTER_ASSERT(reporter, !isa_proc(*path));
    }
}

DEF_TEST(Path_survive_transform, r) {
    const Xforms x;

    SkPath path = SkPath::Rect({10, 10, 40, 50});
    survive(&path, x, true, r, [](const SkPath& p) { return p.isRect(nullptr); });

    path = SkPath::Oval({10, 10, 40, 50});
    survive(&path, x, true, r, [](const SkPath& p) { return p.isOval(nullptr); });

    path = SkPath::RRect(SkRRect::MakeRectXY({10, 10, 40, 50}, 5, 5));
    survive(&path, x, true, r, [](const SkPath& p) { return p.isRRect(nullptr); });

    // make a trapazoid; definitely convex, but not marked as axis-aligned (e.g. oval, rrect)
    path = SkPathBuilder()
           .moveTo(0, 0).lineTo(100, 0).lineTo(70, 100).lineTo(30, 100)
           .detach();
    REPORTER_ASSERT(r, path.isConvex());
    survive(&path, x, false, r, [](const SkPath& p) { return true; });
}

static void test_edger(skiatest::Reporter* r,
                       const std::initializer_list<SkPathVerb>& in,
                       const std::initializer_list<SkPathVerb>& expected) {
    SkPathBuilder builder;
    SkScalar x = 0, y = 0;
    for (auto v : in) {
        switch (v) {
            case SkPathVerb::kMove: builder.moveTo(x++, y++); break;
            case SkPathVerb::kLine: builder.lineTo(x++, y++); break;
            case SkPathVerb::kClose: builder.close(); break;
            default: SkASSERT(false);
        }
    }
    SkPath path = builder.detach();

    SkPathEdgeIter iter(path);
    for (auto v : expected) {
        auto e = iter.next();
        REPORTER_ASSERT(r, e);
        REPORTER_ASSERT(r, SkPathEdgeIter::EdgeToVerb(e.fEdge) == v);
    }
    REPORTER_ASSERT(r, !iter.next());
}

template <typename T> bool span_eq(SkSpan<const T> a, SkSpan<const T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin());
}

static void assert_points(skiatest::Reporter* reporter,
                          const SkPath& path, const std::initializer_list<SkPoint>& list) {
    auto praw = SkPathPriv::Raw(path, SkResolveConvexity::kNo);
    REPORTER_ASSERT(reporter, praw.has_value());
    REPORTER_ASSERT(reporter, span_eq<SkPoint>(praw->fPoints, list));
}

static void test_addRect_and_trailing_lineTo(skiatest::Reporter* reporter) {
    SkPath path;
    const SkRect r = {1, 2, 3, 4};
    // build our default p-array clockwise
    const SkPoint p[] = {
        {r.fLeft,  r.fTop},    {r.fRight, r.fTop},
        {r.fRight, r.fBottom}, {r.fLeft,  r.fBottom},
    };

    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        int increment = dir == SkPathDirection::kCW ? 1 : 3;
        for (int i = 0; i < 4; ++i) {
            SkPathBuilder builder;
            builder.addRect(r, dir, i);
            path = builder.snapshot();

            // check that we return the 4 ponts in the expected order
            SkPoint e[4];
            for (int j = 0; j < 4; ++j) {
                int index = (i + j*increment) % 4;
                e[j] = p[index];
            }
            assert_points(reporter, path, {
                e[0], e[1], e[2], e[3]
            });

            // check that the new line begins where the rect began
            builder.lineTo(7,8);
            path = builder.snapshot();
            assert_points(reporter, path, {
                e[0], e[1], e[2], e[3],
                e[0], {7,8},
            });
        }
    }

    // now add a moveTo before the rect, just to be sure we don't always look at
    // the "first" point in the path when we handle the trailing lineTo
    path = SkPathBuilder()
           .moveTo(7, 8)  // will be replaced by rect's first moveTo
           .addRect(r, SkPathDirection::kCW, 2)
           .lineTo(5, 6)
           .detach();

    assert_points(reporter, path, {
        p[2], p[3], p[0], p[1], // rect
        p[2], {5, 6},           // trailing line
    });
}

DEF_TEST(pathedger, r) {
    auto M = SkPathVerb::kMove;
    auto L = SkPathVerb::kLine;
    auto C = SkPathVerb::kClose;

    test_edger(r, { M }, {});
    test_edger(r, { M, M }, {});
    test_edger(r, { M, C }, {});
    test_edger(r, { M, M, C }, {});
    test_edger(r, { M, L }, { L, L });
    test_edger(r, { M, L, C }, { L, L });
    test_edger(r, { M, L, L }, { L, L, L });
    test_edger(r, { M, L, L, C }, { L, L, L });

    test_edger(r, { M, L, L, M, L, L }, { L, L, L,   L, L, L });

    test_addRect_and_trailing_lineTo(r);
}

DEF_TEST(path_convexity_scale_way_down, r) {
    SkPath path = SkPathBuilder().moveTo(0,0).lineTo(1, 0)
                                 .lineTo(1,1).lineTo(0,1)
                                 .detach();

    REPORTER_ASSERT(r, path.isConvex());
    const SkScalar scale = 1e-8f;
    SkPath path2 = path.makeTransform(SkMatrix::Scale(scale, scale));
    SkPathPriv::ForceComputeConvexity(path2);
    REPORTER_ASSERT(r, path2.isConvex());
}

// crbug.com/1187385
DEF_TEST(path_moveto_addrect, r) {
    // Test both an empty and non-empty rect passed to SkPath::addRect
    SkRect rects[] = {{207.0f, 237.0f, 300.0f, 237.0f},
                      {207.0f, 237.0f, 300.0f, 267.0f}};

    for (SkRect rect: rects) {
        for (int numExtraMoveTos : {0, 1, 2, 3}) {
            SkPathBuilder builder;
            // Convexity and contains functions treat the path as a simple fill, so consecutive
            // moveTos are collapsed together.
            for (int i = 0; i < numExtraMoveTos; ++i) {
                builder.moveTo(i, i);
            }
            builder.addRect(rect); // this will replace the prev moveTos
            SkPath path = builder.detach();

            // addRect should mark the path as known convex automatically (i.e. it wasn't set
            // to unknown after edits)
            REPORTER_ASSERT(r, nocompute_isconvex(path));

            // but it should also agree with the regular convexity computation
            SkPathPriv::ForceComputeConvexity(path);
            REPORTER_ASSERT(r, path.isConvex());

            const SkRect query = rect.makeInset(10.f, 0.f);
            const bool contains = path.conservativelyContainsRect(query);
            // if the rect we used to build the path was empty, then "containing" something
            // else is poorly defined -- so we only assert we contain the query if we're
            // non-empty (i.e. path with some area)
            REPORTER_ASSERT(r, rect.isEmpty() || contains);
        }
    }
}

// crbug.com/1220754
DEF_TEST(path_moveto_twopass_convexity, r) {
    // There had been a bug when the last moveTo index > 0, the calculated point count was incorrect
    // and the BySign convexity pass would not evaluate the entire path, effectively only using the
    // winding rule for determining convexity.
    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(3.25f, 115.5f)
                  .conicTo(9.98099e+17f, 2.83874e+15f, 1.75098e-30f, 1.75097e-30f, 1.05385e+18f)
                  .conicTo(9.96938e+17f, 6.3804e+19f, 9.96934e+17f, 1.75096e-30f, 1.75096e-30f)
                  .quadTo(1.28886e+10f, 9.9647e+17f, 9.98101e+17f, 2.61006e+15f)
                  .detach();
    REPORTER_ASSERT(r, !path.isConvex());

    SkPath pathWithExtraMoveTo = SkPathBuilder(SkPathFillType::kWinding)
                                 .moveTo(5.90043e-39f, 1.34525e-43f)
                                 .addPath(path)
                                 .detach();
    REPORTER_ASSERT(r, !pathWithExtraMoveTo.isConvex());
}

// crbug.com/1154864
DEF_TEST(path_walk_simple_edges_1154864, r) {
    // Drawing this path triggered an assert in walk_simple_edges:
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(32, 32));

    SkPath path = SkPathBuilder(SkPathFillType::kWinding)
                  .moveTo(0.00665998459f, 2)
                  .quadTo(0.00665998459f, 4, -1.99334002f, 4)
                  .quadTo(-3.99334002f, 4, -3.99334002f, 2)
                  .quadTo(-3.99334002f, 0, -1.99334002f, 0)
                  .quadTo(0.00665998459f, 0, 0.00665998459f, 2)
                  .close()
                  .detach();

    SkPaint paint;
    paint.setAntiAlias(true);
    surface->getCanvas()->drawPath(path, paint);
}

// crbug.com/398075927
DEF_TEST(path_walk_edges_concave_large_dx, r) {
    // The large surface size is necessary to reproduce the bug because we need
    // changes in y to be large enough but then also changes in x need to be much greater
    // while also ensuring we are blitting the interesting edge. Also the larger numbers
    // more easily capture the numerical instability with the algorithm.
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(900, 700));

    SkPath path = SkPathBuilder()
                  .lineTo(100, 400)
                  .lineTo(90, 600)
                  .quadTo(35000, 200, 35000, 200)
                  .detach();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    surface->getCanvas()->drawPath(path, paint);
}

DEF_TEST(path_filltype_utils, r) {
    SkPath p1 = SkPathBuilder()
                .lineTo(42, 42)
                .lineTo(42, 0)
                .close()
                .detach();

    REPORTER_ASSERT(r, p1.getFillType() == SkPathFillType::kWinding);

    const SkPath p2 = p1.makeFillType(SkPathFillType::kEvenOdd);
    REPORTER_ASSERT(r, p2 != p1);
    REPORTER_ASSERT(r, p2.getFillType() == SkPathFillType::kEvenOdd);

    const SkPath p3 = p2.makeToggleInverseFillType();
    REPORTER_ASSERT(r, p3 != p2);
    REPORTER_ASSERT(r, p3.getFillType() == SkPathFillType::kInverseEvenOdd);
}

// To test tight bounds, we ...
// 1. build some random paths that contains curves (that is the tricky part of tight bounds)
// 2. compute an approximation of "tight" bounds by evaluating the curves many times
// 3. ask path/builder/pathdata to compute the tight bounds, and then compare
//
static SkPathBuilder make_random_builder(SkRandom& rand) {
    auto rpoint = [&]() -> SkPoint {
        float x = rand.nextF() * 100,
              y = rand.nextF() * 100;
        return {x, y};
    };

    constexpr size_t N = 9;
    SkPoint pts[N];
    for (SkPoint& p : pts) {
        p = rpoint();
    }

    SkPathBuilder bu;
    bu.moveTo(pts[0]);
    bu.lineTo(pts[1]);
    bu.quadTo(pts[2], pts[3]);
    bu.conicTo(pts[4], pts[5], rand.nextF() * 2);
    bu.cubicTo(pts[6], pts[7], pts[8]);
    return bu;
}

static void update_bounds(SkRect* r, SkPoint p) {
    r->fLeft   = std::fminf(r->fLeft,   p.fX);
    r->fTop    = std::fminf(r->fTop,    p.fY);
    r->fRight  = std::fmaxf(r->fRight,  p.fX);
    r->fBottom = std::fmaxf(r->fBottom, p.fY);
}

static void update_bounds_line(SkRect* r, SkSpan<const SkPoint> pts) {
    update_bounds(r, pts[1]);
}

constexpr size_t kEvalLoopCount = 1000;

static void update_bounds_quad(SkRect* r, SkSpan<const SkPoint> pts) {
    for (size_t i = 1; i < kEvalLoopCount; ++i) {
        const float t = (float)i / kEvalLoopCount;
        update_bounds(r, SkEvalQuadAt(pts.data(), t));
    }
    update_bounds(r, pts[2]);
}

static void update_bounds_conic(SkRect* r, SkSpan<const SkPoint> pts, float w) {
    SkConic conic(pts.data(), w);
    for (size_t i = 1; i < kEvalLoopCount; ++i) {
        const float t = (float)i / kEvalLoopCount;
        update_bounds(r, conic.evalAt(t));
    }
    update_bounds(r, pts[2]);
}

static void update_bounds_cubic(SkRect* r, SkSpan<const SkPoint> pts) {
    for (size_t i = 1; i < kEvalLoopCount; ++i) {
        const float t = (float)i / kEvalLoopCount;
        SkPoint point;
        SkEvalCubicAt(pts.data(), t, &point, nullptr, nullptr);
        update_bounds(r, point);
    }
    update_bounds(r, pts[3]);
}

static SkRect compute_tight_bounds(SkPathIter iter) {
    SkRect r;
    bool first = true;
    while (auto rec = iter.next()) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                if (first) {
                    r = SkRect::Bounds(rec->fPoints).value();
                    first = false;
                }
                break;
            case SkPathVerb::kLine:
                update_bounds_line(&r, rec->fPoints);
                break;
            case SkPathVerb::kQuad:
                update_bounds_quad(&r, rec->fPoints);
                break;
            case SkPathVerb::kConic:
                update_bounds_conic(&r, rec->fPoints, rec->conicWeight());
                break;
            case SkPathVerb::kCubic:
                update_bounds_cubic(&r, rec->fPoints);
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
    return r;
}

DEF_TEST(path_computeTightBounds, reporter) {
    SkRandom rand;

    for (int i = 0; i < 100; ++i) {
        SkPathBuilder bu = make_random_builder(rand);
        const SkRect tb = compute_tight_bounds(bu.iter());

        auto nearly_eq = [reporter](const SkRect& a, const SkRect& b) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(a.fLeft,   b.fLeft));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(a.fTop,    b.fTop));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(a.fRight,  b.fRight));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(a.fBottom, b.fBottom));
        };

        SkPath path = bu.snapshot();
        auto pdata = SkPathData::Make(bu.points(), bu.verbs(), bu.conicWeights());

        const SkRect r0 = bu.computeTightBounds().value_or(SkRect::MakeEmpty());
        const SkRect r1 = path.computeTightBounds();
        const SkRect r2 = pdata->computeTightBounds();

        REPORTER_ASSERT(reporter, r0 == r1);
        REPORTER_ASSERT(reporter, r0 == r2);

        nearly_eq(r0, tb);  // check against our approximation
    }
}

DEF_TEST(path_trivial_isrect, reporter) {
    static constexpr struct {
        std::array<SkPoint, 4> pts;
        bool                   isrect;

        SkRect          expectedRect  = SkRect::MakeEmpty();
        SkPathDirection expectedDir = SkPathDirection::kDefault;
    } gTests[] = {
        {{{{ 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}}}, false},
        {{{{10, 10}, {10, 10}, {10, 10}, {10, 10}}}, false},
        {{{{10, 10}, {20, 10}, {20, 10}, {10, 10}}}, false},
        {{{{10, 10}, {10, 30}, {10, 30}, {10, 10}}}, false},
        {{{{10, 10}, {20, 30}, {20, 30}, {10, 10}}}, false},
        {{{{10, 10}, {20, 10}, {20, 30}, {10, 20}}}, false},

        {{{{10, 10}, {20, 10}, {20, 30}, {10, 30}}}, true, {10, 10, 20, 30}, SkPathDirection::kCW },
        {{{{10, 10}, {10, 30}, {20, 30}, {20, 10}}}, true, {10, 10, 20, 30}, SkPathDirection::kCCW},
    };

    for (const auto& tst : gTests) {
        for (size_t i = 0; i < 4; ++i) {
            const auto path = SkPathBuilder()
                .moveTo(tst.pts[i])
                .lineTo(tst.pts[(i + 1) % 4])
                .lineTo(tst.pts[(i + 2) % 4])
                .lineTo(tst.pts[(i + 3) % 4])
                .close()
                .detach();

            SkRect rect;
            bool closed;
            SkPathDirection dir;

            REPORTER_ASSERT(reporter, path.isRect(&rect, &closed, &dir) == tst.isrect);
            if (!tst.isrect) {
                continue;
            }

            REPORTER_ASSERT(reporter, rect == tst.expectedRect);
            REPORTER_ASSERT(reporter, closed);
            REPORTER_ASSERT(reporter, dir == tst.expectedDir);
        }
    }
}

DEF_TEST(path_infinite_transform, reporter) {
    constexpr float coord = 1000;

    SkPath path = SkPath::Circle(0, 0, coord);
    REPORTER_ASSERT(reporter, path.isFinite());
    REPORTER_ASSERT(reporter, (path.getBounds() == SkRect{-coord, -coord, coord, coord}));

    const float scales[] = { 1, 1e12f, 1e24f, 1e36f };
    SkASSERT(!SkIsFinite(scales[3] * coord));   // make sure the last one overflows

    auto check_finite = [reporter](const SkPath& p) {
        const SkRect r = p.getBounds();
        REPORTER_ASSERT(reporter, p.isFinite());
        REPORTER_ASSERT(reporter, r.isFinite());
        REPORTER_ASSERT(reporter, !r.isEmpty());
    };

    auto check_infinite = [reporter](const SkPath& p) {
        const SkRect r = p.getBounds();
        REPORTER_ASSERT(reporter, !p.isFinite());
        REPORTER_ASSERT(reporter, r.isEmpty());
    };

    for (auto scale : scales) {
        const SkMatrix mx = SkMatrix::Scale(scale, scale);

        if (SkIsFinite(scale * coord)) {
            auto maybe = path.tryMakeTransform(mx);
            REPORTER_ASSERT(reporter, maybe.has_value());
            check_finite(maybe.value());

            auto newpath = path.makeTransform(mx);
            check_finite(newpath);

            REPORTER_ASSERT(reporter, newpath == maybe.value());
        } else {
            auto maybe = path.tryMakeTransform(mx);
            REPORTER_ASSERT(reporter, !maybe.has_value());

            auto newpath = path.makeTransform(mx);
            check_infinite(newpath);
        }
    }
}

DEF_TEST(path_factory_inverted_bounds, reporter) {
    constexpr SkRect bounds = {-10, -10, 10, 10};
    constexpr SkRect inverted_bounds = {10, 10, -10, -10};

    REPORTER_ASSERT(reporter, SkPath::Oval(inverted_bounds).getBounds() == bounds);
    REPORTER_ASSERT(reporter, SkPath::Rect(inverted_bounds).getBounds() == bounds);
    REPORTER_ASSERT(reporter,
                    SkPath::RRect(SkRRect::MakeRect(inverted_bounds)).getBounds() == bounds);
}

DEF_TEST(path_trailing_moves_bounds, reporter) {
    const SkPoint pt = {3, 4};
    const SkPathVerb vb = SkPathVerb::kMove;

    // A single move is reflected in the bounds

    SkPathBuilder bu;
    bu.moveTo(pt);
    auto r = bu.computeBounds();
    SkRect r2 = {pt.fX, pt.fY, pt.fX, pt.fY};
    REPORTER_ASSERT(reporter, r == r2);

    auto path = bu.detach();
    REPORTER_ASSERT(reporter, path.points().size() == 1);
    REPORTER_ASSERT(reporter, path.points().back() == pt);
    r = path.getBounds();
    REPORTER_ASSERT(reporter, r == r2);

    path = SkPath::Raw({&pt, 1}, {&vb, 1}, {}, SkPathFillType::kDefault);
    REPORTER_ASSERT(reporter, path.points().size() == 1);
    REPORTER_ASSERT(reporter, path.points().back() == pt);
    r = path.getBounds();
    REPORTER_ASSERT(reporter, r == r2);

    // A trailing move, but not the only contour, does not affect the bounds

    bu.moveTo(1, 2);
    bu.lineTo(3, 4);    // same as pt
    SkPoint lastPt = {5, 6};
    bu.moveTo(lastPt);
    r2 = {1, 2, 3, 4};  // does not include last point
    r = bu.computeBounds();
    REPORTER_ASSERT(reporter, r == r2);

    path = bu.detach();
    REPORTER_ASSERT(reporter, path.points().size() == 3);
    REPORTER_ASSERT(reporter, path.points().back() == lastPt);
    r = path.getBounds();
    REPORTER_ASSERT(reporter, r == r2);

    SkPoint pts2[] = {{1, 2}, {3, 4}, {5, 6}};
    SkPathVerb vbs2[] = {SkPathVerb::kMove, SkPathVerb::kLine, SkPathVerb::kMove};
    path = SkPath::Raw(pts2, vbs2, {}, SkPathFillType::kDefault);
    REPORTER_ASSERT(reporter, path.points().size() == 3);
    REPORTER_ASSERT(reporter, path.points().back() == lastPt);
    r = path.getBounds();
    REPORTER_ASSERT(reporter, r == r2);

    // make sure the trailer survives a round-trip

    auto check_trip = [reporter](const SkPath& path) {
        const SkRect pathBounds = path.getBounds();

        SkPathBuilder bu;
        bu.addPath(path);
        REPORTER_ASSERT(reporter, bu.verbs().size() == path.verbs().size());
        REPORTER_ASSERT(reporter, bu.verbs().back() == SkPathVerb::kMove);
        SkRect r = bu.computeBounds();
        REPORTER_ASSERT(reporter, r == pathBounds);

        const SkMatrix mx = SkMatrix::Translate(1, 2);
        bu.reset().addPath(path, mx);
        REPORTER_ASSERT(reporter, bu.verbs().size() == path.verbs().size());
        REPORTER_ASSERT(reporter, bu.verbs().back() == SkPathVerb::kMove);
        r = bu.computeBounds();
        REPORTER_ASSERT(reporter, r == pathBounds.makeOffset(1, 2));
    };

    check_trip(bu.reset().moveTo(1, 2).detach());
    check_trip(bu.reset().moveTo(1, 2).lineTo(3, 4).moveTo(5, 6).detach());
}

// https://issues.oss-fuzz.com/issues/470703863
DEF_TEST(path_read_from_memory_corrupt_rrect, reporter) {
    constexpr size_t kNumBytes = 4;
    constexpr uint8_t data[kNumBytes] = {4, 43, 61, 16};

    size_t bytesRead = 117;  // Sentinel value
    auto result = SkPath::ReadFromMemory(&data, kNumBytes, &bytesRead);
    REPORTER_ASSERT(reporter, !result.has_value());
    REPORTER_ASSERT(reporter, bytesRead == 0);
}
