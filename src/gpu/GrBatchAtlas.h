/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchAtlas_DEFINED
#define GrBatchAtlas_DEFINED

#include "GrTexture.h"
#include "SkPoint.h"
#include "SkTDArray.h"
#include "SkTInternalLList.h"

#include "batches/GrDrawBatch.h"

class GrRectanizer;

struct GrBatchAtlasConfig {
    int numPlotsX() const { return fWidth / fPlotWidth; }
    int numPlotsY() const { return fHeight / fPlotWidth; }
    int fWidth;
    int fHeight;
    int fLog2Width;
    int fLog2Height;
    int fPlotWidth;
    int fPlotHeight;
};

class GrBatchAtlas {
public:
    // An AtlasID is an opaque handle which callers can use to determine if the atlas contains
    // a specific piece of data
    typedef uint64_t AtlasID;
    static const uint32_t kInvalidAtlasID = 0;
    static const uint64_t kInvalidAtlasGeneration = 0;

    // A function pointer for use as a callback during eviction.  Whenever GrBatchAtlas evicts a
    // specific AtlasID, it will call all of the registered listeners so they can optionally process
    // the eviction
    typedef void (*EvictionFunc)(GrBatchAtlas::AtlasID, void*);

    GrBatchAtlas(GrTexture*, int numPlotsX, int numPlotsY);
    ~GrBatchAtlas();

    // Adds a width x height subimage to the atlas. Upon success it returns
    // the containing GrPlot and absolute location in the backing texture.
    // nullptr is returned if the subimage cannot fit in the atlas.
    // If provided, the image data will be written to the CPU-side backing bitmap.
    // NOTE: If the client intends to refer to the atlas, they should immediately call 'setUseToken'
    // with the currentToken from the batch target, otherwise the next call to addToAtlas might
    // cause an eviction
    bool addToAtlas(AtlasID*, GrDrawBatch::Target*, int width, int height, const void* image,
                    SkIPoint16* loc);

