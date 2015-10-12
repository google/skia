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

/*
 * Three differences from the Android version:
 *     Returns a Skia bitmap instead of an Android bitmap.
 *     Android version attempts to reuse a recycled bitmap.
 *     Removed the options object and used parameters for color type and
 *     sample size.
 */
SkBitmap* SkBitmapRegionSampler::decodeRegion(int start_x, int start_y,
                                              int width, int height,
                                              int sampleSize,
                                              SkColorType prefColorType) {
    // Match Android's default settings
    fDecoder->setDitherImage(true);
    fDecoder->setPreferQualityOverSpeed(false);
    fDecoder->setRequireUnpremultipliedColors(false);
    fDecoder->setSampleSize(sampleSize);

    // kAlpha8 is the legacy representation of kGray8 used by SkImageDecoder
    if (kGray_8_SkColorType == prefColorType) {
        prefColorType = kAlpha_8_SkColorType;
    }

    SkIRect region;
    region.fLeft = start_x;
    region.fTop = start_y;
    region.fRight = start_x + width;
    region.fBottom = start_y + height;

    SkAutoTDelete<SkBitmap> bitmap(new SkBitmap());
    if (!fDecoder->decodeSubset(bitmap.get(), region, prefColorType)) {
        SkCodecPrintf("Error: decodeRegion failed.\n");
        return nullptr;
    }
    return bitmap.detach();
}
