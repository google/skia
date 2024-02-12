/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawAtlas.h"

#include <memory>

#include "include/core/SkColorSpace.h"
#include "include/core/SkStream.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTPin.h"

#include "src/base/SkMathPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

using namespace skia_private;

namespace skgpu::graphite {

#if defined(DUMP_ATLAS_DATA)
static const constexpr bool kDumpAtlasData = true;
#else
static const constexpr bool kDumpAtlasData = false;
#endif

#ifdef SK_DEBUG
void DrawAtlas::validate(const AtlasLocator& atlasLocator) const {
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

std::unique_ptr<DrawAtlas> DrawAtlas::Make(SkColorType colorType, size_t bpp, int width,
                                           int height, int plotWidth, int plotHeight,
                                           AtlasGenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label) {
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(colorType, bpp, width, height,
                                                   plotWidth, plotHeight, generationCounter,
                                                   allowMultitexturing, label));

    if (evictor != nullptr) {
        atlas->fEvictionCallbacks.emplace_back(evictor);
    }
    return atlas;
}

///////////////////////////////////////////////////////////////////////////////
static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}
DrawAtlas::DrawAtlas(SkColorType colorType, size_t bpp, int width, int height,
                     int plotWidth, int plotHeight, AtlasGenerationCounter* generationCounter,
                     AllowMultitexturing allowMultitexturing, std::string_view label)
        : fColorType(colorType)
        , fBytesPerPixel(bpp)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fLabel(label)
        , fAtlasID(next_id())
        , fGenerationCounter(generationCounter)
        , fAtlasGeneration(fGenerationCounter->next())
        , fPrevFlushToken(AtlasToken::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fMaxPages(AllowMultitexturing::kYes == allowMultitexturing ?
                            PlotLocator::kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= PlotLocator::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;

    this->createPages(generationCounter);
}

inline void DrawAtlas::processEviction(PlotLocator plotLocator) {
    for (PlotEvictionCallback* evictor : fEvictionCallbacks) {
        evictor->evict(plotLocator);
    }

    fAtlasGeneration = fGenerationCounter->next();
}

inline bool DrawAtlas::updatePlot(AtlasLocator* atlasLocator, Plot* plot) {
    int pageIdx = plot->pageIndex();
    this->makeMRU(plot, pageIdx);

    // The actual upload will be created in recordUploads().

    atlasLocator->updatePlotLocator(plot->plotLocator());
    SkDEBUGCODE(this->validate(*atlasLocator);)
    return true;
}

bool DrawAtlas::addToPage(unsigned int pageIdx, int width, int height, const void* image,
                          AtlasLocator* atlasLocator) {
    SkASSERT(fProxies[pageIdx]);

    // look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        if (plot->addSubImage(width, height, image, atlasLocator)) {
            return this->updatePlot(atlasLocator, plot);
        }
    }

    return false;
}

