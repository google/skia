/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOpAtlas.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrOpFlushState.h"
#include "GrRectanizer.h"
#include "GrProxyProvider.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "GrTracing.h"

std::unique_ptr<GrDrawOpAtlas> GrDrawOpAtlas::Make(GrContext* ctx, GrPixelConfig config, int width,
                                                   int height, int numPlotsX, int numPlotsY,
                                                   AllowMultitexturing allowMultitexturing,
                                                   GrDrawOpAtlas::EvictionFunc func, void* data) {
    std::unique_ptr<GrDrawOpAtlas> atlas(new GrDrawOpAtlas(ctx, config, width, height, numPlotsX,
                                                           numPlotsY, allowMultitexturing));
    if (!atlas->getProxies()[0]) {
        return nullptr;
    }

    atlas->registerEvictionCallback(func, data);
    return atlas;
}

#ifdef DUMP_ATLAS_DATA
static bool gDumpAtlasData = false;
#endif

////////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::Plot::Plot(int pageIndex, int plotIndex, uint64_t genID, int offX, int offY,
                          int width, int height, GrPixelConfig config)
        : fLastUpload(GrDeferredUploadToken::AlreadyFlushedToken())
        , fLastUse(GrDeferredUploadToken::AlreadyFlushedToken())
        , fFlushesSinceLastUse(0)
        , fPageIndex(pageIndex)
        , fPlotIndex(plotIndex)
        , fGenID(genID)
        , fID(CreateId(fPageIndex, fPlotIndex, fGenID))
        , fData(nullptr)
        , fWidth(width)
        , fHeight(height)
        , fX(offX)
        , fY(offY)
        , fRects(nullptr)
        , fOffset(SkIPoint16::Make(fX * fWidth, fY * fHeight))
        , fConfig(config)
        , fBytesPerPixel(GrBytesPerPixel(config))
#ifdef SK_DEBUG
        , fDirty(false)
#endif
{
    fDirtyRect.setEmpty();
}

GrDrawOpAtlas::Plot::~Plot() {
    sk_free(fData);
    delete fRects;
}

bool GrDrawOpAtlas::Plot::addSubImage(int width, int height, const void* image, SkIPoint16* loc) {
    SkASSERT(width <= fWidth && height <= fHeight);

    if (!fRects) {
        fRects = GrRectanizer::Factory(fWidth, fHeight);
    }

    if (!fRects->addRect(width, height, loc)) {
        return false;
    }

    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(sk_calloc_throw(fBytesPerPixel * fWidth *
                                                                 fHeight));
    }
    size_t rowBytes = width * fBytesPerPixel;
    const unsigned char* imagePtr = (const unsigned char*)image;
    // point ourselves at the right starting spot
    unsigned char* dataPtr = fData;
    dataPtr += fBytesPerPixel * fWidth * loc->fY;
    dataPtr += fBytesPerPixel * loc->fX;
    // copy into the data buffer, swizzling as we go if this is ARGB data
    if (4 == fBytesPerPixel && kSkia8888_GrPixelConfig == kBGRA_8888_GrPixelConfig) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA(reinterpret_cast<uint32_t*>(dataPtr), imagePtr, width);
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

    fDirtyRect.join(loc->fX, loc->fY, loc->fX + width, loc->fY + height);

    loc->fX += fOffset.fX;
    loc->fY += fOffset.fY;
    SkDEBUGCODE(fDirty = true;)

    return true;
}

void GrDrawOpAtlas::Plot::uploadToTexture(GrDeferredTextureUploadWritePixelsFn& writePixels,
                                          GrTextureProxy* proxy) {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData && proxy && proxy->priv().peekTexture());
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    writePixels(proxy, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                fDirtyRect.width(), fDirtyRect.height(), fConfig, dataPtr, rowBytes);
    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