    GrTexture* getTexture() const { return fTexture; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    inline bool hasID(AtlasID id) {
        uint32_t index = GetIndexFromID(id);
        SkASSERT(index < fNumPlots);
        return fPlotArray[index]->genID() == GetGenerationFromID(id);
    }

    // To ensure the atlas does not evict a given entry, the client must set the last use token
    inline void setLastUseToken(AtlasID id, GrBatchDrawToken batchToken) {
        SkASSERT(this->hasID(id));
        uint32_t index = GetIndexFromID(id);
        SkASSERT(index < fNumPlots);
        this->makeMRU(fPlotArray[index]);
        fPlotArray[index]->setLastUseToken(batchToken);
    }

    inline void registerEvictionCallback(EvictionFunc func, void* userData) {
        EvictionData* data = fEvictionCallbacks.append();
        data->fFunc = func;
        data->fData = userData;
    }

    /*
     * A class which can be handed back to GrBatchAtlas for updating in bulk last use tokens.  The
     * current max number of plots the GrBatchAtlas can handle is 32, if in the future this is
     * insufficient then we can move to a 64 bit int
     */
    class BulkUseTokenUpdater {
    public:
        BulkUseTokenUpdater() : fPlotAlreadyUpdated(0) {}
        BulkUseTokenUpdater(const BulkUseTokenUpdater& that)
            : fPlotsToUpdate(that.fPlotsToUpdate)
            , fPlotAlreadyUpdated(that.fPlotAlreadyUpdated) {
        }

        void add(AtlasID id) {
            int index = GrBatchAtlas::GetIndexFromID(id);
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

        friend class GrBatchAtlas;
    };

    void setLastUseTokenBulk(const BulkUseTokenUpdater& updater, GrBatchDrawToken batchToken) {
        int count = updater.fPlotsToUpdate.count();
        for (int i = 0; i < count; i++) {
            BatchPlot* plot = fPlotArray[updater.fPlotsToUpdate[i]];
            this->makeMRU(plot);
            plot->setLastUseToken(batchToken);
        }
    }

    static const int kGlyphMaxDim = 256;
    static bool GlyphTooLargeForAtlas(int width, int height) {
        return width > kGlyphMaxDim || height > kGlyphMaxDim;
    }

private:
    // The backing GrTexture for a GrBatchAtlas is broken into a spatial grid of BatchPlots.
    // The BatchPlots keep track of subimage placement via their GrRectanizer. A BatchPlot
    // manages the lifetime of its data using two tokens, a last use token and a last upload token.
    // Once a BatchPlot is "full" (i.e. there is no room for the new subimage according to the
    // GrRectanizer), it can no longer be used unless the last use of the GrPlot has already been
    // flushed through to the gpu.
    class BatchPlot : public SkRefCnt {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BatchPlot);

    public:
        // index() is a unique id for the plot relative to the owning GrAtlas.  genID() is a
        // monotonically incremented number which is bumped every time this plot is
        // evicted from the cache (i.e., there is continuity in genID() across atlas spills).
        uint32_t index() const { return fIndex; }
        uint64_t genID() const { return fGenID; }
        GrBatchAtlas::AtlasID id() const {
            SkASSERT(GrBatchAtlas::kInvalidAtlasID != fID);
            return fID;
        }
        SkDEBUGCODE(size_t bpp() const { return fBytesPerPixel; })

        bool addSubImage(int width, int height, const void* image, SkIPoint16* loc);

        // To manage the lifetime of a plot, we use two tokens.  We use the last upload token to
        // know when we can 'piggy back' uploads, ie if the last upload hasn't been flushed to gpu,
        // we don't need to issue a new upload even if we update the cpu backing store.  We use
        // lastUse to determine when we can evict a plot from the cache, ie if the last use has
        // already flushed through the gpu then we can reuse the plot.
        GrBatchDrawToken lastUploadToken() const { return fLastUpload; }
        GrBatchDrawToken lastUseToken() const { return fLastUse; }
        void setLastUploadToken(GrBatchDrawToken batchToken) { fLastUpload = batchToken; }
        void setLastUseToken(GrBatchDrawToken batchToken) { fLastUse = batchToken; }

        void uploadToTexture(GrDrawBatch::WritePixelsFn&, GrTexture* texture);
        void resetRects();

    private:
        BatchPlot(int index, uint64_t genID, int offX, int offY, int width, int height,
                  GrPixelConfig config);

        ~BatchPlot() override;

        // Create a clone of this plot. The cloned plot will take the place of the
        // current plot in the atlas.
        BatchPlot* clone() const {
            return new BatchPlot(fIndex, fGenID+1, fX, fY, fWidth, fHeight, fConfig);
        }

        static GrBatchAtlas::AtlasID CreateId(uint32_t index, uint64_t generation) {
            SkASSERT(index < (1 << 16));
            SkASSERT(generation < ((uint64_t)1 << 48));
            return generation << 16 | index;
        }

        GrBatchDrawToken      fLastUpload;
        GrBatchDrawToken      fLastUse;

        const uint32_t        fIndex;
        uint64_t              fGenID;
        GrBatchAtlas::AtlasID fID;
        unsigned char*        fData;
        const int             fWidth;
        const int             fHeight;
        const int             fX;
        const int             fY;
        GrRectanizer*         fRects;
        const SkIPoint16      fOffset;        // the offset of the plot in the backing texture
        const GrPixelConfig   fConfig;
        const size_t          fBytesPerPixel;
        SkIRect               fDirtyRect;
        SkDEBUGCODE(bool      fDirty;)

        friend class GrBatchAtlas;

        typedef SkRefCnt INHERITED;
    };

    typedef SkTInternalLList<BatchPlot> GrBatchPlotList;

    static uint32_t GetIndexFromID(AtlasID id) {
        return id & 0xffff;
    }

    // top 48 bits are reserved for the generation ID
    static uint64_t GetGenerationFromID(AtlasID id) {
        return (id >> 16) & 0xffffffffffff;
    }

    inline void updatePlot(GrDrawBatch::Target*, AtlasID*, BatchPlot*);

    inline void makeMRU(BatchPlot* plot) {
        if (fPlotList.head() == plot) {
            return;
        }

        fPlotList.remove(plot);
        fPlotList.addToHead(plot);
    }

    inline void processEviction(AtlasID);

    GrTexture* fTexture;
    int        fPlotWidth;
    int        fPlotHeight;
    SkDEBUGCODE(uint32_t fNumPlots;)

    uint64_t fAtlasGeneration;

    struct EvictionData {
        EvictionFunc fFunc;
        void* fData;
    };

    SkTDArray<EvictionData> fEvictionCallbacks;
    // allocated array of GrBatchPlots
    SkAutoTUnref<BatchPlot>* fPlotArray;
    // LRU list of GrPlots (MRU at head - LRU at tail)
    GrBatchPlotList fPlotList;
};

#endif
