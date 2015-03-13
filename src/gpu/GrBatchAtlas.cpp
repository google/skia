/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchAtlas.h"
#include "GrBatchTarget.h"
#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrTracing.h"

// for testing
#define ATLAS_STATS 0
#if ATLAS_STATS
static int g_UploadCount = 0;
#endif

static inline void adjust_for_offset(SkIPoint16* loc, const SkIPoint16& offset) {
    loc->fX += offset.fX;
    loc->fY += offset.fY;
}

static GrBatchAtlas::AtlasID create_id(int index, int generation) {
    // Generation ID can roll over because we only check for equality
    SkASSERT(index < (1 << 16));
    return generation << 16 | index;
}

// The backing GrTexture for a GrBatchAtlas is broken into a spatial grid of GrBatchPlots.
// The GrBatchPlots keep track of subimage placement via their GrRectanizer. In turn, a GrBatchPlot
// manages the lifetime of its data using two tokens, a last ref toke and a last upload token.
// Once a GrBatchPlot is "full" (i.e. there is no room for the new subimage according to the
// GrRectanizer), it can no longer be used unless the last ref on the GrPlot has already been
// flushed through to the gpu.

class BatchPlot : public SkRefCnt {
public:
    typedef GrBatchAtlas::BatchToken BatchToken;
    SK_DECLARE_INST_COUNT(BatchPlot);
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BatchPlot);

    // index() refers to the index of the plot in the owning GrAtlas's plot array.  genID() is a
    // monotonically incrementing number which is bumped every time the cpu backing store is
    // wiped, or when the plot itself is evicted from the atlas(ie, there is continuity in genID()
    // across atlas spills)
    int index() const { return fIndex; }
    int genID() const { return fGenID; }
    GrBatchAtlas::AtlasID id() { return fID; }

    GrTexture* texture() const { return fTexture; }

    bool addSubImage(int width, int height, const void* image, SkIPoint16* loc, size_t rowBytes)  {
        if (!fRects->addRect(width, height, loc)) {
            return false;
        }

        SkASSERT(fData);
        const unsigned char* imagePtr = (const unsigned char*)image;
        // point ourselves at the right starting spot
        unsigned char* dataPtr = fData;
        dataPtr += fBytesPerPixel * fWidth * loc->fY;
        dataPtr += fBytesPerPixel * loc->fX;
        // copy into the data buffer
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, rowBytes);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }

        fDirtyRect.join(loc->fX, loc->fY, loc->fX + width, loc->fY + height);
        adjust_for_offset(loc, fOffset);
        SkDEBUGCODE(fDirty = true;)

#if ATLAS_STATS
        ++g_UploadCount;
#endif

        return true;
    }

    // to manage the lifetime of a plot, we use two tokens.  We use last upload token to know when
    // we can 'piggy back' uploads, ie if the last upload hasn't been flushed to gpu, we don't need
    // to issue a new upload even if we update the cpu backing store.  We use lastref to determine
    // when we can evict a plot from the cache, ie if the last ref has already flushed through
    // the gpu then we can reuse the plot
    BatchToken lastUploadToken() const { return fLastUpload; }
    BatchToken lastRefToken() const { return fLastRef; }
    void setLastUploadToken(BatchToken batchToken) { fLastUpload = batchToken; }
    void setLastRefToken(BatchToken batchToken) { fLastRef = batchToken; }

    void uploadToTexture(GrBatchTarget::TextureUploader uploader)  {
        // We should only be issuing uploads if we are in fact dirty
        SkASSERT(fDirty);
        TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "GrBatchPlot::uploadToTexture");
        SkASSERT(fTexture);
        size_t rowBytes = fBytesPerPixel * fRects->width();
        const unsigned char* dataPtr = fData;
        dataPtr += rowBytes * fDirtyRect.fTop;
        dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
        uploader.writeTexturePixels(fTexture,
                                    fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                                    fDirtyRect.width(), fDirtyRect.height(),
                                    fTexture->config(), dataPtr, rowBytes);
        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false;)
    }

    void resetRects() {
        SkASSERT(fRects);
        fRects->reset();
        fGenID++;
        fID = create_id(fIndex, fGenID);

        // zero out the plot
        SkASSERT(fData);
        memset(fData, 0, fBytesPerPixel * fWidth * fHeight);

        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false;)
    }

    int x() const { return fX; }
    int y() const { return fY; }

