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
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkRPDebugTrace.h"
#include "src/sksl/tracing/SkSLDebugInfo.h"

#if !defined(SKSL_STANDALONE)
#include "src/core/SkRasterPipeline.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace SkSL {
namespace RP {

using RPOp = SkRasterPipelineOp;

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
         BuilderOp::mix_n_floats

void Builder::unary_op(BuilderOp op, int32_t slots) {
    switch (op) {
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

            case BuilderOp::push_literal_f:
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

void Builder::copy_stack_to_slots(SlotRange dst, int offsetFromStackTop) {
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

void Builder::pop_return_mask() {
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

void Builder::swizzle(int consumedSlots, SkSpan<const int8_t> elementSpan) {
    // Consumes `consumedSlots` elements on the stack, then generates `elementSpan.size()` elements.
    SkASSERT(consumedSlots >= 0);

    // We only allow up to 16 elements, and they can only reach 0-15 slots, due to nybble packing.
    int numElements = elementSpan.size();
    SkASSERT(numElements <= 16);
    SkASSERT(std::all_of(elementSpan.begin(), elementSpan.end(), [](int8_t e){ return e >= 0; }));
    SkASSERT(std::all_of(elementSpan.begin(), elementSpan.end(), [](int8_t e){ return e <= 0xF; }));

    // Make a local copy of the element array.
    int8_t elements[16] = {};
    std::copy(elementSpan.begin(), elementSpan.end(), std::begin(elements));

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
    return std::make_unique<Program>(std::move(fInstructions), numValueSlots, numUniformSlots,
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
        case BuilderOp::push_uniform:
        case BuilderOp::push_zeros:
        case BuilderOp::push_clone:
        case BuilderOp::push_clone_from_stack:
            return inst.fImmA;

        case BuilderOp::pop_condition_mask:
        case BuilderOp::pop_loop_mask:
        case BuilderOp::pop_return_mask:
            return -1;

        case ALL_MULTI_SLOT_BINARY_OP_CASES:
        case BuilderOp::discard_stack:
        case BuilderOp::select:
            return -inst.fImmA;

        case ALL_MULTI_SLOT_TERNARY_OP_CASES:
            return 2 * -inst.fImmA;

        case BuilderOp::swizzle_1:
            return 1 - inst.fImmA;
        case BuilderOp::swizzle_2:
            return 2 - inst.fImmA;
        case BuilderOp::swizzle_3:
            return 3 - inst.fImmA;
        case BuilderOp::swizzle_4:
            return 4 - inst.fImmA;

        case BuilderOp::shuffle: {
            int consumed = inst.fImmA >> 16;
            int generated = inst.fImmA & 0xFFFF;
            return generated - consumed;
        }
        case ALL_MULTI_SLOT_UNARY_OP_CASES:
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
                 int numUniformSlots,
                 int numLabels,
                 int numBranches,
                 SkRPDebugTrace* debugTrace)
        : fInstructions(std::move(instrs))
        , fNumValueSlots(numValueSlots)
        , fNumUniformSlots(numUniformSlots)
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

void Program::appendCopy(SkTArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         SkRasterPipelineOp baseStage,
                         float* dst, int dstStride,
                         const float* src, int srcStride,
                         int numSlots) {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendCopy(pipeline, alloc, baseStage, dst, dstStride, src, srcStride,/*numSlots=*/4);
        dst += 4 * dstStride;
        src += 4 * srcStride;
        numSlots -= 4;
    }

    if (numSlots > 0) {
        SkASSERT(numSlots <= 4);
        auto stage = (SkRasterPipelineOp)((int)baseStage + numSlots - 1);
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
                                      int numSlots) {
    this->appendCopy(pipeline, alloc,
                     SkRasterPipelineOp::copy_slot_unmasked,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendCopySlotsMasked(SkTArray<Stage>* pipeline,
                                    SkArenaAlloc* alloc,
                                    float* dst,
                                    const float* src,
                                    int numSlots) {
    this->appendCopy(pipeline, alloc,
                     SkRasterPipelineOp::copy_slot_masked,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/SkOpts::raster_pipeline_highp_stride,
                     numSlots);
}

void Program::appendCopyConstants(SkTArray<Stage>* pipeline,
                                  SkArenaAlloc* alloc,
                                  float* dst,
                                  const float* src,
                                  int numSlots) {
    this->appendCopy(pipeline, alloc,
                     SkRasterPipelineOp::copy_constant,
                     dst, /*dstStride=*/SkOpts::raster_pipeline_highp_stride,
                     src, /*srcStride=*/1,
                     numSlots);
}

void Program::appendMultiSlotUnaryOp(SkTArray<Stage>* pipeline, SkRasterPipelineOp baseStage,
                                     float* dst, int numSlots) {
    SkASSERT(numSlots >= 0);
    while (numSlots > 4) {
        this->appendMultiSlotUnaryOp(pipeline, baseStage, dst, /*numSlots=*/4);
        dst += 4 * SkOpts::raster_pipeline_highp_stride;
        numSlots -= 4;
    }

    SkASSERT(numSlots <= 4);
    auto stage = (SkRasterPipelineOp)((int)baseStage + numSlots - 1);
    pipeline->push_back({stage, dst});
}

void Program::appendAdjacentMultiSlotBinaryOp(SkTArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                              SkRasterPipelineOp baseStage,
                                              float* dst, const float* src, int numSlots) {
    // The source and destination must be directly next to one another.
    SkASSERT(numSlots >= 0);
    SkASSERT((dst + SkOpts::raster_pipeline_highp_stride * numSlots) == src);

    if (numSlots > 4) {
        auto ctx = alloc->make<SkRasterPipeline_BinaryOpCtx>();
        ctx->dst = dst;
        ctx->src = src;
        pipeline->push_back({baseStage, ctx});
        return;
    }
    if (numSlots > 0) {
        auto specializedStage = (SkRasterPipelineOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, dst});
    }
}

void Program::appendAdjacentMultiSlotTernaryOp(SkTArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                               SkRasterPipelineOp baseStage, float* dst,
                                               const float* src0, const float* src1, int numSlots) {
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
        auto specializedStage = (SkRasterPipelineOp)((int)baseStage + numSlots);
        pipeline->push_back({specializedStage, dst});
    }
}

void Program::appendStackRewind(SkTArray<Stage>* pipeline) {
#if defined(SKSL_STANDALONE) || !SK_HAS_MUSTTAIL
    pipeline->push_back({RPOp::stack_rewind, nullptr});
#endif
}

template <typename T>
static void* context_bit_pun(T val) {
    static_assert(sizeof(T) <= sizeof(void*));
    void* contextBits = nullptr;
    memcpy(&contextBits, &val, sizeof(val));
    return contextBits;
}

Program::SlotData Program::allocateSlotData(SkArenaAlloc* alloc) {
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

void Program::appendStages(SkRasterPipeline* pipeline,
                           SkArenaAlloc* alloc,
                           SkSpan<const float> uniforms) {
    SkTArray<Stage> stages;
    this->makeStages(&stages, alloc, uniforms, this->allocateSlotData(alloc));

    for (const Stage& stage : stages) {
        switch (stage.op) {
            case RPOp::stack_rewind: pipeline->append_stack_rewind();       break;
            default:                 pipeline->append(stage.op, stage.ctx); break;
        }
    }
}

#endif

void Program::makeStages(SkTArray<Stage>* pipeline,
                         SkArenaAlloc* alloc,
                         SkSpan<const float> uniforms,
                         const SlotData& slots) {
    SkASSERT(fNumUniformSlots == SkToInt(uniforms.size()));

    const int N = SkOpts::raster_pipeline_highp_stride;
    StackDepthMap tempStackDepth;
    int currentStack = 0;
    int mostRecentRewind = 0;

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
        tempStackMap[idx] = slots.stack.begin() + (pos * N);
        pos += depth;
    }

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
                // Write the absolute pipeline position into the label offset list. We will go over
                // the branch targets at the end and fix them up.
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                labelOffsets[inst.fImmA] = pipeline->size();
                break;

            case BuilderOp::jump:
            case BuilderOp::branch_if_any_active_lanes:
            case BuilderOp::branch_if_no_active_lanes:
                // If we have already encountered the label associated with this branch, this is a
                // backwards branch. Add a stack-rewind immediately before the branch to ensure that
                // long-running loops don't use an unbounded amount of stack space.
                if (labelOffsets[inst.fImmA] >= 0) {
                    this->appendStackRewind(pipeline);
                    mostRecentRewind = pipeline->size();
                }

                // Write the absolute pipeline position into the branch targets, because the
                // associated label might not have been reached yet. We will go back over the branch
                // targets at the end and fix them up.
                SkASSERT(inst.fImmA >= 0 && inst.fImmA < fNumLabels);
                SkASSERT(currentBranchOp >= 0 && currentBranchOp < fNumBranches);
                branchTargets[currentBranchOp] = pipeline->size();
                branchGoesToLabel[currentBranchOp] = inst.fImmA;
                pipeline->push_back({(RPOp)inst.fOp, &branchTargets[currentBranchOp]});
                ++currentBranchOp;
                break;

            case BuilderOp::init_lane_masks:
                pipeline->push_back({RPOp::init_lane_masks, nullptr});
                break;

            case BuilderOp::store_src_rg:
                pipeline->push_back({RPOp::store_src_rg, SlotA()});
                break;

            case BuilderOp::store_src:
                pipeline->push_back({RPOp::store_src, SlotA()});
                break;

            case BuilderOp::store_dst:
                pipeline->push_back({RPOp::store_dst, SlotA()});
                break;

            case BuilderOp::load_src:
                pipeline->push_back({RPOp::load_src, SlotA()});
                break;

            case BuilderOp::load_dst:
                pipeline->push_back({RPOp::load_dst, SlotA()});
                break;

            case BuilderOp::immediate_f: {
                pipeline->push_back({RPOp::immediate_f, context_bit_pun(inst.fImmA)});
                break;
            }
            case BuilderOp::load_unmasked:
                pipeline->push_back({RPOp::load_unmasked, SlotA()});
                break;

            case BuilderOp::store_unmasked:
                pipeline->push_back({RPOp::store_unmasked, SlotA()});
                break;

            case BuilderOp::store_masked:
                pipeline->push_back({RPOp::store_masked, SlotA()});
                break;

            case ALL_MULTI_SLOT_UNARY_OP_CASES: {
                float* dst = tempStackPtr - (inst.fImmA * N);
                this->appendMultiSlotUnaryOp(pipeline, (RPOp)inst.fOp, dst, inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_BINARY_OP_CASES: {
                float* src = tempStackPtr - (inst.fImmA * N);
                float* dst = tempStackPtr - (inst.fImmA * 2 * N);
                this->appendAdjacentMultiSlotBinaryOp(pipeline, alloc, (RPOp)inst.fOp,
                                                      dst, src, inst.fImmA);
                break;
            }
            case ALL_MULTI_SLOT_TERNARY_OP_CASES: {
                float* src1 = tempStackPtr - (inst.fImmA * N);
                float* src0 = tempStackPtr - (inst.fImmA * 2 * N);
                float* dst  = tempStackPtr - (inst.fImmA * 3 * N);
                this->appendAdjacentMultiSlotTernaryOp(pipeline, alloc, (RPOp)inst.fOp,
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
                this->appendMultiSlotUnaryOp(pipeline, RPOp::zero_slot_unmasked,
                                             SlotA(), inst.fImmA);
                break;

            case BuilderOp::swizzle_1:
            case BuilderOp::swizzle_2:
            case BuilderOp::swizzle_3:
            case BuilderOp::swizzle_4: {
                auto* ctx = alloc->make<SkRasterPipeline_SwizzleCtx>();
                ctx->ptr = tempStackPtr - (N * inst.fImmA);
                // Unpack component nybbles into byte-offsets pointing at stack slots.
                int components = inst.fImmB;
                for (size_t index = 0; index < std::size(ctx->offsets); ++index) {
                    ctx->offsets[index] = (components & 3) * N * sizeof(float);
                    components >>= 4;
                }
                pipeline->push_back({(RPOp)inst.fOp, ctx});
                break;
            }
            case BuilderOp::shuffle: {
                int consumed = inst.fImmA >> 16;
                int generated = inst.fImmA & 0xFFFF;

                auto* ctx = alloc->make<SkRasterPipeline_ShuffleCtx>();
                ctx->ptr = tempStackPtr - (N * consumed);
                ctx->count = generated;
                // Unpack immB and immC from nybble form into an offset array.
                int packed = inst.fImmB;
                for (int index = 0; index < 8; ++index) {
                    ctx->offsets[index] = (packed & 0xF) * N * sizeof(float);
                    packed >>= 4;
                }
                packed = inst.fImmC;
                for (int index = 8; index < 16; ++index) {
                    ctx->offsets[index] = (packed & 0xF) * N * sizeof(float);
                    packed >>= 4;
                }
                pipeline->push_back({RPOp::shuffle, ctx});
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
                this->appendMultiSlotUnaryOp(pipeline, RPOp::zero_slot_unmasked, dst, inst.fImmA);
                break;
            }
            case BuilderOp::push_condition_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({RPOp::store_condition_mask, dst});
                break;
            }
            case BuilderOp::pop_condition_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({RPOp::load_condition_mask, src});
                break;
            }
            case BuilderOp::merge_condition_mask: {
                float* ptr = tempStackPtr - (2 * N);
                pipeline->push_back({RPOp::merge_condition_mask, ptr});
                break;
            }
            case BuilderOp::push_loop_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({RPOp::store_loop_mask, dst});
                break;
            }
            case BuilderOp::pop_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({RPOp::load_loop_mask, src});
                break;
            }
            case BuilderOp::mask_off_loop_mask:
                pipeline->push_back({RPOp::mask_off_loop_mask, nullptr});
                break;

            case BuilderOp::reenable_loop_mask:
                pipeline->push_back({RPOp::reenable_loop_mask, SlotA()});
                break;

            case BuilderOp::merge_loop_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({RPOp::merge_loop_mask, src});
                break;
            }
            case BuilderOp::push_return_mask: {
                float* dst = tempStackPtr;
                pipeline->push_back({RPOp::store_return_mask, dst});
                break;
            }
            case BuilderOp::pop_return_mask: {
                float* src = tempStackPtr - (1 * N);
                pipeline->push_back({RPOp::load_return_mask, src});
                break;
            }
            case BuilderOp::mask_off_return_mask:
                pipeline->push_back({RPOp::mask_off_return_mask, nullptr});
                break;

            case BuilderOp::push_literal_f: {
                float* dst = tempStackPtr;
                if (inst.fImmA == 0) {
                    pipeline->push_back({RPOp::zero_slot_unmasked, dst});
                    break;
                }
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

    // Fix up every branch target.
    for (int index = 0; index < fNumBranches; ++index) {
        int branchFromIdx = branchTargets[index];
        int branchToIdx = labelOffsets[branchGoesToLabel[index]];
        branchTargets[index] = branchToIdx - branchFromIdx;
    }
}

void Program::dump(SkWStream* out) {
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

    // Emit the program's instruction list.
    for (int index = 0; index < stages.size(); ++index) {
        const Stage& stage = stages[index];

        // Interpret the context value as a branch offset.
        auto BranchOffset = [&](const void* ctx) -> std::string {
            const int *ctxAsInt = static_cast<const int*>(ctx);
            return SkSL::String::printf("%+d (#%d)", *ctxAsInt, *ctxAsInt + index + 1);
        };

        // Print a 32-bit immediate value of unknown type (int/float).
        auto Imm = [&](float immFloat) -> std::string {
            // Start with `0x3F800000` as a baseline.
            uint32_t immUnsigned;
            memcpy(&immUnsigned, &immFloat, sizeof(uint32_t));
            auto text = SkSL::String::printf("0x%08X", immUnsigned);

            // Extend it to `0x3F800000 (1.0)` for finite floating point values.
            if (std::isfinite(immFloat)) {
                text += " (";
                text += skstd::to_string(immFloat);
                text += ")";
            }
            return text;
        };

        // Interpret the context pointer as a 32-bit immediate value of unknown type (int/float).
        auto ImmCtx = [&](const void* ctx) -> std::string {
            float f;
            memcpy(&f, &ctx, sizeof(float));
            return Imm(f);
        };

        // Print `1` for single slots and `1..3` for ranges of slots.
        auto AsRange = [](int first, int count) -> std::string {
            std::string text = std::to_string(first);
            if (count > 1) {
                text += ".." + std::to_string(first + count - 1);
            }
            return text;
        };

        // Attempts to interpret the passed-in pointer as a uniform range.
        auto UniformPtrCtx = [&](const float* ptr, int numSlots) -> std::string {
            if (fDebugTrace) {
                // Handle pointers to named uniform slots.
                if (ptr >= uniforms.begin() && ptr < uniforms.end()) {
                    int slotIdx = ptr - uniforms.begin();
                    if (slotIdx < (int)fDebugTrace->fUniformInfo.size()) {
                        const SlotDebugInfo& slotInfo = fDebugTrace->fUniformInfo[slotIdx];
                        if (!slotInfo.name.empty()) {
                            // If we're covering the entire uniform, return `uniName`.
                            if (numSlots == slotInfo.columns * slotInfo.rows) {
                                return slotInfo.name;
                            }
                            // If we are only covering part of the uniform, return `uniName(1..2)`.
                            return slotInfo.name + "(" +
                                   AsRange(slotInfo.componentIndex, numSlots) + ")";
                        }
                    }
                }
            }
            // Handle pointers to uniforms (when no debug info exists).
            if (ptr >= uniforms.begin() && ptr < uniforms.end()) {
                int uniformIdx = ptr - uniforms.begin();
                return "u" + AsRange(uniformIdx, numSlots);
            }
            return {};
        };

        // Attempts to interpret the passed-in pointer as a value slot range.
        auto ValuePtrCtx = [&](const float* ptr, int numSlots) -> std::string {
            if (fDebugTrace) {
                // Handle pointers to named value slots.
                if (ptr >= slots.values.begin() && ptr < slots.values.end()) {
                    int slotIdx = ptr - slots.values.begin();
                    SkASSERT((slotIdx % N) == 0);
                    slotIdx /= N;
                    if (slotIdx < (int)fDebugTrace->fSlotInfo.size()) {
                        const SlotDebugInfo& slotInfo = fDebugTrace->fSlotInfo[slotIdx];
                        if (!slotInfo.name.empty()) {
                            // If we're covering the entire slot, return `valueName`.
                            if (numSlots == slotInfo.columns * slotInfo.rows) {
                                return slotInfo.name;
                            }
                            // If we are only covering part of the slot, return `valueName(1..2)`.
                            return slotInfo.name + "(" +
                                   AsRange(slotInfo.componentIndex, numSlots) + ")";
                        }
                    }
                }
            }
            // Handle pointers to value slots (when no debug info exists).
            if (ptr >= slots.values.begin() && ptr < slots.values.end()) {
                int valueIdx = ptr - slots.values.begin();
                SkASSERT((valueIdx % N) == 0);
                return "v" + AsRange(valueIdx / N, numSlots);
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

        // Interpret the context value as a Swizzle structure. Note that the slot-width of the
        // source expression is not preserved in the instruction encoding, so we need to do our best
        // using the data we have. (e.g., myFloat4.y would be indistinguishable from myFloat2.y.)
        auto SwizzleCtx = [&](RPOp op, const void* v) -> std::tuple<std::string, std::string> {
            const auto* ctx = static_cast<const SkRasterPipeline_SwizzleCtx*>(v);

            int destSlots = (int)op - (int)RPOp::swizzle_1 + 1;
            int highestComponent =
                    *std::max_element(std::begin(ctx->offsets), std::end(ctx->offsets)) /
                    (N * sizeof(float));

            std::string src = "(" + PtrCtx(ctx->ptr, std::max(destSlots, highestComponent + 1)) +
                              ").";
            for (int index = 0; index < destSlots; ++index) {
                if (ctx->offsets[index] == (0 * N * sizeof(float))) {
                    src.push_back('x');
                } else if (ctx->offsets[index] == (1 * N * sizeof(float))) {
                    src.push_back('y');
                } else if (ctx->offsets[index] == (2 * N * sizeof(float))) {
                    src.push_back('z');
                } else if (ctx->offsets[index] == (3 * N * sizeof(float))) {
                    src.push_back('w');
                } else {
                    src.push_back('?');
                }
            }

            return std::make_tuple(PtrCtx(ctx->ptr, destSlots), src);
        };

        // Interpret the context value as a Shuffle structure.
        auto ShuffleCtx = [&](RPOp op, const void* v) -> std::tuple<std::string, std::string> {
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
        switch (stage.op) {
            case RPOp::immediate_f:
                opArg1 = ImmCtx(stage.ctx);
                break;

            case RPOp::swizzle_1:
            case RPOp::swizzle_2:
            case RPOp::swizzle_3:
            case RPOp::swizzle_4:
                std::tie(opArg1, opArg2) = SwizzleCtx(stage.op, stage.ctx);
                break;

            case RPOp::shuffle:
                std::tie(opArg1, opArg2) = ShuffleCtx(stage.op, stage.ctx);
                break;

            case RPOp::load_unmasked:
            case RPOp::load_condition_mask:
            case RPOp::store_condition_mask:
            case RPOp::load_loop_mask:
            case RPOp::store_loop_mask:
            case RPOp::merge_loop_mask:
            case RPOp::reenable_loop_mask:
            case RPOp::load_return_mask:
            case RPOp::store_return_mask:
            case RPOp::store_masked:
            case RPOp::store_unmasked:
            case RPOp::zero_slot_unmasked:
            case RPOp::bitwise_not_int:
            case RPOp::cast_to_float_from_int: case RPOp::cast_to_float_from_uint:
            case RPOp::cast_to_int_from_float: case RPOp::cast_to_uint_from_float:
            case RPOp::abs_float:              case RPOp::abs_int:
            case RPOp::ceil_float:
            case RPOp::floor_float:
                opArg1 = PtrCtx(stage.ctx, 1);
                break;

            case RPOp::store_src_rg:
            case RPOp::zero_2_slots_unmasked:
            case RPOp::bitwise_not_2_ints:
            case RPOp::cast_to_float_from_2_ints: case RPOp::cast_to_float_from_2_uints:
            case RPOp::cast_to_int_from_2_floats: case RPOp::cast_to_uint_from_2_floats:
            case RPOp::abs_2_floats:              case RPOp::abs_2_ints:
            case RPOp::ceil_2_floats:
            case RPOp::floor_2_floats:
                opArg1 = PtrCtx(stage.ctx, 2);
                break;

            case RPOp::zero_3_slots_unmasked:
            case RPOp::bitwise_not_3_ints:
            case RPOp::cast_to_float_from_3_ints: case RPOp::cast_to_float_from_3_uints:
            case RPOp::cast_to_int_from_3_floats: case RPOp::cast_to_uint_from_3_floats:
            case RPOp::abs_3_floats:              case RPOp::abs_3_ints:
            case RPOp::ceil_3_floats:
            case RPOp::floor_3_floats:
                opArg1 = PtrCtx(stage.ctx, 3);
                break;

            case RPOp::load_src:
            case RPOp::load_dst:
            case RPOp::store_src:
            case RPOp::store_dst:
            case RPOp::zero_4_slots_unmasked:
            case RPOp::bitwise_not_4_ints:
            case RPOp::cast_to_float_from_4_ints: case RPOp::cast_to_float_from_4_uints:
            case RPOp::cast_to_int_from_4_floats: case RPOp::cast_to_uint_from_4_floats:
            case RPOp::abs_4_floats:              case RPOp::abs_4_ints:
            case RPOp::ceil_4_floats:
            case RPOp::floor_4_floats:
                opArg1 = PtrCtx(stage.ctx, 4);
                break;

            case RPOp::copy_constant:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 1);
                break;

            case RPOp::copy_2_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 2);
                break;

            case RPOp::copy_3_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 3);
                break;

            case RPOp::copy_4_constants:
                std::tie(opArg1, opArg2) = CopyConstantCtx(stage.ctx, 4);
                break;

            case RPOp::copy_slot_masked:
            case RPOp::copy_slot_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 1);
                break;

            case RPOp::copy_2_slots_masked:
            case RPOp::copy_2_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 2);
                break;

            case RPOp::copy_3_slots_masked:
            case RPOp::copy_3_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 3);
                break;

            case RPOp::copy_4_slots_masked:
            case RPOp::copy_4_slots_unmasked:
                std::tie(opArg1, opArg2) = BinaryOpCtx(stage.ctx, 4);
                break;

            case RPOp::merge_condition_mask:
            case RPOp::add_float:   case RPOp::add_int:
            case RPOp::sub_float:   case RPOp::sub_int:
            case RPOp::mul_float:   case RPOp::mul_int:
            case RPOp::div_float:   case RPOp::div_int:   case RPOp::div_uint:
                                    case RPOp::bitwise_and_int:
                                    case RPOp::bitwise_or_int:
                                    case RPOp::bitwise_xor_int:
            case RPOp::min_float:   case RPOp::min_int:   case RPOp::min_uint:
            case RPOp::max_float:   case RPOp::max_int:   case RPOp::max_uint:
            case RPOp::cmplt_float: case RPOp::cmplt_int: case RPOp::cmplt_uint:
            case RPOp::cmple_float: case RPOp::cmple_int: case RPOp::cmple_uint:
            case RPOp::cmpeq_float: case RPOp::cmpeq_int:
            case RPOp::cmpne_float: case RPOp::cmpne_int:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 1);
                break;

            case RPOp::mix_float:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 1);
                break;

            case RPOp::add_2_floats:   case RPOp::add_2_ints:
            case RPOp::sub_2_floats:   case RPOp::sub_2_ints:
            case RPOp::mul_2_floats:   case RPOp::mul_2_ints:
            case RPOp::div_2_floats:   case RPOp::div_2_ints:   case RPOp::div_2_uints:
                                       case RPOp::bitwise_and_2_ints:
                                       case RPOp::bitwise_or_2_ints:
                                       case RPOp::bitwise_xor_2_ints:
            case RPOp::min_2_floats:   case RPOp::min_2_ints:   case RPOp::min_2_uints:
            case RPOp::max_2_floats:   case RPOp::max_2_ints:   case RPOp::max_2_uints:
            case RPOp::cmplt_2_floats: case RPOp::cmplt_2_ints: case RPOp::cmplt_2_uints:
            case RPOp::cmple_2_floats: case RPOp::cmple_2_ints: case RPOp::cmple_2_uints:
            case RPOp::cmpeq_2_floats: case RPOp::cmpeq_2_ints:
            case RPOp::cmpne_2_floats: case RPOp::cmpne_2_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 2);
                break;

            case RPOp::mix_2_floats:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 2);
                break;

            case RPOp::add_3_floats:   case RPOp::add_3_ints:
            case RPOp::sub_3_floats:   case RPOp::sub_3_ints:
            case RPOp::mul_3_floats:   case RPOp::mul_3_ints:
            case RPOp::div_3_floats:   case RPOp::div_3_ints:   case RPOp::div_3_uints:
                                       case RPOp::bitwise_and_3_ints:
                                       case RPOp::bitwise_or_3_ints:
                                       case RPOp::bitwise_xor_3_ints:
            case RPOp::min_3_floats:   case RPOp::min_3_ints:   case RPOp::min_3_uints:
            case RPOp::max_3_floats:   case RPOp::max_3_ints:   case RPOp::max_3_uints:
            case RPOp::cmplt_3_floats: case RPOp::cmplt_3_ints: case RPOp::cmplt_3_uints:
            case RPOp::cmple_3_floats: case RPOp::cmple_3_ints: case RPOp::cmple_3_uints:
            case RPOp::cmpeq_3_floats: case RPOp::cmpeq_3_ints:
            case RPOp::cmpne_3_floats: case RPOp::cmpne_3_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 3);
                break;

