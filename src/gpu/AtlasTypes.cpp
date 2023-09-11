/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/AtlasTypes.h"

#include "include/private/base/SkMalloc.h"
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
#ifdef SK_DEBUG
        , fDirty(false)
#endif
{
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT(((width*fBytesPerPixel) & 0x3) == 0);
    // The padding for faster uploads only works for 1, 2 and 4 byte texels
    SkASSERT(fBytesPerPixel != 3 && fBytesPerPixel <= 4);
    fDirtyRect.setEmpty();
    fCachedRect.setEmpty();
}

Plot::~Plot() {
    sk_free(fData);
}

bool Plot::addSubImage(int width, int height, const void* image, AtlasLocator* atlasLocator) {
    SkASSERT(width <= fWidth && height <= fHeight);

    SkIPoint16 loc;
    if (!fRectanizer.addRect(width, height, &loc)) {
        return false;
    }

    auto rect = skgpu::IRect16::MakeXYWH(loc.fX, loc.fY, width, height);

    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(
                sk_calloc_throw(fBytesPerPixel * fWidth * fHeight));
    }
    size_t rowBytes = width * fBytesPerPixel;
    const unsigned char* imagePtr = (const unsigned char*)image;
    // point ourselves at the right starting spot
    unsigned char* dataPtr = fData;
    dataPtr += fBytesPerPixel * fWidth * rect.fTop;
    dataPtr += fBytesPerPixel * rect.fLeft;
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

    fDirtyRect.join({rect.fLeft, rect.fTop, rect.fRight, rect.fBottom});

    rect.offset(fOffset.fX, fOffset.fY);
    atlasLocator->updateRect(rect);
    SkDEBUGCODE(fDirty = true;)

    return true;
}

std::pair<const void*, SkIRect> Plot::prepareForUpload(bool useCachedUploads) {
    // We should only be issuing uploads if we are dirty or uploading the cached rect
    SkASSERT(fDirty || useCachedUploads);
    if (!fData) {
        return {nullptr, {}};
    }
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr;
    SkIRect offsetRect;
    if (!fDirtyRect.isEmpty()) {
        // Clamp to 4-byte aligned boundaries
        unsigned int clearBits = 0x3 / fBytesPerPixel;
        fDirtyRect.fLeft &= ~clearBits;
        fDirtyRect.fRight += clearBits;
        fDirtyRect.fRight &= ~clearBits;
        SkASSERT(fDirtyRect.fRight <= fWidth);
        if (!useCachedUploads) {
            // Set up dataPtr
            dataPtr = fData;
            dataPtr += rowBytes * fDirtyRect.fTop;
            dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
            offsetRect = fDirtyRect.makeOffset(fOffset.fX, fOffset.fY);
        }
        fCachedRect.join(fDirtyRect);
        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false);
    }

    if (useCachedUploads) {
        // use the entire cached rect rather than just the dirty rect
        dataPtr = fData;
        dataPtr += rowBytes * fCachedRect.fTop;
        dataPtr += fBytesPerPixel * fCachedRect.fLeft;
        offsetRect = fCachedRect.makeOffset(fOffset.fX, fOffset.fY);
    }

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
    fCachedRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

} // namespace skgpu