void GrDrawOpAtlas::Plot::resetRects() {
    if (fRects) {
        fRects->reset();
    }

    fGenID++;
    fID = CreateId(fPageIndex, fPlotIndex, fGenID);
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

GrDrawOpAtlas::GrDrawOpAtlas(GrContext* context, GrPixelConfig config, int width, int height,
                             int numPlotsX, int numPlotsY, AllowMultitexturing allowMultitexturing)
        : fContext(context)
        , fPixelConfig(config)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fAtlasGeneration(kInvalidAtlasGeneration + 1)
        , fPrevFlushToken(GrDeferredUploadToken::AlreadyFlushedToken())
        , fAllowMultitexturing(allowMultitexturing)
        , fNumPages(0) {
    fPlotWidth = fTextureWidth / numPlotsX;
    fPlotHeight = fTextureHeight / numPlotsY;
    SkASSERT(numPlotsX * numPlotsY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    SkDEBUGCODE(fNumPlots = numPlotsX * numPlotsY;)

    this->createNewPage();
}

inline void GrDrawOpAtlas::processEviction(AtlasID id) {
    for (int i = 0; i < fEvictionCallbacks.count(); i++) {
        (*fEvictionCallbacks[i].fFunc)(id, fEvictionCallbacks[i].fData);
    }
    ++fAtlasGeneration;
}

inline bool GrDrawOpAtlas::updatePlot(GrDeferredUploadTarget* target, AtlasID* id, Plot* plot) {
    int pageIdx = GetPageIndexFromID(plot->id());
    this->makeMRU(plot, pageIdx);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (plot->lastUploadToken() < target->nextTokenToFlush()) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
        // Once it is deferred more care must be taken upon instantiation failure.
        if (!fProxies[pageIdx]->instantiate(fContext->contextPriv().resourceProvider())) {
            return false;
        }

        GrTextureProxy* proxy = fProxies[pageIdx].get();

        GrDeferredUploadToken lastUploadToken = target->addASAPUpload(
                [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                    plotsp->uploadToTexture(writePixels, proxy);
                });
        plot->setLastUploadToken(lastUploadToken);
    }
    *id = plot->id();
    return true;
}

// Number of atlas-related flushes beyond which we consider a plot to no longer be in use.
//
// This value is somewhat arbitrary -- the idea is to keep it low enough that
// a page with unused plots will get removed reasonably quickly, but allow it
// to hang around for a bit in case it's needed. The assumption is that flushes
// are rare; i.e., we are not continually refreshing the frame.
static constexpr auto kRecentlyUsedCount = 256;

bool GrDrawOpAtlas::addToAtlas(AtlasID* id, GrDeferredUploadTarget* target, int width, int height,
                               const void* image, SkIPoint16* loc) {
    if (width > fPlotWidth || height > fPlotHeight) {
        return false;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (unsigned int pageIdx = 0; pageIdx < fNumPages; ++pageIdx) {
        SkASSERT(fProxies[pageIdx]);
        // look through all allocated plots for one we can share, in Most Recently Refed order
        PlotList::Iter plotIter;
        plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);
        Plot* plot;
        while ((plot = plotIter.get())) {
            SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
            if (plot->addSubImage(width, height, image, loc)) {
                return this->updatePlot(target, id, plot);
            }
            plotIter.next();
        }
    }

    // If the above fails, then see if the least recently used plot per page has already been
    // flushed to the gpu if we're at max page allocation, or if the plot has aged out otherwise.
    // We wait until we've grown to the full number of pages to begin evicting already flushed
    // plots so that we can maximize the opportunity for reuse.
    // As before we prioritize this upload to the first pages, not the most recently used.
    for (unsigned int pageIdx = 0; pageIdx < fNumPages; ++pageIdx) {
        Plot* plot = fPages[pageIdx].fPlotList.tail();
        SkASSERT(plot);
        if ((fNumPages == this->maxPages() && plot->lastUseToken() < target->nextTokenToFlush()) ||
            plot->flushesSinceLastUsed() >= kRecentlyUsedCount) {
            this->processEvictionAndResetRects(plot);
            SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
            SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
            SkASSERT(verify);
            if (!this->updatePlot(target, id, plot)) {
                return false;
            }
            return true;
        }
    }

    // If the simple cases fail, try to create a new page and add to it
    if (this->createNewPage()) {
        unsigned int pageIdx = fNumPages-1;
        SkASSERT(fProxies[pageIdx]);
        Plot* plot = fPages[pageIdx].fPlotList.head();
        SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
        if (plot->addSubImage(width, height, image, loc)) {
            return this->updatePlot(target, id, plot);
        }

        // we shouldn't get here -- if so, something has gone terribly wrong
        SkASSERT(false);
        return false;
    }

    // Try to find a plot that we can perform an inline upload to.
    // We prioritize this upload in reverse order of pages to counterbalance the order above.
    Plot* plot = nullptr;
    for (int pageIdx = (int)(fNumPages-1); pageIdx >= 0; --pageIdx) {
        Plot* currentPlot = fPages[pageIdx].fPlotList.tail();
        if (currentPlot->lastUseToken() != target->nextDrawToken()) {
            plot = currentPlot;
            break;
        }
    }

    // If we can't find a plot that is not used in a draw currently being prepared by an op, then
    // we have to fail. This gives the op a chance to enqueue the draw, and call back into this
    // function. When that draw is enqueued, the draw token advances, and the subsequent call will
    // continue past this branch and prepare an inline upload that will occur after the enqueued
    //draw which references the plot's pre-upload content.
    if (!plot) {
        return false;
    }

    this->processEviction(plot->id());
    int pageIdx = GetPageIndexFromID(plot->id());
    fPages[pageIdx].fPlotList.remove(plot);
    sk_sp<Plot>& newPlot = fPages[pageIdx].fPlotArray[plot->index()];
    newPlot.reset(plot->clone());

    fPages[pageIdx].fPlotList.addToHead(newPlot.get());
    SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded ASAP.
    // With c+14 we could move sk_sp into lambda to only ref once.
    sk_sp<Plot> plotsp(SkRef(newPlot.get()));
    // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
    // Once it is deferred more care must be taken upon instantiation failure.
    if (!fProxies[pageIdx]->instantiate(fContext->contextPriv().resourceProvider())) {
        return false;
    }
    GrTextureProxy* proxy = fProxies[pageIdx].get();

    GrDeferredUploadToken lastUploadToken = target->addInlineUpload(
            [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                plotsp->uploadToTexture(writePixels, proxy);
            });
    newPlot->setLastUploadToken(lastUploadToken);

    *id = newPlot->id();

    return true;
}

void GrDrawOpAtlas::compact(GrDeferredUploadToken startTokenForNextFlush) {
    if (fNumPages <= 1) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }

    // For all plots, reset number of flushes since used if used this frame.
    PlotList::Iter plotIter;
    bool atlasUsedThisFlush = false;
    for (uint32_t pageIndex = 0; pageIndex < fNumPages; ++pageIndex) {
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

    // We only try to compact if the atlas was used in the recently completed flush.
    // This is to handle the case where a lot of text or path rendering has occurred but then just
    // a blinking cursor is drawn.
    // TODO: consider if we should also do this if it's been a long time since the last atlas use
    if (atlasUsedThisFlush) {
        int availablePlots = 0;
        uint32_t lastPageIndex = fNumPages - 1;

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
                if (plot->flushesSinceLastUsed() > kRecentlyUsedCount) {
                    ++availablePlots;
                }

                plotIter.next();
            }
#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("\n");
            }
#endif
        }

        // Count recently used plots in the last page and evict them if there's available space
        // in earlier pages. Since we prioritize uploading to the first pages, this will eventually
        // clear out usage of this page unless we have a large need.
        plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        int usedPlots = 0;
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
            if (plot->flushesSinceLastUsed() <= kRecentlyUsedCount) {
                usedPlots++;
                // see if there's room in an earlier page and if so evict.
                // We need to be somewhat harsh here so that one plot that is consistently in use
                // doesn't end up locking the page in memory.
                if (availablePlots) {
                    this->processEvictionAndResetRects(plot);
                    --availablePlots;
                }
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
        // If none of the plots in the last page have been used recently, delete it.
        if (!usedPlots) {
#ifdef DUMP_ATLAS_DATA
            if (gDumpAtlasData) {
                SkDebugf("delete %d\n", fNumPages-1);
            }
#endif
            this->deleteLastPage();
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool GrDrawOpAtlas::createNewPage() {
    if (fNumPages == this->maxPages()) {
        return false;
    }

    GrProxyProvider* proxyProvider = fContext->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = fTextureWidth;
    desc.fHeight = fTextureHeight;
    desc.fConfig = fPixelConfig;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    // TODO: Determine how to avoid having to do this. (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    // MDB TODO: for now, wrap an instantiated texture. Having the deferred instantiation
    // possess the correct properties (e.g., no pendingIO) should fall out of the system but
    // should receive special attention.
    // Note: When switching over to the deferred proxy, use the kExact flag to create
    // the atlas and assert that the width & height are powers of 2.
    // DDL TODO: remove this use of createInstantitateProxy & convert it to a testing-only method.
    fProxies[fNumPages] = proxyProvider->createInstantiatedProxy(desc, SkBackingFit::kApprox,
                                                                 SkBudgeted::kYes, kFlags);
    if (!fProxies[fNumPages]) {
        return false;
    }

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    // set up allocated plots
    fPages[fNumPages].fPlotArray.reset(new sk_sp<Plot>[ numPlotsX * numPlotsY ]);

    sk_sp<Plot>* currPlot = fPages[fNumPages].fPlotArray.get();
    for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            uint32_t plotIndex = r * numPlotsX + c;
            currPlot->reset(new Plot(fNumPages, plotIndex, 1, x, y, fPlotWidth, fPlotHeight,
                                     fPixelConfig));

            // build LRU list
            fPages[fNumPages].fPlotList.addToHead(currPlot->get());
            ++currPlot;
        }
    }

#ifdef DUMP_ATLAS_DATA
    if (gDumpAtlasData) {
        SkDebugf("created %d\n", fNumPages);
    }
#endif
    fNumPages++;
    return true;
}

inline void GrDrawOpAtlas::deleteLastPage() {
    uint32_t lastPageIndex = fNumPages - 1;
    // clean out the plots
    fPages[lastPageIndex].fPlotList.reset();
    fPages[lastPageIndex].fPlotArray.reset(nullptr);
    // remove ref to texture proxy
    fProxies[lastPageIndex].reset(nullptr);
    --fNumPages;
}
