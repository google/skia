/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLType.h"
#include "tests/Test.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

using namespace skia_private;

DEF_TEST(SkSLMemoryLayoutTest_std140, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::k140);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

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
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fAtomicUInt));
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
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // struct 1
    TArray<SkSL::Field> fields1;
    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("a"), context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s1 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s1));

    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("b"), context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> s2 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s2));

    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("c"), context.fTypes.fBool.get());
    std::unique_ptr<SkSL::Type> s3 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(*s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s3));

    // struct 2
    TArray<SkSL::Field> fields2;
    fields2.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("a"), context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> s4 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s4"), fields2);
    REPORTER_ASSERT(r, 16 == layout.size(*s4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s4));

    fields2.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("b"), context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s5 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(*s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s5));

    // arrays
    std::unique_ptr<SkSL::Type> array1 =
            SkSL::Type::MakeArrayType(context, std::string("float[4]"), *context.fTypes.fFloat, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array1));
    REPORTER_ASSERT(r, 16 == layout.stride(*array1));

    std::unique_ptr<SkSL::Type> array2 =
            SkSL::Type::MakeArrayType(context, std::string("float4[4]"), *context.fTypes.fFloat4, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array2));
    REPORTER_ASSERT(r, 16 == layout.stride(*array2));
}

DEF_TEST(SkSLMemoryLayoutTest_std430, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::k430);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

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
    REPORTER_ASSERT(r,  4 == layout.size(*context.fTypes.fAtomicUInt));
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
    REPORTER_ASSERT(r,  4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // struct 1
    TArray<SkSL::Field> fields1;
    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("a"), context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s1 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s1"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s1));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s1));

    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("b"), context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> s2 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s2"), fields1);
    REPORTER_ASSERT(r, 16 == layout.size(*s2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s2));

    fields1.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("c"), context.fTypes.fBool.get());
    std::unique_ptr<SkSL::Type> s3 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s3"), fields1);
    REPORTER_ASSERT(r, 32 == layout.size(*s3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s3));

    // struct 2
    TArray<SkSL::Field> fields2;
    fields2.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("a"), context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> s4 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s4"), fields2);
    REPORTER_ASSERT(r, 4 == layout.size(*s4));
    REPORTER_ASSERT(r, 4 == layout.alignment(*s4));

    fields2.emplace_back(SkSL::Position(), SkSL::Layout(), SkSL::ModifierFlag::kNone,
                         std::string_view("b"), context.fTypes.fFloat3.get());
    std::unique_ptr<SkSL::Type> s5 =
            SkSL::Type::MakeStructType(context, SkSL::Position(), std::string("s5"), fields2);
    REPORTER_ASSERT(r, 32 == layout.size(*s5));
    REPORTER_ASSERT(r, 16 == layout.alignment(*s5));

    // arrays
    std::unique_ptr<SkSL::Type> array1 =
            SkSL::Type::MakeArrayType(context, std::string("float[4]"), *context.fTypes.fFloat, 4);
    REPORTER_ASSERT(r, 16 == layout.size(*array1));
    REPORTER_ASSERT(r, 4 == layout.alignment(*array1));
    REPORTER_ASSERT(r, 4 == layout.stride(*array1));

    std::unique_ptr<SkSL::Type> array2 =
            SkSL::Type::MakeArrayType(context, std::string("float4[4]"), *context.fTypes.fFloat4, 4);
    REPORTER_ASSERT(r, 64 == layout.size(*array2));
    REPORTER_ASSERT(r, 16 == layout.alignment(*array2));
    REPORTER_ASSERT(r, 16 == layout.stride(*array2));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLUniform_Base, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLUniform_Base);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    // The values here are taken from https://www.w3.org/TR/WGSL/#alignment-and-size, table titled
    // "Alignment and size for host-shareable types". WGSL does not have an i16 type, so short and
    // unsigned-short integer types are treated as full-size integers in WGSL.

    // scalars (i32, u32, f32, f16)
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fHalf));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf));

    // vec2<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fHalf2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2));

    // vec3<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fHalf3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3));

    // vec4<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4));

    // mat2x2<f32>, mat2x2<f16>
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x2));

    // mat3x2<f32>, mat3x2<f16>
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x2));

    // mat4x2<f32>, mat4x2<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x2));

    // mat2x3<f32>, mat2x3<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf2x3));

    // mat3x3<f32>, mat3x3<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf3x3));

    // mat4x3<f32>, mat4x3<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf4x3));

    // mat2x4<f32>, mat2x4<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf2x4));

    // mat3x4<f32>, mat3x4<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf3x4));

    // mat4x4<f32>, mat4x4<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf4x4));

    // atomic<u32>
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fAtomicUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // bool is not a host-shareable type and returns 0 for WGSL.
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool4));

    // Arrays
    // array<f32, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float[4]", *context.fTypes.fFloat, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<f16, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "half[4]", *context.fTypes.fHalf, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec2<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float2[4]", *context.fTypes.fFloat2, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float3[4]", *context.fTypes.fFloat3, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec4<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float4[4]", *context.fTypes.fFloat4, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<mat3x3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "mat3[4]", *context.fTypes.fFloat3x3, 4);
        REPORTER_ASSERT(r, 192 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 48 == layout.stride(*array));
    }

    // Structs A and B from example in https://www.w3.org/TR/WGSL/#structure-member-layout, with
    // offsets adjusted for uniform address space constraints.
    //
    // struct A {        //            align(roundUp(16, 8))  size(roundUp(16, 24))
    //     u: f32,       // offset(0)  align(4)               size(4)
    //     v: f32,       // offset(4)  align(4)               size(4)
    //     w: vec2<f32>, // offset(8)  align(8)               size(8)
    //     x: f32        // offset(16) align(4)               size(4)
    //     // padding    // offset(20)                        size(12)
    // }
    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("u"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("v"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("w"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("x"),
                        context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> structA = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("A"), std::move(fields));
    REPORTER_ASSERT(r, 32 == layout.size(*structA));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structA));
    fields = {};

    // struct B {          //             align(16) size(208)
    //     a: vec2<f32>,   // offset(0)   align(8)  size(8)
    //     // padding      // offset(8)             size(8)
    //     b: vec3<f32>,   // offset(16)  align(16) size(12)
    //     c: f32,         // offset(28)  align(4)  size(4)
    //     d: f32,         // offset(32)  align(4)  size(4)
    //     // padding      // offset(36)            size(12)
    //     e: A,           // offset(48)  align(16) size(32)
    //     f: vec3<f32>,   // offset(80)  align(16) size(12)
    //     // padding      // offset(92)            size(4)
    //     g: array<A, 3>, // offset(96)  align(16) size(96)
    //     h: i32          // offset(192) align(4)  size(4)
    //     // padding      // offset(196)           size(12)
    // }
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("a"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("b"),
                        context.fTypes.fFloat3.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("c"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("d"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("e"),
                        structA.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("f"),
                        context.fTypes.fFloat3.get());
    auto array = SkSL::Type::MakeArrayType(context, "A[3]", *structA, 3);
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("g"),
                        array.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("h"),
                        context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> structB = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("B"), std::move(fields));
    REPORTER_ASSERT(r, 208 == layout.size(*structB));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structB));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLUniform_EnableF16, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLUniform_EnableF16);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    // The values here are taken from https://www.w3.org/TR/WGSL/#alignment-and-size, table titled
    // "Alignment and size for host-shareable types". WGSL does not have an i16 type, so short and
    // unsigned-short integer types are treated as full-size integers in WGSL.

    // scalars (i32, u32, f32, f16)
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 2 == layout.size(*context.fTypes.fHalf));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 2 == layout.alignment(*context.fTypes.fHalf));

    // vec2<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fHalf2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf2));

    // vec3<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r,  6 == layout.size(*context.fTypes.fHalf3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fHalf3));

    // vec4<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fHalf4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fHalf4));

    // mat2x2<f32>, mat2x2<f16>
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf2x2));

    // mat3x2<f32>, mat3x2<f16>
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf3x2));

    // mat4x2<f32>, mat4x2<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf4x2));

    // mat2x3<f32>, mat2x3<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x3));

    // mat3x3<f32>, mat3x3<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x3));

    // mat4x3<f32>, mat4x3<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x3));

    // mat2x4<f32>, mat2x4<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x4));

    // mat3x4<f32>, mat3x4<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x4));

    // mat4x4<f32>, mat4x4<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x4));

    // atomic<u32>
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fAtomicUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // bool is not a host-shareable type and returns 0 for WGSL.
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool4));

    // Arrays
    // array<f32, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float[4]", *context.fTypes.fFloat, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<f16, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "half[4]", *context.fTypes.fHalf, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec2<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float2[4]", *context.fTypes.fFloat2, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float3[4]", *context.fTypes.fFloat3, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec4<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float4[4]", *context.fTypes.fFloat4, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<mat3x3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "mat3[4]", *context.fTypes.fFloat3x3, 4);
        REPORTER_ASSERT(r, 192 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 48 == layout.stride(*array));
    }

    // Structs A and B from example in https://www.w3.org/TR/WGSL/#structure-member-layout, with
    // offsets adjusted for uniform address space constraints.
    //
    // struct A {        //            align(roundUp(16, 8))  size(roundUp(16, 24))
    //     u: f32,       // offset(0)  align(4)               size(4)
    //     v: f32,       // offset(4)  align(4)               size(4)
    //     w: vec2<f32>, // offset(8)  align(8)               size(8)
    //     x: f32        // offset(16) align(4)               size(4)
    //     // padding    // offset(20)                        size(12)
    // }
    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("u"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("v"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("w"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("x"),
                        context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> structA = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("A"), std::move(fields));
    REPORTER_ASSERT(r, 32 == layout.size(*structA));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structA));
    fields = {};

    // struct B {          //             align(16) size(208)
    //     a: vec2<f32>,   // offset(0)   align(8)  size(8)
    //     // padding      // offset(8)             size(8)
    //     b: vec3<f32>,   // offset(16)  align(16) size(12)
    //     c: f32,         // offset(28)  align(4)  size(4)
    //     d: f32,         // offset(32)  align(4)  size(4)
    //     // padding      // offset(36)            size(12)
    //     e: A,           // offset(48)  align(16) size(32)
    //     f: vec3<f32>,   // offset(80)  align(16) size(12)
    //     // padding      // offset(92)            size(4)
    //     g: array<A, 3>, // offset(96)  align(16) size(96)
    //     h: i32          // offset(192) align(4)  size(4)
    //     // padding      // offset(196)           size(12)
    // }
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("a"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("b"),
                        context.fTypes.fFloat3.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("c"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("d"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("e"),
                        structA.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("f"),
                        context.fTypes.fFloat3.get());
    auto array = SkSL::Type::MakeArrayType(context, "A[3]", *structA, 3);
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("g"),
                        array.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("h"),
                        context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> structB = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("B"), std::move(fields));
    REPORTER_ASSERT(r, 208 == layout.size(*structB));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structB));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLStorage_Base, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLStorage_Base);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    // The values here are taken from https://www.w3.org/TR/WGSL/#alignment-and-size, table titled
    // "Alignment and size for host-shareable types".

    // scalars (i32, u32, f32, f16)
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fHalf));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf));

    // vec2<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fHalf2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2));

    // vec3<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fHalf3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3));

    // vec4<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4));

    // mat2x2<f32>, mat2x2<f16>
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x2));

    // mat3x2<f32>, mat3x2<f16>
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x2));

    // mat4x2<f32>, mat4x2<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x2));

    // mat2x3<f32>, mat2x3<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf2x3));

    // mat3x3<f32>, mat3x3<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf3x3));

    // mat4x3<f32>, mat4x3<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf4x3));

    // mat2x4<f32>, mat2x4<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf2x4));

    // mat3x4<f32>, mat3x4<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf3x4));

    // mat4x4<f32>, mat4x4<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fHalf4x4));

    // atomic<u32>
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fAtomicUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // bool is not a host-shareable type and returns 0 for WGSL.
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool4));

    // Arrays
    // array<f32, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float[4]", *context.fTypes.fFloat, 4);
        REPORTER_ASSERT(r, 16 == layout.size(*array));
        REPORTER_ASSERT(r, 4 == layout.alignment(*array));
        REPORTER_ASSERT(r, 4 == layout.stride(*array));
    }
    // array<f16, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "half[4]", *context.fTypes.fHalf, 4);
        REPORTER_ASSERT(r, 16 == layout.size(*array));
        REPORTER_ASSERT(r, 4 == layout.alignment(*array));
        REPORTER_ASSERT(r, 4 == layout.stride(*array));
    }
    // array<vec2<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float2[4]", *context.fTypes.fFloat2, 4);
        REPORTER_ASSERT(r, 32 == layout.size(*array));
        REPORTER_ASSERT(r, 8 == layout.alignment(*array));
        REPORTER_ASSERT(r, 8 == layout.stride(*array));
    }
    // array<vec3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float3[4]", *context.fTypes.fFloat3, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec4<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float4[4]", *context.fTypes.fFloat4, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<mat3x3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "mat3[4]", *context.fTypes.fFloat3x3, 4);
        REPORTER_ASSERT(r, 192 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 48 == layout.stride(*array));
    }

    // Structs A and B from example in https://www.w3.org/TR/WGSL/#structure-member-layout
    //
    // struct A {        //            align(8)               size(24)
    //     u: f32,       // offset(0)  align(4)               size(4)
    //     v: f32,       // offset(4)  align(4)               size(4)
    //     w: vec2<f32>, // offset(8)  align(8)               size(8)
    //     x: f32        // offset(16) align(4)               size(4)
    //     // padding    // offset(20)                        size(4)
    // }
    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("u"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("v"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("w"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("x"),
                        context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> structA = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("A"), std::move(fields));
    REPORTER_ASSERT(r, 24 == layout.size(*structA));
    REPORTER_ASSERT(r, 8 == layout.alignment(*structA));
    fields = {};

    // struct B {          //             align(16) size(160)
    //     a: vec2<f32>,   // offset(0)   align(8)  size(8)
    //     // padding      // offset(8)             size(8)
    //     b: vec3<f32>,   // offset(16)  align(16) size(12)
    //     c: f32,         // offset(28)  align(4)  size(4)
    //     d: f32,         // offset(32)  align(4)  size(4)
    //     // padding      // offset(36)            size(4)
    //     e: A,           // offset(40)  align(8)  size(24)
    //     f: vec3<f32>,   // offset(64)  align(16) size(12)
    //     // padding      // offset(76)            size(4)
    //     g: array<A, 3>, // offset(80)  align(16) size(72)
    //     h: i32          // offset(152) align(4)  size(4)
    //     // padding      // offset(156)           size(4)
    // }
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("a"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("b"),
                        context.fTypes.fFloat3.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("c"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("d"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("e"),
                        structA.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("f"),
                        context.fTypes.fFloat3.get());
    auto array = SkSL::Type::MakeArrayType(context, "A[3]", *structA, 3);
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("g"),
                        array.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("h"),
                        context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> structB = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("B"), std::move(fields));
    REPORTER_ASSERT(r, 160 == layout.size(*structB));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structB));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLStorage_EnableF16, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLStorage_EnableF16);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    // The values here are taken from https://www.w3.org/TR/WGSL/#alignment-and-size, table titled
    // "Alignment and size for host-shareable types".

    // scalars (i32, u32, f32, f16)
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 2 == layout.size(*context.fTypes.fHalf));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, 2 == layout.alignment(*context.fTypes.fHalf));

    // vec2<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fHalf2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf2));

    // vec3<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r,  6 == layout.size(*context.fTypes.fHalf3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fHalf3));

    // vec4<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  8 == layout.size(*context.fTypes.fHalf4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r,  8 == layout.alignment(*context.fTypes.fHalf4));

    // mat2x2<f32>, mat2x2<f16>
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 8 == layout.size(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf2x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf2x2));

    // mat3x2<f32>, mat3x2<f16>
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 12 == layout.size(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf3x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf3x2));

    // mat4x2<f32>, mat4x2<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fHalf4x2));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, 4 == layout.stride(*context.fTypes.fHalf4x2));

    // mat2x3<f32>, mat2x3<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x3));

    // mat3x3<f32>, mat3x3<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x3));

    // mat4x3<f32>, mat4x3<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x3));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x3));

    // mat2x4<f32>, mat2x4<f16>
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 16 == layout.size(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf2x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf2x4));

    // mat3x4<f32>, mat3x4<f16>
    REPORTER_ASSERT(r, 48 == layout.size(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 24 == layout.size(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf3x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf3x4));

    // mat4x4<f32>, mat4x4<f16>
    REPORTER_ASSERT(r, 64 == layout.size(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 32 == layout.size(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.alignment(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 8 == layout.alignment(*context.fTypes.fHalf4x4));
    REPORTER_ASSERT(r, 16 == layout.stride(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, 8 == layout.stride(*context.fTypes.fHalf4x4));

    // atomic<u32>
    REPORTER_ASSERT(r, 4 == layout.size(*context.fTypes.fAtomicUInt));
    REPORTER_ASSERT(r, 4 == layout.alignment(*context.fTypes.fAtomicUInt));

    // bool is not a host-shareable type and returns 0 for WGSL.
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool2));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool3));
    REPORTER_ASSERT(r, 0 == layout.size(*context.fTypes.fBool4));

    // Arrays
    // array<f32, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float[4]", *context.fTypes.fFloat, 4);
        REPORTER_ASSERT(r, 16 == layout.size(*array));
        REPORTER_ASSERT(r, 4 == layout.alignment(*array));
        REPORTER_ASSERT(r, 4 == layout.stride(*array));
    }
    // array<f16, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "half[4]", *context.fTypes.fHalf, 4);
        REPORTER_ASSERT(r, 8 == layout.size(*array));
        REPORTER_ASSERT(r, 2 == layout.alignment(*array));
        REPORTER_ASSERT(r, 2 == layout.stride(*array));
    }
    // array<vec2<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float2[4]", *context.fTypes.fFloat2, 4);
        REPORTER_ASSERT(r, 32 == layout.size(*array));
        REPORTER_ASSERT(r, 8 == layout.alignment(*array));
        REPORTER_ASSERT(r, 8 == layout.stride(*array));
    }
    // array<vec3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float3[4]", *context.fTypes.fFloat3, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<vec4<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "float4[4]", *context.fTypes.fFloat4, 4);
        REPORTER_ASSERT(r, 64 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 16 == layout.stride(*array));
    }
    // array<mat3x3<f32>, 4>
    {
        auto array = SkSL::Type::MakeArrayType(context, "mat3[4]", *context.fTypes.fFloat3x3, 4);
        REPORTER_ASSERT(r, 192 == layout.size(*array));
        REPORTER_ASSERT(r, 16 == layout.alignment(*array));
        REPORTER_ASSERT(r, 48 == layout.stride(*array));
    }

    // Structs A and B from example in https://www.w3.org/TR/WGSL/#structure-member-layout
    //
    // struct A {        //            align(8)               size(24)
    //     u: f32,       // offset(0)  align(4)               size(4)
    //     v: f32,       // offset(4)  align(4)               size(4)
    //     w: vec2<f32>, // offset(8)  align(8)               size(8)
    //     x: f32        // offset(16) align(4)               size(4)
    //     // padding    // offset(20)                        size(4)
    // }
    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("u"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("v"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("w"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("x"),
                        context.fTypes.fFloat.get());
    std::unique_ptr<SkSL::Type> structA = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("A"), std::move(fields));
    REPORTER_ASSERT(r, 24 == layout.size(*structA));
    REPORTER_ASSERT(r, 8 == layout.alignment(*structA));
    fields = {};

    // struct B {          //             align(16) size(160)
    //     a: vec2<f32>,   // offset(0)   align(8)  size(8)
    //     // padding      // offset(8)             size(8)
    //     b: vec3<f32>,   // offset(16)  align(16) size(12)
    //     c: f32,         // offset(28)  align(4)  size(4)
    //     d: f32,         // offset(32)  align(4)  size(4)
    //     // padding      // offset(36)            size(4)
    //     e: A,           // offset(40)  align(8)  size(24)
    //     f: vec3<f32>,   // offset(64)  align(16) size(12)
    //     // padding      // offset(76)            size(4)
    //     g: array<A, 3>, // offset(80)  align(16) size(72)
    //     h: i32          // offset(152) align(4)  size(4)
    //     // padding      // offset(156)           size(4)
    // }
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("a"),
                        context.fTypes.fFloat2.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("b"),
                        context.fTypes.fFloat3.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("c"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("d"),
                        context.fTypes.fFloat.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("e"),
                        structA.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("f"),
                        context.fTypes.fFloat3.get());
    auto array = SkSL::Type::MakeArrayType(context, "A[3]", *structA, 3);
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("g"),
                        array.get());
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("h"),
                        context.fTypes.fInt.get());
    std::unique_ptr<SkSL::Type> structB = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("B"), std::move(fields));
    REPORTER_ASSERT(r, 160 == layout.size(*structB));
    REPORTER_ASSERT(r, 16 == layout.alignment(*structB));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLUnsupportedTypes, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    auto testArray = SkSL::Type::MakeArrayType(context, "bool[3]", *context.fTypes.fBool, 3);

    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("foo"),
                        testArray.get());
    auto testStruct = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("Test"), std::move(fields));

    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLUniform_EnableF16);
    REPORTER_ASSERT(r, !layout.isSupported(*context.fTypes.fBool));
    REPORTER_ASSERT(r, !layout.isSupported(*context.fTypes.fBool2));
    REPORTER_ASSERT(r, !layout.isSupported(*context.fTypes.fBool3));
    REPORTER_ASSERT(r, !layout.isSupported(*context.fTypes.fBool4));
    REPORTER_ASSERT(r, !layout.isSupported(*testArray));
    REPORTER_ASSERT(r, !layout.isSupported(*testStruct));
}

