/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/private/SkSLString.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "include/sksl/SkSLPosition.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"
#include "src/utils/SkBitSet.h"

#if !defined(SKSL_STANDALONE)
#include "src/core/SkRasterPipeline.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace SkSL {
namespace RP {

#define ALL_SINGLE_SLOT_UNARY_OP_CASES  \
         BuilderOp::atan_float:         \
    case BuilderOp::cos_float:          \
    case BuilderOp::exp_float:          \
    case BuilderOp::sin_float:          \
    case BuilderOp::sqrt_float:         \
    case BuilderOp::tan_float

#define ALL_MULTI_SLOT_UNARY_OP_CASES        \
         BuilderOp::abs_float:               \
    case BuilderOp::abs_int:                 \
    case BuilderOp::bitwise_not_int:         \
    case BuilderOp::cast_to_float_from_int:  \
    case BuilderOp::cast_to_float_from_uint: \
    case BuilderOp::cast_to_int_from_float:  \
    case BuilderOp::cast_to_uint_from_float: \
    case BuilderOp::ceil_float:              \
    case BuilderOp::floor_float              \

#define ALL_N_WAY_BINARY_OP_CASES   \
         BuilderOp::atan2_n_floats: \
    case BuilderOp::pow_n_floats

#define ALL_MULTI_SLOT_BINARY_OP_CASES  \
         BuilderOp::add_n_floats:       \
    case BuilderOp::add_n_ints:         \
    case BuilderOp::sub_n_floats:       \
    case BuilderOp::sub_n_ints:         \
    case BuilderOp::mul_n_floats:       \
    case BuilderOp::mul_n_ints:         \
    case BuilderOp::div_n_floats:       \
    case BuilderOp::div_n_ints:         \
    case BuilderOp::div_n_uints:        \
    case BuilderOp::bitwise_and_n_ints: \
    case BuilderOp::bitwise_or_n_ints:  \
    case BuilderOp::bitwise_xor_n_ints: \
    case BuilderOp::min_n_floats:       \
    case BuilderOp::min_n_ints:         \
    case BuilderOp::min_n_uints:        \
    case BuilderOp::max_n_floats:       \
    case BuilderOp::max_n_ints:         \
    case BuilderOp::max_n_uints:        \
    case BuilderOp::cmple_n_floats:     \
    case BuilderOp::cmple_n_ints:       \
    case BuilderOp::cmple_n_uints:      \
    case BuilderOp::cmplt_n_floats:     \
    case BuilderOp::cmplt_n_ints:       \
    case BuilderOp::cmplt_n_uints:      \
    case BuilderOp::cmpeq_n_floats:     \
    case BuilderOp::cmpeq_n_ints:       \
    case BuilderOp::cmpne_n_floats:     \
    case BuilderOp::cmpne_n_ints

#define ALL_MULTI_SLOT_TERNARY_OP_CASES \
         BuilderOp::mix_n_floats:       \
    case BuilderOp::mix_n_ints

void Builder::unary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
        case ALL_MULTI_SLOT_UNARY_OP_CASES:
            fInstructions.push_back({op, {}, slots});
            break;

        default:
            SkDEBUGFAIL("not a unary op");
            break;
    }
}

void Builder::binary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_N_WAY_BINARY_OP_CASES:
        case ALL_MULTI_SLOT_BINARY_OP_CASES:
            fInstructions.push_back({op, {}, slots});
            break;

        default:
            SkDEBUGFAIL("not a binary op");
            break;
    }
}

void Builder::ternary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_MULTI_SLOT_TERNARY_OP_CASES:
            fInstructions.push_back({op, {}, slots});
            break;

        default:
            SkDEBUGFAIL("not a ternary op");
            break;
    }
}

void Builder::dot_floats(int32_t slots) {
    switch (slots) {
        case 1: fInstructions.push_back({BuilderOp::mul_n_floats, {}, slots}); break;
        case 2: fInstructions.push_back({BuilderOp::dot_2_floats, {}, slots}); break;
        case 3: fInstructions.push_back({BuilderOp::dot_3_floats, {}, slots}); break;
        case 4: fInstructions.push_back({BuilderOp::dot_4_floats, {}, slots}); break;

        default:
            SkDEBUGFAIL("invalid number of slots");
            break;
    }
}

void Builder::discard_stack(int32_t count) {
    // If we pushed something onto the stack and then immediately discarded part of it, we can
    // shrink or eliminate the push.
    while (count > 0 && !fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        switch (lastInstruction.fOp) {
            case BuilderOp::discard_stack:
                // Our last op was actually a separate discard_stack; combine the discards.
                lastInstruction.fImmA += count;
                return;

            case BuilderOp::push_zeros:
            case BuilderOp::push_clone:
            case BuilderOp::push_clone_from_stack:
            case BuilderOp::push_slots:
            case BuilderOp::push_uniform:
                // Our last op was a multi-slot push; cancel out one discard and eliminate the op
                // if its count reached zero.
                --count;
                --lastInstruction.fImmA;
                if (lastInstruction.fImmA == 0) {
                    fInstructions.pop_back();
                }
                continue;

            case BuilderOp::push_literal:
            case BuilderOp::push_condition_mask:
            case BuilderOp::push_loop_mask:
            case BuilderOp::push_return_mask:
                // Our last op was a single-slot push; cancel out one discard and eliminate the op.
                --count;
                fInstructions.pop_back();
                continue;

            default:
                break;
        }

        // This instruction wasn't a push.
        break;
    }

    if (count > 0) {
        fInstructions.push_back({BuilderOp::discard_stack, {}, count});
    }
}

void Builder::label(int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);

    // If the previous instruction was a branch to this label, it's a no-op; jumping to the very
    // next instruction is effectively meaningless.
    while (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();
        switch (lastInstruction.fOp) {
            case BuilderOp::jump:
            case BuilderOp::branch_if_any_active_lanes:
            case BuilderOp::branch_if_no_active_lanes:
            case BuilderOp::branch_if_no_active_lanes_on_stack_top_equal:
                if (lastInstruction.fImmA == labelID) {
                    fInstructions.pop_back();
                    continue;
                }
                break;

            default:
                break;
        }
        break;
    }
    fInstructions.push_back({BuilderOp::label, {}, labelID});
}

void Builder::jump(int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (!fInstructions.empty() && fInstructions.back().fOp == BuilderOp::jump) {
        // The previous instruction was also `jump`, so this branch could never possibly occur.
        return;
    }
    fInstructions.push_back({BuilderOp::jump, {}, labelID});
}

void Builder::branch_if_any_active_lanes(int labelID) {
    if (!this->executionMaskWritesAreEnabled()) {
        this->jump(labelID);
        return;
    }

    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (!fInstructions.empty() &&
        (fInstructions.back().fOp == BuilderOp::branch_if_any_active_lanes ||
         fInstructions.back().fOp == BuilderOp::jump)) {
        // The previous instruction was `jump` or `branch_if_any_active_lanes`, so this branch
        // could never possibly occur.
        return;
    }
    fInstructions.push_back({BuilderOp::branch_if_any_active_lanes, {}, labelID});
}

void Builder::branch_if_no_active_lanes(int labelID) {
    if (!this->executionMaskWritesAreEnabled()) {
        return;
    }

    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (!fInstructions.empty() &&
        (fInstructions.back().fOp == BuilderOp::branch_if_no_active_lanes ||
         fInstructions.back().fOp == BuilderOp::jump)) {
        // The previous instruction was `jump` or `branch_if_no_active_lanes`, so this branch
        // could never possibly occur.
        return;
    }
    fInstructions.push_back({BuilderOp::branch_if_no_active_lanes, {}, labelID});
}

void Builder::branch_if_no_active_lanes_on_stack_top_equal(int value, int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (!fInstructions.empty() &&
        (fInstructions.back().fOp == BuilderOp::jump ||
         (fInstructions.back().fOp == BuilderOp::branch_if_no_active_lanes_on_stack_top_equal &&
          fInstructions.back().fImmB == value))) {
        // The previous instruction was `jump` or `branch_if_no_active_lanes_on_stack_top_equal`
        // (checking against the same value), so this branch could never possibly occur.
        return;
    }
    fInstructions.push_back({BuilderOp::branch_if_no_active_lanes_on_stack_top_equal,
                             {}, labelID, value});
}

void Builder::push_slots(SlotRange src) {
    SkASSERT(src.count >= 0);
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the previous instruction was pushing slots contiguous to this range, we can collapse
        // the two pushes into one larger push.
        if (lastInstruction.fOp == BuilderOp::push_slots &&
            lastInstruction.fSlotA + lastInstruction.fImmA == src.index) {
            lastInstruction.fImmA += src.count;
            return;
        }

        // If the previous instruction was discarding an equal number of slots...
        if (lastInstruction.fOp == BuilderOp::discard_stack && lastInstruction.fImmA == src.count) {
            // ... and the instruction before that was copying from the stack to the same slots...
            Instruction& prevInstruction = fInstructions.fromBack(1);
            if ((prevInstruction.fOp == BuilderOp::copy_stack_to_slots ||
                 prevInstruction.fOp == BuilderOp::copy_stack_to_slots_unmasked) &&
                prevInstruction.fSlotA == src.index &&
                prevInstruction.fImmA == src.count) {
                // ... we are emitting `copy stack to X, discard stack, copy X to stack`. This is a
                // common pattern when multiple operations in a row affect the same variable. We can
                // eliminate the discard and just leave X on the stack.
                fInstructions.pop_back();
                return;
            }
        }
    }

    if (src.count > 0) {
        fInstructions.push_back({BuilderOp::push_slots, {src.index}, src.count});
    }
}

void Builder::push_uniform(SlotRange src) {
    SkASSERT(src.count >= 0);
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the previous instruction was pushing uniforms contiguous to this range, we can
        // collapse the two pushes into one larger push.
        if (lastInstruction.fOp == BuilderOp::push_uniform &&
            lastInstruction.fSlotA + lastInstruction.fImmA == src.index) {
            lastInstruction.fImmA += src.count;
            return;
        }
    }

    if (src.count > 0) {
        fInstructions.push_back({BuilderOp::push_uniform, {src.index}, src.count});
    }
}

void Builder::push_duplicates(int count) {
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the previous op is pushing a zero, we can just push more of them.
        if (lastInstruction.fOp == BuilderOp::push_zeros) {
            lastInstruction.fImmA += count;
            return;
        }
    }
    SkASSERT(count >= 0);
    if (count >= 3) {
        // Use a swizzle to splat the input into a 4-slot value.
        this->swizzle(/*consumedSlots=*/1, {0, 0, 0, 0});
        count -= 3;
    }
    for (; count >= 4; count -= 4) {
        // Clone the splatted value four slots at a time.
        this->push_clone(/*numSlots=*/4);
    }
    // Use a swizzle or clone to handle the trailing items.
    switch (count) {
        case 3:  this->swizzle(/*consumedSlots=*/1, {0, 0, 0, 0}); break;
        case 2:  this->swizzle(/*consumedSlots=*/1, {0, 0, 0});    break;
        case 1:  this->push_clone(/*numSlots=*/1);              break;
        default: break;
    }
}

