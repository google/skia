/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextureEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"
#include "GrTexture.h"

#include "SkDistanceFieldGen.h"

// To get optical sizes people don't complain about when we blit correctly,
// we need to slightly bold each glyph. On the Mac, we need a larger bold value.
#if defined(SK_BUILD_FOR_MAC)
#define SK_DistanceFieldLCDFactor    "0.33"
#define SK_DistanceFieldNonLCDFactor "0.25"
#else
#define SK_DistanceFieldLCDFactor    "0.05"
#define SK_DistanceFieldNonLCDFactor "0.05"
#endif

// Assuming a radius of the diagonal of the fragment, hence a factor of sqrt(2)/2
#define SK_DistanceFieldAAFactor     "0.7071"

class GrGLDistanceFieldTextureEffect : public GrGLVertexEffect {
public:
    GrGLDistanceFieldTextureEffect(const GrBackendEffectFactory& factory,
                                   const GrDrawEffect& drawEffect)
        : INHERITED (factory)
        , fTextureSize(SkISize::Make(-1,-1)) {}

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        SkASSERT(1 == drawEffect.castEffect<GrDistanceFieldTextureEffect>().numVertexAttribs());

        SkAssertResult(builder->enableFeature(GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
        const GrDistanceFieldTextureEffect& dfTexEffect =
                                              drawEffect.castEffect<GrDistanceFieldTextureEffect>();

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
        builder->fsCodeAppend("\tfloat distance = "
                          SK_DistanceFieldMultiplier "*(texColor.r - " SK_DistanceFieldThreshold ")"
                          "+ " SK_DistanceFieldNonLCDFactor ";\n");

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.
        builder->fsCodeAppendf("\tvec2 uv = %s;\n", fsCoordName.c_str());
        builder->fsCodeAppendf("\tvec2 st = uv*%s;\n", textureSizeUniName);
        builder->fsCodeAppend("\tfloat afwidth;\n");
        if (dfTexEffect.isSimilarity()) {
            // this gives us a smooth step across approximately one fragment
            builder->fsCodeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*dFdx(st.x);\n");
        } else {
            builder->fsCodeAppend("\tvec2 Jdx = dFdx(st);\n");
            builder->fsCodeAppend("\tvec2 Jdy = dFdy(st);\n");

            builder->fsCodeAppend("\tvec2 uv_grad;\n");
            if (builder->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                builder->fsCodeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                builder->fsCodeAppend("\tif (uv_len2 < 0.0001) {\n");
                builder->fsCodeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                builder->fsCodeAppend("\t} else {\n");
                builder->fsCodeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                builder->fsCodeAppend("\t}\n");
            } else {
                builder->fsCodeAppend("\tuv_grad = normalize(uv);\n");
            }
            builder->fsCodeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            builder->fsCodeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            builder->fsCodeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }
        builder->fsCodeAppend("\tfloat val = smoothstep(-afwidth, afwidth, distance);\n");

#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* luminanceUniName = NULL;
        // width, height, 1/(3*width)
        fLuminanceUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kFloat_GrSLType, "Luminance",
                                            &luminanceUniName);

        builder->fsCodeAppendf("\tuv = vec2(val, %s);\n", luminanceUniName);
        builder->fsCodeAppend("\tvec4 gammaColor = ");
        builder->fsAppendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tval = gammaColor.r;\n");
#endif

        builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("val")).c_str());
    }

    virtual void setData(const GrGLUniformManager& uman,
                         const GrDrawEffect& drawEffect) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());

        GrTexture* texture = drawEffect.effect()->texture(0);
        if (texture->width() != fTextureSize.width() ||
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            uman.set2f(fTextureSizeUni,
                       SkIntToScalar(fTextureSize.width()),
                       SkIntToScalar(fTextureSize.height()));
        }
