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
    fNumStages   = 0;
    fSlotsNeeded = 1;  // We always need one extra slot for just_return().
}

void SkRasterPipeline::append(StockStage stage, void* ctx) {
    SkASSERT(stage != from_srgb);
    this->unchecked_append(stage, ctx);
}
void SkRasterPipeline::unchecked_append(StockStage stage, void* ctx) {
    fStages = fAlloc->make<StageList>( StageList{fStages, stage, ctx} );
    fNumStages   += 1;
    fSlotsNeeded += ctx ? 2 : 1;
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    if (src.empty()) {
        return;
    }
    auto stages = fAlloc->makeArrayDefault<StageList>(src.fNumStages);

    int n = src.fNumStages;
    const StageList* st = src.fStages;
    while (n --> 1) {
        stages[n]      = *st;
        stages[n].prev = &stages[n-1];
        st = st->prev;
    }
    stages[0]      = *st;
    stages[0].prev = fStages;

    fStages = &stages[src.fNumStages - 1];
    fNumStages   += src.fNumStages;
    fSlotsNeeded += src.fSlotsNeeded - 1;  // Don't double count just_returns().
}

void SkRasterPipeline::dump() const {
    SkDebugf("SkRasterPipeline, %d stages (in reverse)\n", fNumStages);
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
