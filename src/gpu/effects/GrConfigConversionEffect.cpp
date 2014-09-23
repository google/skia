/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConfigConversionEffect.h"
#include "GrContext.h"
#include "GrTBackendProcessorFactory.h"
#include "GrSimpleTextureEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "SkMatrix.h"

class GrGLConfigConversionEffect : public GrGLFragmentProcessor {
public:
    GrGLConfigConversionEffect(const GrBackendProcessorFactory& factory,
                               const GrProcessor& processor)
    : INHERITED (factory) {
        const GrConfigConversionEffect& configConversionEffect =
                processor.cast<GrConfigConversionEffect>();
        fSwapRedAndBlue = configConversionEffect.swapsRedAndBlue();
        fPMConversion = configConversionEffect.pmConversion();
    }

    virtual void emitCode(GrGLProgramBuilder* builder,
                          const GrFragmentProcessor&,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        // Using highp for GLES here in order to avoid some precision issues on specific GPUs.
        GrGLShaderVar tmpVar("tmpColor", kVec4f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
        SkString tmpDecl;
        tmpVar.appendDecl(builder->ctxInfo(), &tmpDecl);

        GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();

        fsBuilder->codeAppendf("%s;", tmpDecl.c_str());

        fsBuilder->codeAppendf("%s = ", tmpVar.c_str());
        fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
        fsBuilder->codeAppend(";");

        if (GrConfigConversionEffect::kNone_PMConversion == fPMConversion) {
            SkASSERT(fSwapRedAndBlue);
            fsBuilder->codeAppendf("%s = %s.bgra;", outputColor, tmpVar.c_str());
        } else {
            const char* swiz = fSwapRedAndBlue ? "bgr" : "rgb";
            switch (fPMConversion) {
                case GrConfigConversionEffect::kMulByAlpha_RoundUp_PMConversion:
                    fsBuilder->codeAppendf(
                        "%s = vec4(ceil(%s.%s * %s.a * 255.0) / 255.0, %s.a);",
                        tmpVar.c_str(), tmpVar.c_str(), swiz, tmpVar.c_str(), tmpVar.c_str());
                    break;
                case GrConfigConversionEffect::kMulByAlpha_RoundDown_PMConversion:
                    // Add a compensation(0.001) here to avoid the side effect of the floor operation.
                    // In Intel GPUs, the integer value converted from floor(%s.r * 255.0) / 255.0
                    // is less than the integer value converted from  %s.r by 1 when the %s.r is
                    // converted from the integer value 2^n, such as 1, 2, 4, 8, etc.
                    fsBuilder->codeAppendf(
                        "%s = vec4(floor(%s.%s * %s.a * 255.0 + 0.001) / 255.0, %s.a);",
                        tmpVar.c_str(), tmpVar.c_str(), swiz, tmpVar.c_str(), tmpVar.c_str());
                    break;
                case GrConfigConversionEffect::kDivByAlpha_RoundUp_PMConversion:
                    fsBuilder->codeAppendf(
                        "%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(ceil(%s.%s / %s.a * 255.0) / 255.0, %s.a);",
                        tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), swiz, tmpVar.c_str(), tmpVar.c_str());
                    break;
                case GrConfigConversionEffect::kDivByAlpha_RoundDown_PMConversion:
                    fsBuilder->codeAppendf(
                        "%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(floor(%s.%s / %s.a * 255.0) / 255.0, %s.a);",
                        tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), swiz, tmpVar.c_str(), tmpVar.c_str());
                    break;
                default:
                    SkFAIL("Unknown conversion op.");
                    break;
            }
            fsBuilder->codeAppendf("%s = %s;", outputColor, tmpVar.c_str());
        }
        SkString modulate;
        GrGLSLMulVarBy4f(&modulate, 2, outputColor, inputColor);
        fsBuilder->codeAppend(modulate.c_str());
    }

    static inline void GenKey(const GrProcessor& processor, const GrGLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrConfigConversionEffect& conv = processor.cast<GrConfigConversionEffect>();
        uint32_t key = (conv.swapsRedAndBlue() ? 0 : 1) | (conv.pmConversion() << 1);
        b->add32(key);
    }

