/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrRect.h"

class GrGLTextureDomainEffect;

/**
 * Limits a texture's lookup coordinates to a domain. Samples outside the domain are either clamped
 * the edge of the domain or result in a vec4 of zeros. The domain is clipped to normalized texture
 * coords ([0,1]x[0,1] square). Bilinear filtering can cause texels outside the domain to affect the
 * read value unless the caller considers this when calculating the domain. TODO: This should be a
 * helper that can assist an effect rather than effect unto itself.
 */
class GrTextureDomainEffect : public GrSingleTextureEffect {

public:
    /**
     * If SkShader::kDecal_TileMode sticks then this enum could be replaced by SkShader::TileMode.
     * We could also consider replacing/augmenting Decal mode with Border mode where the color
     * outside of the domain is user-specifiable. Decal mode currently has a hard (non-lerped)
     * transition between the border and the interior.
     */
    enum WrapMode {
        kClamp_WrapMode,
        kDecal_WrapMode,
    };

    static GrEffectRef* Create(GrTexture*,
                               const SkMatrix&,
                               const SkRect& domain,
                               WrapMode,
                               bool bilerp,
                               CoordsType = kLocal_CoordsType);

    virtual ~GrTextureDomainEffect();

    static const char* Name() { return "TextureDomain"; }

    typedef GrGLTextureDomainEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;
    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    const SkRect& domain() const { return fTextureDomain; }
    WrapMode wrapMode() const { return fWrapMode; }

    /* Computes a domain that bounds all the texels in texelRect. Note that with bilerp enabled
       texels neighboring the domain may be read. */
    static const SkRect MakeTexelDomain(const GrTexture* texture, const SkIRect& texelRect) {
        SkScalar wInv = SK_Scalar1 / texture->width();
        SkScalar hInv = SK_Scalar1 / texture->height();
        SkRect result = {
            texelRect.fLeft * wInv,
            texelRect.fTop * hInv,
            texelRect.fRight * wInv,
            texelRect.fBottom * hInv
        };
        return result;
    }

protected:
    WrapMode fWrapMode;
    SkRect   fTextureDomain;

private:
    GrTextureDomainEffect(GrTexture*,
                          const SkMatrix&,
                          const GrRect& domain,
                          WrapMode,
                          bool bilerp,
                          CoordsType type);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
