/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawAtlas_DEFINED
#define skgpu_graphite_DrawAtlas_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/Token.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class SkPixmap;
enum SkColorType : int;

namespace skgpu {
enum class MaskFormat : int;
}

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
    // These are both restricted by the space they occupy in the PlotLocator.
    // maxPages was limited by being crammed into the glyph uvs in Ganesh
    // (no longer necessary in Graphite).
    // maxPlots is also limited by the fPlotAlreadyUpdated bitfield in
    // BulkUsePlotUpdater (below).
    static constexpr auto kMaxMultitexturePages = 4;
    static constexpr int kMaxPlots = 32;

    /**
     * Keep track of generation number for atlases and Plots.
     */
    class GenerationCounter {
    public:
        static constexpr uint64_t kInvalidGeneration = 0;
        uint64_t next() { return fGeneration++; }

    private:
        uint64_t fGeneration{1};
    };

    class PlotLocator;
    class AtlasLocator;

    /**
     * An interface for eviction callbacks. Whenever an atlas evicts a specific PlotLocator,
     * it will call all the registered listeners so they can process the eviction.
     */
    class PlotEvictionCallback {
    public:
        virtual ~PlotEvictionCallback() = default;
        virtual void evict(PlotLocator) = 0;
    };

    class BulkUsePlotUpdater;

    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    /** Should the atlas use storage textures? */
    enum class UseStorageTextures : bool { kNo, kYes };

    /**
     * Returns a DrawAtlas.
     *  @param maskFormat          The maskFormat which this atlas will store.
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
    static std::unique_ptr<DrawAtlas> Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           GenerationCounter* generationCounter,
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
    // Returns a Pixmap pointing to the backing data for the locator. Optionally, the caller can
    // provide an inset that is applied to all four sides. This is useful for use cases that need
    // to leave space between items in the atlas. The pixmap will exclude the padding. The entire
    // atlas is cleared to zero when allocated. By passing an initialColor here, the caller can
    // re-clear the entire locator's rect (including any padding) to any color.
    SkPixmap prepForRender(const AtlasLocator&,
                           int padding = 0,
                           std::optional<SkColor> initialColor = {});
    bool recordUploads(DrawContext*, Recorder*);

    const sk_sp<TextureProxy>* getProxies() const { return fProxies; }

    uint32_t atlasID() const { return fAtlasID; }
    uint64_t atlasGeneration() const { return fAtlasGeneration; }
    uint32_t numActivePages() const { return fNumActivePages; }
    unsigned int numPlots() const { return fNumPlots; }
    SkISize plotSize() const { return {fPlotWidth, fPlotHeight}; }
    uint32_t getListIndex(const PlotLocator& locator);

    bool hasID(const PlotLocator& plotLocator);

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(const AtlasLocator& atlasLocator, Token token);

    void setLastUseTokenBulk(const BulkUsePlotUpdater& updater, Token token);

    void compact(Token startTokenForNextFlush);

    // Mark all plots with any content as full. Used only with Vello because it can't do
    // new renders to a texture without a clear.
    void markUsedPlotsAsFull();

    // Will try to clear out any GPU resources that aren't needed for any pending uploads or draws.
    // TODO: Delete backing data for Plots that don't have pending uploads.
    void freeGpuResources(Token token);

    void evictAllPlots();

    uint32_t maxPages() const {
        return fMaxPages;
    }

#if defined(GPU_TEST_UTILS)
    int numAllocatedPlots() const;
    int numNonEmptyPlots() const;
#endif

private:
    class Plot;
    using PlotList = SkTInternalLList<Plot>;

    DrawAtlas(MaskFormat,
              int width,
              int height,
              int plotWidth,
              int plotHeight,
              GenerationCounter* generationCounter,
              AllowMultitexturing allowMultitexturing,
              UseStorageTextures useStorageTextures,
              std::string_view label);

    bool addRectToPage(unsigned int pageIdx, int width, int height, AtlasLocator*);

    void updatePlot(Plot* plot, AtlasLocator*);

    void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    Plot* findPlot(const AtlasLocator& atlasLocator);

    void internalSetLastUseToken(Plot* plot, uint32_t pageIdx, Token token);

    bool createPages(GenerationCounter*);
    bool activateNewPage(Recorder*);
    void deactivateLastPage();

#if defined(GPU_TEST_UTILS)
    template <typename F> int iteratePlots(F&& func) const {
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
#endif

    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void processEvictionAndResetRects(Plot* plot, bool freeData);

    MaskFormat         fMaskFormat;
    int                fTextureWidth;
    int                fTextureHeight;
    int                fPlotWidth;
    int                fPlotHeight;
    unsigned int       fNumPlots;
    UseStorageTextures fUseStorageTextures;
    const std::string  fLabel;
    uint32_t           fAtlasID;   // unique identifier for this atlas

    // A counter to track the atlas eviction state for Glyphs. Each Glyph has a PlotLocator
    // which contains its current generation. When the atlas evicts a plot, it increases
    // the generation counter. If a Glyph's generation is less than the atlas's
    // generation, then it knows it's been evicted and is either free to be deleted or
    // re-added to the atlas if necessary.
    GenerationCounter* const fGenerationCounter;
    uint64_t                 fAtlasGeneration;

    // nextFlushToken() value at the end of the previous DrawPass
    // TODO: rename
    Token fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    // TODO: rename
    int fFlushesSinceLastUse;

    std::vector<PlotEvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<std::unique_ptr<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    sk_sp<TextureProxy> fProxies[kMaxMultitexturePages];
    Page fPages[kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const AtlasLocator& atlasLocator) const;)
};

class DrawAtlas::PlotLocator {
public:
    PlotLocator(uint32_t pageIdx, uint32_t plotIdx, uint64_t generation)
            : fGenID(generation), fPlotIndex(plotIdx), fPageIndex(pageIdx) {
        SkASSERT(pageIdx < kMaxMultitexturePages);
        SkASSERT(plotIdx < kMaxPlots);
        SkASSERT(generation < ((uint64_t)1 << 48));
    }

    PlotLocator() : fGenID(GenerationCounter::kInvalidGeneration), fPlotIndex(0), fPageIndex(0) {}

    bool isValid() const {
        return fGenID != GenerationCounter::kInvalidGeneration || fPlotIndex != 0 ||
               fPageIndex != 0;
    }

    void makeInvalid() {
        fGenID = GenerationCounter::kInvalidGeneration;
        fPlotIndex = 0;
        fPageIndex = 0;
    }

    bool operator==(const PlotLocator& other) const {
        return fGenID == other.fGenID && fPlotIndex == other.fPlotIndex &&
               fPageIndex == other.fPageIndex;
    }

    uint32_t pageIndex() const { return fPageIndex; }
    uint32_t plotIndex() const { return fPlotIndex; }
    uint64_t genID() const { return fGenID; }

private:
    uint64_t fGenID : 48;
    uint64_t fPlotIndex : 8;
    uint64_t fPageIndex : 8;
};

// AtlasLocator handles atlas position information. It keeps a left-top, right-bottom pair of
// encoded UV coordinates. The bits 13 & 14 of the U coordinates hold the atlas page index.
// This information is handed directly as is from fUVs. This encoding has the nice property
// that width = fUVs[2] - fUVs[0]; the page encoding in the top bits subtracts to zero.
class DrawAtlas::AtlasLocator {
public:
    std::array<uint16_t, 4> getUVs() const { return fUVs; }

    void invalidatePlotLocator() { fPlotLocator.makeInvalid(); }

    // TODO: Remove the small path renderer's use of this for eviction
    PlotLocator plotLocator() const { return fPlotLocator; }

    uint32_t pageIndex() const { return fPlotLocator.pageIndex(); }

    uint32_t plotIndex() const { return fPlotLocator.plotIndex(); }

    uint64_t genID() const { return fPlotLocator.genID(); }

    SkIPoint topLeft() const { return {fUVs[0] & 0x1FFF, fUVs[1]}; }

    SkISize dimensions() const { return {fUVs[2] - fUVs[0], fUVs[3] - fUVs[1]}; }

    uint16_t width() const { return fUVs[2] - fUVs[0]; }

    uint16_t height() const { return fUVs[3] - fUVs[1]; }

    void insetSrc(int padding) {
        SkASSERT(2 * padding <= this->width());
        SkASSERT(2 * padding <= this->height());

        fUVs[0] += padding;
        fUVs[1] += padding;
        fUVs[2] -= padding;
        fUVs[3] -= padding;
    }

    void updatePlotLocator(PlotLocator p) {
        fPlotLocator = p;
        SkASSERT(fPlotLocator.pageIndex() <= 3);
        uint16_t page = fPlotLocator.pageIndex() << 13;
        fUVs[0] = (fUVs[0] & 0x1FFF) | page;
        fUVs[2] = (fUVs[2] & 0x1FFF) | page;
    }

    void updateRect(SkIRect rect) {
        SkASSERT(rect.fLeft <= rect.fRight);
        SkASSERT(rect.fRight <= 0x1FFF);
        fUVs[0] = (fUVs[0] & 0xE000) | SkToU16(rect.fLeft);
        fUVs[1] = SkToU16(rect.fTop);
        fUVs[2] = (fUVs[2] & 0xE000) | SkToU16(rect.fRight);
        fUVs[3] = SkToU16(rect.fBottom);
    }

private:
    PlotLocator fPlotLocator{0, 0, GenerationCounter::kInvalidGeneration};

    // The inset padded bounds in the atlas in the lower 13 bits, and page index in bits 13 &
    // 14 of the Us.
    std::array<uint16_t, 4> fUVs{0, 0, 0, 0};
};

/**
 * A class which can be handed back to an atlas for updating plots in bulk.  The
 * current max number of plots per page an atlas can handle is 32. If in the future
 * this is insufficient then we can move to a 64 bit int.
 */
