/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkAndroidCodecAdapter.h"
#include "src/codec/SkCodecPriv.h"

SkAndroidCodecAdapter::SkAndroidCodecAdapter(SkCodec* codec)
    : INHERITED(codec)
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
    return this->codec()->getPixels(info, pixels, rowBytes, &options);
}
