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

    const GrTextureDomain& domain() const { return fDomain; }

    Direction direction() const { return fDirection; }

    SkAlphaType alphaType() const { return fAlphaType; }

    /**
     * Create a Mitchell filter effect with specified texture matrix with clamp wrap mode.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     const SkMatrix& matrix,
                                                     Direction direction,
                                                     SkAlphaType alphaType) {
        static constexpr GrSamplerState::WrapMode kClampClamp[] = {
                GrSamplerState::WrapMode::kClamp, GrSamplerState::WrapMode::kClamp};
        return Make(std::move(proxy), matrix, kClampClamp, GrTextureDomain::kIgnore_Mode,
                    GrTextureDomain::kIgnore_Mode, direction, alphaType);
    }

    /**
     * Create a Mitchell filter effect with specified texture matrix and x/y tile modes.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     const SkMatrix& matrix,
                                                     const GrSamplerState::WrapMode wrapModes[2],
                                                     Direction direction,
                                                     SkAlphaType alphaType) {
        // Ignore the domain on x and y, since this factory relies solely on the wrap mode of the
        // sampler to constrain texture coordinates
        return Make(std::move(proxy), matrix, wrapModes, GrTextureDomain::kIgnore_Mode,
                    GrTextureDomain::kIgnore_Mode, direction, alphaType);
    }

    /**
     * Create a Mitchell filter effect with specified texture matrix and x/y tile modes. This
     * supports providing modes for the texture domain explicitly, in the event that it should
     * override the behavior of the sampler's tile mode (e.g. clamp to border unsupported).
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     const SkMatrix& matrix,
                                                     const GrSamplerState::WrapMode wrapModes[2],
                                                     GrTextureDomain::Mode modeX,
                                                     GrTextureDomain::Mode modeY,
                                                     Direction direction,
                                                     SkAlphaType alphaType,
                                                     const SkRect* domain = nullptr) {
        SkRect resolvedDomain = domain ? *domain : GrTextureDomain::MakeTexelDomain(
                SkIRect::MakeWH(proxy->width(), proxy->height()), modeX, modeY);
        return std::unique_ptr<GrFragmentProcessor>(
                new GrBicubicEffect(std::move(proxy), matrix, resolvedDomain, wrapModes, modeX,
                                    modeY, direction, alphaType));
    }

    /**
     * Create a Mitchell filter effect with a texture matrix and a domain.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     const SkMatrix& matrix,
                                                     const SkRect& domain,
                                                     Direction direction,
                                                     SkAlphaType alphaType) {
        static const GrSamplerState::WrapMode kClampClamp[] = {
                GrSamplerState::WrapMode::kClamp, GrSamplerState::WrapMode::kClamp};
        return Make(std::move(proxy), matrix, kClampClamp, GrTextureDomain::kClamp_Mode,
                    GrTextureDomain::kClamp_Mode, direction, alphaType, &domain);
    }

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
    GrBicubicEffect(sk_sp<GrTextureProxy>, const SkMatrix& matrix, const SkRect& domain,
                    const GrSamplerState::WrapMode wrapModes[2], GrTextureDomain::Mode modeX,
                    GrTextureDomain::Mode modeY, Direction direction, SkAlphaType alphaType);
    explicit GrBicubicEffect(const GrBicubicEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int) const override { return fTextureSampler; }

    GrCoordTransform fCoordTransform;
    GrTextureDomain fDomain;
    TextureSampler fTextureSampler;
    SkAlphaType fAlphaType;
    Direction fDirection;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