class DrawAtlas::BulkUsePlotUpdater {
public:
    BulkUsePlotUpdater() { memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated)); }
    BulkUsePlotUpdater(const BulkUsePlotUpdater& that) : fPlotsToUpdate(that.fPlotsToUpdate) {
        memcpy(fPlotAlreadyUpdated, that.fPlotAlreadyUpdated, sizeof(fPlotAlreadyUpdated));
    }

    bool add(const AtlasLocator& atlasLocator) {
        int plotIdx = atlasLocator.plotIndex();
        int pageIdx = atlasLocator.pageIndex();
        if (this->find(pageIdx, plotIdx)) {
            return false;
        }
        this->set(pageIdx, plotIdx);
        return true;
    }

    void reset() {
        fPlotsToUpdate.clear();
        memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
    }

    struct PlotData {
        PlotData(int pageIdx, int plotIdx) : fPageIndex(pageIdx), fPlotIndex(plotIdx) {}
        uint32_t fPageIndex;
        uint32_t fPlotIndex;
    };

    int count() const { return fPlotsToUpdate.size(); }

    const PlotData& plotData(int index) const { return fPlotsToUpdate[index]; }

private:
    bool find(int pageIdx, int index) const {
        SkASSERT(index < kMaxPlots);
        return (fPlotAlreadyUpdated[pageIdx] >> index) & 1;
    }

    void set(int pageIdx, int index) {
        SkASSERT(!this->find(pageIdx, index));
        fPlotAlreadyUpdated[pageIdx] |= (1 << index);
        fPlotsToUpdate.push_back(PlotData(pageIdx, index));
    }

    inline static constexpr int kMinItems = 4;
    skia_private::STArray<kMinItems, PlotData, true> fPlotsToUpdate;
    // TODO: increase this to uint64_t to allow more plots per page
    uint32_t fPlotAlreadyUpdated[kMaxMultitexturePages];
};

