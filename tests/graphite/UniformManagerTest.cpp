/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkHalf.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkUniform.h"
#include "src/gpu/graphite/PipelineData.h"
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
        SkSLType::kShort,    SkSLType::kShort2,   SkSLType::kShort3,   SkSLType::kShort4,   //
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

static constexpr int16_t kShorts[16] = { 1,  -2,  3,  -4,
                                         5,  -6,  7,  -8,
                                         9, -10, 11, -12,
                                        13, -14, 15, -16 };

static constexpr int32_t kInts[16] = { 1,  -2,  3,  -4,
                                       5,  -6,  7,  -8,
                                       9, -10, 11, -12,
                                      13, -14, 15, -16 };

static size_t element_size(Layout layout, SkSLType type) {
    // Metal should encode half-precision uniforms in 16 bits.
    // Other layouts should always encode uniforms in 32 bit.
    return (layout == Layout::kMetal && !SkSLTypeIsFullPrecisionNumericType(type)) ? 2 : 4;
}

DEF_TEST(UniformManagerCheckSingleUniform, r) {
    // Verify that the uniform manager can hold all the basic uniform types, in every layout.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, kFloats);
            mgr.doneWithExpectedUniforms();
            REPORTER_ASSERT(r, mgr.size() > 0);
            mgr.reset();
        }
    }
}

DEF_TEST(UniformManagerCheckFloatEncoding, r) {
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
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, kFloats);
            mgr.doneWithExpectedUniforms();

            // Read back the uniform data.
            UniformDataBlock uniformData = mgr.finishUniformDataBlock();
            size_t elementSize = element_size(layout, type);
            const void* validData = (elementSize == 4) ? (const void*)kFloats : (const void*)kHalfs;
            REPORTER_ASSERT(r, uniformData.size() >= vecLength * elementSize);
            REPORTER_ASSERT(r, 0 == memcmp(validData, uniformData.data(), vecLength * elementSize),
                            "Layout:%d Type:%d float encoding failed", (int)layout, (int)type);
            mgr.reset();
        }
    }
}

DEF_TEST(UniformManagerCheckIntEncoding, r) {
    // Verify that the uniform manager encodes int data properly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            if (!SkSLTypeIsIntegralType(type)) {
                continue;
            }

            // Write our uniform int scalar/vector.
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, kInts);
            mgr.doneWithExpectedUniforms();

            // Read back the uniform data.
            UniformDataBlock uniformData = mgr.finishUniformDataBlock();
            int vecLength = SkSLTypeVecLength(type);
            size_t elementSize = element_size(layout, type);
            const void* validData = (elementSize == 4) ? (const void*)kInts : (const void*)kShorts;
            REPORTER_ASSERT(r, uniformData.size() >= vecLength * elementSize);
            REPORTER_ASSERT(r, 0 == memcmp(validData, uniformData.data(), vecLength * elementSize),
                            "Layout:%d Type:%d int encoding failed", (int)layout, (int)type);
            mgr.reset();
        }
    }
}

DEF_TEST(UniformManagerCheckScalarVectorPacking, r) {
    // Verify that the uniform manager can pack scalars and vectors of identical type correctly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            int vecLength = SkSLTypeVecLength(type);
            if (vecLength < 1) {
                continue;
            }

            // Write three matching uniforms.
            const SkUniform expectations[] = {{"a", type}, {"b", type}, {"c", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, kFloats);
            mgr.write(type, kFloats);
            mgr.write(type, kFloats);
            mgr.doneWithExpectedUniforms();

            // Verify that the uniform data was packed as tight as it should be.
            UniformDataBlock uniformData = mgr.finishUniformDataBlock();
            size_t elementSize = element_size(layout, type);
            // Vec3s should be packed as if they were vec4s.
            size_t effectiveVecLength = (vecLength == 3) ? 4 : vecLength;
            REPORTER_ASSERT(r, uniformData.size() == elementSize * effectiveVecLength * 3,
                            "Layout:%d Type:%d tight packing failed", (int)layout, (int)type);
            mgr.reset();
        }
    }
}

DEF_TEST(UniformManagerCheckMatrixPacking, r) {
    // Verify that the uniform manager can pack matrices correctly.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            int matrixSize = SkSLTypeMatrixSize(type);
            if (matrixSize < 2) {
                continue;
            }

            // Write three matching uniforms.
            const SkUniform expectations[] = {{"a", type}, {"b", type}, {"c", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, kFloats);
            mgr.write(type, kFloats);
            mgr.write(type, kFloats);
            mgr.doneWithExpectedUniforms();

            // Verify that the uniform data was packed as tight as it should be.
            UniformDataBlock uniformData = mgr.finishUniformDataBlock();
            size_t elementSize = element_size(layout, type);
            // In all layouts, mat3s should burn 12 elements, not 9.
            size_t numElements = (matrixSize == 3) ? 12 : (matrixSize * matrixSize);
            REPORTER_ASSERT(r, uniformData.size() == elementSize * numElements * 3,
                            "Layout:%d Type:%d matrix packing failed", (int)layout, (int)type);
            mgr.reset();
        }
    }
}

