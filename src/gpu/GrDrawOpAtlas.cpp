/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawOpAtlas.h"

#include <memory>

#include "include/private/SkTPin.h"
#include "src/core/SkOpts.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTracing.h"

#ifdef DUMP_ATLAS_DATA
static bool gDumpAtlasData = false;
#endif

#ifdef SK_DEBUG
void GrDrawOpAtlas::validate(const AtlasLocator& atlasLocator) const {
    // Verify that the plotIndex stored in the PlotLocator is consistent with the glyph rectangle
    int numPlotsX = fTextureWidth / fPlotWidth;
    int numPlotsY = fTextureHeight / fPlotHeight;

    int plotIndex = atlasLocator.plotIndex();
    auto topLeft = atlasLocator.topLeft();
    int plotX = topLeft.x() / fPlotWidth;
    int plotY = topLeft.y() / fPlotHeight;
    SkASSERT(plotIndex == (numPlotsY - plotY - 1) * numPlotsX + (numPlotsX - plotX - 1));
}
#endif

// When proxy allocation is deferred until flush time the proxies acting as atlases require
// special handling. This is because the usage that can be determined from the ops themselves
// isn't sufficient. Independent of the ops there will be ASAP and inline uploads to the
// atlases. Extending the usage interval of any op that uses an atlas to the start of the
// flush (as is done for proxies that are used for sw-generated masks) also won't work because
// the atlas persists even beyond the last use in an op - for a given flush. Given this, atlases
// must explicitly manage the lifetime of their backing proxies via the onFlushCallback system
// (which calls this method).
void GrDrawOpAtlas::instantiate(GrOnFlushResourceProvider* onFlushResourceProvider) {
    for (uint32_t i = 0; i < fNumActivePages; ++i) {
        // All the atlas pages are now instantiated at flush time in the activeNewPage method.
        SkASSERT(fViews[i].proxy() && fViews[i].proxy()->isInstantiated());
    }
}

std::unique_ptr<GrDrawOpAtlas> GrDrawOpAtlas::Make(GrProxyProvider* proxyProvider,
                                                   const GrBackendFormat& format,
                                                   GrColorType colorType, int width,
                                                   int height, int plotWidth, int plotHeight,
                                                   GenerationCounter* generationCounter,
                                                   AllowMultitexturing allowMultitexturing,
                                                   EvictionCallback* evictor) {
    if (!format.isValid()) {
        return nullptr;
    }

    std::unique_ptr<GrDrawOpAtlas> atlas(new GrDrawOpAtlas(proxyProvider, format, colorType,
                                                           width, height, plotWidth, plotHeight,
                                                           generationCounter,
                                                           allowMultitexturing));
    if (!atlas->getViews()[0].proxy()) {
        return nullptr;
    }

    if (evictor != nullptr) {
        atlas->fEvictionCallbacks.emplace_back(evictor);
    }
    return atlas;
}

////////////////////////////////////////////////////////////////////////////////
GrDrawOpAtlas::Plot::Plot(int pageIndex, int plotIndex, GenerationCounter* generationCounter,
        int offX, int offY, int width, int height, GrColorType colorType)
        : fLastUpload(GrDeferredUploadToken::AlreadyFlushedToken())
        , fLastUse(GrDeferredUploadToken::AlreadyFlushedToken())
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
        , fBytesPerPixel(GrColorTypeBytesPerPixel(colorType))
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

GrDrawOpAtlas::Plot::~Plot() {
    sk_free(fData);
}

