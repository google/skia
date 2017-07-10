/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkRawAdapterCodec.h"

SkRawAdapterCodec::SkRawAdapterCodec(SkRawCodec* codec)
    : INHERITED(codec)
{}

SkISize SkRawAdapterCodec::onGetSampledDimensions(int sampleSize) const {
    float scale = 1.f / static_cast<float>(sampleSize);
    return this->codec()->getScaledDimensions(scale);
}

SkCodec::Result SkRawAdapterCodec::onGetAndroidPixels(
        const SkImageInfo& info, void* pixels, size_t rowBytes,
        const AndroidOptions& options) {
    SkCodec::Options codecOptions;
    codecOptions.fZeroInitialized = options.fZeroInitialized;
    codecOptions.fSubset = options.fSubset;
    return this->codec()->getPixels(info, pixels, rowBytes, &codecOptions);
}
