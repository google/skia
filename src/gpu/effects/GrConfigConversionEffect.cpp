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
#include "SkMatrix.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrGLConfigConversionEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrConfigConversionEffect& cce = args.fFp.cast<GrConfigConversionEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // Use highp throughout the shader to avoid some precision issues on specific GPUs.
        fragBuilder->elevateDefaultPrecision(kHigh_GrSLPrecision);

        if (nullptr == args.fInputColor) {
            // could optimize this case, but we aren't for now.
            args.fInputColor = "vec4(1)";
        }

        // Aggressively round to the nearest exact (N / 255) floating point value. This lets us
        // find a round-trip preserving pair on some GPUs that do odd byte to float conversion.
        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
//        fragBuilder->codeAppend("color = floor(color * 255.0 + 0.5) / 255.0;");

        switch (cce.pmConversion()) {
            case GrConfigConversionEffect::kMulByAlpha_RoundUp_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = ceil(color.rgb * color.a * 255.0) / 255.0;");
                break;
            case GrConfigConversionEffect::kMulByAlpha_RoundDown_PMConversion:
                // Add a compensation(0.001) here to avoid the side effect of the floor operation.
                // In Intel GPUs, the integer value converted from floor(%s.r * 255.0) / 255.0
                // is less than the integer value converted from  %s.r by 1 when the %s.r is
                // converted from the integer value 2^n, such as 1, 2, 4, 8, etc.
                fragBuilder->codeAppend(
                    "color.rgb = floor(color.rgb * color.a * 255.0 + 0.001) / 255.0;");
                break;
            case GrConfigConversionEffect::kMulByAlpha_RoundNearest_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = floor(color.rgb * color.a * 255.0 + 0.5) / 255.0;");
                break;

            case GrConfigConversionEffect::kDivByAlpha_RoundUp_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = color.a <= 0.0 ? vec3(0,0,0) : ceil(color.rgb / color.a * 255.0) / 255.0;");
                break;
            case GrConfigConversionEffect::kDivByAlpha_RoundDown_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = color.a <= 0.0 ? vec3(0,0,0) : floor(color.rgb / color.a * 255.0) / 255.0;");
                break;
            case GrConfigConversionEffect::kDivByAlpha_RoundNearest_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = color.a <= 0.0 ? vec3(0,0,0) : floor(color.rgb / color.a * 255.0 + 0.5) / 255.0;");
                break;

            case GrConfigConversionEffect::kRemainder:
                fragBuilder->codeAppend(
                    "vec4 enc = vec4(1.0, 255.0, 65025.0, 160681375.0) * color.r;"
                    "enc = fract(enc);"
                    "enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);"
                    "color = enc;");
                break;

            default:
                SkFAIL("Unknown conversion op.");
                break;
        }
        fragBuilder->codeAppendf("%s = color;", args.fOutputColor);
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

