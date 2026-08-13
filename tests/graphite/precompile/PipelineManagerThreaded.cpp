/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkExecutor.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/TestOptions.h"

using namespace::skgpu::graphite;
using namespace skiatest::graphite;

const RenderPassProperties kRGBA_1_D {
        DepthStencilFlags::kDepth,
        kRGBA_8888_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

namespace {

void precompile_an_arc_draw(skiatest::Reporter* reporter,
                            PrecompileContext* precompileContext) {
    PaintOptions paintOptions;
    paintOptions.addBlendMode(SkBlendMode::kSrcOver);

    Precompile(precompileContext,
               paintOptions,
               DrawTypeFlags::kCircularArc,
               { kRGBA_1_D });
}

void draw_an_arc(skiatest::Reporter* reporter, Context* context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkImageInfo ii = SkImageInfo::Make(16, 16,
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);


    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder.get(), ii, skgpu::Mipmapped::kNo);
    SkCanvas* canvas = surf->getCanvas();

    SkArc arc = SkArc::Make(SkRect::MakeWH(16, 16), 0, 270, SkArc::Type::kWedge);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrcOver);

    canvas->drawArc(arc, paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        ERRORF(reporter, "Failed to make recording");
        return;
    }

    context->insertRecording({ recording.get() });
}

void run_test(skiatest::Reporter* reporter, GraphiteTestContext* testContext,
              const TestOptions& origOptions, bool allowThreads, bool precompileFirst) {
    std::unique_ptr<SkExecutor> executor;
    if (allowThreads) {
        // Ensure the threaded PipelineManager will be used
        executor = SkExecutor::MakeMultiListFIFOThreadPool(/* numWorkLists= */ 2,
                                                           /* threads= */ 1,
                                                           /* allowBorrowing= */ false);
    }

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fExecutor = executor.get();

    {
        std::unique_ptr<Context> context = testContext->makeContext(newOptions);
        std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();

        if (precompileFirst) {
            precompile_an_arc_draw(reporter, precompileContext.get());
        }

        draw_an_arc(reporter, context.get());

        if (!precompileFirst) {
            precompile_an_arc_draw(reporter, precompileContext.get());
        }

        testContext->syncedSubmit(context.get());

        const GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();
        const PipelineManager::Stats& mgrStats =
                context->priv().sharedContext()->pipelineManager()->getStats();

        REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 1);
        REPORTER_ASSERT(reporter, mgrStats.fNumTasksCreated == 1);
    }
}

} // anonymous namespace

// The goal here is to test out the PipelineManager's de-duplication of Pipeline
// creation tasks for both threaded and non-threaded PipelineManagers.
// The tests will request the same Pipeline via both Precompilation and drawing -
// altering which one comes first.


// These two tests forcibly create a threaded PipelineManager and then request the
// same Pipeline.
// If precompilation is first the draw should use the precompiled Pipeline.
// If the draw is first the precompilation should just find it.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerThreadedTest_1,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(reporter,  origTestContext, origOptions,
             /* allowThreads= */ true, /* precompileFirst=*/ true);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerThreadedTest_2,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(reporter, origTestContext, origOptions,
             /* allowThreads= */ true, /* precompileFirst=*/ false);
}

// The next two tests are the same as above but deliberately create a non-threaded
// PipelineManager.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerThreadedTest_3,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(reporter, origTestContext, origOptions,
             /* allowThreads= */ false, /* precompileFirst=*/ true);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerThreadedTest_4,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(reporter, origTestContext, origOptions,
             /* allowThreads= */ false, /* precompileFirst=*/ false);
}
#endif // SK_GRAPHITE
