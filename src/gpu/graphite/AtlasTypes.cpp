/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasTypes.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkSwizzlePriv.h"

namespace skgpu::graphite {

Plot::Plot(int pageIndex,
           int plotIndex,
           AtlasGenerationCounter* generationCounter,
           int offX, int offY,
           int width, int height,
           MaskFormat maskFormat)
        : fLastUse(Token::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fGenerationCounter(generationCounter)
        , fGenID(fGenerationCounter->next())
        , fPlotLocator(pageIndex, plotIndex, fGenID)
        , fData(nullptr)
        , fWidth(width)
        , fHeight(height)
        , fX(offX)
        , fY(offY)
        , fRectanizer(width, height)
        , fOffset(SkIPoint16::Make(fX * fWidth, fY * fHeight))
        , fMaskFormat(maskFormat)
        , fIsFull(false) {
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT(((width * this->bpp()) & 0x3) == 0);
    // The padding for faster uploads only works for 1, 2 and 4 byte texels
    SkASSERT(this->bpp() == 1 || this->bpp() == 2 || this->bpp() == 4);
    fDirtyRect.setEmpty();
}

Plot::~Plot() = default;

bool Plot::addRect(int width, int height, AtlasLocator* atlasLocator) {
    SkASSERT(width <= fWidth && height <= fHeight);

    SkIPoint16 loc;
    if (!fRectanizer.addRect(width, height, &loc)) {
        return false;
    }

    auto rect = SkIRect::MakeXYWH(loc.fX, loc.fY, width, height);
    fDirtyRect.join(rect);

    rect.offset(fOffset.fX, fOffset.fY);
    atlasLocator->updateRect(rect);

    return true;
}

void* Plot::dataAt(SkIPoint atlasPoint) {
    if (!fData) {
        // make_unique will init the data to zeros.
        // This is of particular importance when a caller uses padding with prepForRender().
        fData = std::make_unique<std::byte[]>(this->rowBytes() * fHeight);
    }

    auto localPoint = atlasPoint - SkIPoint{fOffset.fX, fOffset.fY};
    SkASSERT(localPoint.fX >= 0 && localPoint.fX < fWidth);
    SkASSERT(localPoint.fY >= 0 && localPoint.fY < fHeight);

    size_t offset = this->bpp() * (localPoint.fY * fWidth + localPoint.fX);

    return fData.get() + offset;
}

SkPixmap Plot::prepForRender(const AtlasLocator& al,
                             int padding,
                             std::optional<SkColor> initialColor) {
    SkASSERT(padding >= 0);
    auto info = SkImageInfo::Make(al.dimensions(),
                                  MaskFormatToColorType(fMaskFormat),
                                  kOpaque_SkAlphaType);
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
    unsigned char* dataPtr = (unsigned char*)this->dataAt(al.topLeft());
    int width = al.width();
    int height = al.height();
    auto bpp = this->bpp();
    size_t imageRB = width * bpp;
    size_t plotRB = this->rowBytes();

    // copy into the data buffer, swizzling as we go if this is ARGB data
    constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
    if (bpp == 4 && kBGRAIsNative) {
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

std::pair<const void*, SkIRect> Plot::prepareForUpload() {
    // We should only be issuing uploads if we are dirty
    SkASSERT(!fDirtyRect.isEmpty());
    if (!fData) {
        return {nullptr, {}};
    }
    const std::byte* dataPtr;
    SkIRect offsetRect;
    // Clamp to 4-byte aligned boundaries
    auto bpp = this->bpp();
    unsigned int clearBits = 0x3 / bpp;
    fDirtyRect.fLeft &= ~clearBits;
    fDirtyRect.fRight += clearBits;
    fDirtyRect.fRight &= ~clearBits;
    SkASSERT(fDirtyRect.fRight <= fWidth);
    // Set up dataPtr
    dataPtr = fData.get();
    dataPtr += this->rowBytes() * fDirtyRect.fTop;
    dataPtr += bpp * fDirtyRect.fLeft;
    offsetRect = fDirtyRect.makeOffset(fOffset.fX, fOffset.fY);

    fDirtyRect.setEmpty();
    fIsFull = false;

    return {dataPtr, offsetRect};
}

void Plot::resetRects(bool freeData) {
    fRectanizer.reset();
    fGenID = fGenerationCounter->next();
    auto pageIndex = fPlotLocator.pageIndex();
    auto plotIndex = fPlotLocator.plotIndex();
    fPlotLocator = PlotLocator(pageIndex, plotIndex, fGenID);
    fLastUse = Token::InvalidToken();

    if (freeData) {
        fData = {};
    } else if (fData) {
        // zero out the plot
        sk_bzero(fData.get(), this->rowBytes() * fHeight);
    }

    fDirtyRect.setEmpty();
    fIsFull = false;
}

}  // namespace skgpu::graphite
