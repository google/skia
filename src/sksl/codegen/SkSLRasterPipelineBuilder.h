/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkRasterPipeline.h"

class SkArenaAlloc;

namespace SkSL {
namespace RP {

// A single scalar in our program consumes one slot.
using Slot = int;
constexpr Slot NA = -1;

// Scalars, vectors, and matrices can be represented as a range of slot indices.
struct SlotRange {
    Slot index = 0;
    int count = 0;
};

// Represents a single raster-pipeline SkSL instruction.
struct Instruction {
    Instruction(SkRasterPipeline::Stage op, Slot a) : fOp(op), fSlotA(a) {}
    Instruction(SkRasterPipeline::Stage op, Slot a, Slot b) : fOp(op), fSlotA(a), fSlotB(b) {}
    Instruction(SkRasterPipeline::Stage op, Slot a, Slot b, Slot c)
            : fOp(op), fSlotA(a), fSlotB(b), fSlotC(c) {}

    SkRasterPipeline::Stage fOp;
    Slot  fSlotA = NA;
    Slot  fSlotB = NA;
    Slot  fSlotC = NA;
    float fImmF32 = 0.0f;
    int   fImmI32 = 0;
};

class Builder {
public:
    /** Finalizes and optimizes the program. Must be called exactly once, before appendStages. */
    void finish();

    /** Emits instructions into the Raster Pipeline. finish() must be called first. */
    void appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc);

    /** Assemble a program from the Raster Pipeline instructions below. */
    void store_src_rg(SlotRange slots) {
        SkASSERT(slots.count == 2);
        fInstructions.push_back({SkRasterPipeline::store_src_rg, slots.index});
    }

    void store_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::store_src, slots.index});
    }

    void store_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::store_dst, slots.index});
    }

    void load_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::load_src, slots.index});
    }

    void load_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::load_dst, slots.index});
    }

private:
    void optimize();
    Slot findHighestSlot();

    SkTArray<Instruction> fInstructions;
    Slot fHighestSlot = NA;
};

}  // namespace RP
}  // namespace SkSL
