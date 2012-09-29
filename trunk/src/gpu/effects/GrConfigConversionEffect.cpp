/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConfigConversionEffect.h"
#include "gl/GrGLProgramStage.h"

class GrGLConfigConversionEffect : public GrGLProgramStage {
public:
    GrGLConfigConversionEffect(const GrProgramStageFactory& factory,
                               const GrCustomStage& s) : INHERITED (factory) {
        const GrConfigConversionEffect& stage = static_cast<const GrConfigConversionEffect&>(s);
        fSwapRedAndBlue = stage.swapsRedAndBlue();
        fPMConversion = stage.pmConversion();
    }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray& samplers) SK_OVERRIDE {
        builder->fFSCode.appendf("\t\t%s = ", outputColor);
        builder->appendTextureLookup(&builder->fFSCode, samplers[0]);
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

    static inline StageKey GenKey(const GrCustomStage& s, const GrGLCaps&) {
        const GrConfigConversionEffect& stage = static_cast<const GrConfigConversionEffect&>(s);
        return static_cast<int>(stage.swapsRedAndBlue()) | (stage.pmConversion() << 1);
    }

private:
    bool                                    fSwapRedAndBlue;
    GrConfigConversionEffect::PMConversion  fPMConversion;

    typedef GrGLProgramStage INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

GrConfigConversionEffect::GrConfigConversionEffect(GrTexture* texture,
                                                   bool swapRedAndBlue,
                                                   PMConversion pmConversion)
    : GrSingleTextureEffect(texture)
    , fSwapRedAndBlue(swapRedAndBlue)
    , fPMConversion(pmConversion) {
    GrAssert(kRGBA_8888_GrPixelConfig == texture->config() ||
             kBGRA_8888_GrPixelConfig == texture->config());
    // Why did we pollute our texture cache instead of using a GrSingleTextureEffect?
    GrAssert(swapRedAndBlue || kNone_PMConversion != pmConversion);
}

const GrProgramStageFactory& GrConfigConversionEffect::getFactory() const {
    return GrTProgramStageFactory<GrConfigConversionEffect>::getInstance();
}

bool GrConfigConversionEffect::isEqual(const GrCustomStage& s) const {
    const GrConfigConversionEffect& other = static_cast<const GrConfigConversionEffect&>(s);
    return other.fSwapRedAndBlue == fSwapRedAndBlue && other.fPMConversion == fPMConversion;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrConfigConversionEffect);

GrCustomStage* GrConfigConversionEffect::TestCreate(SkRandom* random,
                                                    GrContext* context,
                                                    GrTexture* textures[]) {
    PMConversion pmConv = static_cast<PMConversion>(random->nextULessThan(kPMConversionCnt));
    bool swapRB;
    if (kNone_PMConversion == pmConv) {
        swapRB = true;
    } else {
        swapRB = random->nextBool();
    }
    return SkNEW_ARGS(GrConfigConversionEffect,
            (textures[GrCustomStageUnitTest::kSkiaPMTextureIdx], swapRB, pmConv));
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

        static const GrRect kDstRect = GrRect::MakeWH(GrIntToScalar(256), GrIntToScalar(256));
        static const GrRect kSrcRect = GrRect::MakeWH(GR_Scalar1, GR_Scalar1);
        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        GrPaint paint;
        paint.reset();

        SkAutoTUnref<GrCustomStage> pmToUPMStage1(SkNEW_ARGS(GrConfigConversionEffect,
                                                             (dataTex, false, *pmToUPMRule)));
        SkAutoTUnref<GrCustomStage> upmToPMStage(SkNEW_ARGS(GrConfigConversionEffect,
                                                            (readTex, false, *upmToPMRule)));
        SkAutoTUnref<GrCustomStage> pmToUPMStage2(SkNEW_ARGS(GrConfigConversionEffect,
                                                             (tempTex, false, *pmToUPMRule)));

        context->setRenderTarget(readTex->asRenderTarget());
        paint.textureSampler(0)->setCustomStage(pmToUPMStage1);
        context->drawRectToRect(paint, kDstRect, kSrcRect);

        readTex->readPixels(0, 0, 256, 256, kRGBA_8888_GrPixelConfig, firstRead);

        context->setRenderTarget(tempTex->asRenderTarget());
        paint.textureSampler(0)->setCustomStage(upmToPMStage);
        context->drawRectToRect(paint, kDstRect, kSrcRect);
        context->setRenderTarget(readTex->asRenderTarget());
        paint.textureSampler(0)->setCustomStage(pmToUPMStage2);
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

GrCustomStage* GrConfigConversionEffect::Create(GrTexture* texture,
                                                bool swapRedAndBlue,
                                                PMConversion pmConversion) {
    if (!swapRedAndBlue && kNone_PMConversion == pmConversion) {
        // If we returned a GrConfigConversionEffect that was equivalent to a GrSingleTextureEffect
        // then we may pollute our texture cache with redundant shaders. So in the case that no
        // conversions were requested we instead return a GrSingleTextureEffect.
        return SkNEW_ARGS(GrSingleTextureEffect, (texture));
    } else {
        if (kRGBA_8888_GrPixelConfig != texture->config() &&
            kBGRA_8888_GrPixelConfig != texture->config() &&
            kNone_PMConversion != pmConversion) {
            // The PM conversions assume colors are 0..255
            return NULL;
        }
        return SkNEW_ARGS(GrConfigConversionEffect, (texture, swapRedAndBlue, pmConversion));
    }
}
