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
    #include "src/gpu/GrDirectContextPriv.h"
    #include "src/gpu/GrImageInfo.h"
    #include "src/gpu/GrSurfaceDrawContext.h"
}

@class {
    static bool TestForPreservingPMConversions(GrDirectContext* dContext);
}

@cppEnd {
    bool GrConfigConversionEffect::TestForPreservingPMConversions(GrDirectContext* dContext) {
        static constexpr int kSize = 256;
        SkAutoTMalloc<uint32_t> data(kSize * kSize * 3);
        uint32_t* srcData = data.get();

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

        const SkImageInfo pmII =
                SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        const SkImageInfo upmII = pmII.makeAlphaType(kUnpremul_SkAlphaType);

        auto readSFC = GrSurfaceFillContext::Make(dContext, upmII, SkBackingFit::kExact);
        auto tempSFC = GrSurfaceFillContext::Make(dContext,  pmII, SkBackingFit::kExact);
        if (!readSFC || !tempSFC) {
            return false;
        }

        // This function is only ever called if we are in a GrDirectContext since we are
        // calling read pixels here. Thus the pixel data will be uploaded immediately and we don't
        // need to keep the pixel data alive in the proxy. Therefore the ReleaseProc is nullptr.
        SkBitmap bitmap;
        bitmap.installPixels(pmII, srcData, 4 * kSize);
        bitmap.setImmutable();

        GrBitmapTextureMaker maker(dContext, bitmap, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
        auto dataView = maker.view(GrMipmapped::kNo);
        if (!dataView) {
            return false;
        }

        uint32_t* firstRead  = data.get() +   kSize*kSize;
        uint32_t* secondRead = data.get() + 2*kSize*kSize;
        std::fill_n( firstRead, kSize*kSize, 0);
        std::fill_n(secondRead, kSize*kSize, 0);

        GrPixmap firstReadPM( upmII,  firstRead, kSize*sizeof(uint32_t));
        GrPixmap secondReadPM(upmII, secondRead, kSize*sizeof(uint32_t));

        // We do a PM->UPM draw from dataTex to readTex and read the data. Then we do a UPM->PM draw
        // from readTex to tempTex followed by a PM->UPM draw to readTex and finally read the data.
        // We then verify that two reads produced the same values.

        auto fp1 = GrConfigConversionEffect::Make(GrTextureEffect::Make(std::move(dataView),
                                                                        bitmap.alphaType()),
                                                  PMConversion::kToUnpremul);
        readSFC->fillRectWithFP(SkIRect::MakeWH(kSize, kSize), std::move(fp1));
        if (!readSFC->readPixels(dContext, firstReadPM, {0, 0})) {
            return false;
        }

        auto fp2 = GrConfigConversionEffect::Make(
                GrTextureEffect::Make(readSFC->readSurfaceView(),
                                      readSFC->colorInfo().alphaType()),
                PMConversion::kToPremul);
        tempSFC->fillRectWithFP(SkIRect::MakeWH(kSize, kSize), std::move(fp2));

        auto fp3 = GrConfigConversionEffect::Make(
                GrTextureEffect::Make(tempSFC->readSurfaceView(),
                                      tempSFC->colorInfo().alphaType()),
                PMConversion::kToUnpremul);
        readSFC->fillRectWithFP(SkIRect::MakeWH(kSize, kSize), std::move(fp3));

        if (!readSFC->readPixels(dContext, secondReadPM, {0, 0})) {
            return false;
        }

        for (int y = 0; y < kSize; ++y) {
            for (int x = 0; x <= y; ++x) {
                if (firstRead[kSize*y + x] != secondRead[kSize*y + x]) {
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

half4 main() {
    // Aggressively round to the nearest exact (N / 255) floating point value. This lets us find a
    // round-trip preserving pair on some GPUs that do odd byte to float conversion.
    half4 color = floor(sample(inputFP) * 255 + 0.5) / 255;

    @switch (pmConversion) {
        case PMConversion::kToPremul:
            color.rgb = floor(color.rgb * color.a * 255 + 0.5) / 255;
            break;

        case PMConversion::kToUnpremul:
            color.rgb = color.a <= 0.0
                            ? half3(0)
                            : floor(color.rgb / color.a * 255 + 0.5) / 255;
            break;
    }

    return color;
}

@test(data) {
    PMConversion pmConv = static_cast<PMConversion>(
            data->fRandom->nextRangeU(0, (int)PMConversion::kLast));
    return std::unique_ptr<GrFragmentProcessor>(
            new GrConfigConversionEffect(GrProcessorUnitTest::MakeChildFP(data), pmConv));
}
