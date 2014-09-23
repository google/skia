/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDistanceFieldTextureEffect.h"
#include "gl/builders/GrGLFullProgramBuilder.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "gl/GrGLGeometryProcessor.h"
#include "GrTBackendProcessorFactory.h"
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

class GrGLDistanceFieldTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldTextureEffect(const GrBackendProcessorFactory& factory,
                                   const GrProcessor&)
        : INHERITED (factory)
        , fTextureSize(SkISize::Make(-1,-1))
#ifdef SK_GAMMA_APPLY_TO_A8
        , fLuminance(-1.0f)
#endif
        {}

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrDistanceFieldTextureEffect& dfTexEffect =
                geometryProcessor.cast<GrDistanceFieldTextureEffect>();
        SkASSERT(1 == dfTexEffect.getVertexAttribs().count());

        GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        SkString fsCoordName;
        const char* vsCoordName;
        const char* fsCoordNamePtr;
        builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsCoordName, &fsCoordNamePtr);
        fsCoordName = fsCoordNamePtr;

        GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
        vsBuilder->codeAppendf("\t%s = %s;\n", vsCoordName, dfTexEffect.inTextureCoords().c_str());

        const char* textureSizeUniName = NULL;
        fTextureSizeUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                              kVec2f_GrSLType, "TextureSize",
                                              &textureSizeUniName);

        fsBuilder->codeAppend("\tvec4 texColor = ");
        fsBuilder->appendTextureLookup(samplers[0],
                                       fsCoordName.c_str(),
                                       kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tfloat distance = "
                          SK_DistanceFieldMultiplier "*(texColor.r - " SK_DistanceFieldThreshold ")"
                          "+ " SK_DistanceFieldNonLCDFactor ";\n");

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.
        fsBuilder->codeAppendf("\tvec2 uv = %s;\n", fsCoordName.c_str());
        fsBuilder->codeAppendf("\tvec2 st = uv*%s;\n", textureSizeUniName);
        fsBuilder->codeAppend("\tfloat afwidth;\n");
        if (dfTexEffect.getFlags() & kSimilarity_DistanceFieldEffectFlag) {
            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*dFdx(st.x);\n");
        } else {
            fsBuilder->codeAppend("\tvec2 Jdx = dFdx(st);\n");
            fsBuilder->codeAppend("\tvec2 Jdy = dFdy(st);\n");

            fsBuilder->codeAppend("\tvec2 uv_grad;\n");
            if (builder->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                fsBuilder->codeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                fsBuilder->codeAppend("\tif (uv_len2 < 0.0001) {\n");
                fsBuilder->codeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                fsBuilder->codeAppend("\t} else {\n");
                fsBuilder->codeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                fsBuilder->codeAppend("\t}\n");
            } else {
                fsBuilder->codeAppend("\tuv_grad = normalize(uv);\n");
            }
            fsBuilder->codeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            fsBuilder->codeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }
        fsBuilder->codeAppend("\tfloat val = smoothstep(-afwidth, afwidth, distance);\n");

#ifdef SK_GAMMA_APPLY_TO_A8
        // adjust based on gamma
        const char* luminanceUniName = NULL;
        // width, height, 1/(3*width)
        fLuminanceUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                            kFloat_GrSLType, "Luminance",
                                            &luminanceUniName);

        fsBuilder->codeAppendf("\tuv = vec2(val, %s);\n", luminanceUniName);
        fsBuilder->codeAppend("\tvec4 gammaColor = ");
        fsBuilder->appendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tval = gammaColor.r;\n");
#endif

        fsBuilder->codeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("val")).c_str());
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrProcessor& effect) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());

        GrTexture* texture = effect.texture(0);
        if (texture->width() != fTextureSize.width() ||
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            pdman.set2f(fTextureSizeUni,
                        SkIntToScalar(fTextureSize.width()),
                        SkIntToScalar(fTextureSize.height()));
        }
#ifdef SK_GAMMA_APPLY_TO_A8
        const GrDistanceFieldTextureEffect& dfTexEffect =
                effect.cast<GrDistanceFieldTextureEffect>();
        float luminance = dfTexEffect.getLuminance();
        if (luminance != fLuminance) {
            pdman.set1f(fLuminanceUni, luminance);
            fLuminance = luminance;
        }
#endif
    }

    static inline void GenKey(const GrProcessor& processor, const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldTextureEffect& dfTexEffect =
                processor.cast<GrDistanceFieldTextureEffect>();

        b->add32(dfTexEffect.getFlags());
    }