bool GrDrawOpAtlas::Plot::addSubImage(
        int width, int height, const void* image, AtlasLocator* atlasLocator) {
    SkASSERT(width <= fWidth && height <= fHeight);

    SkIPoint16 loc;
    if (!fRectanizer.addRect(width, height, &loc)) {
        return false;
    }

    GrIRect16 rect = GrIRect16::MakeXYWH(loc.fX, loc.fY, width, height);

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
    if (4 == fBytesPerPixel && kN32_SkColorType == kBGRA_8888_SkColorType) {
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

void GrDrawOpAtlas::Plot::uploadToTexture(GrDeferredTextureUploadWritePixelsFn& writePixels,
                                          GrTextureProxy* proxy) {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData && proxy && proxy->peekTexture());
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    // Clamp to 4-byte aligned boundaries
    unsigned int clearBits = 0x3 / fBytesPerPixel;
    fDirtyRect.fLeft &= ~clearBits;
    fDirtyRect.fRight += clearBits;
    fDirtyRect.fRight &= ~clearBits;
    SkASSERT(fDirtyRect.fRight <= fWidth);
    // Set up dataPtr
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;

    writePixels(proxy, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                fDirtyRect.width(), fDirtyRect.height(), fColorType, dataPtr, rowBytes);
    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

void GrDrawOpAtlas::Plot::resetRects() {
    fRectanizer.reset();

    fGenID = fGenerationCounter->next();
    fPlotLocator = PlotLocator(fPageIndex, fPlotIndex, fGenID);
    fLastUpload = GrDeferredUploadToken::AlreadyFlushedToken();
    fLastUse = GrDeferredUploadToken::AlreadyFlushedToken();

    // zero out the plot
    if (fData) {
        sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
    }

    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

///////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::GrDrawOpAtlas(GrProxyProvider* proxyProvider, const GrBackendFormat& format,
                             GrColorType colorType, int width, int height,
                             int plotWidth, int plotHeight, GenerationCounter* generationCounter,
                             AllowMultitexturing allowMultitexturing)
        : fFormat(format)
        , fColorType(colorType)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fGenerationCounter(generationCounter)
        , fAtlasGeneration(fGenerationCounter->next())
        , fPrevFlushToken(GrDeferredUploadToken::AlreadyFlushedToken())
        , fFlushesSinceLastUse(0)
        , fMaxPages(AllowMultitexturing::kYes == allowMultitexturing ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= GrDrawOpAtlas::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;

    this->createPages(proxyProvider, generationCounter);
}

inline void GrDrawOpAtlas::processEviction(PlotLocator plotLocator) {
    for (EvictionCallback* evictor : fEvictionCallbacks) {
        evictor->evict(plotLocator);
    }

    fAtlasGeneration = fGenerationCounter->next();
}

inline bool GrDrawOpAtlas::updatePlot(GrDeferredUploadTarget* target,
                                      AtlasLocator* atlasLocator, Plot* plot) {
    int pageIdx = plot->pageIndex();
    this->makeMRU(plot, pageIdx);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (plot->lastUploadToken() < target->tokenTracker()->nextTokenToFlush()) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        GrTextureProxy* proxy = fViews[pageIdx].asTextureProxy();
        SkASSERT(proxy && proxy->isInstantiated());  // This is occurring at flush time

        GrDeferredUploadToken lastUploadToken = target->addASAPUpload(
                [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                    plotsp->uploadToTexture(writePixels, proxy);
                });
        plot->setLastUploadToken(lastUploadToken);
    }
    atlasLocator->updatePlotLocator(plot->plotLocator());
    SkDEBUGCODE(this->validate(*atlasLocator);)
    return true;
}

bool GrDrawOpAtlas::uploadToPage(unsigned int pageIdx, GrDeferredUploadTarget* target, int width,
                                 int height, const void* image, AtlasLocator* atlasLocator) {
    SkASSERT(fViews[pageIdx].proxy() && fViews[pageIdx].proxy()->isInstantiated());

    // look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        SkASSERT(GrBackendFormatBytesPerPixel(fViews[pageIdx].proxy()->backendFormat()) ==
                 plot->bpp());

        if (plot->addSubImage(width, height, image, atlasLocator)) {
            return this->updatePlot(target, atlasLocator, plot);
        }
    }

    return false;
}

// Number of atlas-related flushes beyond which we consider a plot to no longer be in use.
//
// This value is somewhat arbitrary -- the idea is to keep it low enough that
// a page with unused plots will get removed reasonably quickly, but allow it
// to hang around for a bit in case it's needed. The assumption is that flushes
// are rare; i.e., we are not continually refreshing the frame.
static constexpr auto kPlotRecentlyUsedCount = 32;
static constexpr auto kAtlasRecentlyUsedCount = 128;