/**
 * The backing texture for an atlas is broken into a spatial grid of Plots. The Plots
 * keep track of subimage placement via their Rectanizer. A Plot may be subclassed if
 * the atlas class needs to track additional information. Plots are initialized to zero
 * for all color types.
 */
class DrawAtlas::Plot final {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);

    Plot(int pageIndex,
         int plotIndex,
         GenerationCounter* generationCounter,
         int offX,
         int offY,
         int width,
         int height,
         MaskFormat);

    Plot(const Plot&) = delete;
    Plot(Plot&&) = delete;

    Plot& operator=(const Plot&) = delete;
    Plot& operator=(Plot&&) = delete;

public:
    static std::unique_ptr<Plot> Make(int pageIndex,
                                      int plotIndex,
                                      GenerationCounter* generationCounter,
                                      int offX,
                                      int offY,
                                      int width,
                                      int height,
                                      MaskFormat format) {
        return std::unique_ptr<Plot>{new Plot{
                pageIndex, plotIndex, generationCounter, offX, offY, width, height, format}};
    }

    ~Plot();

    uint32_t pageIndex() const { return this->plotLocator().pageIndex(); }

    /**
     * genID() is incremented when the plot is evicted due to a atlas spill. It is used to
     * know if a particular subimage is still present in the atlas.
     */
    uint64_t genID() const { return fGenID; }
    PlotLocator plotLocator() const {
        SkASSERT(fPlotLocator.isValid());
        return fPlotLocator;
    }

    size_t bpp() const { return MaskFormatBytesPerPixel(fMaskFormat); }
    size_t rowBytes() const { return fWidth * this->bpp(); }

    /**
     * To add data to the Plot, first call addRect to see if it's possible. If successful,
     * use the atlasLocator to copy data to the location using copySubImage() or use
     * prepForRender() to software rasterize to the location.
     */
    bool addRect(int width, int height, AtlasLocator* atlasLocator);

    void copySubImage(const AtlasLocator& atlasLocator, const void* image);

    // Returns a Pixmap pointing to the backing data for the locator. Optionally, the caller can
    // provide an inset that is applied to all four sides. This is useful for use cases that need
    // to leave space between items in the atlas. The pixmap will exclude the padding. The entire
    // Plot is cleared to zero when allocated. By passing an initialColor here, the caller can
    // re-clear the entire locator's rect (including any padding) to any color.
    SkPixmap prepForRender(const AtlasLocator&,
                           int padding = 0,
                           std::optional<SkColor> initialColor = {});

    /**
     * To manage the lifetime of a plot, we use two tokens. We use the last upload token to
     * know when we can 'piggy back' uploads, i.e. if the last upload hasn't been flushed to
     * the gpu, we don't need to issue a new upload even if we update the cpu backing store. We
     * use lastUse to determine when we can evict a plot from the cache, i.e. if the last use
     * has already flushed through the gpu then we can reuse the plot.
     */
    Token lastUseToken() const { return fLastUse; }
    void setLastUseToken(Token token) { fLastUse = token; }

    int flushesSinceLastUsed() { return fFlushesSinceLastUse; }
    void resetFlushesSinceLastUsed() { fFlushesSinceLastUse = 0; }
    void incFlushesSinceLastUsed() { fFlushesSinceLastUse++; }

    bool needsUpload() { return !fDirtyRect.isEmpty(); }
    std::pair<const void*, SkIRect> prepareForUpload();
    // Re-initialize Plot. The client should ensure that they process any eviction callbacks
    // before calling this, otherwise any cached references will point to invalid data.
    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void resetRects(bool freeData);

    void markFullIfUsed() { fIsFull = !fDirtyRect.isEmpty(); }
    bool isEmpty() const { return fRectanizer.percentFull() == 0; }
    bool hasAllocation() const { return fData != nullptr; }

