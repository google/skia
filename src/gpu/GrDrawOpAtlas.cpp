/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOpAtlas.h"

#include "GrContext.h"
#include "GrOpFlushState.h"
#include "GrRectanizer.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "GrTracing.h"

std::unique_ptr<GrDrawOpAtlas> GrDrawOpAtlas::Make(GrContext* ctx, GrPixelConfig config,
                                                   int width, int height,
                                                   int numPlotsX, int numPlotsY,
                                                   GrDrawOpAtlas::EvictionFunc func,
                                                   void* data) {
    std::unique_ptr<GrDrawOpAtlas> atlas(
            new GrDrawOpAtlas(ctx, config, width, height, numPlotsX, numPlotsY));
    if (!atlas->getProxies()[0]) {
        return nullptr;
    }

    atlas->registerEvictionCallback(func, data);
    return atlas;
}


////////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::Plot::Plot(int pageIndex, int plotIndex, uint64_t genID, int offX, int offY,
                          int width, int height, GrPixelConfig config)
        : fLastUpload(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fLastUse(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fPageIndex(pageIndex)
        , fPlotIndex(plotIndex)
        , fGenID(genID)
        , fID(CreateId(fPageIndex, fPlotIndex, fGenID))
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

GrDrawOpAtlas::Plot::~Plot() {
    sk_free(fData);
    delete fRects;
}

bool GrDrawOpAtlas::Plot::addSubImage(int width, int height, const void* image, SkIPoint16* loc) {
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
    // copy into the data buffer, swizzling as we go if this is ARGB data
    if (4 == fBytesPerPixel && kSkia8888_GrPixelConfig == kBGRA_8888_GrPixelConfig) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA(reinterpret_cast<uint32_t*>(dataPtr), imagePtr, width);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, rowBytes);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    }

    fDirtyRect.join(loc->fX, loc->fY, loc->fX + width, loc->fY + height);

    loc->fX += fOffset.fX;
    loc->fY += fOffset.fY;
    SkDEBUGCODE(fDirty = true;)

    return true;
}

void GrDrawOpAtlas::Plot::uploadToTexture(GrDrawOp::WritePixelsFn& writePixels,
                                          GrTextureProxy* proxy) {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData && proxy && proxy->priv().peekTexture());
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    writePixels(proxy, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
                fDirtyRect.width(), fDirtyRect.height(), fConfig, dataPtr, rowBytes);
    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

void GrDrawOpAtlas::Plot::resetRects() {
    if (fRects) {
        fRects->reset();
    }

    fGenID++;
    fID = CreateId(fPageIndex, fPlotIndex, fGenID);

    // zero out the plot
    if (fData) {
        sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
    }

    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

///////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::GrDrawOpAtlas(GrContext* context, GrPixelConfig config, int width, int height,
                             int numPlotsX, int numPlotsY)
        : fContext(context)
        , fPixelConfig(config)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fAtlasGeneration(kInvalidAtlasGeneration + 1) {

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = fTextureWidth;
    desc.fHeight = fTextureHeight;
    desc.fConfig = fPixelConfig;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    // TODO: Determine how to avoid having to do this. (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    sk_sp<GrTexture> texture(context->resourceProvider()->createApproxTexture(desc, kFlags));
    if (texture) {
        // MDB TODO: for now, wrap an instantiated texture. Having the deferred instantiation
        // possess the correct properties (e.g., no pendingIO) should fall out of the system but
        // should receive special attention.
        // Note: When switching over to the deferred proxy, use the kExact flag to create
        // the atlas and assert that the width & height are powers of 2.
        fProxies[0] = GrSurfaceProxy::MakeWrapped(std::move(texture),
                                                       kTopLeft_GrSurfaceOrigin);
    }

    fPlotWidth = fTextureWidth / numPlotsX;
    fPlotHeight = fTextureHeight / numPlotsY;
    SkASSERT(numPlotsX * numPlotsY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    SkDEBUGCODE(fNumPlots = numPlotsX * numPlotsY;)
    SkDEBUGCODE(fNumPages = 1;)

    // set up allocated plots
    fPages[0].fPlotArray.reset(new sk_sp<Plot>[ numPlotsX * numPlotsY ]);

    sk_sp<Plot>* currPlot = fPages[0].fPlotArray.get();
    for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            uint32_t index = r * numPlotsX + c;
            currPlot->reset(
                    new Plot(0, index, 1, x, y, fPlotWidth, fPlotHeight, fPixelConfig));

            // build LRU list
            fPages[0].fPlotList.addToHead(currPlot->get());
            ++currPlot;
        }
    }
}

