/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkStringView.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "tests/Test.h"

static sk_sp<SkData> get_program_dump(SkSL::RP::Program& program) {
    SkDynamicMemoryWStream stream;
    program.dump(&stream);
    return stream.detachAsData();
}

static std::string_view as_string_view(const sk_sp<SkData>& dump) {
    return std::string_view(static_cast<const char*>(dump->data()), dump->size());
}

static void check(skiatest::Reporter* r, SkSL::RP::Program& program, std::string_view expected) {
    // Verify that the program matches expectations.
    sk_sp<SkData> dump = get_program_dump(program);
    REPORTER_ASSERT(r, as_string_view(dump) == expected,
                    "Output did not match expectation:\n%.*s",
                    (int)dump->size(), static_cast<const char*>(dump->data()));
}

static SkSL::RP::SlotRange one_slot_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 1};
}

static SkSL::RP::SlotRange two_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 2};
}

static SkSL::RP::SlotRange three_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 3};
}

static SkSL::RP::SlotRange four_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 4};
}

static SkSL::RP::SlotRange five_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 5};
}

static SkSL::RP::SlotRange ten_slots_at(SkSL::RP::Slot index) {
    return SkSL::RP::SlotRange{index, 10};
}

DEF_TEST(RasterPipelineBuilder, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.store_src_rg(two_slots_at(0));
    builder.store_src(four_slots_at(2));
    builder.store_dst(four_slots_at(4));
    builder.store_device_xy01(four_slots_at(6));
    builder.init_lane_masks();
    builder.enableExecutionMaskWrites();
    builder.mask_off_return_mask();
    builder.mask_off_loop_mask();
    builder.reenable_loop_mask(one_slot_at(4));
    builder.disableExecutionMaskWrites();
    builder.load_src(four_slots_at(1));
    builder.load_dst(four_slots_at(3));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/10,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(store_src_rg                   v0..1 = src.rg
store_src                      v2..5 = src.rgba
store_dst                      v4..7 = dst.rgba
store_device_xy01              v6..9 = DeviceCoords.xy01
init_lane_masks                CondMask = LoopMask = RetMask = true
mask_off_return_mask           RetMask &= ~(CondMask & LoopMask & RetMask)
mask_off_loop_mask             LoopMask &= ~(CondMask & LoopMask & RetMask)
reenable_loop_mask             LoopMask |= v4
load_src                       src.rgba = v1..4
load_dst                       dst.rgba = v3..6
)");
}

DEF_TEST(RasterPipelineBuilderPushPopMaskRegisters, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;

    REPORTER_ASSERT(r, !builder.executionMaskWritesAreEnabled());
    builder.enableExecutionMaskWrites();
    REPORTER_ASSERT(r, builder.executionMaskWritesAreEnabled());

    builder.push_condition_mask();         // push into 0
    builder.push_loop_mask();              // push into 1
    builder.push_return_mask();            // push into 2
    builder.merge_condition_mask();        // set the condition-mask to 1 & 2
    builder.merge_inv_condition_mask();    // set the condition-mask to 1 & ~2
    builder.pop_condition_mask();          // pop from 2
    builder.merge_loop_mask();             // mask off the loop-mask against 1
    builder.push_condition_mask();         // push into 2
    builder.pop_condition_mask();          // pop from 2
    builder.pop_loop_mask();               // pop from 1
    builder.pop_return_mask();             // pop from 0
    builder.push_condition_mask();         // push into 0
    builder.pop_and_reenable_loop_mask();  // pop from 0

    REPORTER_ASSERT(r, builder.executionMaskWritesAreEnabled());
    builder.disableExecutionMaskWrites();
    REPORTER_ASSERT(r, !builder.executionMaskWritesAreEnabled());

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(store_condition_mask           $0 = CondMask
store_loop_mask                $1 = LoopMask
store_return_mask              $2 = RetMask
merge_condition_mask           CondMask = $1 & $2
merge_inv_condition_mask       CondMask = $1 & ~$2
load_condition_mask            CondMask = $2
merge_loop_mask                LoopMask &= $1
store_condition_mask           $2 = CondMask
load_condition_mask            CondMask = $2
load_loop_mask                 LoopMask = $1
load_return_mask               RetMask = $0
store_condition_mask           $0 = CondMask
reenable_loop_mask             LoopMask |= $0
)");
}


