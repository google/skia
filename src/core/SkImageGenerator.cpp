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
