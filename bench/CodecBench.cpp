/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBench.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkOSFile.h"

CodecBench::CodecBench(SkString baseName, SkData* encoded, SkColorType colorType)
    : fColorType(colorType)
    , fData(SkRef(encoded))
{
    // Parse filename and the color type to give the benchmark a useful name
    const char* colorName;
    switch(colorType) {
        case kN32_SkColorType:
            colorName = "N32";
            break;
        case kRGB_565_SkColorType:
            colorName = "565";
            break;
        case kAlpha_8_SkColorType:
            colorName = "Alpha8";
            break;
        default:
            colorName = "Unknown";
    }
    fName.printf("Codec_%s_%s", baseName.c_str(), colorName);
#ifdef SK_DEBUG
    // Ensure that we can create an SkCodec from this data.
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(fData));
    SkASSERT(codec);
#endif
}

const char* CodecBench::onGetName() {
    return fName.c_str();
}

bool CodecBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void CodecBench::onPreDraw() {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(fData));

    fInfo = codec->getInfo().makeColorType(fColorType);
    SkAlphaType alphaType;
    // Caller should not have created this CodecBench if the alpha type was
    // invalid.
    SkAssertResult(SkColorTypeValidateAlphaType(fColorType, fInfo.alphaType(),
                                                &alphaType));
    if (alphaType != fInfo.alphaType()) {
        fInfo = fInfo.makeAlphaType(alphaType);
    }

    fPixelStorage.reset(fInfo.getSafeSize(fInfo.minRowBytes()));
}

void CodecBench::onDraw(const int n, SkCanvas* canvas) {
    SkAutoTDelete<SkCodec> codec;
    SkPMColor colorTable[256];
    int colorCount;
    for (int i = 0; i < n; i++) {
        colorCount = 256;
        codec.reset(SkCodec::NewFromData(fData));
#ifdef SK_DEBUG
        const SkCodec::Result result =
#endif
        codec->getPixels(fInfo, fPixelStorage.get(), fInfo.minRowBytes(),
                         NULL, colorTable, &colorCount);
        SkASSERT(result == SkCodec::kSuccess
                 || result == SkCodec::kIncompleteInput);
    }
}
