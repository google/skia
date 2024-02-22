/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

RasterPathAtlas::RasterPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {

    // set up LRU lists
    fPageArray = std::make_unique<std::unique_ptr<Page>[]>(kMaxCachedPages+kMaxUncachedPages);
    std::unique_ptr<Page>* currPage = &fPageArray[0];
    for (int i = 0; i < kMaxCachedPages; ++i) {
        *currPage = std::make_unique<Page>(this->width(), this->height(), i);
        fCachedPageList.addToHead(currPage->get());
        ++currPage;
    }
    for (int i = 0; i < kMaxUncachedPages; ++i) {
        *currPage = std::make_unique<Page>(this->width(), this->height(), kMaxCachedPages+i);
        fUncachedPageList.addToHead(currPage->get());
        ++currPage;
    }
}

void RasterPathAtlas::recordUploads(DrawContext* dc) {
    this->uploadPages(dc, &fCachedPageList);
    this->uploadPages(dc, &fUncachedPageList);
}

void RasterPathAtlas::uploadPages(DrawContext* dc, PageList* pageList) {
    // Cycle through all the pages and handle their uploads
    PageList::Iter pageIter;
    pageIter.init(*pageList, PageList::Iter::kHead_IterStart);
    while (Page* currPage = pageIter.get()) {
        // build an upload for the dirty rect and record it
        if (!currPage->fDirtyRect.isEmpty()) {
            // Clamp to 4-byte aligned boundaries
            size_t bpp = currPage->fPixels.info().bytesPerPixel();
            unsigned int clearBits = 0x3 / bpp;
            currPage->fDirtyRect.fLeft &= ~clearBits;
            currPage->fDirtyRect.fRight += clearBits;
            currPage->fDirtyRect.fRight &= ~clearBits;
            SkASSERT(currPage->fDirtyRect.fRight <= (int)this->width());
            // Set up dataPtr
            size_t rowBytes = currPage->fPixels.rowBytes();
            const uint8_t* dataPtr = (const uint8_t*) currPage->fPixels.addr();
            dataPtr += rowBytes * currPage->fDirtyRect.fTop;
            dataPtr += bpp * currPage->fDirtyRect.fLeft;

            std::vector<MipLevel> levels;
            levels.push_back({dataPtr, rowBytes});

            SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

            if (!dc->recordUpload(fRecorder, currPage->fTexture, colorInfo, colorInfo, levels,
                                  currPage->fDirtyRect, nullptr)) {
                SKGPU_LOG_W("Coverage mask upload failed!");
                return;
            }

            currPage->fDirtyRect.setEmpty();
        }
        pageIter.next();
    }
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

void RasterPathAtlas::makeMRU(Page* page, PageList* pageList) {
    page->fLastUse = fRecorder->priv().tokenTracker()->nextFlushToken();

    if (pageList->head() == page) {
        return;
    }
    pageList->remove(page);
    pageList->addToHead(page);
}

RasterPathAtlas::Page* RasterPathAtlas::addRect(PageList* pageList,
                                                skvx::half2 maskSize,
                                                SkIPoint16* outPos) {
    // Look through all pages in MRU order and find the first one with room, and move that to MRU
    PageList::Iter pageIter;
    pageIter.init(*pageList, PageList::Iter::kHead_IterStart);
    for (Page* currPage = pageIter.get(); currPage; currPage = pageIter.next()) {
        if (!currPage->initializeTextureIfNeeded(fRecorder, currPage->fIdentifier)) {
            SKGPU_LOG_E("Failed to instantiate an atlas texture");
            return nullptr;
        }

        // An empty mask always fits, so just return the texture.
        // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
        // another way. See PathAtlas::addShape().
        if (!all(maskSize)) {
            *outPos = {0, 0};
            return currPage;
        }

        if (!currPage->fRectanizer.addPaddedRect(
                maskSize.x(), maskSize.y(), kEntryPadding, outPos)) {
            continue;
        }

        this->makeMRU(currPage, pageList);
        return currPage;
    }

    // If the above fails, then see if the least recently used page has already been
    // queued for upload, in which case we can reuse its space w/o corrupting prior atlas draws.
    Page* lru = pageList->tail();
    SkASSERT(lru);
    if (lru->fLastUse < fRecorder->priv().tokenTracker()->nextFlushToken()) {
        this->reset(lru);
        SkAssertResult(lru->fRectanizer.addPaddedRect(
                maskSize.x(), maskSize.y(), kEntryPadding, outPos));
        this->makeMRU(lru, pageList);
        return lru;
    }

    // No room in any Page
    return nullptr;
}

const TextureProxy* RasterPathAtlas::onAddShape(const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskSize,
                                                skvx::half2* outPos) {
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        // Iterate through pagelist in MRU order and see if this shape is cached
        PageList::Iter pageIter;
        pageIter.init(fCachedPageList, PageList::Iter::kHead_IterStart);
        maskKey = GeneratePathMaskKey(shape, transform, strokeRec, maskSize);
        while (Page* currPage = pageIter.get()) {
            // Look up shape and use cached texture and position if found.
            skvx::half2* found = currPage->fCachedShapes.find(maskKey);
            if (found) {
                *outPos = *found;
                this->makeMRU(currPage, &fCachedPageList);
                return currPage->fTexture.get();
            }
            pageIter.next();
        }
    }

    // Try to add to Rectanizer
    SkIPoint16 iPos;
    Page* maskPage = nullptr;
    if (hasKey) {
        maskPage = this->addRect(&fCachedPageList, maskSize, &iPos);
        // No room in the cached pages, try the uncached and don't worry about caching
        if (!maskPage) {
            maskPage = this->addRect(&fUncachedPageList, maskSize, &iPos);
            hasKey = false;
        }
    } else {
        maskPage = this->addRect(&fUncachedPageList, maskSize, &iPos);
    }
    if (!maskPage || !maskPage->fTexture) {
        return nullptr;
    }
    *outPos = skvx::half2(iPos.x(), iPos.y());
    // If the mask is empty, just return.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        return maskPage->fTexture.get();
    }

    // Handle render

    // Rasterize path to backing pixmap
    RasterMaskHelper helper(&maskPage->fPixels);
    if (!helper.init({maskPage->fRectanizer.width(), maskPage->fRectanizer.height()})) {
        return nullptr;
    }
    SkIRect iAtlasBounds = SkIRect::MakeXYWH(iPos.x(), iPos.y(), maskSize.x(), maskSize.y());
    helper.drawShape(shape, transform, strokeRec, iAtlasBounds);

    // Add atlasBounds to dirtyRect for later upload, including the 1px padding applied by the
    // rectanizer. If we didn't include this then our uploads would not include writes to the
    // padded border, so the GPU texture might not then contain transparency even though the CPU
    // data was cleared properly.
    maskPage->fDirtyRect.join(iAtlasBounds.makeOutset(kEntryPadding, kEntryPadding));

    // Add to cache
    if (hasKey) {
        maskPage->fCachedShapes.set(maskKey, *outPos);
    }

    return maskPage->fTexture.get();
}

void RasterPathAtlas::reset(Page* lru) {
    lru->fRectanizer.reset();

    // clear backing data for next pass
    SkASSERT(lru->fDirtyRect.isEmpty());
    lru->fPixels.erase(0);
    lru->fCachedShapes.reset();
}

}  // namespace skgpu::graphite
