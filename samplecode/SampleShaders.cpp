/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "samplecode/DecodeFile.h"
#include "tools/Resources.h"
#include "samplecode/Sample.h"

namespace {
static sk_sp<SkShader> make_bitmapfade(const SkBitmap& bm) {
    SkPoint pts[2] = {
        {0, 0},
        {0, (float)bm.height()},
    };
    SkColor colors[2] = {
        SkColorSetARGB(255, 0, 0, 0),
        SkColorSetARGB(0,   0, 0, 0),
    };
    return SkShaders::Blend(SkBlendMode::kDstIn,
                            bm.makeShader(),
                            SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                         SkTileMode::kClamp));
}

static sk_sp<SkShader> make_blend_shader() {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(100));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
    auto shaderB = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    return SkShaders::Blend(SkBlendMode::kDstIn, std::move(shaderA), std::move(shaderB));
}

struct ShaderView : public Sample {
    sk_sp<SkShader> fShader;
    sk_sp<SkShader> fShaderFade;
    SkBitmap        fBitmap;

    void onOnceBeforeDraw() override {
        decode_file(GetResourceAsData("images/dog.jpg"), &fBitmap);
        fShader = make_blend_shader();
        fShaderFade = make_bitmapfade(fBitmap);
    }

    SkString name() override { return SkString("Shaders"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawBitmap(fBitmap, 0, 0);
        canvas->translate(20, 120);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect{0, 0, 100, 100}, paint);
        paint.setShader(fShader);
        canvas->drawRect(SkRect{0, 0, 100, 100}, paint);

        canvas->translate(SkIntToScalar(110), 0);

        paint.setShader(nullptr);
        canvas->drawRect(SkRect{0, 0, 120, 80}, paint);
        paint.setShader(fShaderFade);
        canvas->drawRect(SkRect{0, 0, 120, 80}, paint);
    }
};
}  // namespace
DEF_SAMPLE( return new ShaderView(); )
