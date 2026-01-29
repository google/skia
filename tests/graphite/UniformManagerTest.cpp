/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkFloatBits.h"
#include "src/base/SkHalf.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

static constexpr Layout kLayouts[] = {
        Layout::kStd140,
        Layout::kStd140_F16,
        Layout::kStd430,
        Layout::kStd430_F16,
        Layout::kMetal,
};

// This list excludes SkSLTypes that we don't support in uniforms, like Bool, UInt or UShort.
static constexpr SkSLType kTypes[] = {
        SkSLType::kFloat,    SkSLType::kFloat2,   SkSLType::kFloat3,   SkSLType::kFloat4,   //
        SkSLType::kHalf,     SkSLType::kHalf2,    SkSLType::kHalf3,    SkSLType::kHalf4,    //
        SkSLType::kInt,      SkSLType::kInt2,     SkSLType::kInt3,     SkSLType::kInt4,     //
        SkSLType::kFloat2x2, SkSLType::kFloat3x3, SkSLType::kFloat4x4,                      //
        SkSLType::kHalf2x2,  SkSLType::kHalf3x3,  SkSLType::kHalf4x4,
};

static constexpr float kFloats[16] = { 1.0f,  2.0f,  3.0f,  4.0f,
                                       5.0f,  6.0f,  7.0f,  8.0f,
                                       9.0f, 10.0f, 11.0f, 12.0f,
                                      13.0f, 14.0f, 15.0f, 16.0f };

static constexpr SkHalf kHalfs[16] = { 0x3C00, 0x4000, 0x4200, 0x4400,
                                       0x4500, 0x4600, 0x4700, 0x4800,
                                       0x4880, 0x4900, 0x4980, 0x4A00,
                                       0x4A80, 0x4B00, 0x4B80, 0x4C00 };

static constexpr int32_t kInts[16] = { 1,  -2,  3,  -4,
                                       5,  -6,  7,  -8,
                                       9, -10, 11, -12,
                                      13, -14, 15, -16 };

static size_t element_size(Layout layout, SkSLType type) {
    // Metal and the _F16 std extended layouts encodes half-precision uniforms in 16 bits.
    // Other layouts are expected to encode uniforms in 32 bits.
    const bool usesHalf = layout == Layout::kMetal ||
                          layout == Layout::kStd140_F16 ||
                          layout == Layout::kStd430_F16;
    return (usesHalf && !SkSLTypeIsFullPrecisionNumericType(type)) ? 2 : 4;
}

