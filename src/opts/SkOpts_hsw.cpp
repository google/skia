/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS hsw
#include "SkRasterPipeline_opts.h"
#include "SkUtils_opts.h"

namespace SkOpts {
    void Init_hsw() {
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