void Builder::push_clone_from_stack(int numSlots, int otherStackIndex, int offsetFromStackTop) {
    offsetFromStackTop += numSlots;

    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the previous op is also pushing a clone...
        if (lastInstruction.fOp == BuilderOp::push_clone_from_stack &&
            // ... from the same stack...
            lastInstruction.fImmB == otherStackIndex &&
            // ... and this clone starts at the same place that the last clone ends...
            lastInstruction.fImmC - lastInstruction.fImmA == offsetFromStackTop) {
            // ... just extend the existing clone-op.
            lastInstruction.fImmA += numSlots;
            return;
        }
    }

    fInstructions.push_back({BuilderOp::push_clone_from_stack, {},
                             numSlots, otherStackIndex, offsetFromStackTop});
}

void Builder::pop_slots(SlotRange dst) {
    if (!this->executionMaskWritesAreEnabled()) {
        this->pop_slots_unmasked(dst);
        return;
    }

    this->copy_stack_to_slots(dst);
    this->discard_stack(dst.count);
}

void Builder::simplifyPopSlotsUnmasked(SlotRange* dst) {
    if (!dst->count || fInstructions.empty()) {
        // There's nothing left to simplify.
        return;
    }

    Instruction& lastInstruction = fInstructions.back();

    // If the last instruction is pushing a constant, we can simplify it by copying the constant
    // directly into the destination slot.
    if (lastInstruction.fOp == BuilderOp::push_literal) {
        // Remove the constant-push instruction.
        int value = lastInstruction.fImmA;
        fInstructions.pop_back();

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Continue simplifying if possible.
        this->simplifyPopSlotsUnmasked(dst);

        // Write the constant directly to the destination slot.
        this->copy_constant(destinationSlot, value);
        return;
    }

    // If the last instruction is pushing a zero, we can save a step by directly zeroing out
    // the destination slot.
    if (lastInstruction.fOp == BuilderOp::push_zeros) {
        // Remove one zero-push.
        lastInstruction.fImmA--;
        if (lastInstruction.fImmA == 0) {
            fInstructions.pop_back();
        }

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Continue simplifying if possible.
        this->simplifyPopSlotsUnmasked(dst);

        // Zero the destination slot directly.
        this->zero_slots_unmasked({destinationSlot, 1});
        return;
    }

    // If the last instruction is pushing a slot, we can just copy that slot.
    if (lastInstruction.fOp == BuilderOp::push_slots) {
        // Get the last slot.
        Slot sourceSlot = lastInstruction.fSlotA + lastInstruction.fImmA - 1;
        lastInstruction.fImmA--;
        if (lastInstruction.fImmA == 0) {
            fInstructions.pop_back();
        }

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Try once more.
        this->simplifyPopSlotsUnmasked(dst);

        // Copy the slot directly.
        if (destinationSlot != sourceSlot) {
            this->copy_slots_unmasked({destinationSlot, 1}, {sourceSlot, 1});
        }
        return;
    }
}

void Builder::pop_slots_unmasked(SlotRange dst) {
    SkASSERT(dst.count >= 0);

    // If we are popping immediately after a push, we can simplify the code by writing the pushed
    // value directly to the destination range.
    this->simplifyPopSlotsUnmasked(&dst);

    // Pop from the stack normally.
    if (dst.count > 0) {
        this->copy_stack_to_slots_unmasked(dst);
        this->discard_stack(dst.count);
    }
}

void Builder::copy_stack_to_slots(SlotRange dst, int offsetFromStackTop) {
    // If the execution mask is known to be all-true, then we can ignore the write mask.
    if (!this->executionMaskWritesAreEnabled()) {
        this->copy_stack_to_slots_unmasked(dst, offsetFromStackTop);
        return;
    }

    // If the last instruction copied the previous stack slots, just extend it.
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the last op is copy-stack-to-slots...
        if (lastInstruction.fOp == BuilderOp::copy_stack_to_slots &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstruction.fSlotA + lastInstruction.fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstruction.fImmB - lastInstruction.fImmA == offsetFromStackTop) {
            // then we can just extend the copy!
            lastInstruction.fImmA += dst.count;
            return;
        }
    }

    fInstructions.push_back({BuilderOp::copy_stack_to_slots, {dst.index},
                             dst.count, offsetFromStackTop});
}

static bool slot_ranges_overlap(SlotRange x, SlotRange y) {
    return x.index < y.index + y.count &&
           y.index < x.index + x.count;
}

void Builder::copy_slots_unmasked(SlotRange dst, SlotRange src) {
    // If the last instruction copied adjacent slots, just extend it.
    if (!fInstructions.empty()) {
        Instruction& lastInstr = fInstructions.back();

        // If the last op is copy-slots-unmasked...
        if (lastInstr.fOp == BuilderOp::copy_slot_unmasked &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstr.fSlotA + lastInstr.fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstr.fSlotB + lastInstr.fImmA == src.index &&
            // and the source/dest ranges will not overlap
            !slot_ranges_overlap({lastInstr.fSlotB, lastInstr.fImmA + dst.count},
                                 {lastInstr.fSlotA, lastInstr.fImmA + dst.count})) {
            // then we can just extend the copy!
            lastInstr.fImmA += dst.count;
            return;
        }
    }

    SkASSERT(dst.count == src.count);
    fInstructions.push_back({BuilderOp::copy_slot_unmasked, {dst.index, src.index}, dst.count});
}

void Builder::copy_stack_to_slots_unmasked(SlotRange dst, int offsetFromStackTop) {
    // If the last instruction copied the previous stack slots, just extend it.
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        // If the last op is copy-stack-to-slots-unmasked...
        if (lastInstruction.fOp == BuilderOp::copy_stack_to_slots_unmasked &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstruction.fSlotA + lastInstruction.fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstruction.fImmB - lastInstruction.fImmA == offsetFromStackTop) {
            // then we can just extend the copy!
            lastInstruction.fImmA += dst.count;
            return;
        }
    }

    fInstructions.push_back({BuilderOp::copy_stack_to_slots_unmasked, {dst.index},
                             dst.count, offsetFromStackTop});
}

void Builder::pop_return_mask() {
    SkASSERT(this->executionMaskWritesAreEnabled());

    // This instruction is going to overwrite the return mask. If the previous instruction was
    // masking off the return mask, that's wasted work and it can be eliminated.
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        if (lastInstruction.fOp == BuilderOp::mask_off_return_mask) {
            fInstructions.pop_back();
        }
    }

    fInstructions.push_back({BuilderOp::pop_return_mask, {}});
}

void Builder::zero_slots_unmasked(SlotRange dst) {
    if (!fInstructions.empty()) {
        Instruction& lastInstruction = fInstructions.back();

        if (lastInstruction.fOp == BuilderOp::zero_slot_unmasked) {
            if (lastInstruction.fSlotA + lastInstruction.fImmA == dst.index) {
                // The previous instruction was zeroing the range immediately before this range.
                // Combine the ranges.
                lastInstruction.fImmA += dst.count;
                return;
            }
        }

        if (lastInstruction.fOp == BuilderOp::zero_slot_unmasked) {
            if (lastInstruction.fSlotA == dst.index + dst.count) {
                // The previous instruction was zeroing the range immediately after this range.
                // Combine the ranges.
                lastInstruction.fSlotA = dst.index;
                lastInstruction.fImmA += dst.count;
                return;
            }
        }
    }

    fInstructions.push_back({BuilderOp::zero_slot_unmasked, {dst.index}, dst.count});
}

static int pack_nybbles(SkSpan<const int8_t> components) {
    // Pack up to 8 elements into nybbles, in reverse order.
    int packed = 0;
    for (auto iter = components.rbegin(); iter != components.rend(); ++iter) {
        SkASSERT(*iter >= 0 && *iter <= 0xF);
        packed <<= 4;
        packed |= *iter;
    }
    return packed;
}

static void unpack_nybbles_to_offsets(uint32_t components, SkSpan<uint16_t> offsets) {
    // Unpack component nybbles into byte-offsets pointing at stack slots.
    for (size_t index = 0; index < offsets.size(); ++index) {
        offsets[index] = (components & 0xF) * SkOpts::raster_pipeline_highp_stride * sizeof(float);
        components >>= 4;
    }
}

void Builder::swizzle_copy_stack_to_slots(SlotRange dst,
                                          SkSpan<const int8_t> components,
                                          int offsetFromStackTop) {
    // An unmasked version of this op could squeeze out a little bit of extra speed, if needed.
    fInstructions.push_back({BuilderOp::swizzle_copy_stack_to_slots, {dst.index},
                             (int)components.size(), offsetFromStackTop, pack_nybbles(components)});
}

void Builder::swizzle(int consumedSlots, SkSpan<const int8_t> components) {
    // Consumes `consumedSlots` elements on the stack, then generates `elementSpan.size()` elements.
    SkASSERT(consumedSlots >= 0);

    // We only allow up to 16 elements, and they can only reach 0-15 slots, due to nybble packing.
    int numElements = components.size();
    SkASSERT(numElements <= 16);
    SkASSERT(std::all_of(components.begin(), components.end(), [](int8_t e){ return e >= 0; }));
    SkASSERT(std::all_of(components.begin(), components.end(), [](int8_t e){ return e <= 0xF; }));

    // Make a local copy of the element array.
    int8_t elements[16] = {};
    std::copy(components.begin(), components.end(), std::begin(elements));

    while (numElements > 0) {
        // If the first element of the swizzle is zero...
        if (elements[0] != 0) {
            break;
        }
        // ...and zero isn't used elsewhere in the swizzle...
        if (std::any_of(&elements[1], &elements[numElements], [](int8_t e) { return e == 0; })) {
            break;
        }
        // We can omit the first slot from the swizzle entirely.
        // Slide everything forward by one slot, and reduce the element index by one.
        for (int index = 1; index < numElements; ++index) {
            elements[index - 1] = elements[index] - 1;
        }
        elements[numElements - 1] = 0;
        --consumedSlots;
        --numElements;
    }

    // A completely empty swizzle is a no-op.
    if (numElements == 0) {
        this->discard_stack(consumedSlots);
        return;
    }

    if (consumedSlots <= 4 && numElements <= 4) {
        // We can fit everything into a little swizzle.
        int op = (int)BuilderOp::swizzle_1 + numElements - 1;
        fInstructions.push_back({(BuilderOp)op, {}, consumedSlots,
                                 pack_nybbles(SkSpan(elements, numElements))});
        return;
    }

    // This is a big swizzle. We use the `shuffle` op to handle these.
    // Slot usage is packed into immA. The top 16 bits of immA count the consumed slots; the bottom
    // 16 bits count the generated slots.
    int slotUsage = consumedSlots << 16;
    slotUsage |= numElements;

    // Pack immB and immC with the shuffle list in packed-nybble form.
    fInstructions.push_back({BuilderOp::shuffle, {}, slotUsage,
                             pack_nybbles(SkSpan(&elements[0], 8)),
                             pack_nybbles(SkSpan(&elements[8], 8))});
}

