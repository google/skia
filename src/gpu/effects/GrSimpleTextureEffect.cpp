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
    : INHERITED (factory) {
        GrEffect::CoordsType coordsType =
            drawEffect.castEffect<GrSimpleTextureEffect>().coordsType();
        if (GrEffect::kCustom_CoordsType != coordsType) {
            SkNEW_IN_TLAZY(&fEffectMatrix, GrGLEffectMatrix, (coordsType));
        }
    }

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = drawEffect.castEffect<GrSimpleTextureEffect>();
        const char* fsCoordName;
        GrSLType fsCoordSLType;
        if (GrEffect::kCustom_CoordsType == ste.coordsType()) {
            GrAssert(ste.getMatrix().isIdentity());
            GrAssert(1 == ste.numVertexAttribs());
            fsCoordSLType = kVec2f_GrSLType;
            const char* vsVaryingName;
            builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsVaryingName, &fsCoordName);
            const char* attrName =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0])->c_str();
            builder->vsCodeAppendf("\t%s = %s;", vsVaryingName, attrName);
        } else {
            fsCoordSLType = fEffectMatrix.get()->emitCode(builder, key, &fsCoordName);
        }
        builder->fsCodeAppendf("\t%s = ", outputColor);
        builder->appendTextureLookupAndModulate(GrGLShaderBuilder::kFragment_ShaderType,
                                                inputColor,
                                                samplers[0],
                                                fsCoordName,
                                                fsCoordSLType);
        builder->fsCodeAppend(";\n");
    }

    static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        const GrSimpleTextureEffect& ste = drawEffect.castEffect<GrSimpleTextureEffect>();
        if (GrEffect::kCustom_CoordsType == ste.coordsType()) {
            return 1 << GrGLEffectMatrix::kKeyBits;
        } else {
            return GrGLEffectMatrix::GenKey(ste.getMatrix(),
                                            drawEffect,
                                            ste.coordsType(),
                                            ste.texture(0));
        }
    }

    virtual void setData(const GrGLUniformManager& uman,
                         const GrDrawEffect& drawEffect) SK_OVERRIDE {
        const GrSimpleTextureEffect& ste = drawEffect.castEffect<GrSimpleTextureEffect>();
        if (GrEffect::kCustom_CoordsType == ste.coordsType()) {
            GrAssert(ste.getMatrix().isIdentity());
        } else {
            fEffectMatrix.get()->setData(uman, ste.getMatrix(), drawEffect, ste.texture(0));
        }
    }

private:
    SkTLazy<GrGLEffectMatrix> fEffectMatrix;
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

GrEffectRef* GrSimpleTextureEffect::TestCreate(SkMWCRandom* random,
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
    GrTextureParams params(tileModes, random->nextBool());

    static const CoordsType kCoordsTypes[] = {
        kLocal_CoordsType,
        kPosition_CoordsType,
        kCustom_CoordsType
    };
    CoordsType coordsType = kCoordsTypes[random->nextULessThan(GR_ARRAY_COUNT(kCoordsTypes))];

    if (kCustom_CoordsType == coordsType) {
        return GrSimpleTextureEffect::CreateWithCustomCoords(textures[texIdx], params);
    } else {
        const SkMatrix& matrix = GrEffectUnitTest::TestMatrix(random);
        return GrSimpleTextureEffect::Create(textures[texIdx], matrix);
    }
}
