/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"

#include "include/core/SkImage.h"
#include "src/core/SkNextID.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#endif

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
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

GrSurfaceProxyView SkImageGenerator::generateTexture(GrRecordingContext* ctx,
                                                     const SkImageInfo& info,
                                                     GrMipmapped mipmapped,
                                                     GrImageTexGenPolicy texGenPolicy) {
    SkASSERT_RELEASE(fInfo.dimensions() == info.dimensions());

    if (!ctx || ctx->abandoned()) {
        return {};
    }

    return this->onGenerateTexture(ctx, info, mipmapped, texGenPolicy);
}

GrSurfaceProxyView SkImageGenerator::onGenerateTexture(GrRecordingContext*,
                                                       const SkImageInfo&,
                                                       GrMipmapped,
                                                       GrImageTexGenPolicy) {
    return {};
}
#endif // SK_SUPPORT_GPU

#if SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/Image_Graphite.h"

sk_sp<SkImage> SkImageGenerator::makeTextureImage(skgpu::graphite::Recorder* recorder,
                                                  const SkImageInfo& info,
                                                  skgpu::Mipmapped mipmapped) {
    // This still allows for a difference in colorType and colorSpace. Just no subsetting.
    if (fInfo.dimensions() != info.dimensions()) {
        return nullptr;
    }

    return this->onMakeTextureImage(recorder, info, mipmapped);
}

sk_sp<SkImage> SkImageGenerator::onMakeTextureImage(skgpu::graphite::Recorder*,
                                                    const SkImageInfo&,
                                                    skgpu::Mipmapped) {
    return nullptr;
}

#endif // SK_GRAPHITE_ENABLED

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
        sk_sp<SkData> data, std::optional<SkAlphaType> at) {
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
