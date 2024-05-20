/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintOptionsPriv.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

// The default PaintOptions should create a single combination with a solid color shader and
// kSrcOver blending
void empty_test(const KeyContext& keyContext,
                PipelineDataGatherer* gatherer,
                skiatest::Reporter* reporter) {
    PaintOptions paintOptions;

    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 1);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 1);
}

// A PaintOptions will supply a default solid color shader if needed.
void no_shader_option_test(const KeyContext& keyContext,
                           PipelineDataGatherer* gatherer,
                           skiatest::Reporter* reporter) {
    SkBlendMode blendModes[] = { SkBlendMode::kSrcOver };

    PaintOptions paintOptions;
    paintOptions.setBlendModes(blendModes);

    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 1);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 1);
}

// A default kSrcOver blend mode will be supplied if no other blend options are added
void no_blend_mode_option_test(const KeyContext& keyContext,
                               PipelineDataGatherer* gatherer,
                               skiatest::Reporter* reporter) {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Color() });

    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 1);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 1);
}

void big_test(const KeyContext& keyContext,
              PipelineDataGatherer* gatherer,
              skiatest::Reporter* reporter) {
    // paintOptions (248)
    //  |- sweepGrad_0 (2) | blendShader_0 (60)
    //  |                     0: kSrc (1)
    //  |                     1: (dsts) linearGrad_0 (2) | solid_0 (1)
    //  |                     2: (srcs) linearGrad_1 (2) | blendShader_1 (18)
    //  |                                                   0: kDst (1)
    //  |                                                   1: (dsts) radGrad_0 (2) | solid_1 (1)
    //  |                                                   2: (srcs) imageShader_0 (6)
    //  |
    //  |- 4-built-in-blend-modes

    PaintOptions paintOptions;

    // first, shaders. First top-level option (sweepGrad_0)
    sk_sp<PrecompileShader> sweepGrad_0 = PrecompileShaders::SweepGradient();

    std::array<SkBlendMode, 1> blendModes{ SkBlendMode::kSrc };

    std::vector<SkBlendMode> moreBlendModes{ SkBlendMode::kDst };

    // Second top-level option (blendShader_0)
    auto blendShader_0 = PrecompileShaders::Blend(
                                SkSpan<SkBlendMode>(blendModes),                // std::array
                                {                                               // initializer_list
                                    PrecompileShaders::LinearGradient(),
                                    PrecompileShaders::Color()
                                },
                                {
                                    PrecompileShaders::LinearGradient(),
                                    PrecompileShaders::Blend(
                                            SkSpan<SkBlendMode>(moreBlendModes),// std::vector
                                            {
                                                PrecompileShaders::RadialGradient(),
                                                PrecompileShaders::Color()
                                            },
                                            {
                                                PrecompileShaders::Image()
                                            })
                                });

    paintOptions.setShaders({ sweepGrad_0, blendShader_0 });

    SkBlendMode evenMoreBlendModes[] = {
        SkBlendMode::kSrcOver,
        SkBlendMode::kSrc,
        SkBlendMode::kDstOver,
        SkBlendMode::kDst
    };

    // now, blend modes
    paintOptions.setBlendModes(evenMoreBlendModes);                             // c array

    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 248,
                    "Actual # of combinations %d", paintOptions.priv().numCombinations());

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 248);
}

template <typename T>
std::vector<sk_sp<T>> create_runtime_combos(
        skiatest::Reporter* reporter,
        SkRuntimeEffect::Result effectFactory(SkString),
        sk_sp<T> precompileFactory(sk_sp<SkRuntimeEffect>,
                                   SkSpan<const PrecompileChildOptions> childOptions),
        const char* redCode,
        const char* greenCode,
        const char* combineCode) {
    auto [redEffect, error1] = effectFactory(SkString(redCode));
    REPORTER_ASSERT(reporter, redEffect, "%s", error1.c_str());
    auto [greenEffect, error2] = effectFactory(SkString(greenCode));
    REPORTER_ASSERT(reporter, greenEffect, "%s", error2.c_str());
    auto [combineEffect, error3] = effectFactory(SkString(combineCode));
    REPORTER_ASSERT(reporter, combineEffect, "%s", error3.c_str());

    sk_sp<T> red = precompileFactory(redEffect, {});
    REPORTER_ASSERT(reporter, red);

    sk_sp<T> green = precompileFactory(greenEffect, {});
    REPORTER_ASSERT(reporter, green);

    sk_sp<T> combine = precompileFactory(combineEffect, { { red, green }, { green, red } });
    REPORTER_ASSERT(reporter, combine);

    return { combine };
}

