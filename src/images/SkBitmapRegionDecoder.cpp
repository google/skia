/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionDecoder.h"

bool SkBitmapRegionDecoder::decodeRegion(SkBitmap* bitmap, const SkIRect& rect,
                                         SkBitmap::Config pref, int sampleSize) {
    fDecoder->setSampleSize(sampleSize);
    return fDecoder->decodeRegion(bitmap, rect, pref);
}
