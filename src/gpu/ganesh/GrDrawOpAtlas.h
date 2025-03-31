/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpAtlas_DEFINED
#define GrDrawOpAtlas_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class GrOnFlushResourceProvider;
class GrProxyProvider;
class GrResourceProvider;
class GrTextureProxy;
enum SkColorType : int;

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
 * is checked to see whether it was used in that flush. If less than a quarter of the plots have
 * been used recently (within kPlotRecentlyUsedCount iterations) and there are available
 * plots in lower index pages, the higher index page will be deactivated, and its glyphs will
 * gradually migrate to other pages via the usual upload system.
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
     *  @param proxyProvider       Used to create the atlas's texture proxies.
     *  @param format              Backend format for the atlas's textures.
     *                             Should be compatible with ct.
     *  @param ct                  The colorType which this atlas will store.
     *  @param bpp                 Size in bytes of each pixel.
     *  @param width               Width in pixels of the atlas.
     *  @param height              Height in pixels of the atlas.
     *  @param plotWidth           The width of each plot. width/plotWidth should be an integer.
     *  @param plotWidth           The height of each plot. height/plotHeight should be an integer.
     *  @param generationCounter   A pointer to the context's generation counter.
     *  @param allowMultitexturing Can the atlas use more than one texture.
     *  @param evictor             A pointer to an eviction callback class.
     *  @param label               A label for the atlas texture.
     *
     *  @return                    An initialized DrawAtlas, or nullptr if creation fails.
     */
    static std::unique_ptr<GrDrawOpAtlas> Make(GrProxyProvider* proxyProvider,
                                               const GrBackendFormat& format,
                                               SkColorType ct, size_t bpp,
                                               int width, int height,
                                               int plotWidth, int plotHeight,
                                               skgpu::AtlasGenerationCounter* generationCounter,
                                               AllowMultitexturing allowMultitexturing,
                                               skgpu::PlotEvictionCallback* evictor,
                                               std::string_view label);

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
     * 'setLastUseToken' with the currentToken from the GrDrawOp::Target, otherwise the next call to
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
    void setLastUseToken(const skgpu::AtlasLocator& atlasLocator, skgpu::AtlasToken token) {
        SkASSERT(this->hasID(atlasLocator.plotLocator()));
        uint32_t plotIdx = atlasLocator.plotIndex();
        SkASSERT(plotIdx < fNumPlots);
        uint32_t pageIdx = atlasLocator.pageIndex();
        SkASSERT(pageIdx < fNumActivePages);
        skgpu::Plot* plot = fPages[pageIdx].fPlotArray[plotIdx].get();
        this->makeMRU(plot, pageIdx);
        plot->setLastUseToken(token);
    }

    uint32_t numActivePages() { return fNumActivePages; }

    void setLastUseTokenBulk(const skgpu::BulkUsePlotUpdater& updater,
                             skgpu::AtlasToken token) {
        int count = updater.count();
        for (int i = 0; i < count; i++) {
            const skgpu::BulkUsePlotUpdater::PlotData& pd = updater.plotData(i);
            // it's possible we've added a plot to the updater and subsequently the plot's page
            // was deleted -- so we check to prevent a crash
            if (pd.fPageIndex < fNumActivePages) {
                skgpu::Plot* plot = fPages[pd.fPageIndex].fPlotArray[pd.fPlotIndex].get();
                this->makeMRU(plot, pd.fPageIndex);
                plot->setLastUseToken(token);
            }
        }
    }

    void compact(skgpu::AtlasToken startTokenForNextFlush);

    void instantiate(GrOnFlushResourceProvider*);

    uint32_t maxPages() const {
        return fMaxPages;
    }

private:
    friend class GrDrawOpAtlasTools;

    GrDrawOpAtlas(GrProxyProvider*, const GrBackendFormat& format, SkColorType, size_t bpp,
                  int width, int height, int plotWidth, int plotHeight,
                  skgpu::AtlasGenerationCounter* generationCounter,
                  AllowMultitexturing allowMultitexturing, std::string_view label);

    inline bool updatePlot(GrDeferredUploadTarget*, skgpu::AtlasLocator*, skgpu::Plot*);

    inline void makeMRU(skgpu::Plot* plot, uint32_t pageIdx) {
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

    void uploadPlotToTexture(GrDeferredTextureUploadWritePixelsFn& writePixels,
                             GrTextureProxy* proxy,
                             skgpu::Plot* plot);

    bool createPages(GrProxyProvider*, skgpu::AtlasGenerationCounter*);
    bool activateNewPage(GrResourceProvider*);
    void deactivateLastPage();

    void processEviction(skgpu::PlotLocator);
    inline void processEvictionAndResetRects(skgpu::Plot* plot) {
        this->processEviction(plot->plotLocator());
        plot->resetRects(/*freeData=*/false);
    }

    GrBackendFormat       fFormat;
    SkColorType           fColorType;
    size_t                fBytesPerPixel;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;
    const std::string     fLabel;

    // A counter to track the atlas eviction state for Glyphs. Each Glyph has a PlotLocator
    // which contains its current generation. When the atlas evicts a plot, it increases
    // the generation counter. If a Glyph's generation is less than the atlas's
    // generation, then it knows it's been evicted and is either free to be deleted or
    // re-added to the atlas if necessary.
    skgpu::AtlasGenerationCounter* const fGenerationCounter;
    uint64_t                      fAtlasGeneration;

    // nextFlushToken() value at the end of the previous flush
    skgpu::AtlasToken fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    int                   fFlushesSinceLastUse;

    std::vector<skgpu::PlotEvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<skgpu::Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        skgpu::PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    GrSurfaceProxyView fViews[skgpu::PlotLocator::kMaxMultitexturePages];
    Page fPages[skgpu::PlotLocator::kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const skgpu::AtlasLocator& atlasLocator) const;)
};

// There are three atlases (A8, 565, ARGB) that are kept in relation with one another. In
// general, because A8 is the most frequently used mask format its dimensions are 2x the 565 and
// ARGB dimensions, with the constraint that an atlas size will always contain at least one plot.
// Since the ARGB atlas takes the most space, its dimensions are used to size the other two atlases.
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
    // On some systems texture coordinates are represented using half-precision floating point
    // with 11 significant bits, which limits the largest atlas dimensions to 2048x2048.
    // For simplicity we'll use this constraint for all of our atlas textures.
    // This can be revisited later if we need larger atlases.
    inline static constexpr int kMaxAtlasDim = 2048;

    SkISize fARGBDimensions;
    int     fMaxTextureSize;
};

#endif
