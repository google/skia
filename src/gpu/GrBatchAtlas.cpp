/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchAtlas.h"
#include "GrBatchFlushState.h"
#include "GrRectanizer.h"
#include "GrTracing.h"
#include "GrVertexBuffer.h"

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

    bool addSubImage(int width, int height, const void* image, SkIPoint16* loc)  {
        SkASSERT(width <= fWidth && height <= fHeight);

        if (!fRects) {
            fRects = GrRectanizer::Factory(fWidth, fHeight);
        }

        if (!fRects->addRect(width, height, loc)) {
            return false;
        }

        if (!fData) {
            fData = reinterpret_cast<unsigned char*>(sk_calloc_throw(fBytesPerPixel * fWidth *
                                                                     fHeight));
        }
        size_t rowBytes = width * fBytesPerPixel;
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

        loc->fX += fOffset.fX;
        loc->fY += fOffset.fY;
        SkDEBUGCODE(fDirty = true;)

        return true;
    }

    // To manage the lifetime of a plot, we use two tokens.  We use the last upload token to know
    // when we can 'piggy back' uploads, ie if the last upload hasn't been flushed to gpu, we don't
    // need to issue a new upload even if we update the cpu backing store.  We use lastUse to
    // determine when we can evict a plot from the cache, ie if the last use has already flushed
    // through the gpu then we can reuse the plot.
    GrBatchToken lastUploadToken() const { return fLastUpload; }
    GrBatchToken lastUseToken() const { return fLastUse; }
    void setLastUploadToken(GrBatchToken batchToken) {
        SkASSERT(batchToken >= fLastUpload);
        fLastUpload = batchToken;
    }
    void setLastUseToken(GrBatchToken batchToken) {
        SkASSERT(batchToken >= fLastUse);
        fLastUse = batchToken;
    }

    void uploadToTexture(GrBatchUploader::TextureUploader* uploader, GrTexture* texture)  {
        // We should only be issuing uploads if we are in fact dirty
        SkASSERT(fDirty && fData && texture);
        TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "GrBatchPlot::uploadToTexture");
        size_t rowBytes = fBytesPerPixel * fWidth;
        const unsigned char* dataPtr = fData;
        dataPtr += rowBytes * fDirtyRect.fTop;
        dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
        uploader->writeTexturePixels(texture,
                                     fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                                     fDirtyRect.width(), fDirtyRect.height(),
                                     fConfig, dataPtr, rowBytes);
        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false;)
    }

    void resetRects() {
        if (fRects) {
            fRects->reset();
        }

        fGenID++;
        fID = CreateId(fIndex, fGenID);

        // zero out the plot
        if (fData) {
            sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
        }

        fDirtyRect.setEmpty();
        SkDEBUGCODE(fDirty = false;)
    }

private:
    BatchPlot(int index, uint64_t genID, int offX, int offY, int width, int height, 
              GrPixelConfig config)
        : fLastUpload(0)
        , fLastUse(0)
        , fIndex(index)
        , fGenID(genID)
        , fID(CreateId(fIndex, fGenID))
        , fData(nullptr)
        , fWidth(width)
        , fHeight(height)
        , fX(offX)
        , fY(offY)
        , fRects(nullptr)
        , fOffset(SkIPoint16::Make(fX * fWidth, fY * fHeight))
        , fConfig(config)
        , fBytesPerPixel(GrBytesPerPixel(config))
    #ifdef SK_DEBUG
        , fDirty(false)
    #endif
    {
        fDirtyRect.setEmpty();
    }

    ~BatchPlot() override {
        sk_free(fData);
        delete fRects;
    }

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

    GrBatchToken          fLastUpload;
    GrBatchToken          fLastUse;

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

////////////////////////////////////////////////////////////////////////////////

class GrPlotUploader : public GrBatchUploader {
public:
    GrPlotUploader(BatchPlot* plot, GrTexture* texture)
        : INHERITED(plot->lastUploadToken())
        , fPlot(SkRef(plot))
        , fTexture(texture) {
        SkASSERT(plot);
    }

    void upload(TextureUploader* uploader) override {
        fPlot->uploadToTexture(uploader, fTexture);
    }

private:
    SkAutoTUnref<BatchPlot> fPlot;
    GrTexture*              fTexture;

    typedef GrBatchUploader INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrBatchAtlas::GrBatchAtlas(GrTexture* texture, int numPlotsX, int numPlotsY)
    : fTexture(texture)
    , fAtlasGeneration(kInvalidAtlasGeneration + 1) {

    int plotWidth = texture->width() / numPlotsX;
    int plotHeight = texture->height() / numPlotsY;
    SkASSERT(numPlotsX * numPlotsY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(plotWidth * numPlotsX == texture->width());
    SkASSERT(plotHeight * numPlotsY == texture->height());

    SkDEBUGCODE(fNumPlots = numPlotsX * numPlotsY;)

    // We currently do not support compressed atlases...
    SkASSERT(!GrPixelConfigIsCompressed(texture->desc().fConfig));

    // set up allocated plots
    fPlotArray = new SkAutoTUnref<BatchPlot>[numPlotsX * numPlotsY];

    SkAutoTUnref<BatchPlot>* currPlot = fPlotArray;
    for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            uint32_t index = r * numPlotsX + c;
            currPlot->reset(new BatchPlot(index, 1, x, y, plotWidth, plotHeight,
                                          texture->desc().fConfig));

            // build LRU list
            fPlotList.addToHead(currPlot->get());
            ++currPlot;
        }
    }
}

