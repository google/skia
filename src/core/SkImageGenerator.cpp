/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkNextID.h"

SkImageGenerator::SkImageGenerator(const SkImageInfo& info, uint32_t uniqueID)
    : fInfo(info)
    , fUniqueID(kNeedNewImageUniqueID == uniqueID ? SkNextID::ImageID() : uniqueID)
{}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                 const Options* opts) {
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
    if (!opts) {
        opts = &defaultOpts;
    }
    return this->onGetPixels(info, pixels, rowBytes, *opts);
}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    return this->getPixels(info, pixels, rowBytes, nullptr);
}

bool SkImageGenerator::queryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const {
    SkASSERT(sizeInfo);

    return this->onQueryYUV8(sizeInfo, colorSpace);
}

bool SkImageGenerator::getYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth >= 0);
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kY].fHeight >= 0);
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kU].fWidth >= 0);
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kU].fHeight >= 0);
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kV].fWidth >= 0);
    SkASSERT(sizeInfo.fSizes[SkYUVSizeInfo::kV].fHeight >= 0);
    SkASSERT(sizeInfo.fWidthBytes[SkYUVSizeInfo::kY] >=
            (size_t) sizeInfo.fSizes[SkYUVSizeInfo::kY].fWidth);
    SkASSERT(sizeInfo.fWidthBytes[SkYUVSizeInfo::kU] >=
            (size_t) sizeInfo.fSizes[SkYUVSizeInfo::kU].fWidth);
    SkASSERT(sizeInfo.fWidthBytes[SkYUVSizeInfo::kV] >=
            (size_t) sizeInfo.fSizes[SkYUVSizeInfo::kV].fWidth);
    SkASSERT(planes && planes[0] && planes[1] && planes[2]);

    return this->onGetYUV8Planes(sizeInfo, planes);
}

#if SK_SUPPORT_GPU
#include "GrTextureProxy.h"

sk_sp<GrTextureProxy> SkImageGenerator::generateTexture(GrContext* ctx, const SkImageInfo& info,
                                                        const SkIPoint& origin,
                                                        SkTransferFunctionBehavior behavior) {
    SkIRect srcRect = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height());
    if (!SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(srcRect)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, info, origin, behavior);
}

sk_sp<GrTextureProxy> SkImageGenerator::onGenerateTexture(GrContext*, const SkImageInfo&,
                                                          const SkIPoint&,
                                                          SkTransferFunctionBehavior) {
    return nullptr;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkBitmap.h"
#include "SkColorTable.h"

#include "SkGraphics.h"

static SkGraphics::ImageGeneratorFromEncodedDataFactory gFactory;

SkGraphics::ImageGeneratorFromEncodedDataFactory
SkGraphics::SetImageGeneratorFromEncodedDataFactory(ImageGeneratorFromEncodedDataFactory factory)
{
    ImageGeneratorFromEncodedDataFactory prev = gFactory;
    gFactory = factory;
    return prev;
}

std::unique_ptr<SkImageGenerator> SkImageGenerator::MakeFromEncoded(sk_sp<SkData> data) {
    if (!data) {
        return nullptr;
    }
    if (gFactory) {
        if (std::unique_ptr<SkImageGenerator> generator = gFactory(data)) {
            return generator;
        }
    }
    return SkImageGenerator::MakeFromEncodedImpl(std::move(data));
}