void GrDrawOpAtlas::processEviction(AtlasID id) {
    for (int i = 0; i < fEvictionCallbacks.count(); i++) {
        (*fEvictionCallbacks[i].fFunc)(id, fEvictionCallbacks[i].fData);
    }
}

inline bool GrDrawOpAtlas::updatePlot(GrDrawOp::Target* target, AtlasID* id, Plot* plot) {
    int pageIdx = GetPageIndexFromID(plot->id());
    this->makeMRU(plot, pageIdx);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (target->hasDrawBeenFlushed(plot->lastUploadToken())) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
        // Once it is deferred more care must be taken upon instantiation failure.
        if (!fProxies[pageIdx]->instantiate(fContext->resourceProvider())) {
            return false;
        }

        GrTextureProxy* proxy = fProxies[pageIdx].get();

        GrDrawOpUploadToken lastUploadToken = target->addAsapUpload(
            [plotsp, proxy] (GrDrawOp::WritePixelsFn& writePixels) {
                plotsp->uploadToTexture(writePixels, proxy);
            }
        );
        plot->setLastUploadToken(lastUploadToken);
    }
    *id = plot->id();
    return true;
}

bool GrDrawOpAtlas::addToAtlas(AtlasID* id, GrDrawOp::Target* target, int width, int height,
                               const void* image, SkIPoint16* loc) {
    // Eventually we will iterate through these, for now just use the one.
    int pageIdx = 0;

    // We should already have a texture, TODO clean this up
    SkASSERT(fProxies[pageIdx]);
    if (width > fPlotWidth || height > fPlotHeight) {
        return false;
    }

    // now look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);
    Plot* plot;
    while ((plot = plotIter.get())) {
        SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
        if (plot->addSubImage(width, height, image, loc)) {
            return this->updatePlot(target, id, plot);
        }
        plotIter.next();
    }

    // If the above fails, then see if the least recently refed plot has already been flushed to the
    // gpu
    plot = fPages[pageIdx].fPlotList.tail();
    SkASSERT(plot);
    if (target->hasDrawBeenFlushed(plot->lastUseToken())) {
        this->processEviction(plot->id());
        plot->resetRects();
        SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == plot->bpp());
        SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
        SkASSERT(verify);
        if (!this->updatePlot(target, id, plot)) {
            return false;
        }

        fAtlasGeneration++;
        return true;
    }

    // TODO: at this point try to create a new page and add to it before evicting

    // If this plot has been used in a draw that is currently being prepared by an op, then we have
    // to fail. This gives the op a chance to enqueue the draw, and call back into this function.
    // When that draw is enqueued, the draw token advances, and the subsequent call will continue
    // past this branch and prepare an inline upload that will occur after the enqueued draw which
    // references the plot's pre-upload content.
    if (plot->lastUseToken() == target->nextDrawToken()) {
        return false;
    }

    this->processEviction(plot->id());
    fPages[pageIdx].fPlotList.remove(plot);
    sk_sp<Plot>& newPlot = fPages[pageIdx].fPlotArray[plot->index()];
    newPlot.reset(plot->clone());

    fPages[pageIdx].fPlotList.addToHead(newPlot.get());
    SkASSERT(GrBytesPerPixel(fProxies[pageIdx]->config()) == newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded asap.
    // With c+14 we could move sk_sp into lambda to only ref once.
    sk_sp<Plot> plotsp(SkRef(newPlot.get()));
    // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
    // Once it is deferred more care must be taken upon instantiation failure.
    if (!fProxies[pageIdx]->instantiate(fContext->resourceProvider())) {
        return false;
    }
    GrTextureProxy* proxy = fProxies[pageIdx].get();

    GrDrawOpUploadToken lastUploadToken = target->addInlineUpload(
        [plotsp, proxy] (GrDrawOp::WritePixelsFn& writePixels) {
            plotsp->uploadToTexture(writePixels, proxy);
        }
    );
    newPlot->setLastUploadToken(lastUploadToken);

    *id = newPlot->id();

    fAtlasGeneration++;
    return true;
}
