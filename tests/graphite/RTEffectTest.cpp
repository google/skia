/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(Shader_FindOrCreateSnippetForRuntimeEffect, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "half4 main(float2 coords) {"
            "return half4(coords.xy01);"
        "}"
    ));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Verify that it can be looked up and its name is 'RuntimeEffect'. (The name isn't meaningful,
    // but this is an easy way to verify that we didn't get an unrelated snippet.)
    const ShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);
    REPORTER_ASSERT(reporter, std::string_view(snippet->fName) == "RuntimeEffect");

    // If we pass the same effect again, we should get the same snippet ID as before.
    int foundSnippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, foundSnippetID == snippetID);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ColorFilter_FindOrCreateSnippetForRuntimeEffect,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
                "half4 main(half4 color) {"
                    "return color.gbra;"
                "}"
            ));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Verify that it can be looked up and its name is 'RuntimeEffect'. (The name isn't meaningful,
    // but this is an easy way to verify that we didn't get an unrelated snippet.)
    const ShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);
    REPORTER_ASSERT(reporter, std::string_view(snippet->fName) == "RuntimeEffect");

    // If we pass the same effect again, we should get the same snippet ID as before.
    int foundSnippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, foundSnippetID == snippetID);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ShaderUniforms_FindOrCreateSnippetForRuntimeEffect,
                                   reporter, context, CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform float3x3 MyFloat3x3Uniform;"
        "uniform int4 MyInt4ArrayUniform[1];"
        "uniform half2 MyHalf2ArrayUniform[99];"
        "half4 main(float2 coords) {"
            "return half4(coords.xy01);"
        "}"
    ));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Delete the test effect.
    testEffect = nullptr;

    // Verify that it can be looked up by its snippet ID.
    const ShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);

    // The uniform span should match our expectations even though the runtime effect was deleted.
    REPORTER_ASSERT(reporter, snippet->fUniforms.size() == 3);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[0].name()) == "MyFloat3x3Uniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].type() == SkSLType::kFloat3x3);
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].count() == 0);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[1].name()) == "MyInt4ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].type() == SkSLType::kInt4);
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].count() == 1);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[2].name()) == "MyHalf2ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].type() == SkSLType::kHalf2);
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].count() == 99);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ColorFilterUniforms_FindOrCreateSnippetForRuntimeEffect,
                                   reporter, context, CtsEnforcement::kNextRelease) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    std::unique_ptr<SkRuntimeEffect> testEffect(SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
                "uniform float3x3 MyFloat3x3Uniform;"
                "uniform int4 MyInt4ArrayUniform[1];"
                "uniform half2 MyHalf2ArrayUniform[99];"
                "half4 main(half4 color) {"
                    "return color.gbra;"
                "}"
            ));

    // Create a new runtime-effect snippet.
    int snippetID = dict->findOrCreateRuntimeEffectSnippet(testEffect.get());
    REPORTER_ASSERT(reporter, snippetID >= kBuiltInCodeSnippetIDCount);

    // Delete the test effect.
    testEffect = nullptr;

    // Verify that it can be looked up by its snippet ID.
    const ShaderSnippet* snippet = dict->getEntry(snippetID);
    REPORTER_ASSERT(reporter, snippet);

    // The uniform span should match our expectations even though the runtime effect was deleted.
    REPORTER_ASSERT(reporter, snippet->fUniforms.size() == 3);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[0].name()) == "MyFloat3x3Uniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].type() == SkSLType::kFloat3x3);
    REPORTER_ASSERT(reporter, snippet->fUniforms[0].count() == 0);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[1].name()) == "MyInt4ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].type() == SkSLType::kInt4);
    REPORTER_ASSERT(reporter, snippet->fUniforms[1].count() == 1);

    REPORTER_ASSERT(reporter,
                    std::string_view(snippet->fUniforms[2].name()) == "MyHalf2ArrayUniform");
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].type() == SkSLType::kHalf2);
    REPORTER_ASSERT(reporter, snippet->fUniforms[2].count() == 99);
}
