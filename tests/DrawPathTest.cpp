/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkSurface.h"
#include "Test.h"

// test that we can draw an aa-rect at coordinates > 32K (bigger than fixedpoint)
static void test_big_aa_rect(skiatest::Reporter* reporter) {
    SkBitmap output;
    SkPMColor pixel[1];
    output.installPixels(SkImageInfo::MakeN32Premul(1, 1), pixel, 4);

    SkSurface* surf = SkSurface::NewRasterN32Premul(300, 33300);
    SkCanvas* canvas = surf->getCanvas();

    SkRect r = { 0, 33000, 300, 33300 };
    int x = SkScalarRoundToInt(r.left());
    int y = SkScalarRoundToInt(r.top());

    // check that the pixel in question starts as transparent (by the surface)
    if (canvas->readPixels(&output, x, y)) {
        REPORTER_ASSERT(reporter, 0 == pixel[0]);
    } else {
        REPORTER_ASSERT_MESSAGE(reporter, false, "readPixels failed");
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);

    canvas->drawRect(r, paint);

    // Now check that it is BLACK
    if (canvas->readPixels(&output, x, y)) {
        // don't know what swizzling PMColor did, but white should always
        // appear the same.
        REPORTER_ASSERT(reporter, 0xFFFFFFFF == pixel[0]);
    } else {
        REPORTER_ASSERT_MESSAGE(reporter, false, "readPixels failed");
    }
    surf->unref();
}

///////////////////////////////////////////////////////////////////////////////

static void moveToH(SkPath* path, const uint32_t raw[]) {
    const float* fptr = (const float*)raw;
    path->moveTo(fptr[0], fptr[1]);
}

static void cubicToH(SkPath* path, const uint32_t raw[]) {
    const float* fptr = (const float*)raw;
    path->cubicTo(fptr[0], fptr[1], fptr[2], fptr[3], fptr[4], fptr[5]);
}

// This used to assert, because we performed a cast (int)(pt[0].fX * scale) to
// arrive at an int (SkFDot6) rather than calling sk_float_round2int. The assert
// was that the initial line-segment produced by the cubic was not monotonically
// going down (i.e. the initial DY was negative). By rounding the floats, we get
// the more proper result.
//
// http://code.google.com/p/chromium/issues/detail?id=131181
//

// we're not calling this test anymore; is that for a reason?

static void test_crbug131181() {
    /*
     fX = 18.8943768,
     fY = 129.121277
     }, {
     fX = 18.8937435,
     fY = 129.121689
     }, {
     fX = 18.8950119,
     fY = 129.120422
     }, {
     fX = 18.5030727,
     fY = 129.13121
     */
    uint32_t data[] = {
        0x419727af, 0x43011f0c, 0x41972663, 0x43011f27,
        0x419728fc, 0x43011ed4, 0x4194064b, 0x43012197
    };

    SkPath path;
    moveToH(&path, &data[0]);
    cubicToH(&path, &data[2]);

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(640, 480));

    SkPaint paint;
    paint.setAntiAlias(true);
    surface->getCanvas()->drawPath(path, paint);
}

// This used to assert in debug builds (and crash writing bad memory in release)
// because we overflowed an intermediate value (B coefficient) setting up our
// stepper for the quadratic. Now we bias that value by 1/2 so we don't overflow
static void test_crbug_140803() {
    SkBitmap bm;
    bm.allocN32Pixels(2700, 30*1024);
    SkCanvas canvas(bm);

    SkPath path;
    path.moveTo(2762, 20);
    path.quadTo(11, 21702, 10, 21706);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawPath(path, paint);
}

// Need to exercise drawing an inverse-path whose bounds intersect the clip,
// but whose edges do not (since its a quad which draws only in the bottom half
// of its bounds).
// In the debug build, we used to assert in this case, until it was fixed.
//
static void test_inversepathwithclip() {
    SkPath path;

    path.moveTo(0, 20);
    path.quadTo(10, 10, 20, 20);
    path.toggleInverseFillType();

    SkPaint paint;

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(640, 480));
    SkCanvas* canvas = surface->getCanvas();
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(19, 11));

    paint.setAntiAlias(false);
    canvas->drawPath(path, paint);
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);

    canvas->restore();

    // Now do the test again, with the path flipped, so we only draw in the
    // top half of our bounds, and have the clip intersect our bounds at the
    // bottom.
    path.reset();   // preserves our filltype
    path.moveTo(0, 10);
    path.quadTo(10, 20, 20, 10);
    canvas->clipRect(SkRect::MakeXYWH(0, 19, 19, 11));

    paint.setAntiAlias(false);
    canvas->drawPath(path, paint);
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
}

