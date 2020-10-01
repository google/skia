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
#include "include/private/SkTArray.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrRectanizerSkyline.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/geometry/GrRect.h"

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

    // These are both restricted by the space they occupy in the PlotLocator.
    // maxPages is also limited by being crammed into the glyph uvs.
    // maxPlots is also limited by the fPlotAlreadyUpdated bitfield in BulkUseTokenUpdater
    static constexpr auto kMaxMultitexturePages = 4;
    static constexpr int kMaxPlots = 32;

    /**
     * A PlotLocator specifies the plot and is analogous to a directory path:
     *    page/plot/plotGeneration
     *
     * In fact PlotLocator is a portion of a glyph image location in the atlas fully specified by:
     *    format/atlasGeneration/page/plot/plotGeneration/rect
     *
     * TODO: Remove the small path renderer's use of the PlotLocator for eviction.
     */
    class PlotLocator {
    public:
        PlotLocator(uint32_t pageIdx, uint32_t plotIdx, uint64_t generation)
                : fGenID(generation)
                , fPlotIndex(plotIdx)
                , fPageIndex(pageIdx) {
            SkASSERT(pageIdx < kMaxMultitexturePages);
            SkASSERT(plotIdx < kMaxPlots);
            SkASSERT(generation < ((uint64_t)1 << 48));
        }

        PlotLocator() : fGenID(0), fPlotIndex(0), fPageIndex(0) {}

        bool isValid() const {
            return fGenID != 0 || fPlotIndex != 0 || fPageIndex != 0;
        }

        void makeInvalid() {
            fGenID = 0;
            fPlotIndex = 0;
            fPageIndex = 0;
        }

        bool operator==(const PlotLocator& other) const {
            return fGenID == other.fGenID &&
                   fPlotIndex == other.fPlotIndex &&
                   fPageIndex == other.fPageIndex; }

        uint32_t pageIndex() const { return fPageIndex; }
        uint32_t plotIndex() const { return fPlotIndex; }
        uint64_t genID() const { return fGenID; }

    private:
        uint64_t fGenID:48;
        uint64_t fPlotIndex:8;
        uint64_t fPageIndex:8;
    };

    static const uint64_t kInvalidAtlasGeneration = 0;


    // AtlasLocator handles atlas position information. It keeps a left-top, right-bottom pair of
    // encoded UV coordinates. The bits 13 & 14 of the U coordinates hold the atlas page index.
    // This information is handed directly as is from fUVs. This encoding has the nice property
    // that width = fUVs[2] - fUVs[0]; the page encoding in the top bits subtracts to zero.
    class AtlasLocator {
    public:
        std::array<uint16_t, 4> getUVs() const {
            return fUVs;
        }

        void invalidatePlotLocator() { fPlotLocator.makeInvalid(); }

        // TODO: Remove the small path renderer's use of this for eviction
        PlotLocator plotLocator() const { return fPlotLocator; }

        uint32_t pageIndex() const { return fPlotLocator.pageIndex(); }

        uint32_t plotIndex() const { return fPlotLocator.plotIndex(); }

        uint64_t genID() const { return fPlotLocator.genID(); }

        SkIPoint topLeft() const {
            return {fUVs[0] & 0x1FFF, fUVs[1]};
        }

        uint16_t width() const {
            return fUVs[2] - fUVs[0];
        }

        uint16_t height() const {
            return fUVs[3] - fUVs[1];
        }

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

        void updateRect(GrIRect16 rect) {
            SkASSERT(rect.fLeft <= rect.fRight);
            SkASSERT(rect.fRight <= 0x1FFF);
            fUVs[0] = (fUVs[0] & 0xE000) | rect.fLeft;
            fUVs[1] = rect.fTop;
            fUVs[2] = (fUVs[2] & 0xE000) | rect.fRight;
            fUVs[3] = rect.fBottom;
        }

    private:
        PlotLocator fPlotLocator{0, 0, 0};

        // The inset padded bounds in the atlas in the lower 13 bits, and page index in bits 13 &
        // 14 of the Us.
        std::array<uint16_t, 4> fUVs{0, 0, 0, 0};
    };

    /**
     * An interface for eviction callbacks. Whenever GrDrawOpAtlas evicts a
     * specific PlotLocator, it will call all of the registered listeners so they can process the
     * eviction.
     */
    class EvictionCallback {
    public:
        virtual ~EvictionCallback() = default;
        virtual void evict(PlotLocator) = 0;
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
                         int width, int height, const void* image, AtlasLocator*);

    const GrSurfaceProxyView* getViews() const { return fViews; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

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
    void setLastUseToken(const AtlasLocator& atlasLocator, GrDeferredUploadToken token) {
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
        uint32_t pageIndex() const { return fPageIndex; }

        /** plotIndex() is a unique id for the plot relative to the owning GrAtlas and page. */
        uint32_t plotIndex() const { return fPlotIndex; }
        /**
         * genID() is incremented when the plot is evicted due to a atlas spill. It is used to know
         * if a particular subimage is still present in the atlas.
         */
        uint64_t genID() const { return fGenID; }
        PlotLocator plotLocator() const {
            SkASSERT(fPlotLocator.isValid());
            return fPlotLocator;
        }
        SkDEBUGCODE(size_t bpp() const { return fBytesPerPixel; })

        bool addSubImage(int width, int height, const void* image, AtlasLocator* atlasLocator);

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
        PlotLocator fPlotLocator;
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

        using INHERITED = SkRefCnt;
    };

    typedef SkTInternalLList<Plot> PlotList;

    inline bool updatePlot(GrDeferredUploadTarget*, AtlasLocator*, Plot*);

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
                      const void* image, AtlasLocator*);

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

    SkDEBUGCODE(void validate(const AtlasLocator& atlasLocator) const;)
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
