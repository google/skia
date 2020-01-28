/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"

// This GM should draw two yellow boxes; the bug drew one in cyan.

DEF_SIMPLE_GM(skbug_9819, c, 256, 256) {
    auto info = SkImageInfo::Make(1,1, kUnknown_SkColorType, kPremul_SkAlphaType);
    SkBitmap rgba,
             bgra;
    rgba.allocPixels(info.makeColorType(kRGBA_8888_SkColorType));
    bgra.allocPixels(info.makeColorType(kBGRA_8888_SkColorType));

    SkColor yellow = 0xffffff00;
    rgba.eraseColor(yellow);
    bgra.eraseColor(yellow);

    c->save();
        c->scale(128,128);
        c->drawBitmap(rgba, 0,0);
        c->drawBitmap(bgra, 0,1);
    c->restore();

    auto grade = [&](int x, int y){
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(1,1,
                                         kGray_8_SkColorType,
                                         kUnpremul_SkAlphaType,
                                         SkColorSpace::MakeSRGB()));
        if (!c->readPixels(bm, x,y)) {
            // Picture-backed canvases, that sort of thing.  Just assume they're good.
            MarkGMGood(c, x+128, y);
            return;
        }

        // We test only luma so that grayscale destinations are also correctly graded:
        //    - yellow (good) is around 237
        //    - cyan   (bad)  is around 202
        uint8_t gray = *bm.getAddr8(0,0);
        (abs(gray - 237) > 2 ? MarkGMBad
                             : MarkGMGood)(c, x+128,y);
    };

    grade(64,  64);
    grade(64, 192);
}
