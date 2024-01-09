/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include "include/core/SkStream.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineContextUtils.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "src/sksl/tracing/SkSLTraceHook.h"
#include "src/utils/SkBitSet.h"

#if !defined(SKSL_STANDALONE)
#include "src/core/SkRasterPipeline.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL::RP {

#define ALL_SINGLE_SLOT_UNARY_OP_CASES  \
         BuilderOp::acos_float:         \
    case BuilderOp::asin_float:         \
    case BuilderOp::atan_float:         \
    case BuilderOp::cos_float:          \
    case BuilderOp::exp_float:          \
    case BuilderOp::exp2_float:         \
    case BuilderOp::log_float:          \
    case BuilderOp::log2_float:         \
    case BuilderOp::sin_float:          \
    case BuilderOp::sqrt_float:         \
    case BuilderOp::tan_float

#define ALL_MULTI_SLOT_UNARY_OP_CASES        \
         BuilderOp::abs_int:                 \
    case BuilderOp::cast_to_float_from_int:  \
    case BuilderOp::cast_to_float_from_uint: \
    case BuilderOp::cast_to_int_from_float:  \
    case BuilderOp::cast_to_uint_from_float: \
    case BuilderOp::ceil_float:              \
    case BuilderOp::floor_float:             \
    case BuilderOp::invsqrt_float

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
    case BuilderOp::mod_n_floats:       \
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

#define ALL_IMMEDIATE_BINARY_OP_CASES    \
         BuilderOp::add_imm_float:       \
    case BuilderOp::add_imm_int:         \
    case BuilderOp::mul_imm_float:       \
    case BuilderOp::mul_imm_int:         \
    case BuilderOp::bitwise_and_imm_int: \
    case BuilderOp::bitwise_xor_imm_int: \
    case BuilderOp::min_imm_float:       \
    case BuilderOp::max_imm_float:       \
    case BuilderOp::cmple_imm_float:     \
    case BuilderOp::cmple_imm_int:       \
    case BuilderOp::cmple_imm_uint:      \
    case BuilderOp::cmplt_imm_float:     \
    case BuilderOp::cmplt_imm_int:       \
    case BuilderOp::cmplt_imm_uint:      \
    case BuilderOp::cmpeq_imm_float:     \
    case BuilderOp::cmpeq_imm_int:       \
    case BuilderOp::cmpne_imm_float:     \
    case BuilderOp::cmpne_imm_int

#define ALL_IMMEDIATE_MULTI_SLOT_BINARY_OP_CASES \
         BuilderOp::bitwise_and_imm_int

#define ALL_N_WAY_TERNARY_OP_CASES       \
         BuilderOp::smoothstep_n_floats

#define ALL_MULTI_SLOT_TERNARY_OP_CASES \
         BuilderOp::mix_n_floats:       \
    case BuilderOp::mix_n_ints

static bool is_immediate_op(BuilderOp op) {
    switch (op) {
        case ALL_IMMEDIATE_BINARY_OP_CASES: return true;
        default:                            return false;
    }
}

static bool is_multi_slot_immediate_op(BuilderOp op) {
    switch (op) {
        case ALL_IMMEDIATE_MULTI_SLOT_BINARY_OP_CASES: return true;
        default:                                       return false;
    }
}

static BuilderOp convert_n_way_op_to_immediate(BuilderOp op, int slots, int32_t* constantValue) {
    // We rely on the exact ordering of SkRP ops here; the immediate-mode op must always come
    // directly before the n-way op. (If we have more than one, the increasing-slot variations
    // continue backwards from there.)
    BuilderOp immOp = (BuilderOp)((int)op - 1);

    // Some immediate ops support multiple slots.
    if (is_multi_slot_immediate_op(immOp)) {
        return immOp;
    }

    // Most immediate ops only support a single slot.
    if (slots == 1) {
        if (is_immediate_op(immOp)) {
            return immOp;
        }

        // We also allow for immediate-mode subtraction, by adding a negative value.
        switch (op) {
            case BuilderOp::sub_n_ints:
                *constantValue *= -1;
                return BuilderOp::add_imm_int;

            case BuilderOp::sub_n_floats: {
                // This negates the floating-point value by inverting its sign bit.
                *constantValue ^= 0x80000000;
                return BuilderOp::add_imm_float;
            }
            default:
                break;
        }
    }

    // We don't have an immediate-mode version of this op.
    return op;
}

void Builder::appendInstruction(BuilderOp op, SlotList slots,
                                int immA, int immB, int immC, int immD) {
    fInstructions.push_back({op, slots.fSlotA, slots.fSlotB,
                             immA, immB, immC, immD, fCurrentStackID});
}

Instruction* Builder::lastInstruction(int fromBack) {
    if (fInstructions.size() <= fromBack) {
        return nullptr;
    }
    Instruction* inst = &fInstructions.fromBack(fromBack);
    if (inst->fStackID != fCurrentStackID) {
        return nullptr;
    }
    return inst;
}

Instruction* Builder::lastInstructionOnAnyStack(int fromBack) {
    if (fInstructions.size() <= fromBack) {
        return nullptr;
    }
    return &fInstructions.fromBack(fromBack);
}

void Builder::unary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
        case ALL_MULTI_SLOT_UNARY_OP_CASES:
            this->appendInstruction(op, {}, slots);
            break;

        default:
            SkDEBUGFAIL("not a unary op");
            break;
    }
}

void Builder::binary_op(BuilderOp op, int32_t slots) {
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If we just pushed or splatted a constant onto the stack...
        if (lastInstruction->fOp == BuilderOp::push_constant &&
            lastInstruction->fImmA >= slots) {
            // ... and this op has an immediate-mode equivalent...
            int32_t constantValue = lastInstruction->fImmB;
            BuilderOp immOp = convert_n_way_op_to_immediate(op, slots, &constantValue);
            if (immOp != op) {
                // ... discard the constants from the stack, and use an immediate-mode op.
                this->discard_stack(slots);
                this->appendInstruction(immOp, {}, slots, constantValue);
                return;
            }
        }
    }

    switch (op) {
        case ALL_N_WAY_BINARY_OP_CASES:
        case ALL_MULTI_SLOT_BINARY_OP_CASES:
            this->appendInstruction(op, {}, slots);
            break;

        default:
            SkDEBUGFAIL("not a binary op");
            break;
    }
}

void Builder::ternary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_N_WAY_TERNARY_OP_CASES:
        case ALL_MULTI_SLOT_TERNARY_OP_CASES:
            this->appendInstruction(op, {}, slots);
            break;

        default:
            SkDEBUGFAIL("not a ternary op");
            break;
    }
}

void Builder::dot_floats(int32_t slots) {
    switch (slots) {
        case 1: this->appendInstruction(BuilderOp::mul_n_floats, {}, slots); break;
        case 2: this->appendInstruction(BuilderOp::dot_2_floats, {}, slots); break;
        case 3: this->appendInstruction(BuilderOp::dot_3_floats, {}, slots); break;
        case 4: this->appendInstruction(BuilderOp::dot_4_floats, {}, slots); break;

        default:
            SkDEBUGFAIL("invalid number of slots");
            break;
    }
}

void Builder::refract_floats() {
    this->appendInstruction(BuilderOp::refract_4_floats, {});
}

void Builder::inverse_matrix(int32_t n) {
    switch (n) {
        case 2:  this->appendInstruction(BuilderOp::inverse_mat2, {}, 4);  break;
        case 3:  this->appendInstruction(BuilderOp::inverse_mat3, {}, 9);  break;
        case 4:  this->appendInstruction(BuilderOp::inverse_mat4, {}, 16); break;
        default: SkUNREACHABLE;
    }
}

void Builder::pad_stack(int32_t count) {
    if (count > 0) {
        this->appendInstruction(BuilderOp::pad_stack, {}, count);
    }
}

bool Builder::simplifyImmediateUnmaskedOp() {
    if (fInstructions.size() < 3) {
        return false;
    }

    // If we detect a pattern of 'push, immediate-op, unmasked pop', then we can
    // convert it into an immediate-op directly onto the value slots and take the
    // stack entirely out of the equation.
    Instruction* popInstruction  = this->lastInstruction(/*fromBack=*/0);
    Instruction* immInstruction  = this->lastInstruction(/*fromBack=*/1);
    Instruction* pushInstruction = this->lastInstruction(/*fromBack=*/2);

    // If the last instruction is an unmasked pop...
    if (popInstruction && immInstruction && pushInstruction &&
        popInstruction->fOp == BuilderOp::copy_stack_to_slots_unmasked) {
        // ... and the prior instruction was an immediate-mode op, with the same number of slots...
        if (is_immediate_op(immInstruction->fOp) &&
            immInstruction->fImmA == popInstruction->fImmA) {
            // ... and we support multiple-slot immediates (if this op calls for it)...
            if (immInstruction->fImmA == 1 || is_multi_slot_immediate_op(immInstruction->fOp)) {
                // ... and the prior instruction was `push_slots` or `push_immutable` of at least
                // that many slots...
                if ((pushInstruction->fOp == BuilderOp::push_slots ||
                     pushInstruction->fOp == BuilderOp::push_immutable) &&
                    pushInstruction->fImmA >= popInstruction->fImmA) {
                    // ... onto the same slot range...
                    Slot immSlot = popInstruction->fSlotA + popInstruction->fImmA;
                    Slot pushSlot = pushInstruction->fSlotA + pushInstruction->fImmA;
                    if (immSlot == pushSlot) {
                        // ... we can shrink the push, eliminate the pop, and perform the immediate
                        // op in-place instead.
                        pushInstruction->fImmA -= immInstruction->fImmA;
                        immInstruction->fSlotA = immSlot - immInstruction->fImmA;
                        fInstructions.pop_back();
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void Builder::discard_stack(int32_t count, int stackID) {
    // If we pushed something onto the stack and then immediately discarded part of it, we can
    // shrink or eliminate the push.
    while (count > 0) {
        Instruction* lastInstruction = this->lastInstructionOnAnyStack();
        if (!lastInstruction || lastInstruction->fStackID != stackID) {
            break;
        }

        switch (lastInstruction->fOp) {
            case BuilderOp::discard_stack:
                // Our last op was actually a separate discard_stack; combine the discards.
                lastInstruction->fImmA += count;
                return;

            case BuilderOp::push_clone:
            case BuilderOp::push_clone_from_stack:
            case BuilderOp::push_clone_indirect_from_stack:
            case BuilderOp::push_constant:
            case BuilderOp::push_immutable:
            case BuilderOp::push_immutable_indirect:
            case BuilderOp::push_slots:
            case BuilderOp::push_slots_indirect:
            case BuilderOp::push_uniform:
            case BuilderOp::push_uniform_indirect:
            case BuilderOp::pad_stack: {
                // Our last op was a multi-slot push; these cancel out. Eliminate the op if its
                // count reached zero.
                int cancelOut = std::min(count, lastInstruction->fImmA);
                count                  -= cancelOut;
                lastInstruction->fImmA -= cancelOut;
                if (lastInstruction->fImmA == 0) {
                    fInstructions.pop_back();
                }
                continue;
            }
            case BuilderOp::push_condition_mask:
            case BuilderOp::push_loop_mask:
            case BuilderOp::push_return_mask:
                // Our last op was a single-slot push; cancel out one discard and eliminate the op.
                --count;
                fInstructions.pop_back();
                continue;

            case BuilderOp::copy_stack_to_slots_unmasked: {
                // Look for a pattern of `push, immediate-ops, pop` and simplify it down to an
                // immediate-op directly to the value slot.
                if (count == 1) {
                    if (this->simplifyImmediateUnmaskedOp()) {
                        return;
                    }
                }

                // A `copy_stack_to_slots_unmasked` op, followed immediately by a `discard_stack`
                // op with an equal number of slots, is interpreted as an unmasked stack pop.
                // We can simplify pops in a variety of ways. First, temporarily get rid of
                // `copy_stack_to_slots_unmasked`.
                if (count == lastInstruction->fImmA) {
                    SlotRange dst{lastInstruction->fSlotA, lastInstruction->fImmA};
                    fInstructions.pop_back();

                    // See if we can write this pop in a simpler way.
                    this->simplifyPopSlotsUnmasked(&dst);

                    // If simplification consumed the entire range, we're done!
                    if (dst.count == 0) {
                        return;
                    }

                    // Simplification did not consume the entire range. We are still responsible for
                    // copying-back and discarding any remaining slots.
                    this->copy_stack_to_slots_unmasked(dst);
                    count = dst.count;
                }
                break;
            }
            default:
                break;
        }

        // This instruction wasn't a push.
        break;
    }

    if (count > 0) {
        this->appendInstruction(BuilderOp::discard_stack, {}, count);
    }
}

void Builder::label(int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);

    // If the previous instruction was a branch to this label, it's a no-op; jumping to the very
    // next instruction is effectively meaningless.
    while (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        switch (lastInstruction->fOp) {
            case BuilderOp::jump:
            case BuilderOp::branch_if_all_lanes_active:
            case BuilderOp::branch_if_any_lanes_active:
            case BuilderOp::branch_if_no_lanes_active:
            case BuilderOp::branch_if_no_active_lanes_on_stack_top_equal:
                if (lastInstruction->fImmA == labelID) {
                    fInstructions.pop_back();
                    continue;
                }
                break;

            default:
                break;
        }
        break;
    }
    this->appendInstruction(BuilderOp::label, {}, labelID);
}

void Builder::jump(int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::jump) {
            // The previous instruction was also `jump`, so this branch could never possibly occur.
            return;
        }
    }
    this->appendInstruction(BuilderOp::jump, {}, labelID);
}

void Builder::branch_if_any_lanes_active(int labelID) {
    if (!this->executionMaskWritesAreEnabled()) {
        this->jump(labelID);
        return;
    }

    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::branch_if_any_lanes_active ||
            lastInstruction->fOp == BuilderOp::jump) {
            // The previous instruction was `jump` or `branch_if_any_lanes_active`, so this branch
            // could never possibly occur.
            return;
        }
    }
    this->appendInstruction(BuilderOp::branch_if_any_lanes_active, {}, labelID);
}

void Builder::branch_if_all_lanes_active(int labelID) {
    if (!this->executionMaskWritesAreEnabled()) {
        this->jump(labelID);
        return;
    }

    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::branch_if_all_lanes_active ||
            lastInstruction->fOp == BuilderOp::jump) {
            // The previous instruction was `jump` or `branch_if_all_lanes_active`, so this branch
            // could never possibly occur.
            return;
        }
    }
    this->appendInstruction(BuilderOp::branch_if_all_lanes_active, {}, labelID);
}

void Builder::branch_if_no_lanes_active(int labelID) {
    if (!this->executionMaskWritesAreEnabled()) {
        return;
    }

    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::branch_if_no_lanes_active ||
            lastInstruction->fOp == BuilderOp::jump) {
            // The previous instruction was `jump` or `branch_if_no_lanes_active`, so this branch
            // could never possibly occur.
            return;
        }
    }
    this->appendInstruction(BuilderOp::branch_if_no_lanes_active, {}, labelID);
}

void Builder::branch_if_no_active_lanes_on_stack_top_equal(int value, int labelID) {
    SkASSERT(labelID >= 0 && labelID < fNumLabels);
    if (const Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::jump ||
            (lastInstruction->fOp == BuilderOp::branch_if_no_active_lanes_on_stack_top_equal &&
             lastInstruction->fImmB == value)) {
            // The previous instruction was `jump` or `branch_if_no_active_lanes_on_stack_top_equal`
            // (checking against the same value), so this branch could never possibly occur.
            return;
        }
    }
    this->appendInstruction(BuilderOp::branch_if_no_active_lanes_on_stack_top_equal,
                            {}, labelID, value);
}

