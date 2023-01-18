/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/base/SkStringView.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "tests/Test.h"

static sk_sp<SkData> get_program_dump(SkSL::RP::Program& program) {
    SkDynamicMemoryWStream stream;
    program.dump(&stream);
    return stream.detachAsData();
}

static std::string_view as_string_view(sk_sp<SkData> dump) {
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

DEF_TEST(RasterPipelineBuilder, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.store_src_rg(two_slots_at(0));
    builder.store_src(four_slots_at(2));
    builder.store_dst(four_slots_at(6));
    builder.init_lane_masks();
    builder.mask_off_return_mask();
    builder.mask_off_loop_mask();
    builder.reenable_loop_mask(one_slot_at(4));
    builder.load_src(four_slots_at(1));
    builder.load_dst(four_slots_at(3));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/10,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. store_src_rg                   v0..1 = src.rg
    2. store_src                      v2..5 = src.rgba
    3. store_dst                      v6..9 = dst.rgba
    4. init_lane_masks                CondMask = LoopMask = RetMask = true
    5. mask_off_return_mask           RetMask &= ~(CondMask & LoopMask & RetMask)
    6. mask_off_loop_mask             LoopMask &= ~(CondMask & LoopMask & RetMask)
    7. reenable_loop_mask             LoopMask |= v4
    8. load_src                       src.rgba = v1..4
    9. load_dst                       dst.rgba = v3..6
)");
}

DEF_TEST(RasterPipelineBuilderImmediate, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.immediate_f(333.0f);
    builder.immediate_f(0.0f);
    builder.immediate_f(-5555.0f);
    builder.immediate_i(-123);
    builder.immediate_u(456);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. immediate_f                    src.r = 0x43A68000 (333.0)
    2. immediate_f                    src.r = 0x00000000 (0.0)
    3. immediate_f                    src.r = 0xC5AD9800 (-5555.0)
    4. immediate_f                    src.r = 0xFFFFFF85
    5. immediate_f                    src.r = 0x000001C8 (6.389921e-43)
)");
}

DEF_TEST(RasterPipelineBuilderLoadStoreAccumulator, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.load_unmasked(12);
    builder.store_unmasked(34);
    builder.store_unmasked(56);
    builder.store_masked(0);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/57,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. load_unmasked                  src.r = v12
    2. store_unmasked                 v34 = src.r
    3. store_unmasked                 v56 = src.r
    4. store_masked                   v0 = Mask(src.r)
)");
}

DEF_TEST(RasterPipelineBuilderPushPopMaskRegisters, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_condition_mask();  // push into 0
    builder.push_loop_mask();       // push into 1
    builder.push_return_mask();     // push into 2
    builder.merge_condition_mask(); // set the condition-mask to 1 & 2
    builder.pop_condition_mask();   // pop from 2
    builder.merge_loop_mask();      // mask off the loop-mask against 1
    builder.push_condition_mask();  // push into 2
    builder.pop_condition_mask();   // pop from 2
    builder.pop_loop_mask();        // pop from 1
    builder.pop_return_mask();      // pop from 0
    builder.push_condition_mask();  // push into 0
    builder.pop_condition_mask();   // pop from 0
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. store_condition_mask           $0 = CondMask
    2. store_loop_mask                $1 = LoopMask
    3. store_return_mask              $2 = RetMask
    4. merge_condition_mask           CondMask = $1 & $2
    5. load_condition_mask            CondMask = $2
    6. merge_loop_mask                LoopMask &= $1
    7. store_condition_mask           $2 = CondMask
    8. load_condition_mask            CondMask = $2
    9. load_loop_mask                 LoopMask = $1
   10. load_return_mask               RetMask = $0
   11. store_condition_mask           $0 = CondMask
   12. load_condition_mask            CondMask = $0
)");
}

DEF_TEST(RasterPipelineBuilderPushPopTempImmediates, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.set_current_stack(1);
    builder.push_literal_i(999);                                          // push into 2
    builder.set_current_stack(0);
    builder.push_literal_f(13.5f);                                        // push into 0
    builder.push_clone_from_stack(/*numSlots=*/1, /*otherStackIndex=*/1); // push into 1 from 2
    builder.discard_stack();                                              // discard 2
    builder.push_literal_u(357);                                          // push into 2
    builder.set_current_stack(1);
    builder.push_clone_from_stack(/*numSlots=*/1, /*otherStackIndex=*/0,
                                  /*offsetFromStackTop=*/1);              // push into 3 from 0
    builder.discard_stack(2);                                             // discard 2 and 3
    builder.set_current_stack(0);
    builder.discard_stack(2);                                             // discard 0 and 1
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/1,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $2 = 0x000003E7 (1.399897e-42)
    2. copy_constant                  $0 = 0x41580000 (13.5)
    3. copy_constant                  $1 = 0x00000165 (5.002636e-43)
)");
}

