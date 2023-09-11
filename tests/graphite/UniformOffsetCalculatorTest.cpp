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
    size_t alignment;
    size_t size;
};
static AlignmentAndSize calculate_alignment_and_size(Layout layout,
                                                     SkSLType type,
                                                     size_t arrayCount = Uniform::kNonArray) {
    // Set the start offset at 1 to force alignment.
    constexpr uint32_t kStart = 1;
    UniformOffsetCalculator calc(layout, kStart);
    size_t alignment = calc.advanceOffset(type, arrayCount);
    return {alignment, calc.size() - alignment};
}

#define EXPECT(type, expectedAlignment, expectedSize)                                \
    do {                                                                             \
        auto [alignment, size] = calculate_alignment_and_size(kLayout, type);        \
        REPORTER_ASSERT(r,                                                           \
                        alignment == expectedAlignment,                              \
                        "incorrect alignment for type '%s': expected %d, found %zu", \
                        SkSLTypeString(type),                                        \
                        expectedAlignment,                                           \
                        alignment);                                                  \
        REPORTER_ASSERT(r,                                                           \
                        size == expectedSize,                                        \
                        "incorrect size for type '%s': expected %d, found %zu",      \
                        SkSLTypeString(type),                                        \
                        expectedSize,                                                \
                        size);                                                       \
    } while (0)

#define EXPECT_ARRAY(type, expectedAlignment, expectedStride, expectedSize)           \
    do {                                                                              \
        auto [alignment, size] = calculate_alignment_and_size(kLayout, type, kCount); \
        size_t stride = size / kCount;                                                \
        REPORTER_ASSERT(r,                                                            \
                        alignment == expectedAlignment,                               \
                        "incorrect alignment for type '%s': expected %d, found %zu",  \
                        SkSLTypeString(type),                                         \
                        expectedAlignment,                                            \
                        alignment);                                                   \
        REPORTER_ASSERT(r,                                                            \
                        size == expectedSize,                                         \
                        "incorrect size for type '%s': expected %d, found %zu",       \
                        SkSLTypeString(type),                                         \
                        expectedSize,                                                 \
                        size);                                                        \
        REPORTER_ASSERT(r,                                                            \
                        stride == expectedStride,                                     \
                        "incorrect stride for type '%s': expected %d, found %zu",     \
                        SkSLTypeString(type),                                         \
                        expectedStride,                                               \
                        stride);                                                      \
    } while (0)

DEF_GRAPHITE_TEST(UniformOffsetCalculatorMetalBasicTypesTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kMetal;

    // scalars: int, float, short, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kShort,  /*alignment=*/2, /*size=*/2);
    EXPECT(SkSLType::kHalf,   /*alignment=*/2, /*size=*/2);

    // int2, float2, short2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kShort2,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/4, /*size=*/4);

    // int3, float3, short3, half3
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort3,  /*alignment=*/8,  /*size=*/8);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/8,  /*size=*/8);

    // int4, float4, short4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort4,  /*alignment=*/8,  /*size=*/8);
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

DEF_GRAPHITE_TEST(UniformOffsetCalculatorMetalArrayTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kMetal;
    constexpr size_t kCount = 3;

    // int[3], float[3], short[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kShort, /*alignment=*/2, /*stride=*/2, /*size=*/6);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/2, /*stride=*/2, /*size=*/6);

    // int2[3], float2[3], short2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kShort2, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/4, /*stride=*/4, /*size=*/12);

    // int3[3], float3[3], short3[3], half3[3]
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort3, /*alignment=*/8,  /*stride=*/8,  /*size=*/24);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/8,  /*stride=*/8,  /*size=*/24);

    // int4[3], float4[3], short4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort4, /*alignment=*/8,  /*stride=*/8,  /*size=*/24);
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

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd430BasicTypesTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kStd430;

    // scalars: int, float, short, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kShort,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf,   /*alignment=*/4, /*size=*/4);

    // int2, float2, short2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kShort2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/8, /*size=*/8);

    // int3, float3, short3, half3
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/16, /*size=*/16);

    // int4, float4, short4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort4,  /*alignment=*/16, /*size=*/16);
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

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd430ArrayTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kStd430;
    constexpr size_t kCount = 3;

    // int[3], float[3], short[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kShort, /*alignment=*/4, /*stride=*/4, /*size=*/12);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/4, /*stride=*/4, /*size=*/12);

    // int2[3], float2[3], short2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kShort2, /*alignment=*/8, /*stride=*/8, /*size=*/24);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/8, /*stride=*/8, /*size=*/24);

    // int3[3], float3[3], short3[3], half3[3]
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int4[3], float4[3], short4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
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

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd140BasicTypesTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kStd140;

    // scalars: int, float, short, half (unsigned types are disallowed)
    EXPECT(SkSLType::kInt,    /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kFloat,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kShort,  /*alignment=*/4, /*size=*/4);
    EXPECT(SkSLType::kHalf,   /*alignment=*/4, /*size=*/4);

    // int2, float2, short2, half2
    EXPECT(SkSLType::kInt2,    /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kFloat2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kShort2,  /*alignment=*/8, /*size=*/8);
    EXPECT(SkSLType::kHalf2,   /*alignment=*/8, /*size=*/8);

    // int3, float3, short3, half3
    EXPECT(SkSLType::kInt3,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort3,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kHalf3,   /*alignment=*/16, /*size=*/16);

    // int4, float4, short4, half4
    EXPECT(SkSLType::kInt4,    /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kFloat4,  /*alignment=*/16, /*size=*/16);
    EXPECT(SkSLType::kShort4,  /*alignment=*/16, /*size=*/16);
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

DEF_GRAPHITE_TEST(UniformOffsetCalculatorStd140ArrayTest, r, CtsEnforcement::kNextRelease) {
    constexpr Layout kLayout = Layout::kStd140;
    constexpr uint32_t kCount = 3;

    // int[3], float[3], short[3], half[3]
    EXPECT_ARRAY(SkSLType::kInt,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int2[3], float2[3], short2[3], half2[3]
    EXPECT_ARRAY(SkSLType::kInt2,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat2, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort2, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf2,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int3[3], float3[3], short3[3], half3[3]
    EXPECT_ARRAY(SkSLType::kInt3,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort3, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kHalf3,  /*alignment=*/16, /*stride=*/16, /*size=*/48);

    // int4[3], float4[3], short4[3], half4[3]
    EXPECT_ARRAY(SkSLType::kInt4,   /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kFloat4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
    EXPECT_ARRAY(SkSLType::kShort4, /*alignment=*/16, /*stride=*/16, /*size=*/48);
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

}  // namespace
