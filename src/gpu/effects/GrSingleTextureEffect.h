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

class GrTexture;

/**
 * A base class for effects that draw a single texture with a texture matrix. This effect has no
 * backend implementations. One must be provided by the subclass.
 */
class GrSingleTextureEffect : public GrEffect {
public:
    virtual ~GrSingleTextureEffect();

    const SkMatrix& getMatrix() const { return fMatrix; }

    /** Indicates whether the matrix operates on local coords or positions */
    CoordsType coordsType() const { return fCoordsType; }

protected:
    /** unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, CoordsType = kLocal_CoordsType);
    /** clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, bool bilerp, CoordsType = kLocal_CoordsType);
    GrSingleTextureEffect(GrTexture*,
                          const SkMatrix&,
                          const GrTextureParams&,
                          CoordsType = kLocal_CoordsType);

    /**
     * Helper for subclass onIsEqual() functions.
     */
    bool hasSameTextureParamsMatrixAndCoordsType(const GrSingleTextureEffect& other) const {
        const GrTextureAccess& otherAccess = other.fTextureAccess;
        // We don't have to check the accesses' swizzles because they are inferred from the texture.
        return fTextureAccess.getTexture() == otherAccess.getTexture() &&
               fTextureAccess.getParams() == otherAccess.getParams() &&
               this->getMatrix().cheapEqualTo(other.getMatrix()) &&
               fCoordsType == other.fCoordsType;
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
    GrTextureAccess fTextureAccess;
    SkMatrix        fMatrix;
    CoordsType      fCoordsType;

    typedef GrEffect INHERITED;
};

#endif
