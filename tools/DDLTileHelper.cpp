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
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Gpu.h"
#include "tools/DDLPromiseImageHelper.h"

void DDLTileHelper::TileData::init(int id,
                                   GrDirectContext* direct,
                                   const SkSurfaceCharacterization& dstSurfaceCharacterization,
                                   const SkIRect& clip) {
    fID = id;
    fClip = clip;

    fCharacterization = dstSurfaceCharacterization.createResized(clip.width(), clip.height());
    SkASSERT(fCharacterization.isValid());

    GrBackendFormat backendFormat = direct->defaultBackendFormat(fCharacterization.colorType(),
                                                                 GrRenderable::kYes);
    SkDEBUGCODE(const GrCaps* caps = direct->priv().caps());
    SkASSERT(caps->isFormatTexturable(backendFormat));

    fCallbackContext.reset(new PromiseImageCallbackContext(direct, backendFormat));
}

DDLTileHelper::TileData::~TileData() {}

void DDLTileHelper::TileData::createTileSpecificSKP(SkData* compressedPictureData,
                                                    const DDLPromiseImageHelper& helper) {
    SkASSERT(!fReconstitutedPicture);

    // This is bending the DDLRecorder contract! The promise images in the SKP should be
    // created by the same recorder used to create the matching DDL.
    SkDeferredDisplayListRecorder recorder(fCharacterization);

    fReconstitutedPicture = helper.reinflateSKP(&recorder, compressedPictureData, &fPromiseImages);

    auto ddl = recorder.detach();
    if (ddl && ddl->priv().numRenderTasks()) {
        // TODO: remove this once skbug.com/8424 is fixed. If the DDL resulting from the
        // reinflation of the SKPs contains opsTasks that means some image subset operation
        // created a draw.
        fReconstitutedPicture.reset();
    }
}

void DDLTileHelper::TileData::createDDL() {
    SkASSERT(!fDisplayList && fReconstitutedPicture);

    SkDeferredDisplayListRecorder recorder(fCharacterization);

    // DDL TODO: the DDLRecorder's GrContext isn't initialized until getCanvas is called.
    // Maybe set it up in the ctor?
    SkCanvas* recordingCanvas = recorder.getCanvas();

    // Because we cheated in createTileSpecificSKP and used the wrong DDLRecorder, the GrContext's
    // stored in fReconstitutedPicture's promise images are incorrect. Patch them with the correct
    // one now.
    for (int i = 0; i < fPromiseImages.count(); ++i) {
        // CONTEXT TODO: this can be converted to a recording context once images no longer
        // hold GrContexts
        GrContext* newContext = recordingCanvas->getGrContext();

        if (fPromiseImages[i]->isTextureBacked()) {
            SkImage_GpuBase* gpuImage = (SkImage_GpuBase*) fPromiseImages[i].get();
            gpuImage->resetContext(sk_ref_sp(newContext));
        }
    }

    recordingCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
    recordingCanvas->translate(-fClip.fLeft, -fClip.fTop);

    // Note: in this use case we only render a picture to the deferred canvas
    // but, more generally, clients will use arbitrary draw calls.
    recordingCanvas->drawPicture(fReconstitutedPicture);

    fDisplayList = recorder.detach();
}

void DDLTileHelper::createComposeDDL() {
    SkASSERT(!fComposeDDL);

    SkDeferredDisplayListRecorder recorder(fDstCharacterization);

    SkCanvas* recordingCanvas = recorder.getCanvas();

    for (int i = 0; i < this->numTiles(); ++i) {
        TileData* tile = &fTiles[i];

        sk_sp<SkImage> promiseImage = tile->makePromiseImage(&recorder);

        SkIRect clipRect = tile->clipRect();

        SkASSERT(clipRect.width() == promiseImage->width() &&
                 clipRect.height() == promiseImage->height());

        recordingCanvas->drawImage(promiseImage, clipRect.fLeft, clipRect.fTop);
    }

    fComposeDDL = recorder.detach();
    SkASSERT(fComposeDDL);
}

void DDLTileHelper::TileData::precompile(GrDirectContext* direct) {
    SkASSERT(fDisplayList);

    SkDeferredDisplayList::ProgramIterator iter(direct, fDisplayList.get());
    for (; !iter.done(); iter.next()) {
        iter.compile();
    }
}

sk_sp<SkSurface> DDLTileHelper::TileData::makeWrappedTileDest(GrContext* context) {
    SkASSERT(fCallbackContext && fCallbackContext->promiseImageTexture());

    auto promiseImageTexture = fCallbackContext->promiseImageTexture();
    if (!promiseImageTexture->backendTexture().isValid()) {
        return nullptr;
    }

    // Here we are, unfortunately, aliasing the backend texture held by the SkPromiseImageTexture.
    // Both the tile's destination surface and the promise image used to draw the tile will be
    // backed by the same backendTexture - unbeknownst to Ganesh.
    return SkSurface::MakeFromBackendTexture(context,
                                             promiseImageTexture->backendTexture(),
                                             fCharacterization.origin(),
                                             fCharacterization.sampleCount(),
                                             fCharacterization.colorType(),
                                             fCharacterization.refColorSpace(),
                                             &fCharacterization.surfaceProps());
}

void DDLTileHelper::TileData::drawSKPDirectly(GrContext* context) {
    SkASSERT(!fDisplayList && !fTileSurface && fReconstitutedPicture);

    fTileSurface = this->makeWrappedTileDest(context);
    if (fTileSurface) {
        SkCanvas* tileCanvas = fTileSurface->getCanvas();

        tileCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
        tileCanvas->translate(-fClip.fLeft, -fClip.fTop);

        tileCanvas->drawPicture(fReconstitutedPicture);

        // We can't snap an image here bc, since we're using wrapped backend textures for the
        // surfaces, that would incur a copy.
    }
}