void Builder::push_slots_or_immutable(SlotRange src, BuilderOp op) {
    SkASSERT(src.count >= 0);
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous instruction was pushing slots contiguous to this range, we can collapse
        // the two pushes into one larger push.
        if (lastInstruction->fOp == op &&
            lastInstruction->fSlotA + lastInstruction->fImmA == src.index) {
            lastInstruction->fImmA += src.count;
            src.count = 0;
        }
    }

    if (src.count > 0) {
        this->appendInstruction(op, {src.index}, src.count);
    }

    // Look for a sequence of "copy stack to X, discard stack, copy X to stack". This is a common
    // pattern when multiple operations in a row affect the same variable. When we see this, we can
    // eliminate both the discard and the push.
    if (fInstructions.size() >= 3) {
        const Instruction* pushInst        = this->lastInstruction(/*fromBack=*/0);
        const Instruction* discardInst     = this->lastInstruction(/*fromBack=*/1);
        const Instruction* copyToSlotsInst = this->lastInstruction(/*fromBack=*/2);

        if (pushInst && discardInst && copyToSlotsInst && pushInst->fOp == BuilderOp::push_slots) {
            int pushIndex = pushInst->fSlotA;
            int pushCount = pushInst->fImmA;

            // Look for a `discard_stack` matching our push count.
            if (discardInst->fOp == BuilderOp::discard_stack && discardInst->fImmA == pushCount) {
                // Look for a `copy_stack_to_slots` matching our push.
                if ((copyToSlotsInst->fOp == BuilderOp::copy_stack_to_slots ||
                     copyToSlotsInst->fOp == BuilderOp::copy_stack_to_slots_unmasked) &&
                    copyToSlotsInst->fSlotA == pushIndex && copyToSlotsInst->fImmA == pushCount) {
                    // We found a matching sequence. Remove the discard and push.
                    fInstructions.pop_back();
                    fInstructions.pop_back();
                    return;
                }
            }
        }
    }
}

void Builder::push_slots_or_immutable_indirect(SlotRange fixedRange,
                                               int dynamicStackID,
                                               SlotRange limitRange,
                                               BuilderOp op) {
    // SlotA: fixed-range start
    // SlotB: limit-range end
    // immA: number of slots
    // immB: dynamic stack ID
    this->appendInstruction(op,
                            {fixedRange.index, limitRange.index + limitRange.count},
                            fixedRange.count,
                            dynamicStackID);
}

void Builder::push_uniform(SlotRange src) {
    SkASSERT(src.count >= 0);
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous instruction was pushing uniforms contiguous to this range, we can
        // collapse the two pushes into one larger push.
        if (lastInstruction->fOp == BuilderOp::push_uniform &&
            lastInstruction->fSlotA + lastInstruction->fImmA == src.index) {
            lastInstruction->fImmA += src.count;
            return;
        }
    }

    if (src.count > 0) {
        this->appendInstruction(BuilderOp::push_uniform, {src.index}, src.count);
    }
}

void Builder::push_uniform_indirect(SlotRange fixedRange,
                                    int dynamicStackID,
                                    SlotRange limitRange) {
    // SlotA: fixed-range start
    // SlotB: limit-range end
    // immA: number of slots
    // immB: dynamic stack ID
    this->appendInstruction(BuilderOp::push_uniform_indirect,
                            {fixedRange.index, limitRange.index + limitRange.count},
                            fixedRange.count,
                            dynamicStackID);
}

void Builder::trace_var_indirect(int traceMaskStackID,
                                 SlotRange fixedRange,
                                 int dynamicStackID,
                                 SlotRange limitRange) {
    // SlotA: fixed-range start
    // SlotB: limit-range end
    // immA: trace-mask stack ID
    // immB: number of slots
    // immC: dynamic stack ID
    this->appendInstruction(BuilderOp::trace_var_indirect,
                            {fixedRange.index, limitRange.index + limitRange.count},
                            traceMaskStackID,
                            fixedRange.count,
                            dynamicStackID);
}

void Builder::push_constant_i(int32_t val, int count) {
    SkASSERT(count >= 0);
    if (count > 0) {
        if (Instruction* lastInstruction = this->lastInstruction()) {
            // If the previous op is pushing the same value, we can just push more of them.
            if (lastInstruction->fOp == BuilderOp::push_constant && lastInstruction->fImmB == val) {
                lastInstruction->fImmA += count;
                return;
            }
        }
        this->appendInstruction(BuilderOp::push_constant, {}, count, val);
    }
}

void Builder::push_duplicates(int count) {
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous op is pushing a constant, we can just push more of them.
        if (lastInstruction->fOp == BuilderOp::push_constant) {
            lastInstruction->fImmA += count;
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
        case 1:  this->push_clone(/*numSlots=*/1);                 break;
        default: break;
    }
}

void Builder::push_clone(int numSlots, int offsetFromStackTop) {
    // If we are cloning the stack top...
    if (numSlots == 1 && offsetFromStackTop == 0) {
        // ... and the previous op is pushing a constant...
        if (Instruction* lastInstruction = this->lastInstruction()) {
            if (lastInstruction->fOp == BuilderOp::push_constant) {
                // ... we can just push more of them.
                lastInstruction->fImmA += 1;
                return;
            }
        }
    }
    this->appendInstruction(BuilderOp::push_clone, {}, numSlots, numSlots + offsetFromStackTop);
}

void Builder::push_clone_from_stack(SlotRange range, int otherStackID, int offsetFromStackTop) {
    // immA: number of slots
    // immB: other stack ID
    // immC: offset from stack top
    offsetFromStackTop -= range.index;

    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous op is also pushing a clone...
        if (lastInstruction->fOp == BuilderOp::push_clone_from_stack &&
            // ... from the same stack...
            lastInstruction->fImmB == otherStackID &&
            // ... and this clone starts at the same place that the last clone ends...
            lastInstruction->fImmC - lastInstruction->fImmA == offsetFromStackTop) {
            // ... just extend the existing clone-op.
            lastInstruction->fImmA += range.count;
            return;
        }
    }

    this->appendInstruction(BuilderOp::push_clone_from_stack, {},
                            range.count, otherStackID, offsetFromStackTop);
}

void Builder::push_clone_indirect_from_stack(SlotRange fixedOffset,
                                             int dynamicStackID,
                                             int otherStackID,
                                             int offsetFromStackTop) {
    // immA: number of slots
    // immB: other stack ID
    // immC: offset from stack top
    // immD: dynamic stack ID
    offsetFromStackTop -= fixedOffset.index;

    this->appendInstruction(BuilderOp::push_clone_indirect_from_stack, {},
                            fixedOffset.count, otherStackID, offsetFromStackTop, dynamicStackID);
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
    if (!dst->count) {
        // There's nothing left to simplify.
        return;
    }
    Instruction* lastInstruction = this->lastInstruction();
    if (!lastInstruction) {
        // There's nothing left to simplify.
        return;
    }
    BuilderOp lastOp = lastInstruction->fOp;

    // If the last instruction is pushing a constant, we can simplify it by copying the constant
    // directly into the destination slot.
    if (lastOp == BuilderOp::push_constant) {
        // Get the last slot.
        int32_t value = lastInstruction->fImmB;
        lastInstruction->fImmA--;
        if (lastInstruction->fImmA == 0) {
            fInstructions.pop_back();
        }

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Continue simplifying if possible.
        this->simplifyPopSlotsUnmasked(dst);

        // Write the constant directly to the destination slot.
        this->copy_constant(destinationSlot, value);
        return;
    }

    // If the last instruction is pushing a uniform, we can simplify it by copying the uniform
    // directly into the destination slot.
    if (lastOp == BuilderOp::push_uniform) {
        // Get the last slot.
        Slot sourceSlot = lastInstruction->fSlotA + lastInstruction->fImmA - 1;
        lastInstruction->fImmA--;
        if (lastInstruction->fImmA == 0) {
            fInstructions.pop_back();
        }

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Continue simplifying if possible.
        this->simplifyPopSlotsUnmasked(dst);

        // Write the constant directly to the destination slot.
        this->copy_uniform_to_slots_unmasked({destinationSlot, 1}, {sourceSlot, 1});
        return;
    }

    // If the last instruction is pushing a slot or immutable, we can just copy that slot.
    if (lastOp == BuilderOp::push_slots || lastOp == BuilderOp::push_immutable) {
        // Get the last slot.
        Slot sourceSlot = lastInstruction->fSlotA + lastInstruction->fImmA - 1;
        lastInstruction->fImmA--;
        if (lastInstruction->fImmA == 0) {
            fInstructions.pop_back();
        }

        // Consume one destination slot.
        dst->count--;
        Slot destinationSlot = dst->index + dst->count;

        // Try once more.
        this->simplifyPopSlotsUnmasked(dst);

        // Copy the slot directly.
        if (lastOp == BuilderOp::push_slots) {
            if (destinationSlot != sourceSlot) {
                this->copy_slots_unmasked({destinationSlot, 1}, {sourceSlot, 1});
            } else {
                // Copying from a value-slot into the same value-slot is a no-op.
            }
        } else {
            // Copy from immutable data directly to the destination slot.
            this->copy_immutable_unmasked({destinationSlot, 1}, {sourceSlot, 1});
        }
        return;
    }
}

void Builder::pop_slots_unmasked(SlotRange dst) {
    SkASSERT(dst.count >= 0);
    this->copy_stack_to_slots_unmasked(dst);
    this->discard_stack(dst.count);
}

void Builder::exchange_src() {
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous op is also an exchange-src...
        if (lastInstruction->fOp == BuilderOp::exchange_src) {
            // ... both ops can be eliminated. A double-swap is a no-op.
            fInstructions.pop_back();
            return;
        }
    }

    this->appendInstruction(BuilderOp::exchange_src, {});
}

void Builder::pop_src_rgba() {
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the previous op is exchanging src.rgba with the stack...
        if (lastInstruction->fOp == BuilderOp::exchange_src) {
            // ... both ops can be eliminated. It's just sliding the color back and forth.
            fInstructions.pop_back();
            this->discard_stack(4);
            return;
        }
    }

    this->appendInstruction(BuilderOp::pop_src_rgba, {});
}

void Builder::copy_stack_to_slots(SlotRange dst, int offsetFromStackTop) {
    // If the execution mask is known to be all-true, then we can ignore the write mask.
    if (!this->executionMaskWritesAreEnabled()) {
        this->copy_stack_to_slots_unmasked(dst, offsetFromStackTop);
        return;
    }

    // If the last instruction copied the previous stack slots, just extend it.
    if (Instruction* lastInstruction = this->lastInstruction()) {
        // If the last op is copy-stack-to-slots...
        if (lastInstruction->fOp == BuilderOp::copy_stack_to_slots &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstruction->fSlotA + lastInstruction->fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstruction->fImmB - lastInstruction->fImmA == offsetFromStackTop) {
            // then we can just extend the copy!
            lastInstruction->fImmA += dst.count;
            return;
        }
    }

    this->appendInstruction(BuilderOp::copy_stack_to_slots, {dst.index},
                            dst.count, offsetFromStackTop);
}

void Builder::copy_stack_to_slots_indirect(SlotRange fixedRange,
                                           int dynamicStackID,
                                           SlotRange limitRange) {
    // SlotA: fixed-range start
    // SlotB: limit-range end
    // immA: number of slots
    // immB: dynamic stack ID
    this->appendInstruction(BuilderOp::copy_stack_to_slots_indirect,
                            {fixedRange.index, limitRange.index + limitRange.count},
                            fixedRange.count,
                            dynamicStackID);
}

static bool slot_ranges_overlap(SlotRange x, SlotRange y) {
    return x.index < y.index + y.count &&
           y.index < x.index + x.count;
}

void Builder::copy_constant(Slot slot, int constantValue) {
    // If the last instruction copied the same constant, just extend it.
    if (Instruction* lastInstr = this->lastInstruction()) {
        // If the last op is copy-constant...
        if (lastInstr->fOp == BuilderOp::copy_constant &&
            // ... and has the same value...
            lastInstr->fImmB == constantValue &&
            // ... and the slot is immediately after the last copy-constant's destination...
            lastInstr->fSlotA + lastInstr->fImmA == slot) {
            // ... then we can extend the copy!
            lastInstr->fImmA += 1;
            return;
        }
    }

    this->appendInstruction(BuilderOp::copy_constant, {slot}, 1, constantValue);
}

void Builder::copy_slots_unmasked(SlotRange dst, SlotRange src) {
    // If the last instruction copied adjacent slots, just extend it.
    if (Instruction* lastInstr = this->lastInstruction()) {
        // If the last op is a match...
        if (lastInstr->fOp == BuilderOp::copy_slot_unmasked &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstr->fSlotA + lastInstr->fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstr->fSlotB + lastInstr->fImmA == src.index &&
            // and the source/dest ranges will not overlap
            !slot_ranges_overlap({lastInstr->fSlotB, lastInstr->fImmA + dst.count},
                                 {lastInstr->fSlotA, lastInstr->fImmA + dst.count})) {
            // then we can just extend the copy!
            lastInstr->fImmA += dst.count;
            return;
        }
    }

    SkASSERT(dst.count == src.count);
    this->appendInstruction(BuilderOp::copy_slot_unmasked, {dst.index, src.index}, dst.count);
}

void Builder::copy_immutable_unmasked(SlotRange dst, SlotRange src) {
    // If the last instruction copied adjacent immutable data, just extend it.
    if (Instruction* lastInstr = this->lastInstruction()) {
        // If the last op is a match...
        if (lastInstr->fOp == BuilderOp::copy_immutable_unmasked &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstr->fSlotA + lastInstr->fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstr->fSlotB + lastInstr->fImmA == src.index) {
            // then we can just extend the copy!
            lastInstr->fImmA += dst.count;
            return;
        }
    }

    SkASSERT(dst.count == src.count);
    this->appendInstruction(BuilderOp::copy_immutable_unmasked, {dst.index, src.index}, dst.count);
}

void Builder::copy_uniform_to_slots_unmasked(SlotRange dst, SlotRange src) {
    // If the last instruction copied adjacent uniforms, just extend it.
    if (Instruction* lastInstr = this->lastInstruction()) {
        // If the last op is copy-constant...
        if (lastInstr->fOp == BuilderOp::copy_uniform_to_slots_unmasked &&
            // and this op's destination is immediately after the last copy-constant's destination
            lastInstr->fSlotB + lastInstr->fImmA == dst.index &&
            // and this op's source is immediately after the last copy-constant's source
            lastInstr->fSlotA + lastInstr->fImmA == src.index) {
            // then we can just extend the copy!
            lastInstr->fImmA += dst.count;
            return;
        }
    }

    SkASSERT(dst.count == src.count);
    this->appendInstruction(BuilderOp::copy_uniform_to_slots_unmasked, {src.index, dst.index},
                            dst.count);
}

void Builder::copy_stack_to_slots_unmasked(SlotRange dst, int offsetFromStackTop) {
    // If the last instruction copied the previous stack slots, just extend it.
    if (Instruction* lastInstr = this->lastInstruction()) {
        // If the last op is copy-stack-to-slots-unmasked...
        if (lastInstr->fOp == BuilderOp::copy_stack_to_slots_unmasked &&
            // and this op's destination is immediately after the last copy-slots-op's destination
            lastInstr->fSlotA + lastInstr->fImmA == dst.index &&
            // and this op's source is immediately after the last copy-slots-op's source
            lastInstr->fImmB - lastInstr->fImmA == offsetFromStackTop) {
            // then we can just extend the copy!
            lastInstr->fImmA += dst.count;
            return;
        }
    }

    this->appendInstruction(BuilderOp::copy_stack_to_slots_unmasked, {dst.index},
                            dst.count, offsetFromStackTop);
}

