/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BitmapRegionDecoderBench.h"
#include "CodecBenchPriv.h"
#include "SkBitmap.h"
#include "SkOSFile.h"

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
    return kNonRendering_Backend == backend;
}

void BitmapRegionDecoderBench::onDelayedSetup() {
    fBRD.reset(SkBitmapRegionDecoder::Create(fData, SkBitmapRegionDecoder::kAndroidCodec_Strategy));
}

void BitmapRegionDecoderBench::onDraw(int n, SkCanvas* canvas) {
    for (int i = 0; i < n; i++) {
        SkBitmap bm;
        SkAssertResult(fBRD->decodeRegion(&bm, nullptr, fSubset, fSampleSize, fColorType, false));
    }
}
