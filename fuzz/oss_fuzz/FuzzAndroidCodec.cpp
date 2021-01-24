/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"

#include "fuzz/Fuzz.h"

bool FuzzAndroidCodec(sk_sp<SkData> bytes, uint8_t sampleSize) {
    auto codec = SkAndroidCodec::MakeFromData(bytes);
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

    auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
    if (!surface) {
        // May return nullptr in memory-constrained fuzzing environments
        return false;
    }

    surface->getCanvas()->drawImage(bm.asImage(), 0, 0);
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    Fuzz fuzz(bytes);
    uint8_t sampleSize;
    fuzz.nextRange(&sampleSize, 1, 64);
    bytes = SkData::MakeSubset(bytes.get(), 1, size - 1);
    FuzzAndroidCodec(bytes, sampleSize);
    return 0;
}
#endif
