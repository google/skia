/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileBasePriv_DEFINED
#define skgpu_graphite_precompile_PrecompileBasePriv_DEFINED

#include "src/gpu/graphite/PrecompileInternal.h"

namespace skgpu::graphite {

/** Class that exposes methods to PrecompileBase that are only intended for use internal to Skia.
    This class is purely a privileged window into PrecompileBase. It should never have additional
    data members or virtual methods. */
class PrecompileBasePriv {
public:
    int numChildCombinations() const {
        return fPrecompileBase->numChildCombinations();
    }

    int numCombinations() const {
        return fPrecompileBase->numCombinations();
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const {
        fPrecompileBase->addToKey(keyContext, builder, gatherer, desiredCombination);
    }

private:
    friend class PrecompileBase; // to construct/copy this type.

    explicit PrecompileBasePriv(PrecompileBase* precompileBase)
            : fPrecompileBase(precompileBase) {
    }

    PrecompileBasePriv& operator=(const PrecompileBasePriv&) = delete;

    // No taking addresses of this type.
    const PrecompileBasePriv* operator&() const;
    PrecompileBasePriv *operator&();

    PrecompileBase* fPrecompileBase;
};

inline PrecompileBasePriv PrecompileBase::priv() { return PrecompileBasePriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileBasePriv PrecompileBase::priv() const {
    return PrecompileBasePriv(const_cast<PrecompileBase *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileBasePriv_DEFINED
