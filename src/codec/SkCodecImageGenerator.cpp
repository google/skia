/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecImageGenerator.h"

SkImageGenerator* SkCodecImageGenerator::NewFromEncodedCodec(sk_sp<SkData> data) {
    SkCodec* codec = SkCodec::NewFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return new SkCodecImageGenerator(codec, data);
}

static SkImageInfo make_premul(const SkImageInfo& info) {
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        return info.makeAlphaType(kPremul_SkAlphaType);
    }

    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(SkCodec* codec, sk_sp<SkData> data)
    : INHERITED(make_premul(codec->getInfo()))
    , fCodec(codec)
    , fData(std::move(data))
{}

SkData* SkCodecImageGenerator::onRefEncodedData(GrContext* ctx) {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
        SkPMColor ctable[], int* ctableCount) {
    SkCodec::Result result = fCodec->getPixels(info, pixels, rowBytes, nullptr, ctable,
            ctableCount);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
            return true;
        default:
            return false;
    }
}

bool SkCodecImageGenerator::onComputeScaledDimensions(SkScalar scale, SupportedSizes* sizes) {
    SkASSERT(scale > 0 && scale <= 1);
    const auto size = fCodec->getScaledDimensions(SkScalarToFloat(scale));
    if (size == this->getInfo().dimensions()) {
        return false;
    }

    // FIXME: Make SkCodec's API return two potential sizes, like this one. For now, set them both
    // to be the same.
    sizes->fSizes[0] = sizes->fSizes[1] = size;
    return true;
}

bool SkCodecImageGenerator::onGenerateScaledPixels(const SkPixmap& pixmap) {
    if (pixmap.colorType() == kIndex_8_SkColorType) {
        // There is no way to tell the client about the color table with this API.
        return false;
    }

    return this->onGetPixels(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes(),
                             nullptr, nullptr);
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
            return true;
        default:
            return false;
    }
}
