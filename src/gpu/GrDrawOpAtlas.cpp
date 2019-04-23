/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawOpAtlas.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRectanizer.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTracing.h"

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
        SkASSERT(fProxies[i] && fProxies[i]->isInstantiated());
    }
}

std::unique_ptr<GrDrawOpAtlas> GrDrawOpAtlas::Make(GrProxyProvider* proxyProvider,
                                                   const GrBackendFormat& format,
                                                   GrPixelConfig config, int width,
                                                   int height, int plotWidth, int plotHeight,
                                                   AllowMultitexturing allowMultitexturing,
                                                   GrDrawOpAtlas::EvictionFunc func, void* data) {
    std::unique_ptr<GrDrawOpAtlas> atlas(new GrDrawOpAtlas(proxyProvider, format, config, width,
                                                           height, plotWidth, plotHeight,
                                                           allowMultitexturing));
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
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT(((width*fBytesPerPixel) & 0x3) == 0);
    // The padding for faster uploads only works for 1, 2 and 4 byte texels
    SkASSERT(fBytesPerPixel != 3 && fBytesPerPixel <= 4);
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

    fDirtyRect.join(loc->fX, loc->fY, loc->fX + width, loc->fY + height);

    loc->fX += fOffset.fX;
    loc->fY += fOffset.fY;
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
    // TODO: Make GrDrawOpAtlas store a GrColorType rather than GrPixelConfig.
    auto colorType = GrPixelConfigToColorType(fConfig);
    writePixels(proxy, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                fDirtyRect.width(), fDirtyRect.height(), colorType, dataPtr, rowBytes);
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

GrDrawOpAtlas::GrDrawOpAtlas(GrProxyProvider* proxyProvider, const GrBackendFormat& format,
                             GrPixelConfig config, int width, int height,
                             int plotWidth, int plotHeight, AllowMultitexturing allowMultitexturing)
        : fFormat(format)
        , fPixelConfig(config)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fAtlasGeneration(kInvalidAtlasGeneration + 1)
        , fPrevFlushToken(GrDeferredUploadToken::AlreadyFlushedToken())
        , fMaxPages(AllowMultitexturing::kYes == allowMultitexturing ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= GrDrawOpAtlas::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;

    this->createPages(proxyProvider);
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
    if (plot->lastUploadToken() < target->tokenTracker()->nextTokenToFlush()) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        GrTextureProxy* proxy = fProxies[pageIdx].get();
        SkASSERT(proxy->isInstantiated());  // This is occurring at flush time

        GrDeferredUploadToken lastUploadToken = target->addASAPUpload(
                [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                    plotsp->uploadToTexture(writePixels, proxy);
                });
        plot->setLastUploadToken(lastUploadToken);
    }
    *id = plot->id();
    return true;
}

bool GrDrawOpAtlas::uploadToPage(unsigned int pageIdx, AtlasID* id, GrDeferredUploadTarget* target,
                                 int width, int height, const void* image, SkIPoint16* loc) {
    SkASSERT(fProxies[pageIdx] && fProxies[pageIdx]->isInstantiated());

    // look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());

        if (plot->addSubImage(width, height, image, loc)) {
            return this->updatePlot(target, id, plot);
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
static constexpr auto kRecentlyUsedCount = 256;

GrDrawOpAtlas::ErrorCode GrDrawOpAtlas::addToAtlas(GrResourceProvider* resourceProvider,
                                                   AtlasID* id, GrDeferredUploadTarget* target,
                                                   int width, int height,
                                                   const void* image, SkIPoint16* loc) {
    if (width > fPlotWidth || height > fPlotHeight) {
        return ErrorCode::kError;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (this->uploadToPage(pageIdx, id, target, width, height, image, loc)) {
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
                SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
                SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
                SkASSERT(verify);
                if (!this->updatePlot(target, id, plot)) {
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

        if (this->uploadToPage(fNumActivePages-1, id, target, width, height, image, loc)) {
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

    GrTextureProxy* proxy = fProxies[pageIdx].get();
    SkASSERT(proxy->isInstantiated());

    GrDeferredUploadToken lastUploadToken = target->addInlineUpload(
            [plotsp, proxy](GrDeferredTextureUploadWritePixelsFn& writePixels) {
                plotsp->uploadToTexture(writePixels, proxy);
            });
    newPlot->setLastUploadToken(lastUploadToken);

    *id = newPlot->id();

    return ErrorCode::kSucceeded;
}

void GrDrawOpAtlas::compact(GrDeferredUploadToken startTokenForNextFlush) {
    if (fNumActivePages <= 1) {
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

    // We only try to compact if the atlas was used in the recently completed flush.
    // This is to handle the case where a lot of text or path rendering has occurred but then just
    // a blinking cursor is drawn.
    // TODO: consider if we should also do this if it's been a long time since the last atlas use
    if (atlasUsedThisFlush) {
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
                if (plot->flushesSinceLastUsed() > kRecentlyUsedCount) {
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
            if (plot->flushesSinceLastUsed() <= kRecentlyUsedCount) {
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
                if (plot->flushesSinceLastUsed() <= kRecentlyUsedCount) {
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
                SkDebugf("delete %d\n", fNumPages-1);
            }
#endif
            this->deactivateLastPage();
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool GrDrawOpAtlas::createPages(GrProxyProvider* proxyProvider) {
    SkASSERT(SkIsPow2(fTextureWidth) && SkIsPow2(fTextureHeight));

    GrSurfaceDesc desc;
    if (proxyProvider->caps()->shouldInitializeTextures()) {
        // The atlas isn't guaranteed to touch all its pixels so, for platforms that benefit
        // from complete initialization, clear everything.
        desc.fFlags = kPerformInitialClear_GrSurfaceFlag;
    } else {
        desc.fFlags = kNone_GrSurfaceFlags;
    }
    desc.fWidth = fTextureWidth;
    desc.fHeight = fTextureHeight;
    desc.fConfig = fPixelConfig;

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        fProxies[i] = proxyProvider->createProxy(fFormat, desc, kTopLeft_GrSurfaceOrigin,
                SkBackingFit::kExact, SkBudgeted::kYes, GrInternalSurfaceFlags::kNoPendingIO);
        if (!fProxies[i]) {
            return false;
        }

        fProxies[i]->priv().setIgnoredByResourceAllocator();

        // set up allocated plots
        fPages[i].fPlotArray.reset(new sk_sp<Plot>[ numPlotsX * numPlotsY ]);

        sk_sp<Plot>* currPlot = fPages[i].fPlotArray.get();
        for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
            for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
                uint32_t plotIndex = r * numPlotsX + c;
                currPlot->reset(new Plot(i, plotIndex, 1, x, y, fPlotWidth, fPlotHeight,
                                         fPixelConfig));

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

    if (!fProxies[fNumActivePages]->instantiate(resourceProvider)) {
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
    fProxies[lastPageIndex]->deinstantiate();
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
    fARGBDimensions.set(SkTMin<int>(kARGBDimensions[index].width(), maxTextureSize),
                        SkTMin<int>(kARGBDimensions[index].height(), maxTextureSize));
    fMaxTextureSize = SkTMin<int>(maxTextureSize, kMaxAtlasDim);
}

SkISize GrDrawOpAtlasConfig::atlasDimensions(GrMaskFormat type) const {
    if (kA8_GrMaskFormat == type) {
        // A8 is always 2x the ARGB dimensions, clamped to the max allowed texture size
        return { SkTMin<int>(2 * fARGBDimensions.width(), fMaxTextureSize),
                 SkTMin<int>(2 * fARGBDimensions.height(), fMaxTextureSize) };
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