DEF_GRAPHITE_TEST(UniformManagerCheckSingleUniform, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager can hold all the basic uniform types, in every layout.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            const Uniform expectations[] = {{"uniform", type}};
            SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
            mgr.write(expectations[0], kFloats);
            SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
            REPORTER_ASSERT(r, mgr.size() > 0, "Layout: %s - Type: %s",
                            LayoutString(layout), SkSLTypeString(type));
            mgr.reset();
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckFloatEncoding, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager encodes float data properly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            // Only test scalar and vector floats. (Matrices can introduce padding between values.)
            int vecLength = SkSLTypeVecLength(type);
            if (!SkSLTypeIsFloatType(type) || vecLength < 1) {
                continue;
            }

            // Write our uniform float scalar/vector.
            const Uniform expectations[] = {{"uniform", type}};
            SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
            mgr.write(expectations[0], kFloats);
            SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

            // Read back the uniform data.
            SkSpan<const char> uniformData = mgr.finish();
            size_t elementSize = element_size(layout, type);
            const void* validData = (elementSize == 4) ? (const void*)kFloats : (const void*)kHalfs;
            REPORTER_ASSERT(r, uniformData.size() >= vecLength * elementSize);
            REPORTER_ASSERT(r, 0 == memcmp(validData, uniformData.data(), vecLength * elementSize),
                            "Layout: %s - Type: %s float encoding failed",
                            LayoutString(layout), SkSLTypeString(type));
            mgr.reset();
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckIntEncoding, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager encodes int data properly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            if (!SkSLTypeIsIntegralType(type)) {
                continue;
            }

            // Write our uniform int scalar/vector.
            const Uniform expectations[] = {{"uniform", type}};
            SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
            mgr.write(expectations[0], kInts);
            SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

            // Read back the uniform data.
            SkSpan<const char> uniformData = mgr.finish();
            int vecLength = SkSLTypeVecLength(type);
            size_t elementSize = element_size(layout, type);
            REPORTER_ASSERT(r, uniformData.size() >= vecLength * elementSize);
            REPORTER_ASSERT(r, 0 == memcmp(kInts, uniformData.data(), vecLength * elementSize),
                            "Layout: %s - Type: %s int encoding failed",
                            LayoutString(layout), SkSLTypeString(type));
            mgr.reset();
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckScalarVectorPacking, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager can pack scalars and vectors of identical type correctly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            int vecLength = SkSLTypeVecLength(type);
            if (vecLength < 1) {
                continue;
            }

            // Write three matching uniforms.
            const Uniform expectations[] = {{"a", type}, {"b", type}, {"c", type}};
            SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
            mgr.write(expectations[0], kFloats);
            mgr.write(expectations[1], kFloats);
            mgr.write(expectations[2], kFloats);
            SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

            // Verify the uniform data packing.
            SkSpan<const char> uniformData = mgr.finish();
            size_t elementSize = element_size(layout, type);
            // Vec3s must be laid out as if they were vec4s.
            size_t effectiveVecLength = (vecLength == 3) ? 4 : vecLength;
            REPORTER_ASSERT(r, uniformData.size() == elementSize * effectiveVecLength * 3,
                            "Layout: %s - Type: %s tight packing failed",
                            LayoutString(layout), SkSLTypeString(type));
            mgr.reset();
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckMatrixPacking, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager can pack matrices correctly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            int matrixSize = SkSLTypeMatrixSize(type);
            if (matrixSize < 2) {
                continue;
            }

            // Write three matching uniforms.
            const Uniform expectations[] = {{"a", type}, {"b", type}, {"c", type}};
            SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
            mgr.write(expectations[0], kFloats);
            mgr.write(expectations[1], kFloats);
            mgr.write(expectations[2], kFloats);
            SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

            // Verify the uniform data packing.
            SkSpan<const char> uniformData = mgr.finish();
            // In std140-f16, pretend the element size is 4 since the underlying column vectors
            // are still forced to 16-byte alignment
            size_t elementSize = layout == Layout::kStd140_F16 ? 4 : element_size(layout, type);
            // In all layouts, mat3s burn 12 elements, not 9. In std140, mat2s burn 8 elements
            // instead of 4.
            size_t numElements;
            if (matrixSize == 3) {
                numElements = 12;
            } else if (matrixSize == 2 && (layout == Layout::kStd140 ||
                                           layout == Layout::kStd140_F16)) {
                numElements = 8;
            } else {
                numElements = matrixSize * matrixSize;
            }
            REPORTER_ASSERT(r, uniformData.size() == elementSize * numElements * 3,
                            "Layout: %s - Type: %s matrix packing failed",
                            LayoutString(layout), SkSLTypeString(type));
            mgr.reset();
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingScalarVector, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager properly adds padding between pairs of scalar/vector.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type1 : kTypes) {
            const int vecLength1 = SkSLTypeVecLength(type1);
            if (vecLength1 < 1) {
                continue;
            }

            for (SkSLType type2 : kTypes) {
                const int vecLength2 = SkSLTypeVecLength(type2);
                if (vecLength2 < 1) {
                    continue;
                }

                // Write two scalar/vector uniforms.
                const Uniform expectations[] = {{"a", type1}, {"b", type2}};
                SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
                mgr.write(expectations[0], kFloats);
                mgr.write(expectations[1], kFloats);
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                const int layoutIdx = static_cast<int>(layout != Layout::kMetal);

                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (either 16 or 32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // Metal (vec3 consumes vec4 size)
                        {{ "", "",         "",         "",         ""         },
                         { "", "AB",       "A_BB",     "A___BBBb", "A___BBBB" },
                         { "", "AAB_",     "AABB",     "AA__BBBb", "AA__BBBB" },
                         { "", "AAAaB___", "AAAaBB__", "AAAaBBBb", "AAAaBBBB" },
                         { "", "AAAAB___", "AAAABB__", "AAAABBBb", "AAAABBBB" }},
                        // std140 and std430 (vec3 aligns to vec4, but consumes only 3 elements)
                        {{ "", "",         "",         "",         ""         },
                         { "", "AB",       "A_BB",     "A___BBBb", "A___BBBB" },
                         { "", "AAB_",     "AABB",     "AA__BBBb", "AA__BBBB" },
                         { "", "AAAB",     "AAA_BB__", "AAA_BBBb", "AAA_BBBB" },
                         { "", "AAAAB___", "AAAABB__", "AAAABBBb", "AAAABBBB" }},
                    };
                    const size_t size = strlen(kExpectedLayout[layoutIdx][vecLength1][vecLength2]) *
                                        elementSize1;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 2 && elementSize2 == 4) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots in Metal)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // Metal (vec3 consumes vec4 size)
                        {{ "", "",         "",         "",                 ""         },
                         { "", "A_BB",     "A___BBBB", "A_______BBBBBBbb", "A_______BBBBBBBB" },
                         { "", "AABB",     "AA__BBBB", "AA______BBBBBBbb", "AA______BBBBBBBB" },
                         { "", "AAAaBB__", "AAAaBBBB", "AAAa____BBBBBBbb", "AAAa____BBBBBBBB" },
                         { "", "AAAABB__", "AAAABBBB", "AAAA____BBBBBBbb", "AAAA____BBBBBBBB" }},
                        // std140-f16 and std430-f16 (vec3 aligns to vec4 but consumes only 3)
                        {{ "", "",         "",         "",                 ""         },
                         { "", "A_BB",     "A___BBBB", "A_______BBBBBBbb", "A_______BBBBBBBB" },
                         { "", "AABB",     "AA__BBBB", "AA______BBBBBBbb", "AA______BBBBBBBB" },
                         { "", "AAA_BB__", "AAA_BBBB", "AAA_____BBBBBBbb", "AAA_____BBBBBBBB" },
                         { "", "AAAABB__", "AAAABBBB", "AAAA____BBBBBBbb", "AAAA____BBBBBBBB" }},
                    };
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][vecLength1][vecLength2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 4 && elementSize2 == 2) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // Metal (vec3 consumes vec4 size)
                        {{ "", "",                 "",                 "",                 "" },
                         { "", "AAB_",             "AABB",             "AA__BBBb",         "AA__BBBB" },
                         { "", "AAAAB___",         "AAAABB__",         "AAAABBBb",         "AAAABBBB" },
                         { "", "AAAAAAaaB_______", "AAAAAAaaBB______", "AAAAAAaaBBBb____", "AAAAAAaaBBBB____" },
                         { "", "AAAAAAAAB_______", "AAAAAAAABB______", "AAAAAAAABBBb____", "AAAAAAAABBBB____" }},
                        // std140-f16 and std430-f16 (vec3 aligns to vec4 but consumes only 3)
                        {{ "", "",                 "",                 "",                 "" },
                         { "", "AAB_",             "AABB",             "AA__BBB_",         "AA__BBBB" },
                         { "", "AAAAB___",         "AAAABB__",         "AAAABBB_",         "AAAABBBB" },
                         { "", "AAAAAAB_",         "AAAAAABB",         "AAAAAA__BBB_____", "AAAAAA__BBBB____" },
                         { "", "AAAAAAAAB_______", "AAAAAAAABB______", "AAAAAAAABBB_____", "AAAAAAAABBBB____" }},
                    };
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][vecLength1][vecLength2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else {
                    ERRORF(r, "Unexpected element sizes: %zu %zu", elementSize1, elementSize2);
                }
                mgr.reset();
            }
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingVectorMatrix, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager properly adds padding between vectors and matrices.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type1 : kTypes) {
            const int vecLength1 = SkSLTypeVecLength(type1);
            if (vecLength1 < 1) {
                continue;
            }

            for (SkSLType type2 : kTypes) {
                const int matSize2 = SkSLTypeMatrixSize(type2);
                if (matSize2 < 2) {
                    continue;
                }

                // Write the scalar/vector and matrix uniforms.
                const Uniform expectations[] = {{"a", type1}, {"b", type2}};
                SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
                mgr.write(expectations[0], kFloats);
                mgr.write(expectations[1], kFloats);
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                int layoutIdx = static_cast<int>(layout != Layout::kStd140 &&
                                                 layout != Layout::kStd140_F16);

                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (16 or 32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[3][5][5] = {
                        // std140-f16
                        {
                            { "", "", "",             "",                 "" },
                            { "", "", "A_______BBbb____BBbb____", "A_______BBBb____BBBb____BBBb____", "A_______BBBB____BBBB____BBBB____BBBB____" },
                            { "", "", "AA______BBbb____BBbb____", "AA______BBBb____BBBb____BBBb____", "AA______BBBB____BBBB____BBBB____BBBB____" },
                            { "", "", "AAAa____BBbb____BBbb____", "AAAa____BBBb____BBBb____BBBb____", "AAAa____BBBB____BBBB____BBBB____BBBB____" },
                            { "", "", "AAAA____BBbb____BBbb____", "AAAA____BBBb____BBBb____BBBb____", "AAAA____BBBB____BBBB____BBBB____BBBB____" },
                        },
                        // std140
                        {
                            { "", "", "",             "",                 "" },
                            { "", "", "A___BBbbBBbb", "A___BBBbBBBbBBBb", "A___BBBBBBBBBBBBBBBB" },
                            { "", "", "AA__BBbbBBbb", "AA__BBBbBBBbBBBb", "AA__BBBBBBBBBBBBBBBB" },
                            { "", "", "AAAaBBbbBBbb", "AAAaBBBbBBBbBBBb", "AAAaBBBBBBBBBBBBBBBB" },
                            { "", "", "AAAABBbbBBbb", "AAAABBBbBBBbBBBb", "AAAABBBBBBBBBBBBBBBB" },
                        },
                        // All other layouts
                        {
                            { "", "", "",         "",                 "" },
                            { "", "", "A_BBBB",   "A___BBBbBBBbBBBb", "A___BBBBBBBBBBBBBBBB" },
                            { "", "", "AABBBB",   "AA__BBBbBBBbBBBb", "AA__BBBBBBBBBBBBBBBB" },
                            { "", "", "AAAaBBBB", "AAAaBBBbBBBbBBBb", "AAAaBBBBBBBBBBBBBBBB" },
                            { "", "", "AAAABBBB", "AAAABBBbBBBbBBBb", "AAAABBBBBBBBBBBBBBBB" },
                        },
                    };

                    if (elementSize1 != 2 || layout != Layout::kStd140_F16) {
                        layoutIdx++;
                    }
                    // size_t elementSize = elementSize1 == 2 && layout == Layout::kStd140_F16 ? 4 : elementSize1;
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][vecLength1][matSize2]) * elementSize1;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s vector-matrix padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 2 && elementSize2 == 4) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140-f16
                        {
                            {"", "", "",                         "",                                 ""},
                            {"", "", "A_______BBBB____BBBB____", "A_______BBBBBBbbBBBBBBbbBBBBBBbb", "A_______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AA______BBBB____BBBB____", "AA______BBBBBBbbBBBBBBbbBBBBBBbb", "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AAAa____BBBB____BBBB____", "AAAa____BBBBBBbbBBBBBBbbBBBBBBbb", "AAAa____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AAAA____BBBB____BBBB____", "AAAA____BBBBBBbbBBBBBBbbBBBBBBbb", "AAAA____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                        },
                        // Metal and std430-f16
                        {
                            {"", "", "",             "",                                 ""},
                            {"", "", "A___BBBBBBBB", "A_______BBBBBBbbBBBBBBbbBBBBBBbb", "A_______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AA__BBBBBBBB", "AA______BBBBBBbbBBBBBBbbBBBBBBbb", "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AAAaBBBBBBBB", "AAAa____BBBBBBbbBBBBBBbbBBBBBBbb", "AAAa____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "", "AAAABBBBBBBB", "AAAA____BBBBBBbbBBBBBBbbBBBBBBbb", "AAAA____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                        }
                    };
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][vecLength1][matSize2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s vector-matrix padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 4 && elementSize2 == 2) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140-f16
                        {
                            {"", "", "",                         "",                                 ""},
                            {"", "", "AA______BB______BB______", "AA______BBBb____BBBb____BBBb____", "AA______BBBB____BBBB____BBBB____BBBB____"},
                            {"", "", "AAAA____BB______BB______", "AAAA____BBBb____BBBb____BBBb____", "AAAA____BBBB____BBBB____BBBB____BBBB____"},
                            {"", "", "AAAAAAaaBB______BB______", "AAAAAAaaBBBb____BBBb____BBBb____", "AAAAAAaaBBBB____BBBB____BBBB____BBBB____"},
                            {"", "", "AAAAAAAABB______BB______", "AAAAAAAABBBb____BBBb____BBBb____", "AAAAAAAABBBB____BBBB____BBBB____BBBB____"},
                        },
                        // Metal and std430-f16
                        {
                            {"", "", "",                 "",                         ""},
                            {"", "", "AABBBB",           "AA__BBBbBBBbBBBb",         "AA__BBBBBBBBBBBBBBBB"},
                            {"", "", "AAAABBBB",         "AAAABBBbBBBbBBBb",         "AAAABBBBBBBBBBBBBBBB"},
                            {"", "", "AAAAAAaaBBBB____", "AAAAAAaaBBBbBBBbBBBb____", "AAAAAAaaBBBBBBBBBBBBBBBB"},
                            {"", "", "AAAAAAAABBBB____", "AAAAAAAABBBbBBBbBBBb____", "AAAAAAAABBBBBBBBBBBBBBBB"},
                        }
                    };
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][vecLength1][matSize2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s vector-matrix padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                }
                mgr.reset();
            }
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingMatrixVector, r, CtsEnforcement::kApiLevel_202404) {
    // Verify that the uniform manager properly adds padding between matrices and vectors.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type1 : kTypes) {
            const int matSize1 = SkSLTypeMatrixSize(type1);
            if (matSize1 < 2) {
                continue;
            }

            for (SkSLType type2 : kTypes) {
                const int vecLength2 = SkSLTypeVecLength(type2);
                if (vecLength2 < 1) {
                    continue;
                }

                // Write the scalar/vector and matrix uniforms.
                const Uniform expectations[] = {{"a", type1}, {"b", type2}};
                SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
                mgr.write(expectations[0], kFloats);
                mgr.write(expectations[1], kFloats);
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                int layoutIdx = static_cast<int>(layout != Layout::kStd140 &&
                                                 layout != Layout::kStd140_F16);

                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (16 or 32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[3][5][5] = {
                        // std140-f16 layout
                        {
                            { "", "",                                         "",                                         "",                                         "" },
                            { "", "",                                         "",                                         "",                                         "" },
                            { "", "AAaa____AAaa____B_______",                 "AAaa____AAaa____BB______",                 "AAaa____AAaa____BBBb____",                 "AAaa____AAaa____BBBB____" },
                            { "", "AAAa____AAAa____AAAa____B_______",         "AAAa____AAAa____AAAa____BB______",         "AAAa____AAAa____AAAa____BBBb____",         "AAAa____AAAa____AAAa____BBBB____" },
                            { "", "AAAA____AAAA____AAAA____AAAA____B_______", "AAAA____AAAA____AAAA____AAAA____BB______", "AAAA____AAAA____AAAA____AAAA____BBBb____", "AAAA____AAAA____AAAA____AAAA____BBBB____" },
                        },
                        // std140 layout
                        {
                            { "", "",                     "",                     "",                     "" },
                            { "", "",                     "",                     "",                     "" },
                            { "", "AAaaAAaaB___",         "AAaaAAaaBB__",         "AAaaAAaaBBBb",         "AAaaAAaaBBBB" },
                            { "", "AAAaAAAaAAAaB___",     "AAAaAAAaAAAaBB__",     "AAAaAAAaAAAaBBBb",     "AAAaAAAaAAAaBBBB" },
                            { "", "AAAAAAAAAAAAAAAAB___", "AAAAAAAAAAAAAAAABB__", "AAAAAAAAAAAAAAAABBBb", "AAAAAAAAAAAAAAAABBBB" },
                        },
                        // All other layouts
                        {
                            { "", "",                     "",                     "",                     "" },
                            { "", "",                     "",                     "",                     "" },
                            { "", "AAAAB_",               "AAAABB",               "AAAABBBb",             "AAAABBBB" },
                            { "", "AAAaAAAaAAAaB___",     "AAAaAAAaAAAaBB__",     "AAAaAAAaAAAaBBBb",     "AAAaAAAaAAAaBBBB" },
                            { "", "AAAAAAAAAAAAAAAAB___", "AAAAAAAAAAAAAAAABB__", "AAAAAAAAAAAAAAAABBBb", "AAAAAAAAAAAAAAAABBBB" },
                        },
                    };

                    if (elementSize1 != 2 || layout != Layout::kStd140_F16) {
                        layoutIdx++;
                    }
                    const size_t size = strlen(kExpectedLayout[layoutIdx][matSize1][vecLength2]) *
                                        elementSize1;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s matrix-vector padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 2 && elementSize2 == 4) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140-f16
                        {
                            { "", "",                                         "",                                         "",                                         "" },
                            { "", "",                                         "",                                         "",                                         "" },
                            { "", "AA______AA______BB______",                 "AA______AA______BBBB____",                 "AA______AA______BBBBBBbb",                 "AA______AA______BBBBBBBB" },
                            { "", "AAAa____AAAa____AAAa____BB______",         "AAAa____AAAa____AAAa____BBBB____",         "AAAa____AAAa____AAAa____BBBBBBbb",         "AAAa____AAAa____AAAa____BBBBBBBB" },
                            { "", "AAAA____AAAA____AAAA____AAAA____BB______", "AAAA____AAAA____AAAA____AAAA____BBBB____", "AAAA____AAAA____AAAA____AAAA____BBBBBBbb", "AAAA____AAAA____AAAA____AAAA____BBBBBBBB" },
                        },
                        // Metal and std430-f16
                        {
                            { "", "",                     "",                     "",                         "" },
                            { "", "",                     "",                     "",                         "" },
                            { "", "AAAABB",               "AAAABBBB",             "AAAA____BBBBBBbb",         "AAAA____BBBBBBBB" },
                            { "", "AAAaAAAaAAAaBB__",     "AAAaAAAaAAAaBBBB",     "AAAaAAAaAAAa____BBBBBBbb", "AAAaAAAaAAAa____BBBBBBBB" },
                            { "", "AAAAAAAAAAAAAAAABB__", "AAAAAAAAAAAAAAAABBBB", "AAAAAAAAAAAAAAAABBBBBBbb", "AAAAAAAAAAAAAAAABBBBBBBB" },
                        }
                    };
                    const size_t size =
                            strlen(kExpectedLayout[layoutIdx][matSize1][vecLength2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s matrix-vector padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                } else if (elementSize1 == 4 && elementSize2 == 2) {
                    // Elements in the array below correspond to 16 bits apiece.
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140-f16
                        {
                            { "", "",                                         "",                                         "",                                          "" },
                            { "", "",                                         "",                                         "",                                          "" },
                            { "", "AAAA____AAAA____B_______",                 "AAAA____AAAA____BB______",                 "AAAA____AAAA____BBBb____",                  "AAAA____AAAA____BBBB____" },
                            { "", "AAAAAAaaAAAAAAaaAAAAAAaaB_______",         "AAAAAAaaAAAAAAaaAAAAAAaaBB______",         "AAAAAAaaAAAAAAaaAAAAAAaaBBBb____",          "AAAAAAaaAAAAAAaaAAAAAAaaBBBB____" },
                            { "", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB_______", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABB______", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBb____",  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB____" },
                        },
                        // Metal and std430-f16
                        {
                            { "", "",                                         "",                                         "",                                          "" },
                            { "", "",                                         "",                                         "",                                          "" },
                            { "", "AAAAAAAAB___",                             "AAAAAAAABB__",                             "AAAAAAAABBBb",                              "AAAAAAAABBBB" },
                            { "", "AAAAAAaaAAAAAAaaAAAAAAaaB_______",         "AAAAAAaaAAAAAAaaAAAAAAaaBB______",         "AAAAAAaaAAAAAAaaAAAAAAaaBBBb____",          "AAAAAAaaAAAAAAaaAAAAAAaaBBBB____" },
                            { "", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB_______", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABB______", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBb____",  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB____" },
                        }
                    };
                    const size_t size = strlen(kExpectedLayout[layoutIdx][matSize1][vecLength2]) * 2;
                    SkSpan<const char> uniformData = mgr.finish();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout: %s - Types: %s, %s matrix-vector padding test failed",
                                    LayoutString(layout),
                                    SkSLTypeString(type1), SkSLTypeString(type2));
                }
                mgr.reset();
            }
        }
    }
}

