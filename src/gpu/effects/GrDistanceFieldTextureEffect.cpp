/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

// The distance field is constructed as unsigned char values, so that the zero value is at 128,
// and the range is [-4, 4 - 1/255). Hence our multiplier is 8 - 1/32 and zero threshold is 128/255.
#define MULTIPLIER "7.96875"
#define THRESHOLD "0.50196078431"

class GrGLDistanceFieldTextureEffect : public GrGLVertexEffect {
public:
    GrGLDistanceFieldTextureEffect(const GrBackendEffectFactory& factory,
                                   const GrDrawEffect& drawEffect)
        : INHERITED (factory)
        , fTextureSize(SkSize::Make(-1.f,-1.f)) {}

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        SkASSERT(1 == drawEffect.castEffect<GrDistanceFieldTextureEffect>().numVertexAttribs());

        SkAssertResult(builder->enableFeature(GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));

        SkString fsCoordName;
        const char* vsCoordName;
        const char* fsCoordNamePtr;
        builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsCoordName, &fsCoordNamePtr);
        fsCoordName = fsCoordNamePtr;

        const char* attrName0 =
            builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0])->c_str();
        builder->vsCodeAppendf("\t%s = %s;\n", vsCoordName, attrName0);

        const char* textureSizeUniName = NULL;
        fTextureSizeUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                              kVec2f_GrSLType, "TextureSize",
                                              &textureSizeUniName);

        builder->fsCodeAppend("\tvec4 texColor = ");
        builder->fsAppendTextureLookup(samplers[0],
                                       fsCoordName.c_str(),
                                       kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tfloat distance = " MULTIPLIER "*(texColor.r - " THRESHOLD ");\n");

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.
        builder->fsCodeAppendf("\tvec2 st = %s*%s;\n", fsCoordName.c_str(), textureSizeUniName);
        builder->fsCodeAppend("\tvec2 Jdx = dFdx(st);\n");
        builder->fsCodeAppend("\tvec2 Jdy = dFdy(st);\n");
        builder->fsCodeAppend("\tvec2 st_grad = normalize(st);\n");
        builder->fsCodeAppend("\tvec2 grad = vec2(st_grad.x*Jdx.x + st_grad.y*Jdy.x,\n");
        builder->fsCodeAppend("\t                 st_grad.x*Jdx.y + st_grad.y*Jdy.y);\n");

        // this gives us a smooth step across approximately one fragment
        // (assuming a radius of the diagonal of the fragment, hence a factor of sqrt(2)/2)
        builder->fsCodeAppend("\tfloat afwidth = 0.7071*length(grad);\n");
        builder->fsCodeAppend("\tfloat val = smoothstep(-afwidth, afwidth, distance);\n");

        builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("val")).c_str());
    }

    virtual void setData(const GrGLUniformManager& uman,
                         const GrDrawEffect& drawEffect) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());
        const GrDistanceFieldTextureEffect& distanceFieldEffect =
                                              drawEffect.castEffect<GrDistanceFieldTextureEffect>();
        if (distanceFieldEffect.getSize().width() != fTextureSize.width() ||
            distanceFieldEffect.getSize().height() != fTextureSize.height()) {
            fTextureSize = distanceFieldEffect.getSize();
            uman.set2f(fTextureSizeUni,
                       distanceFieldEffect.getSize().width(),
                       distanceFieldEffect.getSize().height());
        }
    }

private:
    GrGLUniformManager::UniformHandle fTextureSizeUni;
    SkSize                            fTextureSize;

    typedef GrGLVertexEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldTextureEffect::GrDistanceFieldTextureEffect(GrTexture* texture,
                                                           const GrTextureParams& params,
                                                           const SkISize& size)
    : fTextureAccess(texture, params)
    , fSize(SkSize::Make(SkIntToScalar(size.width()), SkIntToScalar(size.height()))) {
    this->addTextureAccess(&fTextureAccess);
    this->addVertexAttrib(kVec2f_GrSLType);
}

bool GrDistanceFieldTextureEffect::onIsEqual(const GrEffect& other) const {
    const GrDistanceFieldTextureEffect& cte = CastEffect<GrDistanceFieldTextureEffect>(other);
    return fTextureAccess == cte.fTextureAccess;
}

void GrDistanceFieldTextureEffect::getConstantColorComponents(GrColor* color,
                                                             uint32_t* validFlags) const {
    if ((*validFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
        GrPixelConfigIsOpaque(this->texture(0)->config())) {
        *validFlags = kA_GrColorComponentFlag;
    } else {
        *validFlags = 0;
    }
}

const GrBackendEffectFactory& GrDistanceFieldTextureEffect::getFactory() const {
    return GrTBackendEffectFactory<GrDistanceFieldTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrDistanceFieldTextureEffect);

GrEffectRef* GrDistanceFieldTextureEffect::TestCreate(SkRandom* random,
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
    SkISize size = SkISize::Make(1024, 2048);

    return GrDistanceFieldTextureEffect::Create(textures[texIdx], params, size);
}
