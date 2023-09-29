/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkOpts.h"

#include "include/private/base/SkFeatures.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOptsTargets.h"

#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkRasterPipeline_opts.h"  // IWYU pragma: keep

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
    size_t raster_pipeline_lowp_stride  = SK_OPTS_NS::raster_pipeline_lowp_stride();
    size_t raster_pipeline_highp_stride = SK_OPTS_NS::raster_pipeline_highp_stride();

#define M(st) (StageFn)SK_OPTS_NS::st,
    StageFn ops_highp[] = { SK_RASTER_PIPELINE_OPS_ALL(M) };
    StageFn just_return_highp = (StageFn)SK_OPTS_NS::just_return;
    void (*start_pipeline_highp)(size_t, size_t, size_t, size_t, SkRasterPipelineStage*) =
            SK_OPTS_NS::start_pipeline;
#undef M

#define M(st) (StageFn)SK_OPTS_NS::lowp::st,
    StageFn ops_lowp[] = { SK_RASTER_PIPELINE_OPS_LOWP(M) };
    StageFn just_return_lowp = (StageFn)SK_OPTS_NS::lowp::just_return;
    void (*start_pipeline_lowp)(size_t, size_t, size_t, size_t, SkRasterPipelineStage*) =
            SK_OPTS_NS::lowp::start_pipeline;
#undef M

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_hsw();

    static bool init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX2
            if (SkCpu::Supports(SkCpu::HSW)) { Init_hsw(); }
        #endif
    #endif
        return true;
    }

    void Init() {
        [[maybe_unused]] static bool gInitialized = init();
    }
}  // namespace SkOpts
