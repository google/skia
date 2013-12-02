/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmap.h"
#include "SkRect.h"
#include "SkRandom.h"

static int nextRand(SkRandom& rand, int min, int max) {
    return min + (int)rand.nextRangeU(0, max - min);
}

static void rand_irect(SkIRect* rect, int W, int H, SkRandom& rand) {
    const int DX = W / 2;
    const int DY = H / 2;

    rect->fLeft   = nextRand(rand, -DX, W + DX);
    rect->fTop    = nextRand(rand, -DY, H + DY);
    rect->fRight  = nextRand(rand, -DX, W + DX);
    rect->fBottom = nextRand(rand, -DY, H + DY);
    rect->sort();
}

static void test_equal_A1_A8(skiatest::Reporter* reporter,
                       const SkBitmap& bm1, const SkBitmap& bm8) {
    SkASSERT(SkBitmap::kA1_Config == bm1.config());
    SkASSERT(SkBitmap::kA8_Config == bm8.config());

    REPORTER_ASSERT(reporter, bm1.width() == bm8.width());
    REPORTER_ASSERT(reporter, bm1.height() == bm8.height());
    for (int y = 0; y < bm1.height(); ++y) {
        for (int x = 0; x < bm1.width(); ++x) {
            int p1 = *bm1.getAddr1(x, y) & (1 << (7 - (x & 7)));
            SkASSERT(SkIsPow2(p1));
            p1 = p1 ? 0xFF : 0;

            int p8 = *bm8.getAddr8(x, y);
            SkASSERT(0 == p8 || 0xFF == p8);

            REPORTER_ASSERT(reporter, p1 == p8);
        }
    }
}

static void test_eraserect_A1(skiatest::Reporter* reporter) {
    const int W = 43;
    const int H = 13;

    SkBitmap bm1, bm8;

    bm1.setConfig(SkBitmap::kA1_Config, W, H);
    bm1.allocPixels();
    bm8.setConfig(SkBitmap::kA8_Config, W, H);
    bm8.allocPixels();

    SkRandom rand;
    for (int i = 0; i < 10000; ++i) {
        SkIRect area;
        rand_irect(&area, W, H, rand);

        bm1.eraseColor(0);
        bm8.eraseColor(0);

        bm1.eraseArea(area, SK_ColorWHITE);
        bm8.eraseArea(area, SK_ColorWHITE);
        test_equal_A1_A8(reporter, bm1, bm8);
    }
}

static void TestGetColor(skiatest::Reporter* reporter) {
    static const struct Rec {
        SkBitmap::Config    fConfig;
        SkColor             fInColor;
        SkColor             fOutColor;
    } gRec[] = {
        // todo: add some tests that involve alpha, so we exercise the
        // unpremultiply aspect of getColor()
        {   SkBitmap::kA8_Config,           0xFF000000,     0xFF000000  },
        {   SkBitmap::kA8_Config,           0,              0           },
        {   SkBitmap::kRGB_565_Config,      0xFF00FF00,     0xFF00FF00  },
        {   SkBitmap::kRGB_565_Config,      0xFFFF00FF,     0xFFFF00FF  },
        {   SkBitmap::kARGB_8888_Config,    0xFFFFFFFF,     0xFFFFFFFF  },
        {   SkBitmap::kARGB_8888_Config,    0,              0           },
        {   SkBitmap::kARGB_8888_Config,    0xFF224466,     0xFF224466  },
    };

    // specify an area that doesn't touch (0,0) and may extend beyond the
    // bitmap bounds (to test that we catch that in eraseArea
    const SkColor initColor = 0xFF0000FF;
    const SkIRect area = { 1, 1, 3, 3 };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        SkBitmap bm;
        uint32_t storage[4];
        bm.setConfig(gRec[i].fConfig, 2, 2);
        bm.setPixels(storage);

        bm.eraseColor(initColor);
        bm.eraseArea(area, gRec[i].fInColor);

        SkColor c = bm.getColor(1, 1);
        REPORTER_ASSERT(reporter, c == gRec[i].fOutColor);
    }

    test_eraserect_A1(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("GetColor", TestGetColorClass, TestGetColor)
