/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkSLString.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>
#include <string>
#include <tuple>
#include <vector>

namespace SkSL {
namespace RP {

using SkRP = SkRasterPipeline;

#define ALL_SINGLE_SLOT_UNARY_OP_CASES \
         BuilderOp::bitwise_not

#define ALL_SINGLE_SLOT_BINARY_OP_CASES \
         BuilderOp::bitwise_and:        \
    case BuilderOp::bitwise_or:         \
    case BuilderOp::bitwise_xor

#define ALL_MULTI_SLOT_BINARY_OP_CASES \
         BuilderOp::add_n_floats:      \
    case BuilderOp::add_n_ints:        \
    case BuilderOp::cmple_n_floats:    \
    case BuilderOp::cmple_n_ints:      \
    case BuilderOp::cmplt_n_floats:    \
    case BuilderOp::cmplt_n_ints:      \
    case BuilderOp::cmpeq_n_floats:    \
    case BuilderOp::cmpeq_n_ints:      \
    case BuilderOp::cmpne_n_floats:    \
    case BuilderOp::cmpne_n_ints

void Builder::unary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
            SkASSERT(slots == 1);
            fInstructions.push_back({op, {}, slots});
            break;

        default:
            SkDEBUGFAIL("not a unary op");
            break;
    }
}

void Builder::binary_op(BuilderOp op, int32_t slots) {
    switch (op) {
        case ALL_SINGLE_SLOT_BINARY_OP_CASES:
            SkASSERT(slots == 1);
            [[fallthrough]];

        case ALL_MULTI_SLOT_BINARY_OP_CASES:
            fInstructions.push_back({op, {}, slots});
            break;

        default:
            SkDEBUGFAIL("not a binary op");
            break;
    }
}

std::unique_ptr<Program> Builder::finish(int numValueSlots, SkRPDebugTrace* debugTrace) {
    return std::make_unique<Program>(std::move(fInstructions), numValueSlots,
                                     fNumLabels, fNumBranches, debugTrace);
}

void Program::optimize() {
    // TODO(johnstiles): perform any last-minute cleanup of the instruction stream here
}

static int stack_usage(const Instruction& inst) {
    switch (inst.fOp) {
        case BuilderOp::push_literal_f:
        case BuilderOp::push_condition_mask:
        case BuilderOp::push_loop_mask:
        case BuilderOp::push_return_mask:
            return 1;

        case BuilderOp::push_slots:
        case BuilderOp::duplicate:
            return inst.fImmA;

        case ALL_SINGLE_SLOT_BINARY_OP_CASES:
        case BuilderOp::pop_condition_mask:
        case BuilderOp::pop_loop_mask:
        case BuilderOp::pop_return_mask:
            return -1;

        case ALL_MULTI_SLOT_BINARY_OP_CASES:
        case BuilderOp::discard_stack:
        case BuilderOp::select:
            return -inst.fImmA;

        case ALL_SINGLE_SLOT_UNARY_OP_CASES:
        default:
            return 0;
    }
}

Program::StackDepthMap Program::tempStackMaxDepths() {
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
                 int numLabels,
                 int numBranches,
                 SkRPDebugTrace* debugTrace)
        : fInstructions(std::move(instrs))
        , fNumValueSlots(numValueSlots)
        , fNumLabels(numLabels)
        , fNumBranches(numBranches)
        , fDebugTrace(debugTrace) {
    this->optimize();

    fTempStackMaxDepths = this->tempStackMaxDepths();

    fNumTempStackSlots = 0;
    for (const auto& [stackIdx, depth] : fTempStackMaxDepths) {
        (void)stackIdx;
        fNumTempStackSlots += depth;
    }
}

template <typename T>
[[maybe_unused]] static void* context_bit_pun(T val) {
    static_assert(sizeof(T) <= sizeof(void*));
    void* contextBits = nullptr;
    memcpy(&contextBits, &val, sizeof(val));
    return contextBits;
}

float* Program::allocateSlotData(SkArenaAlloc* alloc) {
    float* slotPtr = nullptr;

#if !defined(SKSL_STANDALONE)
    // Allocate a contiguous slab of slot data.
    const int N = SkOpts::raster_pipeline_highp_stride;
    const int vectorWidth = N * sizeof(float);
    const int allocSize = vectorWidth * (fNumValueSlots + fNumTempStackSlots);
    slotPtr = static_cast<float*>(alloc->makeBytesAlignedTo(allocSize, vectorWidth));
    sk_bzero(slotPtr, allocSize);
#endif

    return slotPtr;
}

