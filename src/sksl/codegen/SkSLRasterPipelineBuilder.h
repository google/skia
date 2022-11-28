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
enum class BuilderOp {
    // We support all the native Raster Pipeline stages.
    #define M(stage) stage,
        SK_RASTER_PIPELINE_STAGES_ALL(M)
    #undef M
    // We also support Builder-specific ops; these are converted into real RP stages during
    // `appendStages`.
    push_literal_f,
    push_slots,
    copy_stack_to_slots,
    discard_stack,
    duplicate,
};

// Represents a single raster-pipeline SkSL instruction.
struct Instruction {
    Instruction(BuilderOp op, std::initializer_list<Slot> slots) : fOp(op), fImmA(0) {
        auto iter = slots.begin();
        if (iter != slots.end()) { fSlotA = *iter++; }
        if (iter != slots.end()) { fSlotB = *iter++; }
        if (iter != slots.end()) { fSlotC = *iter++; }
        SkASSERT(iter == slots.end());
    }

    Instruction(BuilderOp op, std::initializer_list<Slot> slots, int i) : fOp(op), fImmA(i) {
        auto iter = slots.begin();
        if (iter != slots.end()) { fSlotA = *iter++; }
        if (iter != slots.end()) { fSlotB = *iter++; }
        if (iter != slots.end()) { fSlotC = *iter++; }
        SkASSERT(iter == slots.end());
    }

    BuilderOp fOp;
    Slot      fSlotA = NA;
    Slot      fSlotB = NA;
    Slot      fSlotC = NA;
    int       fImmA = 0;
};

class Program {
public:
    Program(SkTArray<Instruction> instrs, int numValueSlots);

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
    std::unique_ptr<Program> finish(int numValueSlots);

    /** Assemble a program from the Raster Pipeline instructions below. */
    void init_lane_masks() {
        fInstructions.push_back({BuilderOp::init_lane_masks, {}});
    }

    void store_src_rg(SlotRange slots) {
        SkASSERT(slots.count == 2);
        fInstructions.push_back({BuilderOp::store_src_rg, {slots.index}});
    }

    void store_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({BuilderOp::store_src, {slots.index}});
    }

    void store_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({BuilderOp::store_dst, {slots.index}});
    }

    void load_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({BuilderOp::load_src, {slots.index}});
    }

    void load_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fInstructions.push_back({BuilderOp::load_dst, {slots.index}});
    }

    // Use the same SkRasterPipeline op regardless of the literal type.
    void immediate_f(float val) {
        fInstructions.push_back({BuilderOp::immediate_f, {}, sk_bit_cast<int32_t>(val)});
    }

    void immediate_i(int32_t val) {
        fInstructions.push_back({BuilderOp::immediate_f, {}, val});
    }

    void immediate_u(uint32_t val) {
        fInstructions.push_back({BuilderOp::immediate_f, {}, sk_bit_cast<int32_t>(val)});
    }

    void push_literal_f(float val) {
        fInstructions.push_back({BuilderOp::push_literal_f, {}, sk_bit_cast<int32_t>(val)});
    }

    void push_literal_i(int32_t val) {
        fInstructions.push_back({BuilderOp::push_literal_f, {}, val});
    }

    void push_literal_u(uint32_t val) {
        fInstructions.push_back({BuilderOp::push_literal_f, {}, sk_bit_cast<int32_t>(val)});
    }

    void push_slots(SlotRange src) {
        // Translates into copy_slots_unmasked (from values into temp stack) in Raster Pipeline.
        fInstructions.push_back({BuilderOp::push_slots, {src.index}, src.count});
    }

    void copy_stack_to_slots(SlotRange dst) {
        // Translates into copy_slots_unmasked (from temp stack to values) in Raster Pipeline.
        // Does not discard any values on the temp stack.
        fInstructions.push_back({BuilderOp::copy_stack_to_slots, {dst.index}, dst.count});
    }

    void discard_stack(int32_t count = 1) {
        // Shrinks the temp stack, discarding values on top.
        fInstructions.push_back({BuilderOp::discard_stack, {}, count});
    }

    void pop_slots(SlotRange dst) {
        // The opposite of push_slots; copies values from the temp stack into value slots, then
        // shrinks the temp stack.
        this->copy_stack_to_slots(dst);
        this->discard_stack(dst.count);
    }

    void duplicate(int count) {
        // Creates duplicates of the top item on the temp stack.
        SkASSERT(count >= 0);
        fInstructions.push_back({BuilderOp::duplicate, {}, count});
    }

    void load_unmasked(Slot slot) {
        fInstructions.push_back({BuilderOp::load_unmasked, {slot}});
    }

    void store_unmasked(Slot slot) {
        fInstructions.push_back({BuilderOp::store_unmasked, {slot}});
    }

    void store_masked(Slot slot) {
        fInstructions.push_back({BuilderOp::store_masked, {slot}});
    }

    void copy_slots_masked(SlotRange dst, SlotRange src) {
        SkASSERT(dst.count == src.count);
        fInstructions.push_back({BuilderOp::copy_slot_masked, {dst.index, src.index}, dst.count});
    }

    void copy_slots_unmasked(SlotRange dst, SlotRange src) {
        SkASSERT(dst.count == src.count);
        fInstructions.push_back({BuilderOp::copy_slot_unmasked, {dst.index, src.index}, dst.count});
    }

    void zero_slots_unmasked(SlotRange dst) {
        fInstructions.push_back({BuilderOp::zero_slot_unmasked, {dst.index}, dst.count});
    }

    void push_condition_mask() {
        // Raster pipeline uses a "store" op, and the builder manages the stack position.
        fInstructions.push_back({BuilderOp::store_condition_mask, {}});
    }

    void pop_condition_mask() {
        // Raster pipeline uses a "load" op, and the builder manages the stack position.
        fInstructions.push_back({BuilderOp::load_condition_mask, {}});
    }

private:
    SkTArray<Instruction> fInstructions;
};

}  // namespace RP
}  // namespace SkSL
