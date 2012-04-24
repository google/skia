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
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawPath", TestDrawPathClass, TestDrawPath)
