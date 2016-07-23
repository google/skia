/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleTextureEffect_DEFINED
#define GrSimpleTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrInvariantOutput;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). It can use
 * local coords or device space coords as input texture coords. The input coords may be transformed
 * by a matrix.
 */
class GrSimpleTextureEffect : public GrSingleTextureEffect {
public:
    /* unfiltered, clamp mode */
    static sk_sp<GrFragmentProcessor> Make(GrTexture* tex,
                                           const SkMatrix& matrix,
                                           GrCoordSet coordSet = kLocal_GrCoordSet) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(tex, matrix, GrTextureParams::kNone_FilterMode, coordSet));
    }

    /* clamp mode */
    static sk_sp<GrFragmentProcessor> Make(GrTexture* tex,
                                            const SkMatrix& matrix,
                                            GrTextureParams::FilterMode filterMode,
                                            GrCoordSet coordSet = kLocal_GrCoordSet) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(tex, matrix, filterMode, coordSet));
    }

    static sk_sp<GrFragmentProcessor> Make(GrTexture* tex,
                                           const SkMatrix& matrix,
                                           const GrTextureParams& p,
                                           GrCoordSet coordSet = kLocal_GrCoordSet) {
        return sk_sp<GrFragmentProcessor>(new GrSimpleTextureEffect(tex, matrix, p, coordSet));
    }

    virtual ~GrSimpleTextureEffect() {}

    const char* name() const override { return "SimpleTexture"; }

private:
    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          GrTextureParams::FilterMode filterMode,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(texture, matrix, filterMode, coordSet) {
        this->initClassID<GrSimpleTextureEffect>();
    }

    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          const GrTextureParams& params,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(texture, matrix, params, coordSet) {
        this->initClassID<GrSimpleTextureEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
