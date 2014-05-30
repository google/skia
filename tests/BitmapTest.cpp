/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"

#include "Test.h"

static void test_bigwidth(skiatest::Reporter* reporter) {
    SkBitmap bm;
    int width = 1 << 29;    // *4 will be the high-bit of 32bit int

    SkImageInfo info = SkImageInfo::MakeA8(width, 1);
    REPORTER_ASSERT(reporter, bm.setInfo(info));
    info.fColorType = kRGB_565_SkColorType;
    REPORTER_ASSERT(reporter, bm.setInfo(info));

    // for a 4-byte config, this width will compute a rowbytes of 0x80000000,
    // which does not fit in a int32_t. setConfig should detect this, and fail.

    // TODO: perhaps skia can relax this, and only require that rowBytes fit
    //       in a uint32_t (or larger), but for now this is the constraint.

    info.fColorType = kN32_SkColorType;
    REPORTER_ASSERT(reporter, !bm.setInfo(info));
}

/**
 *  This test contains basic sanity checks concerning bitmaps.
 */
DEF_TEST(Bitmap, reporter) {
    // Zero-sized bitmaps are allowed
    for (int width = 0; width < 2; ++width) {
        for (int height = 0; height < 2; ++height) {
            SkBitmap bm;
            bool setConf = bm.setInfo(SkImageInfo::MakeN32Premul(width, height));
            REPORTER_ASSERT(reporter, setConf);
            if (setConf) {
                REPORTER_ASSERT(reporter, bm.allocPixels(NULL));
            }
            REPORTER_ASSERT(reporter, SkToBool(width & height) != bm.empty());
        }
    }

    test_bigwidth(reporter);
}
