/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrConfigConversionEffect.h"
#include "../private/GrGLSL.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrSimpleTextureEffect.h"
#include "SkMatrix.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrGLConfigConversionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrConfigConversionEffect& cce = args.fFp.cast<GrConfigConversionEffect>();
        GrConfigConversionEffect::PMConversion pmConversion = cce.pmConversion();

        // Using highp for GLES here in order to avoid some precision issues on specific GPUs.
        GrShaderVar tmpVar("tmpColor", kVec4f_GrSLType, 0, kHigh_GrSLPrecision);
        SkString tmpDecl;
        tmpVar.appendDecl(args.fShaderCaps, &tmpDecl);

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        fragBuilder->codeAppendf("%s;", tmpDecl.c_str());

        fragBuilder->codeAppendf("%s = ", tmpVar.c_str());
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], args.fTransformedCoords[0].c_str(),
                                         args.fTransformedCoords[0].getType());
        fragBuilder->codeAppend(";");

        switch (pmConversion) {
            case GrConfigConversionEffect::kMulByAlpha_RoundUp_PMConversion:
                fragBuilder->codeAppendf(
                    "%s = vec4(ceil(%s.rgb * %s.a * 255.0) / 255.0, %s.a);",
                    tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str());
                break;
            case GrConfigConversionEffect::kMulByAlpha_RoundDown_PMConversion:
                // Add a compensation(0.001) here to avoid the side effect of the floor operation.
                // In Intel GPUs, the integer value converted from floor(%s.r * 255.0) / 255.0
                // is less than the integer value converted from  %s.r by 1 when the %s.r is
                // converted from the integer value 2^n, such as 1, 2, 4, 8, etc.
                fragBuilder->codeAppendf(
                    "%s = vec4(floor(%s.rgb * %s.a * 255.0 + 0.001) / 255.0, %s.a);",
                    tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str());

                break;
            case GrConfigConversionEffect::kDivByAlpha_RoundUp_PMConversion:
                fragBuilder->codeAppendf(
                    "%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(ceil(%s.rgb / %s.a * 255.0) / 255.0, %s.a);",
                    tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(),
                    tmpVar.c_str());
                break;
            case GrConfigConversionEffect::kDivByAlpha_RoundDown_PMConversion:
                fragBuilder->codeAppendf(
                    "%s = %s.a <= 0.0 ? vec4(0,0,0,0) : vec4(floor(%s.rgb / %s.a * 255.0) / 255.0, %s.a);",
                    tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(), tmpVar.c_str(),
                    tmpVar.c_str());
                break;
            default:
                SkFAIL("Unknown conversion op.");
                break;
        }
        fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, tmpVar.c_str());

        SkString modulate;
        GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
        fragBuilder->codeAppend(modulate.c_str());
    }

    static inline void GenKey(const GrProcessor& processor, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrConfigConversionEffect& cce = processor.cast<GrConfigConversionEffect>();
        uint32_t key = cce.pmConversion();
        b->add32(key);
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////
GrConfigConversionEffect::GrConfigConversionEffect(GrResourceProvider* resourceProvider,
                                                   sk_sp<GrTextureProxy> proxy,
                                                   PMConversion pmConversion,
                                                   const SkMatrix& matrix)
        : INHERITED(resourceProvider, kNone_OptimizationFlags, proxy, nullptr, matrix)
        , fPMConversion(pmConversion) {
    this->initClassID<GrConfigConversionEffect>();
    // We expect to get here with non-BGRA/RGBA only if we're doing not doing a premul/unpremul
    // conversion.
    SkASSERT(kRGBA_8888_GrPixelConfig == proxy->config() ||
             kBGRA_8888_GrPixelConfig == proxy->config());
}

bool GrConfigConversionEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrConfigConversionEffect& other = s.cast<GrConfigConversionEffect>();
    return other.fPMConversion == fPMConversion;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrConfigConversionEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrConfigConversionEffect::TestCreate(GrProcessorTestData* d) {
    PMConversion pmConv = static_cast<PMConversion>(d->fRandom->nextULessThan(kPMConversionCnt));
    return sk_sp<GrFragmentProcessor>(new GrConfigConversionEffect(
                            d->resourceProvider(),
                            d->textureProxy(GrProcessorUnitTest::kSkiaPMTextureIdx),
                            pmConv, GrTest::TestMatrix(d->fRandom)));
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrConfigConversionEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLConfigConversionEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrConfigConversionEffect::onCreateGLSLInstance() const {
    return new GrGLConfigConversionEffect();
}



void GrConfigConversionEffect::TestForPreservingPMConversions(GrContext* context,
                                                              PMConversion* pmToUPMRule,
                                                              PMConversion* upmToPMRule) {
    *pmToUPMRule = kPMConversionCnt;
    *upmToPMRule = kPMConversionCnt;
    static constexpr int kSize = 256;
    static constexpr GrPixelConfig kConfig = kRGBA_8888_GrPixelConfig;
    SkAutoTMalloc<uint32_t> data(kSize * kSize * 3);
    uint32_t* srcData = data.get();
    uint32_t* firstRead = data.get() + kSize * kSize;
    uint32_t* secondRead = data.get() + 2 * kSize * kSize;

    // Fill with every possible premultiplied A, color channel value. There will be 256-y duplicate
    // values in row y. We set r,g, and b to the same value since they are handled identically.
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            uint8_t* color = reinterpret_cast<uint8_t*>(&srcData[kSize*y + x]);
            color[3] = y;
            color[2] = SkTMin(x, y);
            color[1] = SkTMin(x, y);
            color[0] = SkTMin(x, y);
        }
    }

    const SkImageInfo ii = SkImageInfo::Make(kSize, kSize,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<GrRenderTargetContext> readRTC(context->makeRenderTargetContext(SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    sk_sp<GrRenderTargetContext> tempRTC(context->makeRenderTargetContext(SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    if (!readRTC || !tempRTC) {
        return;
    }
    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kConfig;

    GrResourceProvider* resourceProvider = context->resourceProvider();
    sk_sp<GrTextureProxy> dataProxy = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                                   SkBudgeted::kYes, data, 0);
    if (!dataProxy) {
        return;
    }

    static const PMConversion kConversionRules[][2] = {
        {kDivByAlpha_RoundDown_PMConversion, kMulByAlpha_RoundUp_PMConversion},
        {kDivByAlpha_RoundUp_PMConversion, kMulByAlpha_RoundDown_PMConversion},
    };

    bool failed = true;

    for (size_t i = 0; i < SK_ARRAY_COUNT(kConversionRules) && failed; ++i) {
        *pmToUPMRule = kConversionRules[i][0];
        *upmToPMRule = kConversionRules[i][1];

        static const SkRect kDstRect = SkRect::MakeIWH(kSize, kSize);
        static const SkRect kSrcRect = SkRect::MakeIWH(kSize, kSize);
        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        if (!readRTC->asTextureProxy()) {
            continue;
        }
        GrPaint paint1;
        GrPaint paint2;
        GrPaint paint3;
        sk_sp<GrFragmentProcessor> pmToUPM1(new GrConfigConversionEffect(
                resourceProvider, dataProxy, *pmToUPMRule, SkMatrix::I()));
        sk_sp<GrFragmentProcessor> upmToPM(new GrConfigConversionEffect(
                resourceProvider, readRTC->asTextureProxyRef(), *upmToPMRule, SkMatrix::I()));
        sk_sp<GrFragmentProcessor> pmToUPM2(new GrConfigConversionEffect(
                resourceProvider, tempRTC->asTextureProxyRef(), *pmToUPMRule, SkMatrix::I()));

        paint1.addColorFragmentProcessor(std::move(pmToUPM1));
        paint1.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint1), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        if (!readRTC->readPixels(ii, firstRead, 0, 0, 0)) {
            continue;
        }

        paint2.addColorFragmentProcessor(std::move(upmToPM));
        paint2.setPorterDuffXPFactory(SkBlendMode::kSrc);

        tempRTC->fillRectToRect(GrNoClip(), std::move(paint2), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        paint3.addColorFragmentProcessor(std::move(pmToUPM2));
        paint3.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint3), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        if (!readRTC->readPixels(ii, secondRead, 0, 0, 0)) {
            continue;
        }

        failed = false;
        for (int y = 0; y < kSize && !failed; ++y) {
            for (int x = 0; x <= y; ++x) {
                if (firstRead[kSize * y + x] != secondRead[kSize * y + x]) {
                    failed = true;
                    break;
                }
            }
        }
    }
    if (failed) {
        *pmToUPMRule = kPMConversionCnt;
        *upmToPMRule = kPMConversionCnt;
    }
}

sk_sp<GrFragmentProcessor> GrConfigConversionEffect::Make(GrResourceProvider* resourceProvider,
                                                          sk_sp<GrTextureProxy> proxy,
                                                          PMConversion pmConversion,
                                                          const SkMatrix& matrix) {
    if (kRGBA_8888_GrPixelConfig != proxy->config() &&
        kBGRA_8888_GrPixelConfig != proxy->config()) {
        // The PM conversions assume colors are 0..255
        return nullptr;
    }
    return sk_sp<GrFragmentProcessor>(new GrConfigConversionEffect(resourceProvider,
                                                                   std::move(proxy),
                                                                   pmConversion, matrix));
}
