/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/DDLTileHelper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/image/SkImage_Gpu.h"
#include "tools/DDLPromiseImageHelper.h"

void DDLTileHelper::TileData::init(int id,
                                   GrDirectContext* direct,
                                   const SkSurfaceCharacterization& dstSurfaceCharacterization,
                                   const SkIRect& clip,
                                   const SkIRect& paddingOutsets) {
    fID = id;
    fClip = clip;
    fPaddingOutsets = paddingOutsets;

    fPlaybackChar  = dstSurfaceCharacterization.createResized(this->paddedRectSize().width(),
                                                              this->paddedRectSize().height());
    SkASSERT(fPlaybackChar.isValid());

    GrBackendFormat backendFormat = direct->defaultBackendFormat(fPlaybackChar.colorType(),
                                                                 GrRenderable::kYes);
    SkDEBUGCODE(const GrCaps* caps = direct->priv().caps());
    SkASSERT(caps->isFormatTexturable(backendFormat));

    fCallbackContext.reset(new PromiseImageCallbackContext(direct, backendFormat));
}

DDLTileHelper::TileData::TileData() {}
DDLTileHelper::TileData::~TileData() {}

void DDLTileHelper::TileData::createDDL(const SkPicture* picture) {
    SkASSERT(!fDisplayList && picture);

    auto recordingChar = fPlaybackChar.createResized(fClip.width(), fClip.height());
    SkASSERT(recordingChar.isValid());

    SkDeferredDisplayListRecorder recorder(recordingChar);

    // DDL TODO: the DDLRecorder's rContext isn't initialized until getCanvas is called.
    // Maybe set it up in the ctor?
    SkCanvas* recordingCanvas = recorder.getCanvas();

    // We always record the DDL in the (0,0) .. (clipWidth, clipHeight) coordinates
    recordingCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
    recordingCanvas->translate(-fClip.fLeft, -fClip.fTop);

    // Note: in this use case we only render a picture to the deferred canvas
    // but, more generally, clients will use arbitrary draw calls.
    recordingCanvas->drawPicture(picture);

    fDisplayList = recorder.detach();
}

void DDLTileHelper::createComposeDDL() {
    SkASSERT(!fComposeDDL);

    SkDeferredDisplayListRecorder recorder(fDstCharacterization);

    SkCanvas* recordingCanvas = recorder.getCanvas();

    for (int i = 0; i < this->numTiles(); ++i) {
        TileData* tile = &fTiles[i];
        if (!tile->initialized()) {
            continue;
        }

        sk_sp<SkImage> promiseImage = tile->makePromiseImageForDst(
                                           recordingCanvas->recordingContext()->threadSafeProxy());

        SkRect dstRect = SkRect::Make(tile->clipRect());
        SkIRect srcRect = tile->clipRect();
        srcRect.offsetTo(tile->padOffset().x(), tile->padOffset().y());

        SkASSERT(promiseImage->bounds().contains(srcRect));

        recordingCanvas->drawImageRect(promiseImage.get(), SkRect::Make(srcRect), dstRect,
                                       SkSamplingOptions(), nullptr,
                                       SkCanvas::kStrict_SrcRectConstraint);
    }

    fComposeDDL = recorder.detach();
    SkASSERT(fComposeDDL);
}

void DDLTileHelper::TileData::precompile(GrDirectContext* direct) {
    if (!this->initialized()) {
        return;
    }

    SkASSERT(fDisplayList);

    SkDeferredDisplayList::ProgramIterator iter(direct, fDisplayList.get());
    for (; !iter.done(); iter.next()) {
        iter.compile();
    }
}

sk_sp<SkSurface> DDLTileHelper::TileData::makeWrappedTileDest(GrRecordingContext* rContext) {
    SkASSERT(fCallbackContext && fCallbackContext->promiseImageTexture());

    auto promiseImageTexture = fCallbackContext->promiseImageTexture();
    if (!promiseImageTexture->backendTexture().isValid()) {
        return nullptr;
    }

    // Here we are, unfortunately, aliasing the backend texture held by the SkPromiseImageTexture.
    // Both the tile's destination surface and the promise image used to draw the tile will be
    // backed by the same backendTexture - unbeknownst to Ganesh.
    return SkSurface::MakeFromBackendTexture(rContext,
                                             promiseImageTexture->backendTexture(),
                                             fPlaybackChar.origin(),
                                             fPlaybackChar.sampleCount(),
                                             fPlaybackChar.colorType(),
                                             fPlaybackChar.refColorSpace(),
                                             &fPlaybackChar.surfaceProps());
}