DEF_TEST(RasterPipelineBuilderCaseOp, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;

    builder.push_constant_i(123);  // push a test value
    builder.push_constant_i(~0);   // push an all-on default mask
    builder.case_op(123);          // do `case 123:`
    builder.case_op(124);          // do `case 124:`
    builder.discard_stack(2);

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_constant                  $0 = 0x0000007B (1.723597e-43)
copy_constant                  $1 = 0xFFFFFFFF
case_op                        if ($0 == 0x0000007B) { LoopMask = true; $1 = false; }
case_op                        if ($0 == 0x0000007C) { LoopMask = true; $1 = false; }
)");
}

DEF_TEST(RasterPipelineBuilderPushPopSrcDst, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;

    builder.push_src_rgba();
    builder.push_dst_rgba();
    builder.pop_src_rgba();
    builder.exchange_src();
    builder.exchange_src();
    builder.exchange_src();
    builder.pop_dst_rgba();

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(store_src                      $0..3 = src.rgba
store_dst                      $4..7 = dst.rgba
load_src                       src.rgba = $4..7
exchange_src                   swap(src.rgba, $0..3)
load_dst                       dst.rgba = $0..3
)");
}

DEF_TEST(RasterPipelineBuilderInvokeChild, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;

    builder.invoke_shader(1);
    builder.invoke_color_filter(2);
    builder.invoke_blender(3);

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
check(r, *program,
R"(invoke_shader                  invoke_shader 0x00000001
invoke_color_filter            invoke_color_filter 0x00000002
invoke_blender                 invoke_blender 0x00000003
)");
}

DEF_TEST(RasterPipelineBuilderPushPopTempImmediates, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.set_current_stack(1);
    builder.push_constant_i(999);                                         // push into 2
    builder.set_current_stack(0);
    builder.push_constant_f(13.5f);                                       // push into 0
    builder.push_clone_from_stack(one_slot_at(0), /*otherStackID=*/1, /*offsetFromStackTop=*/1);
                                                                          // push into 1 from 2
    builder.discard_stack(1);                                             // discard 2
    builder.push_constant_u(357);                                         // push into 2
    builder.set_current_stack(1);
    builder.push_clone_from_stack(one_slot_at(0), /*otherStackID=*/0, /*offsetFromStackTop=*/1);
                                                                          // push into 3 from 0
    builder.discard_stack(2);                                             // discard 2 and 3
    builder.set_current_stack(0);
    builder.push_constant_f(1.2f);                                        // push into 2
    builder.pad_stack(3);                                                 // pad slots 3,4,5
    builder.push_constant_f(3.4f);                                        // push into 6
    builder.discard_stack(7);                                             // discard 0 through 6
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/1,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_constant                  $2 = 0x000003E7 (1.399897e-42)
copy_constant                  $0 = 0x41580000 (13.5)
copy_constant                  $1 = 0x00000165 (5.002636e-43)
)");
}

DEF_TEST(RasterPipelineBuilderPushPopIndirect, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.set_current_stack(1);
    builder.push_constant_i(3);
    builder.set_current_stack(0);
    builder.push_slots_indirect(two_slots_at(0), /*dynamicStack=*/1, ten_slots_at(0));
    builder.push_slots_indirect(four_slots_at(10), /*dynamicStack=*/1, ten_slots_at(10));
    builder.push_uniform_indirect(one_slot_at(0), /*dynamicStack=*/1, five_slots_at(0));
    builder.push_uniform_indirect(three_slots_at(5), /*dynamicStack=*/1, five_slots_at(5));
    builder.swizzle_copy_stack_to_slots_indirect(three_slots_at(6), /*dynamicStackID=*/1,
                                                 ten_slots_at(0), {2, 1, 0},
                                                 /*offsetFromStackTop=*/3);
    builder.copy_stack_to_slots_indirect(three_slots_at(4), /*dynamicStackID=*/1, ten_slots_at(0));
    builder.pop_slots_indirect(five_slots_at(0), /*dynamicStackID=*/1, ten_slots_at(0));
    builder.pop_slots_indirect(five_slots_at(10), /*dynamicStackID=*/1, ten_slots_at(10));
    builder.set_current_stack(1);
    builder.discard_stack(1);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/20,
                                                                /*numUniformSlots=*/10,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_constant                  $10 = 0x00000003 (4.203895e-45)
copy_from_indirect_unmasked    $0..1 = Indirect(v0..1 + $10)
copy_from_indirect_unmasked    $2..5 = Indirect(v10..13 + $10)
copy_from_indirect_uniform_unm $6 = Indirect(u0 + $10)
copy_from_indirect_uniform_unm $7..9 = Indirect(u5..7 + $10)
swizzle_copy_to_indirect_maske Indirect(v6..8 + $10).zyx = Mask($7..9)
copy_to_indirect_masked        Indirect(v4..6 + $10) = Mask($7..9)
copy_to_indirect_masked        Indirect(v0..4 + $10) = Mask($5..9)
copy_to_indirect_masked        Indirect(v10..14 + $10) = Mask($0..4)
)");
}

