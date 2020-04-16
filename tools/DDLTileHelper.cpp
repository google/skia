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
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Gpu.h"
#include "tools/DDLPromiseImageHelper.h"

void DDLTileHelper::TileData::init(int id,
                                   sk_sp<SkSurface> dstSurface,
                                   const SkSurfaceCharacterization& dstSurfaceCharacterization,
                                   const SkIRect& clip) {
    fID = id;
    fDstSurface = dstSurface;
    fClip = clip;

    fCharacterization = dstSurfaceCharacterization.createResized(clip.width(), clip.height());
    SkASSERT(fCharacterization.isValid());
    SkASSERT(!fBackendTexture.isValid());
}

DDLTileHelper::TileData::~TileData() {}

void DDLTileHelper::TileData::createTileSpecificSKP(SkData* compressedPictureData,
                                                    const DDLPromiseImageHelper& helper) {
    SkASSERT(!fReconstitutedPicture);

    // This is bending the DDLRecorder contract! The promise images in the SKP should be
    // created by the same recorder used to create the matching DDL.
    SkDeferredDisplayListRecorder recorder(fCharacterization);

    fReconstitutedPicture = helper.reinflateSKP(&recorder, compressedPictureData, &fPromiseImages);

    std::unique_ptr<SkDeferredDisplayList> ddl = recorder.detach();
    if (ddl->priv().numRenderTasks()) {
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

void DDLTileHelper::TileData::precompile(GrContext* context) {
    SkASSERT(fDisplayList);

    SkDeferredDisplayList::ProgramIterator iter(context, fDisplayList.get());
    for (; !iter.done(); iter.next()) {
        iter.compile();
    }
}

sk_sp<SkSurface> DDLTileHelper::TileData::makeWrappedTileDest(GrContext* context) {
    if (!fBackendTexture.isValid()) {
        return nullptr;
    }

    return SkSurface::MakeFromBackendTexture(context,
                                             fBackendTexture,
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

void DDLTileHelper::TileData::draw(GrContext* context) {
    SkASSERT(fDisplayList && !fTileSurface);

    // The tile's surface needs to be held until after the DDL is flushed
    fTileSurface = this->makeWrappedTileDest(context);
    if (fTileSurface) {
        fTileSurface->draw(fDisplayList.get());

        // We can't snap an image here bc, since we're using wrapped backend textures for the
        // surfaces, that would incur a copy.
    }
}

// TODO: We should create a single DDL for the composition step and just add replaying it
// as the last GPU task
void DDLTileHelper::TileData::compose(GrContext* context) {
    SkASSERT(context->priv().asDirectContext());
    SkASSERT(fDstSurface);

    if (!fBackendTexture.isValid()) {
        return;
    }

    // Here we are, unfortunately, aliasing 'fBackendTexture'. It is backing both 'fTileSurface'
    // and 'tmp'.
    sk_sp<SkImage> tmp = SkImage::MakeFromTexture(context,
                                                  fBackendTexture,
                                                  fCharacterization.origin(),
                                                  fCharacterization.colorType(),
                                                  kPremul_SkAlphaType,
                                                  fCharacterization.refColorSpace());

    SkCanvas* canvas = fDstSurface->getCanvas();
    canvas->save();
    canvas->clipRect(SkRect::Make(fClip));
    canvas->drawImage(tmp, fClip.fLeft, fClip.fTop);
    canvas->restore();
}

void DDLTileHelper::TileData::reset() {
    // TODO: when DDLs are re-renderable we don't need to do this
    fDisplayList = nullptr;
    fTileSurface = nullptr;
}

void DDLTileHelper::TileData::CreateBackendTexture(GrContext* context, TileData* tile) {
    SkASSERT(context->priv().asDirectContext());
    SkASSERT(!tile->fBackendTexture.isValid());

    tile->fBackendTexture = context->createBackendTexture(tile->fCharacterization);
    // TODO: it seems that, on the Linux bots, backend texture creation is failing
    // a lot (skbug.com/10142)
    //SkASSERT(tile->fBackendTexture.isValid());
}

void DDLTileHelper::TileData::DeleteBackendTexture(GrContext* context, TileData* tile) {
    SkASSERT(context->priv().asDirectContext());
    // TODO: it seems that, on the Linux bots, backend texture creation is failing
    // a lot (skbug.com/10142)
    //SkASSERT(tile->fBackendTexture.isValid());

    tile->fTileSurface = nullptr;
    context->deleteBackendTexture(tile->fBackendTexture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLTileHelper::DDLTileHelper(sk_sp<SkSurface> dstSurface,
                             const SkSurfaceCharacterization& dstChar,
                             const SkIRect& viewport,
                             int numDivisions)
        : fNumDivisions(numDivisions) {
    SkASSERT(fNumDivisions > 0);
    fTiles = new TileData[this->numTiles()];

    int xTileSize = viewport.width()/fNumDivisions;
    int yTileSize = viewport.height()/fNumDivisions;

    // Create the destination tiles
    for (int y = 0, yOff = 0; y < fNumDivisions; ++y, yOff += yTileSize) {
        int ySize = (y < fNumDivisions-1) ? yTileSize : viewport.height()-yOff;

        for (int x = 0, xOff = 0; x < fNumDivisions; ++x, xOff += xTileSize) {
            int xSize = (x < fNumDivisions-1) ? xTileSize : viewport.width()-xOff;

            SkIRect clip = SkIRect::MakeXYWH(xOff, yOff, xSize, ySize);

            SkASSERT(viewport.contains(clip));

            fTiles[y*fNumDivisions+x].init(y*fNumDivisions+x, dstSurface, dstChar, clip);
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
    SkTaskGroup().wait();
#else
    // Use this code path to debug w/o threads
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL();
    }
#endif
}

// On the gpu thread:
//    precompile any programs
//    replay the DDL into a surface to make the tile image
//    compose the tile image into the main canvas
static void do_gpu_stuff(GrContext* context, DDLTileHelper::TileData* tile) {

    // TODO: schedule program compilation as their own tasks
    tile->precompile(context);

    tile->draw(context);

    // TODO: we should actually have a separate DDL that does
    // the final composition draw
    tile->compose(context);
}

// We expect to have more than one recording thread but just one gpu thread
void DDLTileHelper::kickOffThreadedWork(SkTaskGroup* recordingTaskGroup,
                                        SkTaskGroup* gpuTaskGroup,
                                        GrContext* gpuThreadContext) {
    SkASSERT(recordingTaskGroup && gpuTaskGroup && gpuThreadContext);

    for (int i = 0; i < this->numTiles(); ++i) {
        TileData* tile = &fTiles[i];

        // On a recording thread:
        //    generate the tile's DDL
        //    schedule gpu-thread processing of the DDL
        // Note: a finer grained approach would be add a scheduling task which would evaluate
        //       which DDLs were ready to be rendered based on their prerequisites
        recordingTaskGroup->add([tile, gpuTaskGroup, gpuThreadContext]() {
                                    tile->createDDL();

                                    gpuTaskGroup->add([gpuThreadContext, tile]() {
                                        do_gpu_stuff(gpuThreadContext, tile);
                                    });
                                });
    }
}

void DDLTileHelper::precompileAndDrawAllTiles(GrContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].precompile(context);
        fTiles[i].draw(context);
    }
}

void DDLTileHelper::interleaveDDLCreationAndDraw(GrContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].createDDL();
        fTiles[i].draw(context);
    }
}

void DDLTileHelper::drawAllTilesDirectly(GrContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].drawSKPDirectly(context);
    }
}

void DDLTileHelper::composeAllTiles(GrContext* context) {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].compose(context);
    }
}

void DDLTileHelper::resetAllTiles() {
    for (int i = 0; i < this->numTiles(); ++i) {
        fTiles[i].reset();
    }
}

void DDLTileHelper::createBackendTextures(SkTaskGroup* taskGroup, GrContext* context) {
    SkASSERT(context->priv().asDirectContext());

    if (taskGroup) {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData* tile = &fTiles[i];

            taskGroup->add([context, tile]() { TileData::CreateBackendTexture(context, tile); });
        }
    } else {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData::CreateBackendTexture(context, &fTiles[i]);
        }
    }
}

void DDLTileHelper::deleteBackendTextures(SkTaskGroup* taskGroup, GrContext* context) {
    SkASSERT(context->priv().asDirectContext());

    if (taskGroup) {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData* tile = &fTiles[i];

            taskGroup->add([context, tile]() { TileData::DeleteBackendTexture(context, tile); });
        }
    } else {
        for (int i = 0; i < this->numTiles(); ++i) {
            TileData::DeleteBackendTexture(context, &fTiles[i]);
        }
    }
}
