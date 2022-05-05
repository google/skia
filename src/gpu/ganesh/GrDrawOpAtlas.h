/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpAtlas_DEFINED
#define GrDrawOpAtlas_DEFINED

#include <cmath>
#include <vector>

#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

class GrOnFlushResourceProvider;
class GrProxyProvider;
class GrResourceProvider;
class GrTextureProxy;

/**
 * This class manages one or more atlas textures on behalf of GrDrawOps. The draw ops that use the
 * atlas perform texture uploads when preparing their draws during flush. The class provides
 * facilities for using GrDrawOpUploadToken to detect data hazards. Op's uploads are performed in
 * "ASAP" mode until it is impossible to add data without overwriting texels read by draws that
 * have not yet executed on the gpu. At that point, the atlas will attempt to allocate a new
 * atlas texture (or "page") of the same size, up to a maximum number of textures, and upload
 * to that texture. If that's not possible, the uploads are performed "inline" between draws. If a
 * single draw would use enough subimage space to overflow the atlas texture then the atlas will
 * fail to add a subimage. This gives the op the chance to end the draw and begin a new one.
 * Additional uploads will then succeed in inline mode.
 *
 * When the atlas has multiple pages, new uploads are prioritized to the lower index pages, i.e.,
 * it will try to upload to page 0 before page 1 or 2. To keep the atlas from continually using
 * excess space, periodic garbage collection is needed to shift data from the higher index pages to
 * the lower ones, and then eventually remove any pages that are no longer in use. "In use" is
 * determined by using the GrDrawUploadToken system: After a flush each subarea of the page
 * is checked to see whether it was used in that flush; if it is not, a counter is incremented.
 * Once that counter reaches a threshold that subarea is considered to be no longer in use.
 *
 * Garbage collection is initiated by the GrDrawOpAtlas's client via the compact() method. One
 * solution is to make the client a subclass of GrOnFlushCallbackObject, register it with the
 * GrContext via addOnFlushCallbackObject(), and the client's postFlush() method calls compact()
 * and passes in the given GrDrawUploadToken.
 */
class GrDrawOpAtlas {
public:
    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    /**
     * Returns a GrDrawOpAtlas. This function can be called anywhere, but the returned atlas
     * should only be used inside of GrMeshDrawOp::onPrepareDraws.
     *  @param ct               The colorType which this atlas will store
     *  @param bpp              Size in bytes of each pixel
     *  @param width            width in pixels of the atlas
     *  @param height           height in pixels of the atlas
     *  @param numPlotsX        The number of plots the atlas should be broken up into in the X
     *                          direction
     *  @param numPlotsY        The number of plots the atlas should be broken up into in the Y
     *                          direction
     *  @param atlasGeneration  a pointer to the context's generation counter.
     *  @param allowMultitexturing Can the atlas use more than one texture.
     *  @param evictor          A pointer to an eviction callback class.
     *
     *  @return                 An initialized GrDrawOpAtlas, or nullptr if creation fails
     */
    static std::unique_ptr<GrDrawOpAtlas> Make(GrProxyProvider*,
                                               const GrBackendFormat& format,
                                               SkColorType ct, size_t bpp,
                                               int width, int height,
                                               int plotWidth, int plotHeight,
                                               skgpu::AtlasGenerationCounter* generationCounter,
                                               AllowMultitexturing allowMultitexturing,
                                               skgpu::PlotEvictionCallback* evictor);

    /**
     * Adds a width x height subimage to the atlas. Upon success it returns 'kSucceeded' and returns
     * the ID and the subimage's coordinates in the backing texture. 'kTryAgain' is returned if
     * the subimage cannot fit in the atlas without overwriting texels that will be read in the
     * current draw. This indicates that the op should end its current draw and begin another
     * before adding more data. Upon success, an upload of the provided image data will have
     * been added to the GrDrawOp::Target, in "asap" mode if possible, otherwise in "inline" mode.
     * Successive uploads in either mode may be consolidated.
     * 'kError' will be returned when some unrecoverable error was encountered while trying to
     * add the subimage. In this case the op being created should be discarded.
     *
     * NOTE: When the GrDrawOp prepares a draw that reads from the atlas, it must immediately call
     * 'setUseToken' with the currentToken from the GrDrawOp::Target, otherwise the next call to
     * addToAtlas might cause the previous data to be overwritten before it has been read.
     */

