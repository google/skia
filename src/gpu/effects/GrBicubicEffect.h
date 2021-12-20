/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBicubicTextureEffect_DEFINED
#define GrBicubicTextureEffect_DEFINED

#include "src/gpu/GrFragmentProcessor.h"

class GrInvariantOutput;

class GrBicubicEffect : public GrFragmentProcessor {
public:
    enum {
        kFilterTexelPad = 2, // Given a src rect in texels to be filtered, this number of
                             // surrounding texels are needed by the kernel in x and y.
    };

    inline static constexpr SkImage::CubicResampler gMitchell = { 1.0f/3, 1.0f/3 };
    inline static constexpr SkImage::CubicResampler gCatmullRom = {    0, 1.0f/2 };

    enum class Direction {
        /** Apply bicubic kernel in local coord x, nearest neighbor in y. */
        kX,
        /** Apply bicubic kernel in local coord y, nearest neighbor in x. */
        kY,
        /** Apply bicubic in both x and y. */
        kXY
    };

    const char* name() const override { return "Bicubic"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrBicubicEffect(*this));
    }

    /**
     * Create a bicubic filter effect with specified texture matrix with clamp wrap mode.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView view,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     SkImage::CubicResampler,
                                                     Direction);

    /**
     * Create a bicubic filter effect for a texture with arbitrary wrap modes.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView view,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     const GrSamplerState::WrapMode wrapX,
                                                     const GrSamplerState::WrapMode wrapY,
                                                     SkImage::CubicResampler,
                                                     Direction,
                                                     const GrCaps&);

    /**
     * Create a bicubic filter effect for a subset of a texture, specified by a texture coordinate
     * rectangle subset. The WrapModes apply to the subset.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeSubset(GrSurfaceProxyView view,
                                                           SkAlphaType,
                                                           const SkMatrix&,
                                                           const GrSamplerState::WrapMode wrapX,
                                                           const GrSamplerState::WrapMode wrapY,
                                                           const SkRect& subset,
                                                           SkImage::CubicResampler,
                                                           Direction,
                                                           const GrCaps&);

    /**
     * Same as above but provides a known 'domain' that bounds the coords at which bicubic sampling
     * occurs. Note that this is a bound on the coords after transformed by the matrix parameter.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeSubset(GrSurfaceProxyView view,
                                                           SkAlphaType,
                                                           const SkMatrix&,
                                                           const GrSamplerState::WrapMode wrapX,
                                                           const GrSamplerState::WrapMode wrapY,
                                                           const SkRect& subset,
                                                           const SkRect& domain,
                                                           SkImage::CubicResampler,
                                                           Direction,
                                                           const GrCaps&);

    /**
     * Make a bicubic filter of a another fragment processor. The bicubic filter assumes that the
     * discrete samples of the provided processor are at half-integer coords.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor>,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     SkImage::CubicResampler,
                                                     Direction);

private:
    class Impl;

    enum class Clamp {
        kUnpremul,  // clamps rgba to 0..1
        kPremul,    // clamps a to 0..1 and rgb to 0..a
    };

    GrBicubicEffect(std::unique_ptr<GrFragmentProcessor>,
                    SkImage::CubicResampler,
                    Direction,
                    Clamp);

    explicit GrBicubicEffect(const GrBicubicEffect&);

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f&) const override;

    SkImage::CubicResampler fKernel;
    Direction fDirection;
    Clamp fClamp;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    using INHERITED = GrFragmentProcessor;
};

#endif
