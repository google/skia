/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSimpleTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

class GrGLSimpleTextureEffect : public GrGLEffect {
public:
    GrGLSimpleTextureEffect(const GrBackendEffectFactory& factory, const GrEffectRef&)
    : INHERITED (factory) {}

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrEffectStage&,
                          EffectKey key,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const char* coordName;
        GrSLType coordType = fEffectMatrix.emitCode(builder, key, vertexCoords, &coordName);
        builder->fFSCode.appendf("\t%s = ", outputColor);
        builder->appendTextureLookupAndModulate(&builder->fFSCode,
                                                inputColor,
                                                samplers[0],
                                                coordName,
                                                coordType);
        builder->fFSCode.append(";\n");
    }

    static inline EffectKey GenKey(const GrEffectStage& stage, const GrGLCaps&) {
        const GrSimpleTextureEffect& ste = GetEffectFromStage<GrSimpleTextureEffect>(stage);
        return GrGLEffectMatrix::GenKey(ste.getMatrix(),
                                        stage.getCoordChangeMatrix(),
                                        ste.texture(0));
    }

    virtual void setData(const GrGLUniformManager& uman, const GrEffectStage& stage) SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = GetEffectFromStage<GrSimpleTextureEffect>(stage);
        fEffectMatrix.setData(uman, ste.getMatrix(), stage.getCoordChangeMatrix(), ste.texture(0));
    }

private:
    GrGLEffectMatrix fEffectMatrix;
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void GrSimpleTextureEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

const GrBackendEffectFactory& GrSimpleTextureEffect::getFactory() const {
    return GrTBackendEffectFactory<GrSimpleTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrSimpleTextureEffect);

GrEffectRef* GrSimpleTextureEffect::TestCreate(SkRandom* random,
                                               GrContext* context,
                                               GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    return GrSimpleTextureEffect::Create(textures[texIdx], matrix);
}
