/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFloatBits.h"
#include "src/core/SkCubicClipper.h"
#include "tests/Test.h"

// Currently the supersampler blitter uses int16_t for its index into an array
// the width of the clip. Test that we don't crash/assert if we try to draw
// with a device/clip that is larger.
static void test_giantClip() {
    SkBitmap bm;
    bm.allocN32Pixels(64919, 1);
    SkCanvas canvas(bm);
    canvas.clear(SK_ColorTRANSPARENT);

    SkPath path;
    path.moveTo(0, 0); path.lineTo(1, 0); path.lineTo(33, 1);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawPath(path, paint);
}

static void PrintCurve(const char *name, const SkPoint crv[4]) {
    SkDebugf("%s: %.10g, %.10g, %.10g, %.10g, %.10g, %.10g, %.10g, %.10g\n",
            name,
            (float)crv[0].fX, (float)crv[0].fY,
            (float)crv[1].fX, (float)crv[1].fY,
            (float)crv[2].fX, (float)crv[2].fY,
            (float)crv[3].fX, (float)crv[3].fY);

}


static bool CurvesAreEqual(const SkPoint c0[4],
                           const SkPoint c1[4],
                           float tol) {
    for (int i = 0; i < 4; i++) {
        if (SkScalarAbs(c0[i].fX - c1[i].fX) > tol ||
            SkScalarAbs(c0[i].fY - c1[i].fY) > tol
        ) {
            PrintCurve("c0", c0);
            PrintCurve("c1", c1);
            return false;
        }
    }
    return true;
}


static SkPoint* SetCurve(float x0, float y0,
                         float x1, float y1,
                         float x2, float y2,
                         float x3, float y3,
                         SkPoint crv[4]) {
    crv[0].fX = x0;   crv[0].fY = y0;
    crv[1].fX = x1;   crv[1].fY = y1;
    crv[2].fX = x2;   crv[2].fY = y2;
    crv[3].fX = x3;   crv[3].fY = y3;
    return crv;
}


DEF_TEST(ClipCubic, reporter) {
    static SkPoint crv[4] = {
        { SkIntToScalar(0), SkIntToScalar(0)  },
        { SkIntToScalar(2), SkIntToScalar(3)  },
        { SkIntToScalar(1), SkIntToScalar(10) },
        { SkIntToScalar(4), SkIntToScalar(12) }
    };

    SkCubicClipper clipper;
    SkPoint clipped[4], shouldbe[4];
    SkIRect clipRect;
    bool success;
    const float tol = 1e-4f;

    // Test no clip, with plenty of room.
    clipRect.set(-2, -2, 6, 14);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0, 0, 2, 3, 1, 10, 4, 12, shouldbe), tol));

    // Test no clip, touching first point.
    clipRect.set(-2, 0, 6, 14);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0, 0, 2, 3, 1, 10, 4, 12, shouldbe), tol));

    // Test no clip, touching last point.
    clipRect.set(-2, -2, 6, 12);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0, 0, 2, 3, 1, 10, 4, 12, shouldbe), tol));

    // Test all clip.
    clipRect.set(-2, 14, 6, 20);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == false);

    // Test clip at 1.
    clipRect.set(-2, 1, 6, 14);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0.5126125216f, 1,
        1.841195941f,  4.337081432f,
        1.297019958f,  10.19801331f,
        4,            12,
        shouldbe), tol));

    // Test clip at 2.
    clipRect.set(-2, 2, 6, 14);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        00.8412352204f, 2,
        1.767683744f,   5.400758266f,
        1.55052948f,    10.36701965f,
        4,             12,
        shouldbe), tol));

    // Test clip at 11.
    clipRect.set(-2, -2, 6, 11);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0,           0,
        1.742904663f, 2.614356995f,
        1.207521796f, 8.266430855f,
        3.026495695f, 11,
        shouldbe), tol));

    // Test clip at 10.
    clipRect.set(-2, -2, 6, 10);
    clipper.setClip(clipRect);
    success = clipper.clipCubic(crv, clipped);
    REPORTER_ASSERT(reporter, success == true);
    REPORTER_ASSERT(reporter, CurvesAreEqual(clipped, SetCurve(
        0,           0,
        1.551193237f, 2.326789856f,
        1.297736168f, 7.059780121f,
        2.505550385f, 10,
        shouldbe), tol));

    test_giantClip();
}

