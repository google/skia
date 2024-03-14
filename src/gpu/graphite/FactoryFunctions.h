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

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkShaders in include/core/SkShader.h
namespace PrecompileShaders {
    // This block of six matches the SkShaders factories in include/core/SkShader.h
    SK_API sk_sp<PrecompileShader> Empty();
    SK_API sk_sp<PrecompileShader> Color();
    SK_API sk_sp<PrecompileShader> Color(sk_sp<SkColorSpace>);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<SkBlendMode> blendModes,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<const sk_sp<PrecompileBlender>> blenders,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> CoordClamp(SkSpan<const sk_sp<PrecompileShader>>);

    // This block of two matches the SkShaders factories in include/effects/SkPerlinNoiseShader.h
    SK_API sk_sp<PrecompileShader> MakeFractalNoise();
    SK_API sk_sp<PrecompileShader> MakeTurbulence();

    // TODO: add an SkShaders::Image to match this and SkImageFilters (skbug.com/13440)
    SK_API sk_sp<PrecompileShader> Image();
    SK_API sk_sp<PrecompileShader> YUVImage();

    // TODO: make SkGradientShader match this convention (skbug.com/13438)
    // This block of four matches all the entry points in include/effects/SkGradientShader.h
    SK_API sk_sp<PrecompileShader> LinearGradient();
    SK_API sk_sp<PrecompileShader> RadialGradient();
    SK_API sk_sp<PrecompileShader> TwoPointConicalGradient();
    SK_API sk_sp<PrecompileShader> SweepGradient();

    // Normally SkPicture shaders are only created via SkPicture::makeShader. Since the
    // SkPicture to be drawn, most likely, won't be available at precompilation time, this
    // entry point can be used to create a precompilation equivalent.
    // Note: this will precompile the program that draws the SkPicture. It, obviously, won't
    // precompile any SkPaints within the SkPicture.
    //
    // API Note: At the end of the day this turns into a LMShader wrapping an image shader. The
    // LMShader has logic to elide itself if the LM is missing or the Identity. Combinatorially,
    // this yields 6 combinations: 2 from the LM x 3 from the ImageShader. We could try to reduce
    // that by adding a "passing-non-null-non-Identity-LM-to-SkPicture::makeShader" flag here
    // in which case we would either add or skip the LMShader. That would be a pretty obscure API
    // though.
    SK_API sk_sp<PrecompileShader> Picture();

    // TODO: hide these? The issue here is that, in the main Skia API, these are only accessed
    // via makeWithLocalMatrix and makeWithColorFilter. However, in the combination API, clients
    // may want to create a set of these (i.e., pass SkSpans to the factory functions vs
    // just single options).
    SK_API sk_sp<PrecompileShader> LocalMatrix(sk_sp<PrecompileShader> wrapped);
    SK_API sk_sp<PrecompileShader> ColorFilter(sk_sp<PrecompileShader>,
                                               sk_sp<PrecompileColorFilter>);
} // namespace PrecompileShaders

//--------------------------------------------------------------------------------------------------
// Initially this will go next to SkMaskFilter in include/core/SkMaskFilter.h but the
// SkMaskFilter::MakeBlur factory should be split out or removed. This namespace will follow
// where ever that factory goes.
class PrecompileMaskFilters {
public:
    // TODO: change SkMaskFilter::MakeBlur to match this and SkImageFilters::Blur (skbug.com/13441)
    static sk_sp<PrecompileMaskFilter> Blur();

private:
    PrecompileMaskFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkColorFilters in include/core/SkColorFilter.h
class PrecompileColorFilters {
public:
    static sk_sp<PrecompileColorFilter> Compose(SkSpan<const sk_sp<PrecompileColorFilter>> outer,
                                                SkSpan<const sk_sp<PrecompileColorFilter>> inner);

    // This encompasses both variants of SkColorFilters::Blend
    static sk_sp<PrecompileColorFilter> Blend();

    // This encompasses both variants of SkColorFilters::Matrix
    static sk_sp<PrecompileColorFilter> Matrix();

    // This encompasses both variants of SkColorFilters::HSLAMatrix
    static sk_sp<PrecompileColorFilter> HSLAMatrix();

    // TODO: add Lerp
    static sk_sp<PrecompileColorFilter> LinearToSRGBGamma();
    static sk_sp<PrecompileColorFilter> SRGBToLinearGamma();

    // This encompases both variants of SkColorFilters::Table and TableARGB
    static sk_sp<PrecompileColorFilter> Table();

    static sk_sp<PrecompileColorFilter> Lighting();

    // The remaining three match those in SkColorFilterPriv
    static sk_sp<PrecompileColorFilter> Gaussian();

    static sk_sp<PrecompileColorFilter> ColorSpaceXform();

    static sk_sp<PrecompileColorFilter> WithWorkingFormat(
            SkSpan<const sk_sp<PrecompileColorFilter>> childOptions);

private:
    PrecompileColorFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkImageFilters in include/effects/SkImageFilters.h
class PrecompileImageFilters {
public:
    static sk_sp<PrecompileImageFilter> Blur();
    static sk_sp<PrecompileImageFilter> Image();
    // TODO: Arithmetic, Blend (2 kinds), ColorFilter, Compose, DisplacementMap,
    // DropShadow, DropShadowOnly, Magnifier, MatrixConvolution, MatrixTransform, Merge, Offset,
    // Picture, Runtime, Shader, Tile, Dilate, Erode, DistantLitDiffuse, PointLitDiffuse,
    // SpotLitDiffuse, DistantLitSpecular, PointLitSpecular, SpotLitSpecular

private:
    PrecompileImageFilters() = delete;
};

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
