/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkCodecImageGenerator.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkPixmapUtils.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkTypes.h"
#include "src/codec/SkPixmapUtilsPriv.h"

#include <utility>


std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(
        sk_sp<SkData> data, std::optional<SkAlphaType> at) {
    auto codec = SkCodec::MakeFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), data, at));
}

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromCodec(
        std::unique_ptr<SkCodec> codec) {
    return codec ? std::unique_ptr<SkImageGenerator>(
                           new SkCodecImageGenerator(std::move(codec), nullptr, std::nullopt))
                 : nullptr;
}

static SkImageInfo adjust_info(SkCodec* codec, std::optional<SkAlphaType> at) {
    SkASSERT(at != kOpaque_SkAlphaType);
    SkImageInfo info = codec->getInfo();
    if (at.has_value()) {
        // If a specific alpha type was requested, use that.
        info = info.makeAlphaType(*at);
    } else if (kUnpremul_SkAlphaType == info.alphaType()) {
        // Otherwise, prefer premul over unpremul (this produces better filtering in general)
        info = info.makeAlphaType(kPremul_SkAlphaType);
    }
    if (SkEncodedOriginSwapsWidthHeight(codec->getOrigin())) {
        info = SkPixmapUtils::SwapWidthHeight(info);
    }
    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(std::unique_ptr<SkCodec> codec,
                                             sk_sp<SkData> data,
                                             std::optional<SkAlphaType> at)
        : INHERITED(adjust_info(codec.get(), at))
        , fCodec(std::move(codec))
        , fData(std::move(data)) {}

sk_sp<SkData> SkCodecImageGenerator::onRefEncodedData() {
    return fData;
}

bool SkCodecImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const SkCodec::Options* options) {
    SkPixmap dst(info, pixels, rowBytes);

    auto decode = [this, options](const SkPixmap& pm) {
        SkCodec::Result result = fCodec->getPixels(pm, options);
        switch (result) {
            case SkCodec::kSuccess:
            case SkCodec::kIncompleteInput:
            case SkCodec::kErrorInInput:
                return true;
            default:
                return false;
        }
    };

    return SkPixmapUtils::Orient(dst, fCodec->getOrigin(), decode);
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& requestInfo, void* requestPixels,
                                        size_t requestRowBytes, const Options& options) {
    return this->getPixels(requestInfo, requestPixels, requestRowBytes, nullptr);
}

bool SkCodecImageGenerator::onQueryYUVAInfo(
        const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
        SkYUVAPixmapInfo* yuvaPixmapInfo) const {
    return fCodec->queryYUVAInfo(supportedDataTypes, yuvaPixmapInfo);
}

bool SkCodecImageGenerator::onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) {
    switch (fCodec->getYUVAPlanes(yuvaPixmaps)) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}

SkISize SkCodecImageGenerator::getScaledDimensions(float desiredScale) const {
    SkISize size = fCodec->getScaledDimensions(desiredScale);
    if (SkEncodedOriginSwapsWidthHeight(fCodec->getOrigin())) {
        std::swap(size.fWidth, size.fHeight);
    }
    return size;
}
