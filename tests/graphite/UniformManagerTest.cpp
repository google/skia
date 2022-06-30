/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkHalf.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkUniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "tests/Test.h"

using Layout = skgpu::graphite::Layout;
using UniformManager = skgpu::graphite::UniformManager;

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

DEF_TEST(UniformManagerCheckSingleUniform, r) {
    // Verify that the uniform manager can hold all the basic uniform types, in every layout.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, 1, kFloats);
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
            if (!SkSLTypeIsFloatType(type) || vecLength < 0) {
                continue;
            }

            // Write our uniform float scalar/vector.
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, 1, kFloats);

            // Read back the uniform data.
            SkUniformDataBlock uniformData = mgr.peekData();
            if (layout == Layout::kMetal && !SkSLTypeIsFullPrecisionNumericType(type)) {
                // Metal should encode half-precision float uniforms in 16-bit floats.
                REPORTER_ASSERT(r, uniformData.size() >= vecLength * sizeof(SkHalf));
                REPORTER_ASSERT(r, 0 == memcmp(kHalfs, uniformData.data(),
                                               vecLength * sizeof(SkHalf)),
                                "Layout:%d Type:%d encoding failed", (int)layout, (int)type);
            } else {
                // Other layouts should always encode float uniforms in full precision.
                REPORTER_ASSERT(r, uniformData.size() >= vecLength * sizeof(float));
                REPORTER_ASSERT(r, 0 == memcmp(kFloats, uniformData.data(),
                                               vecLength * sizeof(float)),
                                "Layout:%d Type:%d encoding failed", (int)layout, (int)type);
            }

            mgr.reset();
        }
    }
}
