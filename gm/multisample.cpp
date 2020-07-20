/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"

DEF_SIMPLE_GM(multisample, canvas, 512,512) {
    // A hard stop gradient with an ugly aliased edge between black and white.
    SkColor colors[] = {SK_ColorBLACK,SK_ColorWHITE};
    SkScalar pos[] = { 0.2f, 0.2f };
    sk_sp<SkShader> gradient = SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f), 180.0f,
                                                            colors,pos, SK_ARRAY_COUNT(colors),
                                                            SkTileMode::kClamp, 0, nullptr);

    auto draw = [&](std::initializer_list<SkPoint>  samples,
                    std::initializer_list<SkScalar> weights) {

        const int N = (int)samples.size();
        SkString src = SkStringPrintf(R"(
        in shader child;
        uniform float3 samples[%d];

        void main(float2 p, inout half4 color) {
            color = half4(0);
            for (int i = 0; i < %d; i++) {
                float2 offset = samples[i].xy;
                float  weight = samples[i].z;

                color += half(weight) * sample(child, p+offset);
            }
        }
        )", N,N);

        auto [effect,err] = SkRuntimeEffect::Make(src);
        SkASSERT(effect);

        std::vector<float> u;
        for (int i = 0; i < N; i++) {
            u.push_back(samples.begin()[i].x());
            u.push_back(samples.begin()[i].y());
            u.push_back(weights.size() ? weights.begin()[i]
                                       : 1.0f/N);
        }

        SkPaint paint;
        paint.setShader(effect->makeShader(SkData::MakeWithCopy(u.data(), N*3*sizeof(float)),
                                           &gradient, 1,
                                           /*localM=*/nullptr, /*isOpaque=*/false));
        canvas->drawRect({0,0, 256,256}, paint);

        canvas->save();
            canvas->translate(128,128);
            canvas->scale(32,32);
            for (int i = 0; i < N; i++) {
                float x = u[3*i+0],
                      y = u[3*i+1],
                      w = u[3*i+2];

                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                paint.setAntiAlias(true);
                canvas->drawCircle(x,y, 0.25f*std::sqrt(w), paint);
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
