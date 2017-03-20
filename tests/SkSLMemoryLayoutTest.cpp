/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLContext.h"
#include "SkSLMemoryLayout.h"

#include "Test.h"

#if SK_SUPPORT_GPU

DEF_TEST(SkSLMemoryLayout140Test, r) {
    SkSL::Context context;
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k140_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fVec2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fVec4_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fIVec2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fIVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fIVec4_Type));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fBVec2_Type));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fBVec3_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fBVec4_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fMat2x2_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fMat2x4_Type));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fMat3x3_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fMat4x2_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fMat4x4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fVec2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fVec4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fIVec2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fIVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fIVec4_Type));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fBVec2_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBVec3_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBVec4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat2x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat2x4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat3x3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat4x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat4x4_Type));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), SkString("a"), context.fVec3_Type.get());
    SkSL::Type s1(SkSL::Position(), SkString("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(s1));

    fields1.emplace_back(SkSL::Modifiers(), SkString("b"), context.fFloat_Type.get());
    SkSL::Type s2(SkSL::Position(), SkString("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(s2));

    fields1.emplace_back(SkSL::Modifiers(), SkString("c"), context.fBool_Type.get());
    SkSL::Type s3(SkSL::Position(), SkString("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), SkString("a"), context.fInt_Type.get());
    SkSL::Type s4(SkSL::Position(), SkString("s4"), fields2);
    REPORTER_ASSERT(r, 16 == layout.size(s4));
    REPORTER_ASSERT(r, 16 == layout.alignment(s4));

    fields2.emplace_back(SkSL::Modifiers(), SkString("b"), context.fVec3_Type.get());
    SkSL::Type s5(SkSL::Position(), SkString("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(s5));

    // arrays
    SkSL::Type array1(SkString("float[4]"), SkSL::Type::kArray_Kind, *context.fFloat_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array1));
    REPORTER_ASSERT(r, 16 == layout.alignment(array1));
    REPORTER_ASSERT(r, 16 == layout.stride(array1));

    SkSL::Type array2(SkString("vec4[4]"), SkSL::Type::kArray_Kind, *context.fVec4_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(array2));
    REPORTER_ASSERT(r, 16 == layout.stride(array2));
}

DEF_TEST(SkSLMemoryLayout430Test, r) {
    SkSL::Context context;
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::k430_Standard);

    // basic types
    REPORTER_ASSERT(r,  4 == layout.size(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fVec2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fVec4_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fIVec2_Type));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fIVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fIVec4_Type));
    REPORTER_ASSERT(r,  1 == layout.size(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.size(*context.fBVec2_Type));
    REPORTER_ASSERT(r,  3 == layout.size(*context.fBVec3_Type));
    REPORTER_ASSERT(r,  4 == layout.size(*context.fBVec4_Type));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fMat2x2_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fMat2x4_Type));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fMat3x3_Type));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fMat4x2_Type));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fMat4x4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fFloat_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fVec2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fVec4_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fInt_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fIVec2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fIVec3_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fIVec4_Type));
    REPORTER_ASSERT(r,  1 == layout.alignment(*context.fBool_Type));
    REPORTER_ASSERT(r,  2 == layout.alignment(*context.fBVec2_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBVec3_Type));
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fBVec4_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fMat2x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat2x4_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat3x3_Type));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fMat4x2_Type));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fMat4x4_Type));

    // struct 1
    std::vector<SkSL::Type::Field> fields1;
    fields1.emplace_back(SkSL::Modifiers(), SkString("a"), context.fVec3_Type.get());
    SkSL::Type s1(SkSL::Position(), SkString("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(s1));

    fields1.emplace_back(SkSL::Modifiers(), SkString("b"), context.fFloat_Type.get());
    SkSL::Type s2(SkSL::Position(), SkString("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(s2));

    fields1.emplace_back(SkSL::Modifiers(), SkString("c"), context.fBool_Type.get());
    SkSL::Type s3(SkSL::Position(), SkString("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(s3));

    // struct 2
    std::vector<SkSL::Type::Field> fields2;
    fields2.emplace_back(SkSL::Modifiers(), SkString("a"), context.fInt_Type.get());
    SkSL::Type s4(SkSL::Position(), SkString("s4"), fields2);
    REPORTER_ASSERT(r, 4 == layout.size(s4));
    REPORTER_ASSERT(r, 4 == layout.alignment(s4));

    fields2.emplace_back(SkSL::Modifiers(), SkString("b"), context.fVec3_Type.get());
    SkSL::Type s5(SkSL::Position(), SkString("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(s5));

    // arrays
    SkSL::Type array1(SkString("float[4]"), SkSL::Type::kArray_Kind, *context.fFloat_Type, 4);
    REPORTER_ASSERT(r, 16 == layout.size(array1));
    REPORTER_ASSERT(r, 4 == layout.alignment(array1));
    REPORTER_ASSERT(r, 4 == layout.stride(array1));

    SkSL::Type array2(SkString("vec4[4]"), SkSL::Type::kArray_Kind, *context.fVec4_Type, 4);
    REPORTER_ASSERT(r, 64 == layout.size(array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(array2));
    REPORTER_ASSERT(r, 16 == layout.stride(array2));
}
#endif
