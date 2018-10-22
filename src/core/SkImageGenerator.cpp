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

bool SkImageGenerator::queryYUVA8(SkYUVSizeInfo* sizeInfo,
                                  SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                  SkYUVColorSpace* colorSpace) const {
    SkASSERT(sizeInfo);

    if (!this->onQueryYUVA8(sizeInfo, yuvaIndices, colorSpace)) {
        // try the deprecated method and make a guess at the other data
        if (this->onQueryYUV8(sizeInfo, colorSpace)) {
            // take a guess at the number of planes
            int numPlanes = 3;  // onQueryYUV8 only supports up to 3 channels
            for (int i = 0; i < 3; ++i) {
                if (sizeInfo->fSizes[i].isEmpty()) {
                    numPlanes = i;
                    break;
                }
            }
            if (!numPlanes) {
                return false;
            }
            switch (numPlanes) {
                case 1:
                    // Assume 3 interleaved planes
                    sizeInfo->fColorTypes[0] = kRGBA_8888_SkColorType;
                    sizeInfo->fColorTypes[1] = kUnknown_SkColorType;
                    sizeInfo->fColorTypes[2] = kUnknown_SkColorType;
                    sizeInfo->fColorTypes[3] = kUnknown_SkColorType;
                    yuvaIndices[SkYUVAIndex::kY_Index].fIndex = 0;
                    yuvaIndices[SkYUVAIndex::kY_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kU_Index].fIndex = 0;
                    yuvaIndices[SkYUVAIndex::kU_Index].fChannel = SkColorChannel::kG;
                    yuvaIndices[SkYUVAIndex::kV_Index].fIndex = 0;
                    yuvaIndices[SkYUVAIndex::kV_Index].fChannel = SkColorChannel::kB;
                    yuvaIndices[SkYUVAIndex::kA_Index].fIndex = -1;
                    yuvaIndices[SkYUVAIndex::kA_Index].fChannel = SkColorChannel::kR;
                    break;
                case 2:
                    // Assume 1 Y plane and interleaved UV planes (NV12)
                    sizeInfo->fColorTypes[0] = kAlpha_8_SkColorType;
                    sizeInfo->fColorTypes[1] = kRGBA_8888_SkColorType;
                    sizeInfo->fColorTypes[2] = kUnknown_SkColorType;
                    sizeInfo->fColorTypes[3] = kUnknown_SkColorType;
                    yuvaIndices[SkYUVAIndex::kY_Index].fIndex = 0;
                    yuvaIndices[SkYUVAIndex::kY_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kU_Index].fIndex = 1;
                    yuvaIndices[SkYUVAIndex::kU_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kV_Index].fIndex = 1;
                    yuvaIndices[SkYUVAIndex::kV_Index].fChannel = SkColorChannel::kG;
                    yuvaIndices[SkYUVAIndex::kA_Index].fIndex = -1;
                    yuvaIndices[SkYUVAIndex::kA_Index].fChannel = SkColorChannel::kR;
                    break;
                case 3:
                default:
                    // Assume 3 separate non-interleaved planes
                    sizeInfo->fColorTypes[0] = kAlpha_8_SkColorType;
                    sizeInfo->fColorTypes[1] = kAlpha_8_SkColorType;
                    sizeInfo->fColorTypes[2] = kAlpha_8_SkColorType;
                    sizeInfo->fColorTypes[3] = kUnknown_SkColorType;
                    yuvaIndices[SkYUVAIndex::kY_Index].fIndex = 0;
                    yuvaIndices[SkYUVAIndex::kY_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kU_Index].fIndex = 1;
                    yuvaIndices[SkYUVAIndex::kU_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kV_Index].fIndex = 2;
                    yuvaIndices[SkYUVAIndex::kV_Index].fChannel = SkColorChannel::kR;
                    yuvaIndices[SkYUVAIndex::kA_Index].fIndex = -1;
                    yuvaIndices[SkYUVAIndex::kA_Index].fChannel = SkColorChannel::kR;
                    break;
            }

            return true;
        }

        return false;
    }

    return true;
}

bool SkImageGenerator::getYUVA8Planes(const SkYUVSizeInfo& sizeInfo,
                                      const SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount],
                                      void* planes[SkYUVSizeInfo::kMaxCount]) {

    for (int i = 0; i < SkYUVSizeInfo::kMaxCount; ++i) {
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

    if (!this->onGetYUVA8Planes(sizeInfo, yuvaIndices, planes)) {
        return this->onGetYUV8Planes(sizeInfo, planes);
    }
    return true;
}

#if SK_SUPPORT_GPU
#include "GrTextureProxy.h"

sk_sp<GrTextureProxy> SkImageGenerator::generateTexture(GrContext* ctx, const SkImageInfo& info,
                                                        const SkIPoint& origin,
                                                        bool willNeedMipMaps) {
    SkIRect srcRect = SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height());
    if (!SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(srcRect)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, info, origin, willNeedMipMaps);
}

sk_sp<GrTextureProxy> SkImageGenerator::onGenerateTexture(GrContext*, const SkImageInfo&,
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
