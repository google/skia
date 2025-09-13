/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawAtlas_DEFINED
#define skgpu_graphite_DrawAtlas_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/AtlasTypes.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class SkAutoPixmapStorage;
enum SkColorType : int;

namespace skgpu::graphite {

class DrawContext;
class Recorder;
class TextureProxy;

/**
 * TODO: the process described here is tentative, and this comment revised once locked down.
 *
 * This class manages one or more atlas textures on behalf of primitive draws in Device. The
 * drawing processes that use the atlas add preceding UploadTasks when generating RenderPassTasks.
 * The class provides facilities for using DrawTokens to detect data hazards. Plots that need
 * uploads are tracked until it is impossible to add data without overwriting texels read by draws
 * that have not yet been snapped to a RenderPassTask. At that point, the atlas will attempt to
 * allocate a new atlas texture (or "page") of the same size, up to a maximum number of textures,
 * and upload to that texture. If that's not possible, then the atlas will fail to add a subimage.
 * This gives the Device the chance to end the current draw, snap a RenderpassTask, and begin a new
 * one. Additional uploads will then succeed.
 *
 * When the atlas has multiple pages, new uploads are prioritized to the lower index pages, i.e.,
 * it will try to upload to page 0 before page 1 or 2. To keep the atlas from continually using
 * excess space, periodic garbage collection is needed to shift data from the higher index pages to
 * the lower ones, and then eventually remove any pages that are no longer in use. "In use" is
 * determined by using the AtlasToken system: After a DrawPass is snapped a subarea of the page, or
 * "plot" is checked to see whether it was used in that DrawPass. If less than a quarter of the
 * plots have been used recently (within kPlotRecentlyUsedCount iterations) and there are available
 * plots in lower index pages, the higher index page will be deactivated, and its glyphs will
 * gradually migrate to other pages via the usual upload system.
 *
 * Garbage collection is initiated by the DrawAtlas's client via the compact() method.
 */
class DrawAtlas {
public:
    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    /** Should the atlas use storage textures? */
    enum class UseStorageTextures : bool { kNo, kYes };

    /**
     * Returns a DrawAtlas.
     *  @param ct                  The colorType which this atlas will store.
     *  @param bpp                 Size in bytes of each pixel.
     *  @param width               Width in pixels of the atlas.
     *  @param height              Height in pixels of the atlas.
     *  @param plotWidth           The width of each plot. width/plotWidth should be an integer.
     *  @param plotWidth           The height of each plot. height/plotHeight should be an integer.
     *  @param atlasGeneration     A pointer to the context's generation counter.
     *  @param allowMultitexturing Can the atlas use more than one texture.
     *  @param useStorageTextures  Should the atlas use storage textures.
     *  @param evictor             A pointer to an eviction callback class.
     *  @param label               Label for texture resources.
     *
     *  @return                    An initialized DrawAtlas, or nullptr if creation fails.
     */
    static std::unique_ptr<DrawAtlas> Make(SkColorType ct, size_t bpp,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AtlasGenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label);

    /**
     * Adds a width x height subimage to the atlas. Upon success it returns 'kSucceeded' and returns
     * the ID and the subimage's coordinates in the backing texture. 'kTryAgain' is returned if
     * the subimage cannot fit in the atlas without overwriting texels that will be read in the
     * current list of draws. This indicates that the Device should end its current draw, snap a
     * DrawPass, and begin another before adding more data. 'kError' will be returned when some
     * unrecoverable error was encountered while trying to add the subimage. In this case the draw
     * being created should be discarded.
     *
     * This tracking does not generate UploadTasks per se. Instead, when the RenderPassTask is
     * ready to be snapped, recordUploads() will be called by the Device and that will generate the
     * necessary UploadTasks. If the useCachedUploads argument in recordUploads() is true, this
     * will generate uploads for the entire area of each Plot that has changed since the last
     * eviction. Otherwise it will only generate uploads for newly added changes.
     *
     * NOTE: When a draw that reads from the atlas is added to the DrawList, the client using this
     * DrawAtlas must immediately call 'setLastUseToken' with the currentToken from the Recorder,
     * otherwise the next call to addToAtlas might cause the previous data to be overwritten before
     * it has been read.
     */

    enum class ErrorCode {
        kError,
        kSucceeded,
        kTryAgain
    };

    ErrorCode addToAtlas(Recorder*, int width, int height, const void* image, AtlasLocator*);
    ErrorCode addRect(Recorder*, int width, int height, AtlasLocator*);
    // Reset Pixmap to point to backing data for the AtlasLocator's Plot.
    // Return relative location within the Plot, as indicated by the AtlasLocator.
    SkIPoint prepForRender(const AtlasLocator&, SkAutoPixmapStorage*);
    bool recordUploads(DrawContext*, Recorder*);

    const sk_sp<TextureProxy>* getProxies() const { return fProxies; }

    uint32_t atlasID() const { return fAtlasID; }
    uint64_t atlasGeneration() const { return fAtlasGeneration; }
    uint32_t numActivePages() const { return fNumActivePages; }
    unsigned int numPlots() const { return fNumPlots; }
    SkISize plotSize() const { return {fPlotWidth, fPlotHeight}; }
    uint32_t getListIndex(const PlotLocator& locator) {
        return locator.pageIndex() * fNumPlots + locator.plotIndex();
    }