GrDrawOpAtlas::ErrorCode GrDrawOpAtlas::addToAtlas(GrResourceProvider* resourceProvider,
                                                   GrDeferredUploadTarget* target,
                                                   int width, int height, const void* image,
                                                   AtlasLocator* atlasLocator) {
    if (width > fPlotWidth || height > fPlotHeight) {
        return ErrorCode::kError;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (this->uploadToPage(pageIdx, target, width, height, image, atlasLocator)) {
            return ErrorCode::kSucceeded;
        }
    }

    // If the above fails, then see if the least recently used plot per page has already been
    // flushed to the gpu if we're at max page allocation, or if the plot has aged out otherwise.
    // We wait until we've grown to the full number of pages to begin evicting already flushed
    // plots so that we can maximize the opportunity for reuse.
    // As before we prioritize this upload to the first pages, not the most recently used.
    if (fNumActivePages == this->maxPages()) {
        for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
            Plot* plot = fPages[pageIdx].fPlotList.tail();
            SkASSERT(plot);
            if (plot->lastUseToken() < target->tokenTracker()->nextTokenToFlush()) {
                this->processEvictionAndResetRects(plot);
                SkASSERT(GrBackendFormatBytesPerPixel(fViews[pageIdx].proxy()->backendFormat()) ==
                         plot->bpp());
                SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, atlasLocator);
                SkASSERT(verify);
                if (!this->updatePlot(target, atlasLocator, plot)) {
                    return ErrorCode::kError;
                }
                return ErrorCode::kSucceeded;
            }
        }
    } else {
        // If we haven't activated all the available pages, try to create a new one and add to it
        if (!this->activateNewPage(resourceProvider)) {
            return ErrorCode::kError;
        }

        if (this->uploadToPage(fNumActivePages-1, target, width, height, image, atlasLocator)) {
            return ErrorCode::kSucceeded;
        } else {
            // If we fail to upload to a newly activated page then something has gone terribly
            // wrong - return an error
            return ErrorCode::kError;
        }
    }

    if (!fNumActivePages) {
        return ErrorCode::kError;
    }

    // Try to find a plot that we can perform an inline upload to.
    // We prioritize this upload in reverse order of pages to counterbalance the order above.
    Plot* plot = nullptr;
    for (int pageIdx = ((int)fNumActivePages)-1; pageIdx >= 0; --pageIdx) {
        Plot* currentPlot = fPages[pageIdx].fPlotList.tail();
        if (currentPlot->lastUseToken() != target->tokenTracker()->nextDrawToken()) {
            plot = currentPlot;
            break;
        }
    }

    // If we can't find a plot that is not used in a draw currently being prepared by an op, then
    // we have to fail. This gives the op a chance to enqueue the draw, and call back into this
    // function. When that draw is enqueued, the draw token advances, and the subsequent call will
    // continue past this branch and prepare an inline upload that will occur after the enqueued
    // draw which references the plot's pre-upload content.
    if (!plot) {
        return ErrorCode::kTryAgain;
    }

    this->processEviction(plot->plotLocator());
    int pageIdx = plot->pageIndex();
    fPages[pageIdx].fPlotList.remove(plot);
    sk_sp<Plot>& newPlot = fPages[pageIdx].fPlotArray[plot->plotIndex()];
    newPlot.reset(plot->clone());

    fPages[pageIdx].fPlotList.addToHead(newPlot.get());
    SkASSERT(GrBackendFormatBytesPerPixel(fViews[pageIdx].proxy()->backendFormat()) ==
             newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, atlasLocator);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded ASAP.
    // With c++14 we could move sk_sp into lambda to only ref once.
    sk_sp<Plot> plotsp(SkRef(newPlot.get()));

    GrTextureProxy* proxy = fViews[pageIdx].asTextureProxy();
    SkASSERT(proxy && proxy->isInstantiated());

    GrDeferredUploadToken lastUploadToken = target->addInlineUpload(
            [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                plotsp->uploadToTexture(writePixels, proxy);
            });
    newPlot->setLastUploadToken(lastUploadToken);

    atlasLocator->updatePlotLocator(newPlot->plotLocator());
    SkDEBUGCODE(this->validate(*atlasLocator);)

    return ErrorCode::kSucceeded;
}

void GrDrawOpAtlas::compact(GrDeferredUploadToken startTokenForNextFlush) {
    if (fNumActivePages < 1) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }

    // For all plots, reset number of flushes since used if used this frame.
    PlotList::Iter plotIter;
    bool atlasUsedThisFlush = false;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            // Reset number of flushes since used
            if (plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->resetFlushesSinceLastUsed();
                atlasUsedThisFlush = true;
            }

            plotIter.next();
        }
    }

    if (atlasUsedThisFlush) {
        fFlushesSinceLastUse = 0;
    } else {
        ++fFlushesSinceLastUse;
    }

    // We only try to compact if the atlas was used in the recently completed flush or
    // hasn't been used in a long time.
    // This is to handle the case where a lot of text or path rendering has occurred but then just
    // a blinking cursor is drawn.
    if (atlasUsedThisFlush || fFlushesSinceLastUse > kAtlasRecentlyUsedCount) {
        SkTArray<Plot*> availablePlots;
        uint32_t lastPageIndex = fNumActivePages - 1;

        // For all plots but the last one, update number of flushes since used, and check to see
        // if there are any in the first pages that the last page can safely upload to.
        for (uint32_t pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("page %d: ", pageIndex);
            }
#endif
            plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // Update number of flushes since plot was last used
                // We only increment the 'sinceLastUsed' count for flushes where the atlas was used
                // to avoid deleting everything when we return to text drawing in the blinking
                // cursor case
                if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                    plot->incFlushesSinceLastUsed();
                }

