/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_FactoryFunctions_DEFINED
#define skgpu_graphite_FactoryFunctions_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"

namespace skgpu::graphite {

class PrecompileBase;
class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileImageFilter;
class PrecompileMaskFilter;
class PrecompileShader;

// All of these factory functions will be moved elsewhere once the pre-compile API becomes public

namespace PrecompileShaders {
    // ??
    SK_API sk_sp<PrecompileShader> YUVImage();

} // namespace PrecompileShaders

// TODO: For all of the PrecompileImageFilter factories, should we have a CropRect parameter or
// have clients explicitly create a crop PrecompileImageFilter?
// Note: In order to make analysis more tractable we don't allow options for the internals of an
// ImageFilter nor in the structure of the DAG.
namespace PrecompileImageFilters {
    // This is the Precompile correlate to SkImageFilters::Arithmetic
    SK_API sk_sp<PrecompileImageFilter> Arithmetic(sk_sp<PrecompileImageFilter> background,
                                                   sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to SkImageFilters::Blend(SkBlendMode, ...)
    SK_API sk_sp<PrecompileImageFilter> Blend(SkBlendMode bm,
                                              sk_sp<PrecompileImageFilter> background,
                                              sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to SkImageFilters::Blend(sk_sp<SkBlender>, ...)
    SK_API sk_sp<PrecompileImageFilter> Blend(sk_sp<PrecompileBlender> blender,
                                              sk_sp<PrecompileImageFilter> background,
                                              sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to the two SkImageFilters::Blur factories
    SK_API sk_sp<PrecompileImageFilter> Blur(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::ColorFilter.
    SK_API sk_sp<PrecompileImageFilter> ColorFilter(sk_sp<PrecompileColorFilter> colorFilter,
                                                    sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::DisplacementMap
    SK_API sk_sp<PrecompileImageFilter> DisplacementMap(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to all of SkImageFilters::
    //      DistantLitDiffuse,  PointLitDiffuse,  SpotLitDiffuse
    //      DistantLitSpecular, PointLitSpecular, SpotLitSpecular
    SK_API sk_sp<PrecompileImageFilter> Lighting(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::MatrixConvolution
    SK_API sk_sp<PrecompileImageFilter> MatrixConvolution(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::Erode and SkImageFilters::Dilate
    SK_API sk_sp<PrecompileImageFilter> Morphology(sk_sp<PrecompileImageFilter> input);

} // namespace PrecompileImageFilters

//--------------------------------------------------------------------------------------------------
// Initially this will go next to SkMaskFilter in include/core/SkMaskFilter.h but the
// SkMaskFilter::MakeBlur factory should be split out or removed. This namespace will follow
// where ever that factory goes.
namespace PrecompileMaskFilters {
    // TODO: change SkMaskFilter::MakeBlur to match this and SkImageFilters::Blur (skbug.com/13441)
    SK_API sk_sp<PrecompileMaskFilter> Blur();
} // namespace PrecompileMaskFilters

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkColorFilters in include/core/SkColorFilter.h
namespace PrecompileColorFilters {
    // -- The next 9 entries match those in include/core/SkColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Compose(SkSpan<const sk_sp<PrecompileColorFilter>> outer,
                                                SkSpan<const sk_sp<PrecompileColorFilter>> inner);

    // This encompasses both variants of SkColorFilters::Blend
    SK_API sk_sp<PrecompileColorFilter> Blend();

    // This encompasses both variants of SkColorFilters::Matrix
    SK_API sk_sp<PrecompileColorFilter> Matrix();

    // This encompasses both variants of SkColorFilters::HSLAMatrix
    SK_API sk_sp<PrecompileColorFilter> HSLAMatrix();

    SK_API sk_sp<PrecompileColorFilter> LinearToSRGBGamma();
    SK_API sk_sp<PrecompileColorFilter> SRGBToLinearGamma();
    SK_API sk_sp<PrecompileColorFilter> Lerp(SkSpan<const sk_sp<PrecompileColorFilter>> dstOptions,
                                             SkSpan<const sk_sp<PrecompileColorFilter>> srcOptions);

    // This encompases both variants of SkColorFilters::Table and TableARGB
    SK_API sk_sp<PrecompileColorFilter> Table();

    SK_API sk_sp<PrecompileColorFilter> Lighting();

    // This matches the main API's factory in include/effects/SkHighContrastFilter.h
    SK_API sk_sp<PrecompileColorFilter> HighContrast();

    // This matches the main API's factory in include/effects/SkLumaColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Luma();

    // This matches the main API's factory in include/effects/SkOverdrawColorFilter.h
    SK_API sk_sp<PrecompileColorFilter> Overdraw();

} // namespace PrecompileColorFilters

//--------------------------------------------------------------------------------------------------
// Object that allows passing a SkPrecompileShader, SkPrecompileColorFilter or
// SkPrecompileBlender as a child
//
// This will moved to be on SkRuntimeEffect
class PrecompileChildPtr {
public:
    PrecompileChildPtr() = default;
    PrecompileChildPtr(sk_sp<PrecompileShader>);
    PrecompileChildPtr(sk_sp<PrecompileColorFilter>);
    PrecompileChildPtr(sk_sp<PrecompileBlender>);

    // Asserts that the SkPrecompileBase is either null, or one of the legal derived types
    PrecompileChildPtr(sk_sp<PrecompileBase>);

    std::optional<SkRuntimeEffect::ChildType> type() const;

    PrecompileShader* shader() const;
    PrecompileColorFilter* colorFilter() const;
    PrecompileBlender* blender() const;
    PrecompileBase* base() const { return fChild.get(); }

private:
    sk_sp<PrecompileBase> fChild;
};

using PrecompileChildOptions = SkSpan<const PrecompileChildPtr>;

// TODO: the precompile RuntimeEffects are handling their child options different from the
// rest of the precompile system!

// These will move to be on SkRuntimeEffect to parallel makeShader, makeColorFilter and
// makeBlender
sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

} // namespace skgpu::graphite

#endif // skgpu_graphite_FactoryFunctions_DEFINED
