/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkNextID.h"
#include "SkYUVAIndex.h"

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

bool SkImageGenerator::queryYUVA8(SkYUVASizeInfo* sizeInfo,
                                  SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                  SkYUVColorSpace* colorSpace) const {
    SkASSERT(sizeInfo);

    return this->onQueryYUVA8(sizeInfo, yuvaIndices, colorSpace);
}

bool SkImageGenerator::getYUVA8Planes(const SkYUVASizeInfo& sizeInfo,
                                      const SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                      void* planes[SkYUVASizeInfo::kMaxCount]) {

    for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
        SkASSERT(sizeInfo.fSizes[i].fWidth >= 0);
        SkASSERT(sizeInfo.fSizes[i].fHeight >= 0);
        SkASSERT(sizeInfo.fWidthBytes[i] >= (size_t) sizeInfo.fSizes[i].fWidth);
    }

    int numPlanes = 0;
    SkASSERT(SkYUVAIndex::AreValidIndices(yuvaIndices, &numPlanes));
    SkASSERT(planes);
    for (int i = 0; i < numPlanes; ++i) {
        SkASSERT(planes[i]);
    }

    return this->onGetYUVA8Planes(sizeInfo, yuvaIndices, planes);
}

#if SK_SUPPORT_GPU
#include "GrTextureProxy.h"

sk_sp<GrTextureProxy> SkImageGenerator::generateTexture(GrRecordingContext* ctx,
                                                        const SkImageInfo& info,
                                                        const SkIPoint& origin,
                                                        bool willNeedMipMaps) {
    SkIRect srcRect = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height());
    if (!SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(srcRect)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, info, origin, willNeedMipMaps);
}

sk_sp<GrTextureProxy> SkImageGenerator::onGenerateTexture(GrRecordingContext*,
                                                          const SkImageInfo&,
                                                          const SkIPoint&,
                                                          bool willNeedMipMaps) {
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
