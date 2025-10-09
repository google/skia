/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkRandom.h"
#include "src/pathops/SkPathOpsCommon.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstdint>

static void testTightBoundsLines(PathOpsThreadState* data) {
    SkRandom ran;
    for (int index = 0; index < 1000; ++index) {
        SkPathBuilder builder;
        int contourCount = ran.nextRangeU(1, 10);
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            int lineCount = ran.nextRangeU(1, 10);
            builder.moveTo(ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000));
            for (int lIndex = 0; lIndex < lineCount; ++lIndex) {
                builder.lineTo(ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000));
            }
            if (ran.nextBool()) {
                builder.close();
            }
        }
        SkPath path = builder.detach();
        SkRect classicBounds = path.getBounds();
        SkRect tightBounds;
        REPORTER_ASSERT(data->fReporter, ComputeTightBounds(path, &tightBounds));
        REPORTER_ASSERT(data->fReporter, classicBounds == tightBounds);
    }
}

DEF_TEST(PathOpsTightBoundsLines, reporter) {
    initializeTests(reporter, "tightBoundsLines");
    PathOpsThreadedTestRunner testRunner(reporter);
    int outerCount = reporter->allowExtendedTest() ? 100 : 1;
    for (int index = 0; index < outerCount; ++index) {
        for (int idx2 = 0; idx2 < 10; ++idx2) {
            *testRunner.fRunnables.append() =
                    new PathOpsThreadedRunnable(&testTightBoundsLines, 0, 0, 0, 0, &testRunner);
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
        SkPathBuilder bu;
        int contourCount = ran.nextRangeU(1, 10);
        for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
            int lineCount = ran.nextRangeU(1, 10);
            bu.moveTo(ran.nextRangeF(1, pathMax), ran.nextRangeF(pathMin, pathMax));
            for (int lIndex = 0; lIndex < lineCount; ++lIndex) {
                if (ran.nextBool()) {
                    bu.lineTo(ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax));
                } else {
                    bu.quadTo(ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax),
                              ran.nextRangeF(pathMin, pathMax), ran.nextRangeF(pathMin, pathMax));
                }
            }
            if (ran.nextBool()) {
                bu.close();
            }
        }
        SkPath path = bu.detach();
        SkRect classicBounds = path.getBounds();
        SkRect tightBounds;
        REPORTER_ASSERT(data->fReporter, ComputeTightBounds(path, &tightBounds));
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
                bitsWritten.fLeft = std::min(bitsWritten.fLeft, x);
                bitsWritten.fRight = std::max(bitsWritten.fRight, x);
            }
            if (!lineWritten) {
                continue;
            }
            bitsWritten.fTop = std::min(bitsWritten.fTop, y);
            bitsWritten.fBottom = std::max(bitsWritten.fBottom, y);
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
            *testRunner.fRunnables.append() =
                    new PathOpsThreadedRunnable(&testTightBoundsQuads, 0, 0, 0, 0, &testRunner);
        }
    }
    testRunner.render();
}

DEF_TEST(PathOpsTightBoundsMove, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(10, 10)
                  .close()
                  .moveTo(20, 20)
                  .lineTo(20, 20)
                  .close()
                  .moveTo(15, 15)
                  .lineTo(15, 15)
                  .close()
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds == tight);
}

DEF_TEST(PathOpsTightBoundsMoveOne, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(20, 20)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds == tight);
}

DEF_TEST(PathOpsTightBoundsMoveTwo, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(20, 20)
                  .moveTo(40, 40)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds == tight);
}

DEF_TEST(PathOpsTightBoundsTiny, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(1, 1)
                  .quadTo(1.000001f, 1, 1, 1)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    SkRect moveBounds = {1, 1, 1, 1};
    REPORTER_ASSERT(reporter, bounds != tight);
    REPORTER_ASSERT(reporter, moveBounds == tight);
}

DEF_TEST(PathOpsTightBoundsWellBehaved, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(1, 1)
                  .quadTo(2, 3, 4, 5)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds == tight);
}

DEF_TEST(PathOpsTightBoundsIllBehaved, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(1, 1)
                  .quadTo(4, 3, 2, 2)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds != tight);
}

DEF_TEST(PathOpsTightBoundsIllBehavedScaled, reporter) {
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .quadTo(1048578, 1048577, 1048576, 1048576)
                  .detach();
    const SkRect& bounds = path.getBounds();
    SkRect tight;
    REPORTER_ASSERT(reporter, ComputeTightBounds(path, &tight));
    REPORTER_ASSERT(reporter, bounds != tight);
    REPORTER_ASSERT(reporter, tight.right() == 1048576);
    REPORTER_ASSERT(reporter, tight.bottom() == 1048576);
}
