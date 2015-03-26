/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkTSort.h"
#include "Test.h"

static void testTightBoundsLines(PathOpsThreadState* data) {
    SkRandom ran;
    for (int index = 0; index < 1000; ++index) {
        SkPath path;
        int contourCount = ran.nextRangeU(1, 10);
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            int lineCount = ran.nextRangeU(1, 10);
            path.moveTo(ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000));
            for (int lIndex = 0; lIndex < lineCount; ++lIndex) {
                path.lineTo(ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000));
            }
            if (ran.nextBool()) {
                path.close();
            }
        }
        SkRect classicBounds = path.getBounds();
        SkRect tightBounds;
        REPORTER_ASSERT(data->fReporter, TightBounds(path, &tightBounds));
        REPORTER_ASSERT(data->fReporter, classicBounds == tightBounds);
    }
}

DEF_TEST(PathOpsTightBoundsLines, reporter) {
    initializeTests(reporter, "tightBoundsLines");
    PathOpsThreadedTestRunner testRunner(reporter);
    int outerCount = reporter->allowExtendedTest() ? 100 : 1;
    for (int index = 0; index < outerCount; ++index) {
        for (int idx2 = 0; idx2 < 10; ++idx2) {
            *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                    (&testTightBoundsLines, 0, 0, 0, 0, &testRunner));
        }
    }
    testRunner.render();
}

static void testTightBoundsQuads(PathOpsThreadState* data) {
    SkRandom ran;
    const int bitWidth = 32;
    const int bitHeight = 32;
    const float pathMin = 1;
    const float pathMax = (float) (bitHeight - 2);
    SkBitmap& bits = *data->fBitmap;
    if (bits.width() == 0) {
        bits.allocN32Pixels(bitWidth, bitHeight);
    }
    SkCanvas canvas(bits);
    SkPaint paint;
    for (int index = 0; index < 100; ++index) {
        SkPath path;
        int contourCount = ran.nextRangeU(1, 10);
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            int lineCount = ran.nextRangeU(1, 10);
            path.moveTo(ran.nextRangeF(1, pathMax), ran.nextRangeF(pathMin, pathMax));
            for (int lIndex = 0; lIndex < lineCount; ++lIndex) {
                if (ran.nextBool()) {
                    path.lineTo(ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax));
                } else {
                    path.quadTo(ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax),
                            ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax));
                }
            }
            if (ran.nextBool()) {
                path.close();
            }
        }
        SkRect classicBounds = path.getBounds();
        SkRect tightBounds;
        REPORTER_ASSERT(data->fReporter, TightBounds(path, &tightBounds));
        REPORTER_ASSERT(data->fReporter, classicBounds.contains(tightBounds));
        canvas.drawColor(SK_ColorWHITE);
        canvas.drawPath(path, paint);
        SkIRect bitsWritten = {31, 31, 0, 0};
        for (int y = 0; y < bitHeight; ++y) {
            uint32_t* addr1 = data->fBitmap->getAddr32(0, y);
            bool lineWritten = false;
            for (int x = 0; x < bitWidth; ++x) {
                if (addr1[x] == (uint32_t) -1) {
                    continue;
                }
                lineWritten = true;
                bitsWritten.fLeft = SkTMin(bitsWritten.fLeft, x);
                bitsWritten.fRight = SkTMax(bitsWritten.fRight, x);
            }
            if (!lineWritten) {
                continue;
            }
            bitsWritten.fTop = SkTMin(bitsWritten.fTop, y);
            bitsWritten.fBottom = SkTMax(bitsWritten.fBottom, y);
        }
        if (!bitsWritten.isEmpty()) {
            SkIRect tightOut;
            tightBounds.roundOut(&tightOut);
            REPORTER_ASSERT(data->fReporter, tightOut.contains(bitsWritten));
        }
    }
}

DEF_TEST(PathOpsTightBoundsQuads, reporter) {
    initializeTests(reporter, "tightBoundsQuads");
    PathOpsThreadedTestRunner testRunner(reporter);
    int outerCount = reporter->allowExtendedTest() ? 100 : 1;
    for (int index = 0; index < outerCount; ++index) {
        for (int idx2 = 0; idx2 < 10; ++idx2) {
            *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                    (&testTightBoundsQuads, 0, 0, 0, 0, &testRunner));
        }
    }
    testRunner.render();
}
