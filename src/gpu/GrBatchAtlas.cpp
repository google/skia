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

////////////////////////////////////////////////////////////////////////////////

GrBatchAtlas::BatchPlot::BatchPlot(int index, uint64_t genID, int offX, int offY, int width,
                                   int height, GrPixelConfig config)
    : fLastUpload(GrBatchDrawToken::AlreadyFlushedToken())
    , fLastUse(GrBatchDrawToken::AlreadyFlushedToken())
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

GrBatchAtlas::BatchPlot::~BatchPlot() {
    sk_free(fData);
    delete fRects;
}

bool GrBatchAtlas::BatchPlot::addSubImage(int width, int height, const void* image,
                                          SkIPoint16* loc) {
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

void GrBatchAtlas::BatchPlot::uploadToTexture(GrDrawBatch::WritePixelsFn& writePixels,
                                              GrTexture* texture) {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData && texture);
    TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "GrBatchPlot::uploadToTexture");
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    writePixels(texture, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                fDirtyRect.width(), fDirtyRect.height(), fConfig, dataPtr, rowBytes);
    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

void GrBatchAtlas::BatchPlot::resetRects() {
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

inline void GrBatchAtlas::updatePlot(GrDrawBatch::Target* target, AtlasID* id, BatchPlot* plot) {
    this->makeMRU(plot);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (target->hasDrawBeenFlushed(plot->lastUploadToken())) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<BatchPlot> plotsp(SkRef(plot));
        GrTexture* texture = fTexture;
        GrBatchDrawToken lastUploadToken = target->addAsapUpload(
            [plotsp, texture] (GrDrawBatch::WritePixelsFn& writePixels) {
               plotsp->uploadToTexture(writePixels, texture);
            }
        );
        plot->setLastUploadToken(lastUploadToken);
    }
    *id = plot->id();
}

bool GrBatchAtlas::addToAtlas(AtlasID* id, GrDrawBatch::Target* target,
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
            this->updatePlot(target, id, plot);
            return true;
        }
        plotIter.next();
    }

    // If the above fails, then see if the least recently refed plot has already been flushed to the
    // gpu
    plot = fPlotList.tail();
    SkASSERT(plot);
    if (target->hasDrawBeenFlushed(plot->lastUseToken())) {
        this->processEviction(plot->id());
        plot->resetRects();
        SkASSERT(GrBytesPerPixel(fTexture->desc().fConfig) == plot->bpp());
        SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
        SkASSERT(verify);
        this->updatePlot(target, id, plot);
        fAtlasGeneration++;
        return true;
    }

    // If this plot has been used in a draw that is currently being prepared by a batch, then we
    // have to fail. This gives the batch a chance to enqueue the draw, and call back into this
    // function. When that draw is enqueued, the draw token advances, and the subsequent call will
    // continue past this branch and prepare an inline upload that will occur after the enqueued
    // draw which references the plot's pre-upload content.
    if (plot->lastUseToken() == target->nextDrawToken()) {
        return false;
    }

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
    // With c+14 we could move sk_sp into lamba to only ref once.
    sk_sp<BatchPlot> plotsp(SkRef(newPlot.get()));
    GrTexture* texture = fTexture;
    GrBatchDrawToken lastUploadToken = target->addInlineUpload(
        [plotsp, texture] (GrDrawBatch::WritePixelsFn& writePixels) {
            plotsp->uploadToTexture(writePixels, texture);
        }
    );
    newPlot->setLastUploadToken(lastUploadToken);

    *id = newPlot->id();

    fAtlasGeneration++;
    return true;
}
