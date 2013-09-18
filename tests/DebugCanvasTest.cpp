/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "debugger/SkDebugCanvas.h"

static void test_debugRect(skiatest::Reporter* reporter) {
    SkDebugCanvas canvas(100, 100);
    canvas.drawRect(SkRect::MakeWH(100, 100), SkPaint());

    const SkTDArray<SkDrawCommand*> cmds = canvas.getDrawCommands();
    REPORTER_ASSERT(reporter, cmds.count() > 0);

    const SkTDArray<SkString*>& result = *cmds[0]->Info();
    REPORTER_ASSERT(reporter, result.count() > 0);

    SkString expected("SkRect: (0, 0, 100, 100)");
    REPORTER_ASSERT(reporter, expected == *result[0]);
}

static void TestDebugCanvas(skiatest::Reporter* reporter) {
    test_debugRect(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DebugCanvas", TestDebugCanvasClass, TestDebugCanvas)