private:
    BatchPlot()
        : fLastUpload(0)
        , fLastRef(0)
        , fIndex(-1)
        , fGenID(-1)
        , fID(0)
        , fData(NULL)
        , fWidth(0)
        , fHeight(0)
        , fX(0)
        , fY(0)
        , fTexture(NULL)
        , fRects(NULL)
        , fAtlas(NULL)
        , fBytesPerPixel(1)
    #ifdef SK_DEBUG
        , fDirty(false)
    #endif
    {
        fOffset.set(0, 0);
    }

    ~BatchPlot() {
        SkDELETE_ARRAY(fData);
        fData = NULL;
        delete fRects;
    }

    void init(GrBatchAtlas* atlas, GrTexture* texture, int index, uint32_t generation,
              int offX, int offY, int width, int height, size_t bpp) {
        fIndex = index;
        fGenID = generation;
        fID = create_id(index, generation);
        fWidth = width;
        fHeight = height;
        fX = offX;
        fY = offY;
        fRects = GrRectanizer::Factory(width, height);
        fAtlas = atlas;
        fOffset.set(offX * width, offY * height);
        fBytesPerPixel = bpp;
        fData = NULL;
        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false;)
        fTexture = texture;

        // allocate backing store
        fData = SkNEW_ARRAY(unsigned char, fBytesPerPixel * width * height);
        memset(fData, 0, fBytesPerPixel * width * height);
    }

    BatchToken fLastUpload;
    BatchToken fLastRef;

    uint32_t fIndex;
    uint32_t fGenID;
    GrBatchAtlas::AtlasID fID;
    unsigned char* fData;
    int fWidth;
    int fHeight;
    int fX;
    int fY;
    GrTexture* fTexture;
    GrRectanizer* fRects;
    GrBatchAtlas* fAtlas;
    SkIPoint16 fOffset;        // the offset of the plot in the backing texture
    size_t fBytesPerPixel;
    SkIRect fDirtyRect;
    SkDEBUGCODE(bool fDirty;)

    friend class GrBatchAtlas;

    typedef SkRefCnt INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

class GrPlotUploader : public GrBatchTarget::Uploader {
public:
    GrPlotUploader(BatchPlot* plot)
        : INHERITED(plot->lastUploadToken())
        , fPlot(SkRef(plot)) {
        SkASSERT(plot);
    }

    void upload(GrBatchTarget::TextureUploader uploader) SK_OVERRIDE {
        fPlot->uploadToTexture(uploader);
    }

private:
    SkAutoTUnref<BatchPlot> fPlot;

    typedef GrBatchTarget::Uploader INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBatchAtlas::GrBatchAtlas(GrTexture* texture, int numPlotsX, int numPlotsY)
    : fTexture(texture)
    , fNumPlotsX(numPlotsX)
    , fNumPlotsY(numPlotsY)
    , fPlotWidth(texture->width() / numPlotsX)
    , fPlotHeight(texture->height() / numPlotsY) {
    SkASSERT(fPlotWidth * fNumPlotsX == texture->width());
    SkASSERT(fPlotHeight * fNumPlotsY == texture->height());

    // We currently do not support compressed atlases...
    SkASSERT(!GrPixelConfigIsCompressed(texture->desc().fConfig));

    // set up allocated plots
    fBPP = GrBytesPerPixel(texture->desc().fConfig);
    fPlotArray = SkNEW_ARRAY(SkAutoTUnref<BatchPlot>, (fNumPlotsX * fNumPlotsY));

    SkAutoTUnref<BatchPlot>* currPlot = fPlotArray;
    for (int y = fNumPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = fNumPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            int id = r * fNumPlotsX + c;
            currPlot->reset(SkNEW(BatchPlot));
            (*currPlot)->init(this, texture, id, 0, x, y, fPlotWidth, fPlotHeight, fBPP);

            // build LRU list
            fPlotList.addToHead(currPlot->get());
            ++currPlot;
        }
    }
}

GrBatchAtlas::~GrBatchAtlas() {
    SkSafeUnref(fTexture);
    SkDELETE_ARRAY(fPlotArray);

#if ATLAS_STATS
      SkDebugf("Num uploads: %d\n", g_UploadCount);
#endif
}

