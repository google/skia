/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMatrixPriv.h"
#include "tests/Test.h"

static bool eq(const SkM44& a, const SkM44& b, float tol) {
    float fa[16], fb[16];
    a.getColMajor(fa);
    b.getColMajor(fb);
    for (int i = 0; i < 16; ++i) {
        if (!SkScalarNearlyEqual(fa[i], fb[i], tol)) {
            return false;
        }
    }
    return true;
}

DEF_TEST(M44, reporter) {
    SkM44 m, im;

    REPORTER_ASSERT(reporter, SkM44(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1) == m);
    REPORTER_ASSERT(reporter, SkM44() == m);
    REPORTER_ASSERT(reporter, m.invert(&im));
    REPORTER_ASSERT(reporter, SkM44() == im);

    m.setTranslate(3, 4, 2);
    REPORTER_ASSERT(reporter, SkM44(1, 0, 0, 3,
                                    0, 1, 0, 4,
                                    0, 0, 1, 2,
                                    0, 0, 0, 1) == m);

    const float f[] = { 1, 0, 0, 2, 3, 1, 2, 5, 0, 5, 3, 0, 0, 1, 0, 2 };
    m = SkM44::ColMajor(f);
    REPORTER_ASSERT(reporter, SkM44(f[0], f[4], f[ 8], f[12],
                                    f[1], f[5], f[ 9], f[13],
                                    f[2], f[6], f[10], f[14],
                                    f[3], f[7], f[11], f[15]) == m);

    {
        SkM44 t = m.transpose();
        REPORTER_ASSERT(reporter, t != m);
        REPORTER_ASSERT(reporter, t.rc(1,0) == m.rc(0,1));
        SkM44 tt = t.transpose();
        REPORTER_ASSERT(reporter, tt == m);
    }

    m = SkM44::RowMajor(f);
    REPORTER_ASSERT(reporter, SkM44(f[ 0], f[ 1], f[ 2], f[ 3],
                                    f[ 4], f[ 5], f[ 6], f[ 7],
                                    f[ 8], f[ 9], f[10], f[14],
                                    f[12], f[13], f[14], f[15]) == m);

    REPORTER_ASSERT(reporter, m.invert(&im));

    m = m * im;
    // m should be identity now, but our calc is not perfect...
    REPORTER_ASSERT(reporter, eq(SkM44(), m, 0.0000005f));
    REPORTER_ASSERT(reporter, SkM44() != m);
}

DEF_TEST(M44_v3, reporter) {
    SkV3 a = {1, 2, 3},
         b = {1, 2, 2};

    REPORTER_ASSERT(reporter, a.lengthSquared() == 1 + 4 + 9);
    REPORTER_ASSERT(reporter, b.length() == 3);
    REPORTER_ASSERT(reporter, a.dot(b) == 1 + 4 + 6);
    REPORTER_ASSERT(reporter, b.dot(a) == 1 + 4 + 6);
    REPORTER_ASSERT(reporter, (a.cross(b) == SkV3{-2,  1, 0}));
    REPORTER_ASSERT(reporter, (b.cross(a) == SkV3{ 2, -1, 0}));

    SkM44 m = {
        2, 0, 0, 3,
        0, 1, 0, 5,
        0, 0, 3, 1,
        0, 0, 0, 1
    };

    SkV3 c = m * a;
    REPORTER_ASSERT(reporter, (c == SkV3{2, 2, 9}));
    SkV4 d = m.map(4, 3, 2, 1);
    REPORTER_ASSERT(reporter, (d == SkV4{11, 8, 7, 1}));
}

