/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RASTERPIPELINEBUILDER
#define SKSL_RASTERPIPELINEBUILDER

#include "include/core/SkTypes.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkUtils.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkArenaAlloc;
class SkRasterPipeline;
class SkWStream;
using SkRPOffset = uint32_t;

namespace SkSL {

class DebugTracePriv;
class TraceHook;

namespace RP {

// A single scalar in our program consumes one slot.
using Slot = int;
constexpr Slot NA = -1;

// Scalars, vectors, and matrices can be represented as a range of slot indices.
struct SlotRange {
    Slot index = 0;
    int count = 0;
};

#define SKRP_EXTENDED_OPS(M)     \
    /* branch targets */         \
    M(label)                     \
                                 \
    /* child programs */         \
    M(invoke_shader)             \
    M(invoke_color_filter)       \
    M(invoke_blender)            \
                                 \
    /* color space transforms */ \
    M(invoke_to_linear_srgb)     \
    M(invoke_from_linear_srgb)

// An RP::Program will consist entirely of ProgramOps. The ProgramOps list is a superset of the
// native SkRasterPipelineOps op-list. It also has a few extra ops to indicate child-effect
// invocation, and a `label` op to indicate branch targets.
enum class ProgramOp {
    #define M(stage) stage,
        // A finished program can contain any native Raster Pipeline op...
        SK_RASTER_PIPELINE_OPS_ALL(M)

        // ... as well as our extended ops.
        SKRP_EXTENDED_OPS(M)
    #undef M
};

// BuilderOps are a superset of ProgramOps. They are used by the RP::Builder, which works in terms
// of Instructions; Instructions are slightly more expressive than raw SkRasterPipelineOps. In
// particular, the Builder supports stacks for pushing and popping scratch values.
// RP::Program::makeStages is responsible for rewriting Instructions/BuilderOps into an array of
// RP::Program::Stages, which will contain only native SkRasterPipelineOps and (optionally)
// child-effect invocations.
enum class BuilderOp {
    #define M(stage) stage,
        // An in-flight program can contain all the native Raster Pipeline ops...
        SK_RASTER_PIPELINE_OPS_ALL(M)

        // ... and our extended ops...
        SKRP_EXTENDED_OPS(M)
    #undef M

    // ... and also has Builder-specific ops. These ops generally interface with the stack, and are
    // converted into ProgramOps during `makeStages`.
    push_clone,
    push_clone_from_stack,
    push_clone_indirect_from_stack,
    push_constant,
    push_immutable,
    push_immutable_indirect,
    push_slots,
    push_slots_indirect,
    push_uniform,
    push_uniform_indirect,
    copy_stack_to_slots,
    copy_stack_to_slots_unmasked,
    copy_stack_to_slots_indirect,
    copy_uniform_to_slots_unmasked,
    store_immutable_value,
    swizzle_copy_stack_to_slots,
    swizzle_copy_stack_to_slots_indirect,
    discard_stack,
    pad_stack,
    select,
    push_condition_mask,
    pop_condition_mask,
    push_loop_mask,
    pop_loop_mask,
    pop_and_reenable_loop_mask,
    push_return_mask,
    pop_return_mask,
    push_src_rgba,
    push_dst_rgba,
    push_device_xy01,
    pop_src_rgba,
    pop_dst_rgba,
    trace_var_indirect,
    branch_if_no_active_lanes_on_stack_top_equal,
    unsupported
};

// If the extended ops are not in sync between enums, program creation will not work.
static_assert((int)ProgramOp::label == (int)BuilderOp::label);

// Represents a single raster-pipeline SkSL instruction.
struct Instruction {
    BuilderOp fOp;
    Slot      fSlotA = NA;
    Slot      fSlotB = NA;
    int       fImmA = 0;
    int       fImmB = 0;
    int       fImmC = 0;
    int       fImmD = 0;
    int       fStackID = 0;
};

class Callbacks {
public:
    virtual ~Callbacks() = default;

    virtual bool appendShader(int index) = 0;
    virtual bool appendColorFilter(int index) = 0;
    virtual bool appendBlender(int index) = 0;

