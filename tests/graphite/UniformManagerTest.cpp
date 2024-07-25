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
        Layout::kStd430,
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
    // Metal encodes half-precision uniforms in 16 bits.
    // Other layouts are expected to encode uniforms in 32 bits.
    return (layout == Layout::kMetal && !SkSLTypeIsFullPrecisionNumericType(type)) ? 2 : 4;
}

DEF_GRAPHITE_TEST(UniformManagerCheckSingleUniform, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerCheckFloatEncoding, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerCheckIntEncoding, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerCheckScalarVectorPacking, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerCheckMatrixPacking, r, CtsEnforcement::kApiLevel_V) {
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
            size_t elementSize = element_size(layout, type);
            // In all layouts, mat3s burn 12 elements, not 9. In std140, mat2s burn 8 elements
            // instead of 4.
            size_t numElements;
            if (matrixSize == 3) {
                numElements = 12;
            } else if (matrixSize == 2 && layout == Layout::kStd140) {
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

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingScalarVector, r, CtsEnforcement::kApiLevel_V) {
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
                    int layoutIdx = static_cast<int>(layout != Layout::kMetal);
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
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "",         "",         "",                 ""         },
                        { "", "A_BB",     "A___BBBB", "A_______BBBBBBbb", "A_______BBBBBBBB" },
                        { "", "AABB",     "AA__BBBB", "AA______BBBBBBbb", "AA______BBBBBBBB" },
                        { "", "AAAaBB__", "AAAaBBBB", "AAAa____BBBBBBbb", "AAAa____BBBBBBBB" },
                        { "", "AAAABB__", "AAAABBBB", "AAAA____BBBBBBbb", "AAAA____BBBBBBBB" },
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][vecLength2]) * 2;
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
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "", "", "", "" },
                        { "", "AAB_",     "AABB",     "AA__BBBb", "AA__BBBB" },
                        { "", "AAAAB___", "AAAABB__", "AAAABBBb", "AAAABBBB" },
                        { "",
                          "AAAAAAaaB_______",
                          "AAAAAAaaBB______",
                          "AAAAAAaaBBBb____",
                          "AAAAAAaaBBBB____" },
                        { "",
                          "AAAAAAAAB_______",
                          "AAAAAAAABB______",
                          "AAAAAAAABBBb____",
                          "AAAAAAAABBBB____" },
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][vecLength2]) * 2;
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

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingVectorMatrix, r, CtsEnforcement::kApiLevel_V) {
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
                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140 layout
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
                    int layoutIdx = static_cast<int>(layout != Layout::kStd140);
                    const size_t size = strlen(kExpectedLayout[layoutIdx][vecLength1][matSize2]) *
                                        elementSize1;
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
                    static constexpr const char* kExpectedLayout[5][5] = {
                            {"", "", "", "", ""},
                            {"", "",
                             "A___BBBBBBBB",
                             "A_______BBBBBBbbBBBBBBbbBBBBBBbb",
                             "A_______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "",
                             "AA__BBBBBBBB",
                             "AA______BBBBBBbbBBBBBBbbBBBBBBbb",
                             "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "",
                             "AAAaBBBBBBBB",
                             "AAAa____BBBBBBbbBBBBBBbbBBBBBBbb",
                             "AAAa____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                            {"", "",
                             "AAAABBBBBBBB",
                             "AAAA____BBBBBBbbBBBBBBbbBBBBBBbb",
                             "AAAA____BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][matSize2]) * 2;
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
                    static constexpr const char* kExpectedLayout[5][5] = {
                            {"", "", "", "", ""},
                            {"", "", "AABBBB",   "AA__BBBbBBBbBBBb", "AA__BBBBBBBBBBBBBBBB"},
                            {"", "", "AAAABBBB", "AAAABBBbBBBbBBBb", "AAAABBBBBBBBBBBBBBBB"},
                            {"", "",
                             "AAAAAAaaBBBB____",
                             "AAAAAAaaBBBbBBBbBBBb____",
                             "AAAAAAaaBBBBBBBBBBBBBBBB"},
                            {"", "",
                             "AAAAAAAABBBB____",
                             "AAAAAAAABBBbBBBbBBBb____",
                             "AAAAAAAABBBBBBBBBBBBBBBB"},
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][matSize2]) * 2;
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

DEF_GRAPHITE_TEST(UniformManagerCheckPaddingMatrixVector, r, CtsEnforcement::kApiLevel_V) {
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
                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[2][5][5] = {
                        // std140 layout
                        {
                            { "", "", "", "", "" },
                            { "", "", "", "", "" },
                            { "", "AAaaAAaaB___", "AAaaAAaaBB__", "AAaaAAaaBBBb", "AAaaAAaaBBBB" },
                            { "",
                              "AAAaAAAaAAAaB___",
                              "AAAaAAAaAAAaBB__",
                              "AAAaAAAaAAAaBBBb",
                              "AAAaAAAaAAAaBBBB" },
                            { "",
                              "AAAAAAAAAAAAAAAAB___",
                              "AAAAAAAAAAAAAAAABB__",
                              "AAAAAAAAAAAAAAAABBBb",
                              "AAAAAAAAAAAAAAAABBBB" },
                        },
                        // All other layouts
                        {
                            { "", "", "", "", "" },
                            { "", "", "", "", "" },
                            { "", "AAAAB_", "AAAABB", "AAAABBBb", "AAAABBBB" },
                            { "",
                              "AAAaAAAaAAAaB___",
                              "AAAaAAAaAAAaBB__",
                              "AAAaAAAaAAAaBBBb",
                              "AAAaAAAaAAAaBBBB" },
                            { "",
                              "AAAAAAAAAAAAAAAAB___",
                              "AAAAAAAAAAAAAAAABB__",
                              "AAAAAAAAAAAAAAAABBBb",
                              "AAAAAAAAAAAAAAAABBBB" },
                        },
                    };
                    int layoutIdx = static_cast<int>(layout != Layout::kStd140);
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
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "", "", "", "" },
                        { "", "", "", "", "" },
                        { "", "AAAABB", "AAAABBBB", "AAAA____BBBBBBbb", "AAAA____BBBBBBBB" },
                        { "",
                          "AAAaAAAaAAAaBB__",
                          "AAAaAAAaAAAaBBBB",
                          "AAAaAAAaAAAa____BBBBBBbb",
                          "AAAaAAAaAAAa____BBBBBBBB" },
                        { "",
                          "AAAAAAAAAAAAAAAABB__",
                          "AAAAAAAAAAAAAAAABBBB",
                          "AAAAAAAAAAAAAAAABBBBBBbb",
                          "AAAAAAAAAAAAAAAABBBBBBBB" },
                    };
                    const size_t size = strlen(kExpectedLayout[matSize1][vecLength2]) * 2;
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
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "", "", "", "" },
                        { "", "", "", "", "" },
                        { "", "AAAAAAAAB___", "AAAAAAAABB__", "AAAAAAAABBBb", "AAAAAAAABBBB" },
                        { "",
                          "AAAAAAaaAAAAAAaaAAAAAAaaB_______",
                          "AAAAAAaaAAAAAAaaAAAAAAaaBB______",
                          "AAAAAAaaAAAAAAaaAAAAAAaaBBBb____",
                          "AAAAAAaaAAAAAAaaAAAAAAaaBBBB____" },
                        { "",
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB_______",
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABB______",
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBb____",
                          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB____" },
                    };
                    const size_t size = strlen(kExpectedLayout[matSize1][vecLength2]) * 2;
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

