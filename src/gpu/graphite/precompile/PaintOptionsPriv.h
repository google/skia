/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintOptionsPriv_DEFINED
#define skgpu_graphite_PaintOptionsPriv_DEFINED

#include "include/gpu/graphite/precompile/PaintOptions.h"

namespace skgpu::graphite {

class ShaderCodeDictionary;

/** Class that exposes methods in PaintOptions that are only intended for use internal to Skia.
    This class is purely a privileged window into PaintOptions. It should never have additional
    data members or virtual methods. */
class PaintOptionsPriv {
public:
    using ProcessCombination = PaintOptions::ProcessCombination;

    void addColorFilter(sk_sp<PrecompileColorFilter> cf);

    void setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders) {
        fPaintOptions->setClipShaders(std::move(clipShaders));
    }

    void setPrimitiveBlendMode(SkBlendMode primitiveBlendMode) {
        fPaintOptions->setPrimitiveBlendMode(primitiveBlendMode);
    }

    void setSkipColorXform(bool skipColorXform) {
        fPaintOptions->setSkipColorXform(skipColorXform);
    }

    int numCombinations() const {
        return fPaintOptions->numCombinations();
    }

    void buildCombinations(
            const KeyContext& keyContext,
            DrawTypeFlags drawTypes,
            bool withPrimitiveBlender,
            Coverage coverage,
            const RenderPassDesc& renderPassDesc,
            const ProcessCombination& processCombination) const {
        fPaintOptions->buildCombinations(keyContext, drawTypes, withPrimitiveBlender, coverage,
                                         renderPassDesc, processCombination);
    }

private:
    friend class PaintOptions; // to construct/copy this type.

    explicit PaintOptionsPriv(PaintOptions* paintOptions) : fPaintOptions(paintOptions) {}

    PaintOptionsPriv& operator=(const PaintOptionsPriv&) = delete;

    // No taking addresses of this type.
    const PaintOptionsPriv* operator&() const;
    PaintOptionsPriv *operator&();

    PaintOptions* fPaintOptions;
};

inline PaintOptionsPriv PaintOptions::priv() { return PaintOptionsPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const PaintOptionsPriv PaintOptions::priv() const {
    return PaintOptionsPriv(const_cast<PaintOptions *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_PaintOptionsPriv_DEFINED