    virtual void toLinearSrgb(const void* color) = 0;
    virtual void fromLinearSrgb(const void* color) = 0;
};

class Program {
public:
    Program(skia_private::TArray<Instruction> instrs,
            int numValueSlots,
            int numUniformSlots,
            int numImmutableSlots,
            int numLabels,
            DebugTracePriv* debugTrace);
    ~Program();

    bool appendStages(SkRasterPipeline* pipeline,
                      SkArenaAlloc* alloc,
                      Callbacks* callbacks,
                      SkSpan<const float> uniforms) const;

    void dump(SkWStream* out, bool writeInstructionCount = false) const;

    int numUniforms() const { return fNumUniformSlots; }

private:
    using StackDepths = skia_private::TArray<int>; // [stack index] = depth of stack

    struct SlotData {
        SkSpan<float> values;
        SkSpan<float> stack;
        SkSpan<float> immutable;
    };
    SlotData allocateSlotData(SkArenaAlloc* alloc) const;

    struct Stage {
        ProgramOp op;
        void*     ctx;
    };
    void makeStages(skia_private::TArray<Stage>* pipeline,
                    SkArenaAlloc* alloc,
                    SkSpan<const float> uniforms,
                    const SlotData& slots) const;
    void optimize();
    StackDepths tempStackMaxDepths() const;

    // These methods are used to split up multi-slot copies into multiple ops as needed.
    void appendCopy(skia_private::TArray<Stage>* pipeline,
                    SkArenaAlloc* alloc,
                    std::byte* basePtr,
                    ProgramOp baseStage,
                    SkRPOffset dst, int dstStride,
                    SkRPOffset src, int srcStride,
                    int numSlots) const;
    void appendCopyImmutableUnmasked(skia_private::TArray<Stage>* pipeline,
                                     SkArenaAlloc* alloc,
                                     std::byte* basePtr,
                                     SkRPOffset dst,
                                     SkRPOffset src,
                                     int numSlots) const;
    void appendCopySlotsUnmasked(skia_private::TArray<Stage>* pipeline,
                                 SkArenaAlloc* alloc,
                                 SkRPOffset dst,
                                 SkRPOffset src,
                                 int numSlots) const;
    void appendCopySlotsMasked(skia_private::TArray<Stage>* pipeline,
                               SkArenaAlloc* alloc,
                               SkRPOffset dst,
                               SkRPOffset src,
                               int numSlots) const;

    // Appends a single-slot single-input math operation to the pipeline. The op `stage` will
    // appended `numSlots` times, starting at position `dst` and advancing one slot for each
    // subsequent invocation.
    void appendSingleSlotUnaryOp(skia_private::TArray<Stage>* pipeline, ProgramOp stage,
                                 float* dst, int numSlots) const;

    // Appends a multi-slot single-input math operation to the pipeline. `baseStage` must refer to
    // a single-slot "apply_op" stage, which must be immediately followed by specializations for
    // 2-4 slots. For instance, {`ceil_float`, `ceil_2_floats`, `ceil_3_floats`, `ceil_4_floats`}
    // must be contiguous ops in the stage list, listed in that order; pass `ceil_float` and we
    // pick the appropriate op based on `numSlots`.
    void appendMultiSlotUnaryOp(skia_private::TArray<Stage>* pipeline, ProgramOp baseStage,
                                float* dst, int numSlots) const;

    // Appends an immediate-mode binary operation to the pipeline. `baseStage` must refer to
    // a single-slot, immediate-mode "apply-imm" stage, which must be immediately preceded by
    // specializations for 2-4 slots if numSlots is greater than 1. For instance, {`add_imm_4_ints`,
    // `add_imm_3_ints`, `add_imm_2_ints`, `add_imm_int`} must be contiguous ops in the stage list,
    // listed in that order; pass `add_imm_int` and we pick the appropriate op based on `numSlots`.
    // Some immediate-mode binary ops are single-slot only in the interest of code size; in this
    // case, the multi-slot ops can be absent, but numSlots must be 1.
    void appendImmediateBinaryOp(skia_private::TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                 ProgramOp baseStage,
                                 SkRPOffset dst, int32_t value, int numSlots) const;