#ifdef SK_GAMMA_APPLY_TO_A8
        const GrDistanceFieldTextureEffect& dfTexEffect =
                                              drawEffect.castEffect<GrDistanceFieldTextureEffect>();
        float luminance = dfTexEffect.getLuminance();
        if (luminance != fLuminance) {
            uman.set1f(fLuminanceUni, luminance);
            fLuminance = luminance;
        }
#endif
    }

    static inline void GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                              GrEffectKeyBuilder* b) {
        const GrDistanceFieldTextureEffect& dfTexEffect =
                                              drawEffect.castEffect<GrDistanceFieldTextureEffect>();

        b->add32(dfTexEffect.isSimilarity());
    }

private:
    GrGLUniformManager::UniformHandle fTextureSizeUni;
    SkISize                           fTextureSize;
    GrGLUniformManager::UniformHandle fLuminanceUni;
    float                             fLuminance;

    typedef GrGLVertexEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldTextureEffect::GrDistanceFieldTextureEffect(GrTexture* texture,
                                                           const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           GrTexture* gamma,
                                                           const GrTextureParams& gammaParams,
                                                           float luminance,
#endif
                                                           bool similarity)
    : fTextureAccess(texture, params)
#ifdef SK_GAMMA_APPLY_TO_A8
    , fGammaTextureAccess(gamma, gammaParams)
    , fLuminance(luminance)
