/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/Swizzle.h"

#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"

namespace skgpu {

void Swizzle::apply(SkRasterPipeline* pipeline) const {
    SkASSERT(pipeline);
    switch (fKey) {
        case Swizzle("rgba").asKey():
            return;
        case Swizzle("bgra").asKey():
            pipeline->append(SkRasterPipelineOp::swap_rb);
            return;
        case Swizzle("aaa1").asKey():
            pipeline->append(SkRasterPipelineOp::alpha_to_gray);
            return;
        case Swizzle("rgb1").asKey():
            pipeline->append(SkRasterPipelineOp::force_opaque);
            return;
        case Swizzle("a001").asKey():
            pipeline->append(SkRasterPipelineOp::alpha_to_red);
            return;
        default: {
            static_assert(sizeof(uintptr_t) >= 4 * sizeof(char));
            // Rather than allocate the 4 control bytes on the heap somewhere, just jam them right
            // into a uintptr_t context.
            uintptr_t ctx;
            memcpy(&ctx, this->asString().c_str(), 4 * sizeof(char));
            pipeline->append(SkRasterPipelineOp::swizzle, ctx);
            return;
        }
    }
}

SkString Swizzle::asString() const {
    char swiz[5];
    uint16_t key = fKey;
    for (int i = 0; i < 4; ++i) {
        swiz[i] = IToC(key & 0xfU);
        key >>= 4;
    }
    swiz[4] = '\0';
    return SkString(swiz);
}

} // namespace skgpu
