/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAIndex.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/core/SkPixmapPriv.h"

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(sk_sp<SkData> data) {
    auto codec = SkCodec::MakeFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), data));
}

std::unique_ptr<SkImageGenerator>
SkCodecImageGenerator::MakeFromCodec(std::unique_ptr<SkCodec> codec) {
    return codec
        ? std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), nullptr))
        : nullptr;
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

sk_sp<SkData> SkCodecImageGenerator::onRefEncodedData() {
    return fData;
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& requestInfo, void* requestPixels,
                                        size_t requestRowBytes, const Options&) {
    SkPixmap dst(requestInfo, requestPixels, requestRowBytes);

    auto decode = [this](const SkPixmap& pm) {
        SkCodec::Result result = fCodec->getPixels(pm);
        switch (result) {
            case SkCodec::kSuccess:
            case SkCodec::kIncompleteInput:
            case SkCodec::kErrorInInput:
                return true;
            default:
                return false;
        }
    };

    return SkPixmapPriv::Orient(dst, fCodec->getOrigin(), decode);
}

bool SkCodecImageGenerator::onQueryYUVA8(SkYUVASizeInfo* sizeInfo,
                                         SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                         SkYUVColorSpace* colorSpace) const {
    // This image generator always returns 3 separate non-interleaved planes
    yuvaIndices[SkYUVAIndex::kY_Index].fIndex = 0;
    yuvaIndices[SkYUVAIndex::kY_Index].fChannel = SkColorChannel::kR;
    yuvaIndices[SkYUVAIndex::kU_Index].fIndex = 1;
    yuvaIndices[SkYUVAIndex::kU_Index].fChannel = SkColorChannel::kR;
    yuvaIndices[SkYUVAIndex::kV_Index].fIndex = 2;
    yuvaIndices[SkYUVAIndex::kV_Index].fChannel = SkColorChannel::kR;
    yuvaIndices[SkYUVAIndex::kA_Index].fIndex = -1;
    yuvaIndices[SkYUVAIndex::kA_Index].fChannel = SkColorChannel::kR;

    return fCodec->queryYUV8(sizeInfo, colorSpace);
}

bool SkCodecImageGenerator::onGetYUVA8Planes(const SkYUVASizeInfo& sizeInfo,
                                             const SkYUVAIndex indices[SkYUVAIndex::kIndexCount],
                                             void* planes[]) {
    SkCodec::Result result = fCodec->getYUV8Planes(sizeInfo, planes);
    // TODO: check indices

    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}
