/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConfigConversionEffect.h"
#include "GrContext.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLEffectMatrix.h"
#include "SkMatrix.h"

class GrGLConfigConversionEffect : public GrGLEffect {
public:
    GrGLConfigConversionEffect(const GrBackendEffectFactory& factory,
                               const GrEffect& s) : INHERITED (factory) {
        const GrConfigConversionEffect& effect = static_cast<const GrConfigConversionEffect&>(s);
        fSwapRedAndBlue = effect.swapsRedAndBlue();
        fPMConversion = effect.pmConversion();
    }

    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrEffectStage&,
                          EffectKey key,
                          const char* vertexCoords,
                          const char* outputColor,
                          const char* inputColor,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        const char* coords;
        GrSLType coordsType = fEffectMatrix.emitCode(builder, key, vertexCoords, &coords);
        builder->fFSCode.appendf("\t\t%s = ", outputColor);
        builder->appendTextureLookup(&builder->fFSCode, samplers[0], coords, coordsType);
        builder->fFSCode.append(";\n");
        if (GrConfigConversionEffect::kNone_PMConversion == fPMConversion) {
            GrAssert(fSwapRedAndBlue);
            builder->fFSCode.appendf("\t%s = %s.bgra;\n", outputColor, outputColor);
        } else {
            const char* swiz = fSwapRedAndBlue ? "bgr" : "rgb";
            switch (fPMConversion) {
                case GrConfigConversionEffect::kMulByAlpha_RoundUp_PMConversion:
                    builder->fFSCode.appendf(
                        "\t\t%s = vec4(ceil(%s.%s * %s.a * 255.0) / 255.0, %s.a);\n",
                        outputColor, outputColor, swiz, outputColor, outputColor);
                    break;
                case GrConfigConversionEffect::kMulByAlpha_RoundDown_PMConversion:
                    builder->fFSCode.appendf(
                        "\t\t%s = vec4(floor(%s.%s * %s.a * 255.0) / 255.0, %s.a);\n",
                        outputColor, outputColor, swiz, outputColor, outputColor);
                    break;
                case GrConfigConversionEffect::kDivByAlpha_RoundUp_PMConversion:
                    builder->fFSCode.appendf("\t\t%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(ceil(%s.%s / %s.a * 255.0) / 255.0, %s.a);\n",
                        outputColor, outputColor, outputColor, swiz, outputColor, outputColor);
                    break;
                case GrConfigConversionEffect::kDivByAlpha_RoundDown_PMConversion:
                    builder->fFSCode.appendf("\t\t%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(floor(%s.%s / %s.a * 255.0) / 255.0, %s.a);\n",
                        outputColor, outputColor, outputColor, swiz, outputColor, outputColor);
                    break;
                default:
                    GrCrash("Unknown conversion op.");
                    break;
            }
        }
        GrGLSLMulVarBy4f(&builder->fFSCode, 2, outputColor, inputColor);
    }

    void setData(const GrGLUniformManager& uman, const GrEffectStage& stage) {
        const GrConfigConversionEffect& effect =
            static_cast<const GrConfigConversionEffect&>(*stage.getEffect());
        fEffectMatrix.setData(uman,
                              effect.getMatrix(),
                              stage.getCoordChangeMatrix(),
                              effect.texture(0));
    }

    static inline EffectKey GenKey(const GrEffectStage& s, const GrGLCaps&) {
        const GrConfigConversionEffect& effect =
            static_cast<const GrConfigConversionEffect&>(*s.getEffect());
        EffectKey key = static_cast<EffectKey>(effect.swapsRedAndBlue()) |
                        (effect.pmConversion() << 1);
        key <<= GrGLEffectMatrix::kKeyBits;
        EffectKey matrixKey =  GrGLEffectMatrix::GenKey(effect.getMatrix(),
                                                        s.getCoordChangeMatrix(),
                                                        effect.texture(0));
        GrAssert(!(matrixKey & key));
        return matrixKey | key;
    }

private:
    bool                                    fSwapRedAndBlue;
    GrConfigConversionEffect::PMConversion  fPMConversion;
    GrGLEffectMatrix                        fEffectMatrix;

    typedef GrGLEffect INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

GrConfigConversionEffect::GrConfigConversionEffect(GrTexture* texture,
                                                   bool swapRedAndBlue,
                                                   PMConversion pmConversion,
                                                   const SkMatrix& matrix)
    : GrSingleTextureEffect(texture, matrix)
    , fSwapRedAndBlue(swapRedAndBlue)
    , fPMConversion(pmConversion) {
    GrAssert(kRGBA_8888_GrPixelConfig == texture->config() ||
             kBGRA_8888_GrPixelConfig == texture->config());
    // Why did we pollute our texture cache instead of using a GrSingleTextureEffect?
    GrAssert(swapRedAndBlue || kNone_PMConversion != pmConversion);
}

