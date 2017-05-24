/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRasterPipeline.h"

SkRasterPipeline::SkRasterPipeline(SkArenaAlloc* alloc) : fAlloc(alloc) {
    this->reset();
}
void SkRasterPipeline::reset() {
    fStages      = nullptr;
    fSlotsNeeded = 1;  // We always need one extra slot for just_return().
}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
    SkASSERT(stage != from_srgb);
    this->unchecked_append(stage, ctx);
}
void SkRasterPipeline::unchecked_append(StockStage stage, void* ctx) {
    fStages = fAlloc->make<StageList>( StageList{fStages, stage, ctx} );
    fSlotsNeeded += ctx ? 2 : 1;
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    this->extend(src.fStages);
}
void SkRasterPipeline::extend(const StageList* stages) {
    if (!stages) {
        return;
    }
    this->extend(stages->prev);
    this->unchecked_append(stages->stage, stages->ctx);
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, (in reverse)\n");
    for (auto st = fStages; st; st = st->prev) {
        const char* name = "";
        switch (st->stage) {
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
    this->unchecked_append(from_srgb, nullptr);
    if (at == kPremul_SkAlphaType) {
        this->append(SkRasterPipeline::clamp_a);
    }
}