    bool hasID(const PlotLocator& plotLocator) {
        if (!plotLocator.isValid()) {
            return false;
        }

        uint32_t plot = plotLocator.plotIndex();
        uint32_t page = plotLocator.pageIndex();
        uint64_t plotGeneration = fPages[page].fPlotArray[plot]->genID();
        uint64_t locatorGeneration = plotLocator.genID();
        return plot < fNumPlots && page < fNumActivePages && plotGeneration == locatorGeneration;
    }

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(const AtlasLocator& atlasLocator, AtlasToken token) {
        Plot* plot = this->findPlot(atlasLocator);
        this->internalSetLastUseToken(plot, atlasLocator.pageIndex(), token);
    }

    void setLastUseTokenBulk(const BulkUsePlotUpdater& updater,
                             AtlasToken token) {
        int count = updater.count();
        for (int i = 0; i < count; i++) {
            const BulkUsePlotUpdater::PlotData& pd = updater.plotData(i);
            // it's possible we've added a plot to the updater and subsequently the plot's page
            // was deleted -- so we check to prevent a crash
            if (pd.fPageIndex < fNumActivePages) {
                Plot* plot = fPages[pd.fPageIndex].fPlotArray[pd.fPlotIndex].get();
                this->internalSetLastUseToken(plot, pd.fPageIndex, token);
            }
        }
    }

    void compact(AtlasToken startTokenForNextFlush);

    // Mark all plots with any content as full. Used only with Vello because it can't do
    // new renders to a texture without a clear.
    void markUsedPlotsAsFull();

    // Will try to clear out any GPU resources that aren't needed for any pending uploads or draws.
    // TODO: Delete backing data for Plots that don't have pending uploads.
    void freeGpuResources(AtlasToken token);

    void evictAllPlots();

    uint32_t maxPages() const {
        return fMaxPages;
    }

#if defined(GPU_TEST_UTILS)
    template <typename F>
    int iteratePlots(F&& func) const {
        int count = 0;
        PlotList::Iter plotIter;
        for (uint32_t pageIndex = 0; pageIndex < this->maxPages(); ++pageIndex) {
            plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                if (func(plot)) {
                    count++;
                }
                plotIter.next();
            }
        }
        return count;
    }

    int numAllocatedPlots() const;
    int numNonEmptyPlots() const;
#endif

private:
    DrawAtlas(SkColorType, size_t bpp,
              int width, int height, int plotWidth, int plotHeight,
              AtlasGenerationCounter* generationCounter,
              AllowMultitexturing allowMultitexturing,
              UseStorageTextures useStorageTextures,
              std::string_view label);

    bool addRectToPage(unsigned int pageIdx, int width, int height, AtlasLocator*);

    void updatePlot(Plot* plot, AtlasLocator*);

    inline void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    Plot* findPlot(const AtlasLocator& atlasLocator) {
        SkASSERT(this->hasID(atlasLocator.plotLocator()));
        uint32_t pageIdx = atlasLocator.pageIndex();
        uint32_t plotIdx = atlasLocator.plotIndex();
        return fPages[pageIdx].fPlotArray[plotIdx].get();
    }

    void internalSetLastUseToken(Plot* plot, uint32_t pageIdx, AtlasToken token) {
        this->makeMRU(plot, pageIdx);
        plot->setLastUseToken(token);
    }

    bool createPages(AtlasGenerationCounter*);
    bool activateNewPage(Recorder*);
    void deactivateLastPage();

    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void processEvictionAndResetRects(Plot* plot, bool freeData);

    SkColorType           fColorType;
    size_t                fBytesPerPixel;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;
    UseStorageTextures    fUseStorageTextures;
    const std::string     fLabel;
    uint32_t              fAtlasID;   // unique identifier for this atlas

    // A counter to track the atlas eviction state for Glyphs. Each Glyph has a PlotLocator
    // which contains its current generation. When the atlas evicts a plot, it increases
    // the generation counter. If a Glyph's generation is less than the atlas's
    // generation, then it knows it's been evicted and is either free to be deleted or
    // re-added to the atlas if necessary.
    AtlasGenerationCounter* const fGenerationCounter;
    uint64_t                      fAtlasGeneration;

    // nextFlushToken() value at the end of the previous DrawPass
    // TODO: rename
    AtlasToken fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    // TODO: rename
    int fFlushesSinceLastUse;

    std::vector<PlotEvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    sk_sp<TextureProxy> fProxies[PlotLocator::kMaxMultitexturePages];
    Page fPages[PlotLocator::kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const AtlasLocator& atlasLocator) const;)
};

// For text there are three atlases (A8, 565, ARGB) that are kept in relation with one another. In
// general, because A8 is the most frequently used mask format its dimensions are 2x the 565 and
// ARGB dimensions, with the constraint that an atlas size will always contain at least one plot.
// Since the ARGB atlas takes the most space, its dimensions are used to size the other two atlases.
class DrawAtlasConfig {
public:
    // The capabilities of the GPU define maxTextureSize. The client provides maxBytes, and this
    // represents the largest they want a single atlas texture to be. Due to multitexturing, we
    // may expand temporarily to use more space as needed.
    DrawAtlasConfig(int maxTextureSize, size_t maxBytes);

    SkISize atlasDimensions(MaskFormat type) const;
    SkISize plotDimensions(MaskFormat type) const;

private:
    // On some systems texture coordinates are represented using half-precision floating point
    // with 11 significant bits, which limits the largest atlas dimensions to 2048x2048.
    // For simplicity we'll use this constraint for all of our atlas textures.
    // This can be revisited later if we need larger atlases.
    inline static constexpr int kMaxAtlasDim = 2048;

    SkISize fARGBDimensions;
    int     fMaxTextureSize;
};

}  // namespace skgpu::graphite

#endif