    enum class ErrorCode {
        kError,
        kSucceeded,
        kTryAgain
    };

    ErrorCode addToAtlas(GrResourceProvider*, GrDeferredUploadTarget*,
                         int width, int height, const void* image, skgpu::AtlasLocator*);

    const GrSurfaceProxyView* getViews() const { return fViews; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    bool hasID(const skgpu::PlotLocator& plotLocator) {
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
    void setLastUseToken(const skgpu::AtlasLocator& atlasLocator, GrDeferredUploadToken token) {
        SkASSERT(this->hasID(atlasLocator.plotLocator()));
        uint32_t plotIdx = atlasLocator.plotIndex();
        SkASSERT(plotIdx < fNumPlots);
        uint32_t pageIdx = atlasLocator.pageIndex();
        SkASSERT(pageIdx < fNumActivePages);
        Plot* plot = fPages[pageIdx].fPlotArray[plotIdx].get();
        this->makeMRU(plot, pageIdx);
        plot->setLastUseToken(token);
    }

    uint32_t numActivePages() { return fNumActivePages; }

    void setLastUseTokenBulk(const skgpu::BulkUsePlotUpdater& updater,
                             GrDeferredUploadToken token) {
        int count = updater.count();
        for (int i = 0; i < count; i++) {
            const skgpu::BulkUsePlotUpdater::PlotData& pd = updater.plotData(i);
            // it's possible we've added a plot to the updater and subsequently the plot's page
            // was deleted -- so we check to prevent a crash
            if (pd.fPageIndex < fNumActivePages) {
                Plot* plot = fPages[pd.fPageIndex].fPlotArray[pd.fPlotIndex].get();
                this->makeMRU(plot, pd.fPageIndex);
                plot->setLastUseToken(token);
            }
        }
    }

    void compact(GrDeferredUploadToken startTokenForNextFlush);

    void instantiate(GrOnFlushResourceProvider*);

    uint32_t maxPages() const {
        return fMaxPages;
    }

    int numAllocated_TestingOnly() const;
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    GrDrawOpAtlas(GrProxyProvider*, const GrBackendFormat& format, SkColorType, size_t bpp,
                  int width, int height, int plotWidth, int plotHeight,
                  skgpu::AtlasGenerationCounter* generationCounter,
                  AllowMultitexturing allowMultitexturing);

    /**
     * The backing GrTexture for a GrDrawOpAtlas is broken into a spatial grid of Plots. The Plots
     * keep track of subimage placement via their Rectanizer. A Plot manages the lifetime of its
     * data using two tokens, a last use token and a last upload token. Once a Plot is "full" (i.e.
     * there is no room for the new subimage according to the Rectanizer), it can no longer be
     * used unless the last use of the Plot has already been flushed through to the gpu.
     */
    class Plot final : public skgpu::Plot {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);

    public:
        Plot(int pageIndex, int plotIndex, skgpu::AtlasGenerationCounter* generationCounter,
             int offX, int offY, int width, int height, SkColorType colorType, size_t bpp);

        /**
         * To manage the lifetime of a plot, we use two tokens. We use the last upload token to
         * know when we can 'piggy back' uploads, i.e. if the last upload hasn't been flushed to
         * the gpu, we don't need to issue a new upload even if we update the cpu backing store. We
         * use lastUse to determine when we can evict a plot from the cache, i.e. if the last use
         * has already flushed through the gpu then we can reuse the plot.
         */
        GrDeferredUploadToken lastUploadToken() const { return fLastUpload; }
        GrDeferredUploadToken lastUseToken() const { return fLastUse; }
        void setLastUploadToken(GrDeferredUploadToken token) { fLastUpload = token; }
        void setLastUseToken(GrDeferredUploadToken token) { fLastUse = token; }

        void uploadToTexture(GrDeferredTextureUploadWritePixelsFn&, GrTextureProxy*);
        void resetRects();

        int flushesSinceLastUsed() { return fFlushesSinceLastUse; }
        void resetFlushesSinceLastUsed() { fFlushesSinceLastUse = 0; }
        void incFlushesSinceLastUsed() { fFlushesSinceLastUse++; }

        /**
         * Create a clone of this plot. The cloned plot will take the place of the current plot in
         * the atlas
         */
        Plot* clone() const {
            return new Plot(
                fPageIndex, fPlotIndex, fGenerationCounter, fX, fY, fWidth, fHeight, fColorType,
                fBytesPerPixel);
        }
#ifdef SK_DEBUG
        void resetListPtrs() {
            fPrev = fNext = nullptr;
            fList = nullptr;
        }
#endif

    private:
        GrDeferredUploadToken fLastUpload;
        GrDeferredUploadToken fLastUse;
        // the number of flushes since this plot has been last used
        int                   fFlushesSinceLastUse;
    };

    typedef SkTInternalLList<Plot> PlotList;

    inline bool updatePlot(GrDeferredUploadTarget*, skgpu::AtlasLocator*, Plot*);

    inline void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    bool uploadToPage(unsigned int pageIdx, GrDeferredUploadTarget*, int width, int height,
                      const void* image, skgpu::AtlasLocator*);

    bool createPages(GrProxyProvider*, skgpu::AtlasGenerationCounter*);
    bool activateNewPage(GrResourceProvider*);
    void deactivateLastPage();

    void processEviction(skgpu::PlotLocator);
    inline void processEvictionAndResetRects(Plot* plot) {
        this->processEviction(plot->plotLocator());
        plot->resetRects();
    }

    GrBackendFormat       fFormat;
    SkColorType           fColorType;
    size_t                fBytesPerPixel;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;

    skgpu::AtlasGenerationCounter* const fGenerationCounter;
    uint64_t                      fAtlasGeneration;

    // nextTokenToFlush() value at the end of the previous flush
    GrDeferredUploadToken fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    int                   fFlushesSinceLastUse;

    std::vector<skgpu::PlotEvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    GrSurfaceProxyView fViews[skgpu::PlotLocator::kMaxMultitexturePages];
    Page fPages[skgpu::PlotLocator::kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const skgpu::AtlasLocator& atlasLocator) const;)
};

