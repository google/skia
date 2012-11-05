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

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture)
    : INHERITED(1)
    , fTextureAccess(texture) {
    fMatrix.reset();
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, bool bilerp)
    : INHERITED(1)
    , fTextureAccess(texture, bilerp) {
    fMatrix.reset();
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const GrTextureParams& params)
    : INHERITED(1)
    , fTextureAccess(texture, params) {
    fMatrix.reset();
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m)
    : INHERITED(1)
    , fTextureAccess(texture)
    , fMatrix(m) {
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture, const SkMatrix& m, bool bilerp)
    : INHERITED(1)
    , fTextureAccess(texture, bilerp)
    , fMatrix(m) {
}

GrSingleTextureEffect::GrSingleTextureEffect(GrTexture* texture,
                                             const SkMatrix& m,
                                             const GrTextureParams& params)
    : INHERITED(1)
    , fTextureAccess(texture, params)
    , fMatrix(m) {
}

GrSingleTextureEffect::~GrSingleTextureEffect() {
}

const GrTextureAccess& GrSingleTextureEffect::textureAccess(int index) const {
    GrAssert(0 == index);
    return fTextureAccess;
}

const GrBackendEffectFactory& GrSingleTextureEffect::getFactory() const {
    return GrTBackendEffectFactory<GrSingleTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrSingleTextureEffect);

GrEffect* GrSingleTextureEffect::TestCreate(SkRandom* random,
                                            GrContext* context,
                                            GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    return SkNEW_ARGS(GrSingleTextureEffect, (textures[texIdx], matrix));
}