void GrBatchAtlas::processEviction(AtlasID id) {
    for (int i = 0; i < fEvictionCallbacks.count(); i++) {
        (*fEvictionCallbacks[i].fFunc)(id, fEvictionCallbacks[i].fData);
    }
}

void GrBatchAtlas::makeMRU(BatchPlot* plot) {
    if (fPlotList.head() == plot) {
        return;
    }

    fPlotList.remove(plot);
    fPlotList.addToHead(plot);
}

inline void GrBatchAtlas::updatePlot(GrBatchTarget* batchTarget, AtlasID* id, BatchPlot* plot) {
    this->makeMRU(plot);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (batchTarget->isIssued(plot->lastUploadToken())) {
        plot->setLastUploadToken(batchTarget->asapToken());
        SkAutoTUnref<GrPlotUploader> uploader(SkNEW_ARGS(GrPlotUploader, (plot)));
        batchTarget->upload(uploader);
    }
    *id = plot->id();
}

bool GrBatchAtlas::addToAtlas(AtlasID* id, GrBatchTarget* batchTarget,
                              int width, int height, const void* image, SkIPoint16* loc) {
    // We should already have a texture, TODO clean this up
    SkASSERT(fTexture && width < fPlotWidth && height < fPlotHeight);

    // now look through all allocated plots for one we can share, in Most Recently Refed order
    GrBatchPlotList::Iter plotIter;
    plotIter.init(fPlotList, GrBatchPlotList::Iter::kHead_IterStart);
    BatchPlot* plot;
    while ((plot = plotIter.get())) {
        if (plot->addSubImage(width, height, image, loc, fBPP * width)) {
            this->updatePlot(batchTarget, id, plot);
            return true;
        }
        plotIter.next();
    }

    // If the above fails, then see if the least recently refed plot has already been flushed to the
    // gpu
    plotIter.init(fPlotList, GrBatchPlotList::Iter::kTail_IterStart);
    plot = plotIter.get();
    SkASSERT(plot);
    if (batchTarget->isIssued(plot->lastRefToken())) {
        this->processEviction(plot->id());
        plot->resetRects();
        SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc, fBPP * width);
        SkASSERT(verify);
        this->updatePlot(batchTarget, id, plot);
        return true;
    }

    // The least recently refed plot hasn't been flushed to the gpu yet, however, if we have flushed
    // it to the batch target than we can reuse it.  Our last ref token is guaranteed to be less
    // than or equal to the current token.  If its 'less than' the current token, than we can spin
    // off the plot(ie let the batch target manage it) and create a new plot in its place in our
    // array.  If it is equal to the currentToken, then the caller has to flush draws to the batch
    // target so we can spin off the plot
    if (plot->lastRefToken() == batchTarget->currentToken()) {
        return false;
    }

    // We take an extra ref here so our plot isn't deleted when we reset its index in the array.
    plot->ref();
    int index = plot->index();
    int x = plot->x();
    int y = plot->y();
    int generation = plot->genID();

    this->processEviction(plot->id());
    fPlotList.remove(plot);
    SkAutoTUnref<BatchPlot>& newPlot = fPlotArray[plot->index()];
    newPlot.reset(SkNEW(BatchPlot));
    newPlot->init(this, fTexture, index, ++generation, x, y, fPlotWidth, fPlotHeight, fBPP);

    fPlotList.addToHead(newPlot.get());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc, fBPP * width);
    SkASSERT(verify);
    newPlot->setLastUploadToken(batchTarget->currentToken());
    SkAutoTUnref<GrPlotUploader> uploader(SkNEW_ARGS(GrPlotUploader, (newPlot)));
    batchTarget->upload(uploader);
    *id = newPlot->id();
    plot->unref();
    return true;
}

bool GrBatchAtlas::hasID(AtlasID id) {
    int index = this->getIndexFromID(id);
    SkASSERT(index < fNumPlotsX * fNumPlotsY);
    return fPlotArray[index]->genID() == this->getGenerationFromID(id);
}

void GrBatchAtlas::setLastRefToken(AtlasID id, BatchToken batchToken) {
    SkASSERT(this->hasID(id));
    int index = this->getIndexFromID(id);
    this->makeMRU(fPlotArray[index]);
    fPlotArray[index]->setLastRefToken(batchToken);
}