#endif
    , fIsSimilarity(similarity) {
    this->addTextureAccess(&fTextureAccess);
#ifdef SK_GAMMA_APPLY_TO_A8
    this->addTextureAccess(&fGammaTextureAccess);
#endif
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

GrEffect* GrDistanceFieldTextureEffect::TestCreate(SkRandom* random,
                                                   GrContext*,
                                                   const GrDrawTargetCaps&,
                                                   GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
#ifdef SK_GAMMA_APPLY_TO_A8
    int texIdx2 = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                       GrEffectUnitTest::kAlphaTextureIdx;
#endif
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
#ifdef SK_GAMMA_APPLY_TO_A8
    GrTextureParams params2(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                                                            GrTextureParams::kNone_FilterMode);
#endif

    return GrDistanceFieldTextureEffect::Create(textures[texIdx], params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                textures[texIdx2], params2,
                                                random->nextF(),
#endif
                                                random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldLCDTextureEffect : public GrGLVertexEffect {
public:
    GrGLDistanceFieldLCDTextureEffect(const GrBackendEffectFactory& factory,
                                      const GrDrawEffect& drawEffect)
    : INHERITED (factory)
    , fTextureSize(SkISize::Make(-1,-1)) {}

    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          const GrEffectKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        SkASSERT(1 == drawEffect.castEffect<GrDistanceFieldLCDTextureEffect>().numVertexAttribs());

        SkAssertResult(builder->enableFeature(GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                                           drawEffect.castEffect<GrDistanceFieldLCDTextureEffect>();

        SkString fsCoordName;
        const char* vsCoordName;
        const char* fsCoordNamePtr;
        builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsCoordName, &fsCoordNamePtr);
        fsCoordName = fsCoordNamePtr;

        const char* attrName0 =
                   builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0])->c_str();
        builder->vsCodeAppendf("\t%s = %s;\n", vsCoordName, attrName0);

        const char* textureSizeUniName = NULL;
        // width, height, 1/(3*width)
        fTextureSizeUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                              kVec3f_GrSLType, "TextureSize",
                                              &textureSizeUniName);

        // create LCD offset adjusted by inverse of transform
        builder->fsCodeAppendf("\tvec2 uv = %s;\n", fsCoordName.c_str());
        builder->fsCodeAppendf("\tvec2 st = uv*%s.xy;\n", textureSizeUniName);
        if (dfTexEffect.isUniformScale()) {
            builder->fsCodeAppend("\tfloat dx = dFdx(st.x);\n");
            builder->fsCodeAppendf("\tvec2 offset = vec2(dx*%s.z, 0.0);\n", textureSizeUniName);
        } else {
            builder->fsCodeAppend("\tvec2 Jdx = dFdx(st);\n");
            builder->fsCodeAppend("\tvec2 Jdy = dFdy(st);\n");
            builder->fsCodeAppendf("\tvec2 offset = %s.z*Jdx;\n", textureSizeUniName);
        }

        // green is distance to uv center
        builder->fsCodeAppend("\tvec4 texColor = ");
        builder->fsAppendTextureLookup(samplers[0], "uv", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tvec3 distance;\n");
        builder->fsCodeAppend("\tdistance.y = texColor.r;\n");
        // red is distance to left offset
        builder->fsCodeAppend("\tvec2 uv_adjusted = uv - offset;\n");
        builder->fsCodeAppend("\ttexColor = ");
        builder->fsAppendTextureLookup(samplers[0], "uv_adjusted", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tdistance.x = texColor.r;\n");
        // blue is distance to right offset
        builder->fsCodeAppend("\tuv_adjusted = uv + offset;\n");
        builder->fsCodeAppend("\ttexColor = ");
        builder->fsAppendTextureLookup(samplers[0], "uv_adjusted", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tdistance.z = texColor.r;\n");

        builder->fsCodeAppend("\tdistance = "
            "vec3(" SK_DistanceFieldMultiplier ")*(distance - vec3(" SK_DistanceFieldThreshold"))"
            "+ vec3(" SK_DistanceFieldLCDFactor ");\n");

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        builder->fsCodeAppend("\tfloat afwidth;\n");
        if (dfTexEffect.isUniformScale()) {
            // this gives us a smooth step across approximately one fragment
            builder->fsCodeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*dx;\n");
        } else {
            builder->fsCodeAppend("\tvec2 uv_grad;\n");
            if (builder->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                builder->fsCodeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                builder->fsCodeAppend("\tif (uv_len2 < 0.0001) {\n");
                builder->fsCodeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                builder->fsCodeAppend("\t} else {\n");
                builder->fsCodeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                builder->fsCodeAppend("\t}\n");
            } else {
                builder->fsCodeAppend("\tuv_grad = normalize(uv);\n");
            }
            builder->fsCodeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            builder->fsCodeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            builder->fsCodeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }

        builder->fsCodeAppend("\tvec4 val = vec4(smoothstep(vec3(-afwidth), vec3(afwidth), distance), 1.0);\n");

        // adjust based on gamma
        const char* textColorUniName = NULL;
        // width, height, 1/(3*width)
        fTextColorUni = builder->addUniform(GrGLShaderBuilder::kFragment_Visibility,
                                            kVec3f_GrSLType, "TextColor",
                                            &textColorUniName);

        builder->fsCodeAppendf("\tuv = vec2(val.x, %s.x);\n", textColorUniName);
        builder->fsCodeAppend("\tvec4 gammaColor = ");
        builder->fsAppendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tval.x = gammaColor.r;\n");

        builder->fsCodeAppendf("\tuv = vec2(val.y, %s.y);\n", textColorUniName);
        builder->fsCodeAppend("\tgammaColor = ");
        builder->fsAppendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tval.y = gammaColor.r;\n");

        builder->fsCodeAppendf("\tuv = vec2(val.z, %s.z);\n", textColorUniName);
        builder->fsCodeAppend("\tgammaColor = ");
        builder->fsAppendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        builder->fsCodeAppend(";\n");
        builder->fsCodeAppend("\tval.z = gammaColor.r;\n");

        builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                               (GrGLSLExpr4(inputColor) * GrGLSLExpr4("val")).c_str());
    }

    virtual void setData(const GrGLUniformManager& uman,
                         const GrDrawEffect& drawEffect) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());
        SkASSERT(fTextColorUni.isValid());

        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                                    drawEffect.castEffect<GrDistanceFieldLCDTextureEffect>();
        GrTexture* texture = drawEffect.effect()->texture(0);
        if (texture->width() != fTextureSize.width() ||
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            float delta = 1.0f/(3.0f*texture->width());
            if (dfTexEffect.useBGR()) {
                delta = -delta;
            }
            uman.set3f(fTextureSizeUni,
                       SkIntToScalar(fTextureSize.width()),
                       SkIntToScalar(fTextureSize.height()),
                       delta);
        }

        GrColor textColor = dfTexEffect.getTextColor();
        if (textColor != fTextColor) {
            static const float ONE_OVER_255 = 1.f / 255.f;
            uman.set3f(fTextColorUni,
                       GrColorUnpackR(textColor) * ONE_OVER_255,
                       GrColorUnpackG(textColor) * ONE_OVER_255,
                       GrColorUnpackB(textColor) * ONE_OVER_255);
            fTextColor = textColor;
        }
    }

    static inline void GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&,
                              GrEffectKeyBuilder* b) {
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                                           drawEffect.castEffect<GrDistanceFieldLCDTextureEffect>();

        b->add32(dfTexEffect.isUniformScale());
    }