void Builder::transpose(int columns, int rows) {
    // Transposes a matrix of size CxR on the stack (into a matrix of size RxC).
    int8_t elements[16] = {};
    size_t index = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < columns; ++c) {
            elements[index++] = (c * rows) + r;
        }
    }
    this->swizzle(/*consumedSlots=*/columns * rows, SkSpan(elements, index));
}

void Builder::diagonal_matrix(int columns, int rows) {
    // Generates a CxR diagonal matrix from the top two scalars on the stack.
    int8_t elements[16] = {};
    size_t index = 0;
    for (int c = 0; c < columns; ++c) {
        for (int r = 0; r < rows; ++r) {
            elements[index++] = (c == r) ? 1 : 0;
        }
    }
    this->swizzle(/*consumedSlots=*/2, SkSpan(elements, index));
}

void Builder::matrix_resize(int origColumns, int origRows, int newColumns, int newRows) {
    // Resizes a CxR matrix at the top of the stack to C'xR'.
    int8_t elements[16] = {};
    size_t index = 0;

    size_t consumedSlots = origColumns * origRows;
    size_t zeroOffset = 0, oneOffset = 0;

    for (int c = 0; c < newColumns; ++c) {
        for (int r = 0; r < newRows; ++r) {
            if (c < origColumns && r < origRows) {
                // Push an element from the original matrix.
                elements[index++] = (c * origRows) + r;
            } else {
                // This element is outside the original matrix; push 1 or 0.
                if (c == r) {
                    // We need to synthesize a literal 1.
                    if (oneOffset == 0) {
                        this->push_literal_f(1.0f);
                        oneOffset = consumedSlots++;
                    }
                    elements[index++] = oneOffset;
                } else {
                    // We need to synthesize a literal 0.
                    if (zeroOffset == 0) {
                        this->push_zeros(1);
                        zeroOffset = consumedSlots++;
                    }
                    elements[index++] = zeroOffset;
                }
            }
        }
    }
    this->swizzle(consumedSlots, SkSpan(elements, index));
}

std::unique_ptr<Program> Builder::finish(int numValueSlots,
                                         int numUniformSlots,
                                         SkRPDebugTrace* debugTrace) {
    // Verify that calls to enableExecutionMaskWrites and disableExecutionMaskWrites are balanced.
    SkASSERT(fExecutionMaskWritesEnabled == 0);

    return std::make_unique<Program>(std::move(fInstructions), numValueSlots, numUniformSlots,
                                     fNumLabels, debugTrace);
}

void Program::optimize() {
    // TODO(johnstiles): perform any last-minute cleanup of the instruction stream here
}

static int stack_usage(const Instruction& inst) {
    switch (inst.fOp) {
        case BuilderOp::push_literal:
        case BuilderOp::push_condition_mask:
        case BuilderOp::push_loop_mask:
        case BuilderOp::push_return_mask:
            return 1;

        case BuilderOp::push_src_rgba:
        case BuilderOp::push_dst_rgba:
            return 4;

        case BuilderOp::push_slots:
        case BuilderOp::push_uniform:
        case BuilderOp::push_zeros:
        case BuilderOp::push_clone:
        case BuilderOp::push_clone_from_stack:
            return inst.fImmA;

        case BuilderOp::pop_condition_mask:
        case BuilderOp::pop_loop_mask:
        case BuilderOp::pop_and_reenable_loop_mask:
        case BuilderOp::pop_return_mask:
            return -1;

        case BuilderOp::pop_src_rg:
            return -2;

        case BuilderOp::pop_src_rgba:
        case BuilderOp::pop_dst_rgba:
            return -4;

        case ALL_N_WAY_BINARY_OP_CASES:
        case ALL_MULTI_SLOT_BINARY_OP_CASES:
        case BuilderOp::discard_stack:
        case BuilderOp::select:
            return -inst.fImmA;

        case ALL_MULTI_SLOT_TERNARY_OP_CASES:
            return 2 * -inst.fImmA;

        case BuilderOp::swizzle_1:
            return 1 - inst.fImmA;  // consumes immA slots and emits a scalar
        case BuilderOp::swizzle_2:
            return 2 - inst.fImmA;  // consumes immA slots and emits a 2-slot vector
        case BuilderOp::swizzle_3:
            return 3 - inst.fImmA;  // consumes immA slots and emits a 3-slot vector
        case BuilderOp::swizzle_4:
            return 4 - inst.fImmA;  // consumes immA slots and emits a 4-slot vector

        case BuilderOp::dot_2_floats:
            return -3;  // consumes two 2-slot vectors and emits one scalar
        case BuilderOp::dot_3_floats:
            return -5;  // consumes two 3-slot vectors and emits one scalar
        case BuilderOp::dot_4_floats:
            return -7;  // consumes two 4-slot vectors and emits one scalar

        case BuilderOp::shuffle: {
            int consumed = inst.fImmA >> 16;
            int generated = inst.fImmA & 0xFFFF;
            return generated - consumed;
        }
        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
        case ALL_MULTI_SLOT_UNARY_OP_CASES:
        default:
            return 0;
    }
}

Program::StackDepthMap Program::tempStackMaxDepths() const {
    StackDepthMap largest;
    StackDepthMap current;

    int curIdx = 0;
    for (const Instruction& inst : fInstructions) {
        if (inst.fOp == BuilderOp::set_current_stack) {
            curIdx = inst.fImmA;
        }
        current[curIdx] += stack_usage(inst);
        largest[curIdx] = std::max(current[curIdx], largest[curIdx]);
        SkASSERTF(current[curIdx] >= 0, "unbalanced temp stack push/pop on stack %d", curIdx);
    }

    for (const auto& [stackIdx, depth] : current) {
        (void)stackIdx;
        SkASSERTF(depth == 0, "unbalanced temp stack push/pop");
    }

    return largest;
}

Program::Program(SkTArray<Instruction> instrs,
                 int numValueSlots,
                 int numUniformSlots,
                 int numLabels,
                 SkRPDebugTrace* debugTrace)
        : fInstructions(std::move(instrs))
        , fNumValueSlots(numValueSlots)
        , fNumUniformSlots(numUniformSlots)
        , fNumLabels(numLabels)
        , fDebugTrace(debugTrace) {
    this->optimize();

    fTempStackMaxDepths = this->tempStackMaxDepths();

    fNumTempStackSlots = 0;
    for (const auto& [stackIdx, depth] : fTempStackMaxDepths) {
        (void)stackIdx;
        fNumTempStackSlots += depth;
    }
}

void Program::appendCopy(SkTArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         ProgramOp baseStage,
                         float* dst, int dstStride,
                         const float* src, int srcStride,
                         int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendCopy(pipeline, alloc, baseStage, dst, dstStride, src, srcStride,/*numSlots=*/4);
        dst += 4 * dstStride;
        src += 4 * srcStride;
        numSlots -= 4;
    }

    if (numSlots > 0) {
        SkASSERT(numSlots <= 4);
        auto stage = (ProgramOp)((int)baseStage + numSlots - 1);
        auto* ctx = alloc->make<SkRasterPipeline_BinaryOpCtx>();
        ctx->dst = dst;
        ctx->src = src;
        pipeline->push_back({stage, ctx});
    }
}

void Program::appendCopySlotsUnmasked(SkTArray<Stage>* pipeline,
                                      SkArenaAlloc* alloc,
                                      float* dst,
                                      const float* src,
                                      int numSlots) const {
    this->appendCopy(pipeline, alloc,
                     ProgramOp::copy_slot_unmasked,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendCopySlotsMasked(SkTArray<Stage>* pipeline,
                                    SkArenaAlloc* alloc,
                                    float* dst,
                                    const float* src,
                                    int numSlots) const {
    this->appendCopy(pipeline, alloc,
                     ProgramOp::copy_slot_masked,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendCopyConstants(SkTArray<Stage>* pipeline,
                                  SkArenaAlloc* alloc,
                                  float* dst,
                                  const float* src,
                                  int numSlots) const {
    this->appendCopy(pipeline, alloc,
                     ProgramOp::copy_constant,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/1,
                     numSlots);
}

void Program::appendSingleSlotUnaryOp(SkTArray<Stage>* pipeline, ProgramOp stage,
                                      float* dst, int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots--) {
        pipeline->push_back({stage, dst});
        dst += SkOpts::raster_pipeline_highp_stride;
    }
}

void Program::appendMultiSlotUnaryOp(SkTArray<Stage>* pipeline, ProgramOp baseStage,
                                     float* dst, int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendMultiSlotUnaryOp(pipeline, baseStage, dst, /*numSlots=*/4);
        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }

    SkASSERT(numSlots <= 4);
    auto stage = (ProgramOp)((int)baseStage + numSlots - 1);
    pipeline->push_back({stage, dst});
}

void Program::appendAdjacentNWayBinaryOp(SkTArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                         ProgramOp stage,
                                         float* dst, const float* src, int numSlots) const {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots) == src);

    if (numSlots > 0) {
        auto ctx = alloc->make<SkRasterPipeline_BinaryOpCtx>();
        ctx->dst = dst;
        ctx->src = src;
        pipeline->push_back({stage, ctx});
        return;
    }
}

void Program::appendAdjacentMultiSlotBinaryOp(SkTArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                              ProgramOp baseStage,
                                              float* dst, const float* src, int numSlots) const {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots) == src);

    if (numSlots > 4) {
        this->appendAdjacentNWayBinaryOp(pipeline, alloc, baseStage, dst, src, numSlots);
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (ProgramOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, dst});
    }
}

void Program::appendAdjacentMultiSlotTernaryOp(SkTArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                               ProgramOp baseStage, float* dst, const float* src0,
                                               const float* src1, int numSlots) const {
    // The float pointers must all be immediately adjacent to each other.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst  + SkOpts::raster_pipeline_highp_stride * numSlots) == src0);
    SkASSERT((src0 + SkOpts::raster_pipeline_highp_stride * numSlots) == src1);

    if (numSlots > 4) {
        auto ctx = alloc->make<SkRasterPipeline_TernaryOpCtx>();
        ctx->dst = dst;
        ctx->src0 = src0;
        ctx->src1 = src1;
        pipeline->push_back({baseStage, ctx});
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (ProgramOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, dst});
    }
}

void Program::appendStackRewind(SkTArray<Stage>* pipeline) const {
#if defined(SKSL_STANDALONE) || !SK_HAS_MUSTTAIL
    pipeline->push_back({ProgramOp::stack_rewind, nullptr});
#endif
}

static void* context_bit_pun(intptr_t val) {
    return sk_bit_cast<void*>(val);
}

Program::SlotData Program::allocateSlotData(SkArenaAlloc* alloc) const {
    // Allocate a contiguous slab of slot data for values and stack entries.
    const int N = SkOpts::raster_pipeline_highp_stride;
    const int vectorWidth = N * sizeof(float);
    const int allocSize = vectorWidth * (fNumValueSlots + fNumTempStackSlots);
    float* slotPtr = static_cast<float*>(alloc->makeBytesAlignedTo(allocSize, vectorWidth));
    sk_bzero(slotPtr, allocSize);

    // Store the temp stack immediately after the values.
    SlotData s;
    s.values = SkSpan{slotPtr,        N * fNumValueSlots};
    s.stack  = SkSpan{s.values.end(), N * fNumTempStackSlots};
    return s;
}

