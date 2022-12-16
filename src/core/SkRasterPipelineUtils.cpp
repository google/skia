/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineUtils.h"

void SkRasterPipelineUtils_Base::appendCopySlotsMasked(SkArenaAlloc* alloc,
                                                       float* dst,
                                                       float* src,
                                                       int numSlots) {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendCopySlotsMasked(alloc, dst, src, /*numSlots=*/4);
        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        src += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }

    SkRasterPipeline::Stage stage;
    switch (numSlots) {
        case 0:  return;
        case 1:  stage = SkRasterPipeline::copy_slot_masked;     break;
        case 2:  stage = SkRasterPipeline::copy_2_slots_masked;  break;
        case 3:  stage = SkRasterPipeline::copy_3_slots_masked;  break;
        case 4:  stage = SkRasterPipeline::copy_4_slots_masked;  break;
        default: SkUNREACHABLE;
    }

    auto* ctx = alloc->make<SkRasterPipeline_CopySlotsCtx>();
    ctx->dst = dst;
    ctx->src = src;
    this->append(stage, ctx);
}

void SkRasterPipelineUtils_Base::appendCopySlotsUnmasked(SkArenaAlloc* alloc,
                                                         float* dst,
                                                         float* src,
                                                         int numSlots) {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendCopySlotsUnmasked(alloc, dst, src, /*numSlots=*/4);
        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        src += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }

    SkRasterPipeline::Stage stage;
    switch (numSlots) {
        case 0:  return;
        case 1:  stage = SkRasterPipeline::copy_slot_unmasked;     break;
        case 2:  stage = SkRasterPipeline::copy_2_slots_unmasked;  break;
        case 3:  stage = SkRasterPipeline::copy_3_slots_unmasked;  break;
        case 4:  stage = SkRasterPipeline::copy_4_slots_unmasked;  break;
        default: SkUNREACHABLE;
    }

    auto* ctx = alloc->make<SkRasterPipeline_CopySlotsCtx>();
    ctx->dst = dst;
    ctx->src = src;
    this->append(stage, ctx);
}

void SkRasterPipelineUtils_Base::appendZeroSlotsUnmasked(float* dst, int numSlots) {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendZeroSlotsUnmasked(dst, /*numSlots=*/4);
        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }

    SkRasterPipeline::Stage stage;
    switch (numSlots) {
        case 0:  return;
        case 1:  stage = SkRasterPipeline::zero_slot_unmasked;     break;
        case 2:  stage = SkRasterPipeline::zero_2_slots_unmasked;  break;
        case 3:  stage = SkRasterPipeline::zero_3_slots_unmasked;  break;
        case 4:  stage = SkRasterPipeline::zero_4_slots_unmasked;  break;
        default: SkUNREACHABLE;
    }

    this->append(stage, dst);
}

void SkRasterPipelineUtils_Base::appendAdjacentMultiSlotOp(SkArenaAlloc* alloc,
                                                           SkRasterPipeline::Stage baseStage,
                                                           float* dst,
                                                           float* src,
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

void SkRasterPipelineUtils_Base::appendAdjacentSingleSlotOp(SkRasterPipeline::Stage stage,
                                                            float* dst,
                                                            float* src) {
    // The source and destination must be directly next to one another.
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride) == src);
    this->append(stage, dst);
}

void SkRasterPipelineUtils::append(SkRasterPipeline::Stage stage, void* ctx) {
#if !defined(SKSL_STANDALONE)
    fPipeline->append(stage, ctx);
#else
    (void)fPipeline;
#endif
}
