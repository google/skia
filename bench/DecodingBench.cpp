/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodingBench.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of image decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
DecodingBench::DecodingBench(SkString path, SkColorType colorType)
    : fColorType(colorType)
{
    // Parse filename and the color type to give the benchmark a useful name
    SkString baseName = SkOSPath::Basename(path.c_str());
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
    fName.printf("Decode_%s_%s", baseName.c_str(), colorName);
    
    // Perform setup for the decode
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
    fDecoder.reset(SkImageDecoder::Factory(fStream.get()));
}

const char* DecodingBench::onGetName() {
    return fName.c_str();
}

bool DecodingBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}

void DecodingBench::onDraw(const int n, SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int i = 0; i < n; i++) {
        fStream->rewind();
        fDecoder->decode(fStream, &bitmap, fColorType,
                         SkImageDecoder::kDecodePixels_Mode);
    }
}