#if !defined(SKSL_STANDALONE)

bool Program::appendStages(SkRasterPipeline* pipeline,
                           SkArenaAlloc* alloc,
                           RP::Callbacks* callbacks,
                           SkSpan<const float> uniforms) const {
    // Convert our Instruction list to an array of ProgramOps.
    SkTArray<Stage> stages;
    this->makeStages(&stages, alloc, uniforms, this->allocateSlotData(alloc));

    // Allocate buffers for branch targets and labels; these are needed to convert labels into
    // actual offsets into the pipeline and fix up branches.
    SkTArray<SkRasterPipeline_BranchCtx*> branchContexts;
    branchContexts.reserve_back(fNumLabels);
    SkTArray<int> labelOffsets;
    labelOffsets.push_back_n(fNumLabels, -1);
    SkTArray<int> branchGoesToLabel;
    branchGoesToLabel.reserve_back(fNumLabels);

    for (const Stage& stage : stages) {
        switch (stage.op) {
            case ProgramOp::stack_rewind:
                pipeline->append_stack_rewind();
                break;

            case ProgramOp::invoke_shader:
                if (!callbacks || !callbacks->appendShader(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                break;

            case ProgramOp::invoke_color_filter:
                if (!callbacks || !callbacks->appendColorFilter(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                break;

            case ProgramOp::invoke_blender:
                if (!callbacks || !callbacks->appendBlender(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                break;

            case ProgramOp::label: {
                // Remember the absolute pipeline position of this label.
                int labelID = sk_bit_cast<intptr_t>(stage.ctx);
                SkASSERT(labelID >= 0 && labelID < fNumLabels);
                labelOffsets[labelID] = pipeline->getNumStages();
                break;
            }
            case ProgramOp::jump:
            case ProgramOp::branch_if_any_active_lanes:
            case ProgramOp::branch_if_no_active_lanes:
            case ProgramOp::branch_if_no_active_lanes_eq: {
                // The branch context contain a valid label ID at this point.
                auto* branchCtx = static_cast<SkRasterPipeline_BranchCtx*>(stage.ctx);
                int labelID = branchCtx->offset;
                SkASSERT(labelID >= 0 && labelID < fNumLabels);

                // Replace the label ID in the branch context with the absolute pipeline position.
                // We will go back over the branch targets at the end and fix them up.
                branchCtx->offset = pipeline->getNumStages();

                SkASSERT(branchContexts.size() == branchGoesToLabel.size());
                branchContexts.push_back(branchCtx);
                branchGoesToLabel.push_back(labelID);
                [[fallthrough]];
            }
            default:
                // Append a regular op to the program.
                SkASSERT((int)stage.op < kNumRasterPipelineHighpOps);
                pipeline->append((SkRasterPipelineOp)stage.op, stage.ctx);
                break;
        }
    }

    // Now that we have assembled the program and know the pipeline positions of each label and
    // branch, fix up every branch target.
    SkASSERT(branchContexts.size() == branchGoesToLabel.size());
    for (int index = 0; index < branchContexts.size(); ++index) {
        int branchFromIdx = branchContexts[index]->offset;
        int branchToIdx = labelOffsets[branchGoesToLabel[index]];
        branchContexts[index]->offset = branchToIdx - branchFromIdx;
    }

    return true;
}

#endif

void Program::makeStages(SkTArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         SkSpan<const float> uniforms,
                         const SlotData& slots) const {
    SkASSERT(fNumUniformSlots == SkToInt(uniforms.size()));

    const int N = SkOpts::raster_pipeline_highp_stride;
    StackDepthMap tempStackDepth;
    int currentStack = 0;
    int mostRecentRewind = 0;

    // Assemble a map holding the current stack-top for each temporary stack. Position each temp
    // stack immediately after the previous temp stack; temp stacks are never allowed to overlap.
    int pos = 0;
    SkTHashMap<int, float*> tempStackMap;
    for (auto& [idx, depth] : fTempStackMaxDepths) {
        tempStackMap[idx] = slots.stack.begin() + (pos * N);
        pos += depth;
    }

    // Track labels that we have reached in processing.
    SkBitSet labelsEncountered(fNumLabels);

    auto EmitStackRewindForBackwardsBranch = [&](int labelID) {
        // If we have already encountered the label associated with this branch, this is a
        // backwards branch. Add a stack-rewind immediately before the branch to ensure that
        // long-running loops don't use an unbounded amount of stack space.
        if (labelsEncountered.test(labelID)) {
            this->appendStackRewind(pipeline);
            mostRecentRewind = pipeline->size();
        }
    };

    // We can reuse constants from our arena by placing them in this map.
    SkTHashMap<int, int*> constantLookupMap; // <constant value, pointer into arena>

    // Write each BuilderOp to the pipeline array.
    pipeline->reserve_back(fInstructions.size());
    for (const Instruction& inst : fInstructions) {
        auto SlotA    = [&]() { return &slots.values[N * inst.fSlotA]; };
        auto SlotB    = [&]() { return &slots.values[N * inst.fSlotB]; };
        auto UniformA = [&]() { return &uniforms[inst.fSlotA]; };
        float*& tempStackPtr = tempStackMap[currentStack];

        switch (inst.fOp) {
            case BuilderOp::label:
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                labelsEncountered.set(inst.fImmA);
                pipeline->push_back({ProgramOp::label, context_bit_pun(inst.fImmA)});
                break;

            case BuilderOp::jump:
            case BuilderOp::branch_if_any_active_lanes:
            case BuilderOp::branch_if_no_active_lanes: {
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                EmitStackRewindForBackwardsBranch(inst.fImmA);

                auto* ctx = alloc->make<SkRasterPipeline_BranchCtx>();
                ctx->offset = inst.fImmA;
                pipeline->push_back({(ProgramOp)inst.fOp, ctx});
                break;
            }
            case BuilderOp::branch_if_no_active_lanes_on_stack_top_equal: {
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                EmitStackRewindForBackwardsBranch(inst.fImmA);

                auto* ctx = alloc->make<SkRasterPipeline_BranchIfEqualCtx>();
                ctx->offset = inst.fImmA;
                ctx->value = inst.fImmB;
                ctx->ptr = reinterpret_cast<int*>(tempStackPtr - N);
                pipeline->push_back({ProgramOp::branch_if_no_active_lanes_eq, ctx});
                break;
            }
            case BuilderOp::init_lane_masks:
                pipeline->push_back({ProgramOp::init_lane_masks, nullptr});
                break;

            case BuilderOp::store_src_rg:
                pipeline->push_back({ProgramOp::store_src_rg, SlotA()});
                break;

            case BuilderOp::store_src:
                pipeline->push_back({ProgramOp::store_src, SlotA()});
                break;

            case BuilderOp::store_dst:
                pipeline->push_back({ProgramOp::store_dst, SlotA()});
                break;

            case BuilderOp::store_device_xy01:
                pipeline->push_back({ProgramOp::store_device_xy01, SlotA()});
                break;

            case BuilderOp::load_src:
                pipeline->push_back({ProgramOp::load_src, SlotA()});
                break;

            case BuilderOp::load_dst:
                pipeline->push_back({ProgramOp::load_dst, SlotA()});
                break;

            case ALL_SINGLE_SLOT_UNARY_OP_CASES: {
                float* dst = tempStackPtr - (inst.fImmA * N);
                this->appendSingleSlotUnaryOp(pipeline, (ProgramOp)inst.fOp, dst, inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_UNARY_OP_CASES: {
                float* dst = tempStackPtr - (inst.fImmA * N);
                this->appendMultiSlotUnaryOp(pipeline, (ProgramOp)inst.fOp, dst, inst.fImmA);
                break;
            }
            case ALL_N_WAY_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendAdjacentNWayBinaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                                 dst, src, inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendAdjacentMultiSlotBinaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                                      dst, src, inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_TERNARY_OP_CASES: {
                float* src1 = tempStackPtr - (inst.fImmA * N);
                float* src0 = tempStackPtr - (inst.fImmA * 2 * N);
                float* dst  = tempStackPtr - (inst.fImmA * 3 * N);
                this->appendAdjacentMultiSlotTernaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                                       dst, src0, src1, inst.fImmA);
                break;
            }
            case BuilderOp::select: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendCopySlotsMasked(pipeline, alloc, dst, src, inst.fImmA);
                break;
            }
            case BuilderOp::copy_slot_masked:
                this->appendCopySlotsMasked(pipeline, alloc, SlotA(), SlotB(), inst.fImmA);
                break;

            case BuilderOp::copy_slot_unmasked:
                this->appendCopySlotsUnmasked(pipeline, alloc, SlotA(), SlotB(), inst.fImmA);
                break;

            case BuilderOp::zero_slot_unmasked:
                this->appendMultiSlotUnaryOp(pipeline, ProgramOp::zero_slot_unmasked,
                                             SlotA(), inst.fImmA);
                break;

            case BuilderOp::dot_2_floats:
            case BuilderOp::dot_3_floats:
            case BuilderOp::dot_4_floats: {
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                pipeline->push_back({(ProgramOp)inst.fOp, dst});
                break;
            }
            case BuilderOp::swizzle_1:
            case BuilderOp::swizzle_2:
            case BuilderOp::swizzle_3:
            case BuilderOp::swizzle_4: {
                auto* ctx = alloc->make<SkRasterPipeline_SwizzleCtx>();
                ctx->ptr = tempStackPtr - (N * inst.fImmA);
                // Unpack component nybbles into byte-offsets pointing at stack slots.
                unpack_nybbles_to_offsets(inst.fImmB, SkSpan(ctx->offsets));
                pipeline->push_back({(ProgramOp)inst.fOp, ctx});
                break;
            }
            case BuilderOp::shuffle: {
                int consumed = inst.fImmA >> 16;
                int generated = inst.fImmA & 0xFFFF;

                auto* ctx = alloc->make<SkRasterPipeline_ShuffleCtx>();
                ctx->ptr = tempStackPtr - (N * consumed);
                ctx->count = generated;
                // Unpack immB and immC from nybble form into the offset array.
                unpack_nybbles_to_offsets(inst.fImmB, SkSpan(&ctx->offsets[0], 8));
                unpack_nybbles_to_offsets(inst.fImmC, SkSpan(&ctx->offsets[8], 8));
                pipeline->push_back({ProgramOp::shuffle, ctx});
                break;
            }
            case BuilderOp::push_src_rgba: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_src, dst});
                break;
            }
            case BuilderOp::push_dst_rgba: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_dst, dst});
                break;
            }
            case BuilderOp::pop_src_rg: {
                float* dst = tempStackPtr - (2 * N);
                pipeline->push_back({ProgramOp::load_src_rg, dst});
                break;
            }
            case BuilderOp::pop_src_rgba: {
                float* dst = tempStackPtr - (4 * N);
                pipeline->push_back({ProgramOp::load_src, dst});
                break;
            }
            case BuilderOp::pop_dst_rgba: {
                float* dst = tempStackPtr - (4 * N);
                pipeline->push_back({ProgramOp::load_dst, dst});
                break;
            }
            case BuilderOp::push_slots: {
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc, dst, SlotA(), inst.fImmA);
                break;
            }
            case BuilderOp::push_uniform: {
                float* dst = tempStackPtr;
                this->appendCopyConstants(pipeline, alloc, dst, UniformA(), inst.fImmA);
                break;
            }
            case BuilderOp::push_zeros: {
                float* dst = tempStackPtr;
                this->appendMultiSlotUnaryOp(pipeline, ProgramOp::zero_slot_unmasked, dst,
                                             inst.fImmA);
                break;
            }
            case BuilderOp::push_condition_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_condition_mask, dst});
                break;
            }
            case BuilderOp::pop_condition_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({ProgramOp::load_condition_mask, src});
                break;
            }
            case BuilderOp::merge_condition_mask: {
                float* ptr = tempStackPtr - (2 * N);
                pipeline->push_back({ProgramOp::merge_condition_mask, ptr});
                break;
            }
            case BuilderOp::push_loop_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_loop_mask, dst});
                break;
            }
            case BuilderOp::pop_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({ProgramOp::load_loop_mask, src});
                break;
            }
            case BuilderOp::pop_and_reenable_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({ProgramOp::reenable_loop_mask, src});
                break;
            }
            case BuilderOp::reenable_loop_mask:
                pipeline->push_back({ProgramOp::reenable_loop_mask, SlotA()});
                break;

            case BuilderOp::mask_off_loop_mask:
                pipeline->push_back({ProgramOp::mask_off_loop_mask, nullptr});
                break;

            case BuilderOp::merge_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({ProgramOp::merge_loop_mask, src});
                break;
            }
            case BuilderOp::push_return_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_return_mask, dst});
                break;
            }
            case BuilderOp::pop_return_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({ProgramOp::load_return_mask, src});
                break;
            }
            case BuilderOp::mask_off_return_mask:
                pipeline->push_back({ProgramOp::mask_off_return_mask, nullptr});
                break;

            case BuilderOp::copy_constant:
            case BuilderOp::push_literal: {
                float* dst = (inst.fOp == BuilderOp::push_literal) ? tempStackPtr : SlotA();
                int* constantPtr;
                if (int** lookup = constantLookupMap.find(inst.fImmA)) {
                    constantPtr = *lookup;
                } else {
                    constantPtr = alloc->make<int>(inst.fImmA);
                    constantLookupMap[inst.fImmA] = constantPtr;
                }
                SkASSERT(constantPtr);
                this->appendCopyConstants(pipeline, alloc, dst, (float*)constantPtr,/*numSlots=*/1);
                break;
            }
            case BuilderOp::copy_stack_to_slots: {
                float* src = tempStackPtr - (inst.fImmB * N);
                this->appendCopySlotsMasked(pipeline, alloc, SlotA(), src, inst.fImmA);
                break;
            }
            case BuilderOp::copy_stack_to_slots_unmasked: {
                float* src = tempStackPtr - (inst.fImmB * N);
                this->appendCopySlotsUnmasked(pipeline, alloc, SlotA(), src, inst.fImmA);
                break;
            }
            case BuilderOp::swizzle_copy_stack_to_slots: {
                auto stage = (ProgramOp)((int)ProgramOp::swizzle_copy_slot_masked + inst.fImmA - 1);
                auto* ctx = alloc->make<SkRasterPipeline_SwizzleCopyCtx>();
                ctx->src = tempStackPtr - (inst.fImmB * N);
                ctx->dst = SlotA();
                unpack_nybbles_to_offsets(inst.fImmC, SkSpan(ctx->offsets));
                pipeline->push_back({stage, ctx});
                break;
            }
            case BuilderOp::push_clone: {
                float* src = tempStackPtr - (inst.fImmB * N);
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc, dst, src, inst.fImmA);
                break;
            }
            case BuilderOp::push_clone_from_stack: {
                float* sourceStackPtr = tempStackMap[inst.fImmB];
                float* src = sourceStackPtr - (inst.fImmC * N);
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc, dst, src, inst.fImmA);
                break;
            }
            case BuilderOp::case_op: {
                auto* ctx = alloc->make<SkRasterPipeline_CaseOpCtx>();
                ctx->ptr = reinterpret_cast<int*>(tempStackPtr - 2 * N);
                ctx->expectedValue = inst.fImmA;
                pipeline->push_back({ProgramOp::case_op, ctx});
                break;
            }
            case BuilderOp::discard_stack:
                break;

            case BuilderOp::set_current_stack:
                currentStack = inst.fImmA;
                break;

            case BuilderOp::invoke_shader:
            case BuilderOp::invoke_color_filter:
            case BuilderOp::invoke_blender:
                pipeline->push_back({(ProgramOp)inst.fOp, context_bit_pun(inst.fImmA)});
                break;

            default:
                SkDEBUGFAILF("Raster Pipeline: unsupported instruction %d", (int)inst.fOp);
                break;
        }

        tempStackPtr += stack_usage(inst) * N;
        SkASSERT(tempStackPtr >= slots.stack.begin());
        SkASSERT(tempStackPtr <= slots.stack.end());

        // Periodically rewind the stack every 500 instructions. When SK_HAS_MUSTTAIL is set,
        // rewinds are not actually used; the appendStackRewind call becomes a no-op. On platforms
        // that don't support SK_HAS_MUSTTAIL, rewinding the stack periodically can prevent a
        // potential stack overflow when running a long program.
        int numPipelineStages = pipeline->size();
        if (numPipelineStages - mostRecentRewind > 500) {
            this->appendStackRewind(pipeline);
            mostRecentRewind = numPipelineStages;
        }
    }
}

