/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineUtils.h"

void SkRasterPipelineUtils_Base::appendAdjacentMultiSlotOp(SkArenaAlloc* alloc,
                                                           SkRasterPipeline::Stage baseStage,
                                                           float* dst,
                                                           const float* src,
                                                           int numSlots) {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots) == src);

    if (numSlots > 4) {
        auto ctx = alloc->make<SkRasterPipeline_CopySlotsCtx>();
        ctx->dst = dst;
        ctx->src = src;
        this->append(baseStage, ctx);
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (SkRasterPipeline::Stage)((int)baseStage + numSlots);
        this->append(specializedStage, dst);
    }
}

void SkRasterPipelineUtils::append(SkRasterPipeline::Stage stage, void* ctx) {
#if !defined(SKSL_STANDALONE)
    fPipeline->append(stage, ctx);
#else
    (void)fPipeline;
#endif
}
