/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawAtlas.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <tuple>

enum SkColorType : int;

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

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           GenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label) {
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   width, height,
                                                   plotWidth, plotHeight,
                                                   generationCounter,
                                                   allowMultitexturing,
                                                   useStorageTextures,
                                                   label));

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
DrawAtlas::DrawAtlas(MaskFormat maskFormat,
                     int width, int height,
                     int plotWidth, int plotHeight,
                     GenerationCounter* generationCounter,
                     AllowMultitexturing allowMultitexturing,
                     UseStorageTextures useStorageTextures,
                     std::string_view label)
        : fMaskFormat(maskFormat)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fUseStorageTextures(useStorageTextures)
        , fLabel(label)
        , fAtlasID(next_id())
        , fGenerationCounter(generationCounter)
        , fAtlasGeneration(fGenerationCounter->next())
        , fPrevFlushToken(Token::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fMaxPages(allowMultitexturing == AllowMultitexturing::kYes ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= kMaxPlots);
    SkASSERTF(fPlotWidth * numPlotsX == fTextureWidth,
             "Invalid DrawAtlas. Plot width: %d, texture width %d", fPlotWidth, fTextureWidth);
    SkASSERTF(fPlotHeight * numPlotsY == fTextureHeight,
              "Invalid DrawAtlas. Plot height: %d, texture height %d", fPlotHeight, fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;

    this->createPages(generationCounter);
}

inline void DrawAtlas::processEvictionAndResetRects(Plot* plot, bool freeData) {
    // Process evictions
    if (!plot->isEmpty()) {
        const PlotLocator& plotLocator = plot->plotLocator();
        for (PlotEvictionCallback* evictor : fEvictionCallbacks) {
            evictor->evict(plotLocator);
        }
        fAtlasGeneration = fGenerationCounter->next();
    }

    plot->resetRects(freeData);
}

inline void DrawAtlas::updatePlot(Plot* plot, AtlasLocator* atlasLocator) {
    int pageIdx = plot->pageIndex();
    this->makeMRU(plot, pageIdx);

    // The actual upload will be created in recordUploads().

    atlasLocator->updatePlotLocator(plot->plotLocator());
    SkDEBUGCODE(this->validate(*atlasLocator);)
}

bool DrawAtlas::addRectToPage(unsigned int pageIdx, int width, int height,
                              AtlasLocator* atlasLocator) {
    SkASSERT(fProxies[pageIdx]);

    // look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        if (plot->addRect(width, height, atlasLocator)) {
            this->updatePlot(plot, atlasLocator);
            return true;
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
                TextureProxyView view{fProxies[pageIdx]};
                SkASSERT(view);

                const void* dataPtr;
                SkIRect dstRect;
                std::tie(dataPtr, dstRect) = plot->prepareForUpload();
                if (dstRect.isEmpty()) {
                    continue;
                }

                std::vector<MipLevel> levels;
                levels.push_back({dataPtr, plot->rowBytes()});

                // Src and dst colorInfo are the same
                SkColorInfo colorInfo(MaskFormatToColorType(fMaskFormat),
                                      kUnknown_SkAlphaType,
                                      nullptr);
                const UploadSource uploadSource = UploadSource::Make(
                        recorder->priv().caps(), view, colorInfo, colorInfo, levels, dstRect);
                if (!uploadSource.isValid()) {
                    return false;
                }
                if (!dc->recordUpload(recorder,
                                      view,
                                      colorInfo,
                                      colorInfo,
                                      uploadSource,
                                      dstRect,
                                      /*ConditionalUploadContext=*/nullptr)) {
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
// Number of flushes before we'll try to evict a plot from a sparsely used page.
static constexpr auto kPlotUsedCountBeforeEvict = 8;
// Number of flushes beyond which we'll consider the atlas no longer in use.
static constexpr auto kAtlasRecentlyUsedCount = 128;

DrawAtlas::ErrorCode DrawAtlas::addRect(Recorder* recorder,
                                        int width, int height,
                                        AtlasLocator* atlasLocator) {
    if (width > fPlotWidth || height > fPlotHeight || width < 0 || height < 0) {
        return ErrorCode::kError;
    }

    // We permit zero-sized rects to allow inverse fills in the PathAtlases to work,
    // but we don't want to enter them in the Rectanizer. So we handle this special case here.
    // For text this should be caught at a higher level, but if not the only end result
    // will be rendering a degenerate quad.
    if (width == 0 || height == 0) {
        if (fNumActivePages == 0) {
            // Make sure we have a Page for the AtlasLocator to refer to
            this->activateNewPage(recorder);
        }
        atlasLocator->updateRect(SkIRect::MakeEmpty());
        // Use the MRU Plot from the first Page
        atlasLocator->updatePlotLocator(fPages[0].fPlotList.head()->plotLocator());
        return ErrorCode::kSucceeded;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (this->addRectToPage(pageIdx, width, height, atlasLocator)) {
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
                this->processEvictionAndResetRects(plot, /*freeData=*/false);
                SkDEBUGCODE(bool verify = )plot->addRect(width, height, atlasLocator);
                SkASSERT(verify);
                this->updatePlot(plot, atlasLocator);
                return ErrorCode::kSucceeded;
            }
        }
    } else {
        // If we haven't activated all the available pages, try to create a new one and add to it
        if (!this->activateNewPage(recorder)) {
            return ErrorCode::kError;
        }

        if (this->addRectToPage(fNumActivePages-1, width, height, atlasLocator)) {
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

DrawAtlas::ErrorCode DrawAtlas::addToAtlas(Recorder* recorder,
                                           int width, int height, const void* image,
                                           AtlasLocator* atlasLocator) {
    ErrorCode ec = this->addRect(recorder, width, height, atlasLocator);
    if (ec == ErrorCode::kSucceeded) {
        Plot* plot = this->findPlot(*atlasLocator);
        plot->copySubImage(*atlasLocator, image);
    }

    return ec;
}

SkPixmap DrawAtlas::prepForRender(const AtlasLocator& locator,
                                  int padding,
                                  std::optional<SkColor> initialColor) {
    Plot* plot = this->findPlot(locator);
    return plot->prepForRender(locator, padding, initialColor);
}

void DrawAtlas::compact(Token startTokenForNextFlush) {
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

        // For all pages but the last one, update number of flushes since used, and check to see
        // if there are any in the first pages that the last page can safely upload to.
        for (uint32_t pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("page %u: ", pageIndex);
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
            SkDebugf("page %u: ", lastPageIndex);
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
            } else if (plot->lastUseToken() != Token::InvalidToken()) {
                // otherwise if aged out just evict it.
                this->processEvictionAndResetRects(plot, /*freeData=*/false);
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
        if (!availablePlots.empty() && usedPlots && usedPlots <= fNumPlots / 4) {
            plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // If this plot was used recently
                int plotFlushes = plot->flushesSinceLastUsed();
                if (kPlotUsedCountBeforeEvict <= plotFlushes &&
                    plotFlushes <= kPlotRecentlyUsedCount) {
                    // See if there's room in an lower index page and if so evict.
                    // We need to be somewhat harsh here so that a handful of plots that are
                    // consistently in use don't end up locking the page in memory.
                    if (!availablePlots.empty()) {
                        this->processEvictionAndResetRects(plot, /*freeData=*/true);
                        this->processEvictionAndResetRects(availablePlots.back(),
                                                           /*freeData=*/false);
                        availablePlots.pop_back();
                        --usedPlots;
                    }
                    if (usedPlots == 0 || availablePlots.empty()) {
                        break;
                    }
                }
                plotIter.next();
            }
        }

        // If none of the plots in the last page have been used recently, delete it.
        if (usedPlots == 0) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("delete %u\n", fNumActivePages-1);
            }

            this->deactivateLastPage();
            fFlushesSinceLastUse = 0;
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool DrawAtlas::createPages(GenerationCounter* generationCounter) {
    SkASSERT(SkIsPow2(fTextureWidth) && SkIsPow2(fTextureHeight));

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        // Proxies are uncreated at first
        fProxies[i] = nullptr;

        // set up allocated plots
        fPages[i].fPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(numPlotsX * numPlotsY);

        auto* currPlot = fPages[i].fPlotArray.get();
        for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
            for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
                uint32_t plotIndex = r * numPlotsX + c;
                *currPlot = Plot::Make(i,
                                       plotIndex,
                                       generationCounter,
                                       x, y,
                                       fPlotWidth, fPlotHeight,
                                       fMaskFormat);

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

    auto ct = MaskFormatToColorType(fMaskFormat);

    const Caps* caps = recorder->priv().caps();
    auto textureInfo = fUseStorageTextures == UseStorageTextures::kYes
                               ? caps->getDefaultStorageTextureInfo(ct)
                               : caps->getDefaultSampledTextureInfo(ct,
                                                                    Mipmapped::kNo,
                                                                    recorder->priv().isProtected(),
                                                                    Renderable::kNo);
    fProxies[fNumActivePages] = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   {fTextureWidth, fTextureHeight},
                                                   textureInfo,
                                                   fLabel,
                                                   skgpu::Budgeted::kYes);
    if (!fProxies[fNumActivePages]) {
        return false;
    }

    if constexpr (kDumpAtlasData) {
        SkDebugf("activated page#: %u\n", fNumActivePages);
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
            this->processEvictionAndResetRects(currPlot, /*freeData=*/true);
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

void DrawAtlas::markUsedPlotsAsFull() {
    PlotList::Iter plotIter;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            plot->markFullIfUsed();
            plotIter.next();
        }
    }
}

void DrawAtlas::freeGpuResources(Token token) {
    PlotList::Iter plotIter;
    for (int pageIndex = (int)(fNumActivePages)-1; pageIndex >= 0; --pageIndex) {
        const Page& currPage = fPages[pageIndex];
        plotIter.init(currPage.fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            if (plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
                // This page is in use and we can only deactivate pages from high index
                // to low index, so bail.
                return;
            }
            plotIter.next();
        }
        this->deactivateLastPage();
    }
}

void DrawAtlas::evictAllPlots() {
    PlotList::Iter plotIter;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            this->processEvictionAndResetRects(plot, /*freeData=*/true);
            plotIter.next();
        }
    }
}

#if defined(GPU_TEST_UTILS)
int DrawAtlas::numAllocatedPlots() const {
    return this->iteratePlots([](const Plot* plot) {
        return plot->hasAllocation();
    });
}

int DrawAtlas::numNonEmptyPlots() const {
    return this->iteratePlots([](const Plot* plot) {
        return !plot->isEmpty();
    });
}
#endif

DrawAtlas::Plot::Plot(int pageIndex,
                      int plotIndex,
                      GenerationCounter* generationCounter,
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

DrawAtlas::Plot::~Plot() = default;

bool DrawAtlas::Plot::addRect(int width, int height, AtlasLocator* atlasLocator) {
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

void* DrawAtlas::Plot::dataAt(SkIPoint atlasPoint) {
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

SkPixmap DrawAtlas::Plot::prepForRender(const AtlasLocator& al,
                                        int padding,
                                        std::optional<SkColor> initialColor) {
    SkASSERT(padding >= 0);
    auto info = SkImageInfo::Make(
            al.dimensions(), MaskFormatToColorType(fMaskFormat), kOpaque_SkAlphaType);
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

void DrawAtlas::Plot::copySubImage(const AtlasLocator& al, const void* image) {
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

std::pair<const void*, SkIRect> DrawAtlas::Plot::prepareForUpload() {
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

void DrawAtlas::Plot::resetRects(bool freeData) {
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
