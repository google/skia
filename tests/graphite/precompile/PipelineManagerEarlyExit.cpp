/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkExecutor.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

const skgpu::graphite::RenderPassProperties kR_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kAlpha_8_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};

const skgpu::graphite::RenderPassProperties kRGBA_4_DS {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

const skgpu::graphite::RenderPassProperties kRGBA_4_DS_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ true
};

namespace {

void add_precompilation(PrecompileContext* precompileContext) {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(
                                      PrecompileShaders::GradientShaderFlags::kAll) });

    Precompile(precompileContext,
               paintOptions,
               static_cast<DrawTypeFlags>(DrawTypeFlags::kSimpleShape |
                                          DrawTypeFlags::kNonSimpleShape),
               { kR_1_D, kRGBA_1_D, kRGBA_1_D_SRGB, kRGBA_4_DS, kRGBA_4_DS_SRGB });
}

void run_test(GraphiteTestContext* testContext, const TestOptions& origOptions,
              bool allowThreads, bool keepPrecompileContextAlive) {
    std::unique_ptr<SkExecutor> executor;
    if (allowThreads) {
        // Ensure the threaded PipelineManager will be used
        executor = SkExecutor::MakeMultiListFIFOThreadPool(/* numWorkLists= */ 2,
                                                           /* threads= */ 1,
                                                           /* allowBorrowing= */ false);
    }

    TestOptions newOptions(origOptions);
    newOptions.fContextOptions.fExecutor = executor.get();

    std::unique_ptr<PrecompileContext> precompileContext;

    // Create a scoped Context so we can destroy it early
    {
        std::unique_ptr<Context> context = testContext->makeContext(newOptions);

        precompileContext = context->makePrecompileContext();

        // Queue up an excess of precompilations
        add_precompilation(precompileContext.get());

        if (!keepPrecompileContextAlive) {
            precompileContext = nullptr;
        }

        // Destroying the Context here should trigger the PipelineManager to shut down with work
        // still pending.
    }

    // However, PrecompileContext can keep things alive on its own but will be downgraded
    // to non-threaded compilation.
    if (keepPrecompileContextAlive) {
        add_precompilation(precompileContext.get());
    }

    // Success is defined by not crashing or triggering any *SAN errors
    precompileContext.reset();
}


} // anonymous namespace


// Verify that the threaded PipelineManager terminates cleanly
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerEarlyExitTest_1,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           /* origContext */,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(origTestContext, origOptions,
             /* allowThreads= */ true, /* keepPrecompileContextAlive= */ false);
}

// Verify that the PrecompileContext pins everything it needs to operate on its own
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerEarlyExitTest_2,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           /* origContext */,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(origTestContext, origOptions,
             /* allowThreads= */ true, /* keepPrecompileContextAlive= */ true);
}

// The next two are the same as above but single threaded
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerEarlyExitTest_3,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           /* origContext */,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(origTestContext, origOptions,
             /* allowThreads= */ false, /* keepPrecompileContextAlive= */ false);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(PipelineManagerEarlyExitTest_4,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           /* origContext */,
                                           origTestContext,
                                           origOptions,
                                           /* optionsProc= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    run_test(origTestContext, origOptions,
             /* allowThreads= */ false, /* keepPrecompileContextAlive= */ true);
}
#endif // SK_GRAPHITE
