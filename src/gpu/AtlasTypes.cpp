/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/AtlasTypes.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkSwizzlePriv.h"

namespace skgpu {

Plot::Plot(int pageIndex, int plotIndex, AtlasGenerationCounter* generationCounter,
           int offX, int offY, int width, int height, SkColorType colorType, size_t bpp)
        : fLastUpload(Token::InvalidToken())
        , fLastUse(Token::InvalidToken())
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

void* Plot::dataAt(SkIPoint atlasPoint) {
    if (!fData) {
        // We use calloc here because our contract is that all pixel data is initially zero.
        // This is of particular importance when a caller uses padding with prepForRender().
        fData = reinterpret_cast<std::byte*>(sk_calloc_throw(this->rowBytes() * fHeight));
    }

    auto localPoint = atlasPoint - SkIPoint{fOffset.fX, fOffset.fY};
    SkASSERT(localPoint.fX >= 0 && localPoint.fX < fWidth);
    SkASSERT(localPoint.fY >= 0 && localPoint.fY < fHeight);

    size_t offset = fBytesPerPixel * (localPoint.fY * fWidth + localPoint.fX);

    return fData + offset;
}

void* Plot::dataAt(const AtlasLocator& atlasLocator) { return dataAt(atlasLocator.topLeft()); }

SkPixmap Plot::prepForRender(const AtlasLocator& al,
                             int padding,
                             std::optional<SkColor> initialColor) {
    SkASSERT(padding >= 0);
    auto info = SkImageInfo::Make(al.width(), al.height(), fColorType, kOpaque_SkAlphaType);
    SkPixmap outerPM{info, this->dataAt(al.topLeft()), this->rowBytes()};
    if (initialColor) {
#if defined(SK_DEBUG)
        if (*initialColor == 0) {
            SkDebugf("Plot Data: potential redudant clear of Plot to zero.");
        }
#endif
        outerPM.erase(*initialColor);
    }
    SkPixmap innerPM;
    SkIRect rect = SkIRect::MakeSize(outerPM.dimensions()).makeInset(padding, padding);
    SkAssertResult(outerPM.extractSubset(&innerPM, rect));
    return innerPM;
}

void Plot::copySubImage(const AtlasLocator& al, const void* image) {
    const unsigned char* imagePtr = (const unsigned char*)image;
    unsigned char* dataPtr = (unsigned char*)this->dataAt(al);
    int width = al.width();
    int height = al.height();
    size_t imageRB = width * fBytesPerPixel;
    size_t plotRB = this->rowBytes();

    // copy into the data buffer, swizzling as we go if this is ARGB data
    constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
    if (4 == fBytesPerPixel && kBGRAIsNative) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA((uint32_t*)dataPtr, (const uint32_t*)imagePtr, width);
            dataPtr += plotRB;
            imagePtr += imageRB;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, imageRB);
            dataPtr += plotRB;
            imagePtr += imageRB;
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
    const std::byte* dataPtr;
    SkIRect offsetRect;
    // Clamp to 4-byte aligned boundaries
    unsigned int clearBits = 0x3 / fBytesPerPixel;
    fDirtyRect.fLeft &= ~clearBits;
    fDirtyRect.fRight += clearBits;
    fDirtyRect.fRight &= ~clearBits;
    SkASSERT(fDirtyRect.fRight <= fWidth);
    // Set up dataPtr
    dataPtr = fData;
    dataPtr += this->rowBytes() * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    offsetRect = fDirtyRect.makeOffset(fOffset.fX, fOffset.fY);

    fDirtyRect.setEmpty();
    fIsFull = false;
    SkDEBUGCODE(fDirty = false);

    return { dataPtr, offsetRect };
}

void Plot::resetRects(bool freeData) {
    fRectanizer.reset();
    fGenID = fGenerationCounter->next();
    fPlotLocator = PlotLocator(fPageIndex, fPlotIndex, fGenID);
    fLastUpload = Token::InvalidToken();
    fLastUse = Token::InvalidToken();

    if (freeData) {
        sk_free(fData);
        fData = nullptr;
    } else if (fData) {
        // zero out the plot
        sk_bzero(fData, this->rowBytes() * fHeight);
    }

    fDirtyRect.setEmpty();
    fIsFull = false;
    SkDEBUGCODE(fDirty = false;)
}

} // namespace skgpu
