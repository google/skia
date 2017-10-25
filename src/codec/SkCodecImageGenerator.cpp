/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecImageGenerator.h"
#include "SkMakeUnique.h"
#include "SkPixmapPriv.h"

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(sk_sp<SkData> data) {
    auto codec = SkCodec::MakeFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), data));
}

static SkImageInfo adjust_info(SkCodec* codec) {
    SkImageInfo info = codec->getInfo();
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        info = info.makeAlphaType(kPremul_SkAlphaType);
    }
    if (SkPixmapPriv::ShouldSwapWidthHeight(codec->getOrigin())) {
        info = SkPixmapPriv::SwapWidthHeight(info);
    }
    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(std::unique_ptr<SkCodec> codec, sk_sp<SkData> data)
    : INHERITED(adjust_info(codec.get()))
    , fCodec(std::move(codec))
    , fData(std::move(data))
{}

SkData* SkCodecImageGenerator::onRefEncodedData() {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& requestInfo, void* requestPixels,
                                        size_t requestRowBytes, const Options& opts) {
    const auto origin = fCodec->getOrigin();
    SkPixmapPriv::Orienter orienter(requestInfo, requestPixels, requestRowBytes, origin);
    const SkPixmap* codecMap = orienter.preOrientDst();
    if (!codecMap) {
        return false;
    }

    SkCodec::Options codecOpts;
    codecOpts.fPremulBehavior = opts.fBehavior;
    SkCodec::Result result = fCodec->getPixels(*codecMap, &codecOpts);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return orienter.orientIfNecessary();
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