DEF_TEST(M44_v4, reporter) {
    SkM44 m( 1,  2,  3,  4,
             5,  6,  7,  8,
             9, 10, 11, 12,
            13, 14, 15, 16);

    SkV4 r0 = m.row(0),
         r1 = m.row(1),
         r2 = m.row(2),
         r3 = m.row(3);

    REPORTER_ASSERT(reporter, (r0 == SkV4{ 1,  2,  3,  4}));
    REPORTER_ASSERT(reporter, (r1 == SkV4{ 5,  6,  7,  8}));
    REPORTER_ASSERT(reporter, (r2 == SkV4{ 9, 10, 11, 12}));
    REPORTER_ASSERT(reporter, (r3 == SkV4{13, 14, 15, 16}));

    REPORTER_ASSERT(reporter, SkM44::Rows(r0, r1, r2, r3) == m);

    SkV4 c0 = m.col(0),
         c1 = m.col(1),
         c2 = m.col(2),
         c3 = m.col(3);

    REPORTER_ASSERT(reporter, (c0 == SkV4{1, 5,  9, 13}));
    REPORTER_ASSERT(reporter, (c1 == SkV4{2, 6, 10, 14}));
    REPORTER_ASSERT(reporter, (c2 == SkV4{3, 7, 11, 15}));
    REPORTER_ASSERT(reporter, (c3 == SkV4{4, 8, 12, 16}));

    REPORTER_ASSERT(reporter, SkM44::Cols(c0, c1, c2, c3) == m);

    // implement matrix * vector using column vectors
    SkV4 v = {1, 2, 3, 4};
    SkV4 v1 = m * v;
    SkV4 v2 = c0 * v.x + c1 * v.y + c2 * v.z + c3 * v.w;
    REPORTER_ASSERT(reporter, v1 == v2);

    REPORTER_ASSERT(reporter, (c0 + r0 == SkV4{c0.x+r0.x, c0.y+r0.y, c0.z+r0.z, c0.w+r0.w}));
    REPORTER_ASSERT(reporter, (c0 - r0 == SkV4{c0.x-r0.x, c0.y-r0.y, c0.z-r0.z, c0.w-r0.w}));
    REPORTER_ASSERT(reporter, (c0 * r0 == SkV4{c0.x*r0.x, c0.y*r0.y, c0.z*r0.z, c0.w*r0.w}));
}

DEF_TEST(M44_rotate, reporter) {
    const SkV3 x = {1, 0, 0},
               y = {0, 1, 0},
               z = {0, 0, 1};

    // We have radians version of setRotateAbout methods, but even with our best approx
    // for PI, sin(SK_ScalarPI) != 0, so to make the comparisons in the unittest clear,
    // I'm using the variants that explicitly take the sin,cos values.

    struct {
        SkScalar sinAngle, cosAngle;
        SkV3 aboutAxis;
        SkV3 expectedX, expectedY, expectedZ;
    } recs[] = {
        { 0, 1,    x,   x, y, z},    // angle = 0
        { 0, 1,    y,   x, y, z},    // angle = 0
        { 0, 1,    z,   x, y, z},    // angle = 0

        { 0,-1,    x,   x,-y,-z},    // angle = 180
        { 0,-1,    y,  -x, y,-z},    // angle = 180
        { 0,-1,    z,  -x,-y, z},    // angle = 180

        // Skia coordinate system is right-handed

        { 1, 0,    x,   x, z,-y},    // angle = 90
        { 1, 0,    y,  -z, y, x},    // angle = 90
        { 1, 0,    z,   y,-x, z},    // angle = 90

        {-1, 0,    x,   x,-z, y},    // angle = -90
        {-1, 0,    y,   z, y,-x},    // angle = -90
        {-1, 0,    z,  -y, x, z},    // angle = -90
    };

    for (const auto& r : recs) {
        SkM44 m(SkM44::kNaN_Constructor);
        m.setRotateUnitSinCos(r.aboutAxis, r.sinAngle, r.cosAngle);

        auto mx = m * x;
        auto my = m * y;
        auto mz = m * z;
        REPORTER_ASSERT(reporter, mx == r.expectedX);
        REPORTER_ASSERT(reporter, my == r.expectedY);
        REPORTER_ASSERT(reporter, mz == r.expectedZ);

        // flipping the axis-of-rotation should flip the results
        mx = m * -x;
        my = m * -y;
        mz = m * -z;
        REPORTER_ASSERT(reporter, mx == -r.expectedX);
        REPORTER_ASSERT(reporter, my == -r.expectedY);
        REPORTER_ASSERT(reporter, mz == -r.expectedZ);
    }
}

