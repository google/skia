/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkWebpAdapterCodec.h"

SkWebpAdapterCodec::SkWebpAdapterCodec(SkWebpCodec* codec, ExifOrientationBehavior behavior)
    : INHERITED(codec, behavior)
{}

SkISize SkWebpAdapterCodec::onGetSampledDimensions(int sampleSize) const {
    float scale = get_scale_from_sample_size(sampleSize);
    return this->codec()->getScaledDimensions(scale);
}

bool SkWebpAdapterCodec::onGetSupportedSubset(SkIRect* desiredSubset) const {
    return this->codec()->getValidSubset(desiredSubset);
}

SkCodec::Result SkWebpAdapterCodec::onGetAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const AndroidOptions& options) {
    SkCodec::Options codecOptions;
    codecOptions.fZeroInitialized = options.fZeroInitialized;
    codecOptions.fSubset = options.fSubset;
    codecOptions.fPremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return this->codec()->getPixels(info, pixels, rowBytes, &codecOptions);
}
