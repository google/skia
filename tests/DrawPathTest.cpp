/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"

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

// we used to assert if the bounds of the device (clip) was larger than 32K
// even when the path itself was smaller. We just draw and hope in the debug
// version to not assert.
static void test_giantaa(skiatest::Reporter* reporter) {
    const int W = 400;
    const int H = 400;
    SkCanvas* canvas = create(SkBitmap::kARGB_8888_Config, 33000, 10, 0, NULL);
    canvas->clear(0);
    
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.addOval(SkRect::MakeXYWH(-10, -10, 20 + W, 20 + H));
    canvas->drawPath(path, paint);
    canvas->unref();
}

static void TestDrawPath(skiatest::Reporter* reporter) {
    test_giantaa(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DrawPath", TestDrawPathClass, TestDrawPath)
