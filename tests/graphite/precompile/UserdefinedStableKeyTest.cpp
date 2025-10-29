/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/graphite/PipelineCallbackHandler.h"
#include "tools/graphite/UniqueKeyUtils.h"
#include "tools/graphite/precompile/PrecompileEffectFactories.h"

using namespace::skgpu::graphite;
using namespace skiatest::graphite;
using namespace skiatools::graphite;

namespace {

std::pair<SkPaint, PaintOptions> create_paint_and_options(bool addBlenders) {
    SkPaint paint;
    PaintOptions paintOptions;

    auto [shader, shaderOption] = PrecompileFactories::CreateAnnulusRuntimeShader();

    paint.setShader(std::move(shader));
    paintOptions.setShaders({ std::move(shaderOption) });

    auto [colorFilter, colorFilterOption] = PrecompileFactories::CreateComboRuntimeColorFilter();

    paint.setColorFilter(std::move(colorFilter));
    paintOptions.setColorFilters({ std::move(colorFilterOption) });

    if (addBlenders) {
        auto [blender, blenderOption] = PrecompileFactories::CreateComboRuntimeBlender();

        paint.setBlender(std::move(blender));
        paintOptions.setBlenders({ std::move(blenderOption) });
    }

    return { paint, paintOptions };
}

bool draw_with_normal_api(skgpu::graphite::Context* context,
                          skgpu::graphite::Recorder* recorder,
                          const SkPaint& paint) {
    auto ii = SkImageInfo::Make({ 256, 256 }, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, ii, skgpu::Mipmapped::kNo);
    if (!surface) {
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->drawRect({0, 0, 100, 100}, paint);

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }

    if (!context->insertRecording({ recording.get() })) {
        return false;
    }

    if (!context->submit(skgpu::graphite::SyncToCpu::kYes)) {
        return false;
    }

    return true;
}

void fetch_keys_and_reset(skiatest::Reporter* reporter,
                          PipelineCallBackHandler* handler,
                          PrecompileContext* precompileContext,
                          std::vector<skgpu::UniqueKey>* uniqueKeys,
                          std::vector<sk_sp<SkData>>* serializedKeys,
                          bool reset) {
    UniqueKeyUtils::FetchUniqueKeys(precompileContext, uniqueKeys);
    handler->retrieveKeys(serializedKeys);

    if (reset) {
        GlobalCache* globalCache = precompileContext->priv().globalCache();

        globalCache->resetGraphicsPipelines();
        REPORTER_ASSERT(reporter, globalCache->numGraphicsPipelines() == 0);
        handler->reset();
    }
}

// Get the existing keys, reset, and try recreating them all w/ the serialized pipeline keys
void reset_and_recreate_pipelines_with_serialized_keys(
            skiatest::Reporter* reporter,
            Context* context,
            Recorder* recorder,
            PrecompileContext* precompileContext,
            PipelineCallBackHandler* handler) {
    GlobalCache* globalCache = precompileContext->priv().globalCache();
    ShaderCodeDictionary* shaderCodeDictionary = context->priv().shaderCodeDictionary();

    auto [paint, _] = create_paint_and_options(/* addBlenders= */ true);

    draw_with_normal_api(context, recorder, paint);

    // None of the user-defined stable runtime effects should've been transmuted to not-stable
    REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());

    std::vector<skgpu::UniqueKey> origKeys;
    std::vector<sk_sp<SkData>> androidStyleKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &origKeys, &androidStyleKeys, /* reset= */ true);

    // Given 'draw_with_normal_api' we expect one serialized key - full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys.size() == 1);

    // Use the serialized keys to regenerate the Pipelines
    for (sk_sp<SkData>& d : androidStyleKeys) {
        bool result = precompileContext->precompile(d);
        SkAssertResult(result);
    }

    // None of the user-defined stable runtime effects should've been transmuted to not-stable
    REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());

    std::vector<skgpu::UniqueKey> recreatedKeys;
    std::vector<sk_sp<SkData>> recreatedAndroidStyleKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &recreatedKeys, &recreatedAndroidStyleKeys, /* reset= */ false);

    REPORTER_ASSERT(reporter, recreatedKeys.size() == 1);
    REPORTER_ASSERT(reporter, origKeys[0] == recreatedKeys[0]);

    REPORTER_ASSERT(reporter, recreatedAndroidStyleKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys[0]->equals(recreatedAndroidStyleKeys[0].get()));

    int numBeforeSecondDraw = globalCache->numGraphicsPipelines();

    draw_with_normal_api(context, recorder, paint);

    // None of the user-defined stable runtime effects should've been transmuted to not-stable
    REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());

    // Re-drawing shouldn't create any new pipelines
    REPORTER_ASSERT(reporter, numBeforeSecondDraw == globalCache->numGraphicsPipelines(),
                    "%d != %d", numBeforeSecondDraw, globalCache->numGraphicsPipelines());
}