void Builder::pop_return_mask() {
    SkASSERT(this->executionMaskWritesAreEnabled());

    // This instruction is going to overwrite the return mask. If the previous instruction was
    // masking off the return mask, that's wasted work and it can be eliminated.
    if (Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::mask_off_return_mask) {
            fInstructions.pop_back();
        }
    }

    this->appendInstruction(BuilderOp::pop_return_mask, {});
}

void Builder::push_condition_mask() {
    SkASSERT(this->executionMaskWritesAreEnabled());

    // If the previous instruction is popping the condition mask, we can restore it onto the stack
    // "for free" instead of copying it.
    if (Instruction* lastInstruction = this->lastInstruction()) {
        if (lastInstruction->fOp == BuilderOp::pop_condition_mask) {
            this->pad_stack(1);
            return;
        }
    }
    this->appendInstruction(BuilderOp::push_condition_mask, {});
}

void Builder::merge_condition_mask() {
    SkASSERT(this->executionMaskWritesAreEnabled());

    // This instruction is going to overwrite the condition mask. If the previous instruction was
    // loading the condition mask, that's wasted work and it can be eliminated.
    if (Instruction* lastInstruction = this->lastInstructionOnAnyStack()) {
        if (lastInstruction->fOp == BuilderOp::pop_condition_mask) {
            int stackID = lastInstruction->fStackID;
            fInstructions.pop_back();
            this->discard_stack(/*count=*/1, stackID);
        }
    }

    this->appendInstruction(BuilderOp::merge_condition_mask, {});
}

void Builder::zero_slots_unmasked(SlotRange dst) {
    if (Instruction* lastInstruction = this->lastInstruction()) {
        if (lastInstruction->fOp == BuilderOp::copy_constant && lastInstruction->fImmB == 0) {
            if (lastInstruction->fSlotA + lastInstruction->fImmA == dst.index) {
                // The previous instruction was zeroing the range immediately before this range.
                // Combine the ranges.
                lastInstruction->fImmA += dst.count;
                return;
            }

            if (lastInstruction->fSlotA == dst.index + dst.count) {
                // The previous instruction was zeroing the range immediately after this range.
                // Combine the ranges.
                lastInstruction->fSlotA = dst.index;
                lastInstruction->fImmA += dst.count;
                return;
            }
        }
    }

    this->appendInstruction(BuilderOp::copy_constant, {dst.index}, dst.count, 0);
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

template <typename T>
static void unpack_nybbles_to_offsets(uint32_t components, SkSpan<T> offsets) {
    // Unpack component nybbles into byte-offsets pointing at stack slots.
    for (size_t index = 0; index < offsets.size(); ++index) {
        offsets[index] = (components & 0xF) * SkOpts::raster_pipeline_highp_stride * sizeof(float);
        components >>= 4;
    }
}

static int max_packed_nybble(uint32_t components, size_t numComponents) {
    int largest = 0;
    for (size_t index = 0; index < numComponents; ++index) {
        largest = std::max<int>(largest, components & 0xF);
        components >>= 4;
    }
    return largest;
}

void Builder::swizzle_copy_stack_to_slots(SlotRange dst,
                                          SkSpan<const int8_t> components,
                                          int offsetFromStackTop) {
    // When the execution-mask writes-enabled flag is off, we could squeeze out a little bit of
    // extra speed here by implementing and using an unmasked version of this op.

    // SlotA: fixed-range start
    // immA: number of swizzle components
    // immB: swizzle components
    // immC: offset from stack top
    this->appendInstruction(BuilderOp::swizzle_copy_stack_to_slots, {dst.index},
                            (int)components.size(),
                            pack_nybbles(components),
                            offsetFromStackTop);
}

void Builder::swizzle_copy_stack_to_slots_indirect(SlotRange fixedRange,
                                                   int dynamicStackID,
                                                   SlotRange limitRange,
                                                   SkSpan<const int8_t> components,
                                                   int offsetFromStackTop) {
    // When the execution-mask writes-enabled flag is off, we could squeeze out a little bit of
    // extra speed here by implementing and using an unmasked version of this op.

    // SlotA: fixed-range start
    // SlotB: limit-range end
    // immA: number of swizzle components
    // immB: swizzle components
    // immC: offset from stack top
    // immD: dynamic stack ID
    this->appendInstruction(BuilderOp::swizzle_copy_stack_to_slots_indirect,
                            {fixedRange.index, limitRange.index + limitRange.count},
                            (int)components.size(),
                            pack_nybbles(components),
                            offsetFromStackTop,
                            dynamicStackID);
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
        this->appendInstruction((BuilderOp)op, {}, consumedSlots,
                                pack_nybbles(SkSpan(elements, numElements)));
        return;
    }

    // This is a big swizzle. We use the `shuffle` op to handle these. immA counts the consumed
    // slots. immB counts the generated slots. immC and immD hold packed-nybble shuffle values.
    this->appendInstruction(BuilderOp::shuffle, {},
                            consumedSlots, numElements,
                            pack_nybbles(SkSpan(&elements[0], 8)),
                            pack_nybbles(SkSpan(&elements[8], 8)));
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
                        this->push_constant_f(1.0f);
                        oneOffset = consumedSlots++;
                    }
                    elements[index++] = oneOffset;
                } else {
                    // We need to synthesize a literal 0.
                    if (zeroOffset == 0) {
                        this->push_constant_f(0.0f);
                        zeroOffset = consumedSlots++;
                    }
                    elements[index++] = zeroOffset;
                }
            }
        }
    }
    this->swizzle(consumedSlots, SkSpan(elements, index));
}

void Builder::matrix_multiply(int leftColumns, int leftRows, int rightColumns, int rightRows) {
    BuilderOp op;
    switch (leftColumns) {
        case 2:  op = BuilderOp::matrix_multiply_2; break;
        case 3:  op = BuilderOp::matrix_multiply_3; break;
        case 4:  op = BuilderOp::matrix_multiply_4; break;
        default: SkDEBUGFAIL("unsupported matrix dimensions"); return;
    }

    this->appendInstruction(op, {}, leftColumns, leftRows, rightColumns, rightRows);
}

std::unique_ptr<Program> Builder::finish(int numValueSlots,
                                         int numUniformSlots,
                                         int numImmutableSlots,
                                         DebugTracePriv* debugTrace) {
    // Verify that calls to enableExecutionMaskWrites and disableExecutionMaskWrites are balanced.
    SkASSERT(fExecutionMaskWritesEnabled == 0);

    return std::make_unique<Program>(std::move(fInstructions), numValueSlots, numUniformSlots,
                                     numImmutableSlots, fNumLabels, debugTrace);
}

void Program::optimize() {
    // TODO(johnstiles): perform any last-minute cleanup of the instruction stream here
}

static int stack_usage(const Instruction& inst) {
    switch (inst.fOp) {
        case BuilderOp::push_condition_mask:
        case BuilderOp::push_loop_mask:
        case BuilderOp::push_return_mask:
            return 1;

        case BuilderOp::push_src_rgba:
        case BuilderOp::push_dst_rgba:
        case BuilderOp::push_device_xy01:
            return 4;

        case BuilderOp::push_immutable:
        case BuilderOp::push_immutable_indirect:
        case BuilderOp::push_constant:
        case BuilderOp::push_slots:
        case BuilderOp::push_slots_indirect:
        case BuilderOp::push_uniform:
        case BuilderOp::push_uniform_indirect:
        case BuilderOp::push_clone:
        case BuilderOp::push_clone_from_stack:
        case BuilderOp::push_clone_indirect_from_stack:
        case BuilderOp::pad_stack:
            return inst.fImmA;

        case BuilderOp::pop_condition_mask:
        case BuilderOp::pop_loop_mask:
        case BuilderOp::pop_and_reenable_loop_mask:
        case BuilderOp::pop_return_mask:
            return -1;

        case BuilderOp::pop_src_rgba:
        case BuilderOp::pop_dst_rgba:
            return -4;

        case ALL_N_WAY_BINARY_OP_CASES:
        case ALL_MULTI_SLOT_BINARY_OP_CASES:
        case BuilderOp::discard_stack:
        case BuilderOp::select:
            return -inst.fImmA;

        case ALL_N_WAY_TERNARY_OP_CASES:
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

        case BuilderOp::refract_4_floats:
            return -5;  // consumes nine slots (N + I + eta) and emits a 4-slot vector (R)

        case BuilderOp::matrix_multiply_2:
        case BuilderOp::matrix_multiply_3:
        case BuilderOp::matrix_multiply_4:
            // consumes the left- and right-matrices; emits result over existing padding slots
            return -(inst.fImmA * inst.fImmB + inst.fImmC * inst.fImmD);

        case BuilderOp::shuffle: {
            int consumed = inst.fImmA;
            int generated = inst.fImmB;
            return generated - consumed;
        }
        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
        case ALL_MULTI_SLOT_UNARY_OP_CASES:
        case ALL_IMMEDIATE_BINARY_OP_CASES:
        default:
            return 0;
    }
}

Program::StackDepths Program::tempStackMaxDepths() const {
    // Count the number of separate temp stacks that the program uses.
    int numStacks = 1;
    for (const Instruction& inst : fInstructions) {
        numStacks = std::max(numStacks, inst.fStackID + 1);
    }

    // Walk the program and calculate how deep each stack can potentially get.
    StackDepths largest, current;
    largest.push_back_n(numStacks, 0);
    current.push_back_n(numStacks, 0);

    for (const Instruction& inst : fInstructions) {
        int stackID = inst.fStackID;
        current[stackID] += stack_usage(inst);
        largest[stackID] = std::max(current[stackID], largest[stackID]);
        // If we assert here, the generated program has popped off the top of the stack.
        SkASSERTF(current[stackID] >= 0, "unbalanced temp stack push/pop on stack %d", stackID);
    }

    // Ensure that when the program is complete, our stacks are fully balanced.
    for (int stackID = 0; stackID < numStacks; ++stackID) {
        // If we assert here, the generated program has pushed more data than it has popped.
        SkASSERTF(current[stackID] == 0, "unbalanced temp stack push/pop on stack %d", stackID);
    }

    return largest;
}

Program::Program(TArray<Instruction> instrs,
                 int numValueSlots,
                 int numUniformSlots,
                 int numImmutableSlots,
                 int numLabels,
                 DebugTracePriv* debugTrace)
        : fInstructions(std::move(instrs))
        , fNumValueSlots(numValueSlots)
        , fNumUniformSlots(numUniformSlots)
        , fNumImmutableSlots(numImmutableSlots)
        , fNumLabels(numLabels)
        , fDebugTrace(debugTrace) {
    this->optimize();

    fTempStackMaxDepths = this->tempStackMaxDepths();

    fNumTempStackSlots = 0;
    for (const int depth : fTempStackMaxDepths) {
        fNumTempStackSlots += depth;
    }

    if (fDebugTrace) {
        fTraceHook = SkSL::Tracer::Make(&fDebugTrace->fTraceInfo);
    }
}

Program::~Program() = default;

static bool immutable_data_is_splattable(int32_t* immutablePtr, int numSlots) {
    // If every value between `immutablePtr[0]` and `immutablePtr[numSlots]` is bit-identical, we
    // can use a splat.
    for (int index = 1; index < numSlots; ++index) {
        if (immutablePtr[0] != immutablePtr[index]) {
            return false;
        }
    }
    return true;
}

void Program::appendCopy(TArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         std::byte* basePtr,  // only used for immutable-value copies
                         ProgramOp baseStage,
                         SkRPOffset dst, int dstStride,
                         SkRPOffset src, int srcStride,
                         int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        // If we are appending a large copy, split it up into groups of four at a time.
        this->appendCopy(pipeline, alloc, basePtr,
                         baseStage,
                         dst, dstStride,
                         src, srcStride,
                         /*numSlots=*/4);
        dst += 4 * dstStride * sizeof(float);
        src += 4 * srcStride * sizeof(float);
        numSlots -= 4;
    }

    SkASSERT(numSlots <= 4);

    if (numSlots > 0) {
        // If we are copying immutable data, it might be representable by a splat; this is
        // preferable, since splats are a tiny bit faster than regular copies.
        if (basePtr) {
            SkASSERT(srcStride == 1);
            int32_t* immutablePtr = reinterpret_cast<int32_t*>(basePtr + src);
            if (immutable_data_is_splattable(immutablePtr, numSlots)) {
                auto stage = (ProgramOp)((int)ProgramOp::copy_constant + numSlots - 1);
                SkRasterPipeline_ConstantCtx ctx;
                ctx.dst = dst;
                ctx.value = *immutablePtr;
                pipeline->push_back({stage, SkRPCtxUtils::Pack(ctx, alloc)});
                return;
            }
        }

        // We can't use a splat, so emit the requested copy op.
        auto stage = (ProgramOp)((int)baseStage + numSlots - 1);
        SkRasterPipeline_BinaryOpCtx ctx;
        ctx.dst = dst;
        ctx.src = src;
        pipeline->push_back({stage, SkRPCtxUtils::Pack(ctx, alloc)});
    }
}

void Program::appendCopySlotsUnmasked(TArray<Stage>* pipeline,
                                      SkArenaAlloc* alloc,
                                      SkRPOffset dst,
                                      SkRPOffset src,
                                      int numSlots) const {
    this->appendCopy(pipeline, alloc, /*basePtr=*/nullptr,
                     ProgramOp::copy_slot_unmasked,
                     dst, SkOpts::raster_pipeline_highp_stride,
                     src, SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendCopyImmutableUnmasked(TArray<Stage>* pipeline,
                                          SkArenaAlloc* alloc,
                                          std::byte* basePtr,
                                          SkRPOffset dst,
                                          SkRPOffset src,
                                          int numSlots) const {
    this->appendCopy(pipeline, alloc, basePtr,
                     ProgramOp::copy_immutable_unmasked,
                     dst, SkOpts::raster_pipeline_highp_stride,
                     src, 1,
                     numSlots);
}

void Program::appendCopySlotsMasked(TArray<Stage>* pipeline,
                                    SkArenaAlloc* alloc,
                                    SkRPOffset dst,
                                    SkRPOffset src,
                                    int numSlots) const {
    this->appendCopy(pipeline, alloc, /*basePtr=*/nullptr,
                     ProgramOp::copy_slot_masked,
                     dst, SkOpts::raster_pipeline_highp_stride,
                     src, SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendSingleSlotUnaryOp(TArray<Stage>* pipeline, ProgramOp stage,
                                      float* dst, int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots--) {
        pipeline->push_back({stage, dst});
        dst += SkOpts::raster_pipeline_highp_stride;
    }
}

void Program::appendMultiSlotUnaryOp(TArray<Stage>* pipeline, ProgramOp baseStage,
                                     float* dst, int numSlots) const {
    SkASSERT(numSlots >= 0);
    while (numSlots > 0) {
        int currentSlots = std::min(numSlots, 4);
        auto stage = (ProgramOp)((int)baseStage + currentSlots - 1);
        pipeline->push_back({stage, dst});

        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }
}

void Program::appendImmediateBinaryOp(TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                      ProgramOp baseStage,
                                      SkRPOffset dst, int32_t value, int numSlots) const {
    SkASSERT(is_immediate_op((BuilderOp)baseStage));
    SkASSERT(numSlots == 1 || is_multi_slot_immediate_op((BuilderOp)baseStage));

    SkRasterPipeline_ConstantCtx ctx;
    ctx.dst = dst;
    ctx.value = value;

    SkASSERT(numSlots >= 0);
    while (numSlots > 0) {
        int currentSlots = std::min(numSlots, 4);
        auto stage = (ProgramOp)((int)baseStage - (currentSlots - 1));
        pipeline->push_back({stage, SkRPCtxUtils::Pack(ctx, alloc)});

        ctx.dst += 4 * SkOpts::raster_pipeline_highp_stride * sizeof(float);
        numSlots -= 4;
    }
}

void Program::appendAdjacentNWayBinaryOp(TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                         ProgramOp stage,
                                         SkRPOffset dst, SkRPOffset src, int numSlots) const {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src);

    if (numSlots > 0) {
        SkRasterPipeline_BinaryOpCtx ctx;
        ctx.dst = dst;
        ctx.src = src;
        pipeline->push_back({stage, SkRPCtxUtils::Pack(ctx, alloc)});
    }
}

void Program::appendAdjacentMultiSlotBinaryOp(TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                              ProgramOp baseStage, std::byte* basePtr,
                                              SkRPOffset dst, SkRPOffset src, int numSlots) const {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src);

    if (numSlots > 4) {
        this->appendAdjacentNWayBinaryOp(pipeline, alloc, baseStage, dst, src, numSlots);
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (ProgramOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, basePtr + dst});
    }
}