            case RPOp::mix_3_floats:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 3);
                break;

            case RPOp::add_4_floats:   case RPOp::add_4_ints:
            case RPOp::sub_4_floats:   case RPOp::sub_4_ints:
            case RPOp::mul_4_floats:   case RPOp::mul_4_ints:
            case RPOp::div_4_floats:   case RPOp::div_4_ints:   case RPOp::div_4_uints:
                                       case RPOp::bitwise_and_4_ints:
                                       case RPOp::bitwise_or_4_ints:
                                       case RPOp::bitwise_xor_4_ints:
            case RPOp::min_4_floats:   case RPOp::min_4_ints:   case RPOp::min_4_uints:
            case RPOp::max_4_floats:   case RPOp::max_4_ints:   case RPOp::max_4_uints:
            case RPOp::cmplt_4_floats: case RPOp::cmplt_4_ints: case RPOp::cmplt_4_uints:
            case RPOp::cmple_4_floats: case RPOp::cmple_4_ints: case RPOp::cmple_4_uints:
            case RPOp::cmpeq_4_floats: case RPOp::cmpeq_4_ints:
            case RPOp::cmpne_4_floats: case RPOp::cmpne_4_ints:
                std::tie(opArg1, opArg2) = AdjacentPtrCtx(stage.ctx, 4);
                break;

            case RPOp::mix_4_floats:
                std::tie(opArg1, opArg2, opArg3) = Adjacent3PtrCtx(stage.ctx, 4);
                break;

            case RPOp::add_n_floats:   case RPOp::add_n_ints:
            case RPOp::sub_n_floats:   case RPOp::sub_n_ints:
            case RPOp::mul_n_floats:   case RPOp::mul_n_ints:
            case RPOp::div_n_floats:   case RPOp::div_n_ints:   case RPOp::div_n_uints:
                                       case RPOp::bitwise_and_n_ints:
                                       case RPOp::bitwise_or_n_ints:
                                       case RPOp::bitwise_xor_n_ints:
            case RPOp::min_n_floats:   case RPOp::min_n_ints:   case RPOp::min_n_uints:
            case RPOp::max_n_floats:   case RPOp::max_n_ints:   case RPOp::max_n_uints:
            case RPOp::cmplt_n_floats: case RPOp::cmplt_n_ints: case RPOp::cmplt_n_uints:
            case RPOp::cmple_n_floats: case RPOp::cmple_n_ints: case RPOp::cmple_n_uints:
            case RPOp::cmpeq_n_floats: case RPOp::cmpeq_n_ints:
            case RPOp::cmpne_n_floats: case RPOp::cmpne_n_ints:
                std::tie(opArg1, opArg2) = AdjacentBinaryOpCtx(stage.ctx);
                break;

            case RPOp::mix_n_floats:
                std::tie(opArg1, opArg2, opArg3) = AdjacentTernaryOpCtx(stage.ctx);
                break;

            case RPOp::jump:
            case RPOp::branch_if_any_active_lanes:
            case RPOp::branch_if_no_active_lanes:
                opArg1 = BranchOffset(stage.ctx);
                break;

            default:
                break;
        }

        const char* opName = "";
        switch (stage.op) {
        #define M(x) case RPOp::x: opName = #x; break;
            SK_RASTER_PIPELINE_OPS_ALL(M)
        #undef M
        }

        std::string opText;
        switch (stage.op) {
            case RPOp::init_lane_masks:
                opText = "CondMask = LoopMask = RetMask = true";
                break;

            case RPOp::load_condition_mask:
                opText = "CondMask = " + opArg1;
                break;

            case RPOp::store_condition_mask:
                opText = opArg1 + " = CondMask";
                break;

            case RPOp::merge_condition_mask:
                opText = "CondMask = " + opArg1 + " & " + opArg2;
                break;

            case RPOp::load_loop_mask:
                opText = "LoopMask = " + opArg1;
                break;

            case RPOp::store_loop_mask:
                opText = opArg1 + " = LoopMask";
                break;

            case RPOp::mask_off_loop_mask:
                opText = "LoopMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case RPOp::reenable_loop_mask:
                opText = "LoopMask |= " + opArg1;
                break;

            case RPOp::merge_loop_mask:
                opText = "LoopMask &= " + opArg1;
                break;

            case RPOp::load_return_mask:
                opText = "RetMask = " + opArg1;
                break;

            case RPOp::store_return_mask:
                opText = opArg1 + " = RetMask";
                break;

            case RPOp::mask_off_return_mask:
                opText = "RetMask &= ~(CondMask & LoopMask & RetMask)";
                break;

            case RPOp::immediate_f:
            case RPOp::load_unmasked:
                opText = "src.r = " + opArg1;
                break;

            case RPOp::store_unmasked:
                opText = opArg1 + " = src.r";
                break;

            case RPOp::store_src_rg:
                opText = opArg1 + " = src.rg";
                break;

            case RPOp::store_src:
                opText = opArg1 + " = src.rgba";
                break;

            case RPOp::store_dst:
                opText = opArg1 + " = dst.rgba";
                break;

            case RPOp::load_src:
                opText = "src.rgba = " + opArg1;
                break;

            case RPOp::load_dst:
                opText = "dst.rgba = " + opArg1;
                break;

            case RPOp::store_masked:
                opText = opArg1 + " = Mask(src.r)";
                break;

            case RPOp::bitwise_and_int:
            case RPOp::bitwise_and_2_ints:
            case RPOp::bitwise_and_3_ints:
            case RPOp::bitwise_and_4_ints:
            case RPOp::bitwise_and_n_ints:
                opText = opArg1 + " &= " + opArg2;
                break;

            case RPOp::bitwise_or_int:
            case RPOp::bitwise_or_2_ints:
            case RPOp::bitwise_or_3_ints:
            case RPOp::bitwise_or_4_ints:
            case RPOp::bitwise_or_n_ints:
                opText = opArg1 + " |= " + opArg2;
                break;

            case RPOp::bitwise_xor_int:
            case RPOp::bitwise_xor_2_ints:
            case RPOp::bitwise_xor_3_ints:
            case RPOp::bitwise_xor_4_ints:
            case RPOp::bitwise_xor_n_ints:
                opText = opArg1 + " ^= " + opArg2;
                break;

            case RPOp::bitwise_not_int:
            case RPOp::bitwise_not_2_ints:
            case RPOp::bitwise_not_3_ints:
            case RPOp::bitwise_not_4_ints:
                opText = opArg1 + " = ~" + opArg1;
                break;

            case RPOp::cast_to_float_from_int:
            case RPOp::cast_to_float_from_2_ints:
            case RPOp::cast_to_float_from_3_ints:
            case RPOp::cast_to_float_from_4_ints:
                opText = opArg1 + " = IntToFloat(" + opArg1 + ")";
                break;

            case RPOp::cast_to_float_from_uint:
            case RPOp::cast_to_float_from_2_uints:
            case RPOp::cast_to_float_from_3_uints:
            case RPOp::cast_to_float_from_4_uints:
                opText = opArg1 + " = UintToFloat(" + opArg1 + ")";
                break;

            case RPOp::cast_to_int_from_float:
            case RPOp::cast_to_int_from_2_floats:
            case RPOp::cast_to_int_from_3_floats:
            case RPOp::cast_to_int_from_4_floats:
                opText = opArg1 + " = FloatToInt(" + opArg1 + ")";
                break;

            case RPOp::cast_to_uint_from_float:
            case RPOp::cast_to_uint_from_2_floats:
            case RPOp::cast_to_uint_from_3_floats:
            case RPOp::cast_to_uint_from_4_floats:
                opText = opArg1 + " = FloatToUint(" + opArg1 + ")";
                break;

            case RPOp::copy_slot_masked:      case RPOp::copy_2_slots_masked:
            case RPOp::copy_3_slots_masked:   case RPOp::copy_4_slots_masked:
                opText = opArg1 + " = Mask(" + opArg2 + ")";
                break;

            case RPOp::copy_constant:         case RPOp::copy_2_constants:
            case RPOp::copy_3_constants:      case RPOp::copy_4_constants:
            case RPOp::copy_slot_unmasked:    case RPOp::copy_2_slots_unmasked:
            case RPOp::copy_3_slots_unmasked: case RPOp::copy_4_slots_unmasked:
            case RPOp::swizzle_1:             case RPOp::swizzle_2:
            case RPOp::swizzle_3:             case RPOp::swizzle_4:
            case RPOp::shuffle:
                opText = opArg1 + " = " + opArg2;
                break;

            case RPOp::zero_slot_unmasked:    case RPOp::zero_2_slots_unmasked:
            case RPOp::zero_3_slots_unmasked: case RPOp::zero_4_slots_unmasked:
                opText = opArg1 + " = 0";
                break;

            case RPOp::abs_float:    case RPOp::abs_int:
            case RPOp::abs_2_floats: case RPOp::abs_2_ints:
            case RPOp::abs_3_floats: case RPOp::abs_3_ints:
            case RPOp::abs_4_floats: case RPOp::abs_4_ints:
                opText = opArg1 + " = abs(" + opArg1 + ")";
                break;

            case RPOp::ceil_float:
            case RPOp::ceil_2_floats:
            case RPOp::ceil_3_floats:
            case RPOp::ceil_4_floats:
                opText = opArg1 + " = ceil(" + opArg1 + ")";
                break;

            case RPOp::floor_float:
            case RPOp::floor_2_floats:
            case RPOp::floor_3_floats:
            case RPOp::floor_4_floats:
                opText = opArg1 + " = floor(" + opArg1 + ")";
                break;

            case RPOp::add_float:    case RPOp::add_int:
            case RPOp::add_2_floats: case RPOp::add_2_ints:
            case RPOp::add_3_floats: case RPOp::add_3_ints:
            case RPOp::add_4_floats: case RPOp::add_4_ints:
            case RPOp::add_n_floats: case RPOp::add_n_ints:
                opText = opArg1 + " += " + opArg2;
                break;

            case RPOp::sub_float:    case RPOp::sub_int:
            case RPOp::sub_2_floats: case RPOp::sub_2_ints:
            case RPOp::sub_3_floats: case RPOp::sub_3_ints:
            case RPOp::sub_4_floats: case RPOp::sub_4_ints:
            case RPOp::sub_n_floats: case RPOp::sub_n_ints:
                opText = opArg1 + " -= " + opArg2;
                break;

            case RPOp::mul_float:    case RPOp::mul_int:
            case RPOp::mul_2_floats: case RPOp::mul_2_ints:
            case RPOp::mul_3_floats: case RPOp::mul_3_ints:
            case RPOp::mul_4_floats: case RPOp::mul_4_ints:
            case RPOp::mul_n_floats: case RPOp::mul_n_ints:
                opText = opArg1 + " *= " + opArg2;
                break;

            case RPOp::div_float:    case RPOp::div_int:    case RPOp::div_uint:
            case RPOp::div_2_floats: case RPOp::div_2_ints: case RPOp::div_2_uints:
            case RPOp::div_3_floats: case RPOp::div_3_ints: case RPOp::div_3_uints:
            case RPOp::div_4_floats: case RPOp::div_4_ints: case RPOp::div_4_uints:
            case RPOp::div_n_floats: case RPOp::div_n_ints: case RPOp::div_n_uints:
                opText = opArg1 + " /= " + opArg2;
                break;

            case RPOp::min_float:    case RPOp::min_int:    case RPOp::min_uint:
            case RPOp::min_2_floats: case RPOp::min_2_ints: case RPOp::min_2_uints:
            case RPOp::min_3_floats: case RPOp::min_3_ints: case RPOp::min_3_uints:
            case RPOp::min_4_floats: case RPOp::min_4_ints: case RPOp::min_4_uints:
            case RPOp::min_n_floats: case RPOp::min_n_ints: case RPOp::min_n_uints:
                opText = opArg1 + " = min(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::max_float:    case RPOp::max_int:    case RPOp::max_uint:
            case RPOp::max_2_floats: case RPOp::max_2_ints: case RPOp::max_2_uints:
            case RPOp::max_3_floats: case RPOp::max_3_ints: case RPOp::max_3_uints:
            case RPOp::max_4_floats: case RPOp::max_4_ints: case RPOp::max_4_uints:
            case RPOp::max_n_floats: case RPOp::max_n_ints: case RPOp::max_n_uints:
                opText = opArg1 + " = max(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::cmplt_float:    case RPOp::cmplt_int:    case RPOp::cmplt_uint:
            case RPOp::cmplt_2_floats: case RPOp::cmplt_2_ints: case RPOp::cmplt_2_uints:
            case RPOp::cmplt_3_floats: case RPOp::cmplt_3_ints: case RPOp::cmplt_3_uints:
            case RPOp::cmplt_4_floats: case RPOp::cmplt_4_ints: case RPOp::cmplt_4_uints:
            case RPOp::cmplt_n_floats: case RPOp::cmplt_n_ints: case RPOp::cmplt_n_uints:
                opText = opArg1 + " = lessThan(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::cmple_float:    case RPOp::cmple_int:    case RPOp::cmple_uint:
            case RPOp::cmple_2_floats: case RPOp::cmple_2_ints: case RPOp::cmple_2_uints:
            case RPOp::cmple_3_floats: case RPOp::cmple_3_ints: case RPOp::cmple_3_uints:
            case RPOp::cmple_4_floats: case RPOp::cmple_4_ints: case RPOp::cmple_4_uints:
            case RPOp::cmple_n_floats: case RPOp::cmple_n_ints: case RPOp::cmple_n_uints:
                opText = opArg1 + " = lessThanEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::cmpeq_float:    case RPOp::cmpeq_int:
            case RPOp::cmpeq_2_floats: case RPOp::cmpeq_2_ints:
            case RPOp::cmpeq_3_floats: case RPOp::cmpeq_3_ints:
            case RPOp::cmpeq_4_floats: case RPOp::cmpeq_4_ints:
            case RPOp::cmpeq_n_floats: case RPOp::cmpeq_n_ints:
                opText = opArg1 + " = equal(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::cmpne_float:    case RPOp::cmpne_int:
            case RPOp::cmpne_2_floats: case RPOp::cmpne_2_ints:
            case RPOp::cmpne_3_floats: case RPOp::cmpne_3_ints:
            case RPOp::cmpne_4_floats: case RPOp::cmpne_4_ints:
            case RPOp::cmpne_n_floats: case RPOp::cmpne_n_ints:
                opText = opArg1 + " = notEqual(" + opArg1 + ", " + opArg2 + ")";
                break;

            case RPOp::mix_float:
            case RPOp::mix_2_floats:
            case RPOp::mix_3_floats:
            case RPOp::mix_4_floats:
            case RPOp::mix_n_floats:
                opText = opArg1 + " = mix(" + opArg1 + ", " + opArg2 + ", " + opArg3 + ")";
                break;

            case RPOp::jump:
            case RPOp::branch_if_any_active_lanes:
            case RPOp::branch_if_no_active_lanes:
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
}

}  // namespace RP
}  // namespace SkSL
