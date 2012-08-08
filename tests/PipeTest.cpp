
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SamplePipeControllers.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGPipe.h"
#include "Test.h"

// Ensures that the pipe gracefully handles drawing an invalid bitmap.
static void testDrawingBadBitmap(skiatest::Reporter* reporter, SkCanvas* pipeCanvas) {
    SkBitmap badBitmap;
    badBitmap.setConfig(SkBitmap::kNo_Config, 5, 5);
    pipeCanvas->drawBitmap(badBitmap, 0, 0);
}

static void test_pipeTests(skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 64, 64);
    SkCanvas canvas(bitmap);

    PipeController pipeController(&canvas);
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    testDrawingBadBitmap(reporter, pipeCanvas);
    writer.endRecording();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PipeTest", PipeTestClass, test_pipeTests)
