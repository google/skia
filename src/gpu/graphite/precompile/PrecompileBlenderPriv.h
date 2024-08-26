/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileBlenderPriv_DEFINED
#define skgpu_graphite_PrecompileBlenderPriv_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBlender.h"

#include <vector>

namespace skgpu::graphite {

/** Class that exposes methods in PrecompileBlender that are only intended for use internal to Skia.
    This class is purely a privileged window into PrecompileBlender. It should never have additional
    data members or virtual methods. */
class PrecompileBlenderPriv {
public:
    std::optional<SkBlendMode> asBlendMode() const { return fPrecompileBlender->asBlendMode(); }

    // The remaining methods make this a viable standin for PrecompileBasePriv
    int numChildCombinations() const { return fPrecompileBlender->numChildCombinations(); }

    int numCombinations() const { return fPrecompileBlender->numCombinations(); }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const {
        fPrecompileBlender->addToKey(keyContext, builder, gatherer, desiredCombination);
    }

private:
    friend class PrecompileBlender; // to construct/copy this type.

    explicit PrecompileBlenderPriv(PrecompileBlender* precompileBlender)
            : fPrecompileBlender(precompileBlender) {}

    PrecompileBlenderPriv& operator=(const PrecompileBlenderPriv&) = delete;

    // No taking addresses of this type.
    const PrecompileBlenderPriv* operator&() const;
    PrecompileBlenderPriv *operator&();

    PrecompileBlender* fPrecompileBlender;
};

inline PrecompileBlenderPriv PrecompileBlender::priv() { return PrecompileBlenderPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileBlenderPriv PrecompileBlender::priv() const {
    return PrecompileBlenderPriv(const_cast<PrecompileBlender *>(this));
}

class PrecompileBlenderList {
public:
    PrecompileBlenderList(SkSpan<const sk_sp<PrecompileBlender>> blenders);
    PrecompileBlenderList(SkSpan<const SkBlendMode> blendModes);

    int numCombinations() const { return fNumCombos; }

    // For options that use a consolidated blend function, a representative blend mode is returned.
    // Blend modes passed directly to the list's constructor will be re-wrapped in a
    // PrecompileBlender that returns the correct value from asBlendMode().
    //
    // The representative blend mode is consistent with the block selection logic in AddBlendMode().
    std::pair<sk_sp<PrecompileBlender>, int> selectOption(int desiredCombination) const;

private:
    // Porter Duff and HSLC blend modes are removed, but any remaining SkBlendModes that do not
    // have a consolidated function must be fixed in the PaintParamsKey just like runtime blenders.
    std::vector<sk_sp<PrecompileBlender>> fFixedBlenderEffects;
    bool fHasPorterDuffBlender = false;
    bool fHasHSLCBlender = false;

    int fNumCombos = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileBlenderPriv_DEFINED