private:
    GrGLProgramDataManager::UniformHandle fTextureSizeUni;
    SkISize                               fTextureSize;
    GrGLProgramDataManager::UniformHandle fLuminanceUni;
    float                                 fLuminance;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldTextureEffect::GrDistanceFieldTextureEffect(GrTexture* texture,
                                                           const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                                           GrTexture* gamma,
                                                           const GrTextureParams& gammaParams,
                                                           float luminance,
#endif
                                                           uint32_t flags)
    : fTextureAccess(texture, params)
#ifdef SK_GAMMA_APPLY_TO_A8
    , fGammaTextureAccess(gamma, gammaParams)
    , fLuminance(luminance)
#endif
    , fFlags(flags & kNonLCD_DistanceFieldEffectMask)
    , fInTextureCoords(this->addVertexAttrib(GrShaderVar("inTextureCoords",
                                                         kVec2f_GrSLType,
                                                         GrShaderVar::kAttribute_TypeModifier))) {
    SkASSERT(!(flags & ~kNonLCD_DistanceFieldEffectMask));
    this->addTextureAccess(&fTextureAccess);
#ifdef SK_GAMMA_APPLY_TO_A8
    this->addTextureAccess(&fGammaTextureAccess);
#endif
}

bool GrDistanceFieldTextureEffect::onIsEqual(const GrProcessor& other) const {
    const GrDistanceFieldTextureEffect& cte = other.cast<GrDistanceFieldTextureEffect>();
    return fTextureAccess == cte.fTextureAccess &&
#ifdef SK_GAMMA_APPLY_TO_A8
           fGammaTextureAccess == cte.fGammaTextureAccess &&
           fLuminance == cte.fLuminance &&
#endif
           fFlags == cte.fFlags;
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

const GrBackendGeometryProcessorFactory& GrDistanceFieldTextureEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrDistanceFieldTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldTextureEffect);

GrGeometryProcessor* GrDistanceFieldTextureEffect::TestCreate(SkRandom* random,
                                                              GrContext*,
                                                              const GrDrawTargetCaps&,
                                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
#ifdef SK_GAMMA_APPLY_TO_A8
    int texIdx2 = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                       GrProcessorUnitTest::kAlphaTextureIdx;
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
                                                random->nextBool() ?
                                                    kSimilarity_DistanceFieldEffectFlag : 0);
}

///////////////////////////////////////////////////////////////////////////////

class GrGLDistanceFieldLCDTextureEffect : public GrGLGeometryProcessor {
public:
    GrGLDistanceFieldLCDTextureEffect(const GrBackendProcessorFactory& factory,
                                      const GrProcessor&)
    : INHERITED (factory)
    , fTextureSize(SkISize::Make(-1,-1))
    , fTextColor(GrColor_ILLEGAL) {}

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                geometryProcessor.cast<GrDistanceFieldLCDTextureEffect>();
        SkASSERT(1 == dfTexEffect.getVertexAttribs().count());

        SkString fsCoordName;
        const char* vsCoordName;
        const char* fsCoordNamePtr;
        builder->addVarying(kVec2f_GrSLType, "textureCoords", &vsCoordName, &fsCoordNamePtr);
        fsCoordName = fsCoordNamePtr;

        GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
        vsBuilder->codeAppendf("\t%s = %s;\n", vsCoordName, dfTexEffect.inTextureCoords().c_str());

