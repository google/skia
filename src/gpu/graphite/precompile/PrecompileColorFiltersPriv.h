/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED
#define skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/private/SkOnlyStackAllocatable.h"

namespace skgpu::graphite {

/** Class that exposes methods in PrecompileShader that are only intended for use internal to Skia.
    This class is purely a privileged window into PrecompileShader. It should never have additional
    data members or virtual methods. */
class PrecompileColorFilterPriv {
public:
    SK_ONLY_STACK_ALLOCATABLE;

    bool isAlphaUnchanged(int desiredCombination) const {
        return fPrecompileColorFilter->isAlphaUnchanged(desiredCombination);
    }

    // The remaining methods make this a viable standin for PrecompileBasePriv
    int numChildCombinations() const { return fPrecompileColorFilter->numChildCombinations(); }

    int numCombinations() const { return fPrecompileColorFilter->numCombinations(); }

    void addToKey(const KeyContext& keyContext, int desiredCombination) const {
        fPrecompileColorFilter->addToKey(keyContext, desiredCombination);
    }

private:
    friend class PrecompileColorFilter; // to construct/copy this type.

    explicit PrecompileColorFilterPriv(PrecompileColorFilter* precompileColorFilter)
            : fPrecompileColorFilter(precompileColorFilter) {}

    PrecompileColorFilterPriv& operator=(const PrecompileColorFilterPriv&) = delete;

    // No taking addresses of this type.
    const PrecompileColorFilterPriv* operator&() const;
    PrecompileColorFilterPriv *operator&();

    PrecompileColorFilter* fPrecompileColorFilter;
};

inline PrecompileColorFilterPriv PrecompileColorFilter::priv() {
    return PrecompileColorFilterPriv(this);
}

// NOLINTNEXTLINE(readability-const-return-type)
inline const PrecompileColorFilterPriv PrecompileColorFilter::priv() const {
    return PrecompileColorFilterPriv(const_cast<PrecompileColorFilter *>(this));
}

namespace PrecompileColorFiltersPriv {
    // These three factories match those in src/core/SkColorFilterPriv.h
    sk_sp<PrecompileColorFilter> Gaussian();

    sk_sp<PrecompileColorFilter> ColorSpaceXform(SkSpan<const sk_sp<SkColorSpace>> src,
                                                 SkSpan<const sk_sp<SkColorSpace>> dst);

    sk_sp<PrecompileColorFilter> WithWorkingFormat(
            SkSpan<const sk_sp<PrecompileColorFilter>> childOptions,
            const skcms_TransferFunction* tf,
            const skcms_Matrix3x3* gamut,
            const SkAlphaType* at);

} // namespace PrecompileColorFiltersPriv

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileColorFiltersPriv_DEFINED
