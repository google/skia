/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionSampler.h"
#include "SkCodecPriv.h"

SkBitmapRegionSampler::SkBitmapRegionSampler(SkImageDecoder* decoder, int width, 
                                             int height)
    : INHERITED(width, height)
    , fDecoder(decoder)
{}

bool SkBitmapRegionSampler::decodeRegion(SkBitmap* bitmap, SkBitmap::Allocator* allocator,
        const SkIRect& desiredSubset, int sampleSize, SkColorType colorType, bool requireUnpremul) {
    fDecoder->setDitherImage(true);
    fDecoder->setPreferQualityOverSpeed(false);
    fDecoder->setRequireUnpremultipliedColors(false);
    fDecoder->setSampleSize(sampleSize);
    fDecoder->setAllocator(allocator);

    // kAlpha8 is the legacy representation of kGray8 used by SkImageDecoder
    if (kGray_8_SkColorType == colorType) {
        colorType = kAlpha_8_SkColorType;
    }

    bool result = fDecoder->decodeSubset(bitmap, desiredSubset, colorType);
    fDecoder->setAllocator(nullptr);
    return result;
}