#ifdef SK_DEBUG
    void resetListPtrs() {
        fPrev = fNext = nullptr;
        fList = nullptr;
    }
#endif

private:
    void* dataAt(SkIPoint atlasPoint);

    Token fLastUse;
    int fFlushesSinceLastUse;

    GenerationCounter* const fGenerationCounter;
    uint64_t fGenID;
    PlotLocator fPlotLocator;
    std::unique_ptr<std::byte[]> fData;
    const int fWidth;
    const int fHeight;
    const int fX;
    const int fY;
    RectanizerSkyline fRectanizer;
    const SkIPoint16 fOffset;  // the offset of the plot in the backing texture
    const MaskFormat fMaskFormat;
    SkIRect fDirtyRect;  // area in the Plot that needs to be uploaded
    bool fIsFull;
};

inline uint32_t DrawAtlas::getListIndex(const PlotLocator& locator) {
    return locator.pageIndex() * fNumPlots + locator.plotIndex();
}

inline bool DrawAtlas::hasID(const PlotLocator& plotLocator) {
    if (!plotLocator.isValid()) {
        return false;
    }

    uint32_t plot = plotLocator.plotIndex();
    uint32_t page = plotLocator.pageIndex();
    uint64_t plotGeneration = fPages[page].fPlotArray[plot]->genID();
    uint64_t locatorGeneration = plotLocator.genID();
    return plot < fNumPlots && page < fNumActivePages && plotGeneration == locatorGeneration;
}

inline void DrawAtlas::setLastUseToken(const AtlasLocator& atlasLocator, Token token) {
    Plot* plot = this->findPlot(atlasLocator);
    this->internalSetLastUseToken(plot, atlasLocator.pageIndex(), token);
}

inline void DrawAtlas::setLastUseTokenBulk(const BulkUsePlotUpdater& updater, Token token) {
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

inline DrawAtlas::Plot* DrawAtlas::findPlot(const AtlasLocator& atlasLocator) {
    SkASSERT(this->hasID(atlasLocator.plotLocator()));
    uint32_t pageIdx = atlasLocator.pageIndex();
    uint32_t plotIdx = atlasLocator.plotIndex();
    return fPages[pageIdx].fPlotArray[plotIdx].get();
}

inline void DrawAtlas::internalSetLastUseToken(Plot* plot, uint32_t pageIdx, Token token) {
    this->makeMRU(plot, pageIdx);
    plot->setLastUseToken(token);
}

}  // namespace skgpu::graphite

#endif
