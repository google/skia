/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleTextureEffect_DEFINED
#define GrSimpleTextureEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrGLSimpleTextureEffect;

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
    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               CoordsType coordsType = kLocal_CoordsType) {
        GrAssert(kLocal_CoordsType == coordsType || kPosition_CoordsType == coordsType);
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, false, coordsType)));
        return CreateEffectRef(effect);
    }

    /* clamp mode */
    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               bool bilerp,
                               CoordsType coordsType = kLocal_CoordsType) {
        GrAssert(kLocal_CoordsType == coordsType || kPosition_CoordsType == coordsType);
        AutoEffectUnref effect(
            SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, bilerp, coordsType)));
        return CreateEffectRef(effect);
    }

    static GrEffectRef* Create(GrTexture* tex,
                               const SkMatrix& matrix,
                               const GrTextureParams& p,
                               CoordsType coordsType = kLocal_CoordsType) {
        GrAssert(kLocal_CoordsType == coordsType || kPosition_CoordsType == coordsType);
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex, matrix, p, coordsType)));
        return CreateEffectRef(effect);
    }

    /** Variant that requires the client to install a custom kVec2 vertex attribute that will be
        the source of the coords. No matrix is allowed in this mode. */
    static GrEffectRef* CreateWithCustomCoords(GrTexture* tex, const GrTextureParams& p) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSimpleTextureEffect, (tex,
                                                                  SkMatrix::I(),
                                                                  p,
                                                                  kCustom_CoordsType)));
        return CreateEffectRef(effect);
    }

    virtual ~GrSimpleTextureEffect() {}

    static const char* Name() { return "Texture"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLSimpleTextureEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          bool bilerp,
                          CoordsType coordsType)
        : GrSingleTextureEffect(texture, matrix, bilerp, coordsType) {
        GrAssert(kLocal_CoordsType == coordsType || kPosition_CoordsType == coordsType);
    }

    GrSimpleTextureEffect(GrTexture* texture,
                          const SkMatrix& matrix,
                          const GrTextureParams& params,
                          CoordsType coordsType)
        : GrSingleTextureEffect(texture, matrix, params, coordsType) {
        if (kCustom_CoordsType == coordsType) {
            GrAssert(matrix.isIdentity());
            this->addVertexAttrib(kVec2f_GrSLType);
        }
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = CastEffect<GrSimpleTextureEffect>(other);
        return this->hasSameTextureParamsMatrixAndCoordsType(ste);
    }

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
