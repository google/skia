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
        fragBuilder->codeAppendf("vec4 color = floor(%s * 255.0 + 0.5) / 255.0;", args.fInputColor);

        switch (cce.pmConversion()) {
            case GrConfigConversionEffect::kToPremul_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = floor(color.rgb * color.a * 255.0 + 0.5) / 255.0;");
                break;

            case GrConfigConversionEffect::kToUnpremul_PMConversion:
                fragBuilder->codeAppend(
                    "color.rgb = color.a <= 0.0 ? vec3(0,0,0) : floor(color.rgb / color.a * 255.0 + 0.5) / 255.0;");
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


bool GrConfigConversionEffect::TestForPreservingPMConversions(GrContext* context) {
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

    sk_sp<GrRenderTargetContext> readRTC(context->makeDeferredRenderTargetContext(
                                                                          SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    sk_sp<GrRenderTargetContext> tempRTC(context->makeDeferredRenderTargetContext(
                                                                          SkBackingFit::kExact,
                                                                          kSize, kSize,
                                                                          kConfig, nullptr));
    if (!readRTC || !readRTC->asTextureProxy() || !tempRTC) {
        return false;
    }
    GrSurfaceDesc desc;
    desc.fWidth = kSize;
    desc.fHeight = kSize;
    desc.fConfig = kConfig;

    GrResourceProvider* resourceProvider = context->resourceProvider();
    sk_sp<GrTextureProxy> dataProxy = GrSurfaceProxy::MakeDeferred(resourceProvider, desc,
                                                                   SkBudgeted::kYes, data, 0);
    if (!dataProxy) {
        return false;
    }

    static const SkRect kRect = SkRect::MakeIWH(kSize, kSize);

    // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
    // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
    // We then verify that two reads produced the same values.

    GrPaint paint1;
    GrPaint paint2;
    GrPaint paint3;
    sk_sp<GrFragmentProcessor> pmToUPM(new GrConfigConversionEffect(kToUnpremul_PMConversion));
    sk_sp<GrFragmentProcessor> upmToPM(new GrConfigConversionEffect(kToPremul_PMConversion));

    paint1.addColorTextureProcessor(resourceProvider, dataProxy, nullptr, SkMatrix::I());
    paint1.addColorFragmentProcessor(pmToUPM);
    paint1.setPorterDuffXPFactory(SkBlendMode::kSrc);

    readRTC->fillRectToRect(GrNoClip(), std::move(paint1), GrAA::kNo, SkMatrix::I(), kRect, kRect);
    if (!readRTC->readPixels(ii, firstRead, 0, 0, 0)) {
        return false;
    }

    paint2.addColorTextureProcessor(resourceProvider, readRTC->asTextureProxyRef(), nullptr,
                                    SkMatrix::I());
    paint2.addColorFragmentProcessor(std::move(upmToPM));
    paint2.setPorterDuffXPFactory(SkBlendMode::kSrc);

    tempRTC->fillRectToRect(GrNoClip(), std::move(paint2), GrAA::kNo, SkMatrix::I(), kRect, kRect);

    paint3.addColorTextureProcessor(resourceProvider, tempRTC->asTextureProxyRef(), nullptr,
                                    SkMatrix::I());
    paint3.addColorFragmentProcessor(std::move(pmToUPM));
    paint3.setPorterDuffXPFactory(SkBlendMode::kSrc);

    readRTC->fillRectToRect(GrNoClip(), std::move(paint3), GrAA::kNo, SkMatrix::I(), kRect, kRect);

    if (!readRTC->readPixels(ii, secondRead, 0, 0, 0)) {
        return false;
    }

    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x <= y; ++x) {
            if (firstRead[kSize * y + x] != secondRead[kSize * y + x]) {
                return false;
            }
        }
    }

    return true;
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