// Get the existing keys, reset, and then try recreating them all using the normal precompile API
void reset_and_recreate_pipelines_with_normal_precompile_api(
            skiatest::Reporter* reporter,
            Context* context,
            Recorder* recorder,
            PrecompileContext* precompileContext,
            PipelineCallBackHandler* handler) {
    // We don't attach runtime blenders to the SkPaint and PaintOptions in this case bc that will
    // force a dest read and complicate the normal-pipeline/Precompile-pipeline
    // comparison on Native Mac and Vulkan. This is bc, for those platforms, Precompile skips some
    // LoadOp combinations which don't matter for those platforms (please see 'numLoadOps' in
    // Precompile) but are serialized (for simplicity) in the serialized pipeline keys.
    auto [paint, paintOptions] = create_paint_and_options(/* addBlenders= */ false);

    draw_with_normal_api(context, recorder, paint);

    std::vector<skgpu::UniqueKey> origKeys;
    std::vector<sk_sp<SkData>> androidStyleKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &origKeys, &androidStyleKeys, /* reset= */ true);

    // Given 'draw_with_normal_api' we expect one serialized key - full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys.size() == 1);

    RenderPassProperties renderPassProps;
    renderPassProps.fDSFlags = DepthStencilFlags::kDepth;

    Precompile(precompileContext,
               paintOptions,
               DrawTypeFlags::kSimpleShape,
               { renderPassProps });

    std::vector<skgpu::UniqueKey> recreatedKeys;
    std::vector<sk_sp<SkData>> recreatedAndroidStyleKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &recreatedKeys, &recreatedAndroidStyleKeys, /* reset= */ true);

    // The normal precompile API will overgenerate, so we need to search for a match
    {
        bool foundIt = false;
        for (const skgpu::UniqueKey& k : recreatedKeys) {
            if (origKeys[0] == k) {
                foundIt = true;
                break;
            }
        }
        REPORTER_ASSERT(reporter, foundIt);
    }
    {
        bool foundIt = false;
        for (const sk_sp<SkData>& k : recreatedAndroidStyleKeys) {
            if (androidStyleKeys[0]->equals(k.get())) {
                foundIt = true;
                break;
            }
        }
        REPORTER_ASSERT(reporter, foundIt);
    }
}

// This helper creates a defective user-defined known runtime effect pair:
//     the blessed shader will have a user-defined stable key
//     the cursed shader has the same SkSL as the blessed one but no stable key
std::pair<sk_sp<SkShader>, sk_sp<SkShader>> make_defective_annulus_shader_pair() {
    SkRuntimeEffect* blessed = PrecompileFactories::GetAnnulusShaderEffect();

    sk_sp<SkRuntimeEffect> cursed(SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                      PrecompileFactories::GetAnnulusShaderCode()));

    SkASSERT(blessed != cursed.get());

    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    return { blessed->makeShader(uniforms, /* children= */ {}),
             cursed->makeShader(uniforms, /* children= */ {}) };
}

