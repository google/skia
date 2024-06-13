/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileBlenderPriv_DEFINED
#define skgpu_graphite_PrecompileBlenderPriv_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBlender.h"

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

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileBlenderPriv_DEFINED