void Program::appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc) {
    this->appendStages(pipeline, alloc, this->allocateSlotData(alloc));
}

void Program::appendStages(SkRasterPipeline* pipeline, SkArenaAlloc* alloc, float* slotPtr) {
#if !defined(SKSL_STANDALONE)
    // Store the temp stack immediately after the values.
    const int N = SkOpts::raster_pipeline_highp_stride;
    float* slotPtrEnd = slotPtr + (N * fNumValueSlots);
    float* tempStackBase = slotPtrEnd;
    [[maybe_unused]] float* tempStackEnd = tempStackBase + (N * fNumTempStackSlots);
    StackDepthMap tempStackDepth;
    int currentStack = 0;

    // Allocate buffers for branch targets (used when running the program) and labels (only needed
    // during initial program construction).
    int* branchTargets = alloc->makeArrayDefault<int>(fNumBranches);
    SkTArray<int> labelOffsets;
    labelOffsets.push_back_n(fNumLabels, -1);
    SkTArray<int> branchGoesToLabel;
    branchGoesToLabel.push_back_n(fNumBranches, -1);
    int currentBranchOp = 0;

    // Assemble a map holding the current stack-top for each temporary stack. Position each temp
    // stack immediately after the previous temp stack; temp stacks are never allowed to overlap.
    int pos = 0;
    SkTHashMap<int, float*> tempStackMap;
    for (auto& [idx, depth] : fTempStackMaxDepths) {
        tempStackMap[idx] = tempStackBase + (pos * N);
        pos += depth;
    }

    // Write each BuilderOp to the pipeline.
    for (const Instruction& inst : fInstructions) {
        auto SlotA = [&]() { return &slotPtr[N * inst.fSlotA]; };
        auto SlotB = [&]() { return &slotPtr[N * inst.fSlotB]; };
        float*& tempStackPtr = tempStackMap[currentStack];

        switch (inst.fOp) {
            case BuilderOp::label:
                // Write the absolute pipeline position into the label offset list. We will go over
                // the branch targets at the end and fix them up.
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                labelOffsets[inst.fImmA] = pipeline->getNumStages();
                break;

            case BuilderOp::jump:
            case BuilderOp::branch_if_any_active_lanes:
            case BuilderOp::branch_if_no_active_lanes:
                // If we have already encountered the label associated with this branch, this is a
                // backwards branch. Add a stack-rewind immediately before the branch to ensure that
                // long-running loops don't use an unbounded amount of stack space.
                if (labelOffsets[inst.fImmA] >= 0) {
                    pipeline->append_stack_rewind();
                }

                // Write the absolute pipeline position into the branch targets, because the
                // associated label might not have been reached yet. We will go back over the branch
                // targets at the end and fix them up.
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                SkASSERT(currentBranchOp >= 0 && currentBranchOp < fNumBranches);
                branchTargets[currentBranchOp] = pipeline->getNumStages();
                branchGoesToLabel[currentBranchOp] = inst.fImmA;
                pipeline->append((SkRP::Stage)inst.fOp, &branchTargets[currentBranchOp]);
                ++currentBranchOp;
                break;

            case BuilderOp::init_lane_masks:
                pipeline->append(SkRP::init_lane_masks);
                break;

            case BuilderOp::store_src_rg:
                pipeline->append(SkRP::store_src_rg, SlotA());
                break;

            case BuilderOp::store_src:
                pipeline->append(SkRP::store_src, SlotA());
                break;

            case BuilderOp::store_dst:
                pipeline->append(SkRP::store_dst, SlotA());
                break;

            case BuilderOp::load_src:
                pipeline->append(SkRP::load_src, SlotA());
                break;

            case BuilderOp::load_dst:
                pipeline->append(SkRP::load_dst, SlotA());
                break;

            case BuilderOp::immediate_f: {
                pipeline->append(SkRP::immediate_f, context_bit_pun(inst.fImmA));
                break;
            }
            case BuilderOp::load_unmasked:
                pipeline->append(SkRP::load_unmasked, SlotA());
                break;

            case BuilderOp::store_unmasked:
                pipeline->append(SkRP::store_unmasked, SlotA());
                break;

            case BuilderOp::store_masked:
                pipeline->append(SkRP::store_masked, SlotA());
                break;

            case ALL_SINGLE_SLOT_UNARY_OP_CASES: {
                float* dst = tempStackPtr - (1 * N);
                pipeline->append((SkRP::Stage)inst.fOp, dst);
                break;
            }
            case ALL_SINGLE_SLOT_BINARY_OP_CASES: {
                float* src = tempStackPtr - (1 * N);
                float* dst = tempStackPtr - (2 * N);
                pipeline->append_adjacent_single_slot_op((SkRP::Stage)inst.fOp, dst, src);
                break;
            }
            case ALL_MULTI_SLOT_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                pipeline->append_adjacent_multi_slot_op(alloc, (SkRP::Stage)inst.fOp,
                                                        dst, src, inst.fImmA);
                break;
            }
            case BuilderOp::select: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                pipeline->append_copy_slots_masked(alloc, dst, src, inst.fImmA);
                break;
            }
            case BuilderOp::copy_slot_masked:
                pipeline->append_copy_slots_masked(alloc, SlotA(), SlotB(), inst.fImmA);
                break;

            case BuilderOp::copy_slot_unmasked:
                pipeline->append_copy_slots_unmasked(alloc, SlotA(), SlotB(), inst.fImmA);
                break;

            case BuilderOp::zero_slot_unmasked:
                pipeline->append_zero_slots_unmasked(SlotA(), inst.fImmA);
                break;

            case BuilderOp::push_slots: {
                float* dst = tempStackPtr;
                pipeline->append_copy_slots_unmasked(alloc, dst, SlotA(), inst.fImmA);
                break;
            }
            case BuilderOp::push_condition_mask: {
                float* dst = tempStackPtr;
                pipeline->append(SkRP::store_condition_mask, dst);
                break;
            }
            case BuilderOp::pop_condition_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->append(SkRP::load_condition_mask, src);
                break;
            }
            case BuilderOp::merge_condition_mask: {
                float* ptr = tempStackPtr - (2 * N);
                pipeline->append(SkRP::merge_condition_mask, ptr);
                break;
            }
            case BuilderOp::push_loop_mask: {
                float* dst = tempStackPtr;
                pipeline->append(SkRP::store_loop_mask, dst);
                break;
            }
            case BuilderOp::pop_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->append(SkRP::load_loop_mask, src);
                break;
            }
            case BuilderOp::mask_off_loop_mask:
                pipeline->append(SkRP::mask_off_loop_mask);
                break;

            case BuilderOp::merge_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->append(SkRP::merge_loop_mask, src);
                break;
            }
            case BuilderOp::push_return_mask: {
                float* dst = tempStackPtr;
                pipeline->append(SkRP::store_return_mask, dst);
                break;
            }
            case BuilderOp::pop_return_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->append(SkRP::load_return_mask, src);
                break;
            }
            case BuilderOp::mask_off_return_mask:
                pipeline->append(SkRP::mask_off_return_mask);
                break;

            case BuilderOp::push_literal_f: {
                float* dst = tempStackPtr;
                if (inst.fImmA == 0) {
                    pipeline->append_zero_slots_unmasked(dst, /*numSlots=*/1);
                } else {
                    pipeline->append(SkRP::immediate_f, context_bit_pun(inst.fImmA));
                    pipeline->append(SkRP::store_unmasked, dst);
                }
                break;
            }
            case BuilderOp::copy_stack_to_slots: {
                float* src = tempStackPtr - (inst.fImmA * N);
                pipeline->append_copy_slots_masked(alloc, SlotA(), src, inst.fImmA);
                break;
            }
            case BuilderOp::copy_stack_to_slots_unmasked: {
                float* src = tempStackPtr - (inst.fImmA * N);
                pipeline->append_copy_slots_unmasked(alloc, SlotA(), src, inst.fImmA);
                break;
            }
            case BuilderOp::duplicate: {
                float* src = tempStackPtr - (1 * N);
                float* dst = tempStackPtr;
                pipeline->append(SkRP::load_unmasked, src);
                for (int index = 0; index < inst.fImmA; ++index) {
                    pipeline->append(SkRP::store_unmasked, dst);
                    dst += N;
                }
                break;
            }
            case BuilderOp::discard_stack:
                break;

            case BuilderOp::set_current_stack:
                currentStack = inst.fImmA;
                break;

            default:
                SkDEBUGFAILF("Raster Pipeline: unsupported instruction %d", (int)inst.fOp);
                break;
        }

        tempStackPtr += stack_usage(inst) * N;
        SkASSERT(tempStackPtr >= tempStackBase);
        SkASSERT(tempStackPtr <= tempStackEnd);
    }

    // Fix up every branch target.
    for (int index = 0; index < fNumBranches; ++index) {
        int branchFromIdx = branchTargets[index];
        int branchToIdx = labelOffsets[branchGoesToLabel[index]];
        branchTargets[index] = branchToIdx - branchFromIdx;
    }

