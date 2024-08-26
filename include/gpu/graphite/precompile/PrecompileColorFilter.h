/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileColorFilter_DEFINED
#define skgpu_graphite_precompile_PrecompileColorFilter_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

enum class SkBlendMode;

namespace skgpu::graphite {

/** \class PrecompileColorFilter
    This class corresponds to the SkColorFilter class in the main API.
*/
class SK_API PrecompileColorFilter : public PrecompileBase {
public:
    /**
     *  This is the Precompile correlate to SkColorFilter::makeComposed.
     *
     *  The PrecompileColorFilters::Compose factory can be used to generate a set of color filters
     *  that would've been generated via multiple makeComposed calls. That is, rather than
     *  performing:
     *     sk_sp<PrecompileColorFilter> option1 = outer->makeComposed(colorFilter1);
     *     sk_sp<PrecompileColorFilter> option2 = outer->makeComposed(colorFilter2);
     *  one could call:
     *     sk_sp<PrecompileColorFilter> combinedOptions = Compose({ outer },
     *                                                            { colorFilter1, colorFilter2 });
     *  With an alternative use case one could also use the Compose factory thusly:
     *     sk_sp<PrecompileColorFilter> combinedOptions = Compose({ outer1, outer2 },
     *                                                            { innerColorFilter });
     */
    sk_sp<PrecompileColorFilter> makeComposed(sk_sp<PrecompileColorFilter> inner) const;

protected:
    PrecompileColorFilter() : PrecompileBase(Type::kColorFilter) {}
    ~PrecompileColorFilter() override;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkColorFilters namespace in the main API
namespace PrecompileColorFilters {

    // --- The next 9 entries match those in include/core/SkColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Compose(SkSpan<const sk_sp<PrecompileColorFilter>> outer,
                                                SkSpan<const sk_sp<PrecompileColorFilter>> inner);

    // This encompasses both variants of SkColorFilters::Blend
    SK_API sk_sp<PrecompileColorFilter> Blend(SkSpan<const SkBlendMode> blendModes);
    SK_API sk_sp<PrecompileColorFilter> Blend(); // Prefer the explicit blend mode variant

    // This encompasses both variants of SkColorFilters::Matrix
    SK_API sk_sp<PrecompileColorFilter> Matrix();

    // This encompasses both variants of SkColorFilters::HSLAMatrix
    SK_API sk_sp<PrecompileColorFilter> HSLAMatrix();

    SK_API sk_sp<PrecompileColorFilter> LinearToSRGBGamma();
    SK_API sk_sp<PrecompileColorFilter> SRGBToLinearGamma();
    SK_API sk_sp<PrecompileColorFilter> Lerp(SkSpan<const sk_sp<PrecompileColorFilter>> dstOptions,
                                             SkSpan<const sk_sp<PrecompileColorFilter>> srcOptions);

    // This encompases both the SkColorFilters::Table and TableARGB variants
    SK_API sk_sp<PrecompileColorFilter> Table();

    SK_API sk_sp<PrecompileColorFilter> Lighting();

    // This matches the main API's factory in include/effects/SkHighContrastFilter.h
    SK_API sk_sp<PrecompileColorFilter> HighContrast();

    // This matches the main API's factory in include/effects/SkLumaColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Luma();

    // This matches the main API's factory in include/effects/SkOverdrawColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Overdraw();

} // namespace PrecompileColorFilters

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileColorFilter_DEFINED
