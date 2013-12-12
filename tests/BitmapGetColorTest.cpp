/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkRect.h"
#include "SkRandom.h"

DEF_TEST(GetColor, reporter) {
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
}
