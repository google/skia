/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace {
using namespace skgpu::graphite;

std::tuple<SkPaint, int> create_paint(ShaderCombo::ShaderType shaderType,
                                      SkTileMode tm,
                                      SkBlendMode bm) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numTextures = 0;
    switch (shaderType) {
        case ShaderCombo::ShaderType::kNone:
            SkDEBUGFAIL("kNone cannot be represented as an SkPaint");
            break;
        case ShaderCombo::ShaderType::kSolidColor:
            break;
        case ShaderCombo::ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            break;
        case ShaderCombo::ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            break;
        case ShaderCombo::ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            break;
        case ShaderCombo::ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            break;
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(bm);
    return { p, numTextures };
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UniformTest, reporter, context) {
    using namespace skgpu::graphite;

    auto recorder = context->makeRecorder();
    SkKeyContext keyContext(recorder.get(), {});
    auto dict = keyContext.dict();
    auto tCache = recorder->priv().textureDataCache();

    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);
    SkPipelineDataGatherer gatherer(Layout::kMetal);

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
                auto [ p, expectedNumTextures ] = create_paint(s, tm, bm);

                auto [ uniqueID1, uIndex, tIndex] = ExtractPaintData(recorder.get(), &gatherer,
                                                                     &builder, {}, PaintParams(p));

                SkUniquePaintParamsID uniqueID2 = CreateKey(keyContext, &builder, s, tm, bm);
                // ExtractPaintData and CreateKey agree
                REPORTER_ASSERT(reporter, uniqueID1 == uniqueID2);

                // TODO: This isn't particularly useful until we add image shaders to the
                // pre-compilation set.
                {
                    const SkTextureDataBlock* textureData = tCache->lookup(tIndex);
                    int actualNumTextures = textureData ? textureData->numTextures() : 0;

                    REPORTER_ASSERT(reporter, expectedNumTextures == actualNumTextures);
                }

                {
                    // TODO: check the blendInfo here too
                }
            }
        }
    }
}
