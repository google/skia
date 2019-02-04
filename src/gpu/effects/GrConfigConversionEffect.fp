/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
    #include "GrClip.h"
    #include "GrContext.h"
    #include "GrContextPriv.h"
    #include "GrProxyProvider.h"
    #include "GrRenderTargetContext.h"
}

@class {
    static bool TestForPreservingPMConversions(GrContext* context) {
        static constexpr int kSize = 256;
        static constexpr GrPixelConfig kConfig = kRGBA_8888_GrPixelConfig;
        static constexpr SkColorType kColorType = kRGBA_8888_SkColorType;
        const GrBackendFormat format =
                context->priv().caps()->getBackendFormatFromColorType(kColorType);
        SkAutoTMalloc<uint32_t> data(kSize * kSize * 3);
        uint32_t* srcData = data.get();
        uint32_t* firstRead = data.get() + kSize * kSize;
        uint32_t* secondRead = data.get() + 2 * kSize * kSize;

        // Fill with every possible premultiplied A, color channel value. There will be 256-y
        // duplicate values in row y. We set r, g, and b to the same value since they are handled
        // identically.
        for (int y = 0; y < kSize; ++y) {
            for (int x = 0; x < kSize; ++x) {
                uint8_t* color = reinterpret_cast<uint8_t*>(&srcData[kSize*y + x]);
                color[3] = y;
                color[2] = SkTMin(x, y);
                color[1] = SkTMin(x, y);
                color[0] = SkTMin(x, y);
            }
        }
        memset(firstRead, 0, kSize * kSize * sizeof(uint32_t));
        memset(secondRead, 0, kSize * kSize * sizeof(uint32_t));

        const SkImageInfo ii = SkImageInfo::Make(kSize, kSize,
                                                 kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<GrRenderTargetContext> readRTC(
                context->priv().makeDeferredRenderTargetContext(format, SkBackingFit::kExact,
                                                                kSize, kSize,
                                                                kConfig, nullptr));
        sk_sp<GrRenderTargetContext> tempRTC(
                context->priv().makeDeferredRenderTargetContext(format, SkBackingFit::kExact,
                                                                kSize, kSize,
                                                                kConfig, nullptr));
        if (!readRTC || !readRTC->asTextureProxy() || !tempRTC) {
            return false;
        }
        // Adding discard to appease vulkan validation warning about loading uninitialized data on
        // draw
        readRTC->discard();

        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        SkPixmap pixmap(ii, srcData, 4 * kSize);

        // This function is only ever called if we are in a GrContext that has a GrGpu since we are
        // calling read pixels here. Thus the pixel data will be uploaded immediately and we don't
        // need to keep the pixel data alive in the proxy. Therefore the ReleaseProc is nullptr.
        sk_sp<SkImage> image = SkImage::MakeFromRaster(pixmap, nullptr, nullptr);

        sk_sp<GrTextureProxy> dataProxy = proxyProvider->createTextureProxy(std::move(image),
                                                                            kNone_GrSurfaceFlags,
                                                                            1,
                                                                            SkBudgeted::kYes,
                                                                            SkBackingFit::kExact);
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
        std::unique_ptr<GrFragmentProcessor> pmToUPM(
                new GrConfigConversionEffect(PMConversion::kToUnpremul));
        std::unique_ptr<GrFragmentProcessor> upmToPM(
                new GrConfigConversionEffect(PMConversion::kToPremul));

        paint1.addColorTextureProcessor(dataProxy, SkMatrix::I());
        paint1.addColorFragmentProcessor(pmToUPM->clone());
        paint1.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint1), GrAA::kNo, SkMatrix::I(), kRect,
                                kRect);
        if (!readRTC->readPixels(ii, firstRead, 0, 0, 0)) {
            return false;
        }

        // Adding discard to appease vulkan validation warning about loading uninitialized data on
        // draw
        tempRTC->discard();

        paint2.addColorTextureProcessor(readRTC->asTextureProxyRef(), SkMatrix::I());
        paint2.addColorFragmentProcessor(std::move(upmToPM));
        paint2.setPorterDuffXPFactory(SkBlendMode::kSrc);

        tempRTC->fillRectToRect(GrNoClip(), std::move(paint2), GrAA::kNo, SkMatrix::I(), kRect,
                                kRect);

        paint3.addColorTextureProcessor(tempRTC->asTextureProxyRef(), SkMatrix::I());
        paint3.addColorFragmentProcessor(std::move(pmToUPM));
        paint3.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(GrNoClip(), std::move(paint3), GrAA::kNo, SkMatrix::I(), kRect,
                                kRect);

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
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                     PMConversion pmConversion) {
        if (!fp) {
            return nullptr;
        }
        std::unique_ptr<GrFragmentProcessor> ccFP(new GrConfigConversionEffect(pmConversion));
        std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { std::move(fp), std::move(ccFP) };
        return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
    }
}

layout(key) in PMConversion pmConversion;

@emitCode {
    fragBuilder->forceHighPrecision();
}

void main() {
    // Aggressively round to the nearest exact (N / 255) floating point value. This lets us find a
    // round-trip preserving pair on some GPUs that do odd byte to float conversion.
    sk_OutColor = floor(sk_InColor * 255 + 0.5) / 255;

    @switch (pmConversion) {
        case PMConversion::kToPremul:
            sk_OutColor.rgb = floor(sk_OutColor.rgb * sk_OutColor.a * 255 + 0.5) / 255;
            break;

        case PMConversion::kToUnpremul:
            sk_OutColor.rgb = sk_OutColor.a <= 0.0 ?
                                          half3(0) :
                                          floor(sk_OutColor.rgb / sk_OutColor.a * 255 + 0.5) / 255;
            break;
    }
}

@test(data) {
    PMConversion pmConv = static_cast<PMConversion>(data->fRandom->nextULessThan(
                                                             (int) PMConversion::kPMConversionCnt));
    return std::unique_ptr<GrFragmentProcessor>(new GrConfigConversionEffect(pmConv));
}
