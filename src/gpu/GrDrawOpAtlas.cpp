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
                                                   const SkISize& plotSize,
                                                   const SkIPoint& startPlots,
                                                   const SkIPoint& maxPlots,
                                                   GrDrawOpAtlas::EvictionFunc func,
                                                   void* data) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = startPlots.fX * plotSize.fWidth;
    desc.fHeight = startPlots.fY * plotSize.fHeight;
    desc.fConfig = config;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    // TODO: Determine how to avoid having to do this. (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    sk_sp<GrTexture> texture(ctx->resourceProvider()->createApproxTexture(desc, kFlags));
    if (!texture) {
        return nullptr;
    }

    // MDB TODO: for now, wrap an instantiated texture. Having the deferred instantiation
    // possess the correct properties (e.g., no pendingIO) should fall out of the system but
    // should receive special attention.
    // Note: When switching over to the deferred proxy, use the kExact flag to create
    // the atlas and assert that the width & height are powers of 2.
    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(texture),
                                                              kTopLeft_GrSurfaceOrigin);
    if (!proxy) {
        return nullptr;
    }

    std::unique_ptr<GrDrawOpAtlas> atlas(new GrDrawOpAtlas(ctx, std::move(proxy), plotSize,
                                                           startPlots, maxPlots));
    atlas->registerEvictionCallback(func, data);
    return atlas;
}


////////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::Plot::Plot(int index, uint64_t genID, const SkIPoint16& xyIndices,
                          const SkISize& size, GrPixelConfig config, bool active)
        : fLastUpload(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fLastUse(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fIndex(index)
        , fGenID(genID)
        , fID(CreateId(fIndex, fGenID))
        , fData(nullptr)
        , fSize(size)
        , fXYIndices(xyIndices)
        , fRects(nullptr)
        , fOffset(SkIPoint16::Make(fXYIndices.fX * fSize.fWidth, fXYIndices.fY * fSize.fHeight))
        , fConfig(config)
        , fBytesPerPixel(GrBytesPerPixel(config))
        , fActive(active)
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
    SkASSERT(width <= fSize.fWidth && height <= fSize.fHeight);

    if (!fRects) {
        fRects = GrRectanizer::Factory(fSize.fWidth, fSize.fHeight);
    }

    if (!fRects->addRect(width, height, loc)) {
        return false;
    }

    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(sk_calloc_throw(fBytesPerPixel * fSize.area()));
    }
    size_t rowBytes = width * fBytesPerPixel;
    const unsigned char* imagePtr = (const unsigned char*)image;
    // point ourselves at the right starting spot
    unsigned char* dataPtr = fData;
    dataPtr += fBytesPerPixel * fSize.fWidth * loc->fY;
    dataPtr += fBytesPerPixel * loc->fX;
    // copy into the data buffer, swizzling as we go if this is ARGB data
    if (4 == fBytesPerPixel && kSkia8888_GrPixelConfig == kBGRA_8888_GrPixelConfig) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA(reinterpret_cast<uint32_t*>(dataPtr), imagePtr, width);
            dataPtr += fBytesPerPixel * fSize.fWidth;
            imagePtr += rowBytes;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, rowBytes);
            dataPtr += fBytesPerPixel * fSize.fWidth;
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
    size_t rowBytes = fBytesPerPixel * fSize.fWidth;
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
    fID = CreateId(fIndex, fGenID);

    // zero out the plot
    if (fData) {
        sk_bzero(fData, fBytesPerPixel * fSize.area());
    }

    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

