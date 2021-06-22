/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/effects/GrGradientEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"

namespace GrGradientColorizer {

std::unique_ptr<GrFragmentProcessor> SingleInterval(SkPMColor4f start, SkPMColor4f end) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform half4 start;
        uniform half4 end;
        half4 main(float2 coord) {
            // Clamping and/or wrapping was already handled by the parent shader so the output
            // color is a simple lerp.
            return mix(start, end, half(coord.x));
        }
    )");
    return GrSkSLFP::Make(effect, "SingleInterval", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
                          "start", start,
                          "end", end);
}

}  // namespace GrGradientColorizer
