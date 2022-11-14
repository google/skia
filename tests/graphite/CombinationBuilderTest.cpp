/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/CombinationBuilder.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/Precompile.h"
#include "tests/graphite/CombinationBuilderTestAccess.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

// For an entirely empty combination builder, both solid color shader and kSrcOver options
// will be added
void empty_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    CombinationBuilder builder(dict);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default solid color shader if no other shader
// option is provided
void no_shader_option_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    CombinationBuilder builder(dict);

    builder.addOption(SkBlendMode::kSrcOver);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default kSrcOver blend mode if no other
// options are added
void no_blend_mode_option_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    CombinationBuilder builder(dict);

    builder.addOption(ShaderType::kSolidColor);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

void big_test_new(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {

    // paintOptions
    //  |- sweepGrad_0 | blendShader_0
    //  |                     0: linearGrad_0 | solid_0
    //  |                     1: linearGrad_1 | blendShader_1
    //  |                                            0: radGrad_0 | solid_1
    //  |                                            1: imageShader_0
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

//    context->precompile({paintOptions});
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

void runtime_effect_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    // paintOptions
    //  |- combineShader
    //  |       0: redShader   | greenShader
    //  |       1: greenShader | redShader
    //  |
    //  |- combineColorFilter
    //  |       0: redColorFilter   | greenColorFilter
    //  |       1: greenColorFilter | redColorFilter
    //  |
    //  |- combineBlender
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

    // context->precompile({paintOptions});
}

void big_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    CombinationBuilder builder(dict);

    static constexpr int kMinNumStops = 4;
    static constexpr int kMaxNumStops = 8;

    // The resulting number of combinations are in braces
    //
    // builder {428}
    //  |- {107} sweepGrad_0 {5} | blendShader_0 {102}
    //  |                            0  {6}: linearGrad_0 {5} | solid_0 {1}
    //  |                            1 {17}: linearGrad_1 {5} | blendShader_1 {12}
    //  |                                                         0 {6}: radGrad_0 {5} | solid_1 {1}
    //  |                                                         1 {2}: imageShader_0 {2}
    //  |
    //  |- {4} 4-built-in-blend-modes {4}

    // first, shaders. First top-level option (sweepGrad_0)
    [[maybe_unused]] auto sweepGrad_0 = builder.addOption(ShaderType::kSweepGradient,
                                                          kMinNumStops, kMaxNumStops);

    // Second top-level option (blendShader_0)
    auto blendShader_0 = builder.addOption(ShaderType::kPorterDuffBlendShader);

    // first child slot of blendShader_0
    {
        // first option (linearGrad_0)
        [[maybe_unused]] auto linearGrad_0 = blendShader_0.addChildOption(
                0, ShaderType::kLinearGradient,
                kMinNumStops, kMaxNumStops);

        // second option (solid_0)
        [[maybe_unused]] auto solid_0 = blendShader_0.addChildOption(0,
                                                                     ShaderType::kSolidColor);
    }

    // second child slot of blendShader_0
    {
        // first option (linearGrad_1)
        {
            [[maybe_unused]] auto linearGrad_1 = blendShader_0.addChildOption(
                    1, ShaderType::kLinearGradient,
                    kMinNumStops, kMaxNumStops);
        }

        // second option (blendShader_1)
        {
            auto blendShader_1 = blendShader_0.addChildOption(1, ShaderType::kBlendShader);

            // nested: first child slot of blendShader_1
            {
                // first option (radialGrad_0)
                [[maybe_unused]] auto radialGrad_0 = blendShader_1.addChildOption(
                        0, ShaderType::kRadialGradient,
                        kMinNumStops, kMaxNumStops);

                // second option (solid_1)
                [[maybe_unused]] auto solid_1 = blendShader_1.addChildOption(
                        0, ShaderType::kSolidColor);
            }

            // nested: second child slot of blendShader_1
            {
                TileModePair tilingOptions[] = {
                        { SkTileMode::kRepeat, SkTileMode::kRepeat },
                        { SkTileMode::kClamp,  SkTileMode::kClamp }
                };

                // only option (imageShader_0)
                [[maybe_unused]] auto imageShader_0 = blendShader_1.addChildOption(
                        1, ShaderType::kImage, tilingOptions);
            }
        }
    }

    // now, blend modes
    builder.addOption(SkBlendMode::kSrcOver);
    builder.addOption(SkBlendMode::kSrc);
    builder.addOption(SkBlendMode::kDstOver);
    builder.addOption(SkBlendMode::kDst);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 428);
}

#ifdef SK_DEBUG
void epoch_test(ShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    CombinationBuilder builder(dict);

    // Check that epochs are updated upon builder reset
    {
        CombinationOption solid_0 = builder.addOption(ShaderType::kSolidColor);

        int optionEpoch = CombinationBuilderTestAccess::Epoch(solid_0);
        REPORTER_ASSERT(reporter, optionEpoch == CombinationBuilderTestAccess::Epoch(builder));

        builder.reset();

        REPORTER_ASSERT(reporter, optionEpoch != CombinationBuilderTestAccess::Epoch(builder));
    }
}
#endif

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CombinationBuilderTest, reporter, context) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    big_test_new(dict, reporter);
    runtime_effect_test(dict, reporter);

    empty_test(dict, reporter);
    no_shader_option_test(dict, reporter);
    no_blend_mode_option_test(dict, reporter);
    big_test(dict, reporter);
    SkDEBUGCODE(epoch_test(dict, reporter));
}

#endif // SK_GRAPHITE_ENABLED