DEF_GRAPHITE_TEST(UniformManagerMetalArrayLayout, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerStd430ArrayLayout, r, CtsEnforcement::kApiLevel_V) {
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

DEF_GRAPHITE_TEST(UniformManagerStd140ArrayLayout, r, CtsEnforcement::kApiLevel_V) {
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

// This test validates that the uniform data for matrix types get written out according to the
// layout expectations.
DEF_GRAPHITE_TEST(UniformManagerStd140MatrixLayoutContents, r, CtsEnforcement::kApiLevel_V) {
    UniformManager mgr(Layout::kStd140);

    // float2x2, half2x2
    for (SkSLType type : {SkSLType::kFloat2x2, SkSLType::kHalf2x2}) {
        const Uniform expectations[] = {{"m", type}};
        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kFloats);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == 32,
                        "%s layout size expected 32, got %zu",
                        SkSLTypeString(type), uniformData.size());

        // The expected offsets of the 4 matrix elements.
        const int kOffsets[4] = {0, 1, 4, 5};
        const float* elements = reinterpret_cast<const float*>(uniformData.data());
        for (size_t i = 0u; i < std::size(kOffsets); ++i) {
            float expected = kFloats[i];
            float el = elements[kOffsets[i]];
            REPORTER_ASSERT(r, el == expected,
                            "Incorrect %s element %zu - expected %f, got %f",
                            SkSLTypeString(type), i, expected, el);
        }
        mgr.reset();
    }

    // float3x3, half3x3
    for (SkSLType type : {SkSLType::kFloat3x3, SkSLType::kHalf3x3}) {
        const Uniform expectations[] = {{"m", type}};
        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kFloats);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == 48,
                        "%s layout size expected 48, got %zu",
                        SkSLTypeString(type), uniformData.size());

        // The expected offsets of the 9 matrix elements.
        const int kOffsets[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
        const float* elements = reinterpret_cast<const float*>(uniformData.data());
        for (size_t i = 0u; i < std::size(kOffsets); ++i) {
            float expected = kFloats[i];
            float el = elements[kOffsets[i]];
            REPORTER_ASSERT(r, el == expected,
                            "Incorrect %s element %zu - expected %f, got %f",
                            SkSLTypeString(type), i, expected, el);
        }
        mgr.reset();
    }
}