void Program::appendAdjacentNWayTernaryOp(TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                          ProgramOp stage, std::byte* basePtr, SkRPOffset dst,
                                          SkRPOffset src0, SkRPOffset src1, int numSlots) const {
    // The float pointers must all be immediately adjacent to each other.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst  + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src0);
    SkASSERT((src0 + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src1);

    if (numSlots > 0) {
        SkRasterPipeline_TernaryOpCtx ctx;
        ctx.dst = dst;
        ctx.delta = src0 - dst;
        pipeline->push_back({stage, SkRPCtxUtils::Pack(ctx, alloc)});
    }
}

void Program::appendAdjacentMultiSlotTernaryOp(TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                               ProgramOp baseStage, std::byte* basePtr,
                                               SkRPOffset dst, SkRPOffset src0, SkRPOffset src1,
                                               int numSlots) const {
    // The float pointers must all be immediately adjacent to each other.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst  + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src0);
    SkASSERT((src0 + SkOpts::raster_pipeline_highp_stride * numSlots * sizeof(float)) == src1);

    if (numSlots > 4) {
        this->appendAdjacentNWayTernaryOp(pipeline, alloc, baseStage, basePtr,
                                          dst, src0, src1, numSlots);
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (ProgramOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, basePtr + dst});
    }
}

void Program::appendStackRewind(TArray<Stage>* pipeline) const {
#if defined(SKSL_STANDALONE) || !SK_HAS_MUSTTAIL
    pipeline->push_back({ProgramOp::stack_rewind, nullptr});
#endif
}

static void* context_bit_pun(intptr_t val) {
    return sk_bit_cast<void*>(val);
}

Program::SlotData Program::allocateSlotData(SkArenaAlloc* alloc) const {
    // Allocate a contiguous slab of slot data for immutables, values, and stack entries.
    const int N = SkOpts::raster_pipeline_highp_stride;
    const int scalarWidth = 1 * sizeof(float);
    const int vectorWidth = N * sizeof(float);
    const int allocSize = vectorWidth * (fNumValueSlots + fNumTempStackSlots) +
                          scalarWidth * fNumImmutableSlots;
    float* slotPtr = static_cast<float*>(alloc->makeBytesAlignedTo(allocSize, vectorWidth));
    sk_bzero(slotPtr, allocSize);

    // Store the temp stack immediately after the values, and immutable data after the stack.
    SlotData s;
    s.values    = SkSpan{slotPtr,        N * fNumValueSlots};
    s.stack     = SkSpan{s.values.end(), N * fNumTempStackSlots};
    s.immutable = SkSpan{s.stack.end(),  1 * fNumImmutableSlots};
    return s;
}

