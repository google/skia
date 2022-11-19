/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkUtils.h"

#include <cstdint>
#include <initializer_list>
#include <memory>

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

// Ops that the builder will contextually rewrite into different RasterPipeline stages.
namespace BuilderOp {
    enum {
        push_temp_f = SkRasterPipeline::kNumHighpStages + 1,
        discard_temp,
    };
}

// Represents a single raster-pipeline SkSL instruction.
struct Instruction {
    Instruction(int op, std::initializer_list<Slot> slots)
            : fOp(op), fImmF32(0.0f), fImmI32(0) {
        auto iter = slots.begin();
        if (iter != slots.end()) { fSlotA = *iter++; }
        if (iter != slots.end()) { fSlotB = *iter++; }
        if (iter != slots.end()) { fSlotC = *iter++; }
        SkASSERT(iter == slots.end());
    }

    Instruction(int op, std::initializer_list<Slot> slots, float f, int i)
            : fOp(op), fImmF32(f), fImmI32(i) {
        auto iter = slots.begin();
        if (iter != slots.end()) { fSlotA = *iter++; }
        if (iter != slots.end()) { fSlotB = *iter++; }
        if (iter != slots.end()) { fSlotC = *iter++; }
        SkASSERT(iter == slots.end());
    }

    int   fOp;
    Slot  fSlotA = NA;
    Slot  fSlotB = NA;
    Slot  fSlotC = NA;
    float fImmF32 = 0.0f;
    int   fImmI32 = 0;
};

class Program {
public:
    Program(SkTArray<Instruction> instrs);

    void appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc);

private:
    void optimize();
    int numValueSlots();
    int numTempStackSlots();
    int numConditionMaskSlots();

    SkTArray<Instruction> fInstructions;
    int fNumValueSlots = 0;
    int fNumTempStackSlots = 0;
    int fNumConditionMaskSlots = 0;
};

class Builder {
public:
    /** Finalizes and optimizes the program. */
    std::unique_ptr<Program> finish();

    /** Assemble a program from the Raster Pipeline instructions below. */
    void init_lane_masks() {
        fInstructions.push_back({SkRasterPipeline::init_lane_masks, {}});
    }

    void store_src_rg(SlotRange slots) {
        SkASSERT(slots.count == 2);
        fInstructions.push_back({SkRasterPipeline::store_src_rg, {slots.index}});
    }

    void store_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::store_src, {slots.index}});
    }

    void store_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::store_dst, {slots.index}});
    }

    void load_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::load_src, {slots.index}});
    }

    void load_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({SkRasterPipeline::load_dst, {slots.index}});
    }

    void immediate_f(float val) {
        fInstructions.push_back({SkRasterPipeline::immediate_f, {}, val, 0});
    }

    // SkRasterPipeline registers are floats, so it's easiest just to reuse immediate_f here.
    void immediate_i(int32_t val) {
        fInstructions.push_back({SkRasterPipeline::immediate_f, {}, sk_bit_cast<float>(val), 0});
    }

    void immediate_u(uint32_t val) {
        fInstructions.push_back({SkRasterPipeline::immediate_f, {}, sk_bit_cast<float>(val), 0});
    }

    void push_temp_f(float val) {
        fInstructions.push_back({BuilderOp::push_temp_f, {}, val, 0});
    }

    // SkRasterPipeline registers are floats, so it's easiest just to reuse push_immediate_f here.
    void push_temp_i(int32_t val) {
        fInstructions.push_back({BuilderOp::push_temp_f, {}, sk_bit_cast<float>(val), 0});
    }

    void push_temp_u(uint32_t val) {
        fInstructions.push_back({BuilderOp::push_temp_f, {}, sk_bit_cast<float>(val), 0});
    }

    void discard_temp() {
        fInstructions.push_back({BuilderOp::discard_temp, {}});
    }

    void load_unmasked(Slot slot) {
        fInstructions.push_back({SkRasterPipeline::load_unmasked, {slot}});
    }

    void store_unmasked(Slot slot) {
        fInstructions.push_back({SkRasterPipeline::store_unmasked, {slot}});
    }

    void store_masked(Slot slot) {
        fInstructions.push_back({SkRasterPipeline::store_masked, {slot}});
    }

    void push_condition_mask() {
        // Raster pipeline uses a "store" op, and the builder manages the stack position.
        fInstructions.push_back({SkRasterPipeline::store_condition_mask, {}});
    }

    void pop_condition_mask() {
        // Raster pipeline uses a "load" op, and the builder manages the stack position.
        fInstructions.push_back({SkRasterPipeline::load_condition_mask, {}});
    }

private:
    SkTArray<Instruction> fInstructions;
};

}  // namespace RP
}  // namespace SkSL
