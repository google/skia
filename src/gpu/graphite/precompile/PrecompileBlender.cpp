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
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"

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

        AddBlendMode(keyContext, builder, gatherer, fBlendMode);
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

//--------------------------------------------------------------------------------------------------
PrecompileBlenderList::PrecompileBlenderList(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
    for (const auto& b : blenders) {
        if (!b) {
            // A null SkBlender is equivalent to kSrcOver
            fHasPorterDuffBlender = true;
        } else if (b->priv().asBlendMode().has_value()) {
            SkBlendMode bm = b->priv().asBlendMode().value();

            SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
            if (!coeffs.empty()) {
                fHasPorterDuffBlender = true;
            } else if (bm >= SkBlendMode::kHue) {
                fHasHSLCBlender = true;
            } else {
                SkASSERT(b->priv().numCombinations() == 1);
                fFixedBlenderEffects.push_back(b); // No reduced shader snippet for this blend mode
                fNumCombos++;
            }
        } else {
            // Runtime blenders are always fixed
            fFixedBlenderEffects.push_back(b);
            fNumCombos += b->priv().numCombinations();
        }
    }

    if (!fHasPorterDuffBlender && !fHasHSLCBlender && fFixedBlenderEffects.empty()) {
        fHasPorterDuffBlender = true; // Fallback to kSrcOver
    }

    if (fHasPorterDuffBlender) {
        fNumCombos++;
    }
    if (fHasHSLCBlender) {
        fNumCombos++;
    }
}

PrecompileBlenderList::PrecompileBlenderList(SkSpan<const SkBlendMode> blendModes) {
    for (SkBlendMode bm : blendModes) {
        SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
        if (!coeffs.empty()) {
            fHasPorterDuffBlender = true;
        } else if (bm >= SkBlendMode::kHue) {
            fHasHSLCBlender = true;
        } else {
            fFixedBlenderEffects.push_back(PrecompileBlenders::Mode(bm));
        }
    }

    if (!fHasPorterDuffBlender && !fHasHSLCBlender && fFixedBlenderEffects.empty()) {
        fHasPorterDuffBlender = true; // Fallback to kSrcOver
    }

    fNumCombos = fHasPorterDuffBlender + fHasHSLCBlender + SkTo<int>(fFixedBlenderEffects.size());
}

std::pair<sk_sp<PrecompileBlender>, int> PrecompileBlenderList::selectOption(
        int desiredCombination) const {
    SkASSERT(desiredCombination >= 0 && desiredCombination < this->numCombinations());

    if (fHasPorterDuffBlender) {
        if (desiredCombination == 0) {
            // Porter-Duff constant consolidated blend option, pick kSrcOver as a stand-in
            return {PrecompileBlenders::Mode(SkBlendMode::kSrcOver), 0};
        } else {
            desiredCombination--;
        }
    }

    if (fHasHSLCBlender) {
        if (desiredCombination == 0) {
            // HSLC blend option, pick kHue arbitrarily
            return {PrecompileBlenders::Mode(SkBlendMode::kHue), 0};
        } else {
            desiredCombination--;
        }
    }

    if (!fFixedBlenderEffects.empty()) {
        auto [option, childCombination] =
                PrecompileBase::SelectOption<PrecompileBlender>(fFixedBlenderEffects,
                                                                desiredCombination);

        // Double-check that we aren't returning a blend mode that should have been consolidated.
        SkDEBUGCODE(auto bm = option->priv().asBlendMode();)
        SkASSERT(!bm || (*bm > SkBlendMode::kXor && *bm < SkBlendMode::kHue));
        return {option, childCombination};
    }

    SkUNREACHABLE;
}


} // namespace skgpu::graphite