#endif
}

void Program::dump(SkWStream* out) {
    // TODO: skslc will want to dump these programs; we'll need to include some portion of
    // SkRasterPipeline into skslc for this to work properly.

#if !defined(SKSL_STANDALONE)
    // Allocate a block of memory for the slot data, even though the program won't ever be executed.
    // The program requires a pointer range for managing its data, and ASAN will report errors if
    // those pointers are pointing at unallocated memory.
    SkArenaAlloc alloc(/*firstHeapAllocation=*/1000);
    const int N = SkOpts::raster_pipeline_highp_stride;
    float* slotBase = this->allocateSlotData(&alloc);
    const float* slotEnd = slotBase + (N * fNumValueSlots);
    const float* tempStackBase = slotEnd;
    const float* tempStackEnd = tempStackBase + (N * fNumTempStackSlots);

    // Instantiate this program.
    SkRasterPipeline pipeline(&alloc);
    this->appendStages(&pipeline, &alloc, slotBase);
    const SkRP::StageList* st = pipeline.getStageList();

    // The stage list is in reverse order, so let's flip it.
    struct Stage {
        SkRP::Stage op;
        void*       ctx;
    };
    SkTArray<Stage> stages;
    for (; st != nullptr; st = st->prev) {
        stages.push_back(Stage{st->stage, st->ctx});
    }
    std::reverse(stages.begin(), stages.end());

    // Emit the program's instruction list.
    for (int index = 0; index < stages.size(); ++index) {
        const Stage& stage = stages[index];

        // Interpret the context value as a branch offset.
        auto BranchOffset = [&](void* ctx) -> std::string {
            int *ctxAsInt = static_cast<int*>(ctx);
            return SkSL::String::printf("%+d (#%d)", *ctxAsInt, *ctxAsInt + index + 1);
        };

        // Interpret the context value as a 32-bit immediate value of unknown type (int/float).
        auto ImmCtx = [&](void* ctx) -> std::string {
            // Start with `0x3F800000` as a baseline.
            uint32_t immUnsigned;
            memcpy(&immUnsigned, &ctx, sizeof(uint32_t));
            auto text = SkSL::String::printf("0x%08X", immUnsigned);

            // Extend it to `0x3F800000 (1.0)` for finite floating point values.
            float immFloat;
            memcpy(&immFloat, &ctx, sizeof(float));
            if (std::isfinite(immFloat)) {
                text += " (";
                text += skstd::to_string(immFloat);
                text += ")";
            }
            return text;
        };

        // Print `1` for single slots and `1..3` for ranges of slots.
        auto AsRange = [](int first, int count) -> std::string {
            std::string text = std::to_string(first);
            if (count > 1) {
                text += ".." + std::to_string(first + count - 1);
            }
            return text;
        };

        // Interpret the context value as a pointer, most likely to a slot range.
        auto PtrCtx = [&](void* ctx, int numSlots) -> std::string {
            float *ctxAsSlot = static_cast<float*>(ctx);
            if (fDebugTrace) {
                // Handle pointers to named slots.
                if (ctxAsSlot >= slotBase && ctxAsSlot < slotEnd) {
                    int slotIdx = ctxAsSlot - slotBase;
                    SkASSERT((slotIdx % N) == 0);
                    slotIdx /= N;
                    if (slotIdx < (int)fDebugTrace->fSlotInfo.size()) {
                        const SlotDebugInfo& slotInfo = fDebugTrace->fSlotInfo[slotIdx];
                        // If we're covering the entire slot, return `name`.
                        if (numSlots == slotInfo.columns * slotInfo.rows) {
                            return slotInfo.name;
                        }
                        // If we are only covering part of the slot, return `name(1..2)`.
                        return slotInfo.name + "(" +
                               AsRange(slotInfo.componentIndex, numSlots) + ")";
                    }
                }
            }
            // Handle pointers to value slots (when no debug info exists).
            if (ctxAsSlot >= slotBase && ctxAsSlot < slotEnd) {
                int valueIdx = ctxAsSlot - slotBase;
                SkASSERT((valueIdx % N) == 0);
                return "v" + AsRange(valueIdx / N, numSlots);
            }
            // Handle pointers to temporary stack slots.
            if (ctxAsSlot >= tempStackBase && ctxAsSlot < tempStackEnd) {
                int stackIdx = ctxAsSlot - tempStackBase;
                SkASSERT((stackIdx % N) == 0);
                return "$" + AsRange(stackIdx / N, numSlots);
            }
            // This pointer is out of our expected bounds; this might happen at the program edges.
            return "ExternalPtr(" + AsRange(0, numSlots) + ")";
        };

        // Interpret the context value as a pointer to two adjacent values.
        auto AdjacentPtrCtx = [&](void* ctx, int numSlots) -> std::tuple<std::string, std::string> {
            float *ctxAsSlot = static_cast<float*>(ctx);
            return std::make_tuple(PtrCtx(ctxAsSlot, numSlots),
                                   PtrCtx(ctxAsSlot + (N * numSlots), numSlots));
        };

        // Interpret the context value as a CopySlots structure.
        auto CopySlotsCtx = [&](void* v, int numSlots) -> std::tuple<std::string, std::string> {
            auto *ctx = static_cast<SkRasterPipeline_CopySlotsCtx*>(v);
            return std::make_tuple(PtrCtx(ctx->dst, numSlots),
                                   PtrCtx(ctx->src, numSlots));
        };

        // Interpret the context value as a CopySlots structure.
        auto AdjacentCopySlotsCtx = [&](void* v) -> std::tuple<std::string, std::string> {
            auto *ctx = static_cast<SkRasterPipeline_CopySlotsCtx*>(v);
            int numSlots = ctx->src - ctx->dst;
            return std::make_tuple(PtrCtx(ctx->dst, numSlots),
                                   PtrCtx(ctx->src, numSlots));
        };

        std::string opArg1, opArg2;
        switch (stage.op) {
            case SkRP::immediate_f:
                opArg1 = ImmCtx(stage.ctx);
                break;

            case SkRP::load_unmasked:
            case SkRP::load_condition_mask:
            case SkRP::store_condition_mask:
            case SkRP::load_loop_mask:
            case SkRP::store_loop_mask:
            case SkRP::merge_loop_mask:
            case SkRP::load_return_mask:
            case SkRP::store_return_mask:
            case SkRP::store_masked:
            case SkRP::store_unmasked:
            case SkRP::bitwise_not:
            case SkRP::zero_slot_unmasked:
                opArg1 = PtrCtx(stage.ctx, 1);
                break;

            case SkRP::store_src_rg:
            case SkRP::zero_2_slots_unmasked:
                opArg1 = PtrCtx(stage.ctx, 2);
                break;

            case SkRP::zero_3_slots_unmasked:
                opArg1 = PtrCtx(stage.ctx, 3);
                break;

            case SkRP::load_src:
            case SkRP::load_dst:
            case SkRP::store_src:
            case SkRP::store_dst:
            case SkRP::zero_4_slots_unmasked:
                opArg1 = PtrCtx(stage.ctx, 4);
                break;

            case SkRP::copy_slot_masked:
            case SkRP::copy_slot_unmasked:
                std::tie(opArg1, opArg2) = CopySlotsCtx(stage.ctx, 1);
                break;

            case SkRP::copy_2_slots_masked:
            case SkRP::copy_2_slots_unmasked:
                std::tie(opArg1, opArg2) = CopySlotsCtx(stage.ctx, 2);
                break;

            case SkRP::copy_3_slots_masked:
            case SkRP::copy_3_slots_unmasked:
                std::tie(opArg1, opArg2) = CopySlotsCtx(stage.ctx, 3);
                break;

            case SkRP::copy_4_slots_masked:
            case SkRP::copy_4_slots_unmasked:
                std::tie(opArg1, opArg2) = CopySlotsCtx(stage.ctx, 4);
                break;

            case SkRP::merge_condition_mask:
            case SkRP::bitwise_and: case SkRP::bitwise_or: case SkRP::bitwise_xor:
            case SkRP::add_float:   case SkRP::add_int:
            case SkRP::cmplt_float: case SkRP::cmplt_int:
            case SkRP::cmple_float: case SkRP::cmple_int:
            case SkRP::cmpeq_float: case SkRP::cmpeq_int:
            case SkRP::cmpne_float: case SkRP::cmpne_int:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 1);
                break;

            case SkRP::add_2_floats:   case SkRP::add_2_ints:
            case SkRP::cmplt_2_floats: case SkRP::cmplt_2_ints:
            case SkRP::cmple_2_floats: case SkRP::cmple_2_ints:
            case SkRP::cmpeq_2_floats: case SkRP::cmpeq_2_ints:
            case SkRP::cmpne_2_floats: case SkRP::cmpne_2_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 2);
                break;

            case SkRP::add_3_floats:   case SkRP::add_3_ints:
            case SkRP::cmplt_3_floats: case SkRP::cmplt_3_ints:
            case SkRP::cmple_3_floats: case SkRP::cmple_3_ints:
            case SkRP::cmpeq_3_floats: case SkRP::cmpeq_3_ints:
            case SkRP::cmpne_3_floats: case SkRP::cmpne_3_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 3);
                break;

            case SkRP::add_4_floats:   case SkRP::add_4_ints:
            case SkRP::cmplt_4_floats: case SkRP::cmplt_4_ints:
            case SkRP::cmple_4_floats: case SkRP::cmple_4_ints:
            case SkRP::cmpeq_4_floats: case SkRP::cmpeq_4_ints:
            case SkRP::cmpne_4_floats: case SkRP::cmpne_4_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 4);
                break;

            case SkRP::add_n_floats:   case SkRP::add_n_ints:
            case SkRP::cmplt_n_floats: case SkRP::cmplt_n_ints:
            case SkRP::cmple_n_floats: case SkRP::cmple_n_ints:
            case SkRP::cmpeq_n_floats: case SkRP::cmpeq_n_ints:
            case SkRP::cmpne_n_floats: case SkRP::cmpne_n_ints:
                std::tie(opArg1, opArg2) = AdjacentCopySlotsCtx(stage.ctx);
                break;

            case SkRP::jump:
            case SkRP::branch_if_any_active_lanes:
            case SkRP::branch_if_no_active_lanes:
                opArg1 = BranchOffset(stage.ctx);
                break;

            default:
                break;
        }

        const char* opName = SkRasterPipeline::GetStageName(stage.op);
        std::string opText;
        switch (stage.op) {
            case SkRP::init_lane_masks:
                opText = "CondMask = LoopMask = RetMask = true";
                break;

            case SkRP::load_condition_mask:
                opText = "CondMask = " + opArg1;
                break;

            case SkRP::store_condition_mask:
                opText = opArg1 + " = CondMask";
                break;

            case SkRP::merge_condition_mask:
                opText = "CondMask = " + opArg1 + " & " + opArg2;
                break;

            case SkRP::load_loop_mask:
                opText = "LoopMask = " + opArg1;
                break;

            case SkRP::store_loop_mask:
                opText = opArg1 + " = LoopMask";
                break;

            case SkRP::mask_off_loop_mask:
                opText = "LoopMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case SkRP::merge_loop_mask:
                opText = "LoopMask &= " + opArg1;
                break;

            case SkRP::load_return_mask:
                opText = "RetMask = " + opArg1;
                break;

            case SkRP::store_return_mask:
                opText = opArg1 + " = RetMask";
                break;

            case SkRP::mask_off_return_mask:
                opText = "RetMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case SkRP::immediate_f:
            case SkRP::load_unmasked:
                opText = "src.r = " + opArg1;
                break;

            case SkRP::store_unmasked:
                opText = opArg1 + " = src.r";
                break;

            case SkRP::store_src_rg:
                opText = opArg1 + " = src.rg";
                break;

            case SkRP::store_src:
                opText = opArg1 + " = src.rgba";
                break;

            case SkRP::store_dst:
                opText = opArg1 + " = dst.rgba";
                break;

            case SkRP::load_src:
                opText = "src.rgba = " + opArg1;
                break;

            case SkRP::load_dst:
                opText = "dst.rgba = " + opArg1;
                break;

            case SkRP::store_masked:
                opText = opArg1 + " = Mask(src.r)";
                break;

            case SkRP::bitwise_and:
                opText = opArg1 + " &= " + opArg2;
                break;

            case SkRP::bitwise_or:
                opText = opArg1 + " |= " + opArg2;
                break;

            case SkRP::bitwise_xor:
                opText = opArg1 + " ^= " + opArg2;
                break;

            case SkRP::bitwise_not:
                opText = opArg1 + " = ~" + opArg1;
                break;

            case SkRP::copy_slot_masked:      case SkRP::copy_2_slots_masked:
            case SkRP::copy_3_slots_masked:   case SkRP::copy_4_slots_masked:
                opText = opArg1 + " = Mask(" + opArg2 + ")";
                break;

            case SkRP::copy_slot_unmasked:    case SkRP::copy_2_slots_unmasked:
            case SkRP::copy_3_slots_unmasked: case SkRP::copy_4_slots_unmasked:
                opText = opArg1 + " = " + opArg2;
                break;

            case SkRP::zero_slot_unmasked:    case SkRP::zero_2_slots_unmasked:
            case SkRP::zero_3_slots_unmasked: case SkRP::zero_4_slots_unmasked:
                opText = opArg1 + " = 0";
                break;

            case SkRP::add_float:    case SkRP::add_int:
            case SkRP::add_2_floats: case SkRP::add_2_ints:
            case SkRP::add_3_floats: case SkRP::add_3_ints:
            case SkRP::add_4_floats: case SkRP::add_4_ints:
            case SkRP::add_n_floats: case SkRP::add_n_ints:
                opText = opArg1 + " += " + opArg2;
                break;

            case SkRP::cmplt_float:    case SkRP::cmplt_int:
            case SkRP::cmplt_2_floats: case SkRP::cmplt_2_ints:
            case SkRP::cmplt_3_floats: case SkRP::cmplt_3_ints:
            case SkRP::cmplt_4_floats: case SkRP::cmplt_4_ints:
            case SkRP::cmplt_n_floats: case SkRP::cmplt_n_ints:
                opText = opArg1 + " = lessThan(" + opArg1 + ", " + opArg2 + ")";
                break;

            case SkRP::cmple_float:    case SkRP::cmple_int:
            case SkRP::cmple_2_floats: case SkRP::cmple_2_ints:
            case SkRP::cmple_3_floats: case SkRP::cmple_3_ints:
            case SkRP::cmple_4_floats: case SkRP::cmple_4_ints:
            case SkRP::cmple_n_floats: case SkRP::cmple_n_ints:
                opText = opArg1 + " = lessThanEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case SkRP::cmpeq_float:    case SkRP::cmpeq_int:
            case SkRP::cmpeq_2_floats: case SkRP::cmpeq_2_ints:
            case SkRP::cmpeq_3_floats: case SkRP::cmpeq_3_ints:
            case SkRP::cmpeq_4_floats: case SkRP::cmpeq_4_ints:
            case SkRP::cmpeq_n_floats: case SkRP::cmpeq_n_ints:
                opText = opArg1 + " = equal(" + opArg1 + ", " + opArg2 + ")";
                break;

            case SkRP::cmpne_float:    case SkRP::cmpne_int:
            case SkRP::cmpne_2_floats: case SkRP::cmpne_2_ints:
            case SkRP::cmpne_3_floats: case SkRP::cmpne_3_ints:
            case SkRP::cmpne_4_floats: case SkRP::cmpne_4_ints:
            case SkRP::cmpne_n_floats: case SkRP::cmpne_n_ints:
                opText = opArg1 + " = notEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case SkRP::jump:
            case SkRP::branch_if_any_active_lanes:
            case SkRP::branch_if_no_active_lanes:
                opText = std::string(opName) + " " + opArg1;
                break;

            default:
                break;
        }

        std::string line = !opText.empty()
                ? SkSL::String::printf("% 5d. %-30s %s\n", index + 1, opName, opText.c_str())
                : SkSL::String::printf("% 5d. %s\n", index + 1, opName);

        out->writeText(line.c_str());
    }
#endif
}

}  // namespace RP
}  // namespace SkSL