GrConfigConversionEffect::GrConfigConversionEffect(PMConversion pmConversion)
        : INHERITED(kNone_OptimizationFlags)
        , fPMConversion(pmConversion) {
    this->initClassID<GrConfigConversionEffect>();
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
    return sk_sp<GrFragmentProcessor>(new GrConfigConversionEffect(pmConv));
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
    if (context->caps()->isConfigRenderable(kRGBA_half_GrPixelConfig, false)) {
        SkDebugf("Half support\n");
    } else {
        SkDebugf("No half support\n");
    }

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

    SkAutoTMalloc<uint32_t> testData(kSize * 2);
    uint32_t* srcTestData = testData.get();
    uint32_t* testRead = testData.get() + kSize;
    for (int x = 0; x < kSize; ++x) {
        uint8_t* color = reinterpret_cast<uint8_t*>(&srcTestData[x]);
        color[3] =
        color[2] =
        color[1] =
        color[0] = x;
    }

    const SkImageInfo ii = SkImageInfo::Make(kSize, kSize,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    const SkImageInfo testInfo = SkImageInfo::Make(kSize, 1,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<GrRenderTargetContext> readRTC(context->makeRenderTargetContext(SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    sk_sp<GrRenderTargetContext> tempRTC(context->makeRenderTargetContext(SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    sk_sp<GrRenderTargetContext> testRTC(context->makeRenderTargetContext(SkBackingFit::kExact,
                                                                          kSize, 1,
                                                                          kConfig, nullptr));
    if (!readRTC || !tempRTC) {
        return;
    }
    if (!testRTC) {
        SkDebugf("Failed to create RTC\n");
        return;
    }
    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kConfig;
    GrSurfaceDesc testDesc;
    testDesc.fWidth = kSize;
    testDesc.fHeight = 1;
    testDesc.fConfig = kConfig;

    GrResourceProvider* resourceProvider = context->resourceProvider();
    sk_sp<GrTextureProxy> dataProxy = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                                   SkBudgeted::kYes, data, 0);
    if (!dataProxy) {
        return;
    }

    sk_sp<GrTextureProxy> testDataProxy = GrSurfaceProxy::MakeDeferred(resourceProvider, testDesc,
                                                                   SkBudgeted::kYes, testData, 0);
    if (!testDataProxy) {
        SkDebugf("Failed to create test data proxy\n");
        return;
    }

    if (true) {
        static const SkRect kDstRect = SkRect::MakeIWH(kSize, 1);
        static const SkRect kSrcRect = SkRect::MakeIWH(kSize, 1);
        GrPaint paint;
        sk_sp<GrFragmentProcessor> fp(new GrConfigConversionEffect(kRemainder));

        paint.addColorTextureProcessor(resourceProvider, testDataProxy, nullptr, SkMatrix::I());
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        testRTC->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);
        if (!testRTC->readPixels(testInfo, testRead, 0, 0, 0)) {
            SkDebugf("Failed to read\n");
            return;
        }
        for (int x = 0; x < kSize; ++x) {
            uint8_t* color = reinterpret_cast<uint8_t*>(&testRead[x]);
            float r = color[0] / 255.0f;
            float g = color[1] / 255.0f;
            float b = color[2] / 255.0f;
            float a = color[3] / 255.0f;
            float c = r + g / 255.0f + b / 65025.0f + a / 160581375.0f;
            SkDebugf("%d : %f\n", x, c);
        }
        _exit(0);
    }


    static const PMConversion kConversionRules[][2] = {
        {kDivByAlpha_RoundNearest_PMConversion, kMulByAlpha_RoundNearest_PMConversion},
        {kDivByAlpha_RoundDown_PMConversion, kMulByAlpha_RoundUp_PMConversion},
        {kDivByAlpha_RoundUp_PMConversion, kMulByAlpha_RoundDown_PMConversion},
    };

    uint32_t bestFailCount = 0xFFFFFFFF;
    size_t bestRule = 0;

    for (size_t i = 0; i < SK_ARRAY_COUNT(kConversionRules) && bestFailCount; ++i) {
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
        sk_sp<GrFragmentProcessor> pmToUPM(new GrConfigConversionEffect(*pmToUPMRule));
        sk_sp<GrFragmentProcessor> upmToPM(new GrConfigConversionEffect(*upmToPMRule));

        paint1.addColorTextureProcessor(resourceProvider, dataProxy, nullptr, SkMatrix::I());
        paint1.addColorFragmentProcessor(pmToUPM);
        paint1.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint1), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        if (!readRTC->readPixels(ii, firstRead, 0, 0, 0)) {
            continue;
        }

        paint2.addColorTextureProcessor(resourceProvider, readRTC->asTextureProxyRef(), nullptr,
                                        SkMatrix::I());
        paint2.addColorFragmentProcessor(std::move(upmToPM));
        paint2.setPorterDuffXPFactory(SkBlendMode::kSrc);

        tempRTC->fillRectToRect(GrNoClip(), std::move(paint2), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        paint3.addColorTextureProcessor(resourceProvider, tempRTC->asTextureProxyRef(), nullptr,
                                        SkMatrix::I());
        paint3.addColorFragmentProcessor(std::move(pmToUPM));
        paint3.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint3), GrAA::kNo, SkMatrix::I(), kDstRect,
                                kSrcRect);

        if (!readRTC->readPixels(ii, secondRead, 0, 0, 0)) {
            continue;
        }

        uint32_t failCount = 0;
        for (int y = 0; y < kSize; ++y) {
            for (int x = 0; x <= y; ++x) {
                if (firstRead[kSize * y + x] != secondRead[kSize * y + x]) {
                    if (++failCount >= bestFailCount) {
                        //break;
                    }
                }
            }
        }
        SkDebugf("Rule: %d FailCount: %d\n", i, failCount);
        if (failCount < bestFailCount) {
            bestFailCount = failCount;
            bestRule = i;
        }
    }
    *pmToUPMRule = kConversionRules[bestRule][0];
    *upmToPMRule = kConversionRules[bestRule][1];
}

sk_sp<GrFragmentProcessor> GrConfigConversionEffect::Make(sk_sp<GrFragmentProcessor> fp,
                                                          PMConversion pmConversion) {
    if (!fp) {
        return nullptr;
    }
    sk_sp<GrFragmentProcessor> ccFP(new GrConfigConversionEffect(pmConversion));
    sk_sp<GrFragmentProcessor> fpPipeline[] = { fp, ccFP };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}
