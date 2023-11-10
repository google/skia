/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

RasterPathAtlas::RasterPathAtlas()
    : PathAtlas(kDefaultAtlasDim, kDefaultAtlasDim) {

    // set up LRU list
    Page* currPage = &fPageArray[0];
    for (int i = 0; i < kMaxPages; ++i) {
        fPageList.addToHead(currPage);
        currPage->fIdentifier = i;
        ++currPage;
    }
}

void RasterPathAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    // Cycle through all the pages and handle their uploads
    PageList::Iter pageIter;
    pageIter.init(fPageList, PageList::Iter::kHead_IterStart);
    while (Page* currPage = pageIter.get()) {
        // build an upload for the dirty rect and record it
        if (!currPage->fDirtyRect.isEmpty()) {
            size_t rowBytes = currPage->fPixels.rowBytes();
            const uint8_t* dataPtr = (const uint8_t*) currPage->fPixels.addr();
            dataPtr += rowBytes * currPage->fDirtyRect.fTop;
            dataPtr += currPage->fPixels.info().bytesPerPixel() * currPage->fDirtyRect.fLeft;

            std::vector<MipLevel> levels;
            levels.push_back({dataPtr, rowBytes});

            SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

            if (!dc->recordUpload(recorder, currPage->fTexture, colorInfo, colorInfo, levels,
                                  currPage->fDirtyRect, nullptr)) {
                SKGPU_LOG_W("Coverage mask upload failed!");
                return;
            }

            currPage->fDirtyRect.setEmpty();
        }
        pageIter.next();
    }

    this->reset();
}

bool RasterPathAtlas::Page::initializeTextureIfNeeded(Recorder* recorder, uint16_t identifier) {
    if (!fTexture) {
        AtlasProvider* atlasProvider = recorder->priv().atlasProvider();
        fTexture = atlasProvider->getAtlasTexture(recorder,
                                                  fRectanizer.width(),
                                                  fRectanizer.height(),
                                                  kAlpha_8_SkColorType,
                                                  identifier,
                                                  /*requireStorageUsage=*/false);
    }
    return fTexture != nullptr;
}

void RasterPathAtlas::makeMRU(Page* page) {
    if (fPageList.head() == page) {
        return;
    }

    fPageList.remove(page);
    fPageList.addToHead(page);
}

const TextureProxy* RasterPathAtlas::addRect(Recorder* recorder,
                                             skvx::half2 maskSize,
                                             SkIPoint16* outPos) {
    // Look through all pages in MRU order and find the first one with room, and move that to MRU
    PageList::Iter pageIter;
    pageIter.init(fPageList, PageList::Iter::kHead_IterStart);
    for (Page* currPage = pageIter.get(); currPage; currPage = pageIter.next()) {
        if (!currPage->initializeTextureIfNeeded(recorder, currPage->fIdentifier)) {
            SKGPU_LOG_E("Failed to instantiate an atlas texture");
            return nullptr;
        }

        // An empty mask always fits, so just return the texture.
        // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
        // another way. See PathAtlas::addShape().
        if (!all(maskSize)) {
            *outPos = {0, 0};
            return currPage->fTexture.get();
        }

        if (!currPage->fRectanizer.addPaddedRect(
                maskSize.x(), maskSize.y(), kEntryPadding, outPos)) {
            continue;
        }

        this->makeMRU(currPage);
        return currPage->fTexture.get();
    }

    // No room in any Page
    return nullptr;
}