        const char* textureSizeUniName = NULL;
        // width, height, 1/(3*width)
        fTextureSizeUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                              kVec3f_GrSLType, "TextureSize",
                                              &textureSizeUniName);

        GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();

        SkAssertResult(fsBuilder->enableFeature(
                GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));

        // create LCD offset adjusted by inverse of transform
        fsBuilder->codeAppendf("\tvec2 uv = %s;\n", fsCoordName.c_str());
        fsBuilder->codeAppendf("\tvec2 st = uv*%s.xy;\n", textureSizeUniName);
        bool isUniformScale = !!(dfTexEffect.getFlags() & kUniformScale_DistanceFieldEffectMask);
        if (isUniformScale) {
            fsBuilder->codeAppend("\tfloat dx = dFdx(st.x);\n");
            fsBuilder->codeAppendf("\tvec2 offset = vec2(dx*%s.z, 0.0);\n", textureSizeUniName);
        } else {
            fsBuilder->codeAppend("\tvec2 Jdx = dFdx(st);\n");
            fsBuilder->codeAppend("\tvec2 Jdy = dFdy(st);\n");
            fsBuilder->codeAppendf("\tvec2 offset = %s.z*Jdx;\n", textureSizeUniName);
        }

        // green is distance to uv center
        fsBuilder->codeAppend("\tvec4 texColor = ");
        fsBuilder->appendTextureLookup(samplers[0], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tvec3 distance;\n");
        fsBuilder->codeAppend("\tdistance.y = texColor.r;\n");
        // red is distance to left offset
        fsBuilder->codeAppend("\tvec2 uv_adjusted = uv - offset;\n");
        fsBuilder->codeAppend("\ttexColor = ");
        fsBuilder->appendTextureLookup(samplers[0], "uv_adjusted", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tdistance.x = texColor.r;\n");
        // blue is distance to right offset
        fsBuilder->codeAppend("\tuv_adjusted = uv + offset;\n");
        fsBuilder->codeAppend("\ttexColor = ");
        fsBuilder->appendTextureLookup(samplers[0], "uv_adjusted", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tdistance.z = texColor.r;\n");

        fsBuilder->codeAppend("\tdistance = "
            "vec3(" SK_DistanceFieldMultiplier ")*(distance - vec3(" SK_DistanceFieldThreshold"))"
            "+ vec3(" SK_DistanceFieldLCDFactor ");\n");

        // we adjust for the effect of the transformation on the distance by using
        // the length of the gradient of the texture coordinates. We use st coordinates
        // to ensure we're mapping 1:1 from texel space to pixel space.

        // To be strictly correct, we should compute the anti-aliasing factor separately
        // for each color component. However, this is only important when using perspective
        // transformations, and even then using a single factor seems like a reasonable
        // trade-off between quality and speed.
        fsBuilder->codeAppend("\tfloat afwidth;\n");
        if (isUniformScale) {
            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*dx;\n");
        } else {
            fsBuilder->codeAppend("\tvec2 uv_grad;\n");
            if (builder->ctxInfo().caps()->dropsTileOnZeroDivide()) {
                // this is to compensate for the Adreno, which likes to drop tiles on division by 0
                fsBuilder->codeAppend("\tfloat uv_len2 = dot(uv, uv);\n");
                fsBuilder->codeAppend("\tif (uv_len2 < 0.0001) {\n");
                fsBuilder->codeAppend("\t\tuv_grad = vec2(0.7071, 0.7071);\n");
                fsBuilder->codeAppend("\t} else {\n");
                fsBuilder->codeAppend("\t\tuv_grad = uv*inversesqrt(uv_len2);\n");
                fsBuilder->codeAppend("\t}\n");
            } else {
                fsBuilder->codeAppend("\tuv_grad = normalize(uv);\n");
            }
            fsBuilder->codeAppend("\tvec2 grad = vec2(uv_grad.x*Jdx.x + uv_grad.y*Jdy.x,\n");
            fsBuilder->codeAppend("\t                 uv_grad.x*Jdx.y + uv_grad.y*Jdy.y);\n");

            // this gives us a smooth step across approximately one fragment
            fsBuilder->codeAppend("\tafwidth = " SK_DistanceFieldAAFactor "*length(grad);\n");
        }

        fsBuilder->codeAppend("\tvec4 val = vec4(smoothstep(vec3(-afwidth), vec3(afwidth), distance), 1.0);\n");

        // adjust based on gamma
        const char* textColorUniName = NULL;
        // width, height, 1/(3*width)
        fTextColorUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                            kVec3f_GrSLType, "TextColor",
                                            &textColorUniName);

        fsBuilder->codeAppendf("\tuv = vec2(val.x, %s.x);\n", textColorUniName);
        fsBuilder->codeAppend("\tvec4 gammaColor = ");
        fsBuilder->appendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tval.x = gammaColor.r;\n");

        fsBuilder->codeAppendf("\tuv = vec2(val.y, %s.y);\n", textColorUniName);
        fsBuilder->codeAppend("\tgammaColor = ");
        fsBuilder->appendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tval.y = gammaColor.r;\n");

        fsBuilder->codeAppendf("\tuv = vec2(val.z, %s.z);\n", textColorUniName);
        fsBuilder->codeAppend("\tgammaColor = ");
        fsBuilder->appendTextureLookup(samplers[1], "uv", kVec2f_GrSLType);
        fsBuilder->codeAppend(";\n");
        fsBuilder->codeAppend("\tval.z = gammaColor.r;\n");

        fsBuilder->codeAppendf("\t%s = %s;\n", outputColor,
                               (GrGLSLExpr4(inputColor) * GrGLSLExpr4("val")).c_str());
    }

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrProcessor& processor) SK_OVERRIDE {
        SkASSERT(fTextureSizeUni.isValid());
        SkASSERT(fTextColorUni.isValid());

        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                processor.cast<GrDistanceFieldLCDTextureEffect>();
        GrTexture* texture = processor.texture(0);
        if (texture->width() != fTextureSize.width() ||
            texture->height() != fTextureSize.height()) {
            fTextureSize = SkISize::Make(texture->width(), texture->height());
            float delta = 1.0f/(3.0f*texture->width());
            if (dfTexEffect.getFlags() & kBGR_DistanceFieldEffectFlag) {
                delta = -delta;
            }
            pdman.set3f(fTextureSizeUni,
                        SkIntToScalar(fTextureSize.width()),
                        SkIntToScalar(fTextureSize.height()),
                        delta);
        }

        GrColor textColor = dfTexEffect.getTextColor();
        if (textColor != fTextColor) {
            static const float ONE_OVER_255 = 1.f / 255.f;
            pdman.set3f(fTextColorUni,
                        GrColorUnpackR(textColor) * ONE_OVER_255,
                        GrColorUnpackG(textColor) * ONE_OVER_255,
                        GrColorUnpackB(textColor) * ONE_OVER_255);
            fTextColor = textColor;
        }
    }

    static inline void GenKey(const GrProcessor& processor, const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrDistanceFieldLCDTextureEffect& dfTexEffect =
                processor.cast<GrDistanceFieldLCDTextureEffect>();

        b->add32(dfTexEffect.getFlags());
    }

