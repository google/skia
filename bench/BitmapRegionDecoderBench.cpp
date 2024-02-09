/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/BitmapRegionDecoderBench.h"
#ifdef SK_ENABLE_ANDROID_UTILS
#include "bench/CodecBenchPriv.h"
#include "client_utils/android/BitmapRegionDecoder.h"
#include "include/core/SkBitmap.h"
#include "src/core/SkOSFile.h"

BitmapRegionDecoderBench::BitmapRegionDecoderBench(const char* baseName, SkData* encoded,
        SkColorType colorType, uint32_t sampleSize, const SkIRect& subset)
    : fBRD(nullptr)
    , fData(SkRef(encoded))
    , fColorType(colorType)
    , fSampleSize(sampleSize)
    , fSubset(subset)
{
    // Choose a useful name for the color type
    const char* colorName = color_type_to_str(colorType);

    fName.printf("BRD_%s_%s", baseName, colorName);
    if (1 != sampleSize) {
        fName.appendf("_%.3f", 1.0f / (float) sampleSize);
    }
}

const char* BitmapRegionDecoderBench::onGetName() {
    return fName.c_str();
}

bool BitmapRegionDecoderBench::isSuitableFor(Backend backend) {
    return Backend::kNonRendering == backend;
}

void BitmapRegionDecoderBench::onDelayedSetup() {
    fBRD = android::skia::BitmapRegionDecoder::Make(fData);
}

void BitmapRegionDecoderBench::onDraw(int n, SkCanvas* canvas) {
    auto ct = fBRD->computeOutputColorType(fColorType);
    auto cs = fBRD->computeOutputColorSpace(ct, nullptr);
    for (int i = 0; i < n; i++) {
        SkBitmap bm;
        SkAssertResult(fBRD->decodeRegion(&bm, nullptr, fSubset, fSampleSize, ct, false, cs));
    }
}
#endif // SK_ENABLE_ANDROID_UTILS