GrBatchAtlas::~GrBatchAtlas() {
    SkSafeUnref(fTexture);
    delete[] fPlotArray;
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

inline void GrBatchAtlas::updatePlot(GrDrawBatch::Target* target, AtlasID* id, BatchPlot* plot) {
    this->makeMRU(plot);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (target->hasTokenBeenFlushed(plot->lastUploadToken())) {
        plot->setLastUploadToken(target->asapToken());
        SkAutoTUnref<GrPlotUploader> uploader(new GrPlotUploader(plot, fTexture));
        target->upload(uploader);
    }
    *id = plot->id();
}

bool GrBatchAtlas::addToAtlas(AtlasID* id, GrDrawBatch::Target* batchTarget,
                              int width, int height, const void* image, SkIPoint16* loc) {
    // We should already have a texture, TODO clean this up
    SkASSERT(fTexture);

    // now look through all allocated plots for one we can share, in Most Recently Refed order
    GrBatchPlotList::Iter plotIter;
    plotIter.init(fPlotList, GrBatchPlotList::Iter::kHead_IterStart);
    BatchPlot* plot;
    while ((plot = plotIter.get())) {
        SkASSERT(GrBytesPerPixel(fTexture->desc().fConfig) == plot->bpp());
        if (plot->addSubImage(width, height, image, loc)) {
            this->updatePlot(batchTarget, id, plot);
            return true;
        }
        plotIter.next();
    }

    // If the above fails, then see if the least recently refed plot has already been flushed to the
    // gpu
    plot = fPlotList.tail();
    SkASSERT(plot);
    if (batchTarget->hasTokenBeenFlushed(plot->lastUseToken())) {
        this->processEviction(plot->id());
        plot->resetRects();
        SkASSERT(GrBytesPerPixel(fTexture->desc().fConfig) == plot->bpp());
        SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
        SkASSERT(verify);
        this->updatePlot(batchTarget, id, plot);
        fAtlasGeneration++;
        return true;
    }

    // The least recently used plot hasn't been flushed to the gpu yet, however, if we have flushed
    // it to the batch target than we can reuse it.  Our last use token is guaranteed to be less
    // than or equal to the current token.  If its 'less than' the current token, than we can spin
    // off the plot (ie let the batch target manage it) and create a new plot in its place in our
    // array.  If it is equal to the currentToken, then the caller has to flush draws to the batch
    // target so we can spin off the plot
    if (plot->lastUseToken() == batchTarget->currentToken()) {
        return false;
    }

    SkASSERT(plot->lastUseToken() < batchTarget->currentToken());
    SkASSERT(!batchTarget->hasTokenBeenFlushed(batchTarget->currentToken()));

    SkASSERT(!plot->unique());  // The GrPlotUpdater should have a ref too

    this->processEviction(plot->id());
    fPlotList.remove(plot);
    SkAutoTUnref<BatchPlot>& newPlot = fPlotArray[plot->index()];
    newPlot.reset(plot->clone());

    fPlotList.addToHead(newPlot.get());
    SkASSERT(GrBytesPerPixel(fTexture->desc().fConfig) == newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded asap.
    newPlot->setLastUploadToken(batchTarget->currentToken());
    SkAutoTUnref<GrPlotUploader> uploader(new GrPlotUploader(newPlot, fTexture));
    batchTarget->upload(uploader);
    *id = newPlot->id();

    fAtlasGeneration++;
    return true;
}

bool GrBatchAtlas::hasID(AtlasID id) {
    uint32_t index = GetIndexFromID(id);
    SkASSERT(index < fNumPlots);
    return fPlotArray[index]->genID() == GetGenerationFromID(id);
}

void GrBatchAtlas::setLastUseToken(AtlasID id, GrBatchToken batchToken) {
    SkASSERT(this->hasID(id));
    uint32_t index = GetIndexFromID(id);
    SkASSERT(index < fNumPlots);
    this->makeMRU(fPlotArray[index]);
    fPlotArray[index]->setLastUseToken(batchToken);
}

void GrBatchAtlas::setLastUseTokenBulk(const BulkUseTokenUpdater& updater,
                                       GrBatchToken batchToken) {
    int count = updater.fPlotsToUpdate.count();
    for (int i = 0; i < count; i++) {
        BatchPlot* plot = fPlotArray[updater.fPlotsToUpdate[i]];
        this->makeMRU(plot);
        plot->setLastUseToken(batchToken);
    }
}
