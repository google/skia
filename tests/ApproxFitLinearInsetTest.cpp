/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkBackingFit.h"
#include "tests/Test.h"

DEF_TEST(ApproxFitLinearInset, r) {
    static constexpr int kMaxTextureSize = 16384;

#define ASSERT_OR_BREAK(cond, ...) \
    do { if (!(cond)) { \
        REPORTER_ASSERT(r, cond, __VA_ARGS__); \
        break; \
    } } while(0)

    for (int dim = 1; dim <= kMaxTextureSize; dim++) {
        float logicalClampEdge = dim - 0.5f;
        ASSERT_OR_BREAK(logicalClampEdge < dim,
                        "Cannot even represent 1/2pixel inset at dim = %d!", dim);

        float safeClampEdge = dim - skgpu::kLinearInset;
        ASSERT_OR_BREAK(safeClampEdge < logicalClampEdge,
                        "Safe inset is not preserved at dim = %d", dim);

        // Ensure that the condition holds after normalizing coordinates too
        float invDim = 1.0f / dim;
        logicalClampEdge *= invDim;
        safeClampEdge *= invDim;
        ASSERT_OR_BREAK(safeClampEdge < logicalClampEdge,
                        "Normalized safe inset is not preserved at dim = %d", dim);
    }

#undef ASSERT_OR_BREAK
}
