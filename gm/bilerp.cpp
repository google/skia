/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"

DEF_SIMPLE_GM(bilerp, canvas, 512, 256) {
    // A hard stop gradient with an ugly aliased frontier between blue and yellow.
    SkColor colors[] = {SK_ColorBLUE,SK_ColorBLUE,SK_ColorYELLOW};
    SkScalar pos[] = { 0.0f, 0.2f, 0.2f };

    sk_sp<SkShader> gradient = SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f), 180.0f,
                                                            colors,pos, SK_ARRAY_COUNT(colors),
                                                            SkTileMode::kClamp, 0, nullptr);

    {
        SkPaint paint;
        paint.setShader(gradient);
        canvas->drawRect({0,0,256,256}, paint);
    }
    canvas->translate(256,0);
    {
        SkPaint paint;
        paint.setShader(SkShaders::Bilerp(gradient));
        canvas->drawRect({0,0,256,256}, paint);
    }
}