// Draw once with a registered runtime effect, reset, and then re-draw w/ an un-registered
// runtime effect that uses the same SkSL.
void test_sksl_reuse(skiatest::Reporter* reporter,
                     Context* context,
                     Recorder* recorder,
                     PrecompileContext* precompileContext,
                     PipelineCallBackHandler* handler) {
    auto [blessedShader, cursedShader] = make_defective_annulus_shader_pair();

    SkASSERT(blessedShader != cursedShader);

    // The blessed paint is the static one from the PrecompileFactories which has been
    // registered as a user-defined known runtime effect.
    SkPaint blessedPaint;
    blessedPaint.setShader(std::move(blessedShader));

    // The cursed paint uses the same SkSL as the blessed version but uses a wholly separate
    // runtime effect (which is not registered).
    SkPaint cursedPaint;
    cursedPaint.setShader(std::move(cursedShader));

    draw_with_normal_api(context, recorder, blessedPaint);

    std::vector<skgpu::UniqueKey> origKeys;
    std::vector<sk_sp<SkData>> serializedKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &origKeys, &serializedKeys, /* reset= */ true);

    // Given 'draw_with_normal_api' we expect one serialized key - full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, serializedKeys.size() == 1);

    draw_with_normal_api(context, recorder, cursedPaint);

    std::vector<skgpu::UniqueKey> recreatedKeys;
    std::vector<sk_sp<SkData>> recreatedSerializedKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &recreatedKeys, &recreatedSerializedKeys, /* reset= */ true);

    REPORTER_ASSERT(reporter, recreatedKeys.size() == 1);
    REPORTER_ASSERT(reporter, origKeys[0] == recreatedKeys[0]);

    // The un-registered runtime effect should've been mapped back to the registered one
    // and successfully serialized.
    REPORTER_ASSERT(reporter, recreatedSerializedKeys.size() == 1);
    REPORTER_ASSERT(reporter, serializedKeys[0]->equals(recreatedSerializedKeys[0].get()));
}

void test_get_pipeline_label_api(skiatest::Reporter* reporter,
                                 Context* context,
                                 Recorder* recorder,
                                 PrecompileContext* precompileContext,
                                 PipelineCallBackHandler* handler) {
    auto [paint, paintOptions] = create_paint_and_options(/* addBlenders= */ true);

    draw_with_normal_api(context, recorder, paint);

    std::vector<skgpu::UniqueKey> origKeys;
    std::vector<sk_sp<SkData>> androidStyleKeys;

    fetch_keys_and_reset(reporter, handler, precompileContext,
                         &origKeys, &androidStyleKeys, /* reset= */ true);

    // Given 'draw_with_normal_api' we expect one serialized key - full of user-defined stable keys
    REPORTER_ASSERT(reporter, origKeys.size() == 1);
    REPORTER_ASSERT(reporter, androidStyleKeys.size() == 1);

    std::string label = precompileContext->getPipelineLabel(androidStyleKeys[0]);

    REPORTER_ASSERT(reporter, std::string::npos != label.find("AnnulusShader"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("SrcBlender"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("DstBlender"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("ComboBlender"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("DoubleColorFilter"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("ComboColorFilter"));
    // We withheld the HalfColorFilter name to test the default name case
    REPORTER_ASSERT(reporter, std::string::npos == label.find("HalfColorFilter"));
    REPORTER_ASSERT(reporter, std::string::npos != label.find("UserDefinedKnownRuntimeEffect"));
}


} // anonymous namespace

// This test adds some user-defined stably-keyed runtime effects and then verifies that
// everything behaves as expected. For the purposes of this test, "behaves as expected" means:
//    1) the user-defined stably keyed runtime effects appear as such in the ShaderCodeDictionary
//    2) the user-defined stable keys appear in the serialized Pipeline
//       keys (i.e., from the PipelineCallBackHandler)
//    3) said keys correctly (re)generate the desired pipelines
//    4) the normal (non-serialized-key) Precompile API generates the same keys.
//    5) if the blessed stable runtime-effects aren't used, the free-range runtime-effects are
//       mapped back to the stable runtime-effects (clients really shouldn't do this though!)
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCachingCallback = PipelineCallBackHandler::CallBack;

    // We're going to also use all these runtime effects via the normal API
    // (c.f. create_paint_and_options)
    static const int kNumUserDefinedStableKeys = 7;
    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[kNumUserDefinedStableKeys] = {
        sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),

        sk_ref_sp(PrecompileFactories::GetSrcBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetDstBlenderEffect()),
        sk_ref_sp(PrecompileFactories::GetComboBlenderEffect()),

        sk_ref_sp(PrecompileFactories::GetDoubleColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetHalfColorFilterEffect()),
        sk_ref_sp(PrecompileFactories::GetComboColorFilterEffect()),
    };

    for (const sk_sp<SkRuntimeEffect>& e: userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        SkRuntimeEffectPriv::ResetStableKey(e.get());
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(origTestContext->contextType());

    Context* newContext = ctxInfo.fContext;
    std::unique_ptr<PrecompileContext> precompileContext = newContext->makePrecompileContext();
    GlobalCache* globalCache = precompileContext->priv().globalCache();
    ShaderCodeDictionary* shaderCodeDictionary = newContext->priv().shaderCodeDictionary();
    std::unique_ptr<Recorder> recorder =
            newContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    REPORTER_ASSERT(reporter, !globalCache->numGraphicsPipelines());
    // The next two lines check #1 above
    REPORTER_ASSERT(reporter, !shaderCodeDictionary->numUserDefinedRuntimeEffects());
    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() ==
                              kNumUserDefinedStableKeys);

    // This verifies #2 and #3 above
    reset_and_recreate_pipelines_with_serialized_keys(reporter,
                                                      newContext,
                                                      recorder.get(),
                                                      precompileContext.get(),
                                                      pipelineHandler.get());

    globalCache->resetGraphicsPipelines();
    pipelineHandler->reset();

    // This tests out #4 above
    reset_and_recreate_pipelines_with_normal_precompile_api(reporter,
                                                            newContext,
                                                            recorder.get(),
                                                            precompileContext.get(),
                                                            pipelineHandler.get());

    globalCache->resetGraphicsPipelines();
    pipelineHandler->reset();

    // This tests out #5 above
    test_sksl_reuse(reporter,
                    newContext,
                    recorder.get(),
                    precompileContext.get(),
                    pipelineHandler.get());

    globalCache->resetGraphicsPipelines();
    pipelineHandler->reset();

    // Extra little test to check on the getPipelineLabel API
    test_get_pipeline_label_api(reporter,
                                newContext,
                                recorder.get(),
                                precompileContext.get(),
                                pipelineHandler.get());
}

// Test that the ShaderCodeDictionary can deduplicate the user-defined known runtime effect list
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Duplicates,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCachingCallback = PipelineCallBackHandler::CallBack;

    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[] = {
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
    };

    for (const sk_sp<SkRuntimeEffect>& e: userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        SkRuntimeEffectPriv::ResetStableKey(e.get());
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() == 1);
}

// Test that the ShaderCodeDictionary can handle nullptrs in the
// user-defined known runtime effect list
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Nullptrs,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCachingCallback = PipelineCallBackHandler::CallBack;

    sk_sp<SkRuntimeEffect> userDefinedKnownRuntimeEffects[] = {
            sk_ref_sp(PrecompileFactories::GetAnnulusShaderEffect()),
            nullptr,
            sk_ref_sp(PrecompileFactories::GetSrcBlenderEffect()),
    };

    for (const sk_sp<SkRuntimeEffect>& e : userDefinedKnownRuntimeEffects) {
        // The PrecompileFactories runtime effects are static so prior runs may have already
        // set their StableKeys. Reset the StableKeys so all our expectations/asserts will be met.
        if (e) {
            SkRuntimeEffectPriv::ResetStableKey(e.get());
        }
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() == 2);
}

