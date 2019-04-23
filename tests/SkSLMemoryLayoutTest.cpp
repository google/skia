/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMemoryLayout.h"

#include "tests/Test.h"

DEF_TEST(SkSLMemoryLayout140Test, r) {
    SkSL::Context context;
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k140_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fFloat2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fFloat3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fFloat4_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fInt2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fInt3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fInt4_Type));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fBool2_Type));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fBool3_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fBool4_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fFloat2x2_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fFloat2x4_Type));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fFloat3x3_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fFloat4x2_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fFloat4x4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fFloat2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fInt2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fInt3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fInt4_Type));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fBool2_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBool3_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBool4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat2x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat2x4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat3x3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat4x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat4x4_Type));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("a"), context.fFloat3_Type.get());
    SkSL::Type s1(-1, SkSL::String("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(s1));

    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("b"), context.fFloat_Type.get());
    SkSL::Type s2(-1, SkSL::String("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(s2));

    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("c"), context.fBool_Type.get());
    SkSL::Type s3(-1, SkSL::String("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("a"), context.fInt_Type.get());
    SkSL::Type s4(-1, SkSL::String("s4"), fields2);
    REPORTER_ASSERT(r, 16 == layout.size(s4));
    REPORTER_ASSERT(r, 16 == layout.alignment(s4));

    fields2.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("b"), context.fFloat3_Type.get());
    SkSL::Type s5(-1, SkSL::String("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(s5));

    // arrays
    SkSL::Type array1(SkSL::String("float[4]"), SkSL::Type::kArray_Kind, *context.fFloat_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array1));
    REPORTER_ASSERT(r, 16 == layout.alignment(array1));
    REPORTER_ASSERT(r, 16 == layout.stride(array1));

    SkSL::Type array2(SkSL::String("float4[4]"), SkSL::Type::kArray_Kind, *context.fFloat4_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(array2));
    REPORTER_ASSERT(r, 16 == layout.stride(array2));
}

DEF_TEST(SkSLMemoryLayout430Test, r) {
    SkSL::Context context;
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k430_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fFloat2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fFloat3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fFloat4_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fInt2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fInt3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fInt4_Type));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fBool2_Type));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fBool3_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fBool4_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fFloat2x2_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fFloat2x4_Type));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fFloat3x3_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fFloat4x2_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fFloat4x4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fFloat2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fInt2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fInt3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fInt4_Type));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fBool2_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBool3_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBool4_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fFloat2x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat2x4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat3x3_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fFloat4x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fFloat4x4_Type));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("a"), context.fFloat3_Type.get());
    SkSL::Type s1(-1, SkSL::String("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(s1));

    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("b"), context.fFloat_Type.get());
    SkSL::Type s2(-1, SkSL::String("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(s2));

    fields1.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("c"), context.fBool_Type.get());
    SkSL::Type s3(-1, SkSL::String("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("a"), context.fInt_Type.get());
    SkSL::Type s4(-1, SkSL::String("s4"), fields2);
    REPORTER_ASSERT(r, 4 == layout.size(s4));
    REPORTER_ASSERT(r, 4 == layout.alignment(s4));

    fields2.emplace_back(SkSL::Modifiers(), SkSL::StringFragment("b"), context.fFloat3_Type.get());
    SkSL::Type s5(-1, SkSL::String("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(s5));

    // arrays
    SkSL::Type array1(SkSL::String("float[4]"), SkSL::Type::kArray_Kind, *context.fFloat_Type, 4);
    REPORTER_ASSERT(r, 16 == layout.size(array1));
    REPORTER_ASSERT(r, 4 == layout.alignment(array1));
    REPORTER_ASSERT(r, 4 == layout.stride(array1));

    SkSL::Type array2(SkSL::String("float4[4]"), SkSL::Type::kArray_Kind, *context.fFloat4_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(array2));
    REPORTER_ASSERT(r, 16 == layout.stride(array2));
}
