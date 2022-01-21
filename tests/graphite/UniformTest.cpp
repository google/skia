/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/PaintParams.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkUniformData.h"

namespace {

std::tuple<SkPaint, int> create_paint(skgpu::ShaderCombo::ShaderType shaderType,
                                      SkTileMode tm,
                                      SkBlendMode bm) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numUniforms = 0;
    switch (shaderType) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            SkDEBUGFAIL("kNone cannot be represented as an SkPaint");
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            numUniforms += 1;
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            numUniforms += 6;
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            numUniforms += 6;
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(bm);
    return { p, numUniforms };
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UniformTest, reporter, context) {
    using namespace skgpu;

    // Intentionally does not include ShaderType::kNone, which represents no fragment shading stage
    // and is thus not relevant to uniform extraction/caching.
    for (auto s : { ShaderCombo::ShaderType::kSolidColor,
                    ShaderCombo::ShaderType::kLinearGradient,
                    ShaderCombo::ShaderType::kRadialGradient,
                    ShaderCombo::ShaderType::kSweepGradient,
                    ShaderCombo::ShaderType::kConicalGradient }) {
        for (auto tm: { SkTileMode::kClamp,
                        SkTileMode::kRepeat,
                        SkTileMode::kMirror,
                        SkTileMode::kDecal }) {
            if (s == ShaderCombo::ShaderType::kSolidColor) {
                tm = SkTileMode::kClamp;  // the TileMode doesn't matter for this case
            }

            for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
                SkPaintParamsKey expected = CreateKey(SkBackend::kGraphite, s, tm, bm);

                auto [ p, expectedNumUniforms ] = create_paint(s, tm, bm);
                auto [ actualID, ud] = ExtractPaintData(context, PaintParams(p));

                auto entry = context->priv().shaderCodeDictionary()->lookup(actualID);

                REPORTER_ASSERT(reporter, expected == entry->paintParamsKey());
                REPORTER_ASSERT(reporter, expectedNumUniforms == ud->count());
                for (int i = 0; i < ud->count(); ++i) {
                    REPORTER_ASSERT(reporter, ud->offset(i) >= 0 && ud->offset(i) < ud->dataSize());
                }
            }
        }
    }
}