private:
    bool                                    fSwapRedAndBlue;
    GrConfigConversionEffect::PMConversion  fPMConversion;

    typedef GrGLFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

GrConfigConversionEffect::GrConfigConversionEffect(GrTexture* texture,
                                                   bool swapRedAndBlue,
                                                   PMConversion pmConversion,
                                                   const SkMatrix& matrix)
    : GrSingleTextureEffect(texture, matrix)
    , fSwapRedAndBlue(swapRedAndBlue)
    , fPMConversion(pmConversion) {
    SkASSERT(kRGBA_8888_GrPixelConfig == texture->config() ||
             kBGRA_8888_GrPixelConfig == texture->config());
    // Why did we pollute our texture cache instead of using a GrSingleTextureEffect?
    SkASSERT(swapRedAndBlue || kNone_PMConversion != pmConversion);
}

const GrBackendFragmentProcessorFactory& GrConfigConversionEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrConfigConversionEffect>::getInstance();
}

bool GrConfigConversionEffect::onIsEqual(const GrProcessor& s) const {
    const GrConfigConversionEffect& other = s.cast<GrConfigConversionEffect>();
    return this->texture(0) == s.texture(0) &&
           other.fSwapRedAndBlue == fSwapRedAndBlue &&
           other.fPMConversion == fPMConversion;
}

void GrConfigConversionEffect::getConstantColorComponents(GrColor* color,
                                                          uint32_t* validFlags) const {
    this->updateConstantColorComponentsForModulation(color, validFlags);
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConfigConversionEffect);

GrFragmentProcessor* GrConfigConversionEffect::TestCreate(SkRandom* random,
                                                          GrContext*,
                                                          const GrDrawTargetCaps&,
                                                          GrTexture* textures[]) {
    PMConversion pmConv = static_cast<PMConversion>(random->nextULessThan(kPMConversionCnt));
    bool swapRB;
    if (kNone_PMConversion == pmConv) {
        swapRB = true;
    } else {
        swapRB = random->nextBool();
    }
    return SkNEW_ARGS(GrConfigConversionEffect,
                                      (textures[GrProcessorUnitTest::kSkiaPMTextureIdx],
                                       swapRB,
                                       pmConv,
                                       GrProcessorUnitTest::TestMatrix(random)));
}

