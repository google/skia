/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#define SK_OPTS_NS lasx
#include "src/opts/SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_lasx() {
        raster_pipeline_lowp_stride  = SK_OPTS_NS::raster_pipeline_lowp_stride();
        raster_pipeline_highp_stride = SK_OPTS_NS::raster_pipeline_highp_stride();

    #define M(st) ops_highp[(int)SkRasterPipelineOp::st] = (StageFn)SK_OPTS_NS::st;
        SK_RASTER_PIPELINE_OPS_ALL(M)
        just_return_highp = (StageFn)SK_OPTS_NS::just_return;
        start_pipeline_highp = SK_OPTS_NS::start_pipeline;
    #undef M

    #define M(st) ops_lowp[(int)SkRasterPipelineOp::st] = (StageFn)SK_OPTS_NS::lowp::st;
        SK_RASTER_PIPELINE_OPS_LOWP(M)
        just_return_lowp = (StageFn)SK_OPTS_NS::lowp::just_return;
        start_pipeline_lowp = SK_OPTS_NS::lowp::start_pipeline;
    #undef M
    }
}  // namespace SkOpts

#endif // SK_ENABLE_OPTIMIZE_SIZE
