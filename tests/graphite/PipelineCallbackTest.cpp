/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/TestOptions.h"

#if defined(SK_ENABLE_PRECOMPILE)
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#endif

#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

bool draw(Context* context) {
    SkPaint paint;
    const SkPoint pts[] = {{0, 0}, {64, 64}};
    const SkColor4f colors[] = {SkColors::kWhite, SkColors::kBlack};
    paint.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}}));

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto ii = SkImageInfo::Make({ 64, 64 }, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii, skgpu::Mipmapped::kNo);
    if (!surface) {
        return false;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->drawRect({0, 0, 64, 64}, paint);

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

void deprecated_callback(void* context, sk_sp<SkData> serializedKey) {
    std::vector<sk_sp<SkData>>* data = static_cast<std::vector<sk_sp<SkData>>*>(context);

    data->push_back(std::move(serializedKey));
}

void pipeline_normal_callback(void* context,
                              ContextOptions::PipelineCacheOp op,
                              const std::string& label,
                              uint32_t uniqueKeyHash,
                              bool fromPrecompile,
                              sk_sp<SkData> serializedKey) {
    std::vector<sk_sp<SkData>>* data = static_cast<std::vector<sk_sp<SkData>>*>(context);

    SkASSERT(!label.empty());
    SkASSERT(!fromPrecompile);
    SkASSERT(serializedKey);
    data->push_back(std::move(serializedKey));
}

void run_normal_test(skiatest::Reporter* reporter,
                     const TestOptions& options,
                     skgpu::ContextType type,
                     const std::vector<sk_sp<SkData>>& data) {
    ContextFactory workaroundFactory(options);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(type);

    REPORTER_ASSERT(reporter, draw(ctxInfo.fContext));

    REPORTER_ASSERT(reporter, !data.empty());    // some Pipeline should've been reported
}

#if defined(SK_ENABLE_PRECOMPILE)
void pipeline_precompile_callback(void* context,
                                  ContextOptions::PipelineCacheOp op,
                                  const std::string& label,
                                  uint32_t uniqueKeyHash,
                                  bool fromPrecompile,
                                  sk_sp<SkData> serializedKey) {
    std::vector<sk_sp<SkData>>* data = static_cast<std::vector<sk_sp<SkData>>*>(context);

    SkASSERT(!label.empty());
    SkASSERT(fromPrecompile);
    if (op == ContextOptions::PipelineCacheOp::kAddingPipeline) {
        SkASSERT(serializedKey);
        data->push_back(std::move(serializedKey));
    } else {
        // Some PaintOption combinatorics produce the same paint key (e.g. opacity variations that
        // lead to no net change). In these cases, we get cache hits on subsequent callbacks.
        SkASSERT(op == skgpu::graphite::ContextOptions::PipelineCacheOp::kPipelineFound);
    }
}

void precompile_linear_gradient(PrecompileContext* precompileContext) {
    PaintOptions paintOptions;
    paintOptions.setShaders({{ PrecompileShaders::LinearGradient(
                                      PrecompileShaders::GradientShaderFlags::kSmall) }});
    constexpr DrawTypeFlags kRectAndRRect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticRRect);

    Precompile(precompileContext, paintOptions, kRectAndRRect,
               {{ { DepthStencilFlags::kDepth, kRGBA_8888_SkColorType} }});
}

void run_precompile_test(skiatest::Reporter* reporter,
                         const TestOptions& options,
                         skgpu::ContextType type,
                         const std::vector<sk_sp<SkData>>& data) {
    ContextFactory workaroundFactory(options);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(type);

    Context* newContext = ctxInfo.fContext;

    std::unique_ptr<PrecompileContext> precompileContext = newContext->makePrecompileContext();

    precompile_linear_gradient(precompileContext.get());

    REPORTER_ASSERT(reporter, !data.empty());    // some Pipeline should've been reported
}
#endif

} // anonymous namespace


// Smoke test for the deprecated ContextOptions::PipelineCallback for normal Pipelines.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineCallbackTest_Deprecated_Normal,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kApiLevel_202604) {
    std::vector<sk_sp<SkData>> data;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCallback = deprecated_callback;

    run_normal_test(reporter, newOptions, origTestContext->contextType(), data);
}

