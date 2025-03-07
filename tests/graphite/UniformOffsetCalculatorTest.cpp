/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

namespace {

// Used to test the exact alignment and size of an individual type. Returns the alignment and size
// as a pair.
struct AlignmentAndSize {
    int alignment;
    int size;
};
static AlignmentAndSize calculate_alignment_and_size(Layout layout,
                                                     SkSLType type,
                                                     int arrayCount = Uniform::kNonArray) {
    // Set the start offset at 1 to force alignment.
    constexpr int kStart = 1;
    auto calc = UniformOffsetCalculator::ForTopLevel(layout, kStart);
    int alignment = calc.advanceOffset(type, arrayCount);
    return {alignment, calc.size() - alignment};
}

static AlignmentAndSize calculate_struct_alignment_and_size(
        Layout layout,
        std::initializer_list<SkSLType> fields,
        int arrayCount = Uniform::kNonArray) {
    // Set the start offset at 1 to force alignment.
    constexpr int kStart = 1;
    auto outer = UniformOffsetCalculator::ForTopLevel(layout, kStart);

    auto substruct = UniformOffsetCalculator::ForStruct(layout);
    for (SkSLType f : fields) {
        substruct.advanceOffset(f);
    }

    int alignment = outer.advanceStruct(substruct, arrayCount);
    SkASSERT(alignment == substruct.requiredAlignment());
    return {alignment, outer.size() - alignment};
}

#define EXPECT(type, expectedAlignment, expectedSize)                                \
    do {                                                                             \
        auto [alignment, size] = calculate_alignment_and_size(kLayout, type);        \
        REPORTER_ASSERT(r,                                                           \
                        alignment == expectedAlignment,                              \
                        "incorrect alignment for type '%s': expected %d, found %d",  \
                        SkSLTypeString(type),                                        \
                        expectedAlignment,                                           \
                        alignment);                                                  \
        REPORTER_ASSERT(r,                                                           \
                        size == expectedSize,                                        \
                        "incorrect size for type '%s': expected %d, found %d",       \
                        SkSLTypeString(type),                                        \
                        expectedSize,                                                \
                        size);                                                       \
    } while (0)

#define EXPECT_ARRAY(type, expectedAlignment, expectedStride, expectedSize)           \
    do {                                                                              \
        auto [alignment, size] = calculate_alignment_and_size(kLayout, type, kCount); \
        int stride = size / kCount;                                                   \
        REPORTER_ASSERT(r,                                                            \
                        alignment == expectedAlignment,                               \
                        "incorrect alignment for type '%s': expected %d, found %d",   \
                        SkSLTypeString(type),                                         \
                        expectedAlignment,                                            \
                        alignment);                                                   \
        REPORTER_ASSERT(r,                                                            \
                        size == expectedSize,                                         \
                        "incorrect size for type '%s': expected %d, found %d",        \
                        SkSLTypeString(type),                                         \
                        expectedSize,                                                 \
                        size);                                                        \
        REPORTER_ASSERT(r,                                                            \
                        stride == expectedStride,                                     \
                        "incorrect stride for type '%s': expected %d, found %d",      \
                        SkSLTypeString(type),                                         \
                        expectedStride,                                               \
                        stride);                                                      \
    } while (0)

#define EXPECT_STRUCT(expectedAlignment, expectedSize, ...)                      \
    do {                                                                         \
        auto [alignment, size] = calculate_struct_alignment_and_size(            \
                                         kLayout, {__VA_ARGS__});                \
        REPORTER_ASSERT(r,                                                       \
                        alignment == expectedAlignment,                          \
                        "incorrect alignment for struct: expected %d, found %d", \
                        expectedAlignment,                                       \
                        alignment);                                              \
        REPORTER_ASSERT(r,                                                       \
                        size == expectedSize,                                    \
                        "incorrect size for struct: expected %d, found %d",      \
                        expectedSize,                                            \
                        size);                                                   \
        REPORTER_ASSERT(r,                                                       \
                        size % alignment == 0,                                   \
                        "struct size must be a multiple of alignment");          \
    } while (0)

#define EXPECT_STRUCT_ARRAY(expectedAlignment, expectedStride, ...)               \
    do {                                                                          \
        auto [alignment, size] = calculate_struct_alignment_and_size(             \
                                        kLayout, {__VA_ARGS__}, kCount);          \
        int stride = size / kCount;                                               \
        REPORTER_ASSERT(r,                                                        \
                        alignment == expectedAlignment,                           \
                        "incorrect alignment for struct: expected %d, found %d",  \
                        expectedAlignment,                                        \
                        alignment);                                               \
        REPORTER_ASSERT(r,                                                        \
                        size == kCount * expectedStride,                          \
                        "incorrect size for struct array: expected %d, found %d", \
                        kCount * expectedStride,                                  \
                        size);                                                    \
        REPORTER_ASSERT(r,                                                        \
                        stride == expectedStride,                                 \
                        "incorrect stride for struct: expected %d, found %d",     \
                        expectedStride,                                           \
                        stride);                                                  \
        REPORTER_ASSERT(r,                                                        \
                        stride % alignment == 0,                                  \
                        "struct stride must be a multiple of alignment");         \
    } while (0)

DEF_GRAPHITE_TEST(UniformOffsetCalculatorMetalBasicTypesTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kMetal;

    // scalars: int, float, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf,   /*alignment=*/2, /*size=*/2);

    // int2, float2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/4, /*size=*/4);

    // int3, float3, half3 (unlike std430, size is also rounded up)
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/8,  /*size=*/8);