// Finds duplicate names in the program and disambiguates them with subscripts.
SkTArray<std::string> build_unique_slot_name_list(const SkRPDebugTrace* debugTrace) {
    SkTArray<std::string> slotName;
    if (debugTrace) {
        slotName.reserve_back(debugTrace->fSlotInfo.size());

        // The map consists of <variable name, <source position, unique name>>.
        SkTHashMap<std::string_view, SkTHashMap<int, std::string>> uniqueNameMap;

        for (const SlotDebugInfo& slotInfo : debugTrace->fSlotInfo) {
            // Look up this variable by its name and source position.
            int pos = slotInfo.pos.valid() ? slotInfo.pos.startOffset() : 0;
            SkTHashMap<int, std::string>& positionMap = uniqueNameMap[slotInfo.name];
            std::string& uniqueName = positionMap[pos];

            // Have we seen this variable name/position combination before?
            if (uniqueName.empty()) {
                // This is a unique name/position pair.
                uniqueName = slotInfo.name;

                // But if it's not a unique _name_, it deserves a subscript to disambiguate it.
                int subscript = positionMap.count() - 1;
                if (subscript > 0) {
                    for (char digit : std::to_string(subscript)) {
                        // U+2080 through U+2089 (₀₁₂₃₄₅₆₇₈₉) in UTF8:
                        uniqueName.push_back((char)0xE2);
                        uniqueName.push_back((char)0x82);
                        uniqueName.push_back((char)(0x80 + digit - '0'));
                    }
                }
            }

            slotName.push_back(uniqueName);
        }
    }
    return slotName;
}

