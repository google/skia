/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#define SK_OPTS_NS hsw
#include "src/core/SkCubicSolver.h"
#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkBlitRow_opts.h"
#include "src/opts/SkRasterPipeline_opts.h"
#include "src/opts/SkSwizzler_opts.h"
#include "src/opts/SkUtils_opts.h"
#include "src/opts/SkVM_opts.h"

namespace SkOpts {
    void Init_hsw() {
        blit_row_color32     = hsw::blit_row_color32;
        blit_row_s32a_opaque = hsw::blit_row_s32a_opaque;

        S32_alpha_D32_filter_DX  = hsw::S32_alpha_D32_filter_DX;

        cubic_solver = SK_OPTS_NS::cubic_solver;

        RGBA_to_BGRA          = SK_OPTS_NS::RGBA_to_BGRA;
        RGBA_to_rgbA          = SK_OPTS_NS::RGBA_to_rgbA;
        RGBA_to_bgrA          = SK_OPTS_NS::RGBA_to_bgrA;
        gray_to_RGB1          = SK_OPTS_NS::gray_to_RGB1;
        grayA_to_RGBA         = SK_OPTS_NS::grayA_to_RGBA;
        grayA_to_rgbA         = SK_OPTS_NS::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = SK_OPTS_NS::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = SK_OPTS_NS::inverted_CMYK_to_BGR1;

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

        interpret_skvm = SK_OPTS_NS::interpret_skvm;
    }
}  // namespace SkOpts
