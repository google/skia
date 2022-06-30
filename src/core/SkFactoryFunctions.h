/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFactoryFunctions_DEFINED
#define SkFactoryFunctions_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"

class SkPrecompileBase;
class SkPrecompileBlender;
class SkPrecompileColorFilter;
class SkPrecompileImageFilter;
class SkPrecompileMaskFilter;
class SkPrecompileShader;

// All of these factory functions will be moved elsewhere once the pre-compile API becomes public

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkShaders in include/core/SkShader.h
class SkPrecompileShaders {
public:
    //TODO: Add Empty? - see skbug.com/12165
    static sk_sp<SkPrecompileShader> Color();
    static sk_sp<SkPrecompileShader> Blend(SkSpan<const sk_sp<SkPrecompileBlender>> blenders,
                                           SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                                           SkSpan<const sk_sp<SkPrecompileShader>> srcs);
    static sk_sp<SkPrecompileShader> Blend(SkSpan<SkBlendMode> blendModes,
                                           SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                                           SkSpan<const sk_sp<SkPrecompileShader>> srcs);
    // TODO: add an SkShaders::Image to match this and SkImageFilters (skbug.com/13440)
    static sk_sp<SkPrecompileShader> Image();

    // TODO: make SkGradientShader match this convention (skbug.com/13438)
    static sk_sp<SkPrecompileShader> LinearGradient();
    static sk_sp<SkPrecompileShader> RadialGradient();
    static sk_sp<SkPrecompileShader> TwoPointConicalGradient();
    static sk_sp<SkPrecompileShader> SweepGradient();

private:
    SkPrecompileShaders() = delete;
};

//--------------------------------------------------------------------------------------------------
// Initially this will go next to SkMaskFilter in include/core/SkMaskFilter.h but the
// SkMaskFilter::MakeBlur factory should be split out or removed. This namespace will follow
// where ever that factory goes.
class SkPrecompileMaskFilters {
public:
    // TODO: change SkMaskFilter::MakeBlur to match this and SkImageFilters::Blur (skbug.com/13441)
    static sk_sp<SkPrecompileMaskFilter> Blur();

private:
    SkPrecompileMaskFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkColorFilters in include/core/SkColorFilter.h
class SkPrecompileColorFilters {
public:
    static sk_sp<SkPrecompileColorFilter> Matrix();
    // TODO: Compose, Blend, HSLAMatrix, LinearToSRGBGamma, SRGBToLinearGamma, Lerp

private:
    SkPrecompileColorFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
// This will move to be beside SkImageFilters in include/effects/SkImageFilters.h
class SkPrecompileImageFilters {
public:
    static sk_sp<SkPrecompileImageFilter> Blur();
    static sk_sp<SkPrecompileImageFilter> Image();
    // TODO: AlphaThreshold, Arithmetic, Blend (2 kinds), ColorFilter, Compose, DisplacementMap,
    // DropShadow, DropShadowOnly, Magnifier, MatrixConvolution, MatrixTransform, Merge, Offset,
    // Picture, Runtime, Shader, Tile, Dilate, Erode, DistantLitDiffuse, PointLitDiffuse,
    // SpotLitDiffuse, DistantLitSpecular, PointLitSpecular, SpotLitSpecular

private:
    SkPrecompileImageFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
// Object that allows passing a SkPrecompileShader, SkPrecompileColorFilter or
// SkPrecompileBlender as a child
//
// This will moved to be on SkRuntimeEffect
class SkPrecompileChildPtr {
public:
    SkPrecompileChildPtr() = default;
    SkPrecompileChildPtr(sk_sp<SkPrecompileShader>);
    SkPrecompileChildPtr(sk_sp<SkPrecompileColorFilter>);
    SkPrecompileChildPtr(sk_sp<SkPrecompileBlender>);

    // Asserts that the SkPrecompileBase is either null, or one of the legal derived types
    SkPrecompileChildPtr(sk_sp<SkPrecompileBase>);

    std::optional<SkRuntimeEffect::ChildType> type() const;

    SkPrecompileShader* shader() const;
    SkPrecompileColorFilter* colorFilter() const;
    SkPrecompileBlender* blender() const;
    SkPrecompileBase* base() const { return fChild.get(); }

private:
    sk_sp<SkPrecompileBase> fChild;
};

using SkPrecompileChildOptions = SkSpan<const SkPrecompileChildPtr>;

// These will move to be on SkRuntimeEffect to parallel makeShader, makeColorFilter and
// makeBlender
sk_sp<SkPrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions = {});

sk_sp<SkPrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions = {});

sk_sp<SkPrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions = {});

#endif // SK_ENABLE_PRECOMPILE

#endif // SkFactoryFunctions_DEFINED
