/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrFragmentProcessor.h"
#include "SkMatrix.h"
#include "GrCoordTransform.h"

class GrTexture;

/**
 * A base class for effects that draw a single texture with a texture matrix. This effect has no
 * backend implementations. One must be provided by the subclass.
 */
class GrSingleTextureEffect : public GrFragmentProcessor {
public:
    virtual ~GrSingleTextureEffect();

protected:
    /** unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, GrCoordSet = kLocal_GrCoordSet);
    /** clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, GrTextureParams::FilterMode filterMode,
                          GrCoordSet = kLocal_GrCoordSet);
    GrSingleTextureEffect(GrTexture*,
                          const SkMatrix&,
                          const GrTextureParams&,
                          GrCoordSet = kLocal_GrCoordSet);

    /**
     * Can be used as a helper to implement subclass onComputeInvariantOutput(). It assumes that
     * the subclass output color will be a modulation of the input color with a value read from the
     * texture.
     */
    void updateInvariantOutputForModulation(InvariantOutput* inout) const {
        if (GrPixelConfigIsOpaque(this->texture(0)->config())) {
            inout->mulByUnknownOpaqueColor();
        } else {
            inout->mulByUnknownColor();
        }
    }

private:
    GrCoordTransform fCoordTransform;
    GrTextureAccess  fTextureAccess;

    typedef GrFragmentProcessor INHERITED;
};

#endif
