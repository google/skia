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
    // A hard stop gradient with an ugly aliased frontier between blue and yellow.
    SkColor colors[] = {SK_ColorBLACK,SK_ColorBLACK,SK_ColorWHITE};
    SkScalar pos[] = { 0.0f, 0.2f, 0.2f };

    sk_sp<SkShader> gradient = SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f), 180.0f,
                                                            colors,pos, SK_ARRAY_COUNT(colors),
                                                            SkTileMode::kClamp, 0, nullptr);

    auto draw = [&](std::initializer_list<SkPoint> samples) {
        SkPaint paint;
        paint.setShader(SkShaders::Multisample(gradient, samples.begin(), (int)samples.size()));
        canvas->drawRect({0,0, 256,256}, paint);
        canvas->save();
            const float scale = 32.0f;
            canvas->translate(128,128);
            canvas->scale(scale,scale);
            for (auto [x,y] : samples) {
                const float off = 4/scale;
                SkPaint p;
                p.setColor(SK_ColorWHITE);
                canvas->drawRect({x-off,y-off,
                                  x+off,y+off}, p);
            }
        canvas->restore();
    };

    draw({{0.0f,0.0f}});

    canvas->translate(256,0);

    draw({{-0.25f,-0.25f},
          {+0.25f,+0.25f}});

    canvas->translate(-256,+256);

    draw({{ 4/9.0f - 0.5f, 2/9.0f - 0.5f },
          { 8/9.0f - 0.5f, 4/9.0f - 0.5f },
          { 2/9.0f - 0.5f, 6/9.0f - 0.5f },
          { 6/9.0f - 0.5f, 8/9.0f - 0.5f }});

    canvas->translate(256,0);

    const float d = 1/3.0f;
    draw({{-d,-d},{ 0,-d},{+d,-d},
          {-d, 0},{ 0, 0},{+d, 0},
          {-d,+d},{ 0,+d},{+d,+d}});
}

