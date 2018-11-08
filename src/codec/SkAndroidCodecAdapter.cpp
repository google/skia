/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodecAdapter.h"
#include "SkCodecPriv.h"

SkAndroidCodecAdapter::SkAndroidCodecAdapter(SkCodec* codec, ExifOrientationBehavior behavior)
    : INHERITED(codec, behavior)
{}

SkISize SkAndroidCodecAdapter::onGetSampledDimensions(int sampleSize) const {
    float scale = get_scale_from_sample_size(sampleSize);
    return this->codec()->getScaledDimensions(scale);
}

bool SkAndroidCodecAdapter::onGetSupportedSubset(SkIRect* desiredSubset) const {
    return this->codec()->getValidSubset(desiredSubset);
}

SkCodec::Result SkAndroidCodecAdapter::onGetAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const AndroidOptions& options) {
    SkCodec::Options codecOptions;
    codecOptions.fZeroInitialized = options.fZeroInitialized;
    codecOptions.fSubset = options.fSubset;
    return this->codec()->getPixels(info, pixels, rowBytes, &codecOptions);
}
