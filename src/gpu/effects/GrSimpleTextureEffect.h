/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleTextureEffect_DEFINED
#define GrSimpleTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrTextureProxy.h"

class GrInvariantOutput;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerParams) and accepts
 * a matrix that is used to compute texture coordinates from local coordinates.
 */
class GrSimpleTextureEffect : public GrSingleTextureEffect {
public:
    /* unfiltered, clamp mode */
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(resourceProvider, std::move(proxy),
                                      std::move(colorSpaceXform), matrix,
                                      GrSamplerParams::kNone_FilterMode));
    }

    /* clamp mode */
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           GrSamplerParams::FilterMode filterMode) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(resourceProvider, std::move(proxy),
                                      std::move(colorSpaceXform),
                                      matrix, filterMode));
    }

    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           const GrSamplerParams& p) {
        return sk_sp<GrFragmentProcessor>(new GrSimpleTextureEffect(resourceProvider,
                                                                    std::move(proxy),
                                                                    std::move(colorSpaceXform),
                                                                    matrix, p));
    }

    ~GrSimpleTextureEffect() override {}

    const char* name() const override { return "SimpleTexture"; }

private:
    GrSimpleTextureEffect(GrResourceProvider*, sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>, const SkMatrix& matrix,
                          GrSamplerParams::FilterMode);

    GrSimpleTextureEffect(GrResourceProvider*, sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>, const SkMatrix& matrix,
                          const GrSamplerParams&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override { return true; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
