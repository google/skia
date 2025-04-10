/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/AtlasTypes.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkSwizzlePriv.h"

namespace skgpu {

Plot::Plot(int pageIndex, int plotIndex, AtlasGenerationCounter* generationCounter,
           int offX, int offY, int width, int height, SkColorType colorType, size_t bpp)
        : fLastUpload(AtlasToken::InvalidToken())
        , fLastUse(AtlasToken::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fPageIndex(pageIndex)
        , fPlotIndex(plotIndex)
        , fGenerationCounter(generationCounter)
        , fGenID(fGenerationCounter->next())
        , fPlotLocator(fPageIndex, fPlotIndex, fGenID)
        , fData(nullptr)
        , fWidth(width)
        , fHeight(height)
        , fX(offX)
        , fY(offY)
        , fRectanizer(width, height)
        , fOffset(SkIPoint16::Make(fX * fWidth, fY * fHeight))
        , fColorType(colorType)
        , fBytesPerPixel(bpp)
        , fIsFull(false)
#ifdef SK_DEBUG
        , fDirty(false)
#endif
{
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT(((width*fBytesPerPixel) & 0x3) == 0);
    // The padding for faster uploads only works for 1, 2 and 4 byte texels
    SkASSERT(fBytesPerPixel != 3 && fBytesPerPixel <= 4);
    fDirtyRect.setEmpty();
}

Plot::~Plot() {
    sk_free(fData);
}

bool Plot::addRect(int width, int height, AtlasLocator* atlasLocator) {
    SkASSERT(width <= fWidth && height <= fHeight);

    SkIPoint16 loc;
    if (!fRectanizer.addRect(width, height, &loc)) {
        return false;
    }

    auto rect = skgpu::IRect16::MakeXYWH(loc.fX, loc.fY, width, height);
    fDirtyRect.join({rect.fLeft, rect.fTop, rect.fRight, rect.fBottom});

    rect.offset(fOffset.fX, fOffset.fY);
    atlasLocator->updateRect(rect);
    SkDEBUGCODE(fDirty = true;)

    return true;
}

void* Plot::dataAt(const AtlasLocator& atlasLocator) {
    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(
                        sk_calloc_throw(fBytesPerPixel * fWidth * fHeight));
    }
    // point ourselves at the right starting spot
    unsigned char* dataPtr = fData;
    SkIPoint topLeft = atlasLocator.topLeft();
    // Assert if we're not accessing the correct Plot
    SkASSERT(topLeft.fX >= fOffset.fX && topLeft.fX < fOffset.fX + fWidth &&
             topLeft.fY >= fOffset.fY && topLeft.fY < fOffset.fY + fHeight);
    topLeft -= SkIPoint::Make(fOffset.fX, fOffset.fY);
    dataPtr += fBytesPerPixel * fWidth * topLeft.fY;
    dataPtr += fBytesPerPixel * topLeft.fX;

    return dataPtr;
}

SkIPoint Plot::prepForRender(const AtlasLocator& al, SkAutoPixmapStorage* pixmap) {
    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(
                        sk_calloc_throw(fBytesPerPixel * fWidth * fHeight));
    }
    pixmap->reset(SkImageInfo::Make(fWidth, fHeight, fColorType, kOpaque_SkAlphaType),
                  fData, fBytesPerPixel * fWidth);
    return al.topLeft() - SkIPoint::Make(fOffset.fX, fOffset.fY);
}

void Plot::copySubImage(const AtlasLocator& al, const void* image) {
    const unsigned char* imagePtr = (const unsigned char*)image;
    unsigned char* dataPtr = (unsigned char*)this->dataAt(al);
    int width = al.width();
    int height = al.height();
    size_t rowBytes = width * fBytesPerPixel;

    // copy into the data buffer, swizzling as we go if this is ARGB data
    constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
    if (4 == fBytesPerPixel && kBGRAIsNative) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA((uint32_t*)dataPtr, (const uint32_t*)imagePtr, width);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, rowBytes);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    }
}

bool Plot::addSubImage(int width, int height, const void* image, AtlasLocator* atlasLocator) {
    if (fIsFull || !this->addRect(width, height, atlasLocator)) {
        return false;
    }
    this->copySubImage(*atlasLocator, image);

    return true;
}

std::pair<const void*, SkIRect> Plot::prepareForUpload() {
    // We should only be issuing uploads if we are dirty
    SkASSERT(fDirty);
    if (!fData) {
        return {nullptr, {}};
    }
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr;
    SkIRect offsetRect;
    // Clamp to 4-byte aligned boundaries
    unsigned int clearBits = 0x3 / fBytesPerPixel;
    fDirtyRect.fLeft &= ~clearBits;
    fDirtyRect.fRight += clearBits;
    fDirtyRect.fRight &= ~clearBits;
    SkASSERT(fDirtyRect.fRight <= fWidth);
    // Set up dataPtr
    dataPtr = fData;
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    offsetRect = fDirtyRect.makeOffset(fOffset.fX, fOffset.fY);

    fDirtyRect.setEmpty();
    fIsFull = false;
    SkDEBUGCODE(fDirty = false);

    return { dataPtr, offsetRect };
}

void Plot::resetRects() {
    fRectanizer.reset();
    fGenID = fGenerationCounter->next();
    fPlotLocator = PlotLocator(fPageIndex, fPlotIndex, fGenID);
    fLastUpload = AtlasToken::InvalidToken();
    fLastUse = AtlasToken::InvalidToken();

    // zero out the plot
    if (fData) {
        sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
    }

    fDirtyRect.setEmpty();
    fIsFull = false;
    SkDEBUGCODE(fDirty = false;)
}

} // namespace skgpu