DEF_GRAPHITE_TEST(UniformManagerMetalArrayLayout, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kMetal);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Each letter (A/B/a/b) corresponds to a single byte.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type.
        // _  : padding between uniforms for alignment.

        /* {half, float[3]}  */  "AA__BBBBBBBBBBBB",
        /* {half, float2[3]} */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3[3]} */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */  "AABBBBBB",
        /* {half, half2[3]}  */  "AA__BBBBBBBBBBBB",
        /* {half, half3[3]}  */  "AA______BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4[3]}  */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int[3]}    */  "AA__BBBBBBBBBBBB",
        /* {half, int2[3]}   */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int3[3]}   */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3] */  "AA__BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3x3[3] */  "AA______"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4x4[3] */  "AA______"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const Uniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kHalfs);
        mgr.write(expectations[1], kBuffer);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd430ArrayLayout, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd430);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Each letter (A/B/a/b) corresponds to a single byte.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type.
        // _  : padding between uniforms for alignment.

        /* {half, float[3]}  */  "AA__BBBBBBBBBBBB",
        /* {half, float2[3]} */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3[3]} */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */  "AA__BBBBBBBBBBBB",
        /* {half, half2[3]}  */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3[3]}  */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, half4[3]}  */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int[3]}    */  "AA__BBBBBBBBBBBB",
        /* {half, int2[3]}   */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int3[3]}   */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3]  */ "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3x3[3]  */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, half4x4[3]  */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const Uniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kHalfs);
        mgr.write(expectations[1], kBuffer);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd430F16ArrayLayout, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd430_F16);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Each letter (A/B/a/b) corresponds to a single byte.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type.
        // _  : padding between uniforms for alignment.

        /* {half, float[3]}  */  "AA__BBBBBBBBBBBB",
        /* {half, float2[3]} */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3[3]} */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */  "AABBBBBB",
        /* {half, half2[3]}  */  "AA__BBBBBBBBBBBB",
        /* {half, half3[3]}  */  "AA______BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4[3]}  */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int[3]}    */  "AA__BBBBBBBBBBBB",
        /* {half, int2[3]}   */  "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int3[3]}   */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3]  */ "AA__BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3x3[3]  */ "AA______"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4x4[3]  */ "AA______"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const Uniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kHalfs);
        mgr.write(expectations[1], kBuffer);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd140ArrayLayout, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd140);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Each letter (A/B/a/b) corresponds to a single byte.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type.
        // _  : padding between uniforms for alignment.

        /* {half, float[3]}  */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, float2[3]} */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float3[3]} */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */  "AA______________BBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbb",
        /* {half, half2[3]}  */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, half3[3]}  */  "AA______________BBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbb",
        /* {half, half4[3]}  */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int[3]}    */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, int2[3]}   */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int3[3]}   */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______________"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3]  */ "AA______________"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, half3x3[3]  */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, half4x4[3]  */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const Uniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kHalfs);
        mgr.write(expectations[1], kBuffer);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

