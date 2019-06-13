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
        case GrSwizzle("rgba").asKey():
            return;
        case GrSwizzle("bgra").asKey():
            pipeline->append(SkRasterPipeline::swap_rb);
            return;
        case GrSwizzle("aaa1").asKey():
            pipeline->append(SkRasterPipeline::alpha_to_gray);
            return;
        case GrSwizzle("rgb1").asKey():
            pipeline->append(SkRasterPipeline::force_opaque);
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