bool DrawAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    for (uint32_t pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        PlotList::Iter plotIter;
        plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);
        for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
            if (plot->needsUpload()) {
                TextureProxy* proxy = fProxies[pageIdx].get();
                SkASSERT(proxy);

                const void* dataPtr;
                SkIRect dstRect;
                std::tie(dataPtr, dstRect) = plot->prepareForUpload();
                if (dstRect.isEmpty()) {
                    continue;
                }

                std::vector<MipLevel> levels;
                levels.push_back({dataPtr, fBytesPerPixel*fPlotWidth});

                // Src and dst colorInfo are the same
                SkColorInfo colorInfo(fColorType, kUnknown_SkAlphaType, nullptr);
                if (!dc->recordUpload(recorder, sk_ref_sp(proxy), colorInfo, colorInfo, levels,
                                      dstRect, /*ConditionalUploadContext=*/nullptr)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Number of atlas-related flushes beyond which we consider a plot to no longer be in use.
//
// This value is somewhat arbitrary -- the idea is to keep it low enough that
// a page with unused plots will get removed reasonably quickly, but allow it
// to hang around for a bit in case it's needed. The assumption is that flushes
// are rare; i.e., we are not continually refreshing the frame.
static constexpr auto kPlotRecentlyUsedCount = 32;
static constexpr auto kAtlasRecentlyUsedCount = 128;

DrawAtlas::ErrorCode DrawAtlas::addToAtlas(Recorder* recorder,
                                           int width, int height, const void* image,
                                           AtlasLocator* atlasLocator) {
    if (width > fPlotWidth || height > fPlotHeight) {
        return ErrorCode::kError;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (this->addToPage(pageIdx, width, height, image, atlasLocator)) {
            return ErrorCode::kSucceeded;
        }
    }

    // If the above fails, then see if the least recently used plot per page has already been
    // queued for upload if we're at max page allocation, or if the plot has aged out otherwise.
    // We wait until we've grown to the full number of pages to begin evicting already queued
    // plots so that we can maximize the opportunity for reuse.
    // As before we prioritize this upload to the first pages, not the most recently used.
    if (fNumActivePages == this->maxPages()) {
        for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
            Plot* plot = fPages[pageIdx].fPlotList.tail();
            SkASSERT(plot);
            if (plot->lastUseToken() < recorder->priv().tokenTracker()->nextFlushToken()) {
                this->processEvictionAndResetRects(plot);
                SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, atlasLocator);
                SkASSERT(verify);
                if (!this->updatePlot(atlasLocator, plot)) {
                    return ErrorCode::kError;
                }
                return ErrorCode::kSucceeded;
            }
        }
    } else {
        // If we haven't activated all the available pages, try to create a new one and add to it
        if (!this->activateNewPage(recorder)) {
            return ErrorCode::kError;
        }

        if (this->addToPage(fNumActivePages-1, width, height, image, atlasLocator)) {
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

    // All plots are currently in use by the current set of draws, so we need to fail. This
    // gives the Device a chance to snap the current set of uploads and draws, advance the draw
    // token, and call back into this function. The subsequent call will have plots available
    // for fresh uploads.
    return ErrorCode::kTryAgain;
}

void DrawAtlas::compact(AtlasToken startTokenForNextFlush) {
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
        TArray<Plot*> availablePlots;
        uint32_t lastPageIndex = fNumActivePages - 1;

        // For all plots but the last one, update number of flushes since used, and check to see
        // if there are any in the first pages that the last page can safely upload to.
        for (uint32_t pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("page %d: ", pageIndex);
            }

            plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // Update number of flushes since plot was last used
                // We only increment the 'sinceLastUsed' count for flushes where the atlas was used
                // to avoid deleting everything when we return to text drawing in the blinking
                // cursor case
                if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                    plot->incFlushesSinceLastUsed();
                }

                if constexpr (kDumpAtlasData) {
                    SkDebugf("%d ", plot->flushesSinceLastUsed());
                }

                // Count plots we can potentially upload to in all pages except the last one
                // (the potential compactee).
                if (plot->flushesSinceLastUsed() > kPlotRecentlyUsedCount) {
                    availablePlots.push_back() = plot;
                }

                plotIter.next();
            }

            if constexpr (kDumpAtlasData) {
                SkDebugf("\n");
            }
        }

        // Count recently used plots in the last page and evict any that are no longer in use.
        // Since we prioritize uploading to the first pages, this will eventually
        // clear out usage of this page unless we have a large need.
        plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        unsigned int usedPlots = 0;
        if constexpr (kDumpAtlasData) {
            SkDebugf("page %d: ", lastPageIndex);
        }
        while (Plot* plot = plotIter.get()) {
            // Update number of flushes since plot was last used
            if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->incFlushesSinceLastUsed();
            }

            if constexpr (kDumpAtlasData) {
                SkDebugf("%d ", plot->flushesSinceLastUsed());
            }

            // If this plot was used recently
            if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                usedPlots++;
            } else if (plot->lastUseToken() != AtlasToken::InvalidToken()) {
                // otherwise if aged out just evict it.
                this->processEvictionAndResetRects(plot);
            }
            plotIter.next();
        }

        if constexpr (kDumpAtlasData) {
            SkDebugf("\n");
        }

        // If recently used plots in the last page are using less than a quarter of the page, try
        // to evict them if there's available space in lower index pages. Since we prioritize
        // uploading to the first pages, this will eventually clear out usage of this page unless
        // we have a large need.
        if (availablePlots.size() && usedPlots && usedPlots <= fNumPlots / 4) {
            plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // If this plot was used recently
                if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                    // See if there's room in an lower index page and if so evict.
                    // We need to be somewhat harsh here so that a handful of plots that are
                    // consistently in use don't end up locking the page in memory.
                    if (availablePlots.size() > 0) {
                        this->processEvictionAndResetRects(plot);
                        this->processEvictionAndResetRects(availablePlots.back());
                        availablePlots.pop_back();
                        --usedPlots;
                    }
                    if (!usedPlots || !availablePlots.size()) {
                        break;
                    }
                }
                plotIter.next();
            }
        }

        // If none of the plots in the last page have been used recently, delete it.
        if (!usedPlots) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("delete %d\n", fNumActivePages-1);
            }

            this->deactivateLastPage();
            fFlushesSinceLastUse = 0;
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool DrawAtlas::createPages(AtlasGenerationCounter* generationCounter) {
    SkASSERT(SkIsPow2(fTextureWidth) && SkIsPow2(fTextureHeight));

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        // Proxies are uncreated at first
        fProxies[i] = nullptr;

        // set up allocated plots
        fPages[i].fPlotArray = std::make_unique<sk_sp<Plot>[]>(numPlotsX * numPlotsY);

        sk_sp<Plot>* currPlot = fPages[i].fPlotArray.get();
        for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
            for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
                uint32_t plotIndex = r * numPlotsX + c;
                currPlot->reset(new Plot(
                    i, plotIndex, generationCounter, x, y, fPlotWidth, fPlotHeight, fColorType,
                    fBytesPerPixel));

                // build LRU list
                fPages[i].fPlotList.addToHead(currPlot->get());
                ++currPlot;
            }
        }

    }

    return true;
}

