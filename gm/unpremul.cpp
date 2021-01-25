/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"

#include <string.h>

DEF_SIMPLE_GM(unpremul, canvas, 200, 200) {
    const SkColor color = 0xbf400000;

    auto grade = [&](int x, int y){
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(1,1,
                                         kBGRA_8888_SkColorType,
                                         kUnpremul_SkAlphaType,
                                         SkColorSpace::MakeSRGB()));
        if (!canvas->readPixels(bm, x,y)) {
            // Picture-backed canvases, that sort of thing.  Just assume they're good.
            MarkGMGood(canvas, 140,40);
            return;
        }

        SkColor pixel;
        memcpy(&pixel, bm.getAddr(0,0), sizeof(pixel));

        auto close = [](int x, int y) {
            return x-y < 2
                && y-x < 2;
        };

        if (close(SkColorGetR(pixel), SkColorGetR(color)) &&
            close(SkColorGetG(pixel), SkColorGetG(color)) &&
            close(SkColorGetB(pixel), SkColorGetB(color)) &&
            close(SkColorGetA(pixel), SkColorGetA(color))) {

            MarkGMGood(canvas, 140,40);
        } else {
            MarkGMBad(canvas, 140,40);
        }
    };

    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(color);

        canvas->drawRect({0,0,100,100}, paint);
        grade(50,50);
    }

    canvas->translate(0,100);

    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);

        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(100,100, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
        bm.eraseColor(color);

        canvas->drawImage(bm.asImage(), 0,0, SkSamplingOptions(), &paint);
        grade(50,150);
    }

}
