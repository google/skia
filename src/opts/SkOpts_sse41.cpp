/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse41
#include "SkBlitRow_opts.h"
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_sse41() {
        blit_row_color32     = sse41::blit_row_color32;
        blit_row_s32a_opaque = sse41::blit_row_s32a_opaque;

    #define M(st) stages_highp[SkRasterPipeline::st] = (StageFn)SK_OPTS_NS::st;
        SK_RASTER_PIPELINE_STAGES(M)
        just_return_highp = (StageFn)SK_OPTS_NS::just_return;
        start_pipeline_highp = SK_OPTS_NS::start_pipeline;
    #undef M

    #define M(st) stages_lowp[SkRasterPipeline::st] = (StageFn)SK_OPTS_NS::lowp::st;
        SK_RASTER_PIPELINE_STAGES(M)
        just_return_lowp = (StageFn)SK_OPTS_NS::lowp::just_return;
        start_pipeline_lowp = SK_OPTS_NS::lowp::start_pipeline;
    #undef M
    }
}
