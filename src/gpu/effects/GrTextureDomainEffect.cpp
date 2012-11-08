/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomainEffect.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "SkFloatingPoint.h"

class GrGLTextureDomainEffect : public GrGLEffect {
public:
    GrGLTextureDomainEffect(const GrBackendEffectFactory&, const GrEffect&);

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrEffectStage&,
                          EffectKey,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrEffectStage&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrEffectStage&, const GrGLCaps&);

private:
    GrGLUniformManager::UniformHandle fNameUni;
    GrGLEffectMatrix                  fEffectMatrix;
    GrGLfloat                         fPrevDomain[4];

    typedef GrGLEffect INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrBackendEffectFactory& factory,
                                                 const GrEffect&)
    : INHERITED(factory)
    , fNameUni(GrGLUniformManager::kInvalidUniformHandle) {
    fPrevDomain[0] = SK_FloatNaN;
}

void GrGLTextureDomainEffect::emitCode(GrGLShaderBuilder* builder,
                                       const GrEffectStage& stage,
                                       EffectKey key,
                                       const char* vertexCoords,
                                       const char* outputColor,
                                       const char* inputColor,
                                       const TextureSamplerArray& samplers) {
    const GrTextureDomainEffect& effect =
        static_cast<const GrTextureDomainEffect&>(*stage.getEffect());

    const char* coords;
    fEffectMatrix.emitCodeMakeFSCoords2D(builder, key, vertexCoords, &coords);
    const char* domain;
    fNameUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                    kVec4f_GrSLType, "TexDom", &domain);
    if (GrTextureDomainEffect::kClamp_WrapMode == effect.wrapMode()) {

        builder->fFSCode.appendf("\tvec2 clampCoord = clamp(%s, %s.xy, %s.zw);\n",
                               coords, domain, domain);

        builder->fFSCode.appendf("\t%s = ", outputColor);
        builder->appendTextureLookupAndModulate(&builder->fFSCode,
                                                inputColor,
                                                samplers[0],
                                                "clampCoord");
        builder->fFSCode.append(";\n");
    } else {
        GrAssert(GrTextureDomainEffect::kDecal_WrapMode == effect.wrapMode());
        builder->fFSCode.append("\tbvec4 outside;\n");
        builder->fFSCode.appendf("\toutside.xy = lessThan(%s, %s.xy);\n", coords, domain);
        builder->fFSCode.appendf("\toutside.zw = greaterThan(%s, %s.zw);\n", coords, domain);
        builder->fFSCode.appendf("\t%s = any(outside) ? vec4(0.0, 0.0, 0.0, 0.0) : ", outputColor);
        builder->appendTextureLookupAndModulate(&builder->fFSCode, inputColor, samplers[0], coords);
        builder->fFSCode.append(";\n");
    }
}

void GrGLTextureDomainEffect::setData(const GrGLUniformManager& uman, const GrEffectStage& stage) {
    const GrTextureDomainEffect& effect =
        static_cast<const GrTextureDomainEffect&>(*stage.getEffect());
    const GrRect& domain = effect.domain();

    float values[4] = {
        SkScalarToFloat(domain.left()),
        SkScalarToFloat(domain.top()),
        SkScalarToFloat(domain.right()),
        SkScalarToFloat(domain.bottom())
    };
    // vertical flip if necessary
    if (GrSurface::kBottomLeft_Origin == effect.texture(0)->origin()) {
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
                          effect.getMatrix(),
                          stage.getCoordChangeMatrix(),
                          effect.texture(0));
}

GrGLEffect::EffectKey GrGLTextureDomainEffect::GenKey(const GrEffectStage& stage, const GrGLCaps&) {
    const GrTextureDomainEffect& effect =
        static_cast<const GrTextureDomainEffect&>(*stage.getEffect());
    EffectKey key = effect.wrapMode();
    key <<= GrGLEffectMatrix::kKeyBits;
    EffectKey matrixKey = GrGLEffectMatrix::GenKey(effect.getMatrix(),
                                                   stage.getCoordChangeMatrix(),
                                                   effect.texture(0));
    return key | matrixKey;
}


///////////////////////////////////////////////////////////////////////////////

GrEffect* GrTextureDomainEffect::Create(GrTexture* texture,
                                        const SkMatrix& matrix,
                                        const GrRect& domain,
                                        WrapMode wrapMode,
                                        bool bilerp) {
    static const SkRect kFullRect = {0, 0, SK_Scalar1, SK_Scalar1};
    if (kClamp_WrapMode == wrapMode && domain.contains(kFullRect)) {
        return SkNEW_ARGS(GrSingleTextureEffect, (texture, matrix, bilerp));
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
        return SkNEW_ARGS(GrTextureDomainEffect,
                          (texture, matrix, clippedDomain, wrapMode, bilerp));
    }
}

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture,
                                             const SkMatrix& matrix,
                                             const GrRect& domain,
                                             WrapMode wrapMode,
                                             bool bilerp)
    : GrSingleTextureEffect(texture, matrix, bilerp)
    , fWrapMode(wrapMode)
    , fTextureDomain(domain) {
}

GrTextureDomainEffect::~GrTextureDomainEffect() {

}

const GrBackendEffectFactory& GrTextureDomainEffect::getFactory() const {
    return GrTBackendEffectFactory<GrTextureDomainEffect>::getInstance();
}

bool GrTextureDomainEffect::isEqual(const GrEffect& sBase) const {
    const GrTextureDomainEffect& s = static_cast<const GrTextureDomainEffect&>(sBase);
    return (INHERITED::isEqual(sBase) && this->fTextureDomain == s.fTextureDomain);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrTextureDomainEffect);

GrEffect* GrTextureDomainEffect::TestCreate(SkRandom* random,
                                            GrContext* context,
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
    return GrTextureDomainEffect::Create(textures[texIdx], matrix, domain, wrapMode);
}