// Test that the ShaderCodeDictionary can handle excess user-defined known runtime effects
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(UserDefinedStableKeyTest_Overflow,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {

    std::unique_ptr<PipelineCallBackHandler> pipelineHandler(new PipelineCallBackHandler);

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = pipelineHandler.get();
    newOptions.fContextOptions.fPipelineCachingCallback = PipelineCallBackHandler::CallBack;

    std::vector<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects;

    for (int i = 0; i < 2*SkKnownRuntimeEffects::kUserDefinedKnownRuntimeEffectsReservedCnt; ++i) {
        SkString sksl;
        sksl.printf("half4 main(float2 xy) { return half4(%d/255.0, %d/255.0, %d/255.0, 1.0); }",
                    i, i, i);
        userDefinedKnownRuntimeEffects.push_back(SkRuntimeEffect::MakeForShader(sksl).effect);
    }

    newOptions.fContextOptions.fUserDefinedKnownRuntimeEffects = { userDefinedKnownRuntimeEffects };

    ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(testContext->contextType());

    ShaderCodeDictionary* shaderCodeDictionary = ctxInfo.fContext->priv().shaderCodeDictionary();

    REPORTER_ASSERT(reporter, shaderCodeDictionary->numUserDefinedKnownRuntimeEffects() ==
                              SkKnownRuntimeEffects::kUserDefinedKnownRuntimeEffectsReservedCnt);
}

#endif // SK_GRAPHITE