#ifdef DUMP_ATLAS_DATA
                if (gDumpAtlasData) {
                    SkDebugf("%d ", plot->flushesSinceLastUsed());
                }
#endif
                // Count plots we can potentially upload to in all pages except the last one
                // (the potential compactee).
                if (plot->flushesSinceLastUsed() > kPlotRecentlyUsedCount) {
                    availablePlots.push_back() = plot;
                }

                plotIter.next();
            }
#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("\n");
            }
#endif
        }

        // Count recently used plots in the last page and evict any that are no longer in use.
        // Since we prioritize uploading to the first pages, this will eventually
        // clear out usage of this page unless we have a large need.
        plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        unsigned int usedPlots = 0;
#ifdef DUMP_ATLAS_DATA
        if (gDumpAtlasData) {
            SkDebugf("page %d: ", lastPageIndex);
        }
#endif
        while (Plot* plot = plotIter.get()) {
            // Update number of flushes since plot was last used
            if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->incFlushesSinceLastUsed();
            }

#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("%d ", plot->flushesSinceLastUsed());
            }
#endif
            // If this plot was used recently
            if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                usedPlots++;
            } else if (plot->lastUseToken() != GrDeferredUploadToken::AlreadyFlushedToken()) {
                // otherwise if aged out just evict it.
                this->processEvictionAndResetRects(plot);
            }
            plotIter.next();
        }
#ifdef DUMP_ATLAS_DATA
        if (gDumpAtlasData) {
            SkDebugf("\n");
        }
#endif

        // If recently used plots in the last page are using less than a quarter of the page, try
        // to evict them if there's available space in earlier pages. Since we prioritize uploading
        // to the first pages, this will eventually clear out usage of this page unless we have a
        // large need.
        if (availablePlots.count() && usedPlots && usedPlots <= fNumPlots / 4) {
            plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // If this plot was used recently
                if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                    // See if there's room in an earlier page and if so evict.
                    // We need to be somewhat harsh here so that a handful of plots that are
                    // consistently in use don't end up locking the page in memory.
                    if (availablePlots.count() > 0) {
                        this->processEvictionAndResetRects(plot);
                        this->processEvictionAndResetRects(availablePlots.back());
                        availablePlots.pop_back();
                        --usedPlots;
                    }
                    if (!usedPlots || !availablePlots.count()) {
                        break;
                    }
                }
                plotIter.next();
            }
        }

        // If none of the plots in the last page have been used recently, delete it.
        if (!usedPlots) {
#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("delete %d\n", fNumActivePages-1);
            }
#endif
            this->deactivateLastPage();
            fFlushesSinceLastUse = 0;
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool GrDrawOpAtlas::createPages(
        GrProxyProvider* proxyProvider, GenerationCounter* generationCounter) {
    SkASSERT(SkIsPow2(fTextureWidth) && SkIsPow2(fTextureHeight));

    SkISize dims = {fTextureWidth, fTextureHeight};

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        GrSwizzle swizzle = proxyProvider->caps()->getReadSwizzle(fFormat, fColorType);
        if (GrColorTypeIsAlphaOnly(fColorType)) {
            swizzle = GrSwizzle::Concat(swizzle, GrSwizzle("aaaa"));
        }
        sk_sp<GrSurfaceProxy> proxy = proxyProvider->createProxy(
                fFormat, dims, GrRenderable::kNo, 1, GrMipmapped::kNo, SkBackingFit::kExact,
                SkBudgeted::kYes, GrProtected::kNo, GrInternalSurfaceFlags::kNone,
                GrSurfaceProxy::UseAllocator::kNo);
        if (!proxy) {
            return false;
        }
        fViews[i] = GrSurfaceProxyView(std::move(proxy), kTopLeft_GrSurfaceOrigin, swizzle);

        // set up allocated plots
        fPages[i].fPlotArray = std::make_unique<sk_sp<Plot>[]>(numPlotsX * numPlotsY);

        sk_sp<Plot>* currPlot = fPages[i].fPlotArray.get();
        for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
            for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
                uint32_t plotIndex = r * numPlotsX + c;
                currPlot->reset(new Plot(
                    i, plotIndex, generationCounter, x, y, fPlotWidth, fPlotHeight, fColorType));

                // build LRU list
                fPages[i].fPlotList.addToHead(currPlot->get());
                ++currPlot;
            }
        }

    }

    return true;
}

