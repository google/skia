/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline() {}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
#ifdef SK_DEBUG
    if (fNum == (int)SK_ARRAY_COUNT(fStages)) {
        this->dump();
    }
#endif
    SkASSERT(fNum < (int)SK_ARRAY_COUNT(fStages));
    fStages[fNum++] = { stage, ctx };
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    for (int i = 0; i < src.fNum; i++) {
        const Stage& s = src.fStages[i];
        this->append(s.stage, s.ctx);
    }
}

void SkRasterPipeline::run(size_t x, size_t y, size_t n) const {
    SkOpts::run_pipeline(x,y,n, fStages, fNum);
}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    return SkOpts::compile_pipeline(fStages, fNum);
}

static bool invariant_in_x(const SkRasterPipeline::Stage& st) {
    if (st.ctx == nullptr) {
        // If a stage doesn't have a context pointer, it can't really do anything with x.
        return true;
    }

    // This would be a lot more compact as a blacklist (the loads, the stores, the gathers),
    // but it's safer to write as a whitelist.  If we get it wrong this way, not a big deal.
    switch (st.stage) {
        default: return false;

        case SkRasterPipeline::trace:
        case SkRasterPipeline::set_rgb:
        case SkRasterPipeline::constant_color:
        case SkRasterPipeline::scale_constant_float:
        case SkRasterPipeline::lerp_constant_float:
        case SkRasterPipeline::matrix_2x3:
        case SkRasterPipeline::matrix_3x4:
        case SkRasterPipeline::matrix_4x5:
        case SkRasterPipeline::matrix_perspective:
        case SkRasterPipeline::parametric_r:
        case SkRasterPipeline::parametric_g:
        case SkRasterPipeline::parametric_b:
        case SkRasterPipeline::table_r:
        case SkRasterPipeline::table_g:
        case SkRasterPipeline::table_b:
        case SkRasterPipeline::color_lookup_table:
        case SkRasterPipeline::lab_to_xyz:
        case SkRasterPipeline::clamp_x:
        case SkRasterPipeline::mirror_x:
        case SkRasterPipeline::repeat_x:
        case SkRasterPipeline::clamp_y:
        case SkRasterPipeline::mirror_y:
        case SkRasterPipeline::repeat_y:
        case SkRasterPipeline::top_left:
        case SkRasterPipeline::top_right:
        case SkRasterPipeline::bottom_left:
        case SkRasterPipeline::bottom_right:
        case SkRasterPipeline::accumulate:
            SkASSERT(st.ctx != nullptr);
            return true;
    }
    return false;
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, %d stages\n", fNum);
    bool in_constant_run = false;
    for (int i = 0; i < fNum; i++) {
        const char* name = "";
        switch (fStages[i].stage) {
        #define M(x) case x: name = #x; break;
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }

        char mark = ' ';
        if (fStages[i].stage == SkRasterPipeline::constant_color) {
            mark = '*';
            in_constant_run = true;
        } else if (in_constant_run && invariant_in_x(fStages[i])) {
            mark = '|';
        } else {
            mark = ' ';
            in_constant_run = false;
        }
        SkDebugf("\t%c %s\n", mark, name);
    }
    SkDebugf("\n");
}
