/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkBitmap.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/CombinationBuilder.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/shaders/SkImageShader.h"
#include "tests/graphite/CombinationBuilderTestAccess.h"

using namespace skgpu::graphite;

namespace {

sk_sp<SkImage> make_image(Recorder* recorder) {
    SkImageInfo info = SkImageInfo::Make(32, 32, SkColorType::kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorBLACK);

    sk_sp<SkImage> img = bitmap.asImage();

    return img->makeTextureImage(recorder);
}

std::tuple<SkPaint, int> create_paint(Recorder* recorder,
                                      ShaderType shaderType,
                                      SkTileMode tm,
                                      SkBlendMode bm) {
    SkPoint pts[2] = {{-100, -100},
                      {100,  100}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    SkScalar offsets[2] = {0.0f, 1.0f};

    sk_sp<SkShader> s;
    int numTextures = 0;
    switch (shaderType) {
        case ShaderType::kSolidColor:
            s = SkShaders::Color(SK_ColorYELLOW);
            break;
        case ShaderType::kLinearGradient:
            s = SkGradientShader::MakeLinear(pts, colors, offsets, 2, tm);
            break;
        case ShaderType::kRadialGradient:
            s = SkGradientShader::MakeRadial({0, 0}, 100, colors, offsets, 2, tm);
            break;
        case ShaderType::kSweepGradient:
            s = SkGradientShader::MakeSweep(0, 0, colors, offsets, 2, tm,
                                            0, 359, 0, nullptr);
            break;
        case ShaderType::kConicalGradient:
            s = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                      {-100, -100}, 100,
                                                      colors, offsets, 2, tm);
            break;
        case ShaderType::kLocalMatrix: {
            s = SkShaders::Color(SK_ColorYELLOW);
            s = s->makeWithLocalMatrix(SkMatrix::Scale(1.5f, 2.0f));
            break;
        }
        case ShaderType::kImage: {
            ++numTextures;
            s = SkImageShader::Make(make_image(recorder), tm, tm,
                                    SkSamplingOptions(), nullptr);
            break;
        }
        case ShaderType::kPorterDuffBlendShader: {
            sk_sp<SkShader> src = SkShaders::Color(SK_ColorGREEN);
            sk_sp<SkShader> dst = SkShaders::Color(SK_ColorLTGRAY);
            s = SkShaders::Blend(SkBlendMode::kSrcOver, std::move(dst), std::move(src));
            break;
        }
        case ShaderType::kBlendShader: {
            sk_sp<SkShader> src = SkShaders::Color(SK_ColorGREEN);
            sk_sp<SkShader> dst = SkShaders::Color(SK_ColorLTGRAY);
            s = SkShaders::Blend(SkBlendMode::kColorDodge, std::move(dst), std::move(src));
            break;
        }
    }
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setShader(std::move(s));
    p.setBlendMode(bm);
    return { p, numTextures };
}

SkUniquePaintParamsID create_key(Context* context,
                                 SkPaintParamsKeyBuilder* keyBuilder,
                                 ShaderType shaderType,
                                 SkTileMode tileMode,
                                 SkBlendMode blendMode) {
    SkShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    CombinationBuilder combinationBuilder(context->priv().shaderCodeDictionary());

    switch (shaderType) {
        case ShaderType::kSolidColor:
            combinationBuilder.addOption(shaderType);
            break;
        case ShaderType::kLinearGradient:         [[fallthrough]];
        case ShaderType::kRadialGradient:         [[fallthrough]];
        case ShaderType::kSweepGradient:          [[fallthrough]];
        case ShaderType::kConicalGradient:
            combinationBuilder.addOption(shaderType, 2, 2);
            break;
        case ShaderType::kLocalMatrix: {
            CombinationOption option = combinationBuilder.addOption(shaderType);
            option.addChildOption(0, ShaderType::kSolidColor);
        } break;
        case ShaderType::kImage: {
            TileModePair tilingOptions[] = { { tileMode,  tileMode } };

            combinationBuilder.addOption(shaderType, tilingOptions);
        } break;
        case ShaderType::kPorterDuffBlendShader: {
            CombinationOption option = combinationBuilder.addOption(shaderType);
            option.addChildOption(0, ShaderType::kSolidColor);
            option.addChildOption(1, ShaderType::kSolidColor);
        } break;
        case ShaderType::kBlendShader: {
            CombinationOption option = combinationBuilder.addOption(shaderType);
            option.addChildOption(0, ShaderType::kSolidColor);
            option.addChildOption(1, ShaderType::kSolidColor);
        } break;
    }

    combinationBuilder.addOption(blendMode);

    std::vector<SkUniquePaintParamsID> uniqueIDs = CombinationBuilderTestAccess::BuildCombinations(
            dict, &combinationBuilder);

    SkASSERT(uniqueIDs.size() == 1);
    return uniqueIDs[0];
}

} // anonymous namespace

// This is intended to be a smoke test for the agreement between the two ways of creating a
// PaintParamsKey:
//    via ExtractPaintData (i.e., from an SkPaint)
//    and via CreateKey (i.e., via the pre-compilation system)
// TODO: we could turn this into a fuzzer
// TODO: rather than what we're doing here we should:
//   create a PaintCombinations object for a single combination
//   traverse it to create an SkPaint (or create it in parallel)
//   call Context::precompile and, somehow, get the created SkPaintParamsKey
//           - maybe via a testing only callback on SkShaderCodeDictionary::findOrCreate
//   draw w/ the SkPaint and, again, somehow, intercept the created SkPaintParamsKey
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(PaintParamsKeyTest, reporter, context) {
    auto recorder = context->makeRecorder();
    SkKeyContext keyContext(recorder.get());
    auto dict = keyContext.dict();

    SkPaintParamsKeyBuilder builder(dict);
    SkPipelineDataGatherer gatherer(Layout::kMetal);

    for (auto s : { ShaderType::kSolidColor,
                    ShaderType::kLinearGradient,
                    ShaderType::kRadialGradient,
                    ShaderType::kSweepGradient,
                    ShaderType::kConicalGradient,
                    ShaderType::kLocalMatrix,
                    ShaderType::kImage,
                    ShaderType::kPorterDuffBlendShader,
                    ShaderType::kBlendShader }) {
        for (auto tm: { SkTileMode::kClamp,
                        SkTileMode::kRepeat,
                        SkTileMode::kMirror,
                        SkTileMode::kDecal }) {
            if (s == ShaderType::kSolidColor || s == ShaderType::kLocalMatrix ||
                s == ShaderType::kPorterDuffBlendShader || s == ShaderType::kBlendShader) {
                if (tm != SkTileMode::kClamp) {
                    continue;  // the TileMode doesn't matter for these cases
                }
            }

            // TODO: test out a runtime SkBlender here
            for (auto bm : { SkBlendMode::kSrc, SkBlendMode::kSrcOver }) {
                auto [ p, expectedNumTextures ] = create_paint(recorder.get(), s, tm, bm);

                auto [ uniqueID1, uData, tData] = ExtractPaintData(recorder.get(), &gatherer,
                                                                   &builder, PaintParams(p));

                SkUniquePaintParamsID uniqueID2 = create_key(context, &builder, s, tm, bm);
                // ExtractPaintData and CreateKey agree
                REPORTER_ASSERT(reporter, uniqueID1 == uniqueID2);

                {
                    int actualNumTextures = tData ? tData->numTextures() : 0;

                    REPORTER_ASSERT(reporter, expectedNumTextures == actualNumTextures);
                }

                {
                    // TODO: check the blendInfo here too
                }
            }
        }
    }
}

#endif // SK_GRAPHITE_ENABLED