DEF_TEST(UniformManagerCheckPaddingScalarVector, r) {
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
                const SkUniform expectations[] = {{"a", type1}, {"b", type2}};
                mgr.setExpectedUniforms(SkSpan(expectations));
                mgr.write(type1, kFloats);
                mgr.write(type2, kFloats);
                mgr.doneWithExpectedUniforms();

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (either 16 or 32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "",         "",         "",         ""         },
                        { "", "AB",       "A_BB",     "A___BBBb", "A___BBBB" },
                        { "", "AAB_",     "AABB",     "AA__BBBb", "AA__BBBB" },
                        { "", "AAAaB___", "AAAaBB__", "AAAaBBBb", "AAAaBBBB" },
                        { "", "AAAAB___", "AAAABB__", "AAAABBBb", "AAAABBBB" },
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][vecLength2]) *
                                        elementSize1;
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout:%d Types:%d %d padding test failed",
                                    (int)layout, (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout:%d Types:%d %d padding test failed",
                                    (int)layout, (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Layout:%d Types:%d %d padding test failed",
                                    (int)layout, (int)type1, (int)type2);
                } else {
                    ERRORF(r, "Unexpected element sizes: %zu %zu", elementSize1, elementSize2);
                }
                mgr.reset();
            }
        }
    }
}

DEF_TEST(UniformManagerCheckPaddingVectorMatrix, r) {
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
                const SkUniform expectations[] = {{"a", type1}, {"b", type2}};
                mgr.setExpectedUniforms(SkSpan(expectations));
                mgr.write(type1, kFloats);
                mgr.write(type2, kFloats);
                mgr.doneWithExpectedUniforms();

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[5][5] = {
                        { "", "", "",         "",                     "" },
                        { "", "", "A_BBBB",   "A___BBBbBBBbBBBb", "A___BBBBBBBBBBBBBBBB" },
                        { "", "", "AABBBB",   "AA__BBBbBBBbBBBb", "AA__BBBBBBBBBBBBBBBB" },
                        { "", "", "AAAaBBBB", "AAAaBBBbBBBbBBBb", "AAAaBBBBBBBBBBBBBBBB" },
                        { "", "", "AAAABBBB", "AAAABBBbBBBbBBBb", "AAAABBBBBBBBBBBBBBBB" },
                    };
                    const size_t size = strlen(kExpectedLayout[vecLength1][matSize2]) *
                                        elementSize1;
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d vector-matrix padding test failed",
                                    (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d vector-matrix padding test failed",
                                    (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d vector-matrix padding test failed",
                                    (int)type1, (int)type2);
                }
                mgr.reset();
            }
        }
    }
}

DEF_TEST(UniformManagerCheckPaddingMatrixVector, r) {
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
                const SkUniform expectations[] = {{"a", type1}, {"b", type2}};
                mgr.setExpectedUniforms(SkSpan(expectations));
                mgr.write(type1, kFloats);
                mgr.write(type2, kFloats);
                mgr.doneWithExpectedUniforms();

                // The expected packing varies depending on the bit-widths of each element.
                const size_t elementSize1 = element_size(layout, type1);
                const size_t elementSize2 = element_size(layout, type2);
                if (elementSize1 == elementSize2) {
                    // Elements in the array correspond to the element size (32 bits).
                    // The expected uniform layout is listed as strings below.
                    // A/B: uniform values.
                    // a/b: padding as part of the uniform type (vec3 takes 4 slots)
                    // _  : padding between uniforms for alignment
                    static constexpr const char* kExpectedLayout[5][5] = {
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
                    };
                    const size_t size = strlen(kExpectedLayout[matSize1][vecLength2]) *
                                        elementSize1;
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d matrix-vector padding test failed",
                                    (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d matrix-vector padding test failed",
                                    (int)type1, (int)type2);
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
                    UniformDataBlock uniformData = mgr.finishUniformDataBlock();
                    REPORTER_ASSERT(r, uniformData.size() == size,
                                    "Types:%d %d matrix-vector padding test failed",
                                    (int)type1, (int)type2);
                }
                mgr.reset();
            }
        }
    }
}