static void test_bug533() {
    /*
        http://code.google.com/p/skia/issues/detail?id=533
        This particular test/bug only applies to the float case, where the
        coordinates are very large.
     */
    SkPath path;
    path.moveTo(64, 3);
    path.quadTo(-329936, -100000000, 1153, 330003);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(640, 480));
    surface->getCanvas()->drawPath(path, paint);
}

static void test_crbug_140642() {
    /*
     *  We used to see this construct, and due to rounding as we accumulated
     *  our length, the loop where we apply the phase would run off the end of
     *  the array, since it relied on just -= each interval value, which did not
     *  behave as "expected". Now the code explicitly checks for walking off the
     *  end of that array.

     *  A different (better) fix might be to rewrite dashing to do all of its
     *  length/phase/measure math using double, but this may need to be
     *  coordinated with SkPathMeasure, to be consistent between the two.

     <path stroke="mintcream" stroke-dasharray="27734 35660 2157846850 247"
           stroke-dashoffset="-248.135982067">
     */

    const SkScalar vals[] = { 27734, 35660, 2157846850.0f, 247 };
    SkAutoTUnref<SkDashPathEffect> dontAssert(SkDashPathEffect::Create(vals, 4, -248.135982067f));
}

static void test_crbug_124652() {
    /*
        http://code.google.com/p/chromium/issues/detail?id=124652
        This particular test/bug only applies to the float case, where
        large values can "swamp" small ones.
     */
    SkScalar intervals[2] = {837099584, 33450};
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, 2, -10));
}

static void test_bigcubic() {
    SkPath path;
    path.moveTo(64, 3);
    path.cubicTo(-329936, -100000000, -329936, 100000000, 1153, 330003);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(640, 480));
    surface->getCanvas()->drawPath(path, paint);
}

// we used to assert if the bounds of the device (clip) was larger than 32K
// even when the path itself was smaller. We just draw and hope in the debug
// version to not assert.
static void test_giantaa() {
    const int W = 400;
    const int H = 400;
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(33000, 10));

    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.addOval(SkRect::MakeXYWH(-10, -10, 20 + W, 20 + H));
    surface->getCanvas()->drawPath(path, paint);
}

// Extremely large path_length/dash_length ratios may cause infinite looping
// in SkDashPathEffect::filterPath() due to single precision rounding.
// The test is quite expensive, but it should get much faster after the fix
// for http://crbug.com/165432 goes in.
static void test_infinite_dash(skiatest::Reporter* reporter) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(5000000, 0);

    SkScalar intervals[] = { 0.2f, 0.2f };
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, 2, 0));

    SkPath filteredPath;
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(dash);

    paint.getFillPath(path, &filteredPath);
    // If we reach this, we passed.
    REPORTER_ASSERT(reporter, true);
}

// http://crbug.com/165432
// Limit extreme dash path effects to avoid exhausting the system memory.
static void test_crbug_165432(skiatest::Reporter* reporter) {
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(10000000, 0);

    SkScalar intervals[] = { 0.5f, 0.5f };
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, 2, 0));

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(dash);

    SkPath filteredPath;
    SkStrokeRec rec(paint);
    REPORTER_ASSERT(reporter, !dash->filterPath(&filteredPath, path, &rec, NULL));
    REPORTER_ASSERT(reporter, filteredPath.isEmpty());
}

DEF_TEST(DrawPath, reporter) {
    test_giantaa();
    test_bug533();
    test_bigcubic();
    test_crbug_124652();
    test_crbug_140642();
    test_crbug_140803();
    test_inversepathwithclip();
    // why?
    if (false) test_crbug131181();
    test_infinite_dash(reporter);
    test_crbug_165432(reporter);
    test_big_aa_rect(reporter);
}