// This test validates that the uniform data for matrix types get written out according to the
// layout expectations.
DEF_GRAPHITE_TEST(UniformManagerStd430MatrixLayoutContents, r, CtsEnforcement::kApiLevel_V) {
    UniformManager mgr(Layout::kStd430);

    // float2x2, half2x2
    for (SkSLType type : {SkSLType::kFloat2x2, SkSLType::kHalf2x2}) {
        const Uniform expectations[] = {{"m", type}};
        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kFloats);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == 16,
                        "%s layout size expected 16, got %zu",
                        SkSLTypeString(type), uniformData.size());

        // The expected offsets of the 4 matrix elements. This uses a tighter packing than std140
        // layout.
        const int kOffsets[4] = {0, 1, 2, 3};
        const float* elements = reinterpret_cast<const float*>(uniformData.data());
        for (size_t i = 0u; i < std::size(kOffsets); ++i) {
            float expected = kFloats[i];
            float el = elements[kOffsets[i]];
            REPORTER_ASSERT(r, el == expected,
                            "Incorrect %s element %zu - expected %f, got %f",
                            SkSLTypeString(type), i, expected, el);
        }
        mgr.reset();
    }

    // float3x3, half3x3
    for (SkSLType type : {SkSLType::kFloat3x3, SkSLType::kHalf3x3}) {
        const Uniform expectations[] = {{"m", type}};
        SkDEBUGCODE(mgr.setExpectedUniforms(SkSpan(expectations), /*isSubstruct=*/false);)
        mgr.write(expectations[0], kFloats);
        SkDEBUGCODE(mgr.doneWithExpectedUniforms();)
        SkSpan<const char> uniformData = mgr.finish();
        REPORTER_ASSERT(r, uniformData.size() == 48,
                        "%s layout size expected 48, got %zu",
                        SkSLTypeString(type), uniformData.size());

        // The expected offsets of the 9 matrix elements. This is the same as std140 layout.
        const int kOffsets[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
        const float* elements = reinterpret_cast<const float*>(uniformData.data());
        for (size_t i = 0u; i < std::size(kOffsets); ++i) {
            float expected = kFloats[i];
            float el = elements[kOffsets[i]];
            REPORTER_ASSERT(r, el == expected,
                            "Incorrect %s element %zu - expected %f, got %f",
                            SkSLTypeString(type), i, expected, el);
        }
        mgr.reset();
    }
}