const GrBackendEffectFactory& GrConfigConversionEffect::getFactory() const {
    return GrTBackendEffectFactory<GrConfigConversionEffect>::getInstance();
}

bool GrConfigConversionEffect::isEqual(const GrEffect& s) const {
    const GrConfigConversionEffect& other = static_cast<const GrConfigConversionEffect&>(s);
    return other.fSwapRedAndBlue == fSwapRedAndBlue && other.fPMConversion == fPMConversion;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrConfigConversionEffect);

GrEffect* GrConfigConversionEffect::TestCreate(SkRandom* random,
                                               GrContext* context,
                                               GrTexture* textures[]) {
    PMConversion pmConv = static_cast<PMConversion>(random->nextULessThan(kPMConversionCnt));
    bool swapRB;
    if (kNone_PMConversion == pmConv) {
        swapRB = true;
    } else {
        swapRB = random->nextBool();
    }
    return SkNEW_ARGS(GrConfigConversionEffect, (textures[GrEffectUnitTest::kSkiaPMTextureIdx],
                                                 swapRB,
                                                 pmConv,
                                                 GrEffectUnitTest::TestMatrix(random)));
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
            color[2] = GrMin(x, y);
            color[1] = GrMin(x, y);
            color[0] = GrMin(x, y);
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

    for (size_t i = 0; i < GR_ARRAY_COUNT(kConversionRules) && failed; ++i) {
        *pmToUPMRule = kConversionRules[i][0];
        *upmToPMRule = kConversionRules[i][1];

        static const GrRect kDstRect = GrRect::MakeWH(SkIntToScalar(256), SkIntToScalar(256));
        static const GrRect kSrcRect = GrRect::MakeWH(SK_Scalar1, SK_Scalar1);
        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        GrPaint paint;
        SkAutoTUnref<GrEffect> pmToUPMEffect1(SkNEW_ARGS(GrConfigConversionEffect,
                                                        (dataTex,
                                                         false,
                                                         *pmToUPMRule,
                                                         SkMatrix::I())));
        SkAutoTUnref<GrEffect> upmToPMEffect(SkNEW_ARGS(GrConfigConversionEffect,
                                                       (readTex,
                                                        false,
                                                        *upmToPMRule,
                                                        SkMatrix::I())));
        SkAutoTUnref<GrEffect> pmToUPMEffect2(SkNEW_ARGS(GrConfigConversionEffect,
                                                        (tempTex,
                                                         false,
                                                         *pmToUPMRule,
                                                         SkMatrix::I())));

        context->setRenderTarget(readTex->asRenderTarget());
        paint.colorStage(0)->setEffect(pmToUPMEffect1);
        context->drawRectToRect(paint, kDstRect, kSrcRect);

        readTex->readPixels(0, 0, 256, 256, kRGBA_8888_GrPixelConfig, firstRead);

        context->setRenderTarget(tempTex->asRenderTarget());
        paint.colorStage(0)->setEffect(upmToPMEffect);
        context->drawRectToRect(paint, kDstRect, kSrcRect);
        context->setRenderTarget(readTex->asRenderTarget());
        paint.colorStage(0)->setEffect(pmToUPMEffect2);
        context->drawRectToRect(paint, kDstRect, kSrcRect);

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

bool GrConfigConversionEffect::InstallEffect(GrTexture* texture,
                                             bool swapRedAndBlue,
                                             PMConversion pmConversion,
                                             const SkMatrix& matrix,
                                             GrEffectStage* stage) {
    if (!swapRedAndBlue && kNone_PMConversion == pmConversion) {
        // If we returned a GrConfigConversionEffect that was equivalent to a GrSingleTextureEffect
        // then we may pollute our texture cache with redundant shaders. So in the case that no
        // conversions were requested we instead return a GrSingleTextureEffect.
        stage->setEffect(SkNEW_ARGS(GrSingleTextureEffect, (texture, matrix)))->unref();
        return true;
    } else {
        if (kRGBA_8888_GrPixelConfig != texture->config() &&
            kBGRA_8888_GrPixelConfig != texture->config() &&
            kNone_PMConversion != pmConversion) {
            // The PM conversions assume colors are 0..255
            return false;
        }
        stage->setEffect(SkNEW_ARGS(GrConfigConversionEffect, (texture,
                                                               swapRedAndBlue,
                                                               pmConversion, matrix)))->unref();
        return true;
    }
}
