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

#include "include/core/SkSize.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkTInternalLList.h"

#include "src/gpu/GrRectanizerSkyline.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrOnFlushResourceProvider;


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
private:
    static constexpr auto kMaxMultitexturePages = 4;


public:
    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    static constexpr int kMaxPlots = 32; // restricted by the fPlotAlreadyUpdated bitfield
                                         // in BulkUseTokenUpdater

    /**
     * A PlotLocator specifies the plot and is analogous to a directory path:
     *    page/plot/plotGeneration
     *
     * In fact PlotLocator is a portion of a glyph image location in the atlas fully specified by:
     * format/atlasGeneration/page/plot/plotGeneration/(u,v)
     */
    typedef uint64_t PlotLocator;
    static const uint64_t kInvalidPlotLocator = 0;
    static const uint64_t kInvalidAtlasGeneration = 0;

    /**
     * An interface for eviction callbacks. Whenever GrDrawOpAtlas evicts a
     * specific PlotLocator, it will call all of the registered listeners so they can process the
     * eviction.
     */
    class EvictionCallback {
    public:
        virtual ~EvictionCallback() = default;
        virtual void evict(PlotLocator plotLocator) = 0;
    };

    /**
     * Keep track of generation number for Atlases and Plots.
     */
    class GenerationCounter {
    public:
        static constexpr uint64_t kInvalidGeneration = 0;
        uint64_t next() {
            return fGeneration++;
        }

    private:
        uint64_t fGeneration{1};
    };

    /**
     * Returns a GrDrawOpAtlas. This function can be called anywhere, but the returned atlas
     * should only be used inside of GrMeshDrawOp::onPrepareDraws.
     *  @param GrColorType      The colorType which this atlas will store
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
                                               GrColorType,
                                               int width, int height,
                                               int plotWidth, int plotHeight,
                                               GenerationCounter* generationCounter,
                                               AllowMultitexturing allowMultitexturing,
                                               EvictionCallback* evictor);

    /**
     * Packs a texture atlas page index into the uint16 texture coordinates.
     *  @param u      U texture coordinate
     *  @param v      V texture coordinate
     *  @param pageIndex   index of the texture these coordinates apply to.
                           Must be in the range [0, 3].
     *  @return    The new u and v coordinates with the packed value
     */
    static std::pair<uint16_t, uint16_t> PackIndexInTexCoords(uint16_t u, uint16_t v,
                                                              int pageIndex);
    /**
     * Unpacks a texture atlas page index from uint16 texture coordinates.
     *  @param u      Packed U texture coordinate
     *  @param v      Packed V texture coordinate
     *  @return    The unpacked u and v coordinates with the page index.
     */
    static std::tuple<uint16_t, uint16_t, int> UnpackIndexFromTexCoords(uint16_t u, uint16_t v);

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

    ErrorCode addToAtlas(GrResourceProvider*, PlotLocator*, GrDeferredUploadTarget*,
                         int width, int height,
                         const void* image, SkIPoint16* loc);

    const GrSurfaceProxyView* getViews() const { return fViews; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    bool hasID(PlotLocator plotLocator) {
        if (kInvalidPlotLocator == plotLocator) {
            return false;
        }

        uint32_t plot = GetPlotIndexFromID(plotLocator);
        uint32_t page = GetPageIndexFromID(plotLocator);
        uint64_t plotGeneration = fPages[page].fPlotArray[plot]->genID();
        uint64_t locatorGeneration = GetGenerationFromID(plotLocator);
        return plot < fNumPlots && page < fNumActivePages && plotGeneration == locatorGeneration;
    }

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(PlotLocator plotLocator, GrDeferredUploadToken token) {
        SkASSERT(this->hasID(plotLocator));
        uint32_t plotIdx = GetPlotIndexFromID(plotLocator);
        SkASSERT(plotIdx < fNumPlots);
        uint32_t pageIdx = GetPageIndexFromID(plotLocator);
        SkASSERT(pageIdx < fNumActivePages);
        Plot* plot = fPages[pageIdx].fPlotArray[plotIdx].get();
        this->makeMRU(plot, pageIdx);
        plot->setLastUseToken(token);
    }

    uint32_t numActivePages() { return fNumActivePages; }

    /**
     * A class which can be handed back to GrDrawOpAtlas for updating last use tokens in bulk.  The
     * current max number of plots per page the GrDrawOpAtlas can handle is 32. If in the future
     * this is insufficient then we can move to a 64 bit int.
     */
    class BulkUseTokenUpdater {
    public:
        BulkUseTokenUpdater() {
            memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
        }
        BulkUseTokenUpdater(const BulkUseTokenUpdater& that)
            : fPlotsToUpdate(that.fPlotsToUpdate) {
            memcpy(fPlotAlreadyUpdated, that.fPlotAlreadyUpdated, sizeof(fPlotAlreadyUpdated));
        }

        bool add(PlotLocator plotLocator) {
            int index = GrDrawOpAtlas::GetPlotIndexFromID(plotLocator);
            int pageIdx = GrDrawOpAtlas::GetPageIndexFromID(plotLocator);
            if (this->find(pageIdx, index)) {
                return false;
            }
            this->set(pageIdx, index);
            return true;
        }

        void reset() {
            fPlotsToUpdate.reset();
            memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
        }

        struct PlotData {
            PlotData(int pageIdx, int plotIdx) : fPageIndex(pageIdx), fPlotIndex(plotIdx) {}
            uint32_t fPageIndex;
            uint32_t fPlotIndex;
        };

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

        static constexpr int kMinItems = 4;
        SkSTArray<kMinItems, PlotData, true> fPlotsToUpdate;
        uint32_t fPlotAlreadyUpdated[kMaxMultitexturePages]; // TODO: increase this to uint64_t
                                                             //       to allow more plots per page

        friend class GrDrawOpAtlas;
    };

    void setLastUseTokenBulk(const BulkUseTokenUpdater& updater, GrDeferredUploadToken token) {
        int count = updater.fPlotsToUpdate.count();
        for (int i = 0; i < count; i++) {
            const BulkUseTokenUpdater::PlotData& pd = updater.fPlotsToUpdate[i];
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

    static uint32_t GetPageIndexFromID(PlotLocator plotLocator) {
        return plotLocator & 0xff;
    }

    void instantiate(GrOnFlushResourceProvider*);

    uint32_t maxPages() const {
        return fMaxPages;
    }

    int numAllocated_TestingOnly() const;
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    GrDrawOpAtlas(GrProxyProvider*, const GrBackendFormat& format, GrColorType, int width,
                  int height, int plotWidth, int plotHeight, GenerationCounter* generationCounter,
                  AllowMultitexturing allowMultitexturing);

    /**
     * The backing GrTexture for a GrDrawOpAtlas is broken into a spatial grid of Plots. The Plots
     * keep track of subimage placement via their GrRectanizer. A Plot manages the lifetime of its
     * data using two tokens, a last use token and a last upload token. Once a Plot is "full" (i.e.
     * there is no room for the new subimage according to the GrRectanizer), it can no longer be
     * used unless the last use of the Plot has already been flushed through to the gpu.
     */
    class Plot : public SkRefCnt {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);

    public:
        /** index() is a unique id for the plot relative to the owning GrAtlas and page. */
        uint32_t index() const { return fPlotIndex; }
        /**
         * genID() is incremented when the plot is evicted due to a atlas spill. It is used to know
         * if a particular subimage is still present in the atlas.
         */
        uint64_t genID() const { return fGenID; }
        GrDrawOpAtlas::PlotLocator plotLocator() const {
            SkASSERT(GrDrawOpAtlas::kInvalidPlotLocator != fPlotLocator);
            return fPlotLocator;
        }
        SkDEBUGCODE(size_t bpp() const { return fBytesPerPixel; })

        bool addSubImage(int width, int height, const void* image, SkIPoint16* loc);

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

    private:
        Plot(int pageIndex, int plotIndex, GenerationCounter* generationCounter,
             int offX, int offY, int width, int height, GrColorType colorType);

        ~Plot() override;

        /**
         * Create a clone of this plot. The cloned plot will take the place of the current plot in
         * the atlas
         */
        Plot* clone() const {
            return new Plot(
                fPageIndex, fPlotIndex, fGenerationCounter, fX, fY, fWidth, fHeight, fColorType);
        }

        static GrDrawOpAtlas::PlotLocator CreatePlotLocator(
                uint32_t pageIdx, uint32_t plotIdx, uint64_t generation) {
            SkASSERT(pageIdx < (1 << 8));
            SkASSERT(pageIdx < kMaxMultitexturePages);
            SkASSERT(plotIdx < (1 << 8));
            SkASSERT(generation < ((uint64_t)1 << 48));
            return generation << 16 | plotIdx << 8 | pageIdx;
        }

        GrDeferredUploadToken fLastUpload;
        GrDeferredUploadToken fLastUse;
        // the number of flushes since this plot has been last used
        int                   fFlushesSinceLastUse;

        struct {
            const uint32_t fPageIndex : 16;
            const uint32_t fPlotIndex : 16;
        };
        GenerationCounter* const fGenerationCounter;
        uint64_t fGenID;
        GrDrawOpAtlas::PlotLocator fPlotLocator;
        unsigned char* fData;
        const int fWidth;
        const int fHeight;
        const int fX;
        const int fY;
        GrRectanizerSkyline fRectanizer;
        const SkIPoint16 fOffset;  // the offset of the plot in the backing texture
        const GrColorType fColorType;
        const size_t fBytesPerPixel;
        SkIRect fDirtyRect;
        SkDEBUGCODE(bool fDirty);

        friend class GrDrawOpAtlas;

        typedef SkRefCnt INHERITED;
    };

    typedef SkTInternalLList<Plot> PlotList;

    static uint32_t GetPlotIndexFromID(PlotLocator plotLocator) {
        return (plotLocator >> 8) & 0xff;
    }

    // top 48 bits are reserved for the generation ID
    static uint64_t GetGenerationFromID(PlotLocator plotLocator) {
        return (plotLocator >> 16) & 0xffffffffffff;
    }

    inline bool updatePlot(GrDeferredUploadTarget*, PlotLocator*, Plot*);

    inline void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    bool uploadToPage(const GrCaps&, unsigned int pageIdx, PlotLocator* plotLocator,
                      GrDeferredUploadTarget* target, int width, int height, const void* image,
                      SkIPoint16* loc);

    bool createPages(GrProxyProvider*, GenerationCounter*);
    bool activateNewPage(GrResourceProvider*);
    void deactivateLastPage();

    void processEviction(PlotLocator);
    inline void processEvictionAndResetRects(Plot* plot) {
        this->processEviction(plot->plotLocator());
        plot->resetRects();
    }

    GrBackendFormat       fFormat;
    GrColorType           fColorType;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;

    GenerationCounter* const fGenerationCounter;
    uint64_t                 fAtlasGeneration;

    // nextTokenToFlush() value at the end of the previous flush
    GrDeferredUploadToken fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    int                   fFlushesSinceLastUse;

    std::vector<EvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    GrSurfaceProxyView fViews[kMaxMultitexturePages];
    Page fPages[kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;
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

    SkISize atlasDimensions(GrMaskFormat type) const;
    SkISize plotDimensions(GrMaskFormat type) const;

private:
    // On some systems texture coordinates are represented using half-precision floating point,
    // which limits the largest atlas dimensions to 2048x2048.
    // For simplicity we'll use this constraint for all of our atlas textures.
    // This can be revisited later if we need larger atlases.
    static constexpr int kMaxAtlasDim = 2048;

    SkISize fARGBDimensions;
    int     fMaxTextureSize;
};

#endif
