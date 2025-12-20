/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurfaceProps.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteMemoryPipelineStorage.h"
#include "tools/graphite/TestOptions.h"

using namespace skiatest::graphite;

namespace {

bool draw(GraphiteTestContext* origTestContext,
          const TestOptions& origOptions,
          sk_gpu_test::GraphiteMemoryPipelineStorage* memoryPipelineStorage,
          bool withGradient) {

    // Rebuild the Context with a PersistentCache
    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fPersistentPipelineStorage = memoryPipelineStorage;

    skiatest::graphite::ContextFactory workaroundFactory(newOptions);
    ContextInfo ctxInfo = workaroundFactory.getContextInfo(origTestContext->contextType());

    skgpu::graphite::Context* newContext = ctxInfo.fContext;
    GraphiteTestContext* newTestContext = ctxInfo.fTestContext;

    std::unique_ptr<skgpu::graphite::Recorder> recorder = newContext->makeRecorder();
    if (!recorder) {
        return false;
    }

    {
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
        auto ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii,
                                                            skgpu::Mipmapped::kNo, &props);
        if (!surface) {
            return false;
        }

        SkPaint paint;
        if (withGradient) {
            const SkPoint pts[] = {{0, 0}, {64, 64}};
            const SkColor4f colors[] = {SkColors::kWhite, SkColors::kBlack};
            paint.setShader(SkShaders::LinearGradient(pts, {{colors, {}, SkTileMode::kClamp}, {}}));
        }
        surface->getCanvas()->drawRRect(SkRRect::MakeOval({16, 16, 48, 48 }), paint);
    }

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!newContext->insertRecording(info)) {
        return false;
    }
    newTestContext->syncedSubmit(newContext);

    newContext->syncPipelineData();

    return true;
}

} // anonymous namespace

namespace skgpu::graphite {

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PersistentPipelineStorageTest,
                                           skiatest::IsVulkanContextType,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNextRelease) {

    sk_gpu_test::GraphiteMemoryPipelineStorage memoryPipelineStorage;

    REPORTER_ASSERT(reporter, draw(origTestContext, origOptions, &memoryPipelineStorage,
                                   /* withGradient= */ false));

    // On the first draw there shouldn't be anything to load but we should store the new pipelines.
    REPORTER_ASSERT(reporter, memoryPipelineStorage.numLoads() == 0,
                    "actual: %d", memoryPipelineStorage.numLoads());
    REPORTER_ASSERT(reporter, memoryPipelineStorage.numStores() == 1,
                    "actual: %d", memoryPipelineStorage.numStores());

    memoryPipelineStorage.resetCacheStats();

    REPORTER_ASSERT(reporter, draw(origTestContext, origOptions, &memoryPipelineStorage,
                                   /* withGradient= */ true));

    // On the second draw we should be able to load the prior pipelines but still need to store
    // the new ones.
    REPORTER_ASSERT(reporter, memoryPipelineStorage.numLoads() == 1,
                    "actual: %d", memoryPipelineStorage.numLoads());
    REPORTER_ASSERT(reporter, memoryPipelineStorage.numStores() == 1,
                    "actual: %d", memoryPipelineStorage.numStores());
}

}  // namespace skgpu::graphite
