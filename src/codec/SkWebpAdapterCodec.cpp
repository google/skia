/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkWebpAdapterCodec.h"

SkWebpAdapterCodec::SkWebpAdapterCodec(SkWebpCodec* codec)
    : INHERITED(codec)
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
    // SkWebpCodec will support pretty much any dimensions that we provide, but we want
    // to be stricter about the type of scaling that we allow, so we will add an extra
    // check here.
    SkISize supportedSize;
    if (!options.fSubset) {
        supportedSize = this->onGetSampledDimensions(options.fSampleSize);
    } else {
        supportedSize = this->getSampledSubsetDimensions(options.fSampleSize, *options.fSubset);
    }
    if (supportedSize != info.dimensions()) {
        return SkCodec::kInvalidParameters;
    }

    SkCodec::Options codecOptions;
    codecOptions.fZeroInitialized = options.fZeroInitialized;
    codecOptions.fSubset = options.fSubset;
    codecOptions.fPremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return this->codec()->getPixels(info, pixels, rowBytes, &codecOptions);
}
