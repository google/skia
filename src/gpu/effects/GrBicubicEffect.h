/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBicubicTextureEffect_DEFINED
#define GrBicubicTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrTextureDomain.h"
#include "glsl/GrGLSLFragmentProcessor.h"

class GrInvariantOutput;

class GrBicubicEffect : public GrSingleTextureEffect {
public:
    enum {
        kFilterTexelPad = 2, // Given a src rect in texels to be filtered, this number of
                             // surrounding texels are needed by the kernel in x and y.
    };
    ~GrBicubicEffect() override;

    const char* name() const override { return "Bicubic"; }

    const GrTextureDomain& domain() const { return fDomain; }

    /**
     * Create a Mitchell filter effect with specified texture matrix and x/y tile modes.
     */
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           const SkShader::TileMode tileModes[2]) {
        return sk_sp<GrFragmentProcessor>(new GrBicubicEffect(resourceProvider, std::move(proxy),
                                                              std::move(colorSpaceXform),
                                                              matrix, tileModes));
    }

    /**
     * Create a Mitchell filter effect with a texture matrix and a domain.
     */
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           const SkRect& domain) {
        return sk_sp<GrFragmentProcessor>(new GrBicubicEffect(resourceProvider, std::move(proxy),
                                                              std::move(colorSpaceXform),
                                                              matrix, domain));
    }

    /**
     * Determines whether the bicubic effect should be used based on the transformation from the
     * local coords to the device. Returns true if the bicubic effect should be used. filterMode
     * is set to appropriate filtering mode to use regardless of the return result (e.g. when this
     * returns false it may indicate that the best fallback is to use kMipMap, kBilerp, or
     * kNearest).
     */
    static bool ShouldUseBicubic(const SkMatrix& localCoordsToDevice,
                                 GrSamplerParams::FilterMode* filterMode);

private:
    GrBicubicEffect(GrResourceProvider*, sk_sp<GrTextureProxy>, sk_sp<GrColorSpaceXform>,
                    const SkMatrix &matrix, const SkShader::TileMode tileModes[2]);
    GrBicubicEffect(GrResourceProvider*, sk_sp<GrTextureProxy>, sk_sp<GrColorSpaceXform>,
                    const SkMatrix &matrix, const SkRect& domain);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrTextureDomain fDomain;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
