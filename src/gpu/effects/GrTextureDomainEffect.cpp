/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomainEffect.h"
#include "GrSimpleTextureEffect.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "SkFloatingPoint.h"

class GrGLTextureDomainEffect : public GrGLEffect {
public:
    GrGLTextureDomainEffect(const GrBackendEffectFactory&, const GrDrawEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&);

private:
    GrGLUniformManager::UniformHandle fNameUni;
    GrGLEffectMatrix                  fEffectMatrix;
    GrGLfloat                         fPrevDomain[4];

    typedef GrGLEffect INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrBackendEffectFactory& factory,
                                                 const GrDrawEffect& drawEffect)
    : INHERITED(factory)
    , fNameUni(GrGLUniformManager::kInvalidUniformHandle)
    , fEffectMatrix(drawEffect.castEffect<GrTextureDomainEffect>().coordsType()) {
    fPrevDomain[0] = SK_FloatNaN;
}

void GrGLTextureDomainEffect::emitCode(GrGLShaderBuilder* builder,
                                       const GrDrawEffect& drawEffect,
                                       EffectKey key,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const TextureSamplerArray& samplers) {
    const GrTextureDomainEffect& texDom = drawEffect.castEffect<GrTextureDomainEffect>();

    const char* coords;
    fEffectMatrix.emitCodeMakeFSCoords2D(builder, key, &coords);
    const char* domain;
    fNameUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                    kVec4f_GrSLType, "TexDom", &domain);
    if (GrTextureDomainEffect::kClamp_WrapMode == texDom.wrapMode()) {

        builder->fsCodeAppendf("\tvec2 clampCoord = clamp(%s, %s.xy, %s.zw);\n",
                                coords, domain, domain);

        builder->fsCodeAppendf("\t%s = ", outputColor);
        builder->appendTextureLookupAndModulate(GrGLShaderBuilder::kFragment_ShaderType,
                                                inputColor,
                                                samplers[0],
                                                "clampCoord");
        builder->fsCodeAppend(";\n");
    } else {
        GrAssert(GrTextureDomainEffect::kDecal_WrapMode == texDom.wrapMode());

        if (kImagination_GrGLVendor == builder->ctxInfo().vendor()) {
            // On the NexusS and GalaxyNexus, the other path (with the 'any'
            // call) causes the compilation error "Calls to any function that
            // may require a gradient calculation inside a conditional block
            // may return undefined results". This appears to be an issue with
            // the 'any' call since even the simple "result=black; if (any())
            // result=white;" code fails to compile.
            builder->fsCodeAppend("\tvec4 outside = vec4(0.0, 0.0, 0.0, 0.0);\n");
            builder->fsCodeAppend("\tvec4 inside = ");
            builder->appendTextureLookupAndModulate(GrGLShaderBuilder::kFragment_ShaderType,
                                                    inputColor,
                                                    samplers[0],
                                                    coords);
            builder->fsCodeAppend(";\n");

            builder->fsCodeAppendf("\tfloat x = abs(2.0*(%s.x - %s.x)/(%s.z - %s.x) - 1.0);\n",
                                   coords, domain, domain, domain);
            builder->fsCodeAppendf("\tfloat y = abs(2.0*(%s.y - %s.y)/(%s.w - %s.y) - 1.0);\n",
                                   coords, domain, domain, domain);
            builder->fsCodeAppend("\tfloat blend = step(1.0, max(x, y));\n");
            builder->fsCodeAppendf("\t%s = mix(inside, outside, blend);\n", outputColor);
        } else {
            builder->fsCodeAppend("\tbvec4 outside;\n");
            builder->fsCodeAppendf("\toutside.xy = lessThan(%s, %s.xy);\n", coords, domain);
            builder->fsCodeAppendf("\toutside.zw = greaterThan(%s, %s.zw);\n", coords, domain);
            builder->fsCodeAppendf("\t%s = any(outside) ? vec4(0.0, 0.0, 0.0, 0.0) : ", outputColor);
            builder->appendTextureLookupAndModulate(GrGLShaderBuilder::kFragment_ShaderType,
                                                    inputColor,
                                                    samplers[0],
                                                    coords);
            builder->fsCodeAppend(";\n");
        }
    }
}

void GrGLTextureDomainEffect::setData(const GrGLUniformManager& uman,
                                      const GrDrawEffect& drawEffect) {
    const GrTextureDomainEffect& texDom = drawEffect.castEffect<GrTextureDomainEffect>();
    const GrRect& domain = texDom.domain();

    float values[4] = {
        SkScalarToFloat(domain.left()),
        SkScalarToFloat(domain.top()),
        SkScalarToFloat(domain.right()),
        SkScalarToFloat(domain.bottom())
    };
    // vertical flip if necessary
    if (kBottomLeft_GrSurfaceOrigin == texDom.texture(0)->origin()) {
        values[1] = 1.0f - values[1];
        values[3] = 1.0f - values[3];
        // The top and bottom were just flipped, so correct the ordering
        // of elements so that values = (l, t, r, b).
        SkTSwap(values[1], values[3]);
    }
    if (0 != memcmp(values, fPrevDomain, 4 * sizeof(GrGLfloat))) {
        uman.set4fv(fNameUni, 0, 1, values);
    }
    fEffectMatrix.setData(uman,
                          texDom.getMatrix(),
                          drawEffect,
                          texDom.texture(0));
}

