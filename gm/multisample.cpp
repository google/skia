/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"

DEF_SIMPLE_GM(multisample, canvas, 512, 512) {
    // A hard stop gradient with an ugly aliased edge between black and white.
    SkColor colors[] = {SK_ColorBLACK,SK_ColorWHITE};
    SkScalar pos[] = { 0.2f, 0.2f };
    sk_sp<SkShader> gradient = SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f), 180.0f,
                                                            colors,pos, SK_ARRAY_COUNT(colors),
                                                            SkTileMode::kClamp, 0, nullptr);

    auto draw = [&](std::initializer_list<SkPoint>  samples,
                    std::initializer_list<SkScalar> weights) {
        SkPaint paint;
        paint.setShader(SkShaders::Multisample(gradient, samples.begin(), (int)samples.size(),
                                               weights.size() ? weights.begin() : nullptr));
        canvas->drawRect({0,0, 256,256}, paint);
        canvas->save();
            canvas->translate(128,128);
            canvas->scale(32,32);
            for (int i = 0; i < (int)samples.size(); i++) {
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                paint.setAntiAlias(true);
                auto [x,y] = samples.begin()[i];
                SkScalar weight = weights.size() ? weights.begin()[i] : 1.0f/samples.size();

                canvas->drawCircle(x,y, 0.25f*std::sqrt(weight), paint);
            }
        canvas->restore();
    };

    // One sample at the center, the same as drawing without Multisample.
    draw({{0.0f,0.0f}}, {});

    canvas->translate(256,0);

    // 2 samples
    draw({{-0.25f,-0.25f},
          {+0.25f,+0.25f}}, {});

    canvas->translate(-256,+256);

    // 4 samples in somewhat common arrangement
    draw({{ 4/9.0f - 0.5f, 2/9.0f - 0.5f },
          { 8/9.0f - 0.5f, 4/9.0f - 0.5f },
          { 2/9.0f - 0.5f, 6/9.0f - 0.5f },
          { 6/9.0f - 0.5f, 8/9.0f - 0.5f }}, {});

    canvas->translate(256,0);

    // 9 samples in a weighted 3x3 grid
    const float d = 1/3.0f;
    draw({{-d,-d},{ 0,-d},{+d,-d},
          {-d, 0},{ 0, 0},{+d, 0},
          {-d,+d},{ 0,+d},{+d,+d}}, {0.05f,0.10f,0.05f,
                                     0.10f,0.40f,0.10f,
                                     0.05f,0.10f,0.05f});


}

