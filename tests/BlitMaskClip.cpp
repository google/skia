/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkMask.h"
#include "tests/Test.h"

#include <string.h>

class TestBlitter : public SkBlitter {
public:
    TestBlitter(SkIRect bounds, skiatest::Reporter* reporter)
        : fBounds(bounds)
        , fReporter(reporter) { }

    void blitH(int x, int y, int width) override {

        REPORTER_ASSERT(fReporter, x >= fBounds.fLeft && x < fBounds.fRight);
        REPORTER_ASSERT(fReporter, y >= fBounds.fTop && y < fBounds.fBottom);
        int right = x + width;
        REPORTER_ASSERT(fReporter, right > fBounds.fLeft && right <= fBounds.fRight);
    }

    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override {
        SkDEBUGFAIL("blitAntiH not implemented");
    }

private:
    SkIRect fBounds;
    skiatest::Reporter* fReporter;
};

// Exercise all clips compared with different widths of bitMask. Make sure that no buffer
// overruns happen.
DEF_TEST(BlitAndClip, reporter) {
    const int originX = 100;
    const int originY = 100;
    for (int width = 1; width <= 32; width++) {
        const int height = 2;
        int rowBytes = (width + 7) >> 3;
        uint8_t* bits = new uint8_t[rowBytes * height];
        memset(bits, 0xAA, rowBytes * height);

        SkIRect b = {originX, originY, originX + width, originY + height};

        SkMask mask;
        mask.fFormat = SkMask::kBW_Format;
        mask.fBounds = b;
        mask.fImage = (uint8_t*)bits;
        mask.fRowBytes = rowBytes;

        TestBlitter tb(mask.fBounds, reporter);

        for (int top = b.fTop; top < b.fBottom; top++) {
            for (int bottom = top + 1; bottom <= b.fBottom; bottom++) {
                for (int left = b.fLeft; left < b.fRight; left++) {
                    for (int right = left + 1; right <= b.fRight; right++) {
                        SkIRect clipRect = {left, top, right, bottom};
                        tb.blitMask(mask, clipRect);
                    }
                }
            }
        }

        delete [] bits;
    }
}
