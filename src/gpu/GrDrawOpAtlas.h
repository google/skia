/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpAtlas_DEFINED
#define GrDrawOpAtlas_DEFINED

#include "SkPoint.h"
#include "SkTDArray.h"
#include "SkTInternalLList.h"

#include "ops/GrDrawOp.h"

class GrRectanizer;

struct GrDrawOpAtlasConfig {
    int numPlotsX() const { return fWidth / fPlotWidth; }
    int numPlotsY() const { return fHeight / fPlotWidth; }
    int fWidth;
    int fHeight;
    int fLog2Width;
    int fLog2Height;
    int fPlotWidth;
    int fPlotHeight;
};

/**
 * This class manages an atlas texture on behalf of GrDrawOps. The draw ops that use the atlas
 * perform texture uploads when preparing their draws during flush. The class provides facilities
 * for using GrDrawOpUploadToken to detect data hazards. Op's uploads are performed in "asap" mode
 * until it is impossible to add data without overwriting texels read by draws that have not yet
 * executed on the gpu. At that point the uploads are performed "inline" between draws. If a single
 * draw would use enough subimage space to overflow the atlas texture then the atlas will fail to
 * add a subimage. This gives the op the chance to end the draw and begin a new one. Additional
 * uploads will then succeed in inline mode.
 */
class GrDrawOpAtlas {
public:
    /**
     * An AtlasID is an opaque handle which callers can use to determine if the atlas contains
     * a specific piece of data.
     */
    typedef uint64_t AtlasID;
    static const uint32_t kInvalidAtlasID = 0;
    static const uint64_t kInvalidAtlasGeneration = 0;

    /**
     * A function pointer for use as a callback during eviction. Whenever GrDrawOpAtlas evicts a
     * specific AtlasID, it will call all of the registered listeners so they can process the
     * eviction.
     */
    typedef void (*EvictionFunc)(GrDrawOpAtlas::AtlasID, void*);

    /**
     * Returns a GrDrawOpAtlas. This function can be called anywhere, but the returned atlas
     * should only be used inside of GrMeshDrawOp::onPrepareDraws.
     *  @param GrPixelConfig    The pixel config which this atlas will store
     *  @param width            width in pixels of the atlas
     *  @param height           height in pixels of the atlas
     *  @param numPlotsX        The number of plots the atlas should be broken up into in the X
     *                          direction
     *  @param numPlotsY        The number of plots the atlas should be broken up into in the Y
     *                          direction
     *  @param func             An eviction function which will be called whenever the atlas has to
     *                          evict data
     *  @param data             User supplied data which will be passed into func whenver an
     *                          eviction occurs
     *  @return                 An initialized GrDrawOpAtlas, or nullptr if creation fails
     */
    static std::unique_ptr<GrDrawOpAtlas> Make(GrContext*, GrPixelConfig,
                                               int width, int height,
                                               int numPlotsX, int numPlotsY,
                                               GrDrawOpAtlas::EvictionFunc func, void* data);

    /**
     * Adds a width x height subimage to the atlas. Upon success it returns an ID and the subimage's
     * coordinates in the backing texture. False is returned if the subimage cannot fit in the
     * atlas without overwriting texels that will be read in the current draw. This indicates that
     * the op should end its current draw and begin another before adding more data. Upon success,
     * an upload of the provided image data will have been added to the GrDrawOp::Target, in "asap"
     * mode if possible, otherwise in "inline" mode. Successive uploads in either mode may be
     * consolidated.
     * NOTE: When the GrDrawOp prepares a draw that reads from the atlas, it must immediately call
     * 'setUseToken' with the currentToken from the GrDrawOp::Target, otherwise the next call to
     * addToAtlas might cause the previous data to be overwritten before it has been read.
     */
    bool addToAtlas(AtlasID*, GrDrawOp::Target*, int width, int height, const void* image,
                    SkIPoint16* loc);