void DDLTileHelper::TileData::draw(GrDirectContext* direct) {
    SkASSERT(fDisplayList && !fTileSurface);

    // The tile's surface needs to be held until after the DDL is flushed bc the DDL doesn't take
    // a ref on its destination proxy.
    // TODO: make the DDL (or probably the drawing manager) take a ref on the destination proxy
    // (maybe in GrDrawingManager::addDDLTarget).
    fTileSurface = this->makeWrappedTileDest(direct);
    if (fTileSurface) {
        fTileSurface->draw(fDisplayList);

        // We can't snap an image here bc, since we're using wrapped backend textures for the
        // surfaces, that would incur a copy.
    }
}

void DDLTileHelper::TileData::reset() {
    // TODO: when DDLs are re-renderable we don't need to do this
    fDisplayList = nullptr;

    fTileSurface = nullptr;
}

sk_sp<SkImage> DDLTileHelper::TileData::makePromiseImage(SkDeferredDisplayListRecorder* recorder) {
    SkASSERT(fCallbackContext);

    // The promise image gets a ref on the promise callback context
    sk_sp<SkImage> promiseImage = recorder->makePromiseTexture(
                                    fCallbackContext->backendFormat(),
                                    fClip.width(),
                                    fClip.height(),
                                    GrMipmapped::kNo,
                                    GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                    fCharacterization.colorType(),
                                    kPremul_SkAlphaType,
                                    fCharacterization.refColorSpace(),
                                    PromiseImageCallbackContext::PromiseImageFulfillProc,
                                    PromiseImageCallbackContext::PromiseImageReleaseProc,
                                    PromiseImageCallbackContext::PromiseImageDoneProc,
                                    (void*)this->refCallbackContext().release(),
                                    SkDeferredDisplayListRecorder::PromiseImageApiVersion::kNew);
    fCallbackContext->wasAddedToImage();

    return promiseImage;
}

void DDLTileHelper::TileData::CreateBackendTexture(GrDirectContext* direct, TileData* tile) {
    SkASSERT(tile->fCallbackContext && !tile->fCallbackContext->promiseImageTexture());

    GrBackendTexture beTex = direct->createBackendTexture(tile->fCharacterization);
    tile->fCallbackContext->setBackendTexture(beTex);
}

void DDLTileHelper::TileData::DeleteBackendTexture(GrDirectContext*, TileData* tile) {
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
                             int numDivisions)
        : fNumDivisions(numDivisions)
        , fTiles(numDivisions * numDivisions)
        , fDstCharacterization(dstChar) {
    SkASSERT(fNumDivisions > 0);

    int xTileSize = viewport.width()/fNumDivisions;
    int yTileSize = viewport.height()/fNumDivisions;

    // Create the destination tiles
    for (int y = 0, yOff = 0; y < fNumDivisions; ++y, yOff += yTileSize) {
        int ySize = (y < fNumDivisions-1) ? yTileSize : viewport.height()-yOff;

        for (int x = 0, xOff = 0; x < fNumDivisions; ++x, xOff += xTileSize) {
            int xSize = (x < fNumDivisions-1) ? xTileSize : viewport.width()-xOff;

            SkIRect clip = SkIRect::MakeXYWH(xOff, yOff, xSize, ySize);

            SkASSERT(viewport.contains(clip));

            fTiles[y*fNumDivisions+x].init(y*fNumDivisions+x, direct, dstChar, clip);
        }
    }
}

void DDLTileHelper::createSKPPerTile(SkData* compressedPictureData,
                                     const DDLPromiseImageHelper& helper) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createTileSpecificSKP(compressedPictureData, helper);
    }
}

void DDLTileHelper::createDDLsInParallel() {
#if 1
    SkTaskGroup().batch(this->numTiles(), [&](int i) { fTiles[i].createDDL(); });
    SkTaskGroup().add([this]{ this->createComposeDDL(); });
    SkTaskGroup().wait();
#else
    // Use this code path to debug w/o threads
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL();
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
                                        GrDirectContext* direct) {
    SkASSERT(recordingTaskGroup && gpuTaskGroup && direct);

    for (int i = 0; i < this->numTiles(); ++i) {
        TileData* tile = &fTiles[i];

        // On a recording thread:
        //    generate the tile's DDL
        //    schedule gpu-thread processing of the DDL
        // Note: a finer grained approach would be add a scheduling task which would evaluate
        //       which DDLs were ready to be rendered based on their prerequisites
        recordingTaskGroup->add([tile, gpuTaskGroup, direct]() {
                                    tile->createDDL();

                                    gpuTaskGroup->add([direct, tile]() {
                                        do_gpu_stuff(direct, tile);
                                    });
                                });
    }

    recordingTaskGroup->add([this] { this->createComposeDDL(); });
}

// Only called from ViaDDL
void DDLTileHelper::precompileAndDrawAllTiles(GrDirectContext* direct) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].precompile(direct);
        fTiles[i].draw(direct);
    }
}

// Only called from skpbench
void DDLTileHelper::interleaveDDLCreationAndDraw(GrDirectContext* direct) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL();
        fTiles[i].draw(direct);
    }
}

// Only called from skpbench
void DDLTileHelper::drawAllTilesDirectly(GrContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].drawSKPDirectly(context);
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