    // Appends a two-input math operation to the pipeline. `src` must be _immediately_ after `dst`
    // in memory. `baseStage` must refer to an unbounded "apply_to_n_slots" stage. A BinaryOpCtx
    // will be used to pass pointers to the destination and source; the delta between the two
    // pointers implicitly gives the number of slots.
    void appendAdjacentNWayBinaryOp(skia_private::TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                    ProgramOp stage,
                                    SkRPOffset dst, SkRPOffset src, int numSlots) const;

    // Appends a multi-slot two-input math operation to the pipeline. `src` must be _immediately_
    // after `dst` in memory. `baseStage` must refer to an unbounded "apply_to_n_slots" stage, which
    // must be immediately followed by specializations for 1-4 slots. For instance, {`add_n_floats`,
    // `add_float`, `add_2_floats`, `add_3_floats`, `add_4_floats`} must be contiguous ops in the
    // stage list, listed in that order; pass `add_n_floats` and we pick the appropriate op based on
    // `numSlots`.
    void appendAdjacentMultiSlotBinaryOp(skia_private::TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                         ProgramOp baseStage, std::byte* basePtr,
                                         SkRPOffset dst, SkRPOffset src, int numSlots) const;

    // Appends a multi-slot math operation having three inputs (dst, src0, src1) and one output
    // (dst) to the pipeline. The three inputs must be _immediately_ adjacent in memory. `baseStage`
    // must refer to an unbounded "apply_to_n_slots" stage, which must be immediately followed by
    // specializations for 1-4 slots.
    void appendAdjacentMultiSlotTernaryOp(skia_private::TArray<Stage>* pipeline,
                                          SkArenaAlloc* alloc, ProgramOp baseStage,
                                          std::byte* basePtr, SkRPOffset dst, SkRPOffset src0,
                                          SkRPOffset src1, int numSlots) const;

    // Appends a math operation having three inputs (dst, src0, src1) and one output (dst) to the
    // pipeline. The three inputs must be _immediately_ adjacent in memory. `baseStage` must refer
    // to an unbounded "apply_to_n_slots" stage. A TernaryOpCtx will be used to pass pointers to the
    // destination and sources; the delta between the each pointer implicitly gives the slot count.
    void appendAdjacentNWayTernaryOp(skia_private::TArray<Stage>* pipeline, SkArenaAlloc* alloc,
                                     ProgramOp stage, std::byte* basePtr, SkRPOffset dst,
                                     SkRPOffset src0, SkRPOffset src1, int numSlots) const;

    // Appends a stack_rewind op on platforms where it is needed (when SK_HAS_MUSTTAIL is not set).
    void appendStackRewind(skia_private::TArray<Stage>* pipeline) const;

    class Dumper;
    friend class Dumper;

    skia_private::TArray<Instruction> fInstructions;
    int fNumValueSlots = 0;
    int fNumUniformSlots = 0;
    int fNumImmutableSlots = 0;
    int fNumTempStackSlots = 0;
    int fNumLabels = 0;
    StackDepths fTempStackMaxDepths;
    DebugTracePriv* fDebugTrace = nullptr;
    std::unique_ptr<SkSL::TraceHook> fTraceHook;
};

class Builder {
public:
    /** Finalizes and optimizes the program. */
    std::unique_ptr<Program> finish(int numValueSlots,
                                    int numUniformSlots,
                                    int numImmutableSlots,
                                    DebugTracePriv* debugTrace = nullptr);
    /**
     * Peels off a label ID for use in the program. Set the label's position in the program with
     * the `label` instruction. Actually branch to the target with an instruction like
     * `branch_if_any_lanes_active` or `jump`.
     */
    int nextLabelID() {
        return fNumLabels++;
    }

    /**
     * The builder keeps track of the state of execution masks; when we know that the execution
     * mask is unaltered, we can generate simpler code. Code which alters the execution mask is
     * required to enable this flag.
     */
    void enableExecutionMaskWrites() {
        ++fExecutionMaskWritesEnabled;
    }

    void disableExecutionMaskWrites() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        --fExecutionMaskWritesEnabled;
    }

    bool executionMaskWritesAreEnabled() {
        return fExecutionMaskWritesEnabled > 0;
    }