GrGLEffect::EffectKey GrGLTextureDomainEffect::GenKey(const GrDrawEffect& drawEffect,
                                                      const GrGLCaps&) {
    const GrTextureDomainEffect& texDom = drawEffect.castEffect<GrTextureDomainEffect>();
    EffectKey key = texDom.wrapMode();
    key <<= GrGLEffectMatrix::kKeyBits;
    EffectKey matrixKey = GrGLEffectMatrix::GenKey(texDom.getMatrix(),
                                                   drawEffect,
                                                   texDom.coordsType(),
                                                   texDom.texture(0));
    return key | matrixKey;
}


///////////////////////////////////////////////////////////////////////////////

GrEffectRef* GrTextureDomainEffect::Create(GrTexture* texture,
                                           const SkMatrix& matrix,
                                           const GrRect& domain,
                                           WrapMode wrapMode,
                                           bool bilerp,
                                           CoordsType coordsType) {
    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (kClamp_WrapMode == wrapMode && domain.contains(kFullRect)) {
        return GrSimpleTextureEffect::Create(texture, matrix, bilerp);
    } else {
        SkRect clippedDomain;
        // We don't currently handle domains that are empty or don't intersect the texture.
        // It is OK if the domain rect is a line or point, but it should not be inverted. We do not
        // handle rects that do not intersect the [0..1]x[0..1] rect.
        GrAssert(domain.fLeft <= domain.fRight);
        GrAssert(domain.fTop <= domain.fBottom);
        clippedDomain.fLeft = SkMaxScalar(domain.fLeft, kFullRect.fLeft);
        clippedDomain.fRight = SkMinScalar(domain.fRight, kFullRect.fRight);
        clippedDomain.fTop = SkMaxScalar(domain.fTop, kFullRect.fTop);
        clippedDomain.fBottom = SkMinScalar(domain.fBottom, kFullRect.fBottom);
        GrAssert(clippedDomain.fLeft <= clippedDomain.fRight);
        GrAssert(clippedDomain.fTop <= clippedDomain.fBottom);

        AutoEffectUnref effect(SkNEW_ARGS(GrTextureDomainEffect, (texture,
                                                                  matrix,
                                                                  clippedDomain,
                                                                  wrapMode,
                                                                  bilerp,
                                                                  coordsType)));
        return CreateEffectRef(effect);

    }
}

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture,
                                             const SkMatrix& matrix,
                                             const GrRect& domain,
                                             WrapMode wrapMode,
                                             bool bilerp,
                                             CoordsType coordsType)
    : GrSingleTextureEffect(texture, matrix, bilerp, coordsType)
    , fWrapMode(wrapMode)
    , fTextureDomain(domain) {
}

GrTextureDomainEffect::~GrTextureDomainEffect() {

}

const GrBackendEffectFactory& GrTextureDomainEffect::getFactory() const {
    return GrTBackendEffectFactory<GrTextureDomainEffect>::getInstance();
}

bool GrTextureDomainEffect::onIsEqual(const GrEffect& sBase) const {
    const GrTextureDomainEffect& s = CastEffect<GrTextureDomainEffect>(sBase);
    return this->hasSameTextureParamsMatrixAndCoordsType(s) &&
           this->fTextureDomain == s.fTextureDomain;
}

void GrTextureDomainEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    if (kDecal_WrapMode == fWrapMode) {
        *validFlags = 0;
    } else {
        this->updateConstantColorComponentsForModulation(color, validFlags);
    }
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrTextureDomainEffect);

GrEffectRef* GrTextureDomainEffect::TestCreate(SkMWCRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps&,
                                               GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    GrRect domain;
    domain.fLeft = random->nextUScalar1();
    domain.fRight = random->nextRangeScalar(domain.fLeft, SK_Scalar1);
    domain.fTop = random->nextUScalar1();
    domain.fBottom = random->nextRangeScalar(domain.fTop, SK_Scalar1);
    WrapMode wrapMode = random->nextBool() ? kClamp_WrapMode : kDecal_WrapMode;
    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    bool bilerp = random->nextBool();
    CoordsType coords = random->nextBool() ? kLocal_CoordsType : kPosition_CoordsType;
    return GrTextureDomainEffect::Create(textures[texIdx],
                                         matrix,
                                         domain,
                                         wrapMode,
                                         bilerp,
                                         coords);
}