bool Program::appendStages(SkRasterPipeline* pipeline,
                           SkArenaAlloc* alloc,
                           RP::Callbacks* callbacks,
                           SkSpan<const float> uniforms) const {
#if defined(SKSL_STANDALONE)
    return false;
#else
    // Convert our Instruction list to an array of ProgramOps.
    TArray<Stage> stages;
    SlotData slotData = this->allocateSlotData(alloc);
    this->makeStages(&stages, alloc, uniforms, slotData);

    // Allocate buffers for branch targets and labels; these are needed to convert labels into
    // actual offsets into the pipeline and fix up branches.
    TArray<SkRasterPipeline_BranchCtx*> branchContexts;
    branchContexts.reserve_exact(fNumLabels);
    TArray<int> labelOffsets;
    labelOffsets.push_back_n(fNumLabels, -1);
    TArray<int> branchGoesToLabel;
    branchGoesToLabel.reserve_exact(fNumLabels);

    auto resetBasePointer = [&]() {
        // Whenever we hand off control to another shader, we have to assume that it might overwrite
        // the base pointer (if it uses SkSL, it will!), so we reset it on return.
        pipeline->append(SkRasterPipelineOp::set_base_pointer, slotData.values.data());
    };

    resetBasePointer();

    for (const Stage& stage : stages) {
        switch (stage.op) {
            case ProgramOp::stack_rewind:
                pipeline->appendStackRewind();
                break;

            case ProgramOp::invoke_shader:
                if (!callbacks || !callbacks->appendShader(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                resetBasePointer();
                break;

            case ProgramOp::invoke_color_filter:
                if (!callbacks || !callbacks->appendColorFilter(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                resetBasePointer();
                break;

            case ProgramOp::invoke_blender:
                if (!callbacks || !callbacks->appendBlender(sk_bit_cast<intptr_t>(stage.ctx))) {
                    return false;
                }
                resetBasePointer();
                break;

            case ProgramOp::invoke_to_linear_srgb:
                if (!callbacks) {
                    return false;
                }
                callbacks->toLinearSrgb(stage.ctx);
                // A ColorSpaceXform shouldn't ever alter the base pointer, so we don't need to call
                // resetBasePointer here.
                break;

            case ProgramOp::invoke_from_linear_srgb:
                if (!callbacks) {
                    return false;
                }
                callbacks->fromLinearSrgb(stage.ctx);
                // A ColorSpaceXform shouldn't ever alter the base pointer, so we don't need to call
                // resetBasePointer here.
                break;

            case ProgramOp::label: {
                // Remember the absolute pipeline position of this label.
                int labelID = sk_bit_cast<intptr_t>(stage.ctx);
                SkASSERT(labelID >= 0 && labelID < fNumLabels);
                labelOffsets[labelID] = pipeline->getNumStages();
                break;
            }
            case ProgramOp::jump:
            case ProgramOp::branch_if_all_lanes_active:
            case ProgramOp::branch_if_any_lanes_active:
            case ProgramOp::branch_if_no_lanes_active:
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
#endif
}

void Program::makeStages(TArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         SkSpan<const float> uniforms,
                         const SlotData& slots) const {
    SkASSERT(fNumUniformSlots == SkToInt(uniforms.size()));

    const int N = SkOpts::raster_pipeline_highp_stride;
    int mostRecentRewind = 0;

    // Assemble a map holding the current stack-top for each temporary stack. Position each temp
    // stack immediately after the previous temp stack; temp stacks are never allowed to overlap.
    int pos = 0;
    TArray<float*> tempStackMap;
    tempStackMap.resize(fTempStackMaxDepths.size());
    for (int idx = 0; idx < fTempStackMaxDepths.size(); ++idx) {
        tempStackMap[idx] = slots.stack.begin() + (pos * N);
        pos += fTempStackMaxDepths[idx];
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

    auto* const basePtr = (std::byte*)slots.values.data();
    auto OffsetFromBase = [&](const void* ptr) -> SkRPOffset {
        return (SkRPOffset)((const std::byte*)ptr - basePtr);
    };

    // Copy all immutable values into the immutable slots.
    for (const Instruction& inst : fInstructions) {
        if (inst.fOp == BuilderOp::store_immutable_value) {
            slots.immutable[inst.fSlotA] = sk_bit_cast<float>(inst.fImmA);
        }
    }

    // Write each BuilderOp to the pipeline array.
    pipeline->reserve_exact(pipeline->size() + fInstructions.size());
    for (const Instruction& inst : fInstructions) {
        auto ImmutableA = [&]() { return &slots.immutable[1 * inst.fSlotA]; };
        auto ImmutableB = [&]() { return &slots.immutable[1 * inst.fSlotB]; };
        auto SlotA      = [&]() { return &slots.values[N * inst.fSlotA]; };
        auto SlotB      = [&]() { return &slots.values[N * inst.fSlotB]; };
        auto UniformA   = [&]() { return &uniforms[inst.fSlotA]; };
        auto AllocTraceContext = [&](auto* ctx) {
            // We pass `ctx` solely for its type; the value is unused.
            using ContextType = typename std::remove_reference<decltype(*ctx)>::type;
            ctx = alloc->make<ContextType>();
            ctx->traceMask = reinterpret_cast<int*>(tempStackMap[inst.fImmA] - N);
            ctx->traceHook = fTraceHook.get();
            return ctx;
        };
        float*& tempStackPtr = tempStackMap[inst.fStackID];

        switch (inst.fOp) {
            case BuilderOp::label:
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                labelsEncountered.set(inst.fImmA);
                pipeline->push_back({ProgramOp::label, context_bit_pun(inst.fImmA)});
                break;

            case BuilderOp::jump:
            case BuilderOp::branch_if_any_lanes_active:
            case BuilderOp::branch_if_no_lanes_active: {
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                EmitStackRewindForBackwardsBranch(inst.fImmA);

                auto* ctx = alloc->make<SkRasterPipeline_BranchCtx>();
                ctx->offset = inst.fImmA;
                pipeline->push_back({(ProgramOp)inst.fOp, ctx});
                break;
            }
            case BuilderOp::branch_if_all_lanes_active: {
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                EmitStackRewindForBackwardsBranch(inst.fImmA);

                auto* ctx = alloc->make<SkRasterPipeline_BranchIfAllLanesActiveCtx>();
                ctx->offset = inst.fImmA;
                pipeline->push_back({ProgramOp::branch_if_all_lanes_active, ctx});
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
            case BuilderOp::init_lane_masks: {
                auto* ctx = alloc->make<SkRasterPipeline_InitLaneMasksCtx>();
                pipeline->push_back({ProgramOp::init_lane_masks, ctx});
                break;
            }
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

            case BuilderOp::store_immutable_value:
                // The immutable slots were populated in an earlier pass.
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
            case ALL_IMMEDIATE_BINARY_OP_CASES: {
                float* dst = (inst.fSlotA == NA) ? tempStackPtr - (inst.fImmA * N)
                                                 : SlotA();

                this->appendImmediateBinaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                              OffsetFromBase(dst), inst.fImmB, inst.fImmA);
                break;
            }
            case ALL_N_WAY_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendAdjacentNWayBinaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                                 OffsetFromBase(dst), OffsetFromBase(src),
                                                 inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendAdjacentMultiSlotBinaryOp(pipeline, alloc, (ProgramOp)inst.fOp,
                                                      basePtr,
                                                      OffsetFromBase(dst),
                                                      OffsetFromBase(src),
                                                      inst.fImmA);
                break;
            }
            case ALL_N_WAY_TERNARY_OP_CASES: {
                float* src1 = tempStackPtr - (inst.fImmA * N);
                float* src0 = tempStackPtr - (inst.fImmA * 2 * N);
                float* dst  = tempStackPtr - (inst.fImmA * 3 * N);
                this->appendAdjacentNWayTernaryOp(pipeline, alloc, (ProgramOp)inst.fOp, basePtr,
                                                  OffsetFromBase(dst),
                                                  OffsetFromBase(src0),
                                                  OffsetFromBase(src1),
                                                  inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_TERNARY_OP_CASES: {
                float* src1 = tempStackPtr - (inst.fImmA * N);
                float* src0 = tempStackPtr - (inst.fImmA * 2 * N);
                float* dst  = tempStackPtr - (inst.fImmA * 3 * N);
                this->appendAdjacentMultiSlotTernaryOp(pipeline, alloc,(ProgramOp)inst.fOp, basePtr,
                                                       OffsetFromBase(dst),
                                                       OffsetFromBase(src0),
                                                       OffsetFromBase(src1),
                                                       inst.fImmA);
                break;
            }
            case BuilderOp::select: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendCopySlotsMasked(pipeline, alloc,
                                            OffsetFromBase(dst),
                                            OffsetFromBase(src),
                                            inst.fImmA);
                break;
            }
            case BuilderOp::copy_slot_masked:
                this->appendCopySlotsMasked(pipeline, alloc,
                                            OffsetFromBase(SlotA()),
                                            OffsetFromBase(SlotB()),
                                            inst.fImmA);
                break;

            case BuilderOp::copy_slot_unmasked:
                this->appendCopySlotsUnmasked(pipeline, alloc,
                                              OffsetFromBase(SlotA()),
                                              OffsetFromBase(SlotB()),
                                              inst.fImmA);
                break;

            case BuilderOp::copy_immutable_unmasked:
                this->appendCopyImmutableUnmasked(pipeline, alloc, basePtr,
                                                  OffsetFromBase(SlotA()),
                                                  OffsetFromBase(ImmutableB()),
                                                  inst.fImmA);
                break;

            case BuilderOp::refract_4_floats: {
                float* dst = tempStackPtr - (9 * N);
                pipeline->push_back({ProgramOp::refract_4_floats, dst});
                break;
            }
            case BuilderOp::inverse_mat2:
            case BuilderOp::inverse_mat3:
            case BuilderOp::inverse_mat4: {
                float* dst = tempStackPtr - (inst.fImmA * N);
                pipeline->push_back({(ProgramOp)inst.fOp, dst});
                break;
            }
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
                SkRasterPipeline_SwizzleCtx ctx;
                ctx.dst = OffsetFromBase(tempStackPtr - (N * inst.fImmA));
                // Unpack component nybbles into byte-offsets pointing at stack slots.
                unpack_nybbles_to_offsets(inst.fImmB, SkSpan(ctx.offsets));
                pipeline->push_back({(ProgramOp)inst.fOp, SkRPCtxUtils::Pack(ctx, alloc)});
                break;
            }
            case BuilderOp::shuffle: {
                int consumed = inst.fImmA;
                int generated = inst.fImmB;

                auto* ctx = alloc->make<SkRasterPipeline_ShuffleCtx>();
                ctx->ptr = reinterpret_cast<int32_t*>(tempStackPtr) - (N * consumed);
                ctx->count = generated;
                // Unpack immB and immC from nybble form into the offset array.
                unpack_nybbles_to_offsets(inst.fImmC, SkSpan(&ctx->offsets[0], 8));
                unpack_nybbles_to_offsets(inst.fImmD, SkSpan(&ctx->offsets[8], 8));
                pipeline->push_back({ProgramOp::shuffle, ctx});
                break;
            }
            case BuilderOp::matrix_multiply_2:
            case BuilderOp::matrix_multiply_3:
            case BuilderOp::matrix_multiply_4: {
                int consumed = (inst.fImmB * inst.fImmC) +  // result
                               (inst.fImmA * inst.fImmB) +  // left-matrix
                               (inst.fImmC * inst.fImmD);   // right-matrix

                SkRasterPipeline_MatrixMultiplyCtx ctx;
                ctx.dst = OffsetFromBase(tempStackPtr - (N * consumed));
                ctx.leftColumns  = inst.fImmA;
                ctx.leftRows     = inst.fImmB;
                ctx.rightColumns = inst.fImmC;
                ctx.rightRows    = inst.fImmD;
                pipeline->push_back({(ProgramOp)inst.fOp, SkRPCtxUtils::Pack(ctx, alloc)});
                break;
            }
            case BuilderOp::exchange_src: {
                float* dst = tempStackPtr - (4 * N);
                pipeline->push_back({ProgramOp::exchange_src, dst});
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
            case BuilderOp::push_device_xy01: {
                float* dst = tempStackPtr;
                pipeline->push_back({ProgramOp::store_device_xy01, dst});
                break;
            }
            case BuilderOp::pop_src_rgba: {
                float* src = tempStackPtr - (4 * N);
                pipeline->push_back({ProgramOp::load_src, src});
                break;
            }
            case BuilderOp::pop_dst_rgba: {
                float* src = tempStackPtr - (4 * N);
                pipeline->push_back({ProgramOp::load_dst, src});
                break;
            }
            case BuilderOp::push_slots: {
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc,
                                              OffsetFromBase(dst),
                                              OffsetFromBase(SlotA()),
                                              inst.fImmA);
                break;
            }
            case BuilderOp::push_immutable: {
                float* dst = tempStackPtr;
                this->appendCopyImmutableUnmasked(pipeline, alloc, basePtr,
                                                  OffsetFromBase(dst),
                                                  OffsetFromBase(ImmutableA()),
                                                  inst.fImmA);
                break;
            }
            case BuilderOp::copy_stack_to_slots_indirect:
            case BuilderOp::push_immutable_indirect:
            case BuilderOp::push_slots_indirect:
            case BuilderOp::push_uniform_indirect: {
                // SlotA: fixed-range start
                // SlotB: limit-range end
                //  immA: number of slots to copy
                //  immB: dynamic stack ID
                ProgramOp op;
                auto* ctx = alloc->make<SkRasterPipeline_CopyIndirectCtx>();
                ctx->indirectOffset =
                        reinterpret_cast<const uint32_t*>(tempStackMap[inst.fImmB]) - (1 * N);
                ctx->indirectLimit = inst.fSlotB - inst.fSlotA - inst.fImmA;
                ctx->slots = inst.fImmA;
                if (inst.fOp == BuilderOp::push_slots_indirect) {
                    op = ProgramOp::copy_from_indirect_unmasked;
                    ctx->src = reinterpret_cast<const int32_t*>(SlotA());
                    ctx->dst = reinterpret_cast<int32_t*>(tempStackPtr);
                } else if (inst.fOp == BuilderOp::push_immutable_indirect) {
                    // We reuse the indirect-uniform op for indirect copies of immutable data.
                    op = ProgramOp::copy_from_indirect_uniform_unmasked;
                    ctx->src = reinterpret_cast<const int32_t*>(ImmutableA());
                    ctx->dst = reinterpret_cast<int32_t*>(tempStackPtr);
                } else if (inst.fOp == BuilderOp::push_uniform_indirect) {
                    op = ProgramOp::copy_from_indirect_uniform_unmasked;
                    ctx->src = reinterpret_cast<const int32_t*>(UniformA());
                    ctx->dst = reinterpret_cast<int32_t*>(tempStackPtr);
                } else {
                    op = ProgramOp::copy_to_indirect_masked;
                    ctx->src = reinterpret_cast<const int32_t*>(tempStackPtr) - (ctx->slots * N);
                    ctx->dst = reinterpret_cast<int32_t*>(SlotA());
                }
                pipeline->push_back({op, ctx});
                break;
            }
            case BuilderOp::push_uniform:
            case BuilderOp::copy_uniform_to_slots_unmasked: {
                const float* src = UniformA();
                float* dst = (inst.fOp == BuilderOp::push_uniform) ? tempStackPtr : SlotB();

                for (int remaining = inst.fImmA; remaining > 0; remaining -= 4) {
                    auto ctx = alloc->make<SkRasterPipeline_UniformCtx>();
                    ctx->dst = reinterpret_cast<int32_t*>(dst);
                    ctx->src = reinterpret_cast<const int32_t*>(src);
                    switch (remaining) {
                        case 1:  pipeline->push_back({ProgramOp::copy_uniform,    ctx}); break;
                        case 2:  pipeline->push_back({ProgramOp::copy_2_uniforms, ctx}); break;
                        case 3:  pipeline->push_back({ProgramOp::copy_3_uniforms, ctx}); break;
                        default: pipeline->push_back({ProgramOp::copy_4_uniforms, ctx}); break;
                    }
                    dst += 4 * N;
                    src += 4;
                }
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
            case BuilderOp::merge_condition_mask:
            case BuilderOp::merge_inv_condition_mask: {
                float* ptr = tempStackPtr - (2 * N);
                pipeline->push_back({(ProgramOp)inst.fOp, ptr});
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
            case BuilderOp::push_constant: {
                float* dst = (inst.fOp == BuilderOp::copy_constant) ? SlotA() : tempStackPtr;
                // Splat constant values onto the stack.
                for (int remaining = inst.fImmA; remaining > 0; remaining -= 4) {
                    SkRasterPipeline_ConstantCtx ctx;
                    ctx.dst = OffsetFromBase(dst);
                    ctx.value = inst.fImmB;
                    void* ptr = SkRPCtxUtils::Pack(ctx, alloc);
                    switch (remaining) {
                        case 1:  pipeline->push_back({ProgramOp::copy_constant,     ptr}); break;
                        case 2:  pipeline->push_back({ProgramOp::splat_2_constants, ptr}); break;
                        case 3:  pipeline->push_back({ProgramOp::splat_3_constants, ptr}); break;
                        default: pipeline->push_back({ProgramOp::splat_4_constants, ptr}); break;
                    }
                    dst += 4 * N;
                }
                break;
            }
            case BuilderOp::copy_stack_to_slots: {
                float* src = tempStackPtr - (inst.fImmB * N);
                this->appendCopySlotsMasked(pipeline, alloc,
                                            OffsetFromBase(SlotA()),
                                            OffsetFromBase(src),
                                            inst.fImmA);
                break;
            }
            case BuilderOp::copy_stack_to_slots_unmasked: {
                float* src = tempStackPtr - (inst.fImmB * N);
                this->appendCopySlotsUnmasked(pipeline, alloc,
                                              OffsetFromBase(SlotA()),
                                              OffsetFromBase(src),
                                              inst.fImmA);
                break;
            }
            case BuilderOp::swizzle_copy_stack_to_slots: {
                // SlotA: fixed-range start
                // immA: number of swizzle components
                // immB: swizzle components
                // immC: offset from stack top
                auto stage = (ProgramOp)((int)ProgramOp::swizzle_copy_slot_masked + inst.fImmA - 1);
                auto* ctx = alloc->make<SkRasterPipeline_SwizzleCopyCtx>();
                ctx->src = reinterpret_cast<const int32_t*>(tempStackPtr) - (inst.fImmC * N);
                ctx->dst = reinterpret_cast<int32_t*>(SlotA());
                unpack_nybbles_to_offsets(inst.fImmB, SkSpan(ctx->offsets));
                pipeline->push_back({stage, ctx});
                break;
            }
            case BuilderOp::push_clone: {
                float* src = tempStackPtr - (inst.fImmB * N);
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc,
                                              OffsetFromBase(dst),
                                              OffsetFromBase(src),
                                              inst.fImmA);
                break;
            }
            case BuilderOp::push_clone_from_stack: {
                // immA: number of slots
                // immB: other stack ID
                // immC: offset from stack top
                float* sourceStackPtr = tempStackMap[inst.fImmB];
                float* src = sourceStackPtr - (inst.fImmC * N);
                float* dst = tempStackPtr;
                this->appendCopySlotsUnmasked(pipeline, alloc,
                                              OffsetFromBase(dst),
                                              OffsetFromBase(src),
                                              inst.fImmA);
                break;
            }
            case BuilderOp::push_clone_indirect_from_stack: {
                // immA: number of slots
                // immB: other stack ID
                // immC: offset from stack top
                // immD: dynamic stack ID
                float* sourceStackPtr = tempStackMap[inst.fImmB];

                auto* ctx = alloc->make<SkRasterPipeline_CopyIndirectCtx>();
                ctx->dst = reinterpret_cast<int32_t*>(tempStackPtr);
                ctx->src = reinterpret_cast<const int32_t*>(sourceStackPtr) - (inst.fImmC * N);
                ctx->indirectOffset =
                        reinterpret_cast<const uint32_t*>(tempStackMap[inst.fImmD]) - (1 * N);
                ctx->indirectLimit = inst.fImmC - inst.fImmA;
                ctx->slots = inst.fImmA;
                pipeline->push_back({ProgramOp::copy_from_indirect_unmasked, ctx});
                break;
            }
            case BuilderOp::swizzle_copy_stack_to_slots_indirect: {
                // SlotA: fixed-range start
                // SlotB: limit-range end
                // immA: number of swizzle components
                // immB: swizzle components
                // immC: offset from stack top
                // immD: dynamic stack ID
                auto* ctx = alloc->make<SkRasterPipeline_SwizzleCopyIndirectCtx>();
                ctx->src = reinterpret_cast<const int32_t*>(tempStackPtr) - (inst.fImmC * N);
                ctx->dst = reinterpret_cast<int32_t*>(SlotA());
                ctx->indirectOffset =
                        reinterpret_cast<const uint32_t*>(tempStackMap[inst.fImmD]) - (1 * N);
                ctx->indirectLimit =
                        inst.fSlotB - inst.fSlotA - (max_packed_nybble(inst.fImmB, inst.fImmA) + 1);
                ctx->slots = inst.fImmA;
                unpack_nybbles_to_offsets(inst.fImmB, SkSpan(ctx->offsets));
                pipeline->push_back({ProgramOp::swizzle_copy_to_indirect_masked, ctx});
                break;
            }
            case BuilderOp::case_op: {
                SkRasterPipeline_CaseOpCtx ctx;
                ctx.expectedValue = inst.fImmA;
                ctx.offset = OffsetFromBase(tempStackPtr - (2 * N));
                pipeline->push_back({ProgramOp::case_op, SkRPCtxUtils::Pack(ctx, alloc)});
                break;
            }
            case BuilderOp::continue_op:
                pipeline->push_back({ProgramOp::continue_op, tempStackMap[inst.fImmA] - (1 * N)});
                break;

            case BuilderOp::pad_stack:
            case BuilderOp::discard_stack:
                break;

            case BuilderOp::invoke_shader:
            case BuilderOp::invoke_color_filter:
            case BuilderOp::invoke_blender:
                pipeline->push_back({(ProgramOp)inst.fOp, context_bit_pun(inst.fImmA)});
                break;

            case BuilderOp::invoke_to_linear_srgb:
            case BuilderOp::invoke_from_linear_srgb:
                pipeline->push_back({(ProgramOp)inst.fOp, tempStackMap[inst.fImmA] - (4 * N)});
                break;

            case BuilderOp::trace_line: {
                auto* ctx = AllocTraceContext((SkRasterPipeline_TraceLineCtx*)nullptr);
                ctx->lineNumber = inst.fImmB;
                pipeline->push_back({ProgramOp::trace_line, ctx});
                break;
            }
            case BuilderOp::trace_scope: {
                auto* ctx = AllocTraceContext((SkRasterPipeline_TraceScopeCtx*)nullptr);
                ctx->delta = inst.fImmB;
                pipeline->push_back({ProgramOp::trace_scope, ctx});
                break;
            }
            case BuilderOp::trace_enter:
            case BuilderOp::trace_exit: {
                auto* ctx = AllocTraceContext((SkRasterPipeline_TraceFuncCtx*)nullptr);
                ctx->funcIdx = inst.fImmB;
                pipeline->push_back({(ProgramOp)inst.fOp, ctx});
                break;
            }
            case BuilderOp::trace_var:
            case BuilderOp::trace_var_indirect: {
                // SlotA: fixed-range start
                // SlotB: limit-range end
                // immA: trace-mask stack ID
                // immB: number of slots
                // immC: dynamic stack ID
                auto* ctx = AllocTraceContext((SkRasterPipeline_TraceVarCtx*)nullptr);
                ctx->slotIdx = inst.fSlotA;
                ctx->numSlots = inst.fImmB;
                ctx->data = reinterpret_cast<int*>(SlotA());
                if (inst.fOp == BuilderOp::trace_var_indirect) {
                    ctx->indirectOffset =
                            reinterpret_cast<const uint32_t*>(tempStackMap[inst.fImmC]) - (1 * N);
                    ctx->indirectLimit = inst.fSlotB - inst.fSlotA - inst.fImmB;
                } else {
                    ctx->indirectOffset = nullptr;
                    ctx->indirectLimit = 0;
                }
                pipeline->push_back({ProgramOp::trace_var, ctx});
                break;
            }
            default:
                SkDEBUGFAILF("Raster Pipeline: unsupported instruction %d", (int)inst.fOp);
                break;
        }

        int stackUsage = stack_usage(inst);
        if (stackUsage != 0) {
            tempStackPtr += stackUsage * N;
            SkASSERT(tempStackPtr >= slots.stack.begin());
            SkASSERT(tempStackPtr <= slots.stack.end());
        }

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

class Program::Dumper {
public:
    Dumper(const Program& p) : fProgram(p) {}

    void dump(SkWStream* out, bool writeInstructionCount);

    // Finds the labels in the program, and keeps track of their offsets.
    void buildLabelToStageMap() {
        for (int index = 0; index < fStages.size(); ++index) {
            if (fStages[index].op == ProgramOp::label) {
                int labelID = sk_bit_cast<intptr_t>(fStages[index].ctx);
                SkASSERT(!fLabelToStageMap.find(labelID));
                fLabelToStageMap[labelID] = index;
            }
        }
    }

    // Assign unique names to each variable slot; our trace might have multiple variables with the
    // same name, which can make a dump hard to read. We disambiguate them with subscripts.
    void buildUniqueSlotNameList() {
        if (fProgram.fDebugTrace) {
            fSlotNameList.reserve_exact(fProgram.fDebugTrace->fSlotInfo.size());

            // The map consists of <variable name, <source position, unique name>>.
            THashMap<std::string_view, THashMap<int, std::string>> uniqueNameMap;

            for (const SlotDebugInfo& slotInfo : fProgram.fDebugTrace->fSlotInfo) {
                // Look up this variable by its name and source position.
                int pos = slotInfo.pos.valid() ? slotInfo.pos.startOffset() : 0;
                THashMap<int, std::string>& positionMap = uniqueNameMap[slotInfo.name];
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

                fSlotNameList.push_back(uniqueName);
            }
        }
    }

    // Interprets the context value as a branch offset.
    std::string branchOffset(const SkRasterPipeline_BranchCtx* ctx, int index) const {
        // The context's offset field contains a label ID
        int labelID = ctx->offset;
        const int* targetIndex = fLabelToStageMap.find(labelID);
        SkASSERT(targetIndex);
        return SkSL::String::printf("%+d (label %d at #%d)", *targetIndex - index, labelID,
                                                             *targetIndex + 1);
    }

    // Prints a 32-bit immediate value of unknown type (int/float).
    std::string imm(float immFloat, bool showAsFloat = true) const {
        // Special case exact zero as "0" for readability (vs `0x00000000 (0.0)`).
        if (sk_bit_cast<int32_t>(immFloat) == 0) {
            return "0";
        }
        // Start with `0x3F800000` as a baseline.
        uint32_t immUnsigned;
        memcpy(&immUnsigned, &immFloat, sizeof(uint32_t));
        auto text = SkSL::String::printf("0x%08X", immUnsigned);

        // Extend it to `0x3F800000 (1.0)` for finite floating point values.
        if (showAsFloat && std::isfinite(immFloat)) {
            text += " (";
            text += skstd::to_string(immFloat);
            text += ')';
        }
        return text;
    }

    // Interprets the context pointer as a 32-bit immediate value of unknown type (int/float).
    std::string immCtx(const void* ctx, bool showAsFloat = true) const {
        float f;
        memcpy(&f, &ctx, sizeof(float));
        return this->imm(f, showAsFloat);
    }

    // Prints `1` for single slots and `1..3` for ranges of slots.
    std::string asRange(int first, int count) const {
        std::string text = std::to_string(first);
        if (count > 1) {
            text += ".." + std::to_string(first + count - 1);
        }
        return text;
    }

    // Generates a reasonable name for a range of slots or uniforms, e.g.:
    // `val`: slot range points at one variable, named val
    // `val(0..1)`: slot range points at the first and second slot of val (which has 3+ slots)
    // `foo, bar`: slot range fully covers two variables, named foo and bar
    // `foo(3), bar(0)`: slot range covers the fourth slot of foo and the first slot of bar
    std::string slotOrUniformName(SkSpan<const SlotDebugInfo> debugInfo,
                                  SkSpan<const std::string> names,
                                  SlotRange range) const {
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
                text += '(' + this->asRange(slotInfo.componentIndex, slotsToChomp) + ')';
            }
            range.index += slotsToChomp;
            range.count -= slotsToChomp;
        }

        return text;
    }

    // Generates a reasonable name for a range of slots.
    std::string slotName(SlotRange range) const {
        return this->slotOrUniformName(fProgram.fDebugTrace->fSlotInfo, fSlotNameList, range);
    }

    // Generates a reasonable name for a range of uniforms.
    std::string uniformName(SlotRange range) const {
        return this->slotOrUniformName(fProgram.fDebugTrace->fUniformInfo, /*names=*/{}, range);
    }

    // Attempts to interpret the passed-in pointer as a uniform range.
    std::string uniformPtrCtx(const float* ptr, int numSlots) const {
        const float* end = ptr + numSlots;
        if (ptr >= fUniforms.begin() && end <= fUniforms.end()) {
            int uniformIdx = ptr - fUniforms.begin();
            if (fProgram.fDebugTrace) {
                // Handle pointers to named uniform slots.
                std::string name = this->uniformName({uniformIdx, numSlots});
                if (!name.empty()) {
                    return name;
                }
            }
            // Handle pointers to uniforms (when no debug info exists).
            return 'u' + this->asRange(uniformIdx, numSlots);
        }
        return {};
    }

    // Attempts to interpret the passed-in pointer as a value slot range.
    std::string valuePtrCtx(const float* ptr, int numSlots) const {
        const float* end = ptr + (N * numSlots);
        if (ptr >= fSlots.values.begin() && end <= fSlots.values.end()) {
            int valueIdx = ptr - fSlots.values.begin();
            SkASSERT((valueIdx % N) == 0);
            valueIdx /= N;
            if (fProgram.fDebugTrace) {
                // Handle pointers to named value slots.
                std::string name = this->slotName({valueIdx, numSlots});
                if (!name.empty()) {
                    return name;
                }
            }
            // Handle pointers to value slots (when no debug info exists).
            return 'v' + this->asRange(valueIdx, numSlots);
        }
        return {};
    }

    // Attempts to interpret the passed-in pointer as a immutable slot range.
    std::string immutablePtrCtx(const float* ptr, int numSlots) const {
        const float* end = ptr + numSlots;
        if (ptr >= fSlots.immutable.begin() && end <= fSlots.immutable.end()) {
            int index = ptr - fSlots.immutable.begin();
            return 'i' + this->asRange(index, numSlots) + ' ' +
                   this->multiImmCtx(ptr, numSlots);
        }
        return {};
    }

    // Interprets the context value as a pointer to `count` immediate values.
    std::string multiImmCtx(const float* ptr, int count) const {
        // If this is a uniform, print it by name.
        if (std::string text = this->uniformPtrCtx(ptr, count); !text.empty()) {
            return text;
        }
        // Emit a single bracketed immediate.
        if (count == 1) {
            return '[' + this->imm(*ptr) + ']';
        }
        // Emit a list like `[0x00000000 (0.0), 0x3F80000 (1.0)]`.
        std::string text = "[";
        auto separator = SkSL::String::Separator();
        while (count--) {
            text += separator();
            text += this->imm(*ptr++);
        }
        return text + ']';
    }

    // Interprets the context value as a generic pointer.
    std::string ptrCtx(const void* ctx, int numSlots) const {
        const float *ctxAsSlot = static_cast<const float*>(ctx);
        // Check for uniform, value, and immutable pointers.
        if (std::string uniform = this->uniformPtrCtx(ctxAsSlot, numSlots); !uniform.empty()) {
            return uniform;
        }
        if (std::string value = this->valuePtrCtx(ctxAsSlot, numSlots); !value.empty()) {
            return value;
        }
        if (std::string value = this->immutablePtrCtx(ctxAsSlot, numSlots); !value.empty()) {
            return value;
        }
        // Handle pointers to temporary stack slots.
        if (ctxAsSlot >= fSlots.stack.begin() && ctxAsSlot < fSlots.stack.end()) {
            int stackIdx = ctxAsSlot - fSlots.stack.begin();
            SkASSERT((stackIdx % N) == 0);
            return '$' + this->asRange(stackIdx / N, numSlots);
        }
        // This pointer is out of our expected bounds; this generally isn't expected to happen.
        return "ExternalPtr(" + this->asRange(0, numSlots) + ")";
    }

    // Converts an SkRPOffset to a pointer into the value-slot range.
    std::byte* offsetToPtr(SkRPOffset offset) const {
        return (std::byte*)fSlots.values.data() + offset;
    }

    // Interprets a slab offset as a slot range.
    std::string offsetCtx(SkRPOffset offset, int numSlots) const {
        return this->ptrCtx(this->offsetToPtr(offset), numSlots);
    }

    // Interprets the context value as a packed ConstantCtx structure.
    std::tuple<std::string, std::string> constantCtx(const void* v,
                                                     int slots,
                                                     bool showAsFloat = true) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_ConstantCtx*)v);
        return {this->offsetCtx(ctx.dst, slots),
                this->imm(sk_bit_cast<float>(ctx.value), showAsFloat)};
    }

    // Interprets the context value as a BinaryOp structure for copy_n_slots (numSlots is dictated
    // by the op itself).
    std::tuple<std::string, std::string> binaryOpCtx(const void* v, int numSlots) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_BinaryOpCtx*)v);
        return {this->offsetCtx(ctx.dst, numSlots),
                this->offsetCtx(ctx.src, numSlots)};
    }

    // Interprets the context value as a BinaryOp structure for copy_n_uniforms (numSlots is
    // dictated by the op itself).
    std::tuple<std::string, std::string> copyUniformCtx(const void* v, int numSlots) const {
        const auto *ctx = static_cast<const SkRasterPipeline_UniformCtx*>(v);
        return {this->ptrCtx(ctx->dst, numSlots),
                this->multiImmCtx(reinterpret_cast<const float*>(ctx->src), numSlots)};
    }

    // Interprets the context value as a pointer to two adjacent values.
    std::tuple<std::string, std::string> adjacentPtrCtx(const void* ctx, int numSlots) const {
        const float *ctxAsSlot = static_cast<const float*>(ctx);
        return std::make_tuple(this->ptrCtx(ctxAsSlot, numSlots),
                               this->ptrCtx(ctxAsSlot + (N * numSlots), numSlots));
    }

    // Interprets a slab offset as two adjacent slot ranges.
    std::tuple<std::string, std::string> adjacentOffsetCtx(SkRPOffset offset, int numSlots) const {
        return this->adjacentPtrCtx((std::byte*)fSlots.values.data() + offset, numSlots);
    }

    // Interprets the context value as a BinaryOp structure (numSlots is inferred from the distance
    // between pointers).
    std::tuple<std::string, std::string> adjacentBinaryOpCtx(const void* v) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_BinaryOpCtx*)v);
        int numSlots = (ctx.src - ctx.dst) / (N * sizeof(float));
        return this->adjacentOffsetCtx(ctx.dst, numSlots);
    }

    // Interprets the context value as a pointer to three adjacent values.
    std::tuple<std::string, std::string, std::string> adjacent3PtrCtx(const void* ctx,
                                                                      int numSlots) const {
        const float *ctxAsSlot = static_cast<const float*>(ctx);
        return {this->ptrCtx(ctxAsSlot, numSlots),
                this->ptrCtx(ctxAsSlot + (N * numSlots), numSlots),
                this->ptrCtx(ctxAsSlot + (2 * N * numSlots), numSlots)};
    }

    // Interprets a slab offset as three adjacent slot ranges.
    std::tuple<std::string, std::string, std::string> adjacent3OffsetCtx(SkRPOffset offset,
                                                                         int numSlots) const {
        return this->adjacent3PtrCtx((std::byte*)fSlots.values.data() + offset, numSlots);
    }

    // Interprets the context value as a TernaryOp structure (numSlots is inferred from `delta`).
    std::tuple<std::string, std::string, std::string> adjacentTernaryOpCtx(const void* v) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_TernaryOpCtx*)v);
        int numSlots = ctx.delta / (sizeof(float) * N);
        return this->adjacent3OffsetCtx(ctx.dst, numSlots);
    }

    // Stringizes a span of swizzle offsets to the textual equivalent (`xyzw`).
    template <typename T>
    std::string swizzleOffsetSpan(SkSpan<T> offsets) const {
        std::string src;
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
    }

    // Determines the effective width of a swizzle op. When we decode a swizzle, we don't know the
    // slot width of the original value; that's not preserved in the instruction encoding. (e.g.,
    // myFloat4.y would be indistinguishable from myFloat2.y.) We do our best to make a readable
    // dump using the data we have.
    template <typename T>
    size_t swizzleWidth(SkSpan<T> offsets) const {
        size_t highestComponent = *std::max_element(offsets.begin(), offsets.end()) /
                                  (N * sizeof(float));
        size_t swizzleWidth = offsets.size();
        return std::max(swizzleWidth, highestComponent + 1);
    }

    // Stringizes a swizzled pointer.
    template <typename T>
    std::string swizzlePtr(const void* ptr, SkSpan<T> offsets) const {
        return "(" + this->ptrCtx(ptr, this->swizzleWidth(SkSpan(offsets))) + ")." +
               this->swizzleOffsetSpan(SkSpan(offsets));
    }

    // Interprets the context value as a SwizzleCtx structure.
    std::tuple<std::string, std::string> swizzleCtx(ProgramOp op, const void* v) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_SwizzleCtx*)v);
        int destSlots = (int)op - (int)BuilderOp::swizzle_1 + 1;
        return {this->offsetCtx(ctx.dst, destSlots),
                this->swizzlePtr(this->offsetToPtr(ctx.dst), SkSpan(ctx.offsets, destSlots))};
    }

    // Interprets the context value as a SwizzleCopyCtx structure.
    std::tuple<std::string, std::string> swizzleCopyCtx(ProgramOp op, const void* v) const {
        const auto* ctx = static_cast<const SkRasterPipeline_SwizzleCopyCtx*>(v);
        int destSlots = (int)op - (int)BuilderOp::swizzle_copy_slot_masked + 1;

        return {this->swizzlePtr(ctx->dst, SkSpan(ctx->offsets, destSlots)),
                this->ptrCtx(ctx->src, destSlots)};
    }

    // Interprets the context value as a ShuffleCtx structure.
    std::tuple<std::string, std::string> shuffleCtx(const void* v) const {
        const auto* ctx = static_cast<const SkRasterPipeline_ShuffleCtx*>(v);

        std::string dst = this->ptrCtx(ctx->ptr, ctx->count);
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
    }

    // Interprets the context value as a packed MatrixMultiplyCtx structure.
    std::tuple<std::string, std::string, std::string> matrixMultiply(const void* v) const {
        auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_MatrixMultiplyCtx*)v);
        int leftMatrix = ctx.leftColumns * ctx.leftRows;
        int rightMatrix = ctx.rightColumns * ctx.rightRows;
        int resultMatrix = ctx.rightColumns * ctx.leftRows;
        SkRPOffset leftOffset = ctx.dst + (ctx.rightColumns * ctx.leftRows * sizeof(float) * N);
        SkRPOffset rightOffset = leftOffset + (ctx.leftColumns * ctx.leftRows * sizeof(float) * N);
        return {SkSL::String::printf("mat%dx%d(%s)",
                                     ctx.rightColumns,
                                     ctx.leftRows,
                                     this->offsetCtx(ctx.dst, resultMatrix).c_str()),
                SkSL::String::printf("mat%dx%d(%s)",
                                     ctx.leftColumns,
                                     ctx.leftRows,
                                     this->offsetCtx(leftOffset, leftMatrix).c_str()),
                SkSL::String::printf("mat%dx%d(%s)",
                                     ctx.rightColumns,
                                     ctx.rightRows,
                                     this->offsetCtx(rightOffset, rightMatrix).c_str())};
    }

