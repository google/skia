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
 * A base class for effects that draw a single texture with a texture matrix.
 */
class GrSingleTextureEffect : public GrEffect {
public:
    virtual ~GrSingleTextureEffect();

    const SkMatrix& getMatrix() const { return fMatrix; }

protected:
    GrSingleTextureEffect(GrTexture*, const SkMatrix&); /* unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, bool bilerp); /* clamp mode */
    GrSingleTextureEffect(GrTexture*, const SkMatrix&, const GrTextureParams&);

    /**
     * Helper for subclass onIsEqual() functions.
     */
    bool hasSameTextureParamsAndMatrix(const GrSingleTextureEffect& other) const {
        const GrTextureAccess& otherAccess = other.fTextureAccess;
        // We don't have to check the accesses' swizzles because they are inferred from the texture.
        return fTextureAccess.getTexture() == otherAccess.getTexture() &&
               fTextureAccess.getParams() == otherAccess.getParams() &&
               this->getMatrix().cheapEqualTo(other.getMatrix());
    }

    /**
     * Can be used as a helper to implement subclass getConstantColorComponents(). It assumes that
     * the subclass output color will be a modulation of the input color with a value read from the
     * texture.
     */
    void updateConstantColorComponentsForModulation(GrColor* color, uint32_t* validFlags) const {
        if ((*validFlags & kA_ValidComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
            GrPixelConfigIsOpaque(this->texture(0)->config())) {
            *validFlags = kA_ValidComponentFlag;
        } else {
            *validFlags = 0;
        }
    }

private:
    GrTextureAccess fTextureAccess;
    SkMatrix        fMatrix;

    typedef GrEffect INHERITED;
};

#endif
