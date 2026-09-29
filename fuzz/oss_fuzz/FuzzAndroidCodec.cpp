/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/private/SkGainmapInfo.h"

#include "fuzz/Fuzz.h"

static bool supports_subset_decoding(SkEncodedImageFormat format) {
    // TODO(b/568295764): Match `BitmapRegionDecoder::Make`: `SkSampledCodec::onGetSupportedSubset`
    // unconditionally returns true, even for bottom-up BMP and ICO codecs whose
    // `startScanlineDecode` does not reject `fSubset` and triggers `SkASSERT`s in `SkSampledCodec`
    // when `fSubset` is set.
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
        case SkEncodedImageFormat::kPNG:
        case SkEncodedImageFormat::kWEBP:
        case SkEncodedImageFormat::kHEIF:
        case SkEncodedImageFormat::kAVIF:
            return true;
        default:
            return false;
    }
}

// Derives a subset from the trailing bytes of `fuzzData` (rather than a prefix, so plain image
// files remain valid seeds and existing prefix-byte consumption is unchanged) and decodes it.
static void fuzz_android_codec_subset(SkAndroidCodec* codec,
                                      SkSurface* surface,
                                      const uint8_t* fuzzData,
                                      size_t fuzzSize,
                                      uint8_t sampleSize) {
    const SkISize origDims = codec->getInfo().dimensions();
    if (fuzzSize < 4 || origDims.isEmpty() ||
        !supports_subset_decoding(codec->getEncodedFormat())) {
        return;
    }

    const int x = fuzzData[fuzzSize - 4] % origDims.width();
    const int y = fuzzData[fuzzSize - 3] % origDims.height();
    const int w = (fuzzData[fuzzSize - 2] % (origDims.width() - x)) + 1;
    const int h = (fuzzData[fuzzSize - 1] % (origDims.height() - y)) + 1;
    SkIRect subset = SkIRect::MakeXYWH(x, y, w, h);
    if (!codec->getSupportedSubset(&subset)) {
        return;
    }

    const SkISize subsetSize = codec->getSampledSubsetDimensions(sampleSize, subset);
    SkBitmap subsetBm;
    if (!subsetBm.tryAllocPixels(SkImageInfo::MakeN32Premul(subsetSize))) {
        return;
    }

    SkAndroidCodec::AndroidOptions subsetOptions;
    subsetOptions.fSampleSize = sampleSize;
    subsetOptions.fSubset = &subset;
    const auto subsetResult = codec->getAndroidPixels(
            subsetBm.info(), subsetBm.getPixels(), subsetBm.rowBytes(), &subsetOptions);
    switch (subsetResult) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            surface->getCanvas()->drawImage(subsetBm.asImage(), 0, 0);
            break;
        default:
            break;
    }
}

bool FuzzAndroidCodec(const uint8_t *fuzzData, size_t fuzzSize, uint8_t sampleSize) {
    auto codec = SkAndroidCodec::MakeFromStream(SkMemoryStream::MakeDirect(fuzzData, fuzzSize));
    if (!codec) {
        return false;
    }

    auto size = codec->getSampledDimensions(sampleSize);
    auto info = SkImageInfo::MakeN32Premul(size);
    SkBitmap bm;
    if (!bm.tryAllocPixels(info)) {
        // May fail in memory-constrained fuzzing environments
        return false;
    }

    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = sampleSize;

    auto result = codec->getAndroidPixels(bm.info(), bm.getPixels(), bm.rowBytes(), &options);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            break;
        default:
            return false;
    }

    SkGainmapInfo gainmapInfo;
    auto gainmapImageStream = std::unique_ptr<SkStream>();

    if (codec->getAndroidGainmap(&gainmapInfo, &gainmapImageStream)) {
        // Do something with the outputs so the compiler does not optimize the call away.
        if (!std::isfinite(gainmapInfo.fDisplayRatioSdr)) {
            return false;
        }
        if (gainmapImageStream->getLength() > 100000000) {
            return false;
        }
    }

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(size.width(), size.height()));
    if (!surface) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }

    surface->getCanvas()->drawImage(bm.asImage(), 0, 0);
    fuzz_android_codec_subset(codec.get(), surface.get(), fuzzData, fuzzSize, sampleSize);
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    Fuzz fuzz(data, size);
    uint8_t sampleSize;
    fuzz.nextRange(&sampleSize, 1, 64);
    FuzzAndroidCodec(fuzz.remainingData(), fuzz.remainingSize(), sampleSize);
    return 0;
}
#endif
