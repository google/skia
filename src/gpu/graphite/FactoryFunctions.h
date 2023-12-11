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
class PrecompileShaders {
public:
    //TODO: Add Empty? - see skbug.com/12165
    static sk_sp<PrecompileShader> Color();
    static sk_sp<PrecompileShader> Blend(SkSpan<const sk_sp<PrecompileBlender>> blenders,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    static sk_sp<PrecompileShader> Blend(SkSpan<SkBlendMode> blendModes,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    // TODO: add an SkShaders::Image to match this and SkImageFilters (skbug.com/13440)
    static sk_sp<PrecompileShader> Image();
    static sk_sp<PrecompileShader> YUVImage();

    // TODO: make SkGradientShader match this convention (skbug.com/13438)
    static sk_sp<PrecompileShader> LinearGradient();
    static sk_sp<PrecompileShader> RadialGradient();
    static sk_sp<PrecompileShader> TwoPointConicalGradient();
    static sk_sp<PrecompileShader> SweepGradient();

    // TODO: hide these? The issue here is that, in the main Skia API, these are only accessed
    // via makeWithLocalMatrix and makeWithColorFilter. However, in the combination API, clients
    // may want to create a set of these (i.e., pass SkSpans to the factory functions vs
    // just single options).
    static sk_sp<PrecompileShader> LocalMatrix(sk_sp<PrecompileShader> wrapped);
    static sk_sp<PrecompileShader> ColorFilter(sk_sp<PrecompileShader>,
                                               sk_sp<PrecompileColorFilter>);

private:
    PrecompileShaders() = delete;
};

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