DEF_TEST(RasterPipelineBuilderCopySlotsMasked, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.copy_slots_masked(two_slots_at(0),  two_slots_at(2));
    builder.copy_slots_masked(four_slots_at(1), four_slots_at(5));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/9,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_2_slots_masked            v0..1 = Mask(v2..3)
    2. copy_4_slots_masked            v1..4 = Mask(v5..8)
)");
}

DEF_TEST(RasterPipelineBuilderCopySlotsUnmasked, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.copy_slots_unmasked(three_slots_at(0), three_slots_at(2));
    builder.copy_slots_unmasked(five_slots_at(1),  five_slots_at(5));
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/10,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_3_slots_unmasked          v0..2 = v2..4
    2. copy_4_slots_unmasked          v1..4 = v5..8
    3. copy_slot_unmasked             v5 = v9
)");
}

DEF_TEST(RasterPipelineBuilderPushPopSlots, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_slots(four_slots_at(10));           // push from 10~13 into $0~$3
    builder.copy_stack_to_slots(one_slot_at(5), 3);  // copy from $1 into 5
    builder.pop_slots_unmasked(two_slots_at(20));    // pop from $2~$3 into 20~21 (unmasked)
    builder.copy_stack_to_slots_unmasked(one_slot_at(4), 2);  // copy from $0 into 4
    builder.push_slots(three_slots_at(30));          // push from 30~32 into $2~$4
    builder.pop_slots(five_slots_at(0));             // pop from $0~$4 into 0~4 (masked)
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/50,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_4_slots_unmasked          $0..3 = v10..13
    2. copy_slot_masked               v5 = Mask($1)
    3. copy_2_slots_unmasked          v20..21 = $2..3
    4. copy_slot_unmasked             v4 = $0
    5. copy_3_slots_unmasked          $2..4 = v30..32
    6. copy_4_slots_masked            v0..3 = Mask($0..3)
    7. copy_slot_masked               v4 = Mask($4)
)");
}

DEF_TEST(RasterPipelineBuilderDuplicateSelectAndSwizzleSlots, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_literal_f(1.0f);           // push into 0
    builder.push_duplicates(1);             // duplicate into 1
    builder.push_duplicates(2);             // duplicate into 2~3
    builder.push_duplicates(3);             // duplicate into 4~6
    builder.push_duplicates(5);             // duplicate into 7~11
    builder.select(4);                      // select from 4~7 and 8~11 into 4~7
    builder.select(3);                      // select from 2~4 and 5~7 into 2~4
    builder.select(1);                      // select from 3 and 4 into 3
    builder.swizzle(4, {3, 2, 1, 0});       // reverse the order of 0~3 (value.wzyx)
    builder.swizzle(4, {1, 2});             // eliminate elements 0 and 3 (value.yz)
    builder.swizzle(2, {0});                // eliminate element 1 (value.x)
    builder.discard_stack(1);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/1,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x3F800000 (1.0)
    2. copy_slot_unmasked             $1 = $0
    3. swizzle_3                      $1..3 = ($1..3).xxx
    4. swizzle_4                      $3..6 = ($3..6).xxxx
    5. swizzle_4                      $6..9 = ($6..9).xxxx
    6. swizzle_3                      $9..11 = ($9..11).xxx
    7. copy_4_slots_masked            $4..7 = Mask($8..11)
    8. copy_3_slots_masked            $2..4 = Mask($5..7)
    9. copy_slot_masked               $3 = Mask($4)
   10. swizzle_4                      $0..3 = ($0..3).wzyx
   11. swizzle_2                      $0..1 = ($0..2).yz
)");
}

DEF_TEST(RasterPipelineBuilderTransposeMatrix, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_literal_f(1.0f);           // push into 0
    builder.push_duplicates(15);            // duplicate into 1~15
    builder.transpose(2, 2);                // transpose a 2x2 matrix
    builder.transpose(3, 3);                // transpose a 3x3 matrix
    builder.transpose(4, 4);                // transpose a 4x4 matrix
    builder.transpose(2, 4);                // transpose a 2x4 matrix
    builder.transpose(4, 3);                // transpose a 4x3 matrix
    builder.discard_stack(16);              // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x3F800000 (1.0)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_4_slots_unmasked          $4..7 = $0..3
    4. copy_4_slots_unmasked          $8..11 = $4..7
    5. copy_4_slots_unmasked          $12..15 = $8..11
    6. swizzle_3                      $13..15 = ($13..15).yxz
    7. shuffle                        $8..15 = ($8..15)[2 5 0 3 6 1 4 7]
    8. shuffle                        $1..15 = ($1..15)[3 7 11 0 4 8 12 1 5 9 13 2 6 10 14]
    9. shuffle                        $9..15 = ($9..15)[3 0 4 1 5 2 6]
   10. shuffle                        $5..15 = ($5..15)[2 5 8 0 3 6 9 1 4 7 10]
)");
}

