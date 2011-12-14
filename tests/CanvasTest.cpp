
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkCanvas.h"

static void test_isDrawingToLayer(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);
    bm.allocPixels();
    
    SkCanvas canvas(bm);

    REPORTER_ASSERT(reporter, !canvas.isDrawingToLayer());
    canvas.save();
    REPORTER_ASSERT(reporter, !canvas.isDrawingToLayer());
    
    const SkRect* bounds = NULL;    // null means include entire bounds
    const SkPaint* paint = NULL;

    canvas.saveLayer(bounds, paint);
    REPORTER_ASSERT(reporter, canvas.isDrawingToLayer());
    canvas.restore();
    REPORTER_ASSERT(reporter, !canvas.isDrawingToLayer());

    canvas.saveLayer(bounds, paint);
    canvas.saveLayer(bounds, paint);
    REPORTER_ASSERT(reporter, canvas.isDrawingToLayer());
    canvas.restore();
    REPORTER_ASSERT(reporter, canvas.isDrawingToLayer());
    canvas.restore();
    // now layer count should be 0
    REPORTER_ASSERT(reporter, !canvas.isDrawingToLayer());
}

static void TestCanvas(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 256, 256);
    bm.allocPixels();

    SkCanvas canvas(bm);
    int n;

    REPORTER_ASSERT(reporter, 1 == canvas.getSaveCount());
    n = canvas.save();
    REPORTER_ASSERT(reporter, 1 == n);
    REPORTER_ASSERT(reporter, 2 == canvas.getSaveCount());
    canvas.save();
    canvas.save();
    REPORTER_ASSERT(reporter, 4 == canvas.getSaveCount());
    canvas.restoreToCount(2);
    REPORTER_ASSERT(reporter, 2 == canvas.getSaveCount());

    // should this pin to 1, or be a no-op, or crash?
    canvas.restoreToCount(0);
    REPORTER_ASSERT(reporter, 1 == canvas.getSaveCount());

    test_isDrawingToLayer(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Canvas", TestCanvasClass, TestCanvas)