void DDLTileHelper::TileData::drawSKPDirectly(GrDirectContext* dContext,
                                              const SkPicture* picture) {
    SkASSERT(!fDisplayList && !fTileSurface && picture);

    fTileSurface = this->makeWrappedTileDest(dContext);
    if (fTileSurface) {
        SkCanvas* tileCanvas = fTileSurface->getCanvas();

        SkASSERT(this->padOffset().isZero() && this->paddedRectSize() == fClip.size());
        tileCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
        tileCanvas->translate(-fClip.fLeft, -fClip.fTop);

        tileCanvas->drawPicture(picture);

        // We can't snap an image here bc, since we're using wrapped backend textures for the
        // surfaces, that would incur a copy.
    }
}

void DDLTileHelper::TileData::draw(GrDirectContext* direct) {
    SkASSERT(fDisplayList && !fTileSurface);

    fTileSurface = this->makeWrappedTileDest(direct);
    if (fTileSurface) {
        fTileSurface->draw(fDisplayList, this->padOffset().x(), this->padOffset().y());

        // We can't snap an image here bc, since we're using wrapped backend textures for the
        // surfaces, that would incur a copy.
    }
}

void DDLTileHelper::TileData::reset() {
    // TODO: when DDLs are re-renderable we don't need to do this
    fDisplayList = nullptr;

    fTileSurface = nullptr;
}

sk_sp<SkImage> DDLTileHelper::TileData::makePromiseImageForDst(
                                                sk_sp<GrContextThreadSafeProxy> threadSafeProxy) {
    SkASSERT(fCallbackContext);

    // The promise image gets a ref on the promise callback context
    sk_sp<SkImage> promiseImage =
                SkImage::MakePromiseTexture(std::move(threadSafeProxy),
                                            fCallbackContext->backendFormat(),
                                            this->paddedRectSize(),
                                            GrMipmapped::kNo,
                                            GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                            fPlaybackChar.colorType(),
                                            kPremul_SkAlphaType,
                                            fPlaybackChar.refColorSpace(),
                                            PromiseImageCallbackContext::PromiseImageFulfillProc,
                                            PromiseImageCallbackContext::PromiseImageReleaseProc,
                                            (void*)this->refCallbackContext().release());
    fCallbackContext->wasAddedToImage();

    return promiseImage;
}

void DDLTileHelper::TileData::CreateBackendTexture(GrDirectContext* direct, TileData* tile) {
    SkASSERT(tile->fCallbackContext && !tile->fCallbackContext->promiseImageTexture());

    const SkSurfaceCharacterization& c = tile->fPlaybackChar;
    GrBackendTexture beTex = direct->createBackendTexture(c.width(), c.height(), c.colorType(),
                                                          GrMipMapped(c.isMipMapped()),
                                                          GrRenderable::kYes);
    tile->fCallbackContext->setBackendTexture(beTex);
}

void DDLTileHelper::TileData::DeleteBackendTexture(GrDirectContext*, TileData* tile) {
    if (!tile->initialized()) {
        return;
    }

    SkASSERT(tile->fCallbackContext);

    // TODO: it seems that, on the Linux bots, backend texture creation is failing
    // a lot (skbug.com/10142)
    SkASSERT(!tile->fCallbackContext->promiseImageTexture() ||
             tile->fCallbackContext->promiseImageTexture()->backendTexture().isValid());

    tile->fTileSurface = nullptr;

    SkASSERT(tile->fCallbackContext->unique());
    tile->fCallbackContext.reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLTileHelper::DDLTileHelper(GrDirectContext* direct,
                             const SkSurfaceCharacterization& dstChar,
                             const SkIRect& viewport,
                             int numXDivisions, int numYDivisions,
                             bool addRandomPaddingToDst)
        : fNumXDivisions(numXDivisions)
        , fNumYDivisions(numYDivisions)
        , fTiles(numXDivisions * numYDivisions)
        , fDstCharacterization(dstChar) {
    SkASSERT(fNumXDivisions > 0 && fNumYDivisions > 0);

    int xTileSize = viewport.width()/fNumXDivisions;
    int yTileSize = viewport.height()/fNumYDivisions;

    SkRandom rand;

    // Create the destination tiles
    for (int y = 0, yOff = 0; y < fNumYDivisions; ++y, yOff += yTileSize) {
        int ySize = (y < fNumYDivisions-1) ? yTileSize : viewport.height()-yOff;

        for (int x = 0, xOff = 0; x < fNumXDivisions; ++x, xOff += xTileSize) {
            int xSize = (x < fNumXDivisions-1) ? xTileSize : viewport.width()-xOff;

            SkIRect clip = SkIRect::MakeXYWH(xOff, yOff, xSize, ySize);

            SkASSERT(viewport.contains(clip));

            static const uint32_t kMaxPad = 64;
            int32_t lPad = addRandomPaddingToDst ? rand.nextRangeU(0, kMaxPad) : 0;
            int32_t tPad = addRandomPaddingToDst ? rand.nextRangeU(0, kMaxPad) : 0;
            int32_t rPad = addRandomPaddingToDst ? rand.nextRangeU(0, kMaxPad) : 0;
            int32_t bPad = addRandomPaddingToDst ? rand.nextRangeU(0, kMaxPad) : 0;

            fTiles[y*fNumXDivisions+x].init(y*fNumXDivisions+x, direct, dstChar, clip,
                                           {lPad, tPad, rPad, bPad});
        }
    }
}

void DDLTileHelper::createSKP(sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                              SkData* compressedPictureData,
                              const DDLPromiseImageHelper& helper) {
    SkASSERT(!fReconstitutedPicture);

    fReconstitutedPicture = helper.reinflateSKP(std::move(threadSafeProxy), compressedPictureData,
                                                &fPromiseImages);
}

void DDLTileHelper::createDDLsInParallel() {
#if 1
    SkTaskGroup().batch(this->numTiles(), [&](int i) {
        fTiles[i].createDDL(fReconstitutedPicture.get());
    });
    SkTaskGroup().add([this]{ this->createComposeDDL(); });
    SkTaskGroup().wait();
#else
    // Use this code path to debug w/o threads
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL(fReconstitutedPicture.get());
    }
    this->createComposeDDL();
#endif
}

