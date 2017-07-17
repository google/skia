/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecImageGenerator.h"
#include "SkMakeUnique.h"

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(sk_sp<SkData> data) {
    SkCodec* codec = SkCodec::NewFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(codec, data));
}

static SkImageInfo adjust_info(const SkImageInfo& info) {
    SkImageInfo newInfo = info;
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        newInfo = newInfo.makeAlphaType(kPremul_SkAlphaType);
    }
    return newInfo;
}

SkCodecImageGenerator::SkCodecImageGenerator(SkCodec* codec, sk_sp<SkData> data)
    : INHERITED(adjust_info(codec->getInfo()))
    , fCodec(codec)
    , fData(std::move(data))
{}

SkData* SkCodecImageGenerator::onRefEncodedData() {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                        const Options& opts) {
    SkCodec::Options codecOpts;
    codecOpts.fPremulBehavior = opts.fBehavior;
    SkCodec::Result result = fCodec->getPixels(info, pixels, rowBytes, &codecOpts);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}

bool SkCodecImageGenerator::onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const
{
    return fCodec->queryYUV8(sizeInfo, colorSpace);
}

bool SkCodecImageGenerator::onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
    SkCodec::Result result = fCodec->getYUV8Planes(sizeInfo, planes);

    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}