DEF_TEST(RasterPipelineBuilderCopySlotsMasked, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.copy_slots_masked(two_slots_at(0),  two_slots_at(2));
    builder.copy_slots_masked(four_slots_at(1), four_slots_at(5));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/9,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_2_slots_masked            v0..1 = Mask(v2..3)
copy_4_slots_masked            v1..4 = Mask(v5..8)
)");
}

DEF_TEST(RasterPipelineBuilderCopySlotsUnmasked, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.copy_slots_unmasked(three_slots_at(0), three_slots_at(2));
    builder.copy_slots_unmasked(five_slots_at(1),  five_slots_at(5));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/10,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_3_slots_unmasked          v0..2 = v2..4
copy_4_slots_unmasked          v1..4 = v5..8
copy_slot_unmasked             v5 = v9
)");
}

DEF_TEST(RasterPipelineBuilderPushPopSlots, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_slots(four_slots_at(10));           // push from 10~13 into $0~$3
    builder.copy_stack_to_slots(one_slot_at(5), 3);  // copy from $1 into 5
    builder.pop_slots_unmasked(two_slots_at(20));    // pop from $2~$3 into 20~21 (unmasked)
    builder.enableExecutionMaskWrites();
    builder.copy_stack_to_slots_unmasked(one_slot_at(4), 2);  // copy from $0 into 4
    builder.push_slots(three_slots_at(30));          // push from 30~32 into $2~$4
    builder.pop_slots(five_slots_at(0));             // pop from $0~$4 into 0~4 (masked)
    builder.disableExecutionMaskWrites();

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/50,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_4_slots_unmasked          $0..3 = v10..13
copy_slot_unmasked             v5 = $1
copy_2_slots_unmasked          v20..21 = $2..3
copy_slot_unmasked             v4 = $0
copy_3_slots_unmasked          $2..4 = v30..32
copy_4_slots_masked            v0..3 = Mask($0..3)
copy_slot_masked               v4 = Mask($4)
)");
}

DEF_TEST(RasterPipelineBuilderDuplicateSelectAndSwizzleSlots, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_constant_f(1.0f);          // push into 0
    builder.push_duplicates(1);             // duplicate into 1
    builder.push_duplicates(2);             // duplicate into 2~3
    builder.push_duplicates(3);             // duplicate into 4~6
    builder.push_duplicates(5);             // duplicate into 7~11
    builder.select(4);                      // select from 4~7 and 8~11 into 4~7
    builder.select(3);                      // select from 2~4 and 5~7 into 2~4
    builder.select(1);                      // select from 3 and 4 into 3
    builder.swizzle_copy_stack_to_slots(four_slots_at(1), {3, 2, 1, 0}, 4);
    builder.swizzle_copy_stack_to_slots(four_slots_at(0), {0, 1, 3}, 3);
    builder.swizzle(4, {3, 2, 1, 0});       // reverse the order of 0~3 (value.wzyx)
    builder.swizzle(4, {1, 2});             // eliminate elements 0 and 3 (value.yz)
    builder.swizzle(2, {0});                // eliminate element 1 (value.x)
    builder.discard_stack(1);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/6,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x3F800000 (1.0)
splat_4_constants              $4..7 = 0x3F800000 (1.0)
splat_4_constants              $8..11 = 0x3F800000 (1.0)
copy_4_slots_masked            $4..7 = Mask($8..11)
copy_3_slots_masked            $2..4 = Mask($5..7)
copy_slot_masked               $3 = Mask($4)
swizzle_copy_4_slots_masked    (v1..4).wzyx = Mask($0..3)
swizzle_copy_3_slots_masked    (v0..3).xyw = Mask($1..3)
swizzle_4                      $0..3 = ($0..3).wzyx
swizzle_2                      $0..1 = ($0..2).yz
)");
}

