/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"

DEF_TEST(GetColor, reporter) {
    static const struct Rec {
        SkColorType fColorType;
        SkColor     fInColor;
        SkColor     fOutColor;
    } gRec[] = {
        // todo: add some tests that involve alpha, so we exercise the
        // unpremultiply aspect of getColor()
        {   kAlpha_8_SkColorType,   0xFF000000,     0xFF000000  },
        {   kAlpha_8_SkColorType,   0,              0           },
        {   kRGB_565_SkColorType,   0xFF00FF00,     0xFF00FF00  },
        {   kRGB_565_SkColorType,   0xFFFF00FF,     0xFFFF00FF  },
        {   kN32_SkColorType,       0xFFFFFFFF,     0xFFFFFFFF  },
        {   kN32_SkColorType,       0,              0           },
        {   kN32_SkColorType,       0xFF224466,     0xFF224466  },
    };

    // specify an area that doesn't touch (0,0) and may extend beyond the
    // bitmap bounds (to test that we catch that in eraseArea
    const SkColor initColor = 0xFF0000FF;
    const SkIRect area = { 1, 1, 3, 3 };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
        SkImageInfo info = SkImageInfo::Make(2, 2, gRec[i].fColorType,
                                             kPremul_SkAlphaType);
        SkBitmap bm;
        uint32_t storage[4];
        bm.installPixels(info, storage, info.minRowBytes());

        bm.eraseColor(initColor);
        bm.eraseArea(area, gRec[i].fInColor);

        SkColor c = bm.getColor(1, 1);
        REPORTER_ASSERT(reporter, c == gRec[i].fOutColor);
    }
}