// NOTE: Since arrays are aligned to 16 bytes, these cases end up matching kStd140 offsets
DEF_GRAPHITE_TEST(UniformManagerStd140F16ArrayLayout, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd140_F16);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Each letter (A/B/a/b) corresponds to a single byte.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type.
        // _  : padding between uniforms for alignment.

        /* {half, float[3]}  */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, float2[3]} */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float3[3]} */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */  "AA______________BBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbb",
        /* {half, half2[3]}  */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, half3[3]}  */  "AA______________BBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbb",
        /* {half, half4[3]}  */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int[3]}    */  "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, int2[3]}   */  "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int3[3]}   */  "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */  "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______________"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3]  */ "AA______________"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb"
                                 "BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, half3x3[3]  */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, half4x4[3]  */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const Uniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kHalfs);
        mgr.write(expectations[1], kBuffer);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

// This test validates that the uniform data for matrix types get written out according to the
// layout expectations.
static void expect_matrix(skiatest::Reporter* reporter,
                          UniformManager& mgr,
                          SkSLType type,
                          bool isFullPrecision,
                          size_t expectedSizeInBytes,
                          SkSpan<const int> expectedOffsetsInPrimitives) {
    const Uniform expectations[] = {{"m", type}};
    SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
    mgr.write(expectations[0], kFloats);
    SkDEBUGCODE(mgr.doneWithExpectedUniforms();)

    SkSpan<const char> data = mgr.finish();
    REPORTER_ASSERT(reporter, data.size() == expectedSizeInBytes,
                    "%s layout size expected %zu, got %zu",
                    SkSLTypeString(type), expectedSizeInBytes, data.size());

    if (isFullPrecision) {


        const float* elements = reinterpret_cast<const float*>(data.data());
        for (size_t index = 0; index < expectedOffsetsInPrimitives.size(); ++index) {
            float el = elements[expectedOffsetsInPrimitives[index]];
            float expected = kFloats[index];
            REPORTER_ASSERT(reporter, el == expected,
                            "Incorrect %s element %zu - expected %f, got %f",
                            SkSLTypeString(type), index, expected, el);
        }
    } else {
        const SkHalf* elements = reinterpret_cast<const SkHalf*>(data.data());
        for (size_t index = 0; index < expectedOffsetsInPrimitives.size(); ++index) {
            SkHalf el = elements[expectedOffsetsInPrimitives[index]];
            SkHalf expected = kHalfs[index];

            REPORTER_ASSERT(reporter, el == expected,
                            "Incorrect %s element %zu - expected 0x%04x, got 0x%04x",
                            SkSLTypeString(type), index, expected, el);
        }
    }

    mgr.reset();
}