DEF_TEST(SkSLMemoryLayoutTest_WGSLSupportedTypes, r) {
    SkSL::TestingOnly_AbortErrorReporter errors;
    SkSL::BuiltinTypes types;
    SkSL::Context context(types, errors);
    SkSL::ProgramConfig config = {};
    context.fConfig = &config;

    auto testArray = SkSL::Type::MakeArrayType(context, "float[3]", *context.fTypes.fFloat, 3);

    TArray<SkSL::Field> fields;
    fields.emplace_back(SkSL::Position(),
                        SkSL::Layout(),
                        SkSL::ModifierFlag::kNone,
                        std::string_view("foo"),
                        testArray.get());
    auto testStruct = SkSL::Type::MakeStructType(
            context, SkSL::Position(), std::string_view("Test"), std::move(fields));

    SkSL::MemoryLayout layout(SkSL::MemoryLayout::Standard::kWGSLUniform_EnableF16);

    // scalars (i32, u32, f32, f16)
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fInt));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUInt));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fShort));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUShort));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf));

    // vec2<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fInt2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUInt2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fShort2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUShort2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf2));

    // vec3<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fInt3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUInt3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fShort3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUShort3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf3));

    // vec4<T>, T: i32, u32, f32, f16
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fInt4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUInt4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fShort4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fUShort4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf4));

    // mat2x2<f32>, mat2x2<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat2x2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf2x2));

    // mat3x2<f32>, mat3x2<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat3x2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf3x2));

    // mat4x2<f32>, mat4x2<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat4x2));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf4x2));

    // mat2x3<f32>, mat2x3<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat2x3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf2x3));

    // mat3x3<f32>, mat3x3<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat3x3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf3x3));

    // mat4x3<f32>, mat4x3<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat4x3));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf4x3));

    // mat2x4<f32>, mat2x4<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat2x4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf2x4));

    // mat3x4<f32>, mat3x4<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat3x4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf3x4));

    // mat4x4<f32>, mat4x4<f16>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fFloat4x4));
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fHalf4x4));

    // atomic<u32>
    REPORTER_ASSERT(r, layout.isSupported(*context.fTypes.fAtomicUInt));

    // arrays and structs
    REPORTER_ASSERT(r, layout.isSupported(*testArray));
    REPORTER_ASSERT(r, layout.isSupported(*testStruct));
}
