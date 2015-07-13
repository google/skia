/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "gm.h"

// This GM reproduces what I think are overflow bugs in the CPU implementations of a
// couple xfermodes.  I've marked non-obvious keys to reproducing the bug // Essential!
DEF_SIMPLE_GM(buggy_blend_modes, canvas, 800, 200) {
    const auto tiling = SkShader::kClamp_TileMode;
    const auto flags = SkGradientShader::kInterpolateColorsInPremul_Flag;  // Essential!

    SkAutoTUnref<SkShader> cyanH, magentaV;
    {
        SkPoint    pts[] = { {0,0}, {200, 0} };
        SkColor colors[] = { 0x00000000, 0xFF00FFFF };
        cyanH.reset(
                SkGradientShader::CreateLinear(pts, colors, nullptr, 2, tiling, flags, nullptr));
    }
    {
        SkPoint    pts[] = { {0,0}, {0, 200} };
        SkColor colors[] = { 0x00000000, 0xFFFF00FF };
        magentaV.reset(
                SkGradientShader::CreateLinear(pts, colors, nullptr, 2, tiling, flags, nullptr));
    }

    SkXfermode::Mode modes[] = {
        SkXfermode::kDarken_Mode,       // Looks ok?
        SkXfermode::kHardLight_Mode,    // Definitely wrong.
        SkXfermode::kLighten_Mode,      // Definitely wrong.
        SkXfermode::kOverlay_Mode,      // Same code as kHardLight_Mode.
    };

    canvas->clear(SK_ColorWHITE);
    for (auto mode : modes) {
        canvas->saveLayer(nullptr, nullptr);  // Essential!
            SkPaint h, v;
            h.setShader(cyanH);
            v.setShader(magentaV);
            v.setXfermodeMode(mode);
            canvas->drawRect(SkRect::MakeWH(200,200), h);
            canvas->drawRect(SkRect::MakeWH(200,200), v);
        canvas->restore();

        canvas->translate(200, 0);
    }
}