    /** Assemble a program from the Raster Pipeline instructions below. */
    void init_lane_masks() {
        this->appendInstruction(BuilderOp::init_lane_masks, {});
    }

    void store_src_rg(SlotRange slots) {
        SkASSERT(slots.count == 2);
        this->appendInstruction(BuilderOp::store_src_rg, {slots.index});
    }

    void store_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        this->appendInstruction(BuilderOp::store_src, {slots.index});
    }

    void store_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        this->appendInstruction(BuilderOp::store_dst, {slots.index});
    }

    void store_device_xy01(SlotRange slots) {
        SkASSERT(slots.count == 4);
        this->appendInstruction(BuilderOp::store_device_xy01, {slots.index});
    }

    void load_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        this->appendInstruction(BuilderOp::load_src, {slots.index});
    }

    void load_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        this->appendInstruction(BuilderOp::load_dst, {slots.index});
    }

    void set_current_stack(int stackID) {
        fCurrentStackID = stackID;
    }

    // Inserts a label into the instruction stream.
    void label(int labelID);

    // Unconditionally branches to a label.
    void jump(int labelID);

    // Branches to a label if the execution mask is active in every lane.
    void branch_if_all_lanes_active(int labelID);

    // Branches to a label if the execution mask is active in any lane.
    void branch_if_any_lanes_active(int labelID);

    // Branches to a label if the execution mask is inactive across all lanes.
    void branch_if_no_lanes_active(int labelID);

    // Branches to a label if the top value on the stack is _not_ equal to `value` in any lane.
    void branch_if_no_active_lanes_on_stack_top_equal(int value, int labelID);

    // We use the same SkRasterPipeline op regardless of the literal type, and bitcast the value.
    void push_constant_i(int32_t val, int count = 1);

    void push_zeros(int count) {
        this->push_constant_i(/*val=*/0, count);
    }

    void push_constant_f(float val) {
        this->push_constant_i(sk_bit_cast<int32_t>(val), /*count=*/1);
    }

    void push_constant_u(uint32_t val, int count = 1) {
        this->push_constant_i(sk_bit_cast<int32_t>(val), count);
    }

    // Translates into copy_uniforms (from uniforms into temp stack) in Raster Pipeline.
    void push_uniform(SlotRange src);

    // Initializes the Raster Pipeline slot with a constant value when the program is first created.
    // Does not add any instructions to the program.
    void store_immutable_value_i(Slot slot, int32_t val) {
        this->appendInstruction(BuilderOp::store_immutable_value, {slot}, val);
    }

    // Translates into copy_uniforms (from uniforms into value-slots) in Raster Pipeline.
    void copy_uniform_to_slots_unmasked(SlotRange dst, SlotRange src);

    // Translates into copy_from_indirect_uniform_unmasked (from values into temp stack) in Raster
    // Pipeline. `fixedRange` denotes a fixed set of slots; this range is pushed forward by the
    // value at the top of stack `dynamicStack`. Pass the range of the uniform being indexed as
    // `limitRange`; this is used as a hard cap, to avoid indexing outside of bounds.
    void push_uniform_indirect(SlotRange fixedRange, int dynamicStack, SlotRange limitRange);


    // Translates into copy_slots_unmasked (from values into temp stack) in Raster Pipeline.
    void push_slots(SlotRange src) {
        this->push_slots_or_immutable(src, BuilderOp::push_slots);
    }

    // Translates into copy_immutable_unmasked (from immutables into temp stack) in Raster Pipeline.
    void push_immutable(SlotRange src) {
        this->push_slots_or_immutable(src, BuilderOp::push_immutable);
    }

    void push_slots_or_immutable(SlotRange src, BuilderOp op);

    // Translates into copy_from_indirect_unmasked (from values into temp stack) in Raster Pipeline.
    // `fixedRange` denotes a fixed set of slots; this range is pushed forward by the value at the
    // top of stack `dynamicStack`. Pass the slot range of the variable being indexed as
    // `limitRange`; this is used as a hard cap, to avoid indexing outside of bounds.
    void push_slots_indirect(SlotRange fixedRange, int dynamicStack, SlotRange limitRange) {
        this->push_slots_or_immutable_indirect(fixedRange, dynamicStack, limitRange,
                                               BuilderOp::push_slots_indirect);
    }

    void push_immutable_indirect(SlotRange fixedRange, int dynamicStack, SlotRange limitRange) {
        this->push_slots_or_immutable_indirect(fixedRange, dynamicStack, limitRange,
                                               BuilderOp::push_immutable_indirect);
    }

    void push_slots_or_immutable_indirect(SlotRange fixedRange, int dynamicStack,
                                          SlotRange limitRange, BuilderOp op);

    // Translates into copy_slots_masked (from temp stack to values) in Raster Pipeline.
    // Does not discard any values on the temp stack.
    void copy_stack_to_slots(SlotRange dst) {
        this->copy_stack_to_slots(dst, /*offsetFromStackTop=*/dst.count);
    }

    void copy_stack_to_slots(SlotRange dst, int offsetFromStackTop);

    // Translates into swizzle_copy_slots_masked (from temp stack to values) in Raster Pipeline.
    // Does not discard any values on the temp stack.
    void swizzle_copy_stack_to_slots(SlotRange dst,
                                     SkSpan<const int8_t> components,
                                     int offsetFromStackTop);

    // Translates into swizzle_copy_to_indirect_masked (from temp stack to values) in Raster
    // Pipeline. Does not discard any values on the temp stack.
    void swizzle_copy_stack_to_slots_indirect(SlotRange fixedRange,
                                              int dynamicStackID,
                                              SlotRange limitRange,
                                              SkSpan<const int8_t> components,
                                              int offsetFromStackTop);

    // Translates into copy_slots_unmasked (from temp stack to values) in Raster Pipeline.
    // Does not discard any values on the temp stack.
    void copy_stack_to_slots_unmasked(SlotRange dst) {
        this->copy_stack_to_slots_unmasked(dst, /*offsetFromStackTop=*/dst.count);
    }

    void copy_stack_to_slots_unmasked(SlotRange dst, int offsetFromStackTop);

    // Translates into copy_to_indirect_masked (from temp stack into values) in Raster Pipeline.
    // `fixedRange` denotes a fixed set of slots; this range is pushed forward by the value at the
    // top of stack `dynamicStack`. Pass the slot range of the variable being indexed as
    // `limitRange`; this is used as a hard cap, to avoid indexing outside of bounds.
    void copy_stack_to_slots_indirect(SlotRange fixedRange,
                                      int dynamicStackID,
                                      SlotRange limitRange);

    // Copies from temp stack to slots, including an indirect offset, then shrinks the temp stack.
    void pop_slots_indirect(SlotRange fixedRange, int dynamicStackID, SlotRange limitRange) {
        this->copy_stack_to_slots_indirect(fixedRange, dynamicStackID, limitRange);
        this->discard_stack(fixedRange.count);
    }

    // Performs a unary op (like `bitwise_not`), given a slot count of `slots`. The stack top is
    // replaced with the result.
    void unary_op(BuilderOp op, int32_t slots);

    // Performs a binary op (like `add_n_floats` or `cmpeq_n_ints`), given a slot count of
    // `slots`. Two n-slot input values are consumed, and the result is pushed onto the stack.
    void binary_op(BuilderOp op, int32_t slots);

    // Performs a ternary op (like `mix` or `smoothstep`), given a slot count of
    // `slots`. Three n-slot input values are consumed, and the result is pushed onto the stack.
    void ternary_op(BuilderOp op, int32_t slots);

    // Computes a dot product on the stack. The slots consumed (`slots`) must be between 1 and 4.
    // Two n-slot input vectors are consumed, and a scalar result is pushed onto the stack.
    void dot_floats(int32_t slots);

    // Computes refract(N, I, eta) on the stack. N and I are assumed to be 4-slot vectors, and can
    // be padded with zeros for smaller inputs. Eta is a scalar. The result is a 4-slot vector.
    void refract_floats();

    // Computes inverse(matN) on the stack. Pass 2, 3 or 4 for n to specify matrix size.
    void inverse_matrix(int32_t n);

    // Shrinks the temp stack, discarding values on top.
    void discard_stack(int32_t count, int stackID);

    void discard_stack(int32_t count) {
        this->discard_stack(count, fCurrentStackID);
    }

    // Grows the temp stack, leaving any preexisting values in place.
    void pad_stack(int32_t count);

    // Copies vales from the temp stack into slots, and then shrinks the temp stack.
    void pop_slots(SlotRange dst);

    // Creates many clones of the top single-slot item on the temp stack.
    void push_duplicates(int count);

    // Creates a single clone of an item on the current temp stack. The cloned item can consist of
    // any number of slots, and can be copied from an earlier position on the stack.
    void push_clone(int numSlots, int offsetFromStackTop = 0);

    // Clones a range of slots from another stack onto this stack.
    void push_clone_from_stack(SlotRange range, int otherStackID, int offsetFromStackTop);

    // Translates into copy_from_indirect_unmasked (from one temp stack to another) in Raster
    // Pipeline. `fixedOffset` denotes a range of slots within the top `offsetFromStackTop` slots of
    // `otherStackID`. This range is pushed forward by the value at the top of `dynamicStackID`.
    void push_clone_indirect_from_stack(SlotRange fixedOffset,
                                        int dynamicStackID,
                                        int otherStackID,
                                        int offsetFromStackTop);

    // Compares the stack top with the passed-in value; if it matches, enables the loop mask.
    void case_op(int value) {
        this->appendInstruction(BuilderOp::case_op, {}, value);
    }

    // Performs a `continue` in a loop.
    void continue_op(int continueMaskStackID) {
        this->appendInstruction(BuilderOp::continue_op, {}, continueMaskStackID);
    }

    void select(int slots) {
        // Overlays the top two entries on the stack, making one hybrid entry. The execution mask
        // is used to select which lanes are preserved.
        SkASSERT(slots > 0);
        this->appendInstruction(BuilderOp::select, {}, slots);
    }

    // The opposite of push_slots; copies values from the temp stack into value slots, then
    // shrinks the temp stack.
    void pop_slots_unmasked(SlotRange dst);

    void copy_slots_masked(SlotRange dst, SlotRange src) {
        SkASSERT(dst.count == src.count);
        this->appendInstruction(BuilderOp::copy_slot_masked, {dst.index, src.index}, dst.count);
    }

    void copy_slots_unmasked(SlotRange dst, SlotRange src);

    void copy_immutable_unmasked(SlotRange dst, SlotRange src);

    // Directly writes a constant value into a slot.
    void copy_constant(Slot slot, int constantValue);

    // Stores zeros across the entire slot range.
    void zero_slots_unmasked(SlotRange dst);

    // Consumes `consumedSlots` elements on the stack, then generates `components.size()` elements.
    void swizzle(int consumedSlots, SkSpan<const int8_t> components);

    // Transposes a matrix of size CxR on the stack (into a matrix of size RxC).
    void transpose(int columns, int rows);

    // Generates a CxR diagonal matrix from the top two scalars on the stack. The second scalar is
    // used as the diagonal value; the first scalar (usually zero) fills in the rest of the slots.
    void diagonal_matrix(int columns, int rows);

    // Resizes a CxR matrix at the top of the stack to C'xR'.
    void matrix_resize(int origColumns, int origRows, int newColumns, int newRows);

    // Multiplies a CxR matrix/vector against an adjacent CxR matrix/vector on the stack.
    void matrix_multiply(int leftColumns, int leftRows, int rightColumns, int rightRows);

    void push_condition_mask();

    void pop_condition_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::pop_condition_mask, {});
    }

    void merge_condition_mask();

    void merge_inv_condition_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::merge_inv_condition_mask, {});
    }

    void push_loop_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::push_loop_mask, {});
    }

    void pop_loop_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::pop_loop_mask, {});
    }

    // Exchanges src.rgba with the four values at the top of the stack.
    void exchange_src();

    void push_src_rgba() {
        this->appendInstruction(BuilderOp::push_src_rgba, {});
    }

    void push_dst_rgba() {
        this->appendInstruction(BuilderOp::push_dst_rgba, {});
    }

    void push_device_xy01() {
        this->appendInstruction(BuilderOp::push_device_xy01, {});
    }

    void pop_src_rgba();

    void pop_dst_rgba() {
        this->appendInstruction(BuilderOp::pop_dst_rgba, {});
    }

    void mask_off_loop_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::mask_off_loop_mask, {});
    }

    void reenable_loop_mask(SlotRange src) {
        SkASSERT(this->executionMaskWritesAreEnabled());
        SkASSERT(src.count == 1);
        this->appendInstruction(BuilderOp::reenable_loop_mask, {src.index});
    }

    void pop_and_reenable_loop_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::pop_and_reenable_loop_mask, {});
    }

    void merge_loop_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::merge_loop_mask, {});
    }

    void push_return_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::push_return_mask, {});
    }

    void pop_return_mask();

    void mask_off_return_mask() {
        SkASSERT(this->executionMaskWritesAreEnabled());
        this->appendInstruction(BuilderOp::mask_off_return_mask, {});
    }

    void invoke_shader(int childIdx) {
        this->appendInstruction(BuilderOp::invoke_shader, {}, childIdx);
    }

    void invoke_color_filter(int childIdx) {
        this->appendInstruction(BuilderOp::invoke_color_filter, {}, childIdx);
    }

    void invoke_blender(int childIdx) {
        this->appendInstruction(BuilderOp::invoke_blender, {}, childIdx);
    }

    void invoke_to_linear_srgb() {
        // The intrinsics accept a three-component value; add a fourth padding element (which
        // will be ignored) since our RP ops deal in RGBA colors.
        this->pad_stack(1);
        this->appendInstruction(BuilderOp::invoke_to_linear_srgb, {});
        this->discard_stack(1);
    }

    void invoke_from_linear_srgb() {
        // The intrinsics accept a three-component value; add a fourth padding element (which
        // will be ignored) since our RP ops deal in RGBA colors.
        this->pad_stack(1);
        this->appendInstruction(BuilderOp::invoke_from_linear_srgb, {});
        this->discard_stack(1);
    }

    // Writes the current line number to the debug trace.
    void trace_line(int traceMaskStackID, int line) {
        this->appendInstruction(BuilderOp::trace_line, {}, traceMaskStackID, line);
    }

    // Writes a variable update to the debug trace.
    void trace_var(int traceMaskStackID, SlotRange r) {
        this->appendInstruction(BuilderOp::trace_var, {r.index}, traceMaskStackID, r.count);
    }

    // Writes a variable update (via indirection) to the debug trace.
    void trace_var_indirect(int traceMaskStackID, SlotRange fixedRange,
                            int dynamicStackID, SlotRange limitRange);

    // Writes a function-entrance to the debug trace.
    void trace_enter(int traceMaskStackID, int funcID) {
        this->appendInstruction(BuilderOp::trace_enter, {}, traceMaskStackID, funcID);
    }

    // Writes a function-exit to the debug trace.
    void trace_exit(int traceMaskStackID, int funcID) {
        this->appendInstruction(BuilderOp::trace_exit, {}, traceMaskStackID, funcID);
    }

    // Writes a scope-level change to the debug trace.
    void trace_scope(int traceMaskStackID, int delta) {
        this->appendInstruction(BuilderOp::trace_scope, {}, traceMaskStackID, delta);
    }

private:
    struct SlotList {
        SlotList(Slot a = NA, Slot b = NA) : fSlotA(a), fSlotB(b) {}
        Slot fSlotA = NA;
        Slot fSlotB = NA;
    };
    void appendInstruction(BuilderOp op, SlotList slots,
                           int a = 0, int b = 0, int c = 0, int d = 0);
    Instruction* lastInstruction(int fromBack = 0);
    Instruction* lastInstructionOnAnyStack(int fromBack = 0);
    void simplifyPopSlotsUnmasked(SlotRange* dst);
    bool simplifyImmediateUnmaskedOp();

    skia_private::TArray<Instruction> fInstructions;
    int fNumLabels = 0;
    int fExecutionMaskWritesEnabled = 0;
    int fCurrentStackID = 0;
};

}  // namespace RP
}  // namespace SkSL

#endif  // SKSL_RASTERPIPELINEBUILDER
