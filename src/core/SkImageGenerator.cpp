/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageGenerator.h"
#include "SkNextID.h"

SkImageGenerator::SkImageGenerator(const SkImageInfo& info)
    : fInfo(info)
    , fUniqueID(SkNextID::ImageID())
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

bool SkImageGenerator::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    SkASSERT(kIndex_8_SkColorType != info.colorType());
    if (kIndex_8_SkColorType == info.colorType()) {
        return false;
    }
    return this->getPixels(info, pixels, rowBytes, nullptr, nullptr);
}

bool SkImageGenerator::getYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                                     SkYUVColorSpace* colorSpace) {
#ifdef SK_DEBUG
    // In all cases, we need the sizes array
    SkASSERT(sizes);

    bool isValidWithPlanes = (planes) && (rowBytes) &&
        ((planes[0]) && (planes[1]) && (planes[2]) &&
         (0  != rowBytes[0]) && (0  != rowBytes[1]) && (0  != rowBytes[2]));
    bool isValidWithoutPlanes =
        ((nullptr == planes) ||
         ((nullptr == planes[0]) && (nullptr == planes[1]) && (nullptr == planes[2]))) &&
        ((nullptr == rowBytes) ||
         ((0 == rowBytes[0]) && (0 == rowBytes[1]) && (0 == rowBytes[2])));

    // Either we have all planes and rowBytes information or we have none of it
    // Having only partial information is not supported
    SkASSERT(isValidWithPlanes || isValidWithoutPlanes);

    // If we do have planes information, make sure all sizes are non 0
    // and all rowBytes are valid
    SkASSERT(!isValidWithPlanes ||
             ((sizes[0].fWidth  >= 0) &&
              (sizes[0].fHeight >= 0) &&
              (sizes[1].fWidth  >= 0) &&
              (sizes[1].fHeight >= 0) &&
              (sizes[2].fWidth  >= 0) &&
              (sizes[2].fHeight >= 0) &&
              (rowBytes[0] >= (size_t)sizes[0].fWidth) &&
              (rowBytes[1] >= (size_t)sizes[1].fWidth) &&
              (rowBytes[2] >= (size_t)sizes[2].fWidth)));
#endif

    return this->onGetYUV8Planes(sizes, planes, rowBytes, colorSpace);
}

bool SkImageGenerator::onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3]) {
    return false;
}

bool SkImageGenerator::onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                                       SkYUVColorSpace* colorSpace) {
    // In order to maintain compatibility with clients that implemented the original
    // onGetYUV8Planes interface, we assume that the color space is JPEG.
    // TODO(rileya): remove this and the old onGetYUV8Planes once clients switch over to
    // the new interface.
    if (colorSpace) {
        *colorSpace = kJPEG_SkYUVColorSpace;
    }
    return this->onGetYUV8Planes(sizes, planes, rowBytes);
}

GrTexture* SkImageGenerator::generateTexture(GrContext* ctx, SkImageUsageType usage,
                                             const SkIRect* subset) {
    if (subset && !SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(*subset)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, usage, subset);
}

/////////////////////////////////////////////////////////////////////////////////////////////

SkData* SkImageGenerator::onRefEncodedData() {
    return nullptr;
}

bool SkImageGenerator::onGetPixels(const SkImageInfo& info, void* dst, size_t rb,
                                   SkPMColor* colors, int* colorCount) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkGraphics.h"

static SkGraphics::ImageGeneratorFromEncodedFactory gFactory;

SkGraphics::ImageGeneratorFromEncodedFactory
SkGraphics::SetImageGeneratorFromEncodedFactory(ImageGeneratorFromEncodedFactory factory)
{
    ImageGeneratorFromEncodedFactory prev = gFactory;
    gFactory = factory;
    return prev;
}

SkImageGenerator* SkImageGenerator::NewFromEncoded(SkData* data) {
    if (nullptr == data) {
        return nullptr;
    }
    if (gFactory) {
        if (SkImageGenerator* generator = gFactory(data)) {
            return generator;
        }
    }
    return SkImageGenerator::NewFromEncodedImpl(data);
}