// Smoke test for the ContextOptions::PipelineCachingCallback for normal Pipelines.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineCallbackTest_Normal,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kApiLevel_202604) {
    std::vector<sk_sp<SkData>> data;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_normal_callback;

    run_normal_test(reporter, newOptions, origTestContext->contextType(), data);
}

#if defined(SK_ENABLE_PRECOMPILE)
// Smoke test for the deprecated ContextOptions::PipelineCallback for Precompile Pipelines.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineCallbackTest_Deprecated_Precompile,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kApiLevel_202604) {
    std::vector<sk_sp<SkData>> data;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCallback = deprecated_callback;

    run_precompile_test(reporter, newOptions, origTestContext->contextType(), data);
}

// Smoke test for the ContextOptions::PipelineCachingCallback for Precompile Pipelines.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineCallbackTest_Precompile,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kApiLevel_202604) {
    std::vector<sk_sp<SkData>> data;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_precompile_callback;

    run_precompile_test(reporter, newOptions, origTestContext->contextType(), data);
}

struct PipelineCheckData {
    sk_sp<SkData> fData;
    uint32_t fUniqueHash = 0;
    bool fSeen = false;
    bool fMatched = false;
};

namespace {

void pipeline_checking_callback(void* context,
                                ContextOptions::PipelineCacheOp op,
                                const std::string& label,
                                uint32_t uniqueKeyHash,
                                bool fromPrecompile,
                                sk_sp<SkData> serializedKey) {
    std::vector<PipelineCheckData> *data = static_cast<std::vector<PipelineCheckData>*>(context);

    SkASSERT(!label.empty());
    SkASSERT(fromPrecompile);

    if (op == ContextOptions::PipelineCacheOp::kAddingPipeline) {
        SkASSERT(serializedKey);

        for (PipelineCheckData& d : *data) {
            if (*d.fData == *serializedKey) {
                d.fSeen = true;
                d.fMatched = (d.fUniqueHash == uniqueKeyHash);
            }
        }
    } else {
        SkASSERT(op == skgpu::graphite::ContextOptions::PipelineCacheOp::kPipelineFound);
        [[maybe_unused]] bool hashMatch = false;
        for (PipelineCheckData& d : *data) {
            // On found pipelines, serializedKey is null so we can only search for the hash
            if (d.fUniqueHash == uniqueKeyHash) {
                hashMatch = true;
                break;
            }
        }
        SkASSERT(hashMatch);
    }
}

void run_checking_test(skiatest::Reporter* reporter,
                         const TestOptions& options,
                         skgpu::ContextType type,
                         const std::vector<sk_sp<SkData>>& origKeys,
                         std::vector<PipelineCheckData>* verificationData) {
    ContextFactory workaroundFactory(options);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(type);

    Context* newContext = ctxInfo.fContext;

    std::unique_ptr<PrecompileContext> precompileContext = newContext->makePrecompileContext();

    // First get the label and uniqueHash for each serialized key in 'origKeys' and fill
    // up 'verificationData'
    for (const sk_sp<SkData>& key : origKeys) {
        uint32_t uniqueHash;
        precompileContext->getPipelineLabel(key, &uniqueHash);

        verificationData->push_back({key, uniqueHash, false, false});
    }

    // Next, precompile the Pipelines like usual but the uniqueHashes will be checked in
    // pipeline_checking_callback.
    precompile_linear_gradient(precompileContext.get());

    // Verify that all the uniqueHashs matched
    for (const PipelineCheckData& d : *verificationData) {
        REPORTER_ASSERT(reporter, d.fSeen && d.fMatched);
    }
}

} // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineCallbackTest_UniqueHash,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNextRelease) {
    std::vector<sk_sp<SkData>> origKeys;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &origKeys;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_precompile_callback;

    // This creates a new Context, precompiles some Pipelines and puts the serialized keys
    // in origKeys
    run_precompile_test(reporter, newOptions, origTestContext->contextType(), origKeys);

    // The prior Context gets destroyed here so all we have are the serialized keys in 'origKeys'

    std::vector<PipelineCheckData> verificationData;

    newOptions.fContextOptions.fPipelineCallbackContext = &verificationData;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_checking_callback;

    // This calls getPipelineLabel on all the serialized keys in 'data' then precompiles
    // them to verify that the uniqueHashs match.
    run_checking_test(reporter, newOptions, origTestContext->contextType(), origKeys,
                      &verificationData);
}
#endif
