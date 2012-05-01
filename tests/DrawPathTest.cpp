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
    test_inversepathwithclip(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawPath", TestDrawPathClass, TestDrawPath)
