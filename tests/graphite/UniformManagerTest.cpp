/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

static constexpr uint8_t kZeroData[64] = {};

DEF_TEST(UniformManagerCheckSingleUniform, r) {
    // Verify that the uniform manager can hold all the basic uniform types, in every layout.
    for (Layout layout : kLayouts) {
        UniformManager mgr(layout);

        for (SkSLType type : kTypes) {
            const SkUniform expectations[] = {{"uniform", type}};
            mgr.setExpectedUniforms(SkSpan(expectations));
            mgr.write(type, 1, kZeroData);
            REPORTER_ASSERT(r, mgr.size() > 0);
            mgr.reset();
        }
    }
}
