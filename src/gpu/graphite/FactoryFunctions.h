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
class PrecompileMaskFilter;
class PrecompileShader;

// All of these factory functions will be moved elsewhere once the pre-compile API becomes public

//--------------------------------------------------------------------------------------------------
namespace PrecompileBlenders {

    // --- This call matches the SkBlenders factory in include/effects/SkBlenders.h
    SK_API sk_sp<PrecompileBlender> Arithmetic();

    // Note: the other main API SkBlender factories are:
    //   SkBlender::Mode in include/core/SkBlender.h
    //   SkRuntimeEffect::makeBlender in include/effects/SkRuntimeEffect.h
    // Their precompilation correlates are:
    //   PrecompileBlender::Mode(bm) in src/gpu/graphite/Precompile.h
    //   MakePrecompileBlender() in src/gpu/graphite/FactoryFunctions.h

} // namespace PrecompileBlenders

//--------------------------------------------------------------------------------------------------
namespace PrecompileShaders {
    // --- This block of six matches the SkShaders factories in include/core/SkShader.h
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

    // --- This block of two matches the SkShaders factories in include/effects/SkPerlinNoiseShader.h
    SK_API sk_sp<PrecompileShader> MakeFractalNoise();
    SK_API sk_sp<PrecompileShader> MakeTurbulence();

    // --- This block of two matches the SkShaders factories in include/core/SkImage.h
    // In the normal Skia API ImageShaders are usually created via a SkImage::makeShader call.
    // Since the SkImage used to create the ImageShader is unlikely to be present at precompilation
    // time this entry point allows the equivalent precompilation program structure to be created.
    SK_API sk_sp<PrecompileShader> Image();
    // As with the above Image call, raw ImageShaders are usually created via an
    // SkImage::makeRawShader call. The RawImage call allows the equivalent precompilation
    // program structure to be created without needing the SkImage.
    SK_API sk_sp<PrecompileShader> RawImage();

    // ??
    SK_API sk_sp<PrecompileShader> YUVImage();

    // TODO: make SkGradientShader match this convention (skbug.com/13438)
    // This block of four matches all the entry points in include/effects/SkGradientShader.h
    SK_API sk_sp<PrecompileShader> LinearGradient();
    SK_API sk_sp<PrecompileShader> RadialGradient();
    SK_API sk_sp<PrecompileShader> TwoPointConicalGradient();
    SK_API sk_sp<PrecompileShader> SweepGradient();

    // Normally, SkPicture shaders are only created via SkPicture::makeShader. Since the
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

    // Normally, LocalMatrixShaders are only created via SkShader::makeWithLocalMatrix.
    // However, in the combination API, clients may want to create a set of precompile
    // LocalMatrixShaders (i.e., pass an SkSpan to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithLocalMatrix() can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> LocalMatrix(SkSpan<const sk_sp<PrecompileShader>> wrapped);

    // Normally, ColorFilterShaders are only created via SkShader::makeWithColorFilter.
    // However, in the combination API, clients may want to create a set of precompile
    // ColorFilterShaders (i.e., pass SkSpans to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithColorFilter can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> ColorFilter(
            SkSpan<const sk_sp<PrecompileShader>> shaders,
            SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters);

    // Normally, WorkingColorSpaceShaders are only created via SkShader::makeWithWorkingColorSpace.
    // However, in the combination API, clients may want to create a set of precompile
    // WorkingColorSpaceShaders (i.e., pass SkSpans to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithWorkingColorSpace can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> WorkingColorSpace(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                                     SkSpan<const sk_sp<SkColorSpace>> colorSpaces);

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
