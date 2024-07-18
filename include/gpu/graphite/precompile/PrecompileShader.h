/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileShader_DEFINED
#define skgpu_graphite_precompile_PrecompileShader_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include "include/core/SkBlendMode.h"

class SkColorSpace;

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileShaderPriv;

/** \class PrecompileShader
    This class corresponds to the SkShader class in the main API.
*/
class SK_API PrecompileShader : public PrecompileBase {
public:
    /**
     *  This is the Precompile correlate to SkShader::makeWithLocalMatrix. The actual matrix
     *  involved is abstracted away, except for whether or the not the matrix involves perspective
     *  so the correct generated shader variation is chosen.
     *  The PrecompileShaders::LocalMatrix factory can be used to generate a set of shaders
     *  that would've been generated via multiple makeWithLocalMatrix calls. That is, rather than
     *  performing:
     *     sk_sp<PrecompileShader> option1 = source1->makeWithLocalMatrix(false);
     *     sk_sp<PrecompileShader> option2 = source2->makeWithLocalMatrix(false);
     *  one could call:
     *     sk_sp<PrecompileShader> combinedOptions = LocalMatrix({ source1, source2 }, false);
     */
    sk_sp<PrecompileShader> makeWithLocalMatrix(bool isPerspective) const;

    /**
     *  This is the Precompile correlate to SkShader::makeWithColorFilter.
     *  The PrecompileShaders::ColorFilter factory can be used to generate a set of shaders that
     *  would've been generated via multiple makeWithColorFilter calls. That is, rather than
     *  performing:
     *     sk_sp<PrecompileShader> option1 = source->makeWithColorFilter(colorFilter1);
     *     sk_sp<PrecompileShader> option2 = source->makeWithColorFilter(colorFilter2);
     *  one could call:
     *     sk_sp<PrecompileShader> combinedOptions = ColorFilter({ source },
     *                                                           { colorFilter1, colorFilter2 });
     *  With an alternative use case one could also use the ColorFilter factory thusly:
     *     sk_sp<PrecompileShader> combinedOptions = ColorFilter({ source1, source2 },
     *                                                           { colorFilter });
     */
    sk_sp<PrecompileShader> makeWithColorFilter(sk_sp<PrecompileColorFilter>) const;

    /**
     *  This is the Precompile correlate to SkShader::makeWithWorkingColorSpace.
     *  The PrecompileShaders::WorkingColorSpace factory can be used to generate a set of shaders
     *  that would've been generated via multiple makeWithWorkingColorSpace calls. That is, rather
     *  than performing:
     *     sk_sp<PrecompileShader> option1 = source->makeWithWorkingColorSpace(colorSpace1);
     *     sk_sp<PrecompileShader> option2 = source->makeWithWorkingColorSpace(colorSpace2);
     *  one could call:
     *     sk_sp<PrecompileShader> combinedOptions = WorkingColorSpace({ source },
     *                                                                 { colorSpace1,
     *                                                                   colorSpace2 });
     *  With an alternative use case one could also use the WorkingColorSpace factory thusly:
     *     sk_sp<PrecompileShader> combinedOptions = WorkingColorSpace({ source1, source2 },
     *                                                                 { colorSpace });
     */
    sk_sp<PrecompileShader> makeWithWorkingColorSpace(sk_sp<SkColorSpace>) const;

    // Provides access to functions that aren't part of the public API.
    PrecompileShaderPriv priv();
    const PrecompileShaderPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    friend class PrecompileShaderPriv;

    PrecompileShader() : PrecompileBase(Type::kShader) {}
    ~PrecompileShader() override;

    virtual bool isConstant(int /* desiredCombination */) const { return false; }

    virtual bool isALocalMatrixShader() const { return false; }
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkShaders namespace in the main API
namespace PrecompileShaders {
    // --- This block of eight matches the SkShaders factories in SkShader.h
    // Note that some of the details of the main API have been elided since they don't impact
    // the generated shader (e.g., the color parameter to the Color() factories).
    SK_API sk_sp<PrecompileShader> Empty();
    SK_API sk_sp<PrecompileShader> Color();
    SK_API sk_sp<PrecompileShader> Color(sk_sp<SkColorSpace>);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<const SkBlendMode> blendModes,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<const sk_sp<PrecompileBlender>> blenders,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> CoordClamp(SkSpan<const sk_sp<PrecompileShader>>);

    // In the main Skia API ImageShaders are usually created via a SkImage::makeShader call.
    // Since the SkImage used to create the ImageShader is unlikely to be present at precompilation
    // time this entry point allows the equivalent precompilation program structure to be created.
    // Note that this factory is for non-YUV SkImages, the YUVImage factory (below) should be used
    // to represent the shading and sampling required for YUV images.
    SK_API sk_sp<PrecompileShader> Image();
    // As with the above Image call, raw ImageShaders are usually created via an
    // SkImage::makeRawShader call. The RawImage call allows the equivalent precompilation
    // program structure to be created without needing the SkImage.
    SK_API sk_sp<PrecompileShader> RawImage();

    // In the main Skia API, the specifics of the SkImage used for the SkImage::makeShader call
    // can determine whether normal or YUV sampling is required. This entry point allows clients
    // to specify that the future image will be a YUV image.
    SK_API sk_sp<PrecompileShader> YUVImage();

    // --- This block of two matches the SkShaders factories in SkPerlinNoiseShader.h
    // Again, most of the details have been elided.
    SK_API sk_sp<PrecompileShader> MakeFractalNoise();
    SK_API sk_sp<PrecompileShader> MakeTurbulence();

    // --- This block of four matches all the factories in SkGradientShader (SkGradientShader.h)
    SK_API sk_sp<PrecompileShader> LinearGradient();
    SK_API sk_sp<PrecompileShader> RadialGradient();
    SK_API sk_sp<PrecompileShader> TwoPointConicalGradient();
    SK_API sk_sp<PrecompileShader> SweepGradient();

    // Normally, SkPicture shaders are only created via SkPicture::makeShader. Since the
    // SkPicture to be drawn, most likely, won't be available at precompilation time, this
    // entry point can be used to create a precompilation equivalent.
    // Note: this will precompile the program that draws the SkPicture. It, obviously, won't
    // precompile any SkPaints within the SkPicture.
    SK_API sk_sp<PrecompileShader> Picture();

    // Normally, LocalMatrixShaders are only created via SkShader::makeWithLocalMatrix.
    // However, in the combination API, clients may want to create a set of precompile
    // LocalMatrixShaders (i.e., pass an SkSpan to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithLocalMatrix() can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> LocalMatrix(SkSpan<const sk_sp<PrecompileShader>> wrapped,
                                               bool isPerspective = false);

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

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileShader_DEFINED