bool DrawAtlas::activateNewPage(Recorder* recorder) {
    SkASSERT(fNumActivePages < this->maxPages());
    SkASSERT(!fProxies[fNumActivePages]);

    const Caps* caps = recorder->priv().caps();
    auto textureInfo = caps->getDefaultSampledTextureInfo(fColorType,
                                                          /*mipmapped=*/Mipmapped::kNo,
                                                          recorder->priv().isProtected(),
                                                          Renderable::kNo);
    fProxies[fNumActivePages] = TextureProxy::Make(
            caps, {fTextureWidth, fTextureHeight}, textureInfo, skgpu::Budgeted::kYes);
    if (!fProxies[fNumActivePages]) {
        return false;
    }

    if constexpr (kDumpAtlasData) {
        SkDebugf("activated page#: %d\n", fNumActivePages);
    }

    ++fNumActivePages;
    return true;
}

inline void DrawAtlas::deactivateLastPage() {
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
            SkDEBUGCODE(currPlot->resetListPtrs());
            fPages[lastPageIndex].fPlotList.addToHead(currPlot);
        }
    }

    // remove ref to the texture proxy
    fProxies[lastPageIndex].reset();
    --fNumActivePages;
}

void DrawAtlas::evictAllPlots() {
    PlotList::Iter plotIter;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            this->processEvictionAndResetRects(plot);
            plotIter.next();
        }
    }
}

DrawAtlasConfig::DrawAtlasConfig(int maxTextureSize, size_t maxBytes) {
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
        ? SkTPin<int>(SkPrevLog2(maxBytes), 0, std::size(kARGBDimensions) - 1)
        : 0;

    SkASSERT(kARGBDimensions[index].width() <= kMaxAtlasDim);
    SkASSERT(kARGBDimensions[index].height() <= kMaxAtlasDim);
    fARGBDimensions.set(std::min<int>(kARGBDimensions[index].width(), maxTextureSize),
                        std::min<int>(kARGBDimensions[index].height(), maxTextureSize));
    fMaxTextureSize = std::min<int>(maxTextureSize, kMaxAtlasDim);
}

SkISize DrawAtlasConfig::atlasDimensions(MaskFormat type) const {
    if (MaskFormat::kA8 == type) {
        // A8 is always 2x the ARGB dimensions, clamped to the max allowed texture size
        return { std::min<int>(2 * fARGBDimensions.width(), fMaxTextureSize),
                 std::min<int>(2 * fARGBDimensions.height(), fMaxTextureSize) };
    } else {
        return fARGBDimensions;
    }
}

SkISize DrawAtlasConfig::plotDimensions(MaskFormat type) const {
    if (MaskFormat::kA8 == type) {
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

}  // namespace skgpu::graphite