void Program::dump(SkWStream* out) const {
    // Allocate memory for the slot and uniform data, even though the program won't ever be
    // executed. The program requires pointer ranges for managing its data, and ASAN will report
    // errors if those pointers are pointing at unallocated memory.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    const int N = SkOpts::raster_pipeline_highp_stride;
    SlotData slots = this->allocateSlotData(&alloc);
    float* uniformPtr = alloc.makeArray<float>(fNumUniformSlots);
    SkSpan<float> uniforms = SkSpan(uniformPtr, fNumUniformSlots);

    // Turn this program into an array of Raster Pipeline stages.
    SkTArray<Stage> stages;
    this->makeStages(&stages, &alloc, uniforms, slots);

    // Find the labels in the program, and keep track of their offsets.
    SkTHashMap<int, int> labelToStageMap; // <label ID, stage index>
    for (int index = 0; index < stages.size(); ++index) {
        if (stages[index].op == ProgramOp::label) {
            int labelID = sk_bit_cast<intptr_t>(stages[index].ctx);
            SkASSERT(!labelToStageMap.find(labelID));
            labelToStageMap[labelID] = index;
        }
    }

    // Assign unique names to each variable slot; our trace might have multiple variables with the
    // same name, which can make a dump hard to read.
    SkTArray<std::string> slotName = build_unique_slot_name_list(fDebugTrace);

    // Emit the program's instruction list.
    for (int index = 0; index < stages.size(); ++index) {
        const Stage& stage = stages[index];

        // Interpret the context value as a branch offset.
        auto BranchOffset = [&](const SkRasterPipeline_BranchCtx* ctx) -> std::string {
            // The context's offset field contains a label ID
            int labelID = ctx->offset;
            SkASSERT(labelToStageMap.find(labelID));
            int labelIndex = labelToStageMap[labelID];
            return SkSL::String::printf("%+d (label %d at #%d)",
                                        labelIndex - index, labelID, labelIndex + 1);
        };

        // Print a 32-bit immediate value of unknown type (int/float).
        auto Imm = [&](float immFloat, bool showAsFloat = true) -> std::string {
            // Start with `0x3F800000` as a baseline.
            uint32_t immUnsigned;
            memcpy(&immUnsigned, &immFloat, sizeof(uint32_t));
            auto text = SkSL::String::printf("0x%08X", immUnsigned);

            // Extend it to `0x3F800000 (1.0)` for finite floating point values.
            if (showAsFloat && std::isfinite(immFloat)) {
                text += " (";
                text += skstd::to_string(immFloat);
                text += ")";
            }
            return text;
        };

        // Interpret the context pointer as a 32-bit immediate value of unknown type (int/float).
        auto ImmCtx = [&](const void* ctx, bool showAsFloat = true) -> std::string {
            float f;
            memcpy(&f, &ctx, sizeof(float));
            return Imm(f, showAsFloat);
        };

        // Print `1` for single slots and `1..3` for ranges of slots.
        auto AsRange = [](int first, int count) -> std::string {
            std::string text = std::to_string(first);
            if (count > 1) {
                text += ".." + std::to_string(first + count - 1);
            }
            return text;
        };

        // Come up with a reasonable name for a range of slots, e.g.:
        // `val`: slot range points at one variable, named val
        // `val(0..1)`: slot range points at the first and second slot of val (which has 3+ slots)
        // `foo, bar`: slot range fully covers two variables, named foo and bar
        // `foo(3), bar(0)`: slot range covers the fourth slot of foo and the first slot of bar
        auto SlotName = [&](SkSpan<const SlotDebugInfo> debugInfo,
                            SkSpan<const std::string> names,
                            SlotRange range) -> std::string {
            SkASSERT(range.index >= 0 && (range.index + range.count) <= (int)debugInfo.size());

            std::string text;
            auto separator = SkSL::String::Separator();
            while (range.count > 0) {
                const SlotDebugInfo& slotInfo = debugInfo[range.index];
                text += separator();
                text += names.empty() ? slotInfo.name : names[range.index];

                // Figure out how many slots we can chomp in this iteration.
                int entireVariable = slotInfo.columns * slotInfo.rows;
                int slotsToChomp = std::min(range.count, entireVariable - slotInfo.componentIndex);
                // If we aren't consuming an entire variable, from first slot to last...
                if (slotsToChomp != entireVariable) {
                    // ... decorate it with a range suffix.
                    text += "(" + AsRange(slotInfo.componentIndex, slotsToChomp) + ")";
                }
                range.index += slotsToChomp;
                range.count -= slotsToChomp;
            }

            return text;
        };

        // Attempts to interpret the passed-in pointer as a uniform range.
        auto UniformPtrCtx = [&](const float* ptr, int numSlots) -> std::string {
            const float* end = ptr + numSlots;
            if (ptr >= uniforms.begin() && end <= uniforms.end()) {
                int uniformIdx = ptr - uniforms.begin();
                if (fDebugTrace) {
                    // Handle pointers to named uniform slots.
                    std::string name = SlotName(fDebugTrace->fUniformInfo, /*names=*/{},
                                                {uniformIdx, numSlots});
                    if (!name.empty()) {
                        return name;
                    }
                }
                // Handle pointers to uniforms (when no debug info exists).
                return "u" + AsRange(uniformIdx, numSlots);
            }
            return {};
        };

        // Attempts to interpret the passed-in pointer as a value slot range.
        auto ValuePtrCtx = [&](const float* ptr, int numSlots) -> std::string {
            const float* end = ptr + (N * numSlots);
            if (ptr >= slots.values.begin() && end <= slots.values.end()) {
                int valueIdx = ptr - slots.values.begin();
                SkASSERT((valueIdx % N) == 0);
                valueIdx /= N;
                if (fDebugTrace) {
                    // Handle pointers to named value slots.
                    std::string name = SlotName(fDebugTrace->fSlotInfo, slotName,
                                                {valueIdx, numSlots});
                    if (!name.empty()) {
                        return name;
                    }
                }
                // Handle pointers to value slots (when no debug info exists).
                return "v" + AsRange(valueIdx, numSlots);
            }
            return {};
        };

        // Interpret the context value as a pointer to `count` immediate values.
        auto MultiImmCtx = [&](const float* ptr, int count) -> std::string {
            // If this is a uniform, print it by name.
            if (std::string text = UniformPtrCtx(ptr, count); !text.empty()) {
                return text;
            }
            // Emit a single unbracketed immediate.
            if (count == 1) {
                return Imm(*ptr);
            }
            // Emit a list like `[0x00000000 (0.0), 0x3F80000 (1.0)]`.
            std::string text = "[";
            auto separator = SkSL::String::Separator();
            while (count--) {
                text += separator();
                text += Imm(*ptr++);
            }
            return text + "]";
        };

        // Interpret the context value as a generic pointer.
        auto PtrCtx = [&](const void* ctx, int numSlots) -> std::string {
            const float *ctxAsSlot = static_cast<const float*>(ctx);
            // Check for uniform and value pointers.
            if (std::string uniform = UniformPtrCtx(ctxAsSlot, numSlots); !uniform.empty()) {
                return uniform;
            }
            if (std::string value = ValuePtrCtx(ctxAsSlot, numSlots); !value.empty()) {
                return value;
            }
            // Handle pointers to temporary stack slots.
            if (ctxAsSlot >= slots.stack.begin() && ctxAsSlot < slots.stack.end()) {
                int stackIdx = ctxAsSlot - slots.stack.begin();
                SkASSERT((stackIdx % N) == 0);
                return "$" + AsRange(stackIdx / N, numSlots);
            }
            // This pointer is out of our expected bounds; this generally isn't expected to happen.
            return "ExternalPtr(" + AsRange(0, numSlots) + ")";
        };

        // Interpret the context value as a pointer to two adjacent values.
        auto AdjacentPtrCtx = [&](const void* ctx,
                                  int numSlots) -> std::tuple<std::string, std::string> {
            const float *ctxAsSlot = static_cast<const float*>(ctx);
            return std::make_tuple(PtrCtx(ctxAsSlot, numSlots),
                                   PtrCtx(ctxAsSlot + (N * numSlots), numSlots));
        };

        // Interpret the context value as a pointer to three adjacent values.
        auto Adjacent3PtrCtx = [&](const void* ctx, int numSlots) ->
                                  std::tuple<std::string, std::string, std::string> {
            const float *ctxAsSlot = static_cast<const float*>(ctx);
            return std::make_tuple(PtrCtx(ctxAsSlot, numSlots),
                                   PtrCtx(ctxAsSlot + (N * numSlots), numSlots),
                                   PtrCtx(ctxAsSlot + (2 * N * numSlots), numSlots));
        };

        // Interpret the context value as a BinaryOp structure for copy_n_slots (numSlots is
        // dictated by the op itself).
        auto BinaryOpCtx = [&](const void* v,
                               int numSlots) -> std::tuple<std::string, std::string> {
            const auto *ctx = static_cast<const SkRasterPipeline_BinaryOpCtx*>(v);
            return std::make_tuple(PtrCtx(ctx->dst, numSlots),
                                   PtrCtx(ctx->src, numSlots));
        };

        // Interpret the context value as a BinaryOp structure for copy_n_constants (numSlots is
        // dictated by the op itself).
        auto CopyConstantCtx = [&](const void* v,
                                   int numSlots) -> std::tuple<std::string, std::string> {
            const auto *ctx = static_cast<const SkRasterPipeline_BinaryOpCtx*>(v);
            return std::make_tuple(PtrCtx(ctx->dst, numSlots),
                                   MultiImmCtx(ctx->src, numSlots));
        };

        // Interpret the context value as a BinaryOp structure (numSlots is inferred from the
        // distance between pointers).
        auto AdjacentBinaryOpCtx = [&](const void* v) -> std::tuple<std::string, std::string> {
            const auto *ctx = static_cast<const SkRasterPipeline_BinaryOpCtx*>(v);
            int numSlots = (ctx->src - ctx->dst) / N;
            return AdjacentPtrCtx(ctx->dst, numSlots);
        };

        // Interpret the context value as a TernaryOp structure (numSlots is inferred from the
        // distance between pointers).
        auto AdjacentTernaryOpCtx = [&](const void* v) ->
                                       std::tuple<std::string, std::string, std::string> {
            const auto* ctx = static_cast<const SkRasterPipeline_TernaryOpCtx*>(v);
            int numSlots = (ctx->src0 - ctx->dst) / N;
            return Adjacent3PtrCtx(ctx->dst, numSlots);
        };

        // Stringize a swizzled pointer. Note that the slot-width of the original expression is not
        // preserved in the instruction encoding, so we need to do our best using the data we have.
        // (e.g., myFloat4.y would be indistinguishable from myFloat2.y.)
        auto SwizzlePtr = [&](const float* ptr, SkSpan<const uint16_t> offsets) {
            size_t highestComponent = *std::max_element(offsets.begin(), offsets.end()) /
                                      (N * sizeof(float));

            std::string src = "(" + PtrCtx(ptr, std::max(offsets.size(), highestComponent + 1)) +
                              ").";
            for (uint16_t offset : offsets) {
                if (offset == (0 * N * sizeof(float))) {
                    src.push_back('x');
                } else if (offset == (1 * N * sizeof(float))) {
                    src.push_back('y');
                } else if (offset == (2 * N * sizeof(float))) {
                    src.push_back('z');
                } else if (offset == (3 * N * sizeof(float))) {
                    src.push_back('w');
                } else {
                    src.push_back('?');
                }
            }
            return src;
        };

        // Interpret the context value as a Swizzle structure.
        auto SwizzleCtx = [&](ProgramOp op, const void* v) -> std::tuple<std::string, std::string> {
            const auto* ctx = static_cast<const SkRasterPipeline_SwizzleCtx*>(v);
            int destSlots = (int)op - (int)BuilderOp::swizzle_1 + 1;

            return std::make_tuple(PtrCtx(ctx->ptr, destSlots),
                                   SwizzlePtr(ctx->ptr, SkSpan(ctx->offsets, destSlots)));
        };

        // Interpret the context value as a SwizzleCopy structure.
        auto SwizzleCopyCtx = [&](ProgramOp op,
                                  const void* v) -> std::tuple<std::string, std::string> {
            const auto* ctx = static_cast<const SkRasterPipeline_SwizzleCopyCtx*>(v);
            int destSlots = (int)op - (int)BuilderOp::swizzle_copy_slot_masked + 1;

            return std::make_tuple(SwizzlePtr(ctx->dst, SkSpan(ctx->offsets, destSlots)),
                                   PtrCtx(ctx->src, destSlots));
        };

        // Interpret the context value as a Shuffle structure.
        auto ShuffleCtx = [&](const void* v) -> std::tuple<std::string, std::string> {
            const auto* ctx = static_cast<const SkRasterPipeline_ShuffleCtx*>(v);

            std::string dst = PtrCtx(ctx->ptr, ctx->count);
            std::string src = "(" + dst + ")[";
            for (int index = 0; index < ctx->count; ++index) {
                if (ctx->offsets[index] % (N * sizeof(float))) {
                    src.push_back('?');
                } else {
                    src += std::to_string(ctx->offsets[index] / (N * sizeof(float)));
                }
                src.push_back(' ');
            }
            src.back() = ']';
            return std::make_tuple(dst, src);
        };

        std::string opArg1, opArg2, opArg3;
        using POp = ProgramOp;
        switch (stage.op) {
            case POp::label:
            case POp::invoke_shader:
            case POp::invoke_color_filter:
            case POp::invoke_blender:
                opArg1 = ImmCtx(stage.ctx, /*showAsFloat=*/false);
                break;

            case POp::case_op: {
                const auto* ctx = static_cast<SkRasterPipeline_CaseOpCtx*>(stage.ctx);
                opArg1 = PtrCtx(ctx->ptr, 1);
                opArg2 = PtrCtx(ctx->ptr + N, 1);
                opArg3 = Imm(sk_bit_cast<float>(ctx->expectedValue), /*showAsFloat=*/false);
                break;
            }
            case POp::swizzle_1:
            case POp::swizzle_2:
            case POp::swizzle_3:
            case POp::swizzle_4:
                std::tie(opArg1, opArg2) = SwizzleCtx(stage.op, stage.ctx);
                break;

            case POp::swizzle_copy_slot_masked:
            case POp::swizzle_copy_2_slots_masked:
            case POp::swizzle_copy_3_slots_masked:
            case POp::swizzle_copy_4_slots_masked:
                std::tie(opArg1, opArg2) = SwizzleCopyCtx(stage.op, stage.ctx);
                break;

            case POp::dot_2_floats:
                opArg1 = PtrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = AdjacentPtrCtx(stage.ctx, 2);
                break;

            case POp::dot_3_floats:
                opArg1 = PtrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = AdjacentPtrCtx(stage.ctx, 3);
                break;

            case POp::dot_4_floats:
                opArg1 = PtrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = AdjacentPtrCtx(stage.ctx, 4);
                break;

            case POp::shuffle:
                std::tie(opArg1, opArg2) = ShuffleCtx(stage.ctx);
                break;

            case POp::load_condition_mask:
            case POp::store_condition_mask:
            case POp::load_loop_mask:
            case POp::store_loop_mask:
            case POp::merge_loop_mask:
            case POp::reenable_loop_mask:
            case POp::load_return_mask:
            case POp::store_return_mask:
            case POp::zero_slot_unmasked:
            case POp::bitwise_not_int:
            case POp::cast_to_float_from_int: case POp::cast_to_float_from_uint:
            case POp::cast_to_int_from_float: case POp::cast_to_uint_from_float:
            case POp::abs_float:              case POp::abs_int:
            case POp::atan_float:
            case POp::ceil_float:
            case POp::cos_float:
            case POp::exp_float:
            case POp::floor_float:
            case POp::sin_float:
            case POp::sqrt_float:
            case POp::tan_float:
                opArg1 = PtrCtx(stage.ctx, 1);
                break;

            case POp::zero_2_slots_unmasked:
            case POp::bitwise_not_2_ints:
            case POp::load_src_rg:               case POp::store_src_rg:
            case POp::cast_to_float_from_2_ints: case POp::cast_to_float_from_2_uints:
            case POp::cast_to_int_from_2_floats: case POp::cast_to_uint_from_2_floats:
            case POp::abs_2_floats:              case POp::abs_2_ints:
            case POp::ceil_2_floats:
            case POp::floor_2_floats:
                opArg1 = PtrCtx(stage.ctx, 2);
                break;

            case POp::zero_3_slots_unmasked:
            case POp::bitwise_not_3_ints:
            case POp::cast_to_float_from_3_ints: case POp::cast_to_float_from_3_uints:
            case POp::cast_to_int_from_3_floats: case POp::cast_to_uint_from_3_floats:
            case POp::abs_3_floats:              case POp::abs_3_ints:
            case POp::ceil_3_floats:
            case POp::floor_3_floats:
                opArg1 = PtrCtx(stage.ctx, 3);
                break;

            case POp::load_src:
            case POp::load_dst:
            case POp::store_src:
            case POp::store_dst:
            case POp::store_device_xy01:
            case POp::zero_4_slots_unmasked:
            case POp::bitwise_not_4_ints:
            case POp::cast_to_float_from_4_ints: case POp::cast_to_float_from_4_uints:
            case POp::cast_to_int_from_4_floats: case POp::cast_to_uint_from_4_floats:
            case POp::abs_4_floats:              case POp::abs_4_ints:
            case POp::ceil_4_floats:
            case POp::floor_4_floats:
                opArg1 = PtrCtx(stage.ctx, 4);
                break;

            case POp::copy_constant:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 1);
                break;

            case POp::copy_2_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 2);
                break;

            case POp::copy_3_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 3);
                break;

            case POp::copy_4_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 4);
                break;

            case POp::copy_slot_masked:
            case POp::copy_slot_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 1);
                break;

            case POp::copy_2_slots_masked:
            case POp::copy_2_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 2);
                break;

            case POp::copy_3_slots_masked:
            case POp::copy_3_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 3);
                break;

            case POp::copy_4_slots_masked:
            case POp::copy_4_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 4);
                break;

            case POp::merge_condition_mask:
            case POp::add_float:   case POp::add_int:
            case POp::sub_float:   case POp::sub_int:
            case POp::mul_float:   case POp::mul_int:
            case POp::div_float:   case POp::div_int:   case POp::div_uint:
                                   case POp::bitwise_and_int:
                                   case POp::bitwise_or_int:
                                   case POp::bitwise_xor_int:
            case POp::min_float:   case POp::min_int:   case POp::min_uint:
            case POp::max_float:   case POp::max_int:   case POp::max_uint:
            case POp::cmplt_float: case POp::cmplt_int: case POp::cmplt_uint:
            case POp::cmple_float: case POp::cmple_int: case POp::cmple_uint:
            case POp::cmpeq_float: case POp::cmpeq_int:
            case POp::cmpne_float: case POp::cmpne_int:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 1);
                break;

            case POp::mix_float:   case POp::mix_int:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 1);
                break;

            case POp::add_2_floats:   case POp::add_2_ints:
            case POp::sub_2_floats:   case POp::sub_2_ints:
            case POp::mul_2_floats:   case POp::mul_2_ints:
            case POp::div_2_floats:   case POp::div_2_ints:   case POp::div_2_uints:
                                      case POp::bitwise_and_2_ints:
                                      case POp::bitwise_or_2_ints:
                                      case POp::bitwise_xor_2_ints:
            case POp::min_2_floats:   case POp::min_2_ints:   case POp::min_2_uints:
            case POp::max_2_floats:   case POp::max_2_ints:   case POp::max_2_uints:
            case POp::cmplt_2_floats: case POp::cmplt_2_ints: case POp::cmplt_2_uints:
            case POp::cmple_2_floats: case POp::cmple_2_ints: case POp::cmple_2_uints:
            case POp::cmpeq_2_floats: case POp::cmpeq_2_ints:
            case POp::cmpne_2_floats: case POp::cmpne_2_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 2);
                break;

            case POp::mix_2_floats:   case POp::mix_2_ints:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 2);
                break;

            case POp::add_3_floats:   case POp::add_3_ints:
            case POp::sub_3_floats:   case POp::sub_3_ints:
            case POp::mul_3_floats:   case POp::mul_3_ints:
            case POp::div_3_floats:   case POp::div_3_ints:   case POp::div_3_uints:
                                      case POp::bitwise_and_3_ints:
                                      case POp::bitwise_or_3_ints:
                                      case POp::bitwise_xor_3_ints:
            case POp::min_3_floats:   case POp::min_3_ints:   case POp::min_3_uints:
            case POp::max_3_floats:   case POp::max_3_ints:   case POp::max_3_uints:
            case POp::cmplt_3_floats: case POp::cmplt_3_ints: case POp::cmplt_3_uints:
            case POp::cmple_3_floats: case POp::cmple_3_ints: case POp::cmple_3_uints:
            case POp::cmpeq_3_floats: case POp::cmpeq_3_ints:
            case POp::cmpne_3_floats: case POp::cmpne_3_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 3);
                break;

            case POp::mix_3_floats:   case POp::mix_3_ints:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 3);
                break;

            case POp::add_4_floats:   case POp::add_4_ints:
            case POp::sub_4_floats:   case POp::sub_4_ints:
            case POp::mul_4_floats:   case POp::mul_4_ints:
            case POp::div_4_floats:   case POp::div_4_ints:   case POp::div_4_uints:
                                      case POp::bitwise_and_4_ints:
                                      case POp::bitwise_or_4_ints:
                                      case POp::bitwise_xor_4_ints:
            case POp::min_4_floats:   case POp::min_4_ints:   case POp::min_4_uints:
            case POp::max_4_floats:   case POp::max_4_ints:   case POp::max_4_uints:
            case POp::cmplt_4_floats: case POp::cmplt_4_ints: case POp::cmplt_4_uints:
            case POp::cmple_4_floats: case POp::cmple_4_ints: case POp::cmple_4_uints:
            case POp::cmpeq_4_floats: case POp::cmpeq_4_ints:
            case POp::cmpne_4_floats: case POp::cmpne_4_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 4);
                break;

            case POp::mix_4_floats:   case POp::mix_4_ints:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 4);
                break;

            case POp::add_n_floats:   case POp::add_n_ints:
            case POp::sub_n_floats:   case POp::sub_n_ints:
            case POp::mul_n_floats:   case POp::mul_n_ints:
            case POp::div_n_floats:   case POp::div_n_ints:   case POp::div_n_uints:
                                      case POp::bitwise_and_n_ints:
                                      case POp::bitwise_or_n_ints:
                                      case POp::bitwise_xor_n_ints:
            case POp::min_n_floats:   case POp::min_n_ints:   case POp::min_n_uints:
            case POp::max_n_floats:   case POp::max_n_ints:   case POp::max_n_uints:
            case POp::cmplt_n_floats: case POp::cmplt_n_ints: case POp::cmplt_n_uints:
            case POp::cmple_n_floats: case POp::cmple_n_ints: case POp::cmple_n_uints:
            case POp::cmpeq_n_floats: case POp::cmpeq_n_ints:
            case POp::cmpne_n_floats: case POp::cmpne_n_ints:
            case POp::atan2_n_floats:
            case POp::pow_n_floats:
                std::tie(opArg1, opArg2) = AdjacentBinaryOpCtx(stage.ctx);
                break;

            case POp::mix_n_floats:   case POp::mix_n_ints:
                std::tie(opArg1, opArg2, opArg3) = AdjacentTernaryOpCtx(stage.ctx);
                break;

            case POp::jump:
            case POp::branch_if_any_active_lanes:
            case POp::branch_if_no_active_lanes:
                opArg1 = BranchOffset(static_cast<SkRasterPipeline_BranchCtx*>(stage.ctx));
                break;

            case POp::branch_if_no_active_lanes_eq: {
                const auto* ctx = static_cast<SkRasterPipeline_BranchIfEqualCtx*>(stage.ctx);
                opArg1 = BranchOffset(ctx);
                opArg2 = PtrCtx(ctx->ptr, 1);
                opArg3 = Imm(sk_bit_cast<float>(ctx->value));
                break;
            }
            default:
                break;
        }

        const char* opName = "";
        switch (stage.op) {
        #define M(x) case POp::x: opName = #x; break;
            SK_RASTER_PIPELINE_OPS_ALL(M)
        #undef M
            case POp::label:               opName = "label";               break;
            case POp::invoke_shader:       opName = "invoke_shader";       break;
            case POp::invoke_color_filter: opName = "invoke_color_filter"; break;
            case POp::invoke_blender:      opName = "invoke_blender";      break;
        }

        std::string opText;
        switch (stage.op) {
            case POp::init_lane_masks:
                opText = "CondMask = LoopMask = RetMask = true";
                break;

            case POp::load_condition_mask:
                opText = "CondMask = " + opArg1;
                break;

            case POp::store_condition_mask:
                opText = opArg1 + " = CondMask";
                break;

            case POp::merge_condition_mask:
                opText = "CondMask = " + opArg1 + " & " + opArg2;
                break;

            case POp::load_loop_mask:
                opText = "LoopMask = " + opArg1;
                break;

            case POp::store_loop_mask:
                opText = opArg1 + " = LoopMask";
                break;

            case POp::mask_off_loop_mask:
                opText = "LoopMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case POp::reenable_loop_mask:
                opText = "LoopMask |= " + opArg1;
                break;

            case POp::merge_loop_mask:
                opText = "LoopMask &= " + opArg1;
                break;

            case POp::load_return_mask:
                opText = "RetMask = " + opArg1;
                break;

            case POp::store_return_mask:
                opText = opArg1 + " = RetMask";
                break;

            case POp::mask_off_return_mask:
                opText = "RetMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case POp::store_src_rg:
                opText = opArg1 + " = src.rg";
                break;

            case POp::store_src:
                opText = opArg1 + " = src.rgba";
                break;

            case POp::store_dst:
                opText = opArg1 + " = dst.rgba";
                break;

            case POp::store_device_xy01:
                opText = opArg1 + " = DeviceCoords.xy01";
                break;

            case POp::load_src_rg:
                opText = "src.rg = " + opArg1;
                break;

            case POp::load_src:
                opText = "src.rgba = " + opArg1;
                break;

            case POp::load_dst:
                opText = "dst.rgba = " + opArg1;
                break;

            case POp::bitwise_and_int:
            case POp::bitwise_and_2_ints:
            case POp::bitwise_and_3_ints:
            case POp::bitwise_and_4_ints:
            case POp::bitwise_and_n_ints:
                opText = opArg1 + " &= " + opArg2;
                break;

            case POp::bitwise_or_int:
            case POp::bitwise_or_2_ints:
            case POp::bitwise_or_3_ints:
            case POp::bitwise_or_4_ints:
            case POp::bitwise_or_n_ints:
                opText = opArg1 + " |= " + opArg2;
                break;

            case POp::bitwise_xor_int:
            case POp::bitwise_xor_2_ints:
            case POp::bitwise_xor_3_ints:
            case POp::bitwise_xor_4_ints:
            case POp::bitwise_xor_n_ints:
                opText = opArg1 + " ^= " + opArg2;
                break;

            case POp::bitwise_not_int:
            case POp::bitwise_not_2_ints:
            case POp::bitwise_not_3_ints:
            case POp::bitwise_not_4_ints:
                opText = opArg1 + " = ~" + opArg1;
                break;

            case POp::cast_to_float_from_int:
            case POp::cast_to_float_from_2_ints:
            case POp::cast_to_float_from_3_ints:
            case POp::cast_to_float_from_4_ints:
                opText = opArg1 + " = IntToFloat(" + opArg1 + ")";
                break;

            case POp::cast_to_float_from_uint:
            case POp::cast_to_float_from_2_uints:
            case POp::cast_to_float_from_3_uints:
            case POp::cast_to_float_from_4_uints:
                opText = opArg1 + " = UintToFloat(" + opArg1 + ")";
                break;

            case POp::cast_to_int_from_float:
            case POp::cast_to_int_from_2_floats:
            case POp::cast_to_int_from_3_floats:
            case POp::cast_to_int_from_4_floats:
                opText = opArg1 + " = FloatToInt(" + opArg1 + ")";
                break;

            case POp::cast_to_uint_from_float:
            case POp::cast_to_uint_from_2_floats:
            case POp::cast_to_uint_from_3_floats:
            case POp::cast_to_uint_from_4_floats:
                opText = opArg1 + " = FloatToUint(" + opArg1 + ")";
                break;

            case POp::copy_slot_masked:            case POp::copy_2_slots_masked:
            case POp::copy_3_slots_masked:         case POp::copy_4_slots_masked:
            case POp::swizzle_copy_slot_masked:    case POp::swizzle_copy_2_slots_masked:
            case POp::swizzle_copy_3_slots_masked: case POp::swizzle_copy_4_slots_masked:
                opText = opArg1 + " = Mask(" + opArg2 + ")";
                break;

            case POp::copy_constant:               case POp::copy_2_constants:
            case POp::copy_3_constants:            case POp::copy_4_constants:
            case POp::copy_slot_unmasked:          case POp::copy_2_slots_unmasked:
            case POp::copy_3_slots_unmasked:       case POp::copy_4_slots_unmasked:
            case POp::swizzle_1:                   case POp::swizzle_2:
            case POp::swizzle_3:                   case POp::swizzle_4:
            case POp::shuffle:
                opText = opArg1 + " = " + opArg2;
                break;

            case POp::zero_slot_unmasked:    case POp::zero_2_slots_unmasked:
            case POp::zero_3_slots_unmasked: case POp::zero_4_slots_unmasked:
                opText = opArg1 + " = 0";
                break;

            case POp::abs_float:    case POp::abs_int:
            case POp::abs_2_floats: case POp::abs_2_ints:
            case POp::abs_3_floats: case POp::abs_3_ints:
            case POp::abs_4_floats: case POp::abs_4_ints:
                opText = opArg1 + " = abs(" + opArg1 + ")";
                break;

            case POp::atan_float:
                opText = opArg1 + " = atan(" + opArg1 + ")";
                break;

            case POp::atan2_n_floats:
                opText = opArg1 + " = atan2(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::ceil_float:
            case POp::ceil_2_floats:
            case POp::ceil_3_floats:
            case POp::ceil_4_floats:
                opText = opArg1 + " = ceil(" + opArg1 + ")";
                break;

            case POp::cos_float:
                opText = opArg1 + " = cos(" + opArg1 + ")";
                break;

            case POp::dot_2_floats:
            case POp::dot_3_floats:
            case POp::dot_4_floats:
                opText = opArg1 + " = dot(" + opArg2 + ", " + opArg3 + ")";
                break;

            case POp::exp_float:
                opText = opArg1 + " = exp(" + opArg1 + ")";
                break;

            case POp::pow_n_floats:
                opText = opArg1 + " = pow(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::sin_float:
                opText = opArg1 + " = sin(" + opArg1 + ")";
                break;

            case POp::sqrt_float:
                opText = opArg1 + " = sqrt(" + opArg1 + ")";
                break;

            case POp::tan_float:
                opText = opArg1 + " = tan(" + opArg1 + ")";
                break;

            case POp::floor_float:
            case POp::floor_2_floats:
            case POp::floor_3_floats:
            case POp::floor_4_floats:
                opText = opArg1 + " = floor(" + opArg1 + ")";
                break;

            case POp::add_float:    case POp::add_int:
            case POp::add_2_floats: case POp::add_2_ints:
            case POp::add_3_floats: case POp::add_3_ints:
            case POp::add_4_floats: case POp::add_4_ints:
            case POp::add_n_floats: case POp::add_n_ints:
                opText = opArg1 + " += " + opArg2;
                break;

            case POp::sub_float:    case POp::sub_int:
            case POp::sub_2_floats: case POp::sub_2_ints:
            case POp::sub_3_floats: case POp::sub_3_ints:
            case POp::sub_4_floats: case POp::sub_4_ints:
            case POp::sub_n_floats: case POp::sub_n_ints:
                opText = opArg1 + " -= " + opArg2;
                break;

            case POp::mul_float:    case POp::mul_int:
            case POp::mul_2_floats: case POp::mul_2_ints:
            case POp::mul_3_floats: case POp::mul_3_ints:
            case POp::mul_4_floats: case POp::mul_4_ints:
            case POp::mul_n_floats: case POp::mul_n_ints:
                opText = opArg1 + " *= " + opArg2;
                break;

            case POp::div_float:    case POp::div_int:    case POp::div_uint:
            case POp::div_2_floats: case POp::div_2_ints: case POp::div_2_uints:
            case POp::div_3_floats: case POp::div_3_ints: case POp::div_3_uints:
            case POp::div_4_floats: case POp::div_4_ints: case POp::div_4_uints:
            case POp::div_n_floats: case POp::div_n_ints: case POp::div_n_uints:
                opText = opArg1 + " /= " + opArg2;
                break;

            case POp::min_float:    case POp::min_int:    case POp::min_uint:
            case POp::min_2_floats: case POp::min_2_ints: case POp::min_2_uints:
            case POp::min_3_floats: case POp::min_3_ints: case POp::min_3_uints:
            case POp::min_4_floats: case POp::min_4_ints: case POp::min_4_uints:
            case POp::min_n_floats: case POp::min_n_ints: case POp::min_n_uints:
                opText = opArg1 + " = min(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::max_float:    case POp::max_int:    case POp::max_uint:
            case POp::max_2_floats: case POp::max_2_ints: case POp::max_2_uints:
            case POp::max_3_floats: case POp::max_3_ints: case POp::max_3_uints:
            case POp::max_4_floats: case POp::max_4_ints: case POp::max_4_uints:
            case POp::max_n_floats: case POp::max_n_ints: case POp::max_n_uints:
                opText = opArg1 + " = max(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmplt_float:    case POp::cmplt_int:    case POp::cmplt_uint:
            case POp::cmplt_2_floats: case POp::cmplt_2_ints: case POp::cmplt_2_uints:
            case POp::cmplt_3_floats: case POp::cmplt_3_ints: case POp::cmplt_3_uints:
            case POp::cmplt_4_floats: case POp::cmplt_4_ints: case POp::cmplt_4_uints:
            case POp::cmplt_n_floats: case POp::cmplt_n_ints: case POp::cmplt_n_uints:
                opText = opArg1 + " = lessThan(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmple_float:    case POp::cmple_int:    case POp::cmple_uint:
            case POp::cmple_2_floats: case POp::cmple_2_ints: case POp::cmple_2_uints:
            case POp::cmple_3_floats: case POp::cmple_3_ints: case POp::cmple_3_uints:
            case POp::cmple_4_floats: case POp::cmple_4_ints: case POp::cmple_4_uints:
            case POp::cmple_n_floats: case POp::cmple_n_ints: case POp::cmple_n_uints:
                opText = opArg1 + " = lessThanEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmpeq_float:    case POp::cmpeq_int:
            case POp::cmpeq_2_floats: case POp::cmpeq_2_ints:
            case POp::cmpeq_3_floats: case POp::cmpeq_3_ints:
            case POp::cmpeq_4_floats: case POp::cmpeq_4_ints:
            case POp::cmpeq_n_floats: case POp::cmpeq_n_ints:
                opText = opArg1 + " = equal(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmpne_float:    case POp::cmpne_int:
            case POp::cmpne_2_floats: case POp::cmpne_2_ints:
            case POp::cmpne_3_floats: case POp::cmpne_3_ints:
            case POp::cmpne_4_floats: case POp::cmpne_4_ints:
            case POp::cmpne_n_floats: case POp::cmpne_n_ints:
                opText = opArg1 + " = notEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::mix_float:      case POp::mix_int:
            case POp::mix_2_floats:   case POp::mix_2_ints:
            case POp::mix_3_floats:   case POp::mix_3_ints:
            case POp::mix_4_floats:   case POp::mix_4_ints:
            case POp::mix_n_floats:   case POp::mix_n_ints:
                opText = opArg1 + " = mix(" + opArg2 + ", " + opArg3 + ", " + opArg1 + ")";
                break;

            case POp::jump:
            case POp::branch_if_any_active_lanes:
            case POp::branch_if_no_active_lanes:
            case POp::invoke_shader:
            case POp::invoke_color_filter:
            case POp::invoke_blender:
                opText = std::string(opName) + " " + opArg1;
                break;

            case POp::branch_if_no_active_lanes_eq:
                opText = "branch " + opArg1 + " if no lanes of " + opArg2 + " == " + opArg3;
                break;

            case POp::label:
                opText = "label " + opArg1;
                break;

            case POp::case_op: {
                opText = "if (" + opArg1 + " == " + opArg3 +
                         ") { LoopMask = true; " + opArg2 + " = false; }";
                break;
            }
            default:
                break;
        }

        std::string line = !opText.empty()
                ? SkSL::String::printf("% 5d. %-30s %s\n", index + 1, opName, opText.c_str())
                : SkSL::String::printf("% 5d. %s\n", index + 1, opName);

        out->writeText(line.c_str());
    }
}

}  // namespace RP
}  // namespace SkSL
