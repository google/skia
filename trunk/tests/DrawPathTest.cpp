/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"

static SkCanvas* create(SkBitmap::Config config, int w, int h, int rb,
                        void* addr = NULL) {
    SkBitmap bm;
    bm.setConfig(config, w, h, rb);
    if (addr) {
        bm.setPixels(addr);
    } else {
        bm.allocPixels();
    }
    return new SkCanvas(bm);
}

static SkCanvas* new_canvas(int w, int h) {
    return create(SkBitmap::kARGB_8888_Config, w, h, 0, NULL);
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
static void test_crbug131181(skiatest::Reporter*) {
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

    SkAutoTUnref<SkCanvas> canvas(new_canvas(640, 480));

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
}

// This used to assert in debug builds (and crash writing bad memory in release)
// because we overflowed an intermediate value (B coefficient) setting up our
// stepper for the quadratic. Now we bias that value by 1/2 so we don't overflow
static void test_crbug_140803(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 2700, 30*1024);
    bm.allocPixels();
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
static void test_inversepathwithclip(skiatest::Reporter* reporter) {
    SkPath path;

    path.moveTo(0, SkIntToScalar(20));
    path.quadTo(SkIntToScalar(10), SkIntToScalar(10),
                SkIntToScalar(20), SkIntToScalar(20));
    path.toggleInverseFillType();

    SkPaint paint;

    SkAutoTUnref<SkCanvas> canvas(new_canvas(640, 480));
    canvas.get()->save();
    canvas.get()->clipRect(SkRect::MakeWH(SkIntToScalar(19), SkIntToScalar(11)));

    paint.setAntiAlias(false);
    canvas.get()->drawPath(path, paint);
    paint.setAntiAlias(true);
    canvas.get()->drawPath(path, paint);

    canvas.get()->restore();

    // Now do the test again, with the path flipped, so we only draw in the
    // top half of our bounds, and have the clip intersect our bounds at the
    // bottom.
    path.reset();   // preserves our filltype
    path.moveTo(0, SkIntToScalar(10));
    path.quadTo(SkIntToScalar(10), SkIntToScalar(20),
                SkIntToScalar(20), SkIntToScalar(10));
    canvas.get()->clipRect(SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(19),
                                            SkIntToScalar(19), SkIntToScalar(11)));

    paint.setAntiAlias(false);
    canvas.get()->drawPath(path, paint);
    paint.setAntiAlias(true);
    canvas.get()->drawPath(path, paint);
}

static void test_bug533(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
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

    SkAutoTUnref<SkCanvas> canvas(new_canvas(640, 480));
    canvas.get()->drawPath(path, paint);
#endif
}

static void test_crbug_140642(skiatest::Reporter* reporter) {
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

#ifdef SK_SCALAR_IS_FLOAT
    const SkScalar vals[] = { 27734, 35660, 2157846850.0f, 247 };
    SkDashPathEffect dontAssert(vals, 4, -248.135982067f);
#endif
}

static void test_crbug_124652(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    /*
        http://code.google.com/p/chromium/issues/detail?id=124652
        This particular test/bug only applies to the float case, where
        large values can "swamp" small ones.
     */
    SkScalar intervals[2] = {837099584, 33450};
    SkAutoTUnref<SkDashPathEffect> dash(
        new SkDashPathEffect(intervals, 2, -10, false));
#endif
}

static void test_bigcubic(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    SkPath path;
    path.moveTo(64, 3);
    path.cubicTo(-329936, -100000000, -329936, 100000000, 1153, 330003);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkAutoTUnref<SkCanvas> canvas(new_canvas(640, 480));
    canvas.get()->drawPath(path, paint);
#endif
}

// we used to assert if the bounds of the device (clip) was larger than 32K
// even when the path itself was smaller. We just draw and hope in the debug
// version to not assert.
static void test_giantaa(skiatest::Reporter* reporter) {
    const int W = 400;
    const int H = 400;
    SkAutoTUnref<SkCanvas> canvas(new_canvas(33000, 10));
    canvas.get()->clear(0);

    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.addOval(SkRect::MakeXYWH(-10, -10, 20 + W, 20 + H));
    canvas.get()->drawPath(path, paint);
}

static void TestDrawPath(skiatest::Reporter* reporter) {
    test_giantaa(reporter);
    test_bug533(reporter);
    test_bigcubic(reporter);
    test_crbug_124652(reporter);
    test_crbug_140642(reporter);
    test_crbug_140803(reporter);
    test_inversepathwithclip(reporter);
//    test_crbug131181(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawPath", TestDrawPathClass, TestDrawPath)
