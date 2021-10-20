/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLMemoryLayout.h"

#include "tests/Test.h"

DEF_TEST(SkSLMemoryLayout140Test, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    GrShaderCaps caps;
    SkSL::Mangler mangler;
    SkSL::Context context(errors, caps, mangler);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k140_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fBool4));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fTypes.fBool));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fTypes.fBool2));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fBool3));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fBool4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("a"),
                         context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s1 = SkSL::Type::MakeStructType(-1, SkSL::String("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s1));

    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("b"), context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> s2 = SkSL::Type::MakeStructType(-1, SkSL::String("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s2));

    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("c"), context.fTypes.fBool.get());
    std::unique_ptr<SkSL::Type> s3 = SkSL::Type::MakeStructType(-1, SkSL::String("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(*s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), skstd::string_view("a"), context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> s4 = SkSL::Type::MakeStructType(-1, SkSL::String("s4"), fields2);
    REPORTER_ASSERT(r, 16 == layout.size(*s4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s4));

    fields2.emplace_back(SkSL::Modifiers(), skstd::string_view("b"),
                         context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s5 = SkSL::Type::MakeStructType(-1, SkSL::String("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(*s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s5));

    // arrays
    std::unique_ptr<SkSL::Type> array1 =
            SkSL::Type::MakeArrayType(SkSL::String("float[4]"), *context.fTypes.fFloat, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array1));
    REPORTER_ASSERT(r, 16 == layout.stride(*array1));

    std::unique_ptr<SkSL::Type> array2 =
            SkSL::Type::MakeArrayType(SkSL::String("float4[4]"), *context.fTypes.fFloat4, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array2));
    REPORTER_ASSERT(r, 16 == layout.stride(*array2));
}

DEF_TEST(SkSLMemoryLayout430Test, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    GrShaderCaps caps;
    SkSL::Mangler mangler;
    SkSL::Context context(errors, caps, mangler);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k430_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fBool4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fTypes.fBool));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fTypes.fBool2));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fBool3));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fBool4));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("a"),
                         context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s1 = SkSL::Type::MakeStructType(-1, SkSL::String("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s1));

    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("b"), context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> s2 = SkSL::Type::MakeStructType(-1, SkSL::String("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s2));

    fields1.emplace_back(SkSL::Modifiers(), skstd::string_view("c"), context.fTypes.fBool.get());
    std::unique_ptr<SkSL::Type> s3 = SkSL::Type::MakeStructType(-1, SkSL::String("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(*s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), skstd::string_view("a"), context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> s4 = SkSL::Type::MakeStructType(-1, SkSL::String("s4"), fields2);
    REPORTER_ASSERT(r, 4 == layout.size(*s4));
    REPORTER_ASSERT(r, 4 == layout.alignment(*s4));

    fields2.emplace_back(SkSL::Modifiers(), skstd::string_view("b"),
                         context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s5 = SkSL::Type::MakeStructType(-1, SkSL::String("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(*s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s5));

    // arrays
    std::unique_ptr<SkSL::Type> array1 =
            SkSL::Type::MakeArrayType(SkSL::String("float[4]"), *context.fTypes.fFloat, 4);
    REPORTER_ASSERT(r, 16 == layout.size(*array1));
    REPORTER_ASSERT(r, 4 == layout.alignment(*array1));
    REPORTER_ASSERT(r, 4 == layout.stride(*array1));

    std::unique_ptr<SkSL::Type> array2 =
            SkSL::Type::MakeArrayType(SkSL::String("float4[4]"), *context.fTypes.fFloat4, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array2));
    REPORTER_ASSERT(r, 16 == layout.stride(*array2));
}