private:
    const int N = SkOpts::raster_pipeline_highp_stride;
    const Program& fProgram;
    TArray<Stage> fStages;
    TArray<std::string> fSlotNameList;
    THashMap<int, int> fLabelToStageMap;  // <label ID, stage index>
    SlotData fSlots;
    SkSpan<float> fUniforms;
};

void Program::Dumper::dump(SkWStream* out, bool writeInstructionCount) {
    using POp = ProgramOp;

    // Allocate memory for the slot and uniform data, even though the program won't ever be
    // executed. The program requires pointer ranges for managing its data, and ASAN will report
    // errors if those pointers are pointing at unallocated memory.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    fSlots = fProgram.allocateSlotData(&alloc);
    float* uniformPtr = alloc.makeArray<float>(fProgram.fNumUniformSlots);
    fUniforms = SkSpan(uniformPtr, fProgram.fNumUniformSlots);

    // Turn this program into an array of Raster Pipeline stages.
    fProgram.makeStages(&fStages, &alloc, fUniforms, fSlots);

    // Assemble lookup tables for program labels and slot names.
    this->buildLabelToStageMap();
    this->buildUniqueSlotNameList();

    // Emit the program's instruction count.
    if (writeInstructionCount) {
        int invocationCount = 0, instructionCount = 0;
        for (const Stage& stage : fStages) {
            switch (stage.op) {
                case POp::label:
                    // consumes zero instructions
                    break;

                case POp::invoke_shader:
                case POp::invoke_color_filter:
                case POp::invoke_blender:
                case POp::invoke_to_linear_srgb:
                case POp::invoke_from_linear_srgb:
                    ++invocationCount;
                    break;

                default:
                    ++instructionCount;
                    break;
            }
        }

        out->writeText(std::to_string(instructionCount).c_str());
        out->writeText(" instructions");
        if (invocationCount > 0) {
            out->writeText(", ");
            out->writeText(std::to_string(invocationCount).c_str());
            out->writeText(" invocations");
        }
        out->writeText("\n\n");
    }

    // Emit all of the program's immutable data.
    const char* header = "[immutable slots]\n";
    const char* footer = "";
    for (const Instruction& inst : fProgram.fInstructions) {
        if (inst.fOp == BuilderOp::store_immutable_value) {
            out->writeText(header);
            out->writeText("i");
            out->writeText(std::to_string(inst.fSlotA).c_str());
            out->writeText(" = ");
            out->writeText(this->imm(sk_bit_cast<float>(inst.fImmA)).c_str());
            out->writeText("\n");

            header = "";
            footer = "\n";
        }
    }
    out->writeText(footer);

    // Emit the program's instruction list.
    for (int index = 0; index < fStages.size(); ++index) {
        const Stage& stage = fStages[index];

        std::string opArg1, opArg2, opArg3, opSwizzle;
        switch (stage.op) {
            case POp::label:
            case POp::invoke_shader:
            case POp::invoke_color_filter:
            case POp::invoke_blender:
                opArg1 = this->immCtx(stage.ctx, /*showAsFloat=*/false);
                break;

            case POp::case_op: {
                auto ctx = SkRPCtxUtils::Unpack((const SkRasterPipeline_CaseOpCtx*)stage.ctx);
                opArg1 = this->offsetCtx(ctx.offset, 1);
                opArg2 = this->offsetCtx(ctx.offset + sizeof(int32_t) * N, 1);
                opArg3 = this->imm(sk_bit_cast<float>(ctx.expectedValue), /*showAsFloat=*/false);
                break;
            }
            case POp::swizzle_1:
            case POp::swizzle_2:
            case POp::swizzle_3:
            case POp::swizzle_4:
                std::tie(opArg1, opArg2) = this->swizzleCtx(stage.op, stage.ctx);
                break;

            case POp::swizzle_copy_slot_masked:
            case POp::swizzle_copy_2_slots_masked:
            case POp::swizzle_copy_3_slots_masked:
            case POp::swizzle_copy_4_slots_masked:
                std::tie(opArg1, opArg2) = this->swizzleCopyCtx(stage.op, stage.ctx);
                break;

            case POp::refract_4_floats:
                std::tie(opArg1, opArg2) = this->adjacentPtrCtx(stage.ctx, 4);
                opArg3 = this->ptrCtx((const float*)(stage.ctx) + (8 * N), 1);
                break;

            case POp::dot_2_floats:
                opArg1 = this->ptrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = this->adjacentPtrCtx(stage.ctx, 2);
                break;

            case POp::dot_3_floats:
                opArg1 = this->ptrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = this->adjacentPtrCtx(stage.ctx, 3);
                break;

            case POp::dot_4_floats:
                opArg1 = this->ptrCtx(stage.ctx, 1);
                std::tie(opArg2, opArg3) = this->adjacentPtrCtx(stage.ctx, 4);
                break;

            case POp::shuffle:
                std::tie(opArg1, opArg2) = this->shuffleCtx(stage.ctx);
                break;

            case POp::matrix_multiply_2:
            case POp::matrix_multiply_3:
            case POp::matrix_multiply_4:
                std::tie(opArg1, opArg2, opArg3) = this->matrixMultiply(stage.ctx);
                break;

            case POp::load_condition_mask:
            case POp::store_condition_mask:
            case POp::load_loop_mask:
            case POp::store_loop_mask:
            case POp::merge_loop_mask:
            case POp::reenable_loop_mask:
            case POp::load_return_mask:
            case POp::store_return_mask:
            case POp::continue_op:
            case POp::cast_to_float_from_int: case POp::cast_to_float_from_uint:
            case POp::cast_to_int_from_float: case POp::cast_to_uint_from_float:
            case POp::abs_int:
            case POp::acos_float:
            case POp::asin_float:
            case POp::atan_float:
            case POp::ceil_float:
            case POp::cos_float:
            case POp::exp_float:
            case POp::exp2_float:
            case POp::log_float:
            case POp::log2_float:
            case POp::floor_float:
            case POp::invsqrt_float:
            case POp::sin_float:
            case POp::sqrt_float:
            case POp::tan_float:
                opArg1 = this->ptrCtx(stage.ctx, 1);
                break;

            case POp::store_src_rg:
            case POp::cast_to_float_from_2_ints: case POp::cast_to_float_from_2_uints:
            case POp::cast_to_int_from_2_floats: case POp::cast_to_uint_from_2_floats:
            case POp::abs_2_ints:
            case POp::ceil_2_floats:
            case POp::floor_2_floats:
            case POp::invsqrt_2_floats:
                opArg1 = this->ptrCtx(stage.ctx, 2);
                break;

            case POp::cast_to_float_from_3_ints: case POp::cast_to_float_from_3_uints:
            case POp::cast_to_int_from_3_floats: case POp::cast_to_uint_from_3_floats:
            case POp::abs_3_ints:
            case POp::ceil_3_floats:
            case POp::floor_3_floats:
            case POp::invsqrt_3_floats:
                opArg1 = this->ptrCtx(stage.ctx, 3);
                break;

            case POp::load_src:
            case POp::load_dst:
            case POp::exchange_src:
            case POp::store_src:
            case POp::store_dst:
            case POp::store_device_xy01:
            case POp::invoke_to_linear_srgb:
            case POp::invoke_from_linear_srgb:
            case POp::cast_to_float_from_4_ints: case POp::cast_to_float_from_4_uints:
            case POp::cast_to_int_from_4_floats: case POp::cast_to_uint_from_4_floats:
            case POp::abs_4_ints:
            case POp::ceil_4_floats:
            case POp::floor_4_floats:
            case POp::invsqrt_4_floats:
            case POp::inverse_mat2:
                opArg1 = this->ptrCtx(stage.ctx, 4);
                break;

            case POp::inverse_mat3:
                opArg1 = this->ptrCtx(stage.ctx, 9);
                break;

            case POp::inverse_mat4:
                opArg1 = this->ptrCtx(stage.ctx, 16);
                break;

            case POp::copy_constant:
            case POp::add_imm_float:
            case POp::mul_imm_float:
            case POp::cmple_imm_float:
            case POp::cmplt_imm_float:
            case POp::cmpeq_imm_float:
            case POp::cmpne_imm_float:
            case POp::min_imm_float:
            case POp::max_imm_float:
                std::tie(opArg1, opArg2) = this->constantCtx(stage.ctx, 1);
                break;

            case POp::add_imm_int:
            case POp::mul_imm_int:
            case POp::bitwise_and_imm_int:
            case POp::bitwise_xor_imm_int:
            case POp::cmple_imm_int:
            case POp::cmple_imm_uint:
            case POp::cmplt_imm_int:
            case POp::cmplt_imm_uint:
            case POp::cmpeq_imm_int:
            case POp::cmpne_imm_int:
                std::tie(opArg1, opArg2) = this->constantCtx(stage.ctx, 1, /*showAsFloat=*/false);
                break;

            case POp::splat_2_constants:
            case POp::bitwise_and_imm_2_ints:
                std::tie(opArg1, opArg2) = this->constantCtx(stage.ctx, 2);
                break;

            case POp::splat_3_constants:
            case POp::bitwise_and_imm_3_ints:
                std::tie(opArg1, opArg2) = this->constantCtx(stage.ctx, 3);
                break;

            case POp::splat_4_constants:
            case POp::bitwise_and_imm_4_ints:
                std::tie(opArg1, opArg2) = this->constantCtx(stage.ctx, 4);
                break;

            case POp::copy_uniform:
                std::tie(opArg1, opArg2) = this->copyUniformCtx(stage.ctx, 1);
                break;

            case POp::copy_2_uniforms:
                std::tie(opArg1, opArg2) = this->copyUniformCtx(stage.ctx, 2);
                break;

            case POp::copy_3_uniforms:
                std::tie(opArg1, opArg2) = this->copyUniformCtx(stage.ctx, 3);
                break;

            case POp::copy_4_uniforms:
                std::tie(opArg1, opArg2) = this->copyUniformCtx(stage.ctx, 4);
                break;

            case POp::copy_slot_masked:
            case POp::copy_slot_unmasked:
            case POp::copy_immutable_unmasked:
                std::tie(opArg1, opArg2) = this->binaryOpCtx(stage.ctx, 1);
                break;

            case POp::copy_2_slots_masked:
            case POp::copy_2_slots_unmasked:
            case POp::copy_2_immutables_unmasked:
                std::tie(opArg1, opArg2) = this->binaryOpCtx(stage.ctx, 2);
                break;

            case POp::copy_3_slots_masked:
            case POp::copy_3_slots_unmasked:
            case POp::copy_3_immutables_unmasked:
                std::tie(opArg1, opArg2) = this->binaryOpCtx(stage.ctx, 3);
                break;

            case POp::copy_4_slots_masked:
            case POp::copy_4_slots_unmasked:
            case POp::copy_4_immutables_unmasked:
                std::tie(opArg1, opArg2) = this->binaryOpCtx(stage.ctx, 4);
                break;

            case POp::copy_from_indirect_uniform_unmasked:
            case POp::copy_from_indirect_unmasked:
            case POp::copy_to_indirect_masked: {
                const auto* ctx = static_cast<SkRasterPipeline_CopyIndirectCtx*>(stage.ctx);
                // We don't incorporate the indirect-limit in the output
                opArg1 = this->ptrCtx(ctx->dst, ctx->slots);
                opArg2 = this->ptrCtx(ctx->src, ctx->slots);
                opArg3 = this->ptrCtx(ctx->indirectOffset, 1);
                break;
            }
            case POp::swizzle_copy_to_indirect_masked: {
                const auto* ctx = static_cast<SkRasterPipeline_SwizzleCopyIndirectCtx*>(stage.ctx);
                opArg1 = this->ptrCtx(ctx->dst, this->swizzleWidth(SkSpan(ctx->offsets,
                                                                          ctx->slots)));
                opArg2 = this->ptrCtx(ctx->src, ctx->slots);
                opArg3 = this->ptrCtx(ctx->indirectOffset, 1);
                opSwizzle = this->swizzleOffsetSpan(SkSpan(ctx->offsets, ctx->slots));
                break;
            }
            case POp::merge_condition_mask:
            case POp::merge_inv_condition_mask:
            case POp::add_float:   case POp::add_int:
            case POp::sub_float:   case POp::sub_int:
            case POp::mul_float:   case POp::mul_int:
            case POp::div_float:   case POp::div_int:   case POp::div_uint:
                                   case POp::bitwise_and_int:
                                   case POp::bitwise_or_int:
                                   case POp::bitwise_xor_int:
            case POp::mod_float:
            case POp::min_float:   case POp::min_int:   case POp::min_uint:
            case POp::max_float:   case POp::max_int:   case POp::max_uint:
            case POp::cmplt_float: case POp::cmplt_int: case POp::cmplt_uint:
            case POp::cmple_float: case POp::cmple_int: case POp::cmple_uint:
            case POp::cmpeq_float: case POp::cmpeq_int:
            case POp::cmpne_float: case POp::cmpne_int:
                std::tie(opArg1, opArg2) = this->adjacentPtrCtx(stage.ctx, 1);
                break;

            case POp::mix_float:   case POp::mix_int:
                std::tie(opArg1, opArg2, opArg3) = this->adjacent3PtrCtx(stage.ctx, 1);
                break;

            case POp::add_2_floats:   case POp::add_2_ints:
            case POp::sub_2_floats:   case POp::sub_2_ints:
            case POp::mul_2_floats:   case POp::mul_2_ints:
            case POp::div_2_floats:   case POp::div_2_ints:   case POp::div_2_uints:
                                      case POp::bitwise_and_2_ints:
                                      case POp::bitwise_or_2_ints:
                                      case POp::bitwise_xor_2_ints:
            case POp::mod_2_floats:
            case POp::min_2_floats:   case POp::min_2_ints:   case POp::min_2_uints:
            case POp::max_2_floats:   case POp::max_2_ints:   case POp::max_2_uints:
            case POp::cmplt_2_floats: case POp::cmplt_2_ints: case POp::cmplt_2_uints:
            case POp::cmple_2_floats: case POp::cmple_2_ints: case POp::cmple_2_uints:
            case POp::cmpeq_2_floats: case POp::cmpeq_2_ints:
            case POp::cmpne_2_floats: case POp::cmpne_2_ints:
                std::tie(opArg1, opArg2) = this->adjacentPtrCtx(stage.ctx, 2);
                break;

            case POp::mix_2_floats:   case POp::mix_2_ints:
                std::tie(opArg1, opArg2, opArg3) = this->adjacent3PtrCtx(stage.ctx, 2);
                break;

            case POp::add_3_floats:   case POp::add_3_ints:
            case POp::sub_3_floats:   case POp::sub_3_ints:
            case POp::mul_3_floats:   case POp::mul_3_ints:
            case POp::div_3_floats:   case POp::div_3_ints:   case POp::div_3_uints:
                                      case POp::bitwise_and_3_ints:
                                      case POp::bitwise_or_3_ints:
                                      case POp::bitwise_xor_3_ints:
            case POp::mod_3_floats:
            case POp::min_3_floats:   case POp::min_3_ints:   case POp::min_3_uints:
            case POp::max_3_floats:   case POp::max_3_ints:   case POp::max_3_uints:
            case POp::cmplt_3_floats: case POp::cmplt_3_ints: case POp::cmplt_3_uints:
            case POp::cmple_3_floats: case POp::cmple_3_ints: case POp::cmple_3_uints:
            case POp::cmpeq_3_floats: case POp::cmpeq_3_ints:
            case POp::cmpne_3_floats: case POp::cmpne_3_ints:
                std::tie(opArg1, opArg2) = this->adjacentPtrCtx(stage.ctx, 3);
                break;

            case POp::mix_3_floats:   case POp::mix_3_ints:
                std::tie(opArg1, opArg2, opArg3) = this->adjacent3PtrCtx(stage.ctx, 3);
                break;

            case POp::add_4_floats:   case POp::add_4_ints:
            case POp::sub_4_floats:   case POp::sub_4_ints:
            case POp::mul_4_floats:   case POp::mul_4_ints:
            case POp::div_4_floats:   case POp::div_4_ints:   case POp::div_4_uints:
                                      case POp::bitwise_and_4_ints:
                                      case POp::bitwise_or_4_ints:
                                      case POp::bitwise_xor_4_ints:
            case POp::mod_4_floats:
            case POp::min_4_floats:   case POp::min_4_ints:   case POp::min_4_uints:
            case POp::max_4_floats:   case POp::max_4_ints:   case POp::max_4_uints:
            case POp::cmplt_4_floats: case POp::cmplt_4_ints: case POp::cmplt_4_uints:
            case POp::cmple_4_floats: case POp::cmple_4_ints: case POp::cmple_4_uints:
            case POp::cmpeq_4_floats: case POp::cmpeq_4_ints:
            case POp::cmpne_4_floats: case POp::cmpne_4_ints:
                std::tie(opArg1, opArg2) = this->adjacentPtrCtx(stage.ctx, 4);
                break;

            case POp::mix_4_floats:   case POp::mix_4_ints:
                std::tie(opArg1, opArg2, opArg3) = this->adjacent3PtrCtx(stage.ctx, 4);
                break;

            case POp::add_n_floats:   case POp::add_n_ints:
            case POp::sub_n_floats:   case POp::sub_n_ints:
            case POp::mul_n_floats:   case POp::mul_n_ints:
            case POp::div_n_floats:   case POp::div_n_ints:   case POp::div_n_uints:
                                      case POp::bitwise_and_n_ints:
                                      case POp::bitwise_or_n_ints:
                                      case POp::bitwise_xor_n_ints:
            case POp::mod_n_floats:
            case POp::min_n_floats:   case POp::min_n_ints:   case POp::min_n_uints:
            case POp::max_n_floats:   case POp::max_n_ints:   case POp::max_n_uints:
            case POp::cmplt_n_floats: case POp::cmplt_n_ints: case POp::cmplt_n_uints:
            case POp::cmple_n_floats: case POp::cmple_n_ints: case POp::cmple_n_uints:
            case POp::cmpeq_n_floats: case POp::cmpeq_n_ints:
            case POp::cmpne_n_floats: case POp::cmpne_n_ints:
            case POp::atan2_n_floats:
            case POp::pow_n_floats:
                std::tie(opArg1, opArg2) = this->adjacentBinaryOpCtx(stage.ctx);
                break;

            case POp::mix_n_floats:        case POp::mix_n_ints:
            case POp::smoothstep_n_floats:
                std::tie(opArg1, opArg2, opArg3) = this->adjacentTernaryOpCtx(stage.ctx);
                break;

            case POp::jump:
            case POp::branch_if_all_lanes_active:
            case POp::branch_if_any_lanes_active:
            case POp::branch_if_no_lanes_active:
                opArg1 = this->branchOffset(static_cast<SkRasterPipeline_BranchCtx*>(stage.ctx),
                                            index);
                break;

            case POp::branch_if_no_active_lanes_eq: {
                const auto* ctx = static_cast<SkRasterPipeline_BranchIfEqualCtx*>(stage.ctx);
                opArg1 = this->branchOffset(ctx, index);
                opArg2 = this->ptrCtx(ctx->ptr, 1);
                opArg3 = this->imm(sk_bit_cast<float>(ctx->value));
                break;
            }
            case POp::trace_var: {
                const auto* ctx = static_cast<SkRasterPipeline_TraceVarCtx*>(stage.ctx);
                opArg1 = this->ptrCtx(ctx->traceMask, 1);
                opArg2 = this->ptrCtx(ctx->data, ctx->numSlots);
                if (ctx->indirectOffset != nullptr) {
                    opArg3 = " + " + this->ptrCtx(ctx->indirectOffset, 1);
                }
                break;
            }
            case POp::trace_line: {
                const auto* ctx = static_cast<SkRasterPipeline_TraceLineCtx*>(stage.ctx);
                opArg1 = this->ptrCtx(ctx->traceMask, 1);
                opArg2 = std::to_string(ctx->lineNumber);
                break;
            }
            case POp::trace_enter:
            case POp::trace_exit: {
                const auto* ctx = static_cast<SkRasterPipeline_TraceFuncCtx*>(stage.ctx);
                opArg1 = this->ptrCtx(ctx->traceMask, 1);
                opArg2 = (fProgram.fDebugTrace &&
                          ctx->funcIdx >= 0 &&
                          ctx->funcIdx < (int)fProgram.fDebugTrace->fFuncInfo.size())
                                 ? fProgram.fDebugTrace->fFuncInfo[ctx->funcIdx].name
                                 : "???";
                break;
            }
            case POp::trace_scope: {
                const auto* ctx = static_cast<SkRasterPipeline_TraceScopeCtx*>(stage.ctx);
                opArg1 = this->ptrCtx(ctx->traceMask, 1);
                opArg2 = SkSL::String::printf("%+d", ctx->delta);
                break;
            }
            default:
                break;
        }

        std::string_view opName;
        switch (stage.op) {
        #define M(x) case POp::x: opName = #x; break;
            SK_RASTER_PIPELINE_OPS_ALL(M)
            SKRP_EXTENDED_OPS(M)
        #undef M
        }

        std::string opText;
        switch (stage.op) {
            case POp::trace_var:
                opText = "TraceVar(" + opArg2 + opArg3 + ") when " + opArg1 + " is true";
                break;

            case POp::trace_line:
                opText = "TraceLine(" + opArg2 + ") when " + opArg1 + " is true";
                break;

            case POp::trace_enter:
                opText = "TraceEnter(" + opArg2 + ") when " + opArg1 + " is true";
                break;

            case POp::trace_exit:
                opText = "TraceExit(" + opArg2 + ") when " + opArg1 + " is true";
                break;

            case POp::trace_scope:
                opText = "TraceScope(" + opArg2 + ") when " + opArg1 + " is true";
                break;

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

            case POp::merge_inv_condition_mask:
                opText = "CondMask = " + opArg1 + " & ~" + opArg2;
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

            case POp::exchange_src:
                opText = "swap(src.rgba, " + opArg1 + ")";
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
            case POp::bitwise_and_imm_int:
            case POp::bitwise_and_imm_2_ints:
            case POp::bitwise_and_imm_3_ints:
            case POp::bitwise_and_imm_4_ints:
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
            case POp::bitwise_xor_imm_int:
                opText = opArg1 + " ^= " + opArg2;
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

            case POp::copy_uniform:                case POp::copy_2_uniforms:
            case POp::copy_3_uniforms:             case POp::copy_4_uniforms:
            case POp::copy_slot_unmasked:          case POp::copy_2_slots_unmasked:
            case POp::copy_3_slots_unmasked:       case POp::copy_4_slots_unmasked:
            case POp::copy_immutable_unmasked:     case POp::copy_2_immutables_unmasked:
            case POp::copy_3_immutables_unmasked:  case POp::copy_4_immutables_unmasked:
            case POp::copy_constant:               case POp::splat_2_constants:
            case POp::splat_3_constants:           case POp::splat_4_constants:
            case POp::swizzle_1:                   case POp::swizzle_2:
            case POp::swizzle_3:                   case POp::swizzle_4:
            case POp::shuffle:
                opText = opArg1 + " = " + opArg2;
                break;

            case POp::copy_from_indirect_unmasked:
            case POp::copy_from_indirect_uniform_unmasked:
                opText = opArg1 + " = Indirect(" + opArg2 + " + " + opArg3 + ")";
                break;

            case POp::copy_to_indirect_masked:
                opText = "Indirect(" + opArg1 + " + " + opArg3 + ") = Mask(" + opArg2 + ")";
                break;

            case POp::swizzle_copy_to_indirect_masked:
                opText = "Indirect(" + opArg1 + " + " + opArg3 + ")." + opSwizzle + " = Mask(" +
                         opArg2 + ")";
                break;

            case POp::abs_int:
            case POp::abs_2_ints:
            case POp::abs_3_ints:
            case POp::abs_4_ints:
                opText = opArg1 + " = abs(" + opArg1 + ")";
                break;

            case POp::acos_float:
                opText = opArg1 + " = acos(" + opArg1 + ")";
                break;

            case POp::asin_float:
                opText = opArg1 + " = asin(" + opArg1 + ")";
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

            case POp::refract_4_floats:
                opText = opArg1 + " = refract(" + opArg1 + ", " + opArg2 + ", " + opArg3 + ")";
                break;

            case POp::dot_2_floats:
            case POp::dot_3_floats:
            case POp::dot_4_floats:
                opText = opArg1 + " = dot(" + opArg2 + ", " + opArg3 + ")";
                break;

            case POp::exp_float:
                opText = opArg1 + " = exp(" + opArg1 + ")";
                break;

            case POp::exp2_float:
                opText = opArg1 + " = exp2(" + opArg1 + ")";
                break;

            case POp::log_float:
                opText = opArg1 + " = log(" + opArg1 + ")";
                break;

            case POp::log2_float:
                opText = opArg1 + " = log2(" + opArg1 + ")";
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

            case POp::invsqrt_float:
            case POp::invsqrt_2_floats:
            case POp::invsqrt_3_floats:
            case POp::invsqrt_4_floats:
                opText = opArg1 + " = inversesqrt(" + opArg1 + ")";
                break;

            case POp::inverse_mat2:
            case POp::inverse_mat3:
            case POp::inverse_mat4:
                opText = opArg1 + " = inverse(" + opArg1 + ")";
                break;

            case POp::add_float:     case POp::add_int:
            case POp::add_2_floats:  case POp::add_2_ints:
            case POp::add_3_floats:  case POp::add_3_ints:
            case POp::add_4_floats:  case POp::add_4_ints:
            case POp::add_n_floats:  case POp::add_n_ints:
            case POp::add_imm_float: case POp::add_imm_int:
                opText = opArg1 + " += " + opArg2;
                break;

            case POp::sub_float:    case POp::sub_int:
            case POp::sub_2_floats: case POp::sub_2_ints:
            case POp::sub_3_floats: case POp::sub_3_ints:
            case POp::sub_4_floats: case POp::sub_4_ints:
            case POp::sub_n_floats: case POp::sub_n_ints:
                opText = opArg1 + " -= " + opArg2;
                break;

            case POp::mul_float:     case POp::mul_int:
            case POp::mul_2_floats:  case POp::mul_2_ints:
            case POp::mul_3_floats:  case POp::mul_3_ints:
            case POp::mul_4_floats:  case POp::mul_4_ints:
            case POp::mul_n_floats:  case POp::mul_n_ints:
            case POp::mul_imm_float: case POp::mul_imm_int:
                opText = opArg1 + " *= " + opArg2;
                break;

            case POp::div_float:    case POp::div_int:    case POp::div_uint:
            case POp::div_2_floats: case POp::div_2_ints: case POp::div_2_uints:
            case POp::div_3_floats: case POp::div_3_ints: case POp::div_3_uints:
            case POp::div_4_floats: case POp::div_4_ints: case POp::div_4_uints:
            case POp::div_n_floats: case POp::div_n_ints: case POp::div_n_uints:
                opText = opArg1 + " /= " + opArg2;
                break;

            case POp::matrix_multiply_2:
            case POp::matrix_multiply_3:
            case POp::matrix_multiply_4:
                opText = opArg1 + " = " + opArg2 + " * " + opArg3;
                break;

            case POp::mod_float:
            case POp::mod_2_floats:
            case POp::mod_3_floats:
            case POp::mod_4_floats:
            case POp::mod_n_floats:
                opText = opArg1 + " = mod(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::min_float:        case POp::min_int:          case POp::min_uint:
            case POp::min_2_floats:     case POp::min_2_ints:       case POp::min_2_uints:
            case POp::min_3_floats:     case POp::min_3_ints:       case POp::min_3_uints:
            case POp::min_4_floats:     case POp::min_4_ints:       case POp::min_4_uints:
            case POp::min_n_floats:     case POp::min_n_ints:       case POp::min_n_uints:
            case POp::min_imm_float:
                opText = opArg1 + " = min(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::max_float:        case POp::max_int:          case POp::max_uint:
            case POp::max_2_floats:     case POp::max_2_ints:       case POp::max_2_uints:
            case POp::max_3_floats:     case POp::max_3_ints:       case POp::max_3_uints:
            case POp::max_4_floats:     case POp::max_4_ints:       case POp::max_4_uints:
            case POp::max_n_floats:     case POp::max_n_ints:       case POp::max_n_uints:
            case POp::max_imm_float:
                opText = opArg1 + " = max(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmplt_float:     case POp::cmplt_int:     case POp::cmplt_uint:
            case POp::cmplt_2_floats:  case POp::cmplt_2_ints:  case POp::cmplt_2_uints:
            case POp::cmplt_3_floats:  case POp::cmplt_3_ints:  case POp::cmplt_3_uints:
            case POp::cmplt_4_floats:  case POp::cmplt_4_ints:  case POp::cmplt_4_uints:
            case POp::cmplt_n_floats:  case POp::cmplt_n_ints:  case POp::cmplt_n_uints:
            case POp::cmplt_imm_float: case POp::cmplt_imm_int: case POp::cmplt_imm_uint:
                opText = opArg1 + " = lessThan(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmple_float:     case POp::cmple_int:     case POp::cmple_uint:
            case POp::cmple_2_floats:  case POp::cmple_2_ints:  case POp::cmple_2_uints:
            case POp::cmple_3_floats:  case POp::cmple_3_ints:  case POp::cmple_3_uints:
            case POp::cmple_4_floats:  case POp::cmple_4_ints:  case POp::cmple_4_uints:
            case POp::cmple_n_floats:  case POp::cmple_n_ints:  case POp::cmple_n_uints:
            case POp::cmple_imm_float: case POp::cmple_imm_int: case POp::cmple_imm_uint:
                opText = opArg1 + " = lessThanEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmpeq_float:     case POp::cmpeq_int:
            case POp::cmpeq_2_floats:  case POp::cmpeq_2_ints:
            case POp::cmpeq_3_floats:  case POp::cmpeq_3_ints:
            case POp::cmpeq_4_floats:  case POp::cmpeq_4_ints:
            case POp::cmpeq_n_floats:  case POp::cmpeq_n_ints:
            case POp::cmpeq_imm_float: case POp::cmpeq_imm_int:
                opText = opArg1 + " = equal(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::cmpne_float:     case POp::cmpne_int:
            case POp::cmpne_2_floats:  case POp::cmpne_2_ints:
            case POp::cmpne_3_floats:  case POp::cmpne_3_ints:
            case POp::cmpne_4_floats:  case POp::cmpne_4_ints:
            case POp::cmpne_n_floats:  case POp::cmpne_n_ints:
            case POp::cmpne_imm_float: case POp::cmpne_imm_int:
                opText = opArg1 + " = notEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case POp::mix_float:      case POp::mix_int:
            case POp::mix_2_floats:   case POp::mix_2_ints:
            case POp::mix_3_floats:   case POp::mix_3_ints:
            case POp::mix_4_floats:   case POp::mix_4_ints:
            case POp::mix_n_floats:   case POp::mix_n_ints:
                opText = opArg1 + " = mix(" + opArg2 + ", " + opArg3 + ", " + opArg1 + ")";
                break;

            case POp::smoothstep_n_floats:
                opText = opArg1 + " = smoothstep(" + opArg1 + ", " + opArg2 + ", " + opArg3 + ")";
                break;

            case POp::jump:
            case POp::branch_if_all_lanes_active:
            case POp::branch_if_any_lanes_active:
            case POp::branch_if_no_lanes_active:
            case POp::invoke_shader:
            case POp::invoke_color_filter:
            case POp::invoke_blender:
                opText = std::string(opName) + " " + opArg1;
                break;

            case POp::invoke_to_linear_srgb:
                opText = opArg1 + " = toLinearSrgb(" + opArg1 + ")";
                break;

            case POp::invoke_from_linear_srgb:
                opText = opArg1 + " = fromLinearSrgb(" + opArg1 + ")";
                break;

            case POp::branch_if_no_active_lanes_eq:
                opText = "branch " + opArg1 + " if no lanes of " + opArg2 + " == " + opArg3;
                break;

            case POp::label:
                opText = "label " + opArg1;
                break;

            case POp::case_op:
                opText = "if (" + opArg1 + " == " + opArg3 +
                         ") { LoopMask = true; " + opArg2 + " = false; }";
                break;

            case POp::continue_op:
                opText = opArg1 +
                         " |= Mask(0xFFFFFFFF); LoopMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            default:
                break;
        }

        opName = opName.substr(0, 30);
        if (!opText.empty()) {
            out->writeText(SkSL::String::printf("%-30.*s %s\n",
                                                (int)opName.size(), opName.data(),
                                                opText.c_str()).c_str());
        } else {
            out->writeText(SkSL::String::printf("%.*s\n",
                                                (int)opName.size(), opName.data()).c_str());
        }
    }
}

void Program::dump(SkWStream* out, bool writeInstructionCount) const {
    Dumper(*this).dump(out, writeInstructionCount);
}

}  // namespace SkSL::RP
