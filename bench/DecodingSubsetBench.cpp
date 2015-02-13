/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodingSubsetBench.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkStream.h"

/*
 *
 * This benchmark is designed to test the performance of image subset decoding.
 * It is invoked from the nanobench.cpp file.
 *
 */
DecodingSubsetBench::DecodingSubsetBench(SkString path, SkColorType colorType,
        const int divisor)
    : fColorType(colorType)
    , fDivisor(divisor)
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
    fName.printf("DecodeSubset_%dx%d_%s_%s", fDivisor, fDivisor,
            baseName.c_str(), colorName);
    
    // Perform the decode setup
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    fStream.reset(new SkMemoryStream(encoded));
    fDecoder.reset(SkImageDecoder::Factory(fStream));
}

const char* DecodingSubsetBench::onGetName() {
    return fName.c_str();
}

bool DecodingSubsetBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend == backend;
}
    
void DecodingSubsetBench::onDraw(const int n, SkCanvas* canvas) {
    for (int i = 0; i < n; i++) {
        int w, h;
        fDecoder->buildTileIndex(fStream->duplicate(), &w, &h);
        // Divide the image into subsets and decode each subset
        const int sW  = w / fDivisor;
        const int sH = h / fDivisor;
        for (int y = 0; y < h; y += sH) {
            for (int x = 0; x < w; x += sW) {
                SkBitmap bitmap;
                SkIRect rect = SkIRect::MakeXYWH(x, y, sW, sH);
                fDecoder->decodeSubset(&bitmap, rect, fColorType);
            }
        }
    }
}