// These tests validate that substructs are written and aligned appropriately.
DEF_GRAPHITE_TEST(UniformManagerStructLayout, r, CtsEnforcement::kNextRelease) {
    static constexpr uint32_t _ = 0; // 0s will only be written as padding
    static const struct TestCase {
        // For convenience, these should only have kFloat[2,3,4] as their types. However the values
        // written are integers and will be bit-punned to floats so that expectations are easy to
        // define (UniformManager and Skia have no defined kInt3 object type). Written integers
        // start at 1 and are incremented with each component across uniforms.
        std::vector<Uniform> fPreStruct;  // before beginStruct(), top-level fields or struct
        std::vector<Uniform> fSubstruct;  // within beginStruct()/endStruct()
        std::vector<Uniform> fPostStruct; // after endStruct(), top-level fields

        std::pair<int, std::vector<uint32_t>> fExpectedAlignmentAndData[std::size(kLayouts)];

        // If non-empty, holds base alignments for fPreStruct base alignment as a struct
        std::vector<int> fPreStructAlignments = {};
    } kCases[] = {
        // Struct tests with no preceeding or following top-level fields
        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1, 2, 3, 4}},
            /*std430=*/{/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4}},
            /*metal=*/ {/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,2,3, 4}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,2,3, 4}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,2,3,_, 4,_,_,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,2, 3, 4, 5,_,_,_}},
            /*std430=*/{/*baseAlign=*/8,  /*data=*/{1,2, 3, 4, 5,_}},
            /*metal=*/ {/*baseAlign=*/8,  /*data=*/{1,2, 3, 4, 5,_}}
         }},

        {/*prestruct=*/ {},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,5, 6,7,_,_, 8,9,10,_}}
         }},


        // Struct tests with a preceeding float to require padding to the struct's base alignment
        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2, 3, 4, 5}},
            /*std430=*/{/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4, 5}},
            /*metal=*/ {/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4, 5}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4, 5}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4, 5}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,_, 5,_,_,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3, 4, 5, 6,_,_,_}},
            /*std430=*/{/*baseAlign=*/8,  /*data=*/{1,_, 2,3, 4, 5, 6,_}},
            /*metal=*/ {/*baseAlign=*/8,  /*data=*/{1,_, 2,3, 4, 5, 6,_}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_}}
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
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2, 3, 4, 5, 6,7,8,9}},
            /*std430=*/{/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4, 5, _,_,_,6,7,8,9}},
            /*metal=*/ {/*baseAlign=*/4,  /*data=*/{1, 2, 3, 4, 5, _,_,_,6,7,8,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat3},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4, 5, 6,7,8,9}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4, 5, 6,7,8,9}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3,4,_, 5,_,_,_, 6,7,8,9}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat2},
                         {"u2", SkSLType::kFloat},
                         {"u3", SkSLType::kFloat},
                         {"u4", SkSLType::kFloat}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,3, 4, 5, 6,_,_,_, 7,8,9,10}},
            /*std430=*/{/*baseAlign=*/8,  /*data=*/{1,_, 2,3, 4, 5, 6,_, 7,8,9,10}},
            /*metal=*/ {/*baseAlign=*/8,  /*data=*/{1,_, 2,3, 4, 5, 6,_, 7,8,9,10}}
         }},

        {/*prestruct=*/ {{"p1", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat4},
                         {"u3", SkSLType::kFloat2},
                         {"u4", SkSLType::kFloat3}},
         /*poststruct=*/{{"p2", SkSLType::kFloat4}},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15}},
            /*std430=*/{/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15}},
            /*metal=*/ {/*baseAlign=*/16, /*data=*/{1,_,_,_, 2,_,_,_, 3,4,5,6, 7,8,_,_, 9,10,11,_, 12,13,14,15}}
         }},

        // Struct tests with two adjacent structs
        {/*prestruct=*/ {{"p1", SkSLType::kFloat2},
                         {"p2", SkSLType::kFloat}},
         /*substruct=*/ {{"u1", SkSLType::kFloat},
                         {"u2", SkSLType::kFloat}},
         /*poststruct=*/{},
         /*expect=*/{
            /*std140=*/{/*baseAlign=*/16, /*data=*/{1,2, 3,_, 4, 5,_,_}},
            /*std430=*/{/*baseAlign=*/4,  /*data=*/{1,2, 3,_, 4, 5}},
            /*metal=*/ {/*baseAlign=*/4,  /*data=*/{1,2, 3,_, 4, 5}}
         },
         /*preStructAlignments=*/{
            /*std140=*/16,
            /*std430=*/8,
            /*metal=*/ 8
         }}
    };

    auto writeFields = [](UniformManager* mgr, SkSpan<const Uniform> fields, uint32_t baseValue) {
        for (const Uniform& f : fields) {
            switch(f.type()) {
                case SkSLType::kFloat:
                    mgr->write(SkBits2Float(baseValue++));
                    break;
                case SkSLType::kFloat2:
                    mgr->write(SkV2{SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++)});
                    break;
                case SkSLType::kFloat3:
                    mgr->write(SkV3{SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++)});
                    break;
                case SkSLType::kFloat4:
                    mgr->write(SkV4{SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++),
                                    SkBits2Float(baseValue++)});
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

            bool sizeMatch = data.size() == sizeof(uint32_t)*expectedData.size();
            // To reduce logging/asserts, pretend contents "match" if the sizes differ since that
            // will already be triggering test failures
            bool contentsMatch = !sizeMatch ||
                                 memcmp(data.data(), expectedData.data(), data.size()) == 0;
            REPORTER_ASSERT(r, sizeMatch, "Size mismatch between written (%zu) and expected (%zu)",
                            data.size(), expectedData.size());
            REPORTER_ASSERT(r, contentsMatch, "Contents differ between written and expected");

            if (!contentsMatch && !dataMatchFailureLogged) {
                // Print out actual and expected values once if it's only the contents that are
                // incorrect (don't bother printing contents if their lengths differ).
                SkDebugf("Expected contents:\n");
                for (size_t i = 0; i < expectedData.size(); ++i) {
                    SkDebugf("%s%u", i % 4 == 0 ? " " : ",", expectedData[i]);
                }
                SkDebugf("\nActual contents:\n");
                SkASSERT(data.size() % 4 == 0);
                const uint32_t* actualData = reinterpret_cast<const uint32_t*>(data.begin());
                for (size_t i = 0; i < expectedData.size(); ++i) {
                    SkDebugf("%s%u", i % 4 == 0 ? " " : ",", actualData[i]);
                }
                SkDebugf("\n\n");
                dataMatchFailureLogged = true;
            }
        }
    }
}