DEF_TEST(RasterPipelineBuilderDiagonalMatrix, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_literal_f(0.0f);           // push into 0
    builder.push_literal_f(1.0f);           // push into 1
    builder.diagonal_matrix(2, 2);          // generate a 2x2 diagonal matrix
    builder.discard_stack(4);               // balance stack
    builder.push_literal_f(0.0f);           // push into 0
    builder.push_literal_f(2.0f);           // push into 1
    builder.diagonal_matrix(4, 4);          // generate a 4x4 diagonal matrix
    builder.discard_stack(16);              // balance stack
    builder.push_literal_f(0.0f);           // push into 0
    builder.push_literal_f(3.0f);           // push into 1
    builder.diagonal_matrix(2, 3);          // generate a 2x3 diagonal matrix
    builder.discard_stack(6);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. zero_slot_unmasked             $0 = 0
    2. copy_constant                  $1 = 0x3F800000 (1.0)
    3. swizzle_4                      $0..3 = ($0..3).yxxy
    4. zero_slot_unmasked             $0 = 0
    5. copy_constant                  $1 = 0x40000000 (2.0)
    6. shuffle                        $0..15 = ($0..15)[1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1]
    7. zero_slot_unmasked             $0 = 0
    8. copy_constant                  $1 = 0x40400000 (3.0)
    9. shuffle                        $0..5 = ($0..5)[1 0 0 0 1 0]
)");
}

DEF_TEST(RasterPipelineBuilderMatrixResize, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    builder.push_literal_f(1.0f);           // synthesize a 2x2 matrix
    builder.push_literal_f(2.0f);
    builder.push_literal_f(3.0f);
    builder.push_literal_f(4.0f);
    builder.matrix_resize(2, 2, 4, 4);      // resize 2x2 matrix into 4x4
    builder.matrix_resize(4, 4, 2, 2);      // resize 4x4 matrix back into 2x2
    builder.matrix_resize(2, 2, 2, 4);      // resize 2x2 matrix into 2x4
    builder.matrix_resize(2, 4, 4, 2);      // resize 2x4 matrix into 4x2
    builder.matrix_resize(4, 2, 3, 3);      // resize 4x2 matrix into 3x3
    builder.discard_stack(9);               // balance stack
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x3F800000 (1.0)
    2. copy_constant                  $1 = 0x40000000 (2.0)
    3. copy_constant                  $2 = 0x40400000 (3.0)
    4. copy_constant                  $3 = 0x40800000 (4.0)
    5. zero_slot_unmasked             $4 = 0
    6. copy_constant                  $5 = 0x3F800000 (1.0)
    7. shuffle                        $2..15 = ($2..15)[2 2 0 1 2 2 2 2 3 2 2 2 2 3]
    8. shuffle                        $2..3 = ($2..3)[2 3]
    9. zero_slot_unmasked             $4 = 0
   10. shuffle                        $2..7 = ($2..7)[2 2 0 1 2 2]
   11. zero_slot_unmasked             $8 = 0
   12. shuffle                        $2..7 = ($2..7)[2 3 6 6 6 6]
   13. zero_slot_unmasked             $8 = 0
   14. copy_constant                  $9 = 0x3F800000 (1.0)
   15. shuffle                        $2..8 = ($2..8)[6 0 1 6 2 3 7]
)");
}

DEF_TEST(RasterPipelineBuilderBranches, r) {
    // Create a very simple nonsense program.
    SkSL::RP::Builder builder;
    int label1 = builder.nextLabelID();
    int label2 = builder.nextLabelID();
    int label3 = builder.nextLabelID();

    builder.jump(label3);
    builder.label(label1);
    builder.immediate_f(1.0f);
    builder.label(label2);
    builder.immediate_f(2.0f);
    builder.branch_if_no_active_lanes(label2);
    builder.branch_if_no_active_lanes(label3);
    builder.label(label3);
    builder.immediate_f(3.0f);
    builder.branch_if_any_active_lanes(label1);
    builder.branch_if_any_active_lanes(label1);

    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/1,
                                                                /*numUniformSlots=*/0);