///////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::GrDrawOpAtlas(GrContext* context, sk_sp<GrTextureProxy> proxy,
                             const SkISize& plotSize,
                             const SkIPoint& startPlots,
                             const SkIPoint& maxPlots)
        : fContext(context)
        , fProxy(std::move(proxy))
        , fPlotSize(plotSize)
        , fActivePlots(startPlots)
        , fMinPlots(startPlots)
        , fMaxPlots(maxPlots)
        , fAtlasGeneration(kInvalidAtlasGeneration + 1) {
    SkASSERT(startPlots.fX * startPlots.fY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(maxPlots.fX * maxPlots.fY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(fPlotSize.fWidth * startPlots.fX == fProxy->width());
    SkASSERT(fPlotSize.fHeight * startPlots.fY == fProxy->height());

    // Allocate the maximum number of plots but only have the starter set be active. This
    // lets us keep the indices of the plots constant.
    fPlotArray.reset(new sk_sp<Plot>[ fMaxPlots.fX * fMaxPlots.fY ]);

    for (int y = 0; y < maxPlots.fY; ++y) {
        for (int x = 0; x < maxPlots.fX; ++x) {
            uint32_t index = y * maxPlots.fX + x;
            bool active =  x < startPlots.fX && y < startPlots.fY;
            fPlotArray[index].reset(new Plot(index, 1, SkIPoint16::Make(x, y),
                                             fPlotSize, fProxy->config(), active));

            // Only active plots are in the LRU list
            if (active) {
                fActivePlotList.addToHead(fPlotArray[index].get());
            }
        }
    }
}

void GrDrawOpAtlas::processEviction(AtlasID id) {
    for (int i = 0; i < fEvictionCallbacks.count(); i++) {
        (*fEvictionCallbacks[i].fFunc)(id, fEvictionCallbacks[i].fData);
    }
}

inline bool GrDrawOpAtlas::updatePlot(GrDrawOp::Target* target, AtlasID* id, Plot* plot) {
    this->makeMRU(plot);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (target->hasDrawBeenFlushed(plot->lastUploadToken())) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
        // Once it is deferred more care must be taken upon instantiation failure.
        if (!fProxy->instantiate(fContext->resourceProvider())) {
            return false;
        }

        GrTextureProxy* proxy = fProxy.get();

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
    // We should already have a texture, TODO clean this up
    SkASSERT(fProxy);
    if (width > fPlotSize.fWidth || height > fPlotSize.fHeight) {
        return false;
    }

    // now look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fActivePlotList, PlotList::Iter::kHead_IterStart);
    Plot* plot;
    while ((plot = plotIter.get())) {
        SkASSERT(GrBytesPerPixel(fProxy->config()) == plot->bpp());
        if (plot->addSubImage(width, height, image, loc)) {
            return this->updatePlot(target, id, plot);
        }
        plotIter.next();
    }

    // If the above fails, then see if the least recently refed plot has already been flushed to the
    // gpu
    plot = fActivePlotList.tail();
    SkASSERT(plot);
    if (target->hasDrawBeenFlushed(plot->lastUseToken())) {
        this->processEviction(plot->id());
        plot->resetRects();
        SkASSERT(GrBytesPerPixel(fProxy->config()) == plot->bpp());
        SkDEBUGCODE(bool verify = )plot->addSubImage(width, height, image, loc);
        SkASSERT(verify);
        if (!this->updatePlot(target, id, plot)) {
            return false;
        }

        fAtlasGeneration++;
        return true;
    }

    // If this plot has been used in a draw that is currently being prepared by an op, then we have
    // to fail. This gives the op a chance to enqueue the draw, and call back into this function.
    // When that draw is enqueued, the draw token advances, and the subsequent call will continue
    // past this branch and prepare an inline upload that will occur after the enqueued draw which
    // references the plot's pre-upload content.
    if (plot->lastUseToken() == target->nextDrawToken()) {
        return false;
    }

    this->processEviction(plot->id());
    fActivePlotList.remove(plot);
    sk_sp<Plot>& newPlot = fPlotArray[plot->index()];
    newPlot.reset(plot->clone());

    fActivePlotList.addToHead(newPlot.get());
    SkASSERT(GrBytesPerPixel(fProxy->config()) == newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded asap.
    // With c+14 we could move sk_sp into lambda to only ref once.
    sk_sp<Plot> plotsp(SkRef(newPlot.get()));
    // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
    // Once it is deferred more care must be taken upon instantiation failure.
    if (!fProxy->instantiate(fContext->resourceProvider())) {
        return false;
    }
    GrTextureProxy* proxy = fProxy.get();

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

bool GrDrawOpAtlas::growInX() {
    SkASSERT(fActivePlots.fX < fMaxPlots.fX);

    for (int y = 0; y < fActivePlots.fY; ++y) {
        Plot* newPlot = fPlotArray[y*fMaxPlots.fX + fActivePlots.fX].get();

        // New atlased objects are added to the MRU plots first. Since we are presumably in a
        // bind (bc we're growing the atlas) make the new plots the MRU ones.
        SkASSERT(!newPlot->fActive);
        newPlot->fActive = true;
        fActivePlotList.addToHead(newPlot);
    }

    fActivePlots.fX++;

    SkDEBUGCODE(this->validate());
    return true;
}

bool GrDrawOpAtlas::growInY() {
    SkASSERT(fActivePlots.fY < fMaxPlots.fY);

    for (int x = 0; x < fActivePlots.fX; ++x) {
        Plot* newPlot = fPlotArray[fActivePlots.fY*fMaxPlots.fX + x].get();

        // New atlased objects are added to the MRU plots first. Since we are presumably in a
        // bind (bc we're growing the atlas) make the new plots the MRU ones.
        SkASSERT(!newPlot->fActive);
        newPlot->fActive = true;
        fActivePlotList.addToHead(newPlot);
    }

    fActivePlots.fY++;

    SkDEBUGCODE(this->validate());
    return true;
}

bool GrDrawOpAtlas::grow() {
    SkDEBUGCODE(this->validate());

    if (fActivePlots.fX >= fMaxPlots.fX) {
        if (fActivePlots.fY >= fMaxPlots.fY) {
            return false;  // can't grow anymore
        }
    
        return this->growInY();
    } else if (fActivePlots.fY >= fMaxPlots.fY) {
        return this->growInX();
    }

    if (this->curAtlasSize().fWidth > this->curAtlasSize().fHeight) {
        return this->growInY();
    }

    return this->growInX();
}

bool GrDrawOpAtlas::shrinkInX(GrOpFlushState* state) {
    SkASSERT(fActivePlots.fX > fMinPlots.fX);

    fActivePlots.fX--;

    for (int y = 0; y < fActivePlots.fY; ++y) {
        Plot* oldPlot = fPlotArray[y*fMaxPlots.fX + fActivePlots.fX].get();

        SkASSERT(state->hasDrawBeenFlushed(oldPlot->lastUseToken()));
        this->processEviction(oldPlot->id());
        oldPlot->resetRects();

        SkASSERT(oldPlot->fActive);
        oldPlot->fActive = false;
        fActivePlotList.remove(oldPlot);
    }

    SkDEBUGCODE(this->validate());
    return true;
}

bool GrDrawOpAtlas::shrinkInY(GrOpFlushState* state) {
    SkASSERT(fActivePlots.fY > fMinPlots.fY);

    fActivePlots.fY--;

    for (int x = 0; x < fActivePlots.fX; ++x) {
        Plot* oldPlot = fPlotArray[fActivePlots.fY*fMaxPlots.fX + x].get();

        SkASSERT(state->hasDrawBeenFlushed(oldPlot->lastUseToken()));
        this->processEviction(oldPlot->id());
        oldPlot->resetRects();

        SkASSERT(oldPlot->fActive);
        oldPlot->fActive = false;
        fActivePlotList.remove(oldPlot);
    }

    SkDEBUGCODE(this->validate());
    return true;
}

bool GrDrawOpAtlas::shrink(GrOpFlushState* state) {
    SkDEBUGCODE(this->validate());

    if (fActivePlots.fX <= fMinPlots.fX) {
        if (fActivePlots.fY <= fMinPlots.fY) {
            return false;  // can't shrink anymore
        }

        return this->shrinkInY(state);
    } else if (fActivePlots.fY <= fMinPlots.fY) {
        return this->shrinkInX(state);
    }

    if (this->curAtlasSize().fWidth > this->curAtlasSize().fHeight) {
        return this->shrinkInX(state);
    }

    return this->shrinkInY(state);
}

#ifdef SK_DEBUG
void GrDrawOpAtlas::validate() const {
    SkASSERT(fActivePlots.fX >= fMinPlots.fX && fActivePlots.fX <= fMaxPlots.fX);
    SkASSERT(fActivePlots.fY >= fMinPlots.fY && fActivePlots.fY <= fMaxPlots.fY);

    for (int i = 0; i < fMaxPlots.fX * fMaxPlots.fY; ++i) {
        Plot* plot = fPlotArray[i].get();

        SkASSERT(i == plot->fIndex);
        SkASSERT(fPlotSize == plot->fSize);
        SkASSERT(i == plot->fXYIndices.fY * fMaxPlots.fX + plot->fXYIndices.fX);
        SkASSERT(plot->fOffset.fX == plot->fXYIndices.fX * fPlotSize.fWidth);
        SkASSERT(plot->fOffset.fY == plot->fXYIndices.fY * fPlotSize.fHeight);
        SkASSERT(fProxy->config() == plot->fConfig);

        if (plot->fActive) {
            SkASSERT(plot->fXYIndices.fX <= fActivePlots.fX && plot->fXYIndices.fY <= fActivePlots.fY);
            SkASSERT(fActivePlotList.isInList(plot));
        } else {
            SkASSERT(plot->fXYIndices.fX >= fActivePlots.fX || plot->fXYIndices.fY >= fActivePlots.fY);
            SkASSERT(plot->fXYIndices.fX < fMaxPlots.fX && plot->fXYIndices.fY < fMaxPlots.fY);
            SkASSERT(!fActivePlotList.isInList(plot));
        }
    }

    SkASSERT(fActivePlotList.countEntries() == fActivePlots.fX * fActivePlots.fY);

    PlotList::Iter iter;
    iter.init(fActivePlotList, PlotList::Iter::kHead_IterStart);
    for (Plot* cur = iter.get(); cur; cur = iter.next()) {
        SkASSERT(cur->fActive);
    }
}
#endif

