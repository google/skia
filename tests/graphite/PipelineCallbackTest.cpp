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
#include "include/effects/SkGradientShader.h"
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
    const SkColor colors[] = {SK_ColorWHITE, SK_ColorBLACK};
    paint.setShader(SkGradientShader::MakeLinear(pts,
                                                 colors,
                                                 nullptr,
                                                 /* count= */ 2,
                                                 SkTileMode::kClamp));

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
    SkASSERT(serializedKey);
    data->push_back(std::move(serializedKey));
}

void run_precompile_test(skiatest::Reporter* reporter,
                         const TestOptions& options,
                         skgpu::ContextType type,
                         const std::vector<sk_sp<SkData>>& data) {
    ContextFactory workaroundFactory(options);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(type);

    Context* newContext = ctxInfo.fContext;

    std::unique_ptr<PrecompileContext> precompileContext = newContext->makePrecompileContext();

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(
                                      PrecompileShaders::GradientShaderFlags::kSmall) });

    Precompile(precompileContext.get(), paintOptions,
               DrawTypeFlags::kNonAAFillRect,
               { { DepthStencilFlags::kDepth, kRGBA_8888_SkColorType} });

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
                                           CtsEnforcement::kNextRelease) {
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
                                           CtsEnforcement::kNextRelease) {
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
                                           CtsEnforcement::kNextRelease) {
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
                                           CtsEnforcement::kNextRelease) {
    std::vector<sk_sp<SkData>> data;

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPipelineCallbackContext = &data;
    newOptions.fContextOptions.fPipelineCachingCallback = pipeline_precompile_callback;

    run_precompile_test(reporter, newOptions, origTestContext->contextType(), data);
}
#endif
