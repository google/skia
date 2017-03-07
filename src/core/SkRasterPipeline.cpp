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
    SkASSERT(stage != from_srgb);
    fStages.push_back({stage, ctx});
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    fStages.insert(fStages.end(),
                   src.fStages.begin(), src.fStages.end());
}

void SkRasterPipeline::run(size_t x, size_t n) const {
    if (!fStages.empty()) {
        if (this->run_with_jumper(x, n)) {
            return;
        }
        SkOpts::run_pipeline(x,n, fStages.data(), SkToInt(fStages.size()));
    }
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

// It's pretty easy to start with sound premultiplied linear floats, pack those
// to sRGB encoded bytes, then read them back to linear floats and find them not
// quite premultiplied, with a color channel just a smidge greater than the alpha
// channel.  This can happen basically any time we have different transfer
// functions for alpha and colors... sRGB being the only one we draw into.

// This is an annoying problem with no known good solution.  So apply the clamp hammer.

void SkRasterPipeline::append_from_srgb(SkAlphaType at) {
    //this->append(from_srgb);
    fStages.push_back({from_srgb, nullptr});

    if (at == kPremul_SkAlphaType) {
        this->append(SkRasterPipeline::clamp_a);
    }
}
