/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBicubicTextureEffect_DEFINED
#define GrBicubicTextureEffect_DEFINED

#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"

class GrInvariantOutput;

class GrBicubicEffect : public GrFragmentProcessor {
public:
    enum {
        kFilterTexelPad = 2, // Given a src rect in texels to be filtered, this number of
                             // surrounding texels are needed by the kernel in x and y.
    };

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

    Direction direction() const { return fDirection; }

    /**
     * Create a Mitchell filter effect with specified texture matrix with clamp wrap mode.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy>,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     Direction direction);

    /**
     * Create a Mitchell filter effect for a texture with arbitrary wrap modes.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy>,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     const GrSamplerState::WrapMode[2],
                                                     Direction,
                                                     const GrCaps&);

    /**
     * Create a Mitchell filter effect for a subset of a texture, specified by a texture coordinate
     * rectangle subset. The WrapModes apply to the subset.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeSubset(sk_sp<GrSurfaceProxy>,
                                                           SkAlphaType,
                                                           const SkMatrix&,
                                                           const GrSamplerState::WrapMode[2],
                                                           const SkRect& subset,
                                                           Direction,
                                                           const GrCaps&);

    /**
     * Create a Mitchell filter effect with a texture matrix and integer texel subset. The WrapModes
     * apply to the subset.
     */
    static std::unique_ptr<GrFragmentProcessor> MakeTexelSubset(sk_sp<GrSurfaceProxy>,
                                                                SkAlphaType,
                                                                const SkMatrix&,
                                                                const GrSamplerState::WrapMode[2],
                                                                const SkIRect texelSubset,
                                                                Direction,
                                                                const GrCaps&);

    /**
     * Determines whether the bicubic effect should be used based on the transformation from the
     * local coords to the device. Returns true if the bicubic effect should be used. filterMode
     * is set to appropriate filtering mode to use regardless of the return result (e.g. when this
     * returns false it may indicate that the best fallback is to use kMipMap, kBilerp, or
     * kNearest).
     */
    static bool ShouldUseBicubic(const SkMatrix& localCoordsToDevice,
                                 GrSamplerState::Filter* filterMode);

private:
    class GLSLEffect;

    enum Clamp {
        kNone,
        kUnpremul,  // clamps rgba to 0..1
        kPremul,    // clamps a to 0..1 and rgb to 0..a
    };

    GrBicubicEffect(std::unique_ptr<GrFragmentProcessor> fp, Direction direction, Clamp clamp);
    explicit GrBicubicEffect(const GrBicubicEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    Direction fDirection;
    Clamp fClamp;
    GrCoordTransform fCoordTransform;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
