/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CodecBench.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkImageGenerator.h"
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
    fBitmap.allocPixels(codec->getInfo().makeColorType(fColorType));
}

void CodecBench::onDraw(const int n, SkCanvas* canvas) {
    SkAutoTDelete<SkCodec> codec;
    for (int i = 0; i < n; i++) {
        codec.reset(SkCodec::NewFromData(fData));
#ifdef SK_DEBUG
        const SkImageGenerator::Result result =
#endif
        // fBitmap.info() was set to use fColorType in onPreDraw.
        codec->getPixels(fBitmap.info(), fBitmap.getPixels(), fBitmap.rowBytes());
        SkASSERT(result == SkImageGenerator::kSuccess
                 || result == SkImageGenerator::kIncompleteInput);
    }
}
