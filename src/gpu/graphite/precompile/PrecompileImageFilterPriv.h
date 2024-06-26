/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileImageFilterPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileImageFilterPriv_DEFINED

#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"

namespace skgpu::graphite {

/** Class that exposes methods in PrecompileImageFilter that are only intended for use internal to
    Skia.
    This class is purely a privileged window into PrecompileImageFilter. It should never have
    additional data members or virtual methods. */
class PrecompileImageFilterPriv {
public:
    sk_sp<PrecompileColorFilter> isColorFilterNode() const {
        return fPrecompileImageFilter->isColorFilterNode();
    }

    const PrecompileImageFilter* getInput(int index) const {
        return fPrecompileImageFilter->getInput(index);
    }

private:
    friend class PrecompileImageFilter; // to construct/copy this type.

    explicit PrecompileImageFilterPriv(PrecompileImageFilter* precompileImageFilter)
            : fPrecompileImageFilter(precompileImageFilter) {}

    PrecompileImageFilterPriv& operator=(const PrecompileImageFilterPriv&) = delete;

    // No taking addresses of this type.
    const PrecompileImageFilterPriv* operator&() const;
    PrecompileImageFilterPriv *operator&();

    PrecompileImageFilter* fPrecompileImageFilter;
};

inline PrecompileImageFilterPriv PrecompileImageFilter::priv() {
    return PrecompileImageFilterPriv(this);
}

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileImageFilterPriv PrecompileImageFilter::priv() const {
    return PrecompileImageFilterPriv(const_cast<PrecompileImageFilter *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileImageFilterPriv_DEFINED
