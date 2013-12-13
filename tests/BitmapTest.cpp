/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"

#include "Test.h"
#include "TestClassDef.h"

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
}
