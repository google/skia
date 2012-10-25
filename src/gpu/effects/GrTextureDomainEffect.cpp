/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureDomainEffect.h"
#include "gl/GrGLEffect.h"
#include "GrBackendEffectFactory.h"

class GrGLTextureDomainEffect : public GrGLLegacyEffect {
public:
    GrGLTextureDomainEffect(const GrBackendEffectFactory&, const GrEffect&);

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&, const GrEffect&) SK_OVERRIDE;

    static inline EffectKey GenKey(const GrEffect&, const GrGLCaps&) { return 0; }

private:
    GrGLUniformManager::UniformHandle fNameUni;

    typedef GrGLLegacyEffect INHERITED;
};

GrGLTextureDomainEffect::GrGLTextureDomainEffect(const GrBackendEffectFactory& factory,
                                                 const GrEffect&)
    : INHERITED(factory)
    , fNameUni(GrGLUniformManager::kInvalidUniformHandle) {
}

void GrGLTextureDomainEffect::setupVariables(GrGLShaderBuilder* builder) {
    fNameUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                   kVec4f_GrSLType, "TexDom");
};

void GrGLTextureDomainEffect::emitFS(GrGLShaderBuilder* builder,
                                     const char* outputColor,
                                     const char* inputColor,
                                     const TextureSamplerArray& samplers) {
    builder->fFSCode.appendf("\tvec2 clampCoord = clamp(%s, %s.xy, %s.zw);\n",
                           builder->defaultTexCoordsName(),
                           builder->getUniformCStr(fNameUni),
                           builder->getUniformCStr(fNameUni));

    builder->fFSCode.appendf("\t%s = ", outputColor);
    builder->appendTextureLookupAndModulate(&builder->fFSCode,
                                            inputColor,
                                            samplers[0],
                                            "clampCoord");
    builder->fFSCode.append(";\n");
}

void GrGLTextureDomainEffect::setData(const GrGLUniformManager& uman, const GrEffect& data) {
    const GrTextureDomainEffect& effect = static_cast<const GrTextureDomainEffect&>(data);
    const GrRect& domain = effect.domain();

    float values[4] = {
        GrScalarToFloat(domain.left()),
        GrScalarToFloat(domain.top()),
        GrScalarToFloat(domain.right()),
        GrScalarToFloat(domain.bottom())
    };
    // vertical flip if necessary
    const GrGLTexture* texture = static_cast<const GrGLTexture*>(effect.texture(0));
    if (GrGLTexture::kBottomUp_Orientation == texture->orientation()) {
        values[1] = 1.0f - values[1];
        values[3] = 1.0f - values[3];
        // The top and bottom were just flipped, so correct the ordering
        // of elements so that values = (l, t, r, b).
        SkTSwap(values[1], values[3]);
    }
    uman.set4fv(fNameUni, 0, 1, values);
}


///////////////////////////////////////////////////////////////////////////////

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture, const GrRect& domain)
    : GrSingleTextureEffect(texture)
    , fTextureDomain(domain) {
}

GrTextureDomainEffect::GrTextureDomainEffect(GrTexture* texture,
                                             const GrRect& domain,
                                             const GrTextureParams& params)
    : GrSingleTextureEffect(texture, params)
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
    return SkNEW_ARGS(GrTextureDomainEffect, (textures[texIdx], domain));
}
