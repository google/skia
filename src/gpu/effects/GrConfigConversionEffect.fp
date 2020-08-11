/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;

@header {
    #include "include/gpu/GrDirectContext.h"
    #include "src/gpu/GrBitmapTextureMaker.h"
    #include "src/gpu/GrContextPriv.h"
    #include "src/gpu/GrImageInfo.h"
    #include "src/gpu/GrRenderTargetContext.h"
}

@class {
    static bool TestForPreservingPMConversions(GrDirectContext* dContext);
}

@cppEnd {
    bool GrConfigConversionEffect::TestForPreservingPMConversions(GrDirectContext* dContext) {
        static constexpr int kSize = 256;
        static constexpr GrColorType kColorType = GrColorType::kRGBA_8888;
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
                color[2] = std::min(x, y);
                color[1] = std::min(x, y);
                color[0] = std::min(x, y);
            }
        }
        memset(firstRead, 0, kSize * kSize * sizeof(uint32_t));
        memset(secondRead, 0, kSize * kSize * sizeof(uint32_t));

        const SkImageInfo ii = SkImageInfo::Make(kSize, kSize,
                                                 kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        auto readRTC = GrRenderTargetContext::Make(
                dContext, kColorType, nullptr, SkBackingFit::kExact, {kSize, kSize});
        auto tempRTC = GrRenderTargetContext::Make(
                dContext, kColorType, nullptr, SkBackingFit::kExact, {kSize, kSize});
        if (!readRTC || !readRTC->asTextureProxy() || !tempRTC) {
            return false;
        }
        // Adding discard to appease vulkan validation warning about loading uninitialized data on
        // draw
        readRTC->discard();

        // This function is only ever called if we are in a GrContext that has a GrGpu since we are
        // calling read pixels here. Thus the pixel data will be uploaded immediately and we don't
        // need to keep the pixel data alive in the proxy. Therefore the ReleaseProc is nullptr.
        SkBitmap bitmap;
        bitmap.installPixels(ii, srcData, 4 * kSize);
        bitmap.setImmutable();

        GrBitmapTextureMaker maker(dContext, bitmap, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        auto dataView = maker.view(GrMipmapped::kNo);
        if (!dataView) {
            return false;
        }

        static const SkRect kRect = SkRect::MakeIWH(kSize, kSize);

        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        GrPaint paint1;
        paint1.setColorFragmentProcessor(GrConfigConversionEffect::Make(
                GrTextureEffect::Make(std::move(dataView), kPremul_SkAlphaType),
                PMConversion::kToUnpremul));
        paint1.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(nullptr, std::move(paint1), GrAA::kNo, SkMatrix::I(), kRect, kRect);
        if (!readRTC->readPixels(dContext, ii, firstRead, 0, {0, 0})) {
            return false;
        }

        // Adding discard to appease vulkan validation warning about loading uninitialized data on
        // draw
        tempRTC->discard();

        GrPaint paint2;
        paint2.setColorFragmentProcessor(GrConfigConversionEffect::Make(
                GrTextureEffect::Make(readRTC->readSurfaceView(), kUnpremul_SkAlphaType),
                PMConversion::kToPremul));
        paint2.setPorterDuffXPFactory(SkBlendMode::kSrc);

        tempRTC->fillRectToRect(nullptr, std::move(paint2), GrAA::kNo, SkMatrix::I(), kRect, kRect);

        GrPaint paint3;
        paint3.setColorFragmentProcessor(GrConfigConversionEffect::Make(
                GrTextureEffect::Make(tempRTC->readSurfaceView(), kPremul_SkAlphaType),
                PMConversion::kToUnpremul));
        paint3.setPorterDuffXPFactory(SkBlendMode::kSrc);

        readRTC->fillRectToRect(nullptr, std::move(paint3), GrAA::kNo, SkMatrix::I(), kRect, kRect);

        if (!readRTC->readPixels(dContext, ii, secondRead, 0, {0, 0})) {
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
        return std::unique_ptr<GrFragmentProcessor>(
                new GrConfigConversionEffect(std::move(fp), pmConversion));
    }
}

layout(key) in PMConversion pmConversion;

@emitCode {
    fragBuilder->forceHighPrecision();
}

void main() {
    // Aggressively round to the nearest exact (N / 255) floating point value. This lets us find a
    // round-trip preserving pair on some GPUs that do odd byte to float conversion.
    sk_OutColor = floor(sample(inputFP) * 255 + 0.5) / 255;

    @switch (pmConversion) {
        case PMConversion::kToPremul:
            sk_OutColor.rgb = floor(sk_OutColor.rgb * sk_OutColor.a * 255 + 0.5) / 255;
            break;

        case PMConversion::kToUnpremul:
            sk_OutColor.rgb = sk_OutColor.a <= 0.0
                                      ? half3(0)
                                      : floor(sk_OutColor.rgb / sk_OutColor.a * 255 + 0.5) / 255;
            break;
    }
}

@test(data) {
    PMConversion pmConv = static_cast<PMConversion>(
            data->fRandom->nextULessThan((int)PMConversion::kPMConversionCnt));
    return std::unique_ptr<GrFragmentProcessor>(
            new GrConfigConversionEffect(GrProcessorUnitTest::MakeChildFP(data), pmConv));
}
