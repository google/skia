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

class BatchPlot;
class GrBatchTarget;
class GrRectanizer;

typedef SkTInternalLList<BatchPlot> GrBatchPlotList;

class GrBatchAtlas {
public:
    typedef uint64_t BatchToken;
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
    // NULL is returned if the subimage cannot fit in the atlas.
    // If provided, the image data will be written to the CPU-side backing bitmap.
    // NOTE: If the client intends to refer to the atlas, they should immediately call 'setUseToken'
    // with the currentToken from the batch target, otherwise the next call to addToAtlas might
    // cause an eviction
    bool addToAtlas(AtlasID*, GrBatchTarget*, int width, int height, const void* image,
                    SkIPoint16* loc);

    GrTexture* getTexture() const { return fTexture; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }
    bool hasID(AtlasID id);

    // To ensure the atlas does not evict a given entry, the client must set the last use token
    void setLastUseToken(AtlasID id, BatchToken batchToken);
    void registerEvictionCallback(EvictionFunc func, void* userData) {
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

    void setLastUseTokenBulk(const BulkUseTokenUpdater& reffer, BatchToken);

    static const int kGlyphMaxDim = 256;
    static bool GlyphTooLargeForAtlas(int width, int height) {
        return width > kGlyphMaxDim || height > kGlyphMaxDim;
    }

private:
    static uint32_t GetIndexFromID(AtlasID id) {
        return id & 0xffff;
    }

    // top 48 bits are reserved for the generation ID
    static uint64_t GetGenerationFromID(AtlasID id) {
        return (id >> 16) & 0xffffffffffff;
    }

    inline void updatePlot(GrBatchTarget*, AtlasID*, BatchPlot*);

    inline void makeMRU(BatchPlot* plot);

    inline void processEviction(AtlasID);

    GrTexture* fTexture;
    uint32_t fNumPlotsX;
    uint32_t fNumPlotsY;
    uint32_t fPlotWidth;
    uint32_t fPlotHeight;
    size_t fBPP;
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