#if SK_HAS_MUSTTAIL
    // We have guaranteed tail-calling, and don't need to rewind the stack.
    static constexpr char kExpectation[] =
R"(    1. jump                           jump +4 (#5)
    2. immediate_f                    src.r = 0x3F800000 (1.0)
    3. immediate_f                    src.r = 0x40000000 (2.0)
    4. branch_if_no_active_lanes      branch_if_no_active_lanes -1 (#3)
    5. immediate_f                    src.r = 0x40400000 (3.0)
    6. branch_if_any_active_lanes     branch_if_any_active_lanes -4 (#2)
)";
#else
    // We don't have guaranteed tail-calling, so we rewind the stack immediately before any backward
    // branches.
    static constexpr char kExpectation[] =
R"(    1. jump                           jump +5 (#6)
    2. immediate_f                    src.r = 0x3F800000 (1.0)
    3. immediate_f                    src.r = 0x40000000 (2.0)
    4. stack_rewind
    5. branch_if_no_active_lanes      branch_if_no_active_lanes -2 (#3)
    6. immediate_f                    src.r = 0x40400000 (3.0)
    7. stack_rewind
    8. branch_if_any_active_lanes     branch_if_any_active_lanes -6 (#2)
)";
#endif

    check(r, *program, kExpectation);
}

DEF_TEST(RasterPipelineBuilderBinaryFloatOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_f(10.0f);
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
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x41200000 (10.0)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_4_slots_unmasked          $4..7 = $0..3
    4. copy_4_slots_unmasked          $8..11 = $4..7
    5. copy_4_slots_unmasked          $12..15 = $8..11
    6. copy_4_slots_unmasked          $16..19 = $12..15
    7. copy_4_slots_unmasked          $20..23 = $16..19
    8. copy_4_slots_unmasked          $24..27 = $20..23
    9. swizzle_4                      $27..30 = ($27..30).xxxx
   10. add_float                      $29 += $30
   11. sub_2_floats                   $26..27 -= $28..29
   12. mul_3_floats                   $22..24 *= $25..27
   13. div_4_floats                   $17..20 /= $21..24
   14. max_3_floats                   $15..17 = max($15..17, $18..20)
   15. min_2_floats                   $14..15 = min($14..15, $16..17)
   16. cmplt_n_floats                 $6..10 = lessThan($6..10, $11..15)
   17. cmple_4_floats                 $3..6 = lessThanEqual($3..6, $7..10)
   18. cmpeq_3_floats                 $1..3 = equal($1..3, $4..6)
   19. cmpne_2_floats                 $0..1 = notEqual($0..1, $2..3)
)");
}

DEF_TEST(RasterPipelineBuilderBinaryIntOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_i(123);
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
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x0000007B (1.723597e-43)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_4_slots_unmasked          $4..7 = $0..3
    4. copy_4_slots_unmasked          $8..11 = $4..7
    5. copy_4_slots_unmasked          $12..15 = $8..11
    6. copy_4_slots_unmasked          $16..19 = $12..15
    7. copy_4_slots_unmasked          $20..23 = $16..19
    8. copy_4_slots_unmasked          $24..27 = $20..23
    9. copy_4_slots_unmasked          $28..31 = $24..27
   10. copy_4_slots_unmasked          $32..35 = $28..31
   11. copy_4_slots_unmasked          $36..39 = $32..35
   12. copy_slot_unmasked             $40 = $39
   13. bitwise_and_int                $39 &= $40
   14. bitwise_xor_2_ints             $36..37 ^= $38..39
   15. bitwise_or_3_ints              $32..34 |= $35..37
   16. add_2_ints                     $31..32 += $33..34
   17. sub_3_ints                     $27..29 -= $30..32
   18. mul_4_ints                     $22..25 *= $26..29
   19. div_n_ints                     $16..20 /= $21..25
   20. max_4_ints                     $13..16 = max($13..16, $17..20)
   21. min_3_ints                     $11..13 = min($11..13, $14..16)
   22. cmplt_int                      $12 = lessThan($12, $13)
   23. cmple_2_ints                   $9..10 = lessThanEqual($9..10, $11..12)
   24. cmpeq_3_ints                   $5..7 = equal($5..7, $8..10)
   25. cmpne_4_ints                   $0..3 = notEqual($0..3, $4..7)
)");
}

