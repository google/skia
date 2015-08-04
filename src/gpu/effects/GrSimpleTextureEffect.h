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
 * local coords, positions, or a custom vertex attribute as input texture coords. The input coords
 * can have a matrix applied in the VS in both the local and position cases but not with a custom
 * attribute coords at this time. It will add a varying to input interpolate texture coords to the
 * FS.
 */
class GrSimpleTextureEffect : public GrSingleTextureEffect {
public:
    /* unfiltered, clamp mode */
    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager,
                                       GrTexture* tex,
                                       const SkMatrix& matrix,
                                       GrCoordSet coordSet = kLocal_GrCoordSet) {
        return SkNEW_ARGS(GrSimpleTextureEffect, (procDataManager, tex, matrix,
                                                  GrTextureParams::kNone_FilterMode, coordSet));
    }

    /* clamp mode */
    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager,
                                       GrTexture* tex,
                                       const SkMatrix& matrix,
                                       GrTextureParams::FilterMode filterMode,
                                       GrCoordSet coordSet = kLocal_GrCoordSet) {
        return SkNEW_ARGS(GrSimpleTextureEffect, (procDataManager, tex, matrix, filterMode,
                                                  coordSet));
    }

    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager,
                                       GrTexture* tex,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& p,
                                       GrCoordSet coordSet = kLocal_GrCoordSet) {
        return SkNEW_ARGS(GrSimpleTextureEffect, (procDataManager, tex, matrix, p, coordSet));
    }

    virtual ~GrSimpleTextureEffect() {}

    const char* name() const override { return "SimpleTexture"; }

    GrGLFragmentProcessor* createGLInstance() const override;

private:
    GrSimpleTextureEffect(GrProcessorDataManager* procDataManager,
                          GrTexture* texture,
                          const SkMatrix& matrix,
                          GrTextureParams::FilterMode filterMode,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(procDataManager, texture, matrix, filterMode, coordSet) {
        this->initClassID<GrSimpleTextureEffect>();
    }

    GrSimpleTextureEffect(GrProcessorDataManager* procDataManager,
                          GrTexture* texture,
                          const SkMatrix& matrix,
                          const GrTextureParams& params,
                          GrCoordSet coordSet)
        : GrSingleTextureEffect(procDataManager, texture, matrix, params, coordSet) {
        this->initClassID<GrSimpleTextureEffect>();
    }

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