DEF_TEST(UniformManagerMetalArrayLayout, r) {
    UniformManager mgr(Layout::kMetal);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Elements in the array below correspond to 16 bits apiece.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type (vec3 takes 4 slots)
        // _  : padding between uniforms for alignment.

        /* {half, short[3]}  */ "AABBBBBB",
        /* {half, short2[3]} */ "AA__BBBBBBBBBBBB",
        /* {half, short3[3]} */ "AA______BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, short4[3]} */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float[3]}  */ "AA__BBBBBBBBBBBB",
        /* {half, float2[3]} */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3[3]} */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */ "AABBBBBB",
        /* {half, half2[3]}  */ "AA__BBBBBBBBBBBB",
        /* {half, half3[3]}  */ "AA______BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4[3]}  */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int[3]}    */ "AA__BBBBBBBBBBBB",
        /* {half, int2[3]}   */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int3[3]}   */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3] */ "AA__BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3x3[3] */ "AA______"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb"
                                 "BBBBBBbbBBBBBBbbBBBBBBbb",
        /* {half, half4x4[3] */ "AA______"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    for (size_t i = 0; i < std::size(kExpectedLayout); i++) {
        const SkSLType arrayType = kTypes[i];
        const SkUniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        mgr.setExpectedUniforms(SkSpan(expectations));
        mgr.write(SkSLType::kHalf, kHalfs);
        mgr.writeArray(arrayType, kBuffer, kArraySize);
        mgr.doneWithExpectedUniforms();

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        const UniformDataBlock uniformData = mgr.finishUniformDataBlock();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

DEF_TEST(UniformManagerStd430ArrayLayout, r) {
    UniformManager mgr(Layout::kStd430);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Elements in the array below correspond to 16 bits apiece.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type (vec3 takes 4 slots)
        // _  : padding between uniforms for alignment.

        /* {half, short[3]}  */ "AA__BBBBBBBBBBBB",
        /* {half, short2[3]} */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, short3[3]} */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, short4[3]} */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float[3]}  */ "AA__BBBBBBBBBBBB",
        /* {half, float2[3]} */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3[3]} */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */ "AA__BBBBBBBBBBBB",
        /* {half, half2[3]}  */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half3[3]}  */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, half4[3]}  */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int[3]}    */ "AA__BBBBBBBBBBBB",
        /* {half, int2[3]}   */ "AA______BBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, int3[3]}   */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

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
        const SkUniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        mgr.setExpectedUniforms(SkSpan(expectations));
        mgr.write(SkSLType::kHalf, kHalfs);
        mgr.writeArray(arrayType, kBuffer, kArraySize);
        mgr.doneWithExpectedUniforms();

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        const UniformDataBlock uniformData = mgr.finishUniformDataBlock();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}

DEF_TEST(UniformManagerStd140ArrayLayout, r) {
    UniformManager mgr(Layout::kStd140);

    // Tests set up a uniform block with a single half (to force alignment) and an array of 3
    // elements. Test every type that can appear in an array.
    constexpr size_t kArraySize = 3;

    // Buffer large enough to hold a float4x4[3] array.
    static constexpr uint8_t kBuffer[192] = {};
    static const char* kExpectedLayout[] = {
        // Elements in the array below correspond to 16 bits apiece.
        // The expected uniform layout is listed as strings below.
        // A/B: uniform values.
        // a/b: padding as part of the uniform type (vec3 takes 4 slots)
        // _  : padding between uniforms for alignment.

        /* {half, short[3]}  */ "AA______________BBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbb",
        /* {half, short2[3]} */ "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, short3[3]} */ "AA______________BBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbb",
        /* {half, short4[3]} */ "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float[3]}  */ "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, float2[3]} */ "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, float3[3]} */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4[3]} */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, half[3]}   */ "AA______________BBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbbBBbbbbbbbbbbbbbb",
        /* {half, half2[3]}  */ "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, half3[3]}  */ "AA______________BBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbbBBBBBBbbbbbbbbbb",
        /* {half, half4[3]}  */ "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int[3]}    */ "AA______________BBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbbBBBBbbbbbbbbbbbb",
        /* {half, int2[3]}   */ "AA______________BBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbbBBBBBBBBbbbbbbbb",
        /* {half, int3[3]}   */ "AA______________BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, int4[3]}   */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, float2x2[3] */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        /* {half, float3x3[3] */ "AA______________"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb"
                                 "BBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbbBBBBBBBBBBBBbbbb",
        /* {half, float4x4[3] */ "AA______________"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                                 "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",

        /* {half, half2x2[3]  */ "AA______________BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
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
        const SkUniform expectations[] = {{"a", SkSLType::kHalf}, {"b", arrayType, kArraySize}};

        mgr.setExpectedUniforms(SkSpan(expectations));
        mgr.write(SkSLType::kHalf, kHalfs);
        mgr.writeArray(arrayType, kBuffer, kArraySize);
        mgr.doneWithExpectedUniforms();

        const size_t expectedSize = strlen(kExpectedLayout[i]);
        const UniformDataBlock uniformData = mgr.finishUniformDataBlock();
        REPORTER_ASSERT(r, uniformData.size() == expectedSize,
                        "array test %d for type %s failed - expected size: %zu, actual size: %zu",
                        (int)i, SkSLTypeString(arrayType), expectedSize, uniformData.size());

        mgr.reset();
    }
}
