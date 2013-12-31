/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"

#include "Test.h"
#include "TestClassDef.h"

static void test_bigwidth(skiatest::Reporter* reporter) {
    SkBitmap bm;
    int width = 1 << 29;    // *4 will be the high-bit of 32bit int

    REPORTER_ASSERT(reporter, bm.setConfig(SkBitmap::kA8_Config, width, 1));
    REPORTER_ASSERT(reporter, bm.setConfig(SkBitmap::kRGB_565_Config, width, 1));

    // for a 4-byte config, this width will compute a rowbytes of 0x80000000,
    // which does not fit in a int32_t. setConfig should detect this, and fail.

    // TODO: perhaps skia can relax this, and only require that rowBytes fit
    //       in a uint32_t (or larger), but for now this is the constraint.

    REPORTER_ASSERT(reporter, !bm.setConfig(SkBitmap::kARGB_8888_Config, width, 1));
}

/**
 *  This test contains basic sanity checks concerning bitmaps.
 */
DEF_TEST(Bitmap, reporter) {
    const SkBitmap::Config conf = SkBitmap::kARGB_8888_Config;
    // Zero-sized bitmaps are allowed
    for (int width = 0; width < 2; ++width) {
        for (int height = 0; height < 2; ++height) {
            SkBitmap bm;
            bool setConf = bm.setConfig(conf, width, height);
            REPORTER_ASSERT(reporter, setConf);
            if (setConf) {
                REPORTER_ASSERT(reporter, bm.allocPixels(NULL));
            }
            REPORTER_ASSERT(reporter, SkToBool(width & height) != bm.empty());
        }
    }

    test_bigwidth(reporter);
}