private:
    GrGLProgramDataManager::UniformHandle fTextureSizeUni;
    SkISize                               fTextureSize;
    GrGLProgramDataManager::UniformHandle fTextColorUni;
    SkColor                               fTextColor;

    typedef GrGLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDistanceFieldLCDTextureEffect::GrDistanceFieldLCDTextureEffect(
                                                  GrTexture* texture, const GrTextureParams& params,
                                                  GrTexture* gamma, const GrTextureParams& gParams,
                                                  SkColor textColor,
                                                  uint32_t flags)
    : fTextureAccess(texture, params)
    , fGammaTextureAccess(gamma, gParams)
    , fTextColor(textColor)
    , fFlags(flags & kLCD_DistanceFieldEffectMask)
    , fInTextureCoords(this->addVertexAttrib(GrShaderVar("inTextureCoords",
                                                         kVec2f_GrSLType,
                                                         GrShaderVar::kAttribute_TypeModifier))) {
    SkASSERT(!(flags & ~kLCD_DistanceFieldEffectMask) && (flags & kUseLCD_DistanceFieldEffectFlag));
        
    this->addTextureAccess(&fTextureAccess);
    this->addTextureAccess(&fGammaTextureAccess);
}

bool GrDistanceFieldLCDTextureEffect::onIsEqual(const GrProcessor& other) const {
    const GrDistanceFieldLCDTextureEffect& cte = other.cast<GrDistanceFieldLCDTextureEffect>();
    return (fTextureAccess == cte.fTextureAccess &&
            fGammaTextureAccess == cte.fGammaTextureAccess &&
            fTextColor == cte.fTextColor &&
            fFlags == cte.fFlags);
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

const GrBackendGeometryProcessorFactory& GrDistanceFieldLCDTextureEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrDistanceFieldLCDTextureEffect>::getInstance();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrDistanceFieldLCDTextureEffect);

GrGeometryProcessor* GrDistanceFieldLCDTextureEffect::TestCreate(SkRandom* random,
                                                                 GrContext*,
                                                                 const GrDrawTargetCaps&,
                                                                 GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                      GrProcessorUnitTest::kAlphaTextureIdx;
    int texIdx2 = random->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx :
                                       GrProcessorUnitTest::kAlphaTextureIdx;
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
    uint32_t flags = kUseLCD_DistanceFieldEffectFlag;
    flags |= random->nextBool() ? kUniformScale_DistanceFieldEffectMask : 0;
    flags |= random->nextBool() ? kBGR_DistanceFieldEffectFlag : 0;
    return GrDistanceFieldLCDTextureEffect::Create(textures[texIdx], params,
                                                   textures[texIdx2], params2,
                                                   textColor,
                                                   flags);
}
