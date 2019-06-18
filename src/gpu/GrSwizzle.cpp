/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSwizzle.h"

#include "src/core/SkRasterPipeline.h"

void GrSwizzle::apply(SkRasterPipeline* pipeline) const {
    SkASSERT(pipeline);
    switch (fKey) {
        case GrSwizzle::RGBA().asKey():
            return;
        case GrSwizzle::BGRA().asKey():
            pipeline->append(SkRasterPipeline::swap_rb);
            return;
        default: {
            GR_STATIC_ASSERT(sizeof(uintptr_t) >= 4 * sizeof(char));
            // Rather than allocate the 4 control bytes on the heap somewhere, just jam them right
            // into a uintptr_t context.
            uintptr_t ctx;
            memcpy(&ctx, fSwiz, 4 * sizeof(char));
            pipeline->append(SkRasterPipeline::swizzle, ctx);
            return;
        }
    }
}
