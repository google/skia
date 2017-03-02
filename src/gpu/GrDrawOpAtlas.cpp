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
#include "GrTracing.h"

std::unique_ptr<GrDrawOpAtlas> GrDrawOpAtlas::Make(GrContext* ctx, GrPixelConfig config,
                                                   int width, int height,
                                                   int numPlotsX, int numPlotsY,
                                                   GrDrawOpAtlas::EvictionFunc func,
                                                   void* data) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = width;
    desc.fHeight = height;
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
    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(texture));
    if (!proxy) {
        return nullptr;
    }

    std::unique_ptr<GrDrawOpAtlas> atlas(
            new GrDrawOpAtlas(ctx, std::move(proxy), numPlotsX, numPlotsY));
    atlas->registerEvictionCallback(func, data);
    return atlas;
}


////////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::Plot::Plot(int index, uint64_t genID, int offX, int offY, int width, int height,
                          GrPixelConfig config)
        : fLastUpload(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fLastUse(GrDrawOpUploadToken::AlreadyFlushedToken())
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
                                          GrTexture* texture) {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData && texture);
    TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "GrDrawOpAtlas::Plot::uploadToTexture");
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    writePixels(texture, fOffset.fX + fDirtyRect.fLeft, fOffset.fY + fDirtyRect.fTop,
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
        sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
    }

    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

///////////////////////////////////////////////////////////////////////////////

GrDrawOpAtlas::GrDrawOpAtlas(GrContext* context, sk_sp<GrTextureProxy> proxy,
                             int numPlotsX, int numPlotsY)
        : fContext(context)
        , fProxy(std::move(proxy))
        , fAtlasGeneration(kInvalidAtlasGeneration + 1) {
    fPlotWidth = fProxy->width() / numPlotsX;
    fPlotHeight = fProxy->height() / numPlotsY;
    SkASSERT(numPlotsX * numPlotsY <= BulkUseTokenUpdater::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fProxy->width());
    SkASSERT(fPlotHeight * numPlotsY == fProxy->height());

    SkDEBUGCODE(fNumPlots = numPlotsX * numPlotsY;)

    // We currently do not support compressed atlases...
    SkASSERT(!GrPixelConfigIsCompressed(fProxy->desc().fConfig));

    // set up allocated plots
    fPlotArray.reset(new sk_sp<Plot>[ numPlotsX * numPlotsY ]);

    sk_sp<Plot>* currPlot = fPlotArray.get();
    for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            uint32_t index = r * numPlotsX + c;
            currPlot->reset(
                    new Plot(index, 1, x, y, fPlotWidth, fPlotHeight, fProxy->desc().fConfig));

            // build LRU list
            fPlotList.addToHead(currPlot->get());
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
    this->makeMRU(plot);

    // If our most recent upload has already occurred then we have to insert a new
    // upload. Otherwise, we already have a scheduled upload that hasn't yet ocurred.
    // This new update will piggy back on that previously scheduled update.
    if (target->hasDrawBeenFlushed(plot->lastUploadToken())) {
        // With c+14 we could move sk_sp into lamba to only ref once.
        sk_sp<Plot> plotsp(SkRef(plot));

        // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
        // Once it is deferred more care must be taken upon instantiation failure.
        GrTexture* texture = fProxy->instantiate(fContext->resourceProvider());
        if (!texture) {
            return false;
        }

        GrDrawOpUploadToken lastUploadToken = target->addAsapUpload(
            [plotsp, texture] (GrDrawOp::WritePixelsFn& writePixels) {
                plotsp->uploadToTexture(writePixels, texture);
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
    if (width > fPlotWidth || height > fPlotHeight) {
        return false;
    }

    // now look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPlotList, PlotList::Iter::kHead_IterStart);
    Plot* plot;
    while ((plot = plotIter.get())) {
        SkASSERT(GrBytesPerPixel(fProxy->desc().fConfig) == plot->bpp());
        if (plot->addSubImage(width, height, image, loc)) {
            return this->updatePlot(target, id, plot);
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
        SkASSERT(GrBytesPerPixel(fProxy->desc().fConfig) == plot->bpp());
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
    fPlotList.remove(plot);
    sk_sp<Plot>& newPlot = fPlotArray[plot->index()];
    newPlot.reset(plot->clone());

    fPlotList.addToHead(newPlot.get());
    SkASSERT(GrBytesPerPixel(fProxy->desc().fConfig) == newPlot->bpp());
    SkDEBUGCODE(bool verify = )newPlot->addSubImage(width, height, image, loc);
    SkASSERT(verify);

    // Note that this plot will be uploaded inline with the draws whereas the
    // one it displaced most likely was uploaded asap.
    // With c+14 we could move sk_sp into lambda to only ref once.
    sk_sp<Plot> plotsp(SkRef(newPlot.get()));
    // MDB TODO: this is currently fine since the atlas' proxy is always pre-instantiated.
    // Once it is deferred more care must be taken upon instantiation failure.
    GrTexture* texture = fProxy->instantiate(fContext->resourceProvider());
    if (!texture) {
        return false;
    }

    GrDrawOpUploadToken lastUploadToken = target->addInlineUpload(
        [plotsp, texture] (GrDrawOp::WritePixelsFn& writePixels) {
            plotsp->uploadToTexture(writePixels, texture);
        }
    );
    newPlot->setLastUploadToken(lastUploadToken);

    *id = newPlot->id();

    fAtlasGeneration++;
    return true;
}
