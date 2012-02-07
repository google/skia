
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkDeferredCanvas.h"


static const int gWidth = 2;
static const int gHeight = 2;

static void create(SkBitmap* bm, SkBitmap::Config config, SkColor color) {
    bm->setConfig(config, gWidth, gHeight);
    bm->allocPixels();
    bm->eraseColor(color);
}

static void TestDeferredCanvasBitmapAccess(skiatest::Reporter* reporter) {
    SkBitmap store;

    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    canvas.clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    SkBitmap accessed = canvas.getDevice()->accessBitmap(false);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0x00000000); //verify that clear was executed
    REPORTER_ASSERT(reporter, accessed.pixelRef() == store.pixelRef());
}

static void TestDeferredCanvasFlush(skiatest::Reporter* reporter) {
    SkBitmap store;

    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    canvas.clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    canvas.flush();
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0x00000000); //verify that clear was executed
}

static void TestDeferredCanvas(skiatest::Reporter* reporter) {
    TestDeferredCanvasBitmapAccess(reporter);
    TestDeferredCanvasFlush(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DeferredCanvas", TestDeferredCanvasClass, TestDeferredCanvas)
