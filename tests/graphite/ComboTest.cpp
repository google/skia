/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkPaintParamsKey.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "src/core/SkPaintPriv.h"
#include "tests/Test.h"

namespace {

sk_sp<SkShader> make_image_shader(int imageWidth, int imageHeight,
                                  SkTileMode xTileMode, SkTileMode yTileMode,
                                  SkColor color) {
    auto surface = SkSurface::MakeRasterN32Premul(imageWidth, imageHeight);
    SkCanvas *canvas = surface->getCanvas();
    canvas->clear(color);
    return surface->makeImageSnapshot()->makeShader(xTileMode, yTileMode, SkSamplingOptions());
}

sk_sp<SkShader> make_linear_gradient_shader(SkTileMode tileMode) {
    SkPoint pts[2];
    SkColor colors[2] = {SK_ColorRED, SK_ColorBLUE};

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, tileMode);
}

sk_sp<SkShader> make_blend_shader(sk_sp<SkShader> shaderA,
                                  sk_sp<SkShader> shaderB,
                                  SkBlendMode mode) {
    return SkShaders::Blend(mode, std::move(shaderA), std::move(shaderB));
}

void dump_keys(SkShaderCodeDictionary *dict, const SkPaint &paint) {
#ifdef SK_DEBUG
    auto keys = SkPaintPriv::ToKeys(paint, dict, SkBackend::kGanesh);

    for (auto k: keys) {
        // TODO: we need a better way to assess that key creation succeeded
        k.dump();
    }
#endif
}

} // anonymous namespace

DEF_GRAPHITE_TEST(ComboTest, r) {
    SkShaderCodeDictionary dict;

    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kLighten);
        dump_keys(&dict, paint);
    }

    {
        SkPaint paint;
        paint.setShader(make_image_shader(16, 16, SkTileMode::kClamp,
                                          SkTileMode::kRepeat, SK_ColorRED));
        dump_keys(&dict, paint);
    }

    {
        SkPaint paint;
        paint.setShader(make_linear_gradient_shader(SkTileMode::kClamp));
        dump_keys(&dict, paint);
    }

    {
        SkPaint paint;
        auto shaderA = make_image_shader(16, 16, SkTileMode::kDecal,
                                         SkTileMode::kRepeat, SK_ColorBLUE);
        auto shaderB = make_linear_gradient_shader(SkTileMode::kClamp);
        paint.setShader(make_blend_shader(std::move(shaderA), std::move(shaderB),
                                          SkBlendMode::kDstIn));
        dump_keys(&dict, paint);
    }
}