void runtime_effect_test(const KeyContext& keyContext,
                         PipelineDataGatherer* gatherer,
                         skiatest::Reporter* reporter) {
    // paintOptions (8)
    //  |- combineShader (2)
    //  |       0: redShader   | greenShader
    //  |       1: greenShader | redShader
    //  |
    //  |- combineColorFilter (2)
    //  |       0: redColorFilter   | greenColorFilter
    //  |       1: greenColorFilter | redColorFilter
    //  |
    //  |- combineBlender (2)
    //  |       0: redBlender   | greenBlender
    //  |       1: greenBlender | redBlender

    PaintOptions paintOptions;

    // shaders
    {
        static const char* kRedS = R"(
            half4 main(vec2 fragcoord) { return half4(.5, 0, 0, .5); }
        )";
        static const char* kGreenS = R"(
            half4 main(vec2 fragcoord) { return half4(0, .5, 0, .5); }
        )";

        static const char* kCombineS = R"(
            uniform shader first;
            uniform shader second;
            half4 main(vec2 fragcoords) {
                return first.eval(fragcoords) + second.eval(fragcoords);
            }
        )";

        std::vector<sk_sp<PrecompileShader>> combinations =
                create_runtime_combos<PrecompileShader>(reporter,
                                                        SkRuntimeEffect::MakeForShader,
                                                        MakePrecompileShader,
                                                        kRedS,
                                                        kGreenS,
                                                        kCombineS);
        paintOptions.setShaders(combinations);
    }

    // color filters
    {
        static const char* kRedCF = R"(
            half4 main(half4 color) { return half4(.5, 0, 0, .5); }
        )";
        static const char* kGreenCF = R"(
            half4 main(half4 color) { return half4(0, .5, 0, .5); }
        )";

        static const char* kCombineCF = R"(
            uniform colorFilter first;
            uniform colorFilter second;
            half4 main(half4 color) { return first.eval(color) + second.eval(color); }
        )";

        std::vector<sk_sp<PrecompileColorFilter>> combinations =
                create_runtime_combos<PrecompileColorFilter>(reporter,
                                                             SkRuntimeEffect::MakeForColorFilter,
                                                             MakePrecompileColorFilter,
                                                             kRedCF,
                                                             kGreenCF,
                                                             kCombineCF);
        paintOptions.setColorFilters(combinations);
    }

    // blenders
    {
        static const char* kRedB = R"(
            half4 main(half4 src, half4 dst) { return half4(.5, 0, 0, .5); }
        )";
        static const char* kGreenB = R"(
            half4 main(half4 src, half4 dst) { return half4(0, .5, 0, .5); }
        )";

        static const char* kCombineB = R"(
            uniform blender first;
            uniform blender second;
            half4 main(half4 src, half4 dst) {
                return first.eval(src, dst) + second.eval(src, dst);
            }
        )";

        std::vector<sk_sp<PrecompileBlender>> combinations =
                create_runtime_combos<PrecompileBlender>(reporter,
                                                         SkRuntimeEffect::MakeForBlender,
                                                         MakePrecompileBlender,
                                                         kRedB,
                                                         kGreenB,
                                                         kCombineB);
        paintOptions.setBlenders(combinations);
    }

    REPORTER_ASSERT(reporter, paintOptions.priv().numCombinations() == 8);

    std::vector<UniquePaintParamsID> precompileIDs;
    paintOptions.priv().buildCombinations(keyContext,
                                          gatherer,
                                          DrawTypeFlags::kNone,
                                          /* withPrimitiveBlender= */ false,
                                          Coverage::kNone,
                                          [&precompileIDs](UniquePaintParamsID id,
                                                           DrawTypeFlags,
                                                           bool /* withPrimitiveBlender */,
                                                           Coverage) {
                                                               precompileIDs.push_back(id);
                                                           });

    SkASSERT(precompileIDs.size() == 8);
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CombinationBuilderTest, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    auto rtEffectDict = std::make_unique<RuntimeEffectDictionary>();

    SkColorInfo ci(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    KeyContext keyContext(context->priv().caps(),
                          dict,
                          rtEffectDict.get(),
                          ci,
                          /* dstTexture= */ nullptr,
                          /* dstOffset= */ {0, 0});

    PipelineDataGatherer gatherer(context->priv().caps(), Layout::kMetal);

    empty_test(keyContext, &gatherer, reporter);
    no_shader_option_test(keyContext, &gatherer, reporter);
    no_blend_mode_option_test(keyContext, &gatherer, reporter);
    big_test(keyContext, &gatherer, reporter);
    runtime_effect_test(keyContext, &gatherer, reporter);
}

#endif // SK_GRAPHITE
