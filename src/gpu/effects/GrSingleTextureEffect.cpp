/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrSingleTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

class GrGLSingleTextureEffect : public GrGLEffect {
public:
    GrGLSingleTextureEffect(const GrBackendEffectFactory& factory, const GrEffect&)
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
        const GrSingleTextureEffect& ste =
            static_cast<const GrSingleTextureEffect&>(*stage.getEffect());
        return GrGLEffectMatrix::GenKey(ste.getMatrix(),
                                        stage.getCoordChangeMatrix(),
                                        ste.texture(0));
    }

    virtual void setData(const GrGLUniformManager& uman, const GrEffectStage& stage) SK_OVERRIDE {
        const GrSingleTextureEffect& ste =
            static_cast<const GrSingleTextureEffect&>(*stage.getEffect());
        fEffectMatrix.setData(uman, ste.getMatrix(), stage.getCoordChangeMatrix(), ste.texture(0));
    }

private:
    GrGLEffectMatrix fEffectMatrix;
    typedef GrGLEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m)
    : fTextureAccess(texture)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m, bool bilerp)
    : fTextureAccess(texture, bilerp)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             const GrTextureParams& params)
    : fTextureAccess(texture, params)
    , fMatrix(m) {
    this->addTextureAccess(&fTextureAccess);
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}

void GrSingleTextureEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    // If the input alpha is 0xff and the texture has no alpha channel, then the output alpha is
    // 0xff
    if ((*validFlags & kA_ValidComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
        GrPixelConfigIsOpaque(fTextureAccess.getTexture()->config())) {
        *validFlags = kA_ValidComponentFlag;
    } else {
        *validFlags = 0;
    }
}

const GrBackendEffectFactory& GrSingleTextureEffect::getFactory() const {
    return GrTBackendEffectFactory<GrSingleTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrSingleTextureEffect);

GrEffectRef* GrSingleTextureEffect::TestCreate(SkRandom* random,
                                               GrContext* context,
                                               GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    return GrSingleTextureEffect::Create(textures[texIdx], matrix);
}