    // int4, float4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf4,   /*alignment=*/8,  /*size=*/8);

    // float2x2, half2x2
    EXPECT(SkSLType::kFloat2x2, /*alignment=*/8, /*size=*/16);
    EXPECT(SkSLType::kHalf2x2,  /*alignment=*/4, /*size=*/8);

    // float3x3, half3x3
    EXPECT(SkSLType::kFloat3x3, /*alignment=*/16, /*size=*/48);
    EXPECT(SkSLType::kHalf3x3,  /*alignment=*/8,  /*size=*/24);

    // float4x4, half4x4
    EXPECT(SkSLType::kFloat4x4, /*alignment=*/16, /*size=*/64);
    EXPECT(SkSLType::kHalf4x4,  /*alignment=*/8,  /*size=*/32);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorMetalArrayTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kMetal;
    constexpr int kCount = 3;

    // int[3], float[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/2, /*stride=*/2, /*size=*/6);

    // int2[3], float2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/4, /*stride=*/4, /*size=*/12);

    // int3[3], float3[3], half3[3]
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/8,  /*stride=*/8,  /*size=*/24);

    // int4[3], float4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf4,  /*alignment=*/8,  /*stride=*/8,  /*size=*/24);

    // float2x2[3], half2x2[3]
    EXPECT_ARRAY(SkSLType::kFloat2x2, /*alignment=*/8, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf2x2,  /*alignment=*/4, /*stride=*/8,  /*size=*/24);

    // float3x3[3], half3x3[3]
    EXPECT_ARRAY(SkSLType::kFloat3x3, /*alignment=*/16, /*stride=*/48, /*size=*/144);
    EXPECT_ARRAY(SkSLType::kHalf3x3,  /*alignment=*/8,  /*stride=*/24, /*size=*/72);

    // float4x4[3], half4x4[3]
    EXPECT_ARRAY(SkSLType::kFloat4x4, /*alignment=*/16, /*stride=*/64, /*size=*/192);
    EXPECT_ARRAY(SkSLType::kHalf4x4,  /*alignment=*/8,  /*stride=*/32, /*size=*/96);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorMetalStructTest, r, CtsEnforcement::kNever) {
    constexpr Layout kLayout = Layout::kMetal;
    constexpr int    kCount  = 3;

    EXPECT_STRUCT(/*alignment=*/16, /*size=*/32, /*fields=*/SkSLType::kFloat4,
                                                            SkSLType::kFloat3);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/32, /*fields=*/SkSLType::kFloat3,
                                                            SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/8,  /*size=*/16, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat2);
    EXPECT_STRUCT(/*alignment=*/4,  /*size=*/4,  /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/4,  /*size=*/12, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat,
                                                            SkSLType::kInt);
    EXPECT_STRUCT(/*alignment=*/4,  /*size=*/8,  /*fields=*/SkSLType::kHalf2,
                                                            SkSLType::kInt);

    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/32, /*fields=*/SkSLType::kFloat4,
                                                                    SkSLType::kFloat3);
    EXPECT_STRUCT_ARRAY(/*alignment=*/8,  /*stride=*/16, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat2);
    EXPECT_STRUCT_ARRAY(/*alignment=*/4,  /*stride=*/4,  /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT_ARRAY(/*alignment=*/4,  /*stride=*/12, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat,
                                                                    SkSLType::kInt);
    EXPECT_STRUCT_ARRAY(/*alignment=*/4,  /*stride=*/8,  /*fields=*/SkSLType::kHalf2,
                                                                    SkSLType::kInt);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd430BasicTypesTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kStd430;

    // scalars: int, float, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf,   /*alignment=*/4, /*size=*/4);

    // int2, float2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/8, /*size=*/8);

    // int3, float3, half3 (size is not rounded up for non-arrays of vec3s)
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/12);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/12);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/16, /*size=*/12);

    // int4, float4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf4,   /*alignment=*/16, /*size=*/16);

    // float2x2, half2x2
    EXPECT(SkSLType::kFloat2x2, /*alignment=*/8, /*size=*/16);
    EXPECT(SkSLType::kHalf2x2,  /*alignment=*/8, /*size=*/16);

    // float3x3, half3x3
    EXPECT(SkSLType::kFloat3x3, /*alignment=*/16, /*size=*/48);
    EXPECT(SkSLType::kHalf3x3,  /*alignment=*/16, /*size=*/48);

    // float4x4, half4x4
    EXPECT(SkSLType::kFloat4x4, /*alignment=*/16, /*size=*/64);
    EXPECT(SkSLType::kHalf4x4,  /*alignment=*/16, /*size=*/64);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd430ArrayTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kStd430;
    constexpr int    kCount  = 3;

    // int[3], float[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/4, /*stride=*/4, /*size=*/12);

    // int2[3], float2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/8, /*stride=*/8, /*size=*/24);

    // int3[3], float3[3], half3[3] (stride is rounded up in arrays)
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int4[3], float4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf4,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // float2x2[3], half2x2[3]
    EXPECT_ARRAY(SkSLType::kFloat2x2, /*alignment=*/8, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf2x2,  /*alignment=*/8, /*stride=*/16, /*size=*/48);

    // float3x3[3], half3x3[3]
    EXPECT_ARRAY(SkSLType::kFloat3x3, /*alignment=*/16, /*stride=*/48, /*size=*/144);
    EXPECT_ARRAY(SkSLType::kHalf3x3,  /*alignment=*/16, /*stride=*/48, /*size=*/144);

    // float4x4[3], half4x4[3]
    EXPECT_ARRAY(SkSLType::kFloat4x4, /*alignment=*/16, /*stride=*/64, /*size=*/192);
    EXPECT_ARRAY(SkSLType::kHalf4x4,  /*alignment=*/16, /*stride=*/64, /*size=*/192);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd430StructTest, r, CtsEnforcement::kNever) {
    constexpr Layout kLayout = Layout::kStd430;
    constexpr int    kCount  = 3;

    EXPECT_STRUCT(/*alignment=*/16, /*size=*/32, /*fields=*/SkSLType::kFloat4,
                                                            SkSLType::kFloat3);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kFloat3,
                                                            SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/8,  /*size=*/16, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat2);
    EXPECT_STRUCT(/*alignment=*/4,  /*size=*/4,  /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/4,  /*size=*/12, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat,
                                                            SkSLType::kInt);
    EXPECT_STRUCT(/*alignment=*/8,  /*size=*/16, /*fields=*/SkSLType::kHalf2,
                                                            SkSLType::kInt);

    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/32, /*fields=*/SkSLType::kFloat4,
                                                                    SkSLType::kFloat3);
    EXPECT_STRUCT_ARRAY(/*alignment=*/8,  /*stride=*/16, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat2);
    EXPECT_STRUCT_ARRAY(/*alignment=*/4,  /*stride=*/4,  /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT_ARRAY(/*alignment=*/4,  /*stride=*/12, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat,
                                                                    SkSLType::kInt);
    EXPECT_STRUCT_ARRAY(/*alignment=*/8,  /*stride=*/16, /*fields=*/SkSLType::kHalf2,
                                                                    SkSLType::kInt);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd140BasicTypesTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kStd140;

    // scalars: int, float, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf,   /*alignment=*/4, /*size=*/4);

    // int2, float2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/8, /*size=*/8);

    // int3, float3, half3 (size is not rounded up for non-arrays of vec3s)
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/12);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/12);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/16, /*size=*/12);

    // int4, float4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf4,   /*alignment=*/16, /*size=*/16);

    // float2x2, half2x2
    EXPECT(SkSLType::kFloat2x2, /*alignment=*/16, /*size=*/32);
    EXPECT(SkSLType::kHalf2x2,  /*alignment=*/16, /*size=*/32);

    // float3x3, half3x3
    EXPECT(SkSLType::kFloat3x3, /*alignment=*/16,  /*size=*/48);
    EXPECT(SkSLType::kHalf3x3,  /*alignment=*/16,  /*size=*/48);

    // float4x4, half4x4
    EXPECT(SkSLType::kFloat4x4, /*alignment=*/16, /*size=*/64);
    EXPECT(SkSLType::kHalf4x4,  /*alignment=*/16, /*size=*/64);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd140ArrayTest, r, CtsEnforcement::kApiLevel_202404) {
    constexpr Layout kLayout = Layout::kStd140;
    constexpr int    kCount  = 3;

    // int[3], float[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int2[3], float2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int3[3], float3[3], half3[3]
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int4[3], float4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf4,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // float2x2[3], half2x2[3]
    EXPECT_ARRAY(SkSLType::kFloat2x2, /*alignment=*/16, /*stride=*/32, /*size=*/96);
    EXPECT_ARRAY(SkSLType::kHalf2x2,  /*alignment=*/16, /*stride=*/32, /*size=*/96);

    // float3x3[3], half3x3[3]
    EXPECT_ARRAY(SkSLType::kFloat3x3, /*alignment=*/16, /*stride=*/48, /*size=*/144);
    EXPECT_ARRAY(SkSLType::kHalf3x3,  /*alignment=*/16, /*stride=*/48, /*size=*/144);

    // float4x4[3], half4x4[3]
    EXPECT_ARRAY(SkSLType::kFloat4x4, /*alignment=*/16, /*stride=*/64, /*size=*/192);
    EXPECT_ARRAY(SkSLType::kHalf4x4,  /*alignment=*/16, /*stride=*/64, /*size=*/192);
}

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd140StructTest, r, CtsEnforcement::kNever) {
    constexpr Layout kLayout = Layout::kStd140;
    constexpr int    kCount  = 3;

    EXPECT_STRUCT(/*alignment=*/16, /*size=*/32, /*fields=*/SkSLType::kFloat4,
                                                            SkSLType::kFloat3);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kFloat3,
                                                            SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat2);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kFloat,
                                                            SkSLType::kFloat,
                                                            SkSLType::kInt);
    EXPECT_STRUCT(/*alignment=*/16, /*size=*/16, /*fields=*/SkSLType::kHalf2,
                                                            SkSLType::kInt);

    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/32, /*fields=*/SkSLType::kFloat4,
                                                                    SkSLType::kFloat3);
    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/16, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat2);
    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/16, /*fields=*/SkSLType::kFloat);
    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/16, /*fields=*/SkSLType::kFloat,
                                                                    SkSLType::kFloat,
                                                                    SkSLType::kInt);
    EXPECT_STRUCT_ARRAY(/*alignment=*/16, /*stride=*/16, /*fields=*/SkSLType::kHalf2,
                                                                    SkSLType::kInt);
}

}  // namespace
