/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCombinationBuilder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/ContextPriv.h"

using namespace skgpu::graphite;

namespace {

// combines two child blenders using 'blendFrac'
//    1 uniform ("blendFrac")
//    2 children ("a", "b")
// TODO: add a helper function
sk_sp<SkRuntimeEffect> get_combo_effect() {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float blendFrac;
            uniform blender a;
            uniform blender b;
            half4 main(half4 src, half4 dst) {
                return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));
            }
        )"));

    return result.effect;
}

// returns opaque red w/ the red value determined by 'redColor'
//    1 uniform ("redColor)
sk_sp<SkRuntimeEffect> get_red_effect() {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float redColor;
            half4 main(half4 src, half4 dst) {
                return half4(redColor, 0, 0, 1);
            }
        )"));

    return result.effect;
}

// returns opaque blue w/ the blue value determined by 'blueColor'
//    1 uniform ("blueColor)
sk_sp<SkRuntimeEffect> get_blue_effect() {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(SkString(R"(
            uniform float blueColor;
            half4 main(half4 src, half4 dst) {
                return half4(0, 0, blueColor, 1);
            }
        )"));

    return result.effect;
}

static sk_sp<SkBlender> get_blender(sk_sp<SkRuntimeEffect> comboEffect,
                                    sk_sp<SkRuntimeEffect> redEffect,
                                    sk_sp<SkRuntimeEffect> blueEffect) {
    sk_sp<SkBlender> redBlender;
    {
        SkRuntimeBlendBuilder builder(redEffect);
        builder.uniform("redColor") = 1.0f;
        redBlender = builder.makeBlender();
    }

    sk_sp<SkBlender> blueBlender;
    {
        SkRuntimeBlendBuilder builder(blueEffect);
        builder.uniform("blueColor") = 1.0f;
        blueBlender = builder.makeBlender();
    }

    sk_sp<SkBlender> linearBlender;
    {
        SkRuntimeBlendBuilder builder(comboEffect);
        builder.uniform("blendFrac") = 0.5f;
        builder.child("a") = redBlender;
        builder.child("b") = blueBlender;
        linearBlender = builder.makeBlender();
    }

    return linearBlender;
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(RTEffectTest, reporter, context) {
    SkShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    sk_sp<SkRuntimeEffect> comboEffect = get_combo_effect();
    sk_sp<SkRuntimeEffect> redEffect = get_red_effect();
    sk_sp<SkRuntimeEffect> blueEffect = get_blue_effect();

    SkBlenderID comboId = context->addUserDefinedBlender(comboEffect);
    SkASSERT(comboId.isValid());

    SkBlenderID redId = context->addUserDefinedBlender(redEffect);
    SkASSERT(redId.isValid());

    SkBlenderID blueId = context->addUserDefinedBlender(blueEffect);
    SkASSERT(blueId.isValid());

    auto comboEntry = dict->getEntry(comboId);
    REPORTER_ASSERT(reporter, comboEntry);

    auto redEntry = dict->getEntry(redId);
    REPORTER_ASSERT(reporter, redEntry);

    auto blueEntry = dict->getEntry(blueId);
    REPORTER_ASSERT(reporter, blueEntry);

    // TODO:
    //   add runtime effect shaders and color filters
    //   check that the shader snippet has the expected properties: uniforms, children, functions

    sk_sp<SkBlender> blender = get_blender(comboEffect, redEffect, blueEffect);

    // TODO:
    //   check that the uniforms can be extracted from 'blender' correctly
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(FindOrCreateSnippetForRuntimeEffectWorks, reporter, context) {
    SkShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
    R"(
        half4 main(float2 coords) {
            return half4(coords.xy01);
        }
    )"));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Verify that it can be looked up and its name is 'RuntimeEffect'. (The name isn't meaningful,
    // but this is an easy way to verify that we didn't get an unrelated snippet.)
    const SkShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);
    REPORTER_ASSERT(reporter, std::string_view(snippet->fName) == "RuntimeEffect");

    // If we pass the same effect again, we should get the same snippet ID as before.
    int foundSnippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, foundSnippetID == snippetID);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UniformsInFindOrCreateSnippetForRuntimeEffect, reporter, context) {
    SkShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
    R"(
        uniform float3x3 MyFloat3x3Uniform;
        uniform int4 MyInt4ArrayUniform[1];
        uniform half2 MyHalf2ArrayUniform[99];
        half4 main(float2 coords) {
            return half4(coords.xy01);
        }
    )"));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Delete the test effect.
    testEffect = nullptr;

    // Verify that it can be looked up by its snippet ID.
    const SkShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);

    // The uniform span should match our expectations even though the runtime effect was deleted.
    REPORTER_ASSERT(reporter, snippet->fUniforms.size() == 4);

    REPORTER_ASSERT(reporter, std::string_view(snippet->fUniforms[0].name()) == "localMatrix");
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].type() == SkSLType::kFloat4x4);
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].count() == 0);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[1].name()) == "MyFloat3x3Uniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].type() == SkSLType::kFloat3x3);
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].count() == 0);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[2].name()) == "MyInt4ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].type() == SkSLType::kInt4);
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].count() == 1);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[3].name()) == "MyHalf2ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[3].type() == SkSLType::kHalf2);
    REPORTER_ASSERT(reporter, snippet->fUniforms[3].count() == 99);

}
