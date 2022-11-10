/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include <algorithm>

namespace SkSL {
namespace RP {

using SkRP = SkRasterPipeline;

void Builder::optimize() {
    // TODO(johnstiles): perform any last-minute cleanup of the instruction stream here
}

Slot Builder::findHighestSlot() {
    Slot s = NA;
    for (const Instruction& inst : fInstructions) {
        s = std::max(s, inst.fSlotA);
        s = std::max(s, inst.fSlotB);
        s = std::max(s, inst.fSlotC);
    }
    return s;
}

void Builder::finish() {
    SkASSERT(fHighestSlot == NA);  // this program has already been finished

    this->optimize();
    fHighestSlot = this->findHighestSlot();
}

void Builder::appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc) {
    // skslc and sksl-minify do not actually include SkRasterPipeline.
#if !defined(SKSL_STANDALONE)
    SkASSERT(fHighestSlot >= 0);  // a program should be finished first

    // Allocate a contiguous slab of slot data. TODO(skia:13676): use an architecture-specific value
    // for N to avoid tons of dead space between slots.
    constexpr int N = SkRasterPipeline_kMaxStride_highp;
    float* slotPtr = alloc->makeArray<float>(N * fHighestSlot);

    for (const Instruction& inst : fInstructions) {
        float* slotA = &slotPtr[N * inst.fSlotA];
        // float* slotB = &slotPtr[N * inst.fSlotB];
        // float* slotC = &slotPtr[N * inst.fSlotC];

        switch (inst.fOp) {
            case SkRP::store_src_rg:
                pipeline->append(SkRP::store_src_rg, slotA);
                break;

            case SkRP::store_src:
                pipeline->append(SkRP::store_src, slotA);
                break;

            case SkRP::store_dst:
                pipeline->append(SkRP::store_dst, slotA);
                break;

            case SkRP::load_src:
                pipeline->append(SkRP::load_src, slotA);
                break;

            case SkRP::load_dst:
                pipeline->append(SkRP::load_dst, slotA);
                break;

            default:
                SkDEBUGFAILF("Raster Pipeline: unsupported instruction %d", (int)inst.fOp);
                break;
        }
    }
#endif
}

}  // namespace RP
}  // namespace SkSL
