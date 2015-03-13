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
    typedef uint32_t AtlasID;

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
    bool addToAtlas(AtlasID*, GrBatchTarget*, int width, int height, const void* image,
                    SkIPoint16* loc);

    GrTexture* getTexture() const { return fTexture; }

    bool hasID(AtlasID id);
    void setLastRefToken(AtlasID id, BatchToken batchToken);
    void registerEvictionCallback(EvictionFunc func, void* userData) {
        EvictionData* data = fEvictionCallbacks.append();
        data->fFunc = func;
        data->fData = userData;
    }

private:
    int getIndexFromID(AtlasID id) {
        return id & 0xffff;
    }

    int getGenerationFromID(AtlasID id) {
        return (id >> 16) & 0xffff;
    }

    inline void updatePlot(GrBatchTarget*, AtlasID*, BatchPlot*);

    inline void makeMRU(BatchPlot* plot);

    inline void processEviction(AtlasID);

    GrTexture* fTexture;
    int fNumPlotsX;
    int fNumPlotsY;
    int fPlotWidth;
    int fPlotHeight;
    size_t fBPP;

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
