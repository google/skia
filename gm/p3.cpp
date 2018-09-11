/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorSpace.h"
#include "SkString.h"

DEF_SIMPLE_GM(p3, canvas, 320, 240) {
    auto dp3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                     SkColorSpace::kDCIP3_D65_Gamut);

    // Draw a P3 red rectangle.
    SkPaint paint;
    paint.setColor4f({1,0,0,1}, dp3.get());
    canvas->drawRect({10,10,70,70}, SkPaint{});
    canvas->drawRect({10,10,70,70}, paint);

    // Read it back in the color space of the canvas, and in P3.
    auto info = SkImageInfo::Make(60,60, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType);

    SkBitmap native_bm,
                 p3_bm;
    native_bm.allocPixels(info.makeColorSpace(canvas->imageInfo().refColorSpace()));
    p3_bm    .allocPixels(info.makeColorSpace(dp3));

    if (canvas->readPixels(native_bm, 10,10) &&
        canvas->readPixels(    p3_bm, 10,10))
    {
        canvas->drawString("drawRect()", 100,20, SkPaint{});

        const float* rgb = (const float*)native_bm.getAddr(10,10);
        canvas->drawString("Native:", 80,40, SkPaint{});
        canvas->drawString(SkStringPrintf("%.2g %.2g %.2g", rgb[0], rgb[1], rgb[2]).c_str(),
                           120,40, SkPaint{});

        canvas->drawString("P3:", 80,60, SkPaint{});
        rgb = (const float*)p3_bm.getAddr(10,10);
        canvas->drawString(SkStringPrintf("%.2g %.2g %.2g", rgb[0], rgb[1], rgb[2]).c_str(),
                           120,60, SkPaint{});
    } else {
        canvas->drawString("can't readPixels() :(", 100,20, SkPaint{});
    }

    // TODO: draw P3 colors more ways
}
