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
    fStages.push_back({stage, ctx});
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    fStages.insert(fStages.end(),
                   src.fStages.begin(), src.fStages.end());
}

void SkRasterPipeline::run(size_t x, size_t y, size_t n) const {
    SkOpts::run_pipeline(x,y,n, fStages.data(), SkToInt(fStages.size()));
}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    return SkOpts::compile_pipeline(fStages.data(), SkToInt(fStages.size()));
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, %d stages\n", SkToInt(fStages.size()));
    for (auto&& st : fStages) {
        const char* name = "";
        switch (st.stage) {
        #define M(x) case x: name = #x; break;
            SK_RASTER_PIPELINE_STAGES(M)
        #undef M
        }
        SkDebugf("\t%s\n", name);
    }
    SkDebugf("\n");
}