DEF_TEST(RasterPipelineBuilderTransposeMatrix, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_constant_f(1.0f);          // push into 0
    builder.push_duplicates(15);            // duplicate into 1~15
    builder.transpose(2, 2);                // transpose a 2x2 matrix
    builder.transpose(3, 3);                // transpose a 3x3 matrix
    builder.transpose(4, 4);                // transpose a 4x4 matrix
    builder.transpose(2, 4);                // transpose a 2x4 matrix
    builder.transpose(4, 3);                // transpose a 4x3 matrix
    builder.discard_stack(16);              // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x3F800000 (1.0)
splat_4_constants              $4..7 = 0x3F800000 (1.0)
splat_4_constants              $8..11 = 0x3F800000 (1.0)
splat_4_constants              $12..15 = 0x3F800000 (1.0)
swizzle_3                      $13..15 = ($13..15).yxz
shuffle                        $8..15 = ($8..15)[2 5 0 3 6 1 4 7]
shuffle                        $1..15 = ($1..15)[3 7 11 0 4 8 12 1 5 9 13 2 6 10 14]
shuffle                        $9..15 = ($9..15)[3 0 4 1 5 2 6]
shuffle                        $5..15 = ($5..15)[2 5 8 0 3 6 9 1 4 7 10]
)");
}

DEF_TEST(RasterPipelineBuilderDiagonalMatrix, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_constant_f(0.0f);          // push into 0
    builder.push_constant_f(1.0f);          // push into 1
    builder.diagonal_matrix(2, 2);          // generate a 2x2 diagonal matrix
    builder.discard_stack(4);               // balance stack
    builder.push_constant_f(0.0f);          // push into 0
    builder.push_constant_f(2.0f);          // push into 1
    builder.diagonal_matrix(4, 4);          // generate a 4x4 diagonal matrix
    builder.discard_stack(16);              // balance stack
    builder.push_constant_f(0.0f);          // push into 0
    builder.push_constant_f(3.0f);          // push into 1
    builder.diagonal_matrix(2, 3);          // generate a 2x3 diagonal matrix
    builder.discard_stack(6);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_constant                  $0 = 0
copy_constant                  $1 = 0x3F800000 (1.0)
swizzle_4                      $0..3 = ($0..3).yxxy
copy_constant                  $0 = 0
copy_constant                  $1 = 0x40000000 (2.0)
shuffle                        $0..15 = ($0..15)[1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1]
copy_constant                  $0 = 0
copy_constant                  $1 = 0x40400000 (3.0)
shuffle                        $0..5 = ($0..5)[1 0 0 0 1 0]
)");
}

DEF_TEST(RasterPipelineBuilderMatrixResize, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_constant_f(1.0f);          // synthesize a 2x2 matrix
    builder.push_constant_f(2.0f);
    builder.push_constant_f(3.0f);
    builder.push_constant_f(4.0f);
    builder.matrix_resize(2, 2, 4, 4);      // resize 2x2 matrix into 4x4
    builder.matrix_resize(4, 4, 2, 2);      // resize 4x4 matrix back into 2x2
    builder.matrix_resize(2, 2, 2, 4);      // resize 2x2 matrix into 2x4
    builder.matrix_resize(2, 4, 4, 2);      // resize 2x4 matrix into 4x2
    builder.matrix_resize(4, 2, 3, 3);      // resize 4x2 matrix into 3x3
    builder.discard_stack(9);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_constant                  $0 = 0x3F800000 (1.0)
copy_constant                  $1 = 0x40000000 (2.0)
copy_constant                  $2 = 0x40400000 (3.0)
copy_constant                  $3 = 0x40800000 (4.0)
copy_constant                  $4 = 0
copy_constant                  $5 = 0x3F800000 (1.0)
shuffle                        $2..15 = ($2..15)[2 2 0 1 2 2 2 2 3 2 2 2 2 3]
shuffle                        $2..3 = ($2..3)[2 3]
copy_constant                  $4 = 0
shuffle                        $2..7 = ($2..7)[2 2 0 1 2 2]
copy_constant                  $8 = 0
shuffle                        $2..7 = ($2..7)[2 3 6 6 6 6]
copy_constant                  $8 = 0
copy_constant                  $9 = 0x3F800000 (1.0)
shuffle                        $2..8 = ($2..8)[6 0 1 6 2 3 7]
)");
}

