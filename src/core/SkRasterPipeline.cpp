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
    SkASSERT(fNum < (int)SK_ARRAY_COUNT(fStages));
    fStages[fNum++] = { stage, ctx };
}

void SkRasterPipeline::extend(const SkRasterPipeline& src) {
    for (int i = 0; i < src.fNum; i++) {
        const Stage& s = src.fStages[i];
        this->append(s.stage, s.ctx);
    }
}

std::function<void(size_t, size_t, size_t)> SkRasterPipeline::compile() const {
    return SkOpts::compile_pipeline(fStages, fNum);
}