private:
    GrGLUniformManager::UniformHandle fTextureSizeUni;
    SkISize                           fTextureSize;
    GrGLUniformManager::UniformHandle fTextColorUni;
    SkColor                           fTextColor;

    typedef GrGLVertexEffect INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextureEffect::GrDistanceFieldLCDTextureEffect(
                                                  GrTexture* texture, const GrTextureParams& params,
                                                  GrTexture* gamma, const GrTextureParams& gParams,
                                                  SkColor textColor,
                                                  bool uniformScale, bool useBGR)
    : fTextureAccess(texture, params)
    , fGammaTextureAccess(gamma, gParams)
    , fTextColor(textColor)
    , fUniformScale(uniformScale)
    , fUseBGR(useBGR) {
    this->addTextureAccess(&fTextureAccess);
    this->addTextureAccess(&fGammaTextureAccess);
    this->addVertexAttrib(kVec2f_GrSLType);
}

bool GrDistanceFieldLCDTextureEffect::onIsEqual(const GrEffect& other) const {
    const GrDistanceFieldLCDTextureEffect& cte = 
                                            CastEffect<GrDistanceFieldLCDTextureEffect>(other);
    return (fTextureAccess == cte.fTextureAccess && fGammaTextureAccess == cte.fGammaTextureAccess);
}

void GrDistanceFieldLCDTextureEffect::getConstantColorComponents(GrColor* color,
                                                                 uint32_t* validFlags) const {
    if ((*validFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(*color) &&
        GrPixelConfigIsOpaque(this->texture(0)->config())) {
        *validFlags = kA_GrColorComponentFlag;
    } else {
        *validFlags = 0;
    }
}

const GrBackendEffectFactory& GrDistanceFieldLCDTextureEffect::getFactory() const {
    return GrTBackendEffectFactory<GrDistanceFieldLCDTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrDistanceFieldLCDTextureEffect);

GrEffect* GrDistanceFieldLCDTextureEffect::TestCreate(SkRandom* random,
                                                      GrContext*,
                                                      const GrDrawTargetCaps&,
                                                      GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
                                      GrEffectUnitTest::kAlphaTextureIdx;
    int texIdx2 = random->nextBool() ? GrEffectUnitTest::kSkiaPMTextureIdx :
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
    GrTextureParams params2(tileModes, random->nextBool() ? GrTextureParams::kBilerp_FilterMode :
                           GrTextureParams::kNone_FilterMode);
    GrColor textColor = GrColorPackRGBA(random->nextULessThan(256),
                                        random->nextULessThan(256),
                                        random->nextULessThan(256),
                                        random->nextULessThan(256));
    return GrDistanceFieldLCDTextureEffect::Create(textures[texIdx], params,
                                                   textures[texIdx2], params2,
                                                   textColor,
                                                   random->nextBool(), random->nextBool());
}
