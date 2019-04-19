/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"
#include "SkRandom.h"
#include "ToolUtils.h"
#include "gm.h"

// TODO deprecate imageblur

constexpr int kWidth  = 500;
constexpr int kHeight = 500;

DEF_SIMPLE_GM(imageblur2, canvas, kWidth, kHeight) {
    constexpr float kBlurSigmas[] = { 0.0, 0.3f, 0.5f, 2.0f, 32.0f, 80.0f };
    const char* kTestStrings[] = {
        "The quick`~",
        "brown fox[]",
        "jumped over",
        "the lazy@#$",
        "dog.{}!%^&",
        "*()+=-\\'\"/",
    };
    constexpr int sigmaCount = SK_ARRAY_COUNT(kBlurSigmas);
    constexpr int testStringCount = SK_ARRAY_COUNT(kTestStrings);
    constexpr SkScalar dx = kWidth / sigmaCount;
    constexpr SkScalar dy = kHeight / sigmaCount;
    constexpr SkScalar textSize = 12;

    SkFont font(ToolUtils::create_portable_typeface(), textSize);
    font.setEdging(SkFont::Edging::kAlias);

    for (int x = 0; x < sigmaCount; x++) {
        SkScalar sigmaX = kBlurSigmas[x];
        for (int y = 0; y < sigmaCount; y++) {
            SkScalar sigmaY = kBlurSigmas[y];

            SkPaint paint;
            paint.setImageFilter(SkBlurImageFilter::Make(sigmaX, sigmaY, nullptr));
            canvas->saveLayer(nullptr, &paint);

            SkRandom rand;
            SkPaint textPaint;
            textPaint.setColor(ToolUtils::color_to_565(rand.nextBits(24) | 0xFF000000));
            for (int i = 0; i < testStringCount; i++) {
                canvas->drawString(kTestStrings[i],
                                   SkIntToScalar(x * dx),
                                   SkIntToScalar(y * dy + textSize * i + textSize),
                                   font,
                                   textPaint);
            }
            canvas->restore();
        }
    }
}
