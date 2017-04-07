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
                                 SkPMColor ctable[], int* ctableCount) {
    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (nullptr == pixels) {
        return false;
    }
    if (rowBytes < info.minRowBytes()) {
        return false;
    }

    if (kIndex_8_SkColorType == info.colorType()) {
        if (nullptr == ctable || nullptr == ctableCount) {
            return false;
        }
    } else {
        if (ctableCount) {
            *ctableCount = 0;
        }
        ctableCount = nullptr;
        ctable = nullptr;
    }

    const bool success = this->onGetPixels(info, pixels, rowBytes, ctable, ctableCount);
    if (success && ctableCount) {
        SkASSERT(*ctableCount >= 0 && *ctableCount <= 256);
    }
    return success;
}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                 const Options* opts) {
    Options defaultOpts;
    if (!opts) {
        opts = &defaultOpts;
    }
    return this->onGetPixels(info, pixels, rowBytes, *opts);
}

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    SkASSERT(kIndex_8_SkColorType != info.colorType());
    if (kIndex_8_SkColorType == info.colorType()) {
        return false;
    }
    return this->getPixels(info, pixels, rowBytes, nullptr, nullptr);
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
                                                        const SkIPoint& origin) {
    SkIRect srcRect = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height());
    if (!SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(srcRect)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, info, origin);
}

sk_sp<GrTextureProxy> SkImageGenerator::onGenerateTexture(GrContext*, const SkImageInfo&,
                                                          const SkIPoint&) {
    return nullptr;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////

SkData* SkImageGenerator::onRefEncodedData(GrContext* ctx) {
    return nullptr;
}

bool SkImageGenerator::onGetPixels(const SkImageInfo& info, void* dst, size_t rb,
                                   SkPMColor* colors, int* colorCount) {
    return false;
}

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
