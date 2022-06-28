/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkCombinationBuilder.h"
#include "src/core/SkFactoryFunctions.h"
#include "src/core/SkPrecompile.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tests/graphite/CombinationBuilderTestAccess.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

// For an entirely empty combination builder, both solid color shader and kSrcOver options
// will be added
void empty_test(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(dict);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default solid color shader if no other shader
// option is provided
void no_shader_option_test(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(dict);

    builder.addOption(SkBlendMode::kSrcOver);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default kSrcOver blend mode if no other
// options are added
void no_blend_mode_option_test(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(dict);

    builder.addOption(SkShaderType::kSolidColor);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

void big_test_new(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {

    // paintOptions
    //  |- sweepGrad_0 | blendShader_0
    //  |                     0: linearGrad_0 | solid_0
    //  |                     1: linearGrad_1 | blendShader_1
    //  |                                            0: radGrad_0 | solid_1
    //  |                                            1: imageShader_0
    //  |
    //  |- 4-built-in-blend-modes

    SkPaintOptions paintOptions;

    // first, shaders. First top-level option (sweepGrad_0)
    sk_sp<SkPrecompileShader> sweepGrad_0 = SkPrecompileShaders::SweepGradient();

    std::array<SkBlendMode, 1> blendModes{ SkBlendMode::kSrc };

    std::vector<SkBlendMode> moreBlendModes{ SkBlendMode::kDst };

    // Second top-level option (blendShader_0)
    auto blendShader_0 = SkPrecompileShaders::Blend(
                                SkSpan<SkBlendMode>(blendModes),                // std::array
                                {                                               // initializer_list
                                    SkPrecompileShaders::LinearGradient(),
                                    SkPrecompileShaders::Color()
                                },
                                {
                                    SkPrecompileShaders::LinearGradient(),
                                    SkPrecompileShaders::Blend(
                                            SkSpan<SkBlendMode>(moreBlendModes),// std::vector
                                            {
                                                SkPrecompileShaders::RadialGradient(),
                                                 SkPrecompileShaders::Color()
                                            },
                                            {
                                                  SkPrecompileShaders::Image()
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

void big_test(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(dict);

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
    [[maybe_unused]] auto sweepGrad_0 = builder.addOption(SkShaderType::kSweepGradient,
                                                          kMinNumStops, kMaxNumStops);

    // Second top-level option (blendShader_0)
    auto blendShader_0 = builder.addOption(SkShaderType::kBlendShader);

    // first child slot of blendShader_0
    {
        // first option (linearGrad_0)
        [[maybe_unused]] auto linearGrad_0 = blendShader_0.addChildOption(
                0, SkShaderType::kLinearGradient,
                kMinNumStops, kMaxNumStops);

        // second option (solid_0)
        [[maybe_unused]] auto solid_0 = blendShader_0.addChildOption(0,
                                                                     SkShaderType::kSolidColor);
    }

    // second child slot of blendShader_0
    {
        // first option (linearGrad_1)
        {
            [[maybe_unused]] auto linearGrad_1 = blendShader_0.addChildOption(
                    1, SkShaderType::kLinearGradient,
                    kMinNumStops, kMaxNumStops);
        }

        // second option (blendShader_1)
        {
            auto blendShader_1 = blendShader_0.addChildOption(1, SkShaderType::kBlendShader);

            // nested: first child slot of blendShader_1
            {
                // first option (radialGrad_0)
                [[maybe_unused]] auto radialGrad_0 = blendShader_1.addChildOption(
                        0, SkShaderType::kRadialGradient,
                        kMinNumStops, kMaxNumStops);

                // second option (solid_1)
                [[maybe_unused]] auto solid_1 = blendShader_1.addChildOption(
                        0, SkShaderType::kSolidColor);
            }

            // nested: second child slot of blendShader_1
            {
                SkTileModePair tilingOptions[] = {
                        { SkTileMode::kRepeat, SkTileMode::kRepeat },
                        { SkTileMode::kClamp,  SkTileMode::kClamp }
                };

                // only option (imageShader_0)
                [[maybe_unused]] auto imageShader_0 = blendShader_1.addChildOption(
                        1, SkShaderType::kImage, tilingOptions);
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
void epoch_test(SkShaderCodeDictionary* dict, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(dict);

    // Check that epochs are updated upon builder reset
    {
        SkCombinationOption solid_0 = builder.addOption(SkShaderType::kSolidColor);

        int optionEpoch = CombinationBuilderTestAccess::Epoch(solid_0);
        REPORTER_ASSERT(reporter, optionEpoch == CombinationBuilderTestAccess::Epoch(builder));

        builder.reset();

        REPORTER_ASSERT(reporter, optionEpoch != CombinationBuilderTestAccess::Epoch(builder));
    }
}
#endif

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(CombinationBuilderTest, reporter, context) {
    SkShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    big_test_new(dict, reporter);

    empty_test(dict, reporter);
    no_shader_option_test(dict, reporter);
    no_blend_mode_option_test(dict, reporter);
    big_test(dict, reporter);
    SkDEBUGCODE(epoch_test(dict, reporter));
}

#endif // SK_GRAPHITE_ENABLED
