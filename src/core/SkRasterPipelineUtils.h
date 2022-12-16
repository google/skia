/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRasterPipeline.h"

class SkArenaAlloc;

class SkRasterPipelineUtils_Base {
public:
    virtual ~SkRasterPipelineUtils_Base() = default;

    // Forwards `append` calls to a Raster Pipeline.
    virtual void append(SkRasterPipeline::Stage stage, void* ctx) = 0;

    // Appends one or more `copy_n_slots_[un]masked` stages to the pipeline, based on `numSlots`.
    void appendCopySlotsMasked(SkArenaAlloc* alloc, float* dst, float* src, int numSlots);
    void appendCopySlotsUnmasked(SkArenaAlloc* alloc, float* dst, float* src, int numSlots);
    void appendCopyConstants(SkArenaAlloc* alloc, float* dst, float* src, int numSlots);

    // Appends one or more `zero_n_slots_unmasked` stages to the pipeline, based on `numSlots`.
    void appendZeroSlotsUnmasked(float* dst, int numSlots);

    // Appends a multi-slot math operation to the pipeline. `src` must be _immediately_ after `dst`
    // in memory. `baseStage` must refer to an unbounded "apply_to_n_slots" stage, which must be
    // immediately followed by specializations for 1-4 slots. For instance, {`add_n_floats`,
    // `add_float`, `add_2_floats`, `add_3_floats`, `add_4_floats`} must be contiguous ops in the
    // stage list, listed in that order; pass `add_n_floats` and we pick the appropriate op based on
    // `numSlots`.
    void appendAdjacentMultiSlotOp(SkArenaAlloc* alloc,
                                   SkRasterPipeline::Stage baseStage,
                                   float* dst,
                                   float* src,
                                   int numSlots);

    // Appends a math operation with two inputs (dst op src) and one output (dst) to the pipeline.
    // `src` must be _immediately_ after `dst` in memory.
    void appendAdjacentSingleSlotOp(SkRasterPipeline::Stage stage, float* dst, float* src);

private:
    void appendCopy(SkArenaAlloc* alloc,
                    SkRasterPipeline::Stage baseStage,
                    float* dst, int dstStride,
                    float* src, int srcStride,
                    int numSlots);
};

class SkRasterPipelineUtils final : public SkRasterPipelineUtils_Base {
public:
    SkRasterPipelineUtils(SkRasterPipeline& p) : fPipeline(&p) {}

    void append(SkRasterPipeline::Stage stage, void* ctx) override;

private:
    SkRasterPipeline* fPipeline;
};