DEF_TEST(RasterPipelineBuilderBranches, r) {
#if SK_HAS_MUSTTAIL
    // We have guaranteed tail-calling, and don't need to rewind the stack.
    static constexpr char kExpectationWithKnownExecutionMask[] =
R"(jump                           jump +9 (label 3 at #10)
label                          label 0
copy_constant                  v0 = 0
label                          label 0x00000001
copy_constant                  v1 = 0
jump                           jump -4 (label 0 at #2)
label                          label 0x00000002
copy_constant                  v2 = 0
jump                           jump -7 (label 0 at #2)
label                          label 0x00000003
branch_if_no_active_lanes_eq   branch -4 (label 2 at #7) if no lanes of v2 == 0
branch_if_no_active_lanes_eq   branch -10 (label 0 at #2) if no lanes of v2 == 0x00000001 (1.401298e-45)
)";
    static constexpr char kExpectationWithExecutionMaskWrites[] =
R"(jump                           jump +10 (label 3 at #11)
label                          label 0
copy_constant                  v0 = 0
label                          label 0x00000001
copy_constant                  v1 = 0
branch_if_no_lanes_active      branch_if_no_lanes_active -2 (label 1 at #4)
branch_if_all_lanes_active     branch_if_all_lanes_active -5 (label 0 at #2)
label                          label 0x00000002
copy_constant                  v2 = 0
branch_if_any_lanes_active     branch_if_any_lanes_active -8 (label 0 at #2)
label                          label 0x00000003
branch_if_no_active_lanes_eq   branch -4 (label 2 at #8) if no lanes of v2 == 0
branch_if_no_active_lanes_eq   branch -11 (label 0 at #2) if no lanes of v2 == 0x00000001 (1.401298e-45)
)";
#else
    // We don't have guaranteed tail-calling, so we rewind the stack immediately before any backward
    // branches.
    static constexpr char kExpectationWithKnownExecutionMask[] =
R"(jump                           jump +11 (label 3 at #12)
label                          label 0
copy_constant                  v0 = 0
label                          label 0x00000001
copy_constant                  v1 = 0
stack_rewind
jump                           jump -5 (label 0 at #2)
label                          label 0x00000002
copy_constant                  v2 = 0
stack_rewind
jump                           jump -9 (label 0 at #2)
label                          label 0x00000003
stack_rewind
branch_if_no_active_lanes_eq   branch -6 (label 2 at #8) if no lanes of v2 == 0
stack_rewind
branch_if_no_active_lanes_eq   branch -14 (label 0 at #2) if no lanes of v2 == 0x00000001 (1.401298e-45)
)";
    static constexpr char kExpectationWithExecutionMaskWrites[] =
R"(jump                           jump +13 (label 3 at #14)
label                          label 0
copy_constant                  v0 = 0
label                          label 0x00000001
copy_constant                  v1 = 0
stack_rewind
branch_if_no_lanes_active      branch_if_no_lanes_active -3 (label 1 at #4)
stack_rewind
branch_if_all_lanes_active     branch_if_all_lanes_active -7 (label 0 at #2)
label                          label 0x00000002
copy_constant                  v2 = 0
stack_rewind
branch_if_any_lanes_active     branch_if_any_lanes_active -11 (label 0 at #2)
label                          label 0x00000003
stack_rewind
branch_if_no_active_lanes_eq   branch -6 (label 2 at #10) if no lanes of v2 == 0
stack_rewind
branch_if_no_active_lanes_eq   branch -16 (label 0 at #2) if no lanes of v2 == 0x00000001 (1.401298e-45)
)";
#endif

    for (bool enableExecutionMaskWrites : {false, true}) {
        // Create a very simple nonsense program.
        SkSL::RP::Builder builder;
        int label1 = builder.nextLabelID();
        int label2 = builder.nextLabelID();
        int label3 = builder.nextLabelID();
        int label4 = builder.nextLabelID();

        if (enableExecutionMaskWrites) {
            builder.enableExecutionMaskWrites();
        }

        builder.jump(label4);
        builder.label(label1);
        builder.zero_slots_unmasked(one_slot_at(0));
        builder.label(label2);
        builder.zero_slots_unmasked(one_slot_at(1));
        builder.branch_if_no_lanes_active(label2);
        builder.branch_if_no_lanes_active(label3);
        builder.branch_if_all_lanes_active(label1);
        builder.label(label3);
        builder.zero_slots_unmasked(one_slot_at(2));
        builder.branch_if_any_lanes_active(label1);
        builder.branch_if_any_lanes_active(label1);
        builder.label(label4);
        builder.branch_if_no_active_lanes_on_stack_top_equal(0, label3);
        builder.branch_if_no_active_lanes_on_stack_top_equal(0, label2);
        builder.branch_if_no_active_lanes_on_stack_top_equal(1, label1);
        builder.branch_if_no_active_lanes_on_stack_top_equal(1, label4);

        if (enableExecutionMaskWrites) {
            builder.disableExecutionMaskWrites();
        }

        std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/3,
                                                                    /*numUniformSlots=*/0,
                                                                    /*numImmutableSlots=*/0);

        check(r, *program, enableExecutionMaskWrites ? kExpectationWithExecutionMaskWrites
                                                     : kExpectationWithKnownExecutionMask);
    }
}

DEF_TEST(RasterPipelineBuilderBinaryFloatOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_f(10.0f);
    builder.push_duplicates(30);
    builder.binary_op(BuilderOp::add_n_floats, 1);
    builder.binary_op(BuilderOp::sub_n_floats, 2);
    builder.binary_op(BuilderOp::mul_n_floats, 3);
    builder.binary_op(BuilderOp::div_n_floats, 4);
    builder.binary_op(BuilderOp::max_n_floats, 3);
    builder.binary_op(BuilderOp::min_n_floats, 2);
    builder.binary_op(BuilderOp::cmplt_n_floats, 5);
    builder.binary_op(BuilderOp::cmple_n_floats, 4);
    builder.binary_op(BuilderOp::cmpeq_n_floats, 3);
    builder.binary_op(BuilderOp::cmpne_n_floats, 2);
    builder.discard_stack(2);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x41200000 (10.0)
splat_4_constants              $4..7 = 0x41200000 (10.0)
splat_4_constants              $8..11 = 0x41200000 (10.0)
splat_4_constants              $12..15 = 0x41200000 (10.0)
splat_4_constants              $16..19 = 0x41200000 (10.0)
splat_4_constants              $20..23 = 0x41200000 (10.0)
splat_4_constants              $24..27 = 0x41200000 (10.0)
splat_2_constants              $28..29 = 0x41200000 (10.0)
add_imm_float                  $29 += 0x41200000 (10.0)
sub_2_floats                   $26..27 -= $28..29
mul_3_floats                   $22..24 *= $25..27
div_4_floats                   $17..20 /= $21..24
max_3_floats                   $15..17 = max($15..17, $18..20)
min_2_floats                   $14..15 = min($14..15, $16..17)
cmplt_n_floats                 $6..10 = lessThan($6..10, $11..15)
cmple_4_floats                 $3..6 = lessThanEqual($3..6, $7..10)
cmpeq_3_floats                 $1..3 = equal($1..3, $4..6)
cmpne_2_floats                 $0..1 = notEqual($0..1, $2..3)
)");
}

DEF_TEST(RasterPipelineBuilderBinaryIntOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_i(123);
    builder.push_duplicates(40);
    builder.binary_op(BuilderOp::bitwise_and_n_ints, 1);
    builder.binary_op(BuilderOp::bitwise_xor_n_ints, 2);
    builder.binary_op(BuilderOp::bitwise_or_n_ints, 3);
    builder.binary_op(BuilderOp::add_n_ints, 2);
    builder.binary_op(BuilderOp::sub_n_ints, 3);
    builder.binary_op(BuilderOp::mul_n_ints, 4);
    builder.binary_op(BuilderOp::div_n_ints, 5);
    builder.binary_op(BuilderOp::max_n_ints, 4);
    builder.binary_op(BuilderOp::min_n_ints, 3);
    builder.binary_op(BuilderOp::cmplt_n_ints, 1);
    builder.binary_op(BuilderOp::cmple_n_ints, 2);
    builder.binary_op(BuilderOp::cmpeq_n_ints, 3);
    builder.binary_op(BuilderOp::cmpne_n_ints, 4);
    builder.discard_stack(4);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x0000007B (1.723597e-43)
splat_4_constants              $4..7 = 0x0000007B (1.723597e-43)
splat_4_constants              $8..11 = 0x0000007B (1.723597e-43)
splat_4_constants              $12..15 = 0x0000007B (1.723597e-43)
splat_4_constants              $16..19 = 0x0000007B (1.723597e-43)
splat_4_constants              $20..23 = 0x0000007B (1.723597e-43)
splat_4_constants              $24..27 = 0x0000007B (1.723597e-43)
splat_4_constants              $28..31 = 0x0000007B (1.723597e-43)
splat_4_constants              $32..35 = 0x0000007B (1.723597e-43)
splat_4_constants              $36..39 = 0x0000007B (1.723597e-43)
bitwise_and_imm_int            $39 &= 0x0000007B
bitwise_xor_2_ints             $36..37 ^= $38..39
bitwise_or_3_ints              $32..34 |= $35..37
add_2_ints                     $31..32 += $33..34
sub_3_ints                     $27..29 -= $30..32
mul_4_ints                     $22..25 *= $26..29
div_n_ints                     $16..20 /= $21..25
max_4_ints                     $13..16 = max($13..16, $17..20)
min_3_ints                     $11..13 = min($11..13, $14..16)
cmplt_int                      $12 = lessThan($12, $13)
cmple_2_ints                   $9..10 = lessThanEqual($9..10, $11..12)
cmpeq_3_ints                   $5..7 = equal($5..7, $8..10)
cmpne_4_ints                   $0..3 = notEqual($0..3, $4..7)
)");
}

DEF_TEST(RasterPipelineBuilderBinaryUIntOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_u(456);
    builder.push_duplicates(21);
    builder.binary_op(BuilderOp::div_n_uints, 6);
    builder.binary_op(BuilderOp::cmplt_n_uints, 5);
    builder.binary_op(BuilderOp::cmple_n_uints, 4);
    builder.binary_op(BuilderOp::max_n_uints, 3);
    builder.binary_op(BuilderOp::min_n_uints, 2);
    builder.discard_stack(2);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x000001C8 (6.389921e-43)
splat_4_constants              $4..7 = 0x000001C8 (6.389921e-43)
splat_4_constants              $8..11 = 0x000001C8 (6.389921e-43)
splat_4_constants              $12..15 = 0x000001C8 (6.389921e-43)
splat_4_constants              $16..19 = 0x000001C8 (6.389921e-43)
splat_2_constants              $20..21 = 0x000001C8 (6.389921e-43)
div_n_uints                    $10..15 /= $16..21
cmplt_n_uints                  $6..10 = lessThan($6..10, $11..15)
cmple_4_uints                  $3..6 = lessThanEqual($3..6, $7..10)
max_3_uints                    $1..3 = max($1..3, $4..6)
min_2_uints                    $0..1 = min($0..1, $2..3)
)");
}

DEF_TEST(RasterPipelineBuilderUnaryOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_i(456);
    builder.push_duplicates(4);
    builder.unary_op(BuilderOp::cast_to_float_from_int, 1);
    builder.unary_op(BuilderOp::cast_to_float_from_uint, 2);
    builder.unary_op(BuilderOp::cast_to_int_from_float, 3);
    builder.unary_op(BuilderOp::cast_to_uint_from_float, 4);
    builder.unary_op(BuilderOp::cos_float, 4);
    builder.unary_op(BuilderOp::tan_float, 3);
    builder.unary_op(BuilderOp::sin_float, 2);
    builder.unary_op(BuilderOp::sqrt_float, 1);
    builder.unary_op(BuilderOp::abs_int, 2);
    builder.unary_op(BuilderOp::floor_float, 3);
    builder.unary_op(BuilderOp::ceil_float, 4);
    builder.discard_stack(5);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x000001C8 (6.389921e-43)
copy_constant                  $4 = 0x000001C8 (6.389921e-43)
cast_to_float_from_int         $4 = IntToFloat($4)
cast_to_float_from_2_uints     $3..4 = UintToFloat($3..4)
cast_to_int_from_3_floats      $2..4 = FloatToInt($2..4)
cast_to_uint_from_4_floats     $1..4 = FloatToUint($1..4)
cos_float                      $1 = cos($1)
cos_float                      $2 = cos($2)
cos_float                      $3 = cos($3)
cos_float                      $4 = cos($4)
tan_float                      $2 = tan($2)
tan_float                      $3 = tan($3)
tan_float                      $4 = tan($4)
sin_float                      $3 = sin($3)
sin_float                      $4 = sin($4)
sqrt_float                     $4 = sqrt($4)
abs_2_ints                     $3..4 = abs($3..4)
floor_3_floats                 $2..4 = floor($2..4)
ceil_4_floats                  $1..4 = ceil($1..4)
)");
}

DEF_TEST(RasterPipelineBuilderUniforms, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_uniform(one_slot_at(0));        // push into 0
    builder.push_uniform(two_slots_at(1));       // push into 1~2
    builder.push_uniform(three_slots_at(3));     // push into 3~5
    builder.push_uniform(four_slots_at(6));      // push into 6~9
    builder.push_uniform(five_slots_at(0));      // push into 10~14
    builder.unary_op(BuilderOp::abs_int, 1);     // perform work so the program isn't eliminated
    builder.discard_stack(15);                   // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/10,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(copy_4_uniforms                $0..3 = u0..3
copy_4_uniforms                $4..7 = u4..7
copy_2_uniforms                $8..9 = u8..9
copy_4_uniforms                $10..13 = u0..3
copy_uniform                   $14 = u4
abs_int                        $14 = abs($14)
)");
}

DEF_TEST(RasterPipelineBuilderPushZeros, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_zeros(1);                    // push into 0
    builder.push_zeros(2);                    // push into 1~2
    builder.push_zeros(3);                    // push into 3~5
    builder.push_zeros(4);                    // push into 6~9
    builder.push_zeros(5);                    // push into 10~14
    builder.unary_op(BuilderOp::abs_int, 1);  // perform work so the program isn't eliminated
    builder.discard_stack(15);                // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/10,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0
splat_4_constants              $4..7 = 0
splat_4_constants              $8..11 = 0
splat_3_constants              $12..14 = 0
abs_int                        $14 = abs($14)
)");
}

DEF_TEST(RasterPipelineBuilderTernaryFloatOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_f(0.75f);
    builder.push_duplicates(8);
    builder.ternary_op(BuilderOp::mix_n_floats, 3);
    builder.discard_stack(3);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    check(r, *program,
R"(splat_4_constants              $0..3 = 0x3F400000 (0.75)
splat_4_constants              $4..7 = 0x3F400000 (0.75)
copy_constant                  $8 = 0x3F400000 (0.75)
mix_3_floats                   $0..2 = mix($3..5, $6..8, $0..2)
)");
}

DEF_TEST(RasterPipelineBuilderAutomaticStackRewinding, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_constant_i(1);
    builder.push_duplicates(2000);
    builder.unary_op(BuilderOp::abs_int, 1);  // perform work so the program isn't eliminated
    builder.discard_stack(2001);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0,
                                                                /*numImmutableSlots=*/0);
    sk_sp<SkData> dump = get_program_dump(*program);

#if SK_HAS_MUSTTAIL
    // We have guaranteed tail-calling, so we never use `stack_rewind`.
    REPORTER_ASSERT(r, !skstd::contains(as_string_view(dump), "stack_rewind"));
#else
    // We can't guarantee tail-calling, so we should automatically insert `stack_rewind` stages into
    // long programs.
    REPORTER_ASSERT(r, skstd::contains(as_string_view(dump), "stack_rewind"));
#endif
}

DEF_TEST(RasterPipelineBuilderTraceOps, r) {
    for (bool provideDebugTrace : {false, true}) {
        SkSL::RP::Builder builder;
        // Create a trace mask stack on stack-ID 123.
        builder.set_current_stack(123);
        builder.push_constant_i(~0);
        // Emit trace ops.
        builder.trace_enter(123, 2);
        builder.trace_scope(123, +1);
        builder.trace_line(123, 456);
        builder.trace_var(123, two_slots_at(3));
        builder.trace_scope(123, -1);
        builder.trace_exit(123, 2);
        // Discard the trace mask.
        builder.discard_stack(1);

        if (!provideDebugTrace) {
            // Test the output when no DebugTrace info is provided.
            std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/20,
                                                                        /*numUniformSlots=*/0,
                                                                        /*numImmutableSlots=*/0);
            check(r, *program,
R"(copy_constant                  $0 = 0xFFFFFFFF
trace_enter                    TraceEnter(???) when $0 is true
trace_scope                    TraceScope(+1) when $0 is true
trace_line                     TraceLine(456) when $0 is true
trace_var                      TraceVar(v3..4) when $0 is true
trace_scope                    TraceScope(-1) when $0 is true
trace_exit                     TraceExit(???) when $0 is true
)");
        } else {
            // Test the output when we supply a populated DebugTrace.
            SkSL::DebugTracePriv trace;
            trace.fFuncInfo = {{"FunctionA"}, {"FunctionB"}, {"FunctionC"}, {"FunctionD"}};

            SkSL::SlotDebugInfo slot;
            slot.name = "Var0";
            trace.fSlotInfo.push_back(slot);
            slot.name = "Var1";
            trace.fSlotInfo.push_back(slot);
            slot.name = "Var2";
            trace.fSlotInfo.push_back(slot);
            slot.name = "Var3";
            trace.fSlotInfo.push_back(slot);
            slot.name = "Var4";
            trace.fSlotInfo.push_back(slot);

            std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/20,
                                                                        /*numUniformSlots=*/0,
                                                                        /*numImmutableSlots=*/0,
                                                                        &trace);
            check(r, *program,
R"(copy_constant                  $0 = 0xFFFFFFFF
trace_enter                    TraceEnter(FunctionC) when $0 is true
trace_scope                    TraceScope(+1) when $0 is true
trace_line                     TraceLine(456) when $0 is true
trace_var                      TraceVar(Var3, Var4) when $0 is true
trace_scope                    TraceScope(-1) when $0 is true
trace_exit                     TraceExit(FunctionC) when $0 is true
)");
        }
    }
}