DEF_TEST(test_fuzz_crbug_698714, reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(500, 500));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0,0
    path.lineTo(SkBits2Float(0x43434343), SkBits2Float(0x43430143));  //195.263f, 195.005f
    path.lineTo(SkBits2Float(0x43434343), SkBits2Float(0x43434343));  //195.263f, 195.263f
    path.lineTo(SkBits2Float(0xb5434343), SkBits2Float(0x434300be));  //-7.2741e-07f, 195.003f
    // 195.263f, 195.263f, -1.16387e-05f, 3.58641e-38f, 3.85088e-29f,1.86082e-39f
    path.cubicTo(SkBits2Float(0x43434343), SkBits2Float(0x43434341),
            SkBits2Float(0xb74343bd), SkBits2Float(0x01434343),
            SkBits2Float(0x10434343), SkBits2Float(0x00144332));
    // 4.11823e-38f, 195.263f, 195.263f, 195.263f, -7.2741e-07f, 195.263f
    path.cubicTo(SkBits2Float(0x016037c0), SkBits2Float(0x43434343),
            SkBits2Float(0x43434343), SkBits2Float(0x43434343),
            SkBits2Float(0xb5434343), SkBits2Float(0x43434343));
    // 195.263f, 195.263f, -1.16387e-05f, 3.58641e-38f, 195.263f, -2
    path.cubicTo(SkBits2Float(0x43434344), SkBits2Float(0x43434341),
            SkBits2Float(0xb74343bd), SkBits2Float(0x01434343),
            SkBits2Float(0x43434343), SkBits2Float(0xc0000014));
    // -5.87228e+06f, 3.7773e-07f, 3.60231e-13f, -6.64511e+06f,2.77692e-15f, 2.48803e-15f
    path.cubicTo(SkBits2Float(0xcab33535), SkBits2Float(0x34cacaca),
            SkBits2Float(0x2acacaca), SkBits2Float(0xcacacae3),
            SkBits2Float(0x27481927), SkBits2Float(0x27334805));
    path.lineTo(SkBits2Float(0xb5434343), SkBits2Float(0x43434343));  //-7.2741e-07f, 195.263f
    // 195.263f, 195.263f, -1.16387e-05f, 195.212f, 195.263f, -2
    path.cubicTo(SkBits2Float(0x43434343), SkBits2Float(0x43434341),
            SkBits2Float(0xb74343b9), SkBits2Float(0x43433643),
            SkBits2Float(0x43434343), SkBits2Float(0xc0000014));
    path.lineTo(SkBits2Float(0xc7004343), SkBits2Float(0x27480527));  //-32835.3f, 2.77584e-15f
    path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0,0
    path.close();
    canvas->clipRect({0, 0, 65, 202});
    canvas->drawPath(path, paint);
}

DEF_TEST(cubic_scan_error_crbug_844457_and_845489, reporter) {
    auto surface(SkSurface::MakeRasterN32Premul(100, 100));
    SkCanvas* canvas = surface->getCanvas();
    SkPaint p;

    SkPath path;
    path.moveTo(-30/64.0, -31/64.0);
    path.cubicTo(-31/64.0, -31/64,-31/64.0, -31/64,-31/64.0, 100);
    path.lineTo(100, 100);
    canvas->drawPath(path, p);

    // May need to define SK_RASTERIZE_EVEN_ROUNDING to trigger the need for this test
    path.reset();
    path.moveTo(-30/64.0f,             -31/64.0f + 1/256.0f);
    path.cubicTo(-31/64.0f + 1/256.0f, -31/64.0f + 1/256.0f,
                 -31/64.0f + 1/256.0f, -31/64.0f + 1/256.0f,
                 -31/64.0f + 1/256.0f, 100);
    path.lineTo(100, 100);
    canvas->drawPath(path, p);
}