///////////////////////////////////////////////////////////////////////////////
void GrConfigConversionEffect::TestForPreservingPMConversions(GrContext* context,
                                                              PMConversion* pmToUPMRule,
                                                              PMConversion* upmToPMRule) {
    *pmToUPMRule = kNone_PMConversion;
    *upmToPMRule = kNone_PMConversion;
    SkAutoTMalloc<uint32_t> data(256 * 256 * 3);
    uint32_t* srcData = data.get();
    uint32_t* firstRead = data.get() + 256 * 256;
    uint32_t* secondRead = data.get() + 2 * 256 * 256;

    // Fill with every possible premultiplied A, color channel value. There will be 256-y duplicate
    // values in row y. We set r,g, and b to the same value since they are handled identically.
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            uint8_t* color = reinterpret_cast<uint8_t*>(&srcData[256*y + x]);
            color[3] = y;
            color[2] = SkTMin(x, y);
            color[1] = SkTMin(x, y);
            color[0] = SkTMin(x, y);
        }
    }

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit |
                  kNoStencil_GrTextureFlagBit;
    desc.fWidth = 256;
    desc.fHeight = 256;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    SkAutoTUnref<GrTexture> readTex(context->createUncachedTexture(desc, NULL, 0));
    if (!readTex.get()) {
        return;
    }
    SkAutoTUnref<GrTexture> tempTex(context->createUncachedTexture(desc, NULL, 0));
    if (!tempTex.get()) {
        return;
    }
    desc.fFlags = kNone_GrTextureFlags;
    SkAutoTUnref<GrTexture> dataTex(context->createUncachedTexture(desc, data, 0));
    if (!dataTex.get()) {
        return;
    }

    static const PMConversion kConversionRules[][2] = {
        {kDivByAlpha_RoundDown_PMConversion, kMulByAlpha_RoundUp_PMConversion},
        {kDivByAlpha_RoundUp_PMConversion, kMulByAlpha_RoundDown_PMConversion},
    };

    GrContext::AutoWideOpenIdentityDraw awoid(context, NULL);

    bool failed = true;

    for (size_t i = 0; i < SK_ARRAY_COUNT(kConversionRules) && failed; ++i) {
        *pmToUPMRule = kConversionRules[i][0];
        *upmToPMRule = kConversionRules[i][1];

        static const SkRect kDstRect = SkRect::MakeWH(SkIntToScalar(256), SkIntToScalar(256));
        static const SkRect kSrcRect = SkRect::MakeWH(SK_Scalar1, SK_Scalar1);
        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        SkAutoTUnref<GrFragmentProcessor> pmToUPM1(
                SkNEW_ARGS(GrConfigConversionEffect,
                           (dataTex, false, *pmToUPMRule, SkMatrix::I())));
        SkAutoTUnref<GrFragmentProcessor> upmToPM(
                SkNEW_ARGS(GrConfigConversionEffect,
                           (readTex, false, *upmToPMRule, SkMatrix::I())));
        SkAutoTUnref<GrFragmentProcessor> pmToUPM2(
                SkNEW_ARGS(GrConfigConversionEffect,
                           (tempTex, false, *pmToUPMRule, SkMatrix::I())));

        context->setRenderTarget(readTex->asRenderTarget());
        GrPaint paint1;
        paint1.addColorProcessor(pmToUPM1);
        context->drawRectToRect(paint1, kDstRect, kSrcRect);

        readTex->readPixels(0, 0, 256, 256, kRGBA_8888_GrPixelConfig, firstRead);

        context->setRenderTarget(tempTex->asRenderTarget());
        GrPaint paint2;
        paint2.addColorProcessor(upmToPM);
        context->drawRectToRect(paint2, kDstRect, kSrcRect);
        context->setRenderTarget(readTex->asRenderTarget());

        GrPaint paint3;
        paint3.addColorProcessor(pmToUPM2);
        context->drawRectToRect(paint3, kDstRect, kSrcRect);

        readTex->readPixels(0, 0, 256, 256, kRGBA_8888_GrPixelConfig, secondRead);

        failed = false;
        for (int y = 0; y < 256 && !failed; ++y) {
            for (int x = 0; x <= y; ++x) {
                if (firstRead[256 * y + x] != secondRead[256 * y + x]) {
                    failed = true;
                    break;
                }
            }
        }
    }
    if (failed) {
        *pmToUPMRule = kNone_PMConversion;
        *upmToPMRule = kNone_PMConversion;
    }
}

const GrFragmentProcessor* GrConfigConversionEffect::Create(GrTexture* texture,
                                                 bool swapRedAndBlue,
                                                 PMConversion pmConversion,
                                                 const SkMatrix& matrix) {
    if (!swapRedAndBlue && kNone_PMConversion == pmConversion) {
        // If we returned a GrConfigConversionEffect that was equivalent to a GrSimpleTextureEffect
        // then we may pollute our texture cache with redundant shaders. So in the case that no
        // conversions were requested we instead return a GrSimpleTextureEffect.
        return GrSimpleTextureEffect::Create(texture, matrix);
    } else {
        if (kRGBA_8888_GrPixelConfig != texture->config() &&
            kBGRA_8888_GrPixelConfig != texture->config() &&
            kNone_PMConversion != pmConversion) {
            // The PM conversions assume colors are 0..255
            return NULL;
        }
        return SkNEW_ARGS(GrConfigConversionEffect, (texture,
                                                     swapRedAndBlue,
                                                     pmConversion,
                                                     matrix));
    }
}