DEF_TEST(M44_rectToRect, reporter) {
    enum EdgeMapping {
        kExactEdges,  // src edges map to dst edges exactly
        kInsideEdges, // src edges map to within dst
        kOutsideEdges, // src edges map to outside dst
    };
    using Factory = decltype(&SkM44::RectToRect);
    struct {
        Factory     factory;
        SkV2        dstScale;
        EdgeMapping xMapping;
        EdgeMapping yMapping;
    } recs[] = {
        // no aspect ratio change, nor up/down scaling
        {SkM44::RectToRect,      {1.f, 1.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {1.f, 1.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToCover, {1.f, 1.f},    kExactEdges,   kExactEdges},
        // aspect ratio narrows, downscale x and y
        {SkM44::RectToRect,      {0.25f, 0.5f}, kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {0.25f, 0.5f}, kExactEdges,   kInsideEdges},
        {SkM44::IsoScaleToCover, {0.25f, 0.5f}, kOutsideEdges, kExactEdges},
        // aspect ratio widens, downscale x and y
        {SkM44::RectToRect,      {0.5f, 0.25f}, kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {0.5f, 0.25f}, kInsideEdges,  kExactEdges},
        {SkM44::IsoScaleToCover, {0.5f, 0.25f}, kExactEdges,   kOutsideEdges},
        // no aspect ratio change, downscale x and y
        {SkM44::RectToRect,      {0.5f, 0.5f},  kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {0.5f, 0.5f},  kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToCover, {0.5f, 0.5f},  kExactEdges,   kExactEdges},
        // aspect ratio narrows, upscale x and y
        {SkM44::RectToRect,      {2.f, 3.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {2.f, 3.f},    kExactEdges,   kInsideEdges},
        {SkM44::IsoScaleToCover, {2.f, 3.f},    kOutsideEdges, kExactEdges},
        // aspect ratio widens, upscale x and y
        {SkM44::RectToRect,      {3.f, 2.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {3.f, 2.f},    kInsideEdges,  kExactEdges},
        {SkM44::IsoScaleToCover, {3.f, 2.f},    kExactEdges,   kOutsideEdges},
        // no aspect ratio change, upscale x and y
        {SkM44::RectToRect,      {2.f, 2.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {2.f, 2.f},    kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToCover, {2.f, 2.f},    kExactEdges,   kExactEdges},
        // aspect ratio narrows, downscale x and upscale y
        {SkM44::RectToRect,      {0.5f, 2.f},   kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {0.5f, 2.f},   kExactEdges,   kInsideEdges},
        {SkM44::IsoScaleToCover, {0.5f, 2.f},   kOutsideEdges, kExactEdges},
        // aspect ratio widens, upscale x and downscale y
        {SkM44::RectToRect,      {2.f, 0.5f},   kExactEdges,   kExactEdges},
        {SkM44::IsoScaleToFit,   {2.f, 0.5f},   kInsideEdges,  kExactEdges},
        {SkM44::IsoScaleToCover, {2.f, 0.5f},   kExactEdges,   kOutsideEdges},
    };

    auto map2d = [&](const SkM44& m, SkV2 p) {
        SkV4 mapped = m.map(p.x, p.y, 0.f, 1.f);
        REPORTER_ASSERT(reporter, mapped.z == 0.f);
        REPORTER_ASSERT(reporter, mapped.w == 1.f);
        return SkV2{mapped.x, mapped.y};
    };
    auto assertNearlyEqual = [&](float actual, float expected) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual, expected),
                        "Expected %g == %g", actual, expected);
    };
    auto assertEdges = [&](float actualLow, float actualHigh, float expectedLow, float expectedHigh,
                           EdgeMapping mapping) {
        SkASSERT(expectedLow < expectedHigh);
        REPORTER_ASSERT(reporter, actualLow < actualHigh,
                        "Expected %g < %g", actualLow, actualHigh);

        switch(mapping) {
            case kExactEdges:
                assertNearlyEqual(actualLow, expectedLow);
                assertNearlyEqual(actualHigh, expectedHigh);
                break;
            case kInsideEdges:
                REPORTER_ASSERT(reporter, actualLow > expectedLow,
                                "Expected %g > %g", actualLow, expectedLow);
                REPORTER_ASSERT(reporter, actualHigh < expectedHigh,
                                "Expected %g < %g", actualHigh, expectedHigh);
                break;
            case kOutsideEdges:
                REPORTER_ASSERT(reporter, actualLow < expectedLow,
                                "Expected %g < %g", actualLow, expectedLow);
                REPORTER_ASSERT(reporter, actualHigh > expectedHigh,
                                "Expected %g > %g", actualHigh, expectedHigh);
                break;
        }
    };

    SkRandom rand;
    for (const auto& r : recs) {
        SkRect src = SkRect::MakeXYWH(rand.nextRangeF(-10.f, 10.f),
                                      rand.nextRangeF(-10.f, 10.f),
                                      rand.nextRangeF(1.f, 10.f),
                                      rand.nextRangeF(1.f, 10.f));
        SkRect dst = SkRect::MakeXYWH(rand.nextRangeF(-10.f, 10.f),
                                      rand.nextRangeF(-10.f, 10.f),
                                      r.dstScale.x * src.width(),
                                      r.dstScale.y * src.height());

        SkM44 m = r.factory(src, dst);

        // Regardless of the factory, center of src maps to center of dst
        SkV2 center = map2d(m, {src.centerX(), src.centerY()});
        assertNearlyEqual(center.x, dst.centerX());
        assertNearlyEqual(center.y, dst.centerY());

        // Map the four corners of src and validate against expected edge mapping
        SkV2 tl = map2d(m, {src.fLeft, src.fTop});
        SkV2 tr = map2d(m, {src.fRight, src.fTop});
        SkV2 br = map2d(m, {src.fRight, src.fBottom});
        SkV2 bl = map2d(m, {src.fLeft, src.fBottom});

        assertEdges(tl.x, tr.x, dst.fLeft, dst.fRight, r.xMapping);
        assertEdges(bl.x, br.x, dst.fLeft, dst.fRight, r.xMapping);
        assertEdges(tl.y, bl.y, dst.fTop, dst.fBottom, r.yMapping);
        assertEdges(tr.y, br.y, dst.fTop, dst.fBottom, r.yMapping);
    }
}

DEF_TEST(M44_mapRect, reporter) {
    auto assertRectsNearlyEqual = [&](const SkRect& actual, const SkRect& expected,
                                      SkV4 epsilon) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual.fLeft, expected.fLeft, epsilon[0]),
                        "Expected %g == %g", actual.fLeft, expected.fLeft);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual.fTop, expected.fTop, epsilon[1]),
                        "Expected %g == %g", actual.fTop, expected.fTop);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual.fRight, expected.fRight, epsilon[2]),
                        "Expected %g == %g", actual.fRight, expected.fRight);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(actual.fBottom, expected.fBottom, epsilon[3]),
                        "Expected %g == %g", actual.fBottom, expected.fBottom);
    };
    auto assertMapRect = [&](const SkM44& m, const SkRect& src, const SkRect* expected,
                             SkV4 epsilon = {1e-5f, 1e-5f, 1e-5f, 1e-5f}) {
        SkRect actual = SkMatrixPriv::MapRect(m, src);
        if (expected) {
            assertRectsNearlyEqual(actual, *expected, epsilon);
        }
        // Use SkPath's perspective clipping to confirm that mapRect also clips appropriately
        SkPath path = SkPath::Rect(src);
        path.transform(m.asM33(), SkApplyPerspectiveClip::kYes);
        assertRectsNearlyEqual(actual, path.getBounds(), epsilon);
    };

    SkRandom rand;
    const SkRect src = SkRect::MakeXYWH(rand.nextRangeF(-10.f, 10.f), rand.nextRangeF(-10.f, 10.f),
                                        rand.nextRangeF(0.1f, 100.f), rand.nextRangeF(0.1f, 100.f));

    // Identity maps src to src
    assertMapRect(SkM44(), src, &src);
    // Scale+Translate just offsets src
    SkRect st = SkRect::MakeLTRB(10.f + 2.f * src.fLeft,  8.f + 4.f * src.fTop,
                                 10.f + 2.f * src.fRight, 8.f + 4.f * src.fBottom);
    assertMapRect(SkM44::Scale(2.f, 4.f).postTranslate(10.f, 8.f), src, &st);
    // Rotate 45 degrees about center
    assertMapRect(SkM44::Rotate({0.f, 0.f, 1.f}, SK_ScalarPI / 4.f)
                        .preTranslate(-src.centerX(), -src.centerY())
                        .postTranslate(src.centerX(), src.centerY()),
                  src, nullptr);

    // Perspective matrix where src does not need to be clipped w > 0
    SkM44 p = SkM44::Perspective(0.01f, 10.f, SK_ScalarPI / 3.f);
    p.preTranslate(0.f, 5.f, -0.1f);
    p.preConcat(SkM44::Rotate({0.f, 1.f, 0.f}, 0.008f /* radians */));
    assertMapRect(p, src, nullptr);

    // Perspective matrix where src *does* need to be clipped w > 0
    p.setIdentity();
    p.setRow(3, {-.2f, -.6f, 0.f, 8.f});
    // The bottom edge is clipped, but do to the difference in methods between path clipping
    // and mapRect, more tolerance is needed
    assertMapRect(p, src, nullptr, {1e-5f, 1e-5f, 1e-5f, 0.8f});
}
