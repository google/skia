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

GrTexture* SkImageGenerator::generateTexture(GrContext* ctx, const SkIRect* subset) {
    if (subset && !SkIRect::MakeWH(fInfo.width(), fInfo.height()).contains(*subset)) {
        return nullptr;
    }
    return this->onGenerateTexture(ctx, subset);
}

bool SkImageGenerator::computeScaledDimensions(SkScalar scale, SupportedSizes* sizes) {
    if (scale > 0 && scale <= 1) {
        return this->onComputeScaledDimensions(scale, sizes);
    }
    return false;
}

bool SkImageGenerator::generateScaledPixels(const SkISize& scaledSize,
                                            const SkIPoint& subsetOrigin,
                                            const SkPixmap& subsetPixels) {
    if (scaledSize.width() <= 0 || scaledSize.height() <= 0) {
        return false;
    }
    if (subsetPixels.width() <= 0 || subsetPixels.height() <= 0) {
        return false;
    }
    const SkIRect subset = SkIRect::MakeXYWH(subsetOrigin.x(), subsetOrigin.y(),
                                             subsetPixels.width(), subsetPixels.height());
    if (!SkIRect::MakeWH(scaledSize.width(), scaledSize.height()).contains(subset)) {
        return false;
    }
    return this->onGenerateScaledPixels(scaledSize, subsetOrigin, subsetPixels);
}

/////////////////////////////////////////////////////////////////////////////////////////////

SkData* SkImageGenerator::onRefEncodedData(SK_REFENCODEDDATA_CTXPARAM) {
    return nullptr;
}

bool SkImageGenerator::onGetPixels(const SkImageInfo& info, void* dst, size_t rb,
                                   SkPMColor* colors, int* colorCount) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkBitmap.h"
#include "SkColorTable.h"

static bool reset_and_return_false(SkBitmap* bitmap) {
    bitmap->reset();
    return false;
}

bool SkImageGenerator::tryGenerateBitmap(SkBitmap* bitmap, const SkImageInfo* infoPtr,
                                         SkBitmap::Allocator* allocator) {
    SkImageInfo info = infoPtr ? *infoPtr : this->getInfo();
    if (0 == info.getSafeSize(info.minRowBytes())) {
        return false;
    }
    if (!bitmap->setInfo(info)) {
        return reset_and_return_false(bitmap);
    }

    SkPMColor ctStorage[256];
    memset(ctStorage, 0xFF, sizeof(ctStorage)); // init with opaque-white for the moment
    SkAutoTUnref<SkColorTable> ctable(new SkColorTable(ctStorage, 256));
    if (!bitmap->tryAllocPixels(allocator, ctable)) {
        // SkResourceCache's custom allcator can'thandle ctables, so it may fail on
        // kIndex_8_SkColorTable.
        // https://bug.skia.org/4355
#if 1
        // ignroe the allocator, and see if we can succeed without it
        if (!bitmap->tryAllocPixels(nullptr, ctable)) {
            return reset_and_return_false(bitmap);
        }
#else
        // this is the up-scale technique, not fully debugged, but we keep it here at the moment
        // to remind ourselves that this might be better than ignoring the allocator.

        info = SkImageInfo::MakeN32(info.width(), info.height(), info.alphaType());
        if (!bitmap->setInfo(info)) {
            return reset_and_return_false(bitmap);
        }
        // we pass nullptr for the ctable arg, since we are now explicitly N32
        if (!bitmap->tryAllocPixels(allocator, nullptr)) {
            return reset_and_return_false(bitmap);
        }
#endif
    }

    bitmap->lockPixels();
    if (!bitmap->getPixels()) {
        return reset_and_return_false(bitmap);
    }

    int ctCount = 0;
    if (!this->getPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(),
                         ctStorage, &ctCount)) {
        return reset_and_return_false(bitmap);
    }

    if (ctCount > 0) {
        SkASSERT(kIndex_8_SkColorType == bitmap->colorType());
        // we and bitmap should be owners
        SkASSERT(!ctable->unique());

        // Now we need to overwrite the ctable we built earlier, with the correct colors.
        // This does mean that we may have made the table too big, but that cannot be avoided
        // until we can change SkImageGenerator's API to return us the ctable *before* we have to
        // allocate space for all the pixels.
        ctable->dangerous_overwriteColors(ctStorage, ctCount);
    } else {
        SkASSERT(kIndex_8_SkColorType != bitmap->colorType());
        // we should be the only owner
        SkASSERT(ctable->unique());
    }
    return true;
}

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
