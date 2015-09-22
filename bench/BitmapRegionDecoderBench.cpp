/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BitmapRegionDecoderBench.h"
#include "CodecBenchPriv.h"
#include "SkBitmap.h"
#include "SkCodecTools.h"
#include "SkOSFile.h"

BitmapRegionDecoderBench::BitmapRegionDecoderBench(const char* baseName, SkData* encoded,
        SkBitmapRegionDecoderInterface::Strategy strategy, SkColorType colorType,
        uint32_t sampleSize, const SkIRect& subset)
    : fBRD(nullptr)
    , fData(SkRef(encoded))
    , fStrategy(strategy)
    , fColorType(colorType)
    , fSampleSize(sampleSize)
    , fSubset(subset)
{
    // Choose a useful name for the region decoding strategy
    const char* strategyName;
    switch (strategy) {
        case SkBitmapRegionDecoderInterface::kOriginal_Strategy:
            strategyName = "Original";
            break;
        case SkBitmapRegionDecoderInterface::kCanvas_Strategy:
            strategyName = "Canvas";
            break;
        default:
            SkASSERT(false);
            strategyName = "";
            break;
    }

    // Choose a useful name for the color type
    const char* colorName = color_type_to_str(colorType);

    fName.printf("BRD_%s_%s_%s", baseName, strategyName, colorName);
    if (1 != sampleSize) {
        fName.appendf("_%.3f", get_scale_from_sample_size(sampleSize));
    }
}

const char* BitmapRegionDecoderBench::onGetName() {
    return fName.c_str();
}

bool BitmapRegionDecoderBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void BitmapRegionDecoderBench::onPreDraw() {
    SkStreamRewindable* stream = new SkMemoryStream(fData);
    fBRD.reset(SkBitmapRegionDecoderInterface::CreateBitmapRegionDecoder(stream, fStrategy));
}

void BitmapRegionDecoderBench::onDraw(const int n, SkCanvas* canvas) {
    SkAutoTDelete<SkBitmap> bitmap;
    for (int i = 0; i < n; i++) {
        bitmap.reset(fBRD->decodeRegion(fSubset.left(), fSubset.top(), fSubset.width(),
                fSubset.height(), fSampleSize, fColorType));
        SkASSERT(nullptr != bitmap.get());
    }
}