DEF_TEST(RasterPipelineBuilderBinaryUIntOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_u(456);
    builder.push_duplicates(21);
    builder.binary_op(BuilderOp::div_n_uints, 6);
    builder.binary_op(BuilderOp::cmplt_n_uints, 5);
    builder.binary_op(BuilderOp::cmple_n_uints, 4);
    builder.binary_op(BuilderOp::max_n_uints, 3);
    builder.binary_op(BuilderOp::min_n_uints, 2);
    builder.discard_stack(2);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x000001C8 (6.389921e-43)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_4_slots_unmasked          $4..7 = $0..3
    4. copy_4_slots_unmasked          $8..11 = $4..7
    5. copy_4_slots_unmasked          $12..15 = $8..11
    6. copy_4_slots_unmasked          $16..19 = $12..15
    7. swizzle_3                      $19..21 = ($19..21).xxx
    8. div_n_uints                    $10..15 /= $16..21
    9. cmplt_n_uints                  $6..10 = lessThan($6..10, $11..15)
   10. cmple_4_uints                  $3..6 = lessThanEqual($3..6, $7..10)
   11. max_3_uints                    $1..3 = max($1..3, $4..6)
   12. min_2_uints                    $0..1 = min($0..1, $2..3)
)");
}

DEF_TEST(RasterPipelineBuilderUnaryOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_i(456);
    builder.push_duplicates(4);
    builder.unary_op(BuilderOp::cast_to_float_from_int, 1);
    builder.unary_op(BuilderOp::cast_to_float_from_uint, 2);
    builder.unary_op(BuilderOp::cast_to_int_from_float, 3);
    builder.unary_op(BuilderOp::cast_to_uint_from_float, 4);
    builder.unary_op(BuilderOp::bitwise_not_int, 5);
    builder.unary_op(BuilderOp::abs_float, 4);
    builder.unary_op(BuilderOp::abs_int, 3);
    builder.unary_op(BuilderOp::floor_float, 2);
    builder.unary_op(BuilderOp::ceil_float, 1);
    builder.discard_stack(5);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x000001C8 (6.389921e-43)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_slot_unmasked             $4 = $3
    4. cast_to_float_from_int         $4 = IntToFloat($4)
    5. cast_to_float_from_2_uints     $3..4 = UintToFloat($3..4)
    6. cast_to_int_from_3_floats      $2..4 = FloatToInt($2..4)
    7. cast_to_uint_from_4_floats     $1..4 = FloatToUint($1..4)
    8. bitwise_not_4_ints             $0..3 = ~$0..3
    9. bitwise_not_int                $4 = ~$4
   10. abs_4_floats                   $1..4 = abs($1..4)
   11. abs_3_ints                     $2..4 = abs($2..4)
   12. floor_2_floats                 $3..4 = floor($3..4)
   13. ceil_float                     $4 = ceil($4)
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
                                                                /*numUniformSlots=*/10);
    check(r, *program,
R"(    1. copy_4_constants               $0..3 = u0..3
    2. copy_4_constants               $4..7 = u4..7
    3. copy_2_constants               $8..9 = u8..9
    4. copy_4_constants               $10..13 = u0..3
    5. copy_constant                  $14 = u4
    6. abs_int                        $14 = abs($14)
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
                                                                /*numUniformSlots=*/10);
    check(r, *program,
R"(    1. zero_4_slots_unmasked          $0..3 = 0
    2. zero_4_slots_unmasked          $4..7 = 0
    3. zero_4_slots_unmasked          $8..11 = 0
    4. zero_3_slots_unmasked          $12..14 = 0
    5. abs_int                        $14 = abs($14)
)");
}

DEF_TEST(RasterPipelineBuilderTernaryFloatOps, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_f(0.75f);
    builder.push_duplicates(8);
    builder.ternary_op(BuilderOp::mix_n_floats, 3);
    builder.discard_stack(3);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
    check(r, *program,
R"(    1. copy_constant                  $0 = 0x3F400000 (0.75)
    2. swizzle_4                      $0..3 = ($0..3).xxxx
    3. copy_4_slots_unmasked          $4..7 = $0..3
    4. copy_slot_unmasked             $8 = $7
    5. mix_3_floats                   $0..2 = mix($0..2, $3..5, $6..8)
)");
}

DEF_TEST(RasterPipelineBuilderAutomaticStackRewinding, r) {
    using BuilderOp = SkSL::RP::BuilderOp;

    SkSL::RP::Builder builder;
    builder.push_literal_i(1);
    builder.push_duplicates(2000);
    builder.unary_op(BuilderOp::abs_int, 1);  // perform work so the program isn't eliminated
    builder.discard_stack(2001);
    std::unique_ptr<SkSL::RP::Program> program = builder.finish(/*numValueSlots=*/0,
                                                                /*numUniformSlots=*/0);
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