// On the gpu thread:
//    precompile any programs
//    replay the DDL into a surface to make the tile image
//    compose the tile image into the main canvas
static void do_gpu_stuff(GrDirectContext* direct, DDLTileHelper::TileData* tile) {

    // TODO: schedule program compilation as their own tasks
    tile->precompile(direct);

    tile->draw(direct);

    tile->dropDDL();
}

// We expect to have more than one recording thread but just one gpu thread
void DDLTileHelper::kickOffThreadedWork(SkTaskGroup* recordingTaskGroup,
                                        SkTaskGroup* gpuTaskGroup,
                                        GrDirectContext* dContext) {
    SkASSERT(recordingTaskGroup && gpuTaskGroup && dContext);

    for (int i = 0; i < this->numTiles(); ++i) {
        TileData* tile = &fTiles[i];
        if (!tile->initialized()) {
            continue;
        }

        // On a recording thread:
        //    generate the tile's DDL
        //    schedule gpu-thread processing of the DDL
        // Note: a finer grained approach would be add a scheduling task which would evaluate
        //       which DDLs were ready to be rendered based on their prerequisites
        recordingTaskGroup->add([this, tile, gpuTaskGroup, dContext]() {
                                    tile->createDDL(fReconstitutedPicture.get());

                                    gpuTaskGroup->add([dContext, tile]() {
                                        do_gpu_stuff(dContext, tile);
                                    });
                                });
    }

    recordingTaskGroup->add([this] { this->createComposeDDL(); });
}

// Only called from skpbench
void DDLTileHelper::interleaveDDLCreationAndDraw(GrDirectContext* direct) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL(fReconstitutedPicture.get());
        fTiles[i].draw(direct);
    }
}

// Only called from skpbench
void DDLTileHelper::drawAllTilesDirectly(GrDirectContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].drawSKPDirectly(context, fReconstitutedPicture.get());
    }
}

void DDLTileHelper::dropCallbackContexts() {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].dropCallbackContext();
    }
}

void DDLTileHelper::resetAllTiles() {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].reset();
    }
    fComposeDDL.reset();
}

void DDLTileHelper::createBackendTextures(SkTaskGroup* taskGroup, GrDirectContext* direct) {

    if (taskGroup) {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData* tile = &fTiles[i];
            if (!tile->initialized()) {
                continue;
            }

            taskGroup->add([direct, tile]() { TileData::CreateBackendTexture(direct, tile); });
        }
    } else {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData::CreateBackendTexture(direct, &fTiles[i]);
        }
    }
}

void DDLTileHelper::deleteBackendTextures(SkTaskGroup* taskGroup, GrDirectContext* direct) {
    if (taskGroup) {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData* tile = &fTiles[i];

            taskGroup->add([direct, tile]() { TileData::DeleteBackendTexture(direct, tile); });
        }
    } else {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData::DeleteBackendTexture(direct, &fTiles[i]);
        }
    }
}
