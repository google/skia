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
    GrGLSimpleTextureEffect(const GrBackendEffectFactory& factory, const GrDrawEffect& drawEffect)
        : INHERITED (factory)
        , fEffectMatrix(drawEffect.castEffect<GrSimpleTextureEffect>().coordsType()) {
    }

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        SkString fsCoordName;
        GrSLType fsCoordSLType;
        fsCoordSLType = fEffectMatrix.emitCode(builder, key, &fsCoordName);

        builder->fsCodeAppendf("\t%s = ", outputColor);
        builder->fsAppendTextureLookupAndModulate(inputColor,
                                                  samplers[0],
                                                  fsCoordName.c_str(),
                                                  fsCoordSLType);
        builder->fsCodeAppend(";\n");
    }

    static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        const GrSimpleTextureEffect& ste = drawEffect.castEffect<GrSimpleTextureEffect>();
        return GrGLEffectMatrix::GenKey(ste.getMatrix(),
                                        drawEffect,
                                        ste.coordsType(),
                                        ste.texture(0));
    }

    virtual void setData(const GrGLUniformManager& uman,
                         const GrDrawEffect& drawEffect) SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = drawEffect.castEffect<GrSimpleTextureEffect>();
        fEffectMatrix.setData(uman, ste.getMatrix(), drawEffect, ste.texture(0));
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
                                               GrContext*,
                                               const GrDrawTargetCaps&,
                                               GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[random->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrTextureParams params(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                           GrTextureParams::kNone_FilterMode);

    static const CoordsType kCoordsTypes[] = {
        kLocal_CoordsType,
        kPosition_CoordsType
    };
    CoordsType coordsType = kCoordsTypes[random->nextULessThan(GR_ARRAY_COUNT(kCoordsTypes))];

    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    return GrSimpleTextureEffect::Create(textures[texIdx], matrix, coordsType);
}
