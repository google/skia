/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCombinationBuilder.h"

#include "tests/graphite/CombinationBuilderTestAccess.h"

using namespace::skgpu::graphite;

namespace {

// For an entirely empty combination builder, both solid color shader and kSrcOver options
// will be added
void empty_test(Context *context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default solid color shader if no other shader
// option is provided
void no_shader_option_test(Context *context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    builder.addOption(SkBlendMode::kSrcOver);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default kSrcOver blend mode if no other
// options are added
void no_blend_mode_option_test(Context *context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    builder.addOption(SkShaderType::kSolidColor);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

void big_test(Context *context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

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
void epoch_test(Context *context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

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
    empty_test(context, reporter);
    no_shader_option_test(context, reporter);
    no_blend_mode_option_test(context, reporter);
    big_test(context, reporter);
    SkDEBUGCODE(epoch_test(context, reporter));
}
