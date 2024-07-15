/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileBlender.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/graphite/PaintParams.h"

namespace skgpu::graphite {

PrecompileBlender::~PrecompileBlender() = default;

//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeBlender final : public PrecompileBlender {
public:
    PrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

protected:
    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const final {
        SkASSERT(desiredCombination == 0); // The blend mode blender only ever has one combination

        AddModeBlend(keyContext, builder, gatherer, fBlendMode);
    }

private:
    SkBlendMode fBlendMode;
};

sk_sp<PrecompileBlender> PrecompileBlenders::Mode(SkBlendMode blendMode) {
    return sk_make_sp<PrecompileBlendModeBlender>(blendMode);
}

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileBlender> PrecompileBlenders::Arithmetic() {
    const SkRuntimeEffect* arithmeticEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kArithmetic);

    return PrecompileRuntimeEffects::MakePrecompileBlender(sk_ref_sp(arithmeticEffect));
}

} // namespace skgpu::graphite