DEF_GRAPHITE_TEST(UniformManagerStd140MatrixLayoutContents, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd140);

    // 2x2
    for (SkSLType type : {SkSLType::kFloat2x2, SkSLType::kHalf2x2}) {
        expect_matrix(r, mgr, type, /*isFullPrecision=*/true, /*expectedSizeInBytes=*/32,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 4, 5});
    }

    // 3x3
    for (SkSLType type : {SkSLType::kFloat3x3, SkSLType::kHalf3x3}) {
        expect_matrix(r, mgr, type, /*isFullPrecision=*/true, /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd140F16MatrixLayoutContents, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd140_F16);

    // 2x2
    {
        expect_matrix(r, mgr, SkSLType::kFloat2x2, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/32, /*expectedOffsetsInPrimitives=*/{0, 1, 4, 5});
        expect_matrix(r, mgr, SkSLType::kHalf2x2, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/32, /*expectedOffsetsInPrimitives=*/{0, 1, 8, 9});
    }

    // 3x3
    {
        expect_matrix(r, mgr, SkSLType::kFloat3x3, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
        expect_matrix(r, mgr, SkSLType::kHalf3x3, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 8, 9, 10, 16, 17, 18});
    }

    // 4x4
    {
        expect_matrix(r, mgr, SkSLType::kFloat4x4, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/64,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
        expect_matrix(r, mgr, SkSLType::kHalf4x4, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/64,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       8, 9, 10, 11,
                                                       16, 17, 18, 19,
                                                       24, 25, 26, 27});
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd430MatrixLayoutContents, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd430);

    // 2x2
    for (SkSLType type : {SkSLType::kFloat2x2, SkSLType::kHalf2x2}) {
        expect_matrix(r, mgr, type, /*isFullPrecision=*/true, /*expectedSizeInBytes=*/16,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3});
    }

    // 3x3
    for (SkSLType type : {SkSLType::kFloat3x3, SkSLType::kHalf3x3}) {
        expect_matrix(r, mgr, type, /*isFullPrecision=*/true, /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
    }

    // 4x4
    for (SkSLType type : {SkSLType::kFloat4x4, SkSLType::kHalf4x4}) {
        expect_matrix(r, mgr, type, /*isFullPrecision=*/true, /*expectedSizeInBytes=*/64,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
    }
}

DEF_GRAPHITE_TEST(UniformManagerStd430F16MatrixLayoutContents, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kStd430_F16);

    // 2x2
    {
        expect_matrix(r, mgr, SkSLType::kFloat2x2, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/16, /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3});
        expect_matrix(r, mgr, SkSLType::kHalf2x2, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/8, /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3});
    }

    // 3x3
    {
        expect_matrix(r, mgr, SkSLType::kFloat3x3, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
        expect_matrix(r, mgr, SkSLType::kHalf3x3, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/24,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
    }

    // 4x4
    {
        expect_matrix(r, mgr, SkSLType::kFloat4x4, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/64,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
        expect_matrix(r, mgr, SkSLType::kHalf4x4, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/32,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
    }
}

DEF_GRAPHITE_TEST(UniformManagerMetalMatrixLayoutContents, r, CtsEnforcement::kApiLevel_202404) {
    UniformManager mgr(Layout::kMetal);

    // 2x2
    {
        expect_matrix(r, mgr, SkSLType::kFloat2x2, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/16, /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3});
        expect_matrix(r, mgr, SkSLType::kHalf2x2, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/8, /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3});
    }

    // 3x3
    {
        expect_matrix(r, mgr, SkSLType::kFloat3x3, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/48,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
        expect_matrix(r, mgr, SkSLType::kHalf3x3, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/24,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 4, 5, 6, 8, 9, 10});
    }

    // 4x4
    {
        expect_matrix(r, mgr, SkSLType::kFloat4x4, /*isFullPrecision=*/true,
                      /*expectedSizeInBytes=*/64,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
        expect_matrix(r, mgr, SkSLType::kHalf4x4, /*isFullPrecision=*/false,
                      /*expectedSizeInBytes=*/32,
                      /*expectedOffsetsInPrimitives=*/{0, 1, 2, 3,
                                                       4, 5, 6, 7,
                                                       8, 9, 10, 11,
                                                       12, 13, 14, 15});
    }
}

// These tests validate that substructs are written and aligned appropriately.
DEF_GRAPHITE_TEST(UniformManagerStructLayout, r, CtsEnforcement::kNever) {
    static constexpr uint16_t _ = 0; // 0s will only be written as padding
    static const struct TestCase {
        // For convenience, these should only have kFloat[2,3,4] or only kHalf[2,3,4] as their
        // types. However the values written are integers and will be bit-punned to floats so that
        // expectations are easy to define (UniformManager and Skia have no defined kInt3 object
        // type). Written integers start at 1 and are incremented with each component across
        // uniforms. 4-byte primitive types copy the expected integer value into the low and high
        // 16-bits
        //
        // Expectations are written in 16-bit units.
        std::vector<Uniform> fPreStruct;  // before beginStruct(), top-level fields or struct
        std::vector<Uniform> fSubstruct;  // within beginStruct()/endStruct()
        std::vector<Uniform> fPostStruct; // after endStruct(), top-level fields

        std::pair<int, std::vector<uint16_t>> fExpectedAlignmentAndData[std::size(kLayouts)];

        // If non-empty, holds base alignments for fPreStruct base alignment as a struct
        std::vector<int> fPreStructAlignments = {};
    } kCases[] = {
        // Struct tests with no preceeding or following top-level fields
        // NOTE: For kFloat types, the f16 layout variants are equivalent to the regular layout,
        // and for kHalf types, the regular layouts are equivalent to their kFloat cases
        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1, 2,2, 3,3, 4,4}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1, 2,2, 3,3, 4,4}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_, 2,_, 3,_, 4,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1, 2, 3, 4, _,_,_,_}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,_, 2,_, 3,_, 4,_}},
            /*std430-f16=*/{/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4}},
            /*metal=*/     {/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,2,2,3,3, 4,4}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,2,2,3,3, 4,4}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,2,2,3,3, 4,4}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,2,2,3,3, 4,4}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,2,2,3,3,_,_, 4,4,_,_,_,_,_,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kHalf3},
                         {"u2", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,2,_,3,_, 4,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,2,3, 4, _,_,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,2,_,3,_, 4,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,2,3, 4}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,2,3,_, 4,_,_,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,2,2, 3,3, 4,4, 5,5,_,_,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,2,2, 3,3, 4,4, 5,5,_,_,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,1,2,2, 3,3, 4,4, 5,5,_,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,1,2,2, 3,3, 4,4, 5,5,_,_}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,1,2,2, 3,3, 4,4, 5,5,_,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kHalf2},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,2,_, 3,_, 4,_, 5,_,_,_,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,2, 3, 4, 5,_,_,_}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,_,2,_, 3,_, 4,_, 5,_,_,_}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,2, 3, 4, 5,_}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,2, 3, 4, 5,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,5,5, 6,6,7,7,_,_,_,_, 8,8,9,9,10,10,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,5,5, 6,6,7,7,_,_,_,_, 8,8,9,9,10,10,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,5,5, 6,6,7,7,_,_,_,_, 8,8,9,9,10,10,_,_}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,5,5, 6,6,7,7,_,_,_,_, 8,8,9,9,10,10,_,_}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,5,5, 6,6,7,7,_,_,_,_, 8,8,9,9,10,10,_,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf4},
                         {"u3", SkSLType::kHalf2},
                         {"u4", SkSLType::kHalf3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_,5,_, 6,_,7,_,_,_,_,_, 8,_,9,_,10,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_,5,_, 6,_,7,_,_,_,_,_, 8,_,9,_,10,_,_,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}}
         }},


        // // Struct tests with a preceeding float to require padding to the struct's base alignment
        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2, 3,3, 4,4, 5,5}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2, 3,3, 4,4, 5,5}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_, 3,_, 4,_, 5,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2, 3, 4, 5,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,_, 2,_, 3,_, 4,_, 5,_}},
            /*std430-f16=*/{/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4, 5}},
            /*metal=*/     {/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4, 5}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,_,_, 5,5,_,_,_,_,_,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf3},
                         {"u2", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_, 5,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,3,4, 5,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_, 5,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,3,4, 5}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,3,4,_, 5,_,_,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf2},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_, 4,_, 5,_, 6,_,_,_,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,3, 4, 5, 6,_,_,_,}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,3,_, 4,_, 5,_, 6,_,_,_}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,_, 2,3, 4, 5, 6,_}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,_, 2,3, 4, 5, 6,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf4},
                         {"u3", SkSLType::kHalf2},
                         {"u4", SkSLType::kHalf3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_,_,_,_,_, 3,_,4,_,5,_,6,_, 7,_,8,_,_,_,_,_, 9,_,10,_,11,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_,_,_,_,_, 3,_,4,_,5,_,6,_, 7,_,8,_,_,_,_,_, 9,_,10,_,11,_,_,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}}
         }},

        // Struct tests with a preceeding float to require padding to the struct's base alignment,
        // and a following float4 to test alignment after a struct.
        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2, 3,3, 4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2, 3,3, 4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5, _,_,_,_,_,_,6,6,7,7,8,8,9,9}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5, _,_,_,_,_,_,6,6,7,7,8,8,9,9}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,1, 2,2, 3,3, 4,4, 5,5, _,_,_,_,_,_,6,6,7,7,8,8,9,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{{"p2", SkSLType::kHalf4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_, 3,_, 4,_, 5,_, 6,_,7,_,8,_,9,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2, 3, 4, 5,_,_,_,_, 6,7,8,9,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,_, 2,_, 3,_, 4,_, 5,_, _,_,_,_,_,_,6,_,7,_,8,_,9,_}},
            /*std430-f16=*/{/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4, 5, _,_,_,6,7,8,9}},
            /*metal=*/     {/*baseAlign=*/2,  /*data=*/{1, 2, 3, 4, 5, _,_,_,6,7,8,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4, 5,5, 6,6,7,7,8,8,9,9}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3,4,4,_,_, 5,5,_,_,_,_,_,_, 6,6,7,7,8,8,9,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf3},
                         {"u2", SkSLType::kHalf}},
         /*poststruct=*/{{"p2", SkSLType::kHalf4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_, 5,_, 6,_,7,_,8,_,9,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,3,4, 5,_,_,_,_, 6,7,8,9,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_,4,_, 5,_, 6,_,7,_,8,_,9,_}},
            /*std430-f16=*/{/*baseAlign=*/8, /*data=*/{1,_,_,_, 2,3,4, 5, 6,7,8,9}},
            /*metal=*/     {/*baseAlign=*/8, /*data=*/{1,_,_,_, 2,3,4,_, 5,_,_,_, 6,7,8,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_,_,_,_,_, 7,7,8,8,9,9,10,10}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_,_,_,_,_, 7,7,8,8,9,9,10,10}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_, 7,7,8,8,9,9,10,10}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_, 7,7,8,8,9,9,10,10}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,1,_,_, 2,2,3,3, 4,4, 5,5, 6,6,_,_, 7,7,8,8,9,9,10,10}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf2},
                         {"u2", SkSLType::kHalf},
                         {"u3", SkSLType::kHalf},
                         {"u4", SkSLType::kHalf}},
         /*poststruct=*/{{"p2", SkSLType::kHalf4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,3,_, 4,_, 5,_, 6,_,_,_,_,_,_,_, 7,_,8,_,9,_,10,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,3, 4, 5, 6,_,_,_, 7,8,9,10,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,3,_, 4,_, 5,_, 6,_,_,_, 7,_,8,_,9,_,10,_}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,_, 2,3, 4, 5, 6,_, 7,8,9,10}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,_, 2,3, 4, 5, 6,_, 7,8,9,10}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_, 12,12,13,13,14,14,15,15}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_, 12,12,13,13,14,14,15,15}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_, 12,12,13,13,14,14,15,15}},
            /*std430-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_, 12,12,13,13,14,14,15,15}},
            /*metal=*/     {/*baseAlign=*/16, /*data=*/{1,1,_,_,_,_,_,_, 2,2,_,_,_,_,_,_, 3,3,4,4,5,5,6,6, 7,7,8,8,_,_,_,_, 9,9,10,10,11,11,_,_, 12,12,13,13,14,14,15,15}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf4},
                         {"u3", SkSLType::kHalf2},
                         {"u4", SkSLType::kHalf3}},
         /*poststruct=*/{{"p2", SkSLType::kHalf4}},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_,_,_,_,_, 3,_,4,_,5,_,6,_, 7,_,8,_,_,_,_,_, 9,_,10,_,11,_,_,_, 12,_,13,_,14,_,15,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/16, /*data=*/{1,_,_,_,_,_,_,_, 2,_,_,_,_,_,_,_, 3,_,4,_,5,_,6,_, 7,_,8,_,_,_,_,_, 9,_,10,_,11,_,_,_, 12,_,13,_,14,_,15,_}},
            /*std430-f16=*/{/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15}},
            /*metal=*/     {/*baseAlign=*/8,  /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15}}
         }},

        // Struct tests with two adjacent structs
        {/*prestruct=*/ {{"p1", SkSLType::kFloat2},
                         {"p2", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,1,2,2, 3,3,_,_, 4,4, 5,5,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,1,2,2, 3,3,_,_, 4,4, 5,5,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,1,2,2, 3,3,_,_, 4,4, 5,5}},
            /*std430-f16=*/{/*baseAlign=*/4,  /*data=*/{1,1,2,2, 3,3,_,_, 4,4, 5,5}},
            /*metal=*/     {/*baseAlign=*/4,  /*data=*/{1,1,2,2, 3,3,_,_, 4,4, 5,5}}
         },
         /*preStructAlignments=*/{
            /*std140=*/16,
            /*std140-f16=*/16,
            /*std430=*/8,
            /*std430-f16=*/8,
            /*metal=*/ 8
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kHalf2},
                         {"p2", SkSLType::kHalf}},
         /*substruct=*/ {{"u1", SkSLType::kHalf},
                         {"u2", SkSLType::kHalf}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/    {/*baseAlign=*/16, /*data=*/{1,_,2,_, 3,_,_,_, 4,_, 5,_,_,_,_,_}},
            /*std140-f16=*/{/*baseAlign=*/16, /*data=*/{1,2, 3,_,_,_,_,_, 4, 5,_,_,_,_,_,_}},
            /*std430=*/    {/*baseAlign=*/4,  /*data=*/{1,_,2,_, 3,_,_,_, 4,_, 5,_}},
            /*std430-f16=*/{/*baseAlign=*/2,  /*data=*/{1,2, 3,_, 4, 5}},
            /*metal=*/     {/*baseAlign=*/2,  /*data=*/{1,2, 3,_, 4, 5}}
         },
         /*preStructAlignments=*/{
            /*std140=*/16,
            /*std140-f16=*/16,
            /*std430=*/8,
            /*std430-f16=*/4,
            /*metal=*/ 4
         }}
    };

    auto writeFields = [](UniformManager* mgr, SkSpan<const Uniform> fields, uint32_t baseValue) {
        auto gen16Bit = [mgr](int baseValue) {
            if (LayoutRules::UseFullPrecision(mgr->layout())) {
                return SkBits2Float(baseValue);
            } else {
                return SkHalfToFloat((SkHalf) baseValue);
            }
        };
        auto gen32Bit = [](int baseValue) { return SkBits2Float(baseValue | (baseValue << 16)); };

        for (const Uniform& f : fields) {
            switch(f.type()) {
                case SkSLType::kHalf:
                    mgr->writeHalf(gen16Bit(baseValue++));
                    break;

                case SkSLType::kFloat:
                    mgr->write(gen32Bit(baseValue++));
                    break;

                case SkSLType::kHalf2:
                    mgr->writeHalf(SkV2{gen16Bit(baseValue++),
                                        gen16Bit(baseValue++)});
                    break;
                case SkSLType::kFloat2:
                    mgr->write(SkV2{gen32Bit(baseValue++),
                                    gen32Bit(baseValue++)});
                    break;

                case SkSLType::kHalf3:
                    mgr->writeHalf(SkV3{gen16Bit(baseValue++),
                                        gen16Bit(baseValue++),
                                        gen16Bit(baseValue++)});
                    break;

                case SkSLType::kFloat3:
                    mgr->write(SkV3{gen32Bit(baseValue++),
                                    gen32Bit(baseValue++),
                                    gen32Bit(baseValue++)});
                    break;

                case SkSLType::kHalf4:
                    mgr->writeHalf(SkV4{gen16Bit(baseValue++),
                                        gen16Bit(baseValue++),
                                        gen16Bit(baseValue++),
                                        gen16Bit(baseValue++)});
                    break;
                case SkSLType::kFloat4:
                    mgr->write(SkV4{gen32Bit(baseValue++),
                                    gen32Bit(baseValue++),
                                    gen32Bit(baseValue++),
                                    gen32Bit(baseValue++)});
                    break;
                default:
                    SkUNREACHABLE;
            }
        }
        return baseValue;
    };

    bool dataMatchFailureLogged = false;
    for (size_t l = 0; l < std::size(kLayouts); ++l) {
        const Layout layout = kLayouts[l];
        skiatest::ReporterContext layoutLabel(r, LayoutString(layout));

        for (size_t t = 0; t < std::size(kCases); ++t) {
            const TestCase& test = kCases[t];
            skiatest::ReporterContext testLabel(r, std::to_string(t));

            auto [baseAlignment, expectedData] = test.fExpectedAlignmentAndData[l];

            UniformManager mgr{layout};
            int baseValue = 1;
            if (!test.fPreStruct.empty()) {
                // pre-struct fields
                const bool preStructIsStruct = !test.fPreStructAlignments.empty();
                SkDEBUGCODE(mgr.setExpectedUniforms(test.fPreStruct, preStructIsStruct);)
                if (preStructIsStruct) {
                    mgr.beginStruct(test.fPreStructAlignments[l]);
                }
                baseValue = writeFields(&mgr, test.fPreStruct, baseValue);
                if (preStructIsStruct) {
                    mgr.endStruct();
                }
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
            }
            if (!test.fSubstruct.empty()) {
                // substruct fields
                SkDEBUGCODE(mgr.setExpectedUniforms(test.fSubstruct, /*isSubstruct=*/true);)
                mgr.beginStruct(baseAlignment);
                baseValue = writeFields(&mgr, test.fSubstruct, baseValue);
                mgr.endStruct();
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
            }
            if (!test.fPostStruct.empty()) {
                // post-struct fields
                SkDEBUGCODE(mgr.setExpectedUniforms(test.fPostStruct, /*isSubstruct=*/false);)
                baseValue = writeFields(&mgr, test.fPostStruct, baseValue);
                SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
            }

            SkSpan<const char> data = mgr.finish();

            bool sizeMatch = data.size() == sizeof(uint16_t)*expectedData.size();
            // To reduce logging/asserts, pretend contents "match" if the sizes differ since that
            // will already be triggering test failures
            bool contentsMatch = !sizeMatch ||
                                 memcmp(data.data(), expectedData.data(), data.size()) == 0;
            REPORTER_ASSERT(r, sizeMatch, "Size mismatch between written (%zu) and expected (%zu)",
                            data.size(), sizeof(uint16_t)*expectedData.size());
            REPORTER_ASSERT(r, contentsMatch, "Contents differ between written and expected");

            if (!contentsMatch && !dataMatchFailureLogged) {
                // Print out actual and expected values once if it's only the contents that are
                // incorrect (don't bother printing contents if their lengths differ).
                SkDebugf("Expected contents:\n");
                for (size_t i = 0; i < expectedData.size(); ++i) {
                    SkDebugf("%s%u", i % 8 == 0 ? " " : (i % 2 == 0 ? "," : ""), expectedData[i]);
                }
                SkDebugf("\nActual contents:\n");
                SkASSERT(data.size() % 8 == 0);
                const uint16_t* actualData = reinterpret_cast<const uint16_t*>(data.data());
                for (size_t i = 0; i < expectedData.size(); ++i) {
                    SkDebugf("%s%u", i % 8 == 0 ? " " : (i % 2 == 0 ? "," : ""), actualData[i]);
                }
                SkDebugf("\n\n");
                dataMatchFailureLogged = true;
            }
        }
    }
}
