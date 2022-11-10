/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "tests/Test.h"

struct TestingOnly_SkRasterPipelineInspector {
    using StageList = SkRasterPipeline::StageList;
    static StageList* GetStageList(SkRasterPipeline* p) { return p->fStages; }
};

static SkSL::RP::SlotRange two_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 2};
}

static SkSL::RP::SlotRange four_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 4};
}

DEF_TEST(RasterPipelineBuilder, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.store_src_rg(two_slots_at(0));
    builder.store_src(four_slots_at(2));
    builder.store_dst(four_slots_at(6));
    builder.load_src(four_slots_at(1));
    builder.load_dst(four_slots_at(3));
    builder.finish();

    // Ask the builder to instantiate this program.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    SkRasterPipeline pipeline(&alloc);
    builder.appendStages(&pipeline, &alloc);

    // Double check that the resulting stage list contains the correct ops.
    // (Note that the RasterPipeline stage list is stored in backwards order.)
    const auto* stages = TestingOnly_SkRasterPipelineInspector::GetStageList(&pipeline);
    REPORTER_ASSERT(r, stages->stage == SkRasterPipeline::load_dst);
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->stage == SkRasterPipeline::load_src);
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->stage == SkRasterPipeline::store_dst);
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->stage == SkRasterPipeline::store_src);
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->stage == SkRasterPipeline::store_src_rg);

    // Double check that the resulting stage list contains the correct context pointers.
    // All of the ops here hold a pointer directly to their associated slot, and slots are always
    // assigned contiguously and in order, and never rearranged. We should be able to verify that
    // they are all where we expect them to be.
    const auto* firstStage = stages;
    const float* slot0 = (const float*)firstStage->ctx;
    constexpr int N = SkRasterPipeline_kMaxStride_highp;

    stages = TestingOnly_SkRasterPipelineInspector::GetStageList(&pipeline);
    REPORTER_ASSERT(r, stages->ctx == slot0 + (3 * N));
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->ctx == slot0 + (1 * N));
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->ctx == slot0 + (6 * N));
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->ctx == slot0 + (2 * N));
    stages = stages->prev;

    REPORTER_ASSERT(r, stages->ctx == slot0 + (0 * N));
}