// There are three atlases (A8, 565, ARGB) that are kept in relation with one another. In
// general, the A8 dimensions are 2x the 565 and ARGB dimensions with the constraint that an atlas
// size will always contain at least one plot. Since the ARGB atlas takes the most space, its
// dimensions are used to size the other two atlases.
class GrDrawOpAtlasConfig {
public:
    // The capabilities of the GPU define maxTextureSize. The client provides maxBytes, and this
    // represents the largest they want a single atlas texture to be. Due to multitexturing, we
    // may expand temporarily to use more space as needed.
    GrDrawOpAtlasConfig(int maxTextureSize, size_t maxBytes);

    // For testing only - make minimum sized atlases -- a single plot for ARGB, four for A8
    GrDrawOpAtlasConfig() : GrDrawOpAtlasConfig(kMaxAtlasDim, 0) {}

    SkISize atlasDimensions(skgpu::MaskFormat type) const;
    SkISize plotDimensions(skgpu::MaskFormat type) const;

private:
    // On some systems texture coordinates are represented using half-precision floating point,
    // which limits the largest atlas dimensions to 2048x2048.
    // For simplicity we'll use this constraint for all of our atlas textures.
    // This can be revisited later if we need larger atlases.
    inline static constexpr int kMaxAtlasDim = 2048;

    SkISize fARGBDimensions;
    int     fMaxTextureSize;
};

#endif