    GrContext* context() const { return fContext; }
    sk_sp<GrTextureProxy> getProxy() const { return fProxy; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    inline bool hasID(AtlasID id) {
        uint32_t index = GetIndexFromID(id);
        SkASSERT(index < fNumPlots);
        return fPlotArray[index]->genID() == GetGenerationFromID(id);
    }

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    inline void setLastUseToken(AtlasID id, GrDrawOpUploadToken token) {
        SkASSERT(this->hasID(id));
        uint32_t index = GetIndexFromID(id);
        SkASSERT(index < fNumPlots);
        this->makeMRU(fPlotArray[index].get());
        fPlotArray[index]->setLastUseToken(token);
    }

    inline void registerEvictionCallback(EvictionFunc func, void* userData) {
        EvictionData* data = fEvictionCallbacks.append();
        data->fFunc = func;
        data->fData = userData;
    }

    /**
     * A class which can be handed back to GrDrawOpAtlas for updating last use tokens in bulk.  The
     * current max number of plots the GrDrawOpAtlas can handle is 32. If in the future this is
     * insufficient then we can move to a 64 bit int.
     */
    class BulkUseTokenUpdater {
    public:
        BulkUseTokenUpdater() : fPlotAlreadyUpdated(0) {}
        BulkUseTokenUpdater(const BulkUseTokenUpdater& that)
            : fPlotsToUpdate(that.fPlotsToUpdate)
            , fPlotAlreadyUpdated(that.fPlotAlreadyUpdated) {
        }

        void add(AtlasID id) {
            int index = GrDrawOpAtlas::GetIndexFromID(id);
            if (!this->find(index)) {
                this->set(index);
            }
        }

        void reset() {
            fPlotsToUpdate.reset();
            fPlotAlreadyUpdated = 0;
        }

    private:
        bool find(int index) const {
            SkASSERT(index < kMaxPlots);
            return (fPlotAlreadyUpdated >> index) & 1;
        }

        void set(int index) {
            SkASSERT(!this->find(index));
            fPlotAlreadyUpdated = fPlotAlreadyUpdated | (1 << index);
            fPlotsToUpdate.push_back(index);
        }

        static const int kMinItems = 4;
        static const int kMaxPlots = 32;
        SkSTArray<kMinItems, int, true> fPlotsToUpdate;
        uint32_t fPlotAlreadyUpdated;

        friend class GrDrawOpAtlas;
    };

    void setLastUseTokenBulk(const BulkUseTokenUpdater& updater, GrDrawOpUploadToken token) {
        int count = updater.fPlotsToUpdate.count();
        for (int i = 0; i < count; i++) {
            Plot* plot = fPlotArray[updater.fPlotsToUpdate[i]].get();
            this->makeMRU(plot);
            plot->setLastUseToken(token);
        }
    }

    static const int kGlyphMaxDim = 256;
    static bool GlyphTooLargeForAtlas(int width, int height) {
        return width > kGlyphMaxDim || height > kGlyphMaxDim;
    }

private:
    GrDrawOpAtlas(GrContext*, sk_sp<GrTextureProxy>, int numPlotsX, int numPlotsY);

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
        /** index() is a unique id for the plot relative to the owning GrAtlas. */
        uint32_t index() const { return fIndex; }
        /**
         * genID() is incremented when the plot is evicted due to a atlas spill. It is used to know
         * if a particular subimage is still present in the atlas.
         */
        uint64_t genID() const { return fGenID; }
        GrDrawOpAtlas::AtlasID id() const {
            SkASSERT(GrDrawOpAtlas::kInvalidAtlasID != fID);
            return fID;
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
        GrDrawOpUploadToken lastUploadToken() const { return fLastUpload; }
        GrDrawOpUploadToken lastUseToken() const { return fLastUse; }
        void setLastUploadToken(GrDrawOpUploadToken token) { fLastUpload = token; }
        void setLastUseToken(GrDrawOpUploadToken token) { fLastUse = token; }

        void uploadToTexture(GrDrawOp::WritePixelsFn&, GrTexture* texture);
        void resetRects();

    private:
        Plot(int index, uint64_t genID, int offX, int offY, int width, int height,
             GrPixelConfig config);

        ~Plot() override;

        /**
         * Create a clone of this plot. The cloned plot will take the place of the current plot in
         * the atlas
         */
        Plot* clone() const {
            return new Plot(fIndex, fGenID + 1, fX, fY, fWidth, fHeight, fConfig);
        }

        static GrDrawOpAtlas::AtlasID CreateId(uint32_t index, uint64_t generation) {
            SkASSERT(index < (1 << 16));
            SkASSERT(generation < ((uint64_t)1 << 48));
            return generation << 16 | index;
        }

        GrDrawOpUploadToken   fLastUpload;
        GrDrawOpUploadToken   fLastUse;

        const uint32_t fIndex;
        uint64_t fGenID;
        GrDrawOpAtlas::AtlasID fID;
        unsigned char* fData;
        const int fWidth;
        const int fHeight;
        const int fX;
        const int fY;
        GrRectanizer* fRects;
        const SkIPoint16 fOffset;  // the offset of the plot in the backing texture
        const GrPixelConfig fConfig;
        const size_t fBytesPerPixel;
        SkIRect fDirtyRect;
        SkDEBUGCODE(bool fDirty);

        friend class GrDrawOpAtlas;

        typedef SkRefCnt INHERITED;
    };

    typedef SkTInternalLList<Plot> PlotList;

    static uint32_t GetIndexFromID(AtlasID id) {
        return id & 0xffff;
    }

    // top 48 bits are reserved for the generation ID
    static uint64_t GetGenerationFromID(AtlasID id) {
        return (id >> 16) & 0xffffffffffff;
    }

    inline bool updatePlot(GrDrawOp::Target*, AtlasID*, Plot*);

    inline void makeMRU(Plot* plot) {
        if (fPlotList.head() == plot) {
            return;
        }

        fPlotList.remove(plot);
        fPlotList.addToHead(plot);
    }

    inline void processEviction(AtlasID);

    GrContext*            fContext;
    sk_sp<GrTextureProxy> fProxy;
    int                   fPlotWidth;
    int                   fPlotHeight;
    SkDEBUGCODE(uint32_t  fNumPlots;)

    uint64_t              fAtlasGeneration;

    struct EvictionData {
        EvictionFunc fFunc;
        void* fData;
    };

    SkTDArray<EvictionData> fEvictionCallbacks;
    // allocated array of Plots
    std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
    // LRU list of Plots (MRU at head - LRU at tail)
    PlotList fPlotList;
};

#endif
