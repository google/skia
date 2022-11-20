/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include <algorithm>
#include <cstring>
#include <utility>

namespace SkSL {
namespace RP {

using SkRP = SkRasterPipeline;

std::unique_ptr<Program> Builder::finish() {
    return std::make_unique<Program>(std::move(fInstructions));
}

void Program::optimize() {
    // TODO(johnstiles): perform any last-minute cleanup of the instruction stream here
}

int Program::numValueSlots() {
    Slot s = NA;
    for (const Instruction& inst : fInstructions) {
        for (Slot cur : {inst.fSlotA, inst.fSlotB, inst.fSlotC}) {
            s = std::max(s, cur);
        }
    }
    return s + 1;
}

int Program::numTempStackSlots() {
    int largest = 0;
    int current = 0;
    for (const Instruction& inst : fInstructions) {
        switch (inst.fOp) {
            case BuilderOp::push_temp_f:
                ++current;
                largest = std::max(current, largest);
                break;

            case BuilderOp::discard_temp:
                --current;
                SkASSERTF(current >= 0, "unbalanced temp stack push/pop");
                break;

            default:
                // This op doesn't affect the stack.
                break;
        }
    }

    SkASSERTF(current == 0, "unbalanced temp stack push/pop");
    return largest;
}

int Program::numConditionMaskSlots() {
    int largest = 0;
    int current = 0;
    for (const Instruction& inst : fInstructions) {
        switch (inst.fOp) {
            case SkRasterPipeline::store_condition_mask:
                ++current;
                largest = std::max(current, largest);
                break;

            case SkRasterPipeline::load_condition_mask:
                --current;
                SkASSERTF(current >= 0, "unbalanced condition-mask push/pop");
                break;

            default:
                // This op doesn't affect the stack.
                break;
        }
    }

    SkASSERTF(current == 0, "unbalanced condition-mask push/pop");
    return largest;
}

Program::Program(SkTArray<Instruction> instrs) : fInstructions(std::move(instrs)) {
    this->optimize();
    fNumValueSlots = this->numValueSlots();
    fNumTempStackSlots = this->numTempStackSlots();
    fNumConditionMaskSlots = this->numConditionMaskSlots();
}

template <typename T>
[[maybe_unused]] static void* context_bit_pun(T val) {
    static_assert(sizeof(T) <= sizeof(void*));
    void* contextBits = nullptr;
    memcpy(&contextBits, &val, sizeof(val));
    return contextBits;
}

void Program::appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc) {
    // skslc and sksl-minify do not actually include SkRasterPipeline.
#if !defined(SKSL_STANDALONE)
    // Allocate a contiguous slab of slot data.
    const int N = SkOpts::raster_pipeline_highp_stride;
    int totalSlots = fNumValueSlots + fNumTempStackSlots + fNumConditionMaskSlots;
    float* slotPtr = alloc->makeArray<float>(N * totalSlots);

    // Store the stacks immediately after the values.
    float* tempStackPtr = slotPtr + (N * fNumValueSlots);
    float* conditionStackPtr = tempStackPtr + (N * fNumTempStackSlots);

    for (const Instruction& inst : fInstructions) {
        auto SlotA = [&]() { return &slotPtr[N * inst.fSlotA]; };
        auto SlotB = [&]() { return &slotPtr[N * inst.fSlotB]; };

        switch (inst.fOp) {
            case SkRP::init_lane_masks:
                pipeline->append(SkRP::init_lane_masks);
                break;

            case SkRP::store_src_rg:
                pipeline->append(SkRP::store_src_rg, SlotA());
                break;

            case SkRP::store_src:
                pipeline->append(SkRP::store_src, SlotA());
                break;

            case SkRP::store_dst:
                pipeline->append(SkRP::store_dst, SlotA());
                break;

            case SkRP::load_src:
                pipeline->append(SkRP::load_src, SlotA());
                break;

            case SkRP::load_dst:
                pipeline->append(SkRP::load_dst, SlotA());
                break;

            case SkRP::immediate_f: {
                pipeline->append(SkRP::immediate_f, context_bit_pun(inst.fImmF32));
                break;
            }
            case SkRP::load_unmasked:
                pipeline->append(SkRP::load_unmasked, SlotA());
                break;

            case SkRP::store_unmasked:
                pipeline->append(SkRP::store_unmasked, SlotA());
                break;

            case SkRP::store_masked:
                pipeline->append(SkRP::store_masked, SlotA());
                break;

            case SkRP::copy_slot_masked:
                pipeline->append_copy_slots_masked(alloc, SlotA(), SlotB(), inst.fImmI32);
                break;

            case SkRP::store_condition_mask:
                pipeline->append(SkRP::store_condition_mask, conditionStackPtr);
                conditionStackPtr += N;
                break;

            case SkRP::load_condition_mask:
                conditionStackPtr -= N;
                pipeline->append(SkRP::load_condition_mask, conditionStackPtr);
                break;

            case BuilderOp::push_temp_f:
                pipeline->append(SkRP::immediate_f, context_bit_pun(inst.fImmF32));
                pipeline->append(SkRP::store_unmasked, tempStackPtr);
                tempStackPtr += N;
                break;

            case BuilderOp::discard_temp:
                tempStackPtr -= N;
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
