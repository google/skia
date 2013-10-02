/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSimpleTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

class GrGLSimpleTextureEffect : public GrGLEffect {
public:
    GrGLSimpleTextureEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {
    }

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        builder->fsCodeAppendf("\t%s = ", outputColor);
        builder->fsAppendTextureLookupAndModulate(inputColor,
                                                  samplers[0],
                                                  coords[0].c_str(),
                                                  coords[0].type());
        builder->fsCodeAppend(";\n");
    }

private:
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

    static const GrCoordSet kCoordSets[] = {
        kLocal_GrCoordSet,
        kPosition_GrCoordSet
    };
    GrCoordSet coordSet = kCoordSets[random->nextULessThan(GR_ARRAY_COUNT(kCoordSets))];

    const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
    return GrSimpleTextureEffect::Create(textures[texIdx], matrix, coordSet);
}
