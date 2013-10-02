/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrEffect.h"
#include "SkMatrix.h"
#include "GrCoordTransform.h"

class GrTexture;

/**
 * A base class for effects that draw a single texture with a texture matrix. This effect has no
 * backend implementations. One must be provided by the subclass.
 */
class GrSingleTextureEffect : public GrEffect {
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
     * Helper for subclass onIsEqual() functions.
     */
    bool hasSameTextureParamsMatrixAndSourceCoords(const GrSingleTextureEffect& other) const {
        // We don't have to check the accesses' swizzles because they are inferred from the texture.
        return fTextureAccess == other.fTextureAccess &&
               fCoordTransform.getMatrix().cheapEqualTo(other.fCoordTransform.getMatrix()) &&
               fCoordTransform.sourceCoords() == other.fCoordTransform.sourceCoords();
    }

    /**
     * Can be used as a helper to implement subclass getConstantColorComponents(). It assumes that
     * the subclass output color will be a modulation of the input color with a value read from the
     * texture.
     */
    void updateConstantColorComponentsForModulation(GrColor* color, uint32_t* validFlags) const {
        if ((*validFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
            GrPixelConfigIsOpaque(this->texture(0)->config())) {
            *validFlags = kA_GrColorComponentFlag;
        } else {
            *validFlags = 0;
        }
    }

private:
    GrCoordTransform fCoordTransform;
    GrTextureAccess  fTextureAccess;

    typedef GrEffect INHERITED;
};

#endif