namespace {
skgpu::UniqueKey generate_key(const Shape& shape,
                              const Transform& transform,
                              const SkStrokeRec& strokeRec,
                              skvx::half2 maskSize) {
    skgpu::UniqueKey maskKey;
    {
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 6 + shape.keySize(),
                                          "Raster Path Mask");
        builder[0] = maskSize.x() | (maskSize.y() << 16);

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkMatrix mat = transform.matrix().asM33();
        SkScalar sx = mat.get(SkMatrix::kMScaleX);
        SkScalar sy = mat.get(SkMatrix::kMScaleY);
        SkScalar kx = mat.get(SkMatrix::kMSkewX);
        SkScalar ky = mat.get(SkMatrix::kMSkewY);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // Fractional translate does not affect caching on Android. This is done for better cache
        // hit ratio and speed and is matching HWUI behavior, which didn't consider the matrix
        // at all when caching paths.
        SkFixed fracX = 0;
        SkFixed fracY = 0;
#else
        SkScalar tx = mat.get(SkMatrix::kMTransX);
        SkScalar ty = mat.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
#endif
        builder[1] = SkFloat2Bits(sx);
        builder[2] = SkFloat2Bits(sy);
        builder[3] = SkFloat2Bits(kx);
        builder[4] = SkFloat2Bits(ky);
        // FracX and fracY are &ed with 0x0000ff00, so need to shift one down to fill 16 bits.
        uint32_t fracBits = fracX | (fracY >> 8);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = strokeRec.isHairlineStyle() ? ((strokeRec.getCap() << 1) | 1) : 0;
        builder[5] = fracBits | (styleBits << 16);
        shape.writeKey(&builder[6], /*includeInverted=*/false);
    }
    return maskKey;
}
} // namespace

const TextureProxy* RasterPathAtlas::onAddShape(Recorder* recorder,
                                                const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskSize,
                                                skvx::half2* outPos) {
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        // Iterate through pagelist in MRU order and see if this shape is cached
        PageList::Iter pageIter;
        pageIter.init(fPageList, PageList::Iter::kHead_IterStart);
        maskKey = generate_key(shape, transform, strokeRec, maskSize);
        while (Page* currPage = pageIter.get()) {
            // Look up shape and use cached texture and position if found.
            skvx::half2* found = currPage->fCachedShapes.find(maskKey);
            if (found) {
                *outPos = *found;
                this->makeMRU(currPage);
                return currPage->fTexture.get();
            }
            pageIter.next();
        }
    }

    // Try to add to Rectanizer
    SkIPoint16 iPos;
    const TextureProxy* texProxy = this->addRect(recorder, maskSize, &iPos);
    if (!texProxy) {
        // Reset LRU Page
        fPageList.tail()->fNeedsReset = true;
        return nullptr;
    }
    *outPos = skvx::half2(iPos.x(), iPos.y());
    // If the mask is empty, just return.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        return texProxy;
    }

    // Handle render
    Page* mru = fPageList.head();  // set up by addRect()
    SkASSERT(mru->fTexture.get() == texProxy);

    // Allocate pixmap if needed
    if (!mru->fPixels.addr()) {
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(mru->fRectanizer.width(),
                                                            mru->fRectanizer.height());
        if (!mru->fPixels.tryAlloc(bmImageInfo)) {
            return nullptr;
        }
        mru->fPixels.erase(0);
    }

    // Rasterize path to backing pixmap
    // TODO: render in a separate thread?
    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst      = mru->fPixels;
    SkRasterClip rasterClip;
    SkIRect iAtlasBounds = SkIRect::MakeXYWH(iPos.x(), iPos.y(), maskSize.x(), maskSize.y());
    rasterClip.setRect(iAtlasBounds);
    draw.fRC       = &rasterClip;

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SK_ColorWHITE);
    strokeRec.applyToPaint(&paint);

    SkMatrix translatedMatrix = SkMatrix(transform);
    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds.
    translatedMatrix.postTranslate(iAtlasBounds.x(), iAtlasBounds.y());

    draw.fCTM = &translatedMatrix;
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    draw.drawPathCoverage(path, paint);

    // Add atlasBounds to dirtyRect for later upload, including the 1px padding applied by the
    // rectanizer. If we didn't include this then our uploads would not include writes to the
    // padded border, so the GPU texture might not then contain transparency even though the CPU
    // data was cleared properly.
    mru->fDirtyRect.join(iAtlasBounds.makeOutset(kEntryPadding,kEntryPadding));

    // Add to cache
    if (hasKey) {
        mru->fCachedShapes.set(maskKey, *outPos);
    }

    return texProxy;
}

void RasterPathAtlas::reset() {
    // Only reset LRU Page if needed
    Page* lru = fPageList.tail();
    SkASSERT(lru);
    if (lru->fNeedsReset) {
        lru->fRectanizer.reset();

        // clear backing data for next pass
        SkASSERT(lru->fDirtyRect.isEmpty());
        lru->fPixels.erase(0);
        lru->fCachedShapes.reset();
        lru->fNeedsReset = false;
    }
}

}  // namespace skgpu::graphite
