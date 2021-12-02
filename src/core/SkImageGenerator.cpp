/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"

#include "include/core/SkImage.h"
#include "src/core/SkNextID.h"

SkImageGenerator::SkImageGenerator(const SkImageInfo& info, uint32_t uniqueID)
    : fInfo(info)
    , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID)
{}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (nullptr == pixels) {
        return false;
    }
    if (rowBytes < info.minRowBytes()) {
        return false;
    }

    Options defaultOpts;
    return this->onGetPixels(info, pixels, rowBytes, defaultOpts);
}

bool SkImageGenerator::queryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& supportedDataTypes,
                                     SkYUVAPixmapInfo* yuvaPixmapInfo) const {
    SkASSERT(yuvaPixmapInfo);

    return this->onQueryYUVAInfo(supportedDataTypes, yuvaPixmapInfo) &&
           yuvaPixmapInfo->isSupported(supportedDataTypes);
}

bool SkImageGenerator::getYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) {
    return this->onGetYUVAPlanes(yuvaPixmaps);
}

#if SK_SUPPORT_GPU
#include "src/gpu/GrSurfaceProxyView.h"

GrSurfaceProxyView SkImageGenerator::generateTexture(GrRecordingContext* ctx,
                                                     const SkImageInfo& info,
                                                     const SkIPoint& origin,
                                                     GrMipmapped mipMapped,
                                                     GrImageTexGenPolicy texGenPolicy) {
    SkIRect srcRect = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height());
    if (!SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(srcRect)) {
        return {};
    }
    return this->onGenerateTexture(ctx, info, origin, mipMapped, texGenPolicy);
}

GrSurfaceProxyView SkImageGenerator::onGenerateTexture(GrRecordingContext*,
                                                       const SkImageInfo&,
                                                       const SkIPoint&,
                                                       GrMipmapped,
                                                       GrImageTexGenPolicy) {
    return {};
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkBitmap.h"
#include "src/codec/SkColorTable.h"

#include "include/core/SkGraphics.h"

static SkGraphics::ImageGeneratorFromEncodedDataFactory gFactory;

SkGraphics::ImageGeneratorFromEncodedDataFactory
SkGraphics::SetImageGeneratorFromEncodedDataFactory(ImageGeneratorFromEncodedDataFactory factory)
{
    ImageGeneratorFromEncodedDataFactory prev = gFactory;
    gFactory = factory;
    return prev;
}

std::unique_ptr<SkImageGenerator> SkImageGenerator::MakeFromEncoded(
        sk_sp<SkData> data, skstd::optional<SkAlphaType> at) {
    if (!data || at == kOpaque_SkAlphaType) {
        return nullptr;
    }
    if (gFactory) {
        if (std::unique_ptr<SkImageGenerator> generator = gFactory(data)) {
            return generator;
        }
    }
    return SkImageGenerator::MakeFromEncodedImpl(std::move(data), at);
}