bool GrDrawOpAtlas::activateNewPage(GrResourceProvider* resourceProvider) {
    SkASSERT(fNumActivePages < this->maxPages());

    if (!fViews[fNumActivePages].proxy()->instantiate(resourceProvider)) {
        return false;
    }

#ifdef DUMP_ATLAS_DATA
    if (gDumpAtlasData) {
        SkDebugf("activated page#: %d\n", fNumActivePages);
    }
#endif

    ++fNumActivePages;
    return true;
}


inline void GrDrawOpAtlas::deactivateLastPage() {
    SkASSERT(fNumActivePages);

    uint32_t lastPageIndex = fNumActivePages - 1;

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    fPages[lastPageIndex].fPlotList.reset();
    for (int r = 0; r < numPlotsY; ++r) {
        for (int c = 0; c < numPlotsX; ++c) {
            uint32_t plotIndex = r * numPlotsX + c;

            Plot* currPlot = fPages[lastPageIndex].fPlotArray[plotIndex].get();
            currPlot->resetRects();
            currPlot->resetFlushesSinceLastUsed();

            // rebuild the LRU list
            SkDEBUGCODE(currPlot->fPrev = currPlot->fNext = nullptr);
            SkDEBUGCODE(currPlot->fList = nullptr);
            fPages[lastPageIndex].fPlotList.addToHead(currPlot);
        }
    }

    // remove ref to the backing texture
    fViews[lastPageIndex].proxy()->deinstantiate();
    --fNumActivePages;
}

GrDrawOpAtlasConfig::GrDrawOpAtlasConfig(int maxTextureSize, size_t maxBytes) {
    static const SkISize kARGBDimensions[] = {
        {256, 256},   // maxBytes < 2^19
        {512, 256},   // 2^19 <= maxBytes < 2^20
        {512, 512},   // 2^20 <= maxBytes < 2^21
        {1024, 512},  // 2^21 <= maxBytes < 2^22
        {1024, 1024}, // 2^22 <= maxBytes < 2^23
        {2048, 1024}, // 2^23 <= maxBytes
    };

    // Index 0 corresponds to maxBytes of 2^18, so start by dividing it by that
    maxBytes >>= 18;
    // Take the floor of the log to get the index
    int index = maxBytes > 0
        ? SkTPin<int>(SkPrevLog2(maxBytes), 0, SK_ARRAY_COUNT(kARGBDimensions) - 1)
        : 0;

    SkASSERT(kARGBDimensions[index].width() <= kMaxAtlasDim);
    SkASSERT(kARGBDimensions[index].height() <= kMaxAtlasDim);
    fARGBDimensions.set(std::min<int>(kARGBDimensions[index].width(), maxTextureSize),
                        std::min<int>(kARGBDimensions[index].height(), maxTextureSize));
    fMaxTextureSize = std::min<int>(maxTextureSize, kMaxAtlasDim);
}

SkISize GrDrawOpAtlasConfig::atlasDimensions(GrMaskFormat type) const {
    if (kA8_GrMaskFormat == type) {
        // A8 is always 2x the ARGB dimensions, clamped to the max allowed texture size
        return { std::min<int>(2 * fARGBDimensions.width(), fMaxTextureSize),
                 std::min<int>(2 * fARGBDimensions.height(), fMaxTextureSize) };
    } else {
        return fARGBDimensions;
    }
}

SkISize GrDrawOpAtlasConfig::plotDimensions(GrMaskFormat type) const {
    if (kA8_GrMaskFormat == type) {
        SkISize atlasDimensions = this->atlasDimensions(type);
        // For A8 we want to grow the plots at larger texture sizes to accept more of the
        // larger SDF glyphs. Since the largest SDF glyph can be 170x170 with padding, this
        // allows us to pack 3 in a 512x256 plot, or 9 in a 512x512 plot.

        // This will give us 512x256 plots for 2048x1024, 512x512 plots for 2048x2048,
        // and 256x256 plots otherwise.
        int plotWidth = atlasDimensions.width() >= 2048 ? 512 : 256;
        int plotHeight = atlasDimensions.height() >= 2048 ? 512 : 256;

        return { plotWidth, plotHeight };
    } else {
        // ARGB and LCD always use 256x256 plots -- this has been shown to be faster
        return { 256, 256 };
    }
}

constexpr int GrDrawOpAtlasConfig::kMaxAtlasDim;
