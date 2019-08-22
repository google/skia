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
#include "src/image/SkImage_Gpu.h"
#include "tools/DDLPromiseImageHelper.h"

DDLTileHelper::TileData::TileData(sk_sp<SkSurface> s, const SkIRect& clip)
        : fSurface(std::move(s))
        , fClip(clip) {
    SkAssertResult(fSurface->characterize(&fCharacterization));
}

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
        // reinflation of the SKPs contains opLists that means some image subset operation
        // created a draw.
        fReconstitutedPicture.reset();
    }
}

void DDLTileHelper::TileData::createDDL() {
    SkASSERT(!fDisplayList);

    SkDeferredDisplayListRecorder recorder(fCharacterization);

    // DDL TODO: the DDLRecorder's GrContext isn't initialized until getCanvas is called.
    // Maybe set it up in the ctor?
    SkCanvas* subCanvas = recorder.getCanvas();

    // Because we cheated in createTileSpecificSKP and used the wrong DDLRecorder, the GrContext's
    // stored in fReconstitutedPicture's promise images are incorrect. Patch them with the correct
    // one now.
    for (int i = 0; i < fPromiseImages.count(); ++i) {
        GrContext* newContext = subCanvas->getGrContext();

        if (fPromiseImages[i]->isTextureBacked()) {
            SkImage_GpuBase* gpuImage = (SkImage_GpuBase*) fPromiseImages[i].get();
            gpuImage->resetContext(sk_ref_sp(newContext));
        }
    }

    subCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
    subCanvas->translate(-fClip.fLeft, -fClip.fTop);

    // Note: in this use case we only render a picture to the deferred canvas
    // but, more generally, clients will use arbitrary draw calls.
    if (fReconstitutedPicture) {
        subCanvas->drawPicture(fReconstitutedPicture);
    }

    fDisplayList = recorder.detach();
}

void DDLTileHelper::TileData::draw() {
    SkASSERT(fDisplayList);

    fSurface->draw(fDisplayList.get());
}

void DDLTileHelper::TileData::compose(SkCanvas* dst) {
    sk_sp<SkImage> img = fSurface->makeImageSnapshot();
    dst->save();
    dst->clipRect(SkRect::Make(fClip));
    dst->drawImage(std::move(img), fClip.fLeft, fClip.fTop);
    dst->restore();
}

void DDLTileHelper::TileData::reset() {
    // TODO: when DDLs are re-renderable we don't need to do this
    fDisplayList = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DDLTileHelper::DDLTileHelper(SkCanvas* canvas, const SkIRect& viewport, int numDivisions)
        : fNumDivisions(numDivisions) {
    SkASSERT(fNumDivisions > 0);
    fTiles.reserve(fNumDivisions*fNumDivisions);

    int xTileSize = viewport.width()/fNumDivisions;
    int yTileSize = viewport.height()/fNumDivisions;

    // Create the destination tiles
    for (int y = 0, yOff = 0; y < fNumDivisions; ++y, yOff += yTileSize) {
        int ySize = (y < fNumDivisions-1) ? yTileSize : viewport.height()-yOff;

        for (int x = 0, xOff = 0; x < fNumDivisions; ++x, xOff += xTileSize) {
            int xSize = (x < fNumDivisions-1) ? xTileSize : viewport.width()-xOff;

            SkIRect clip = SkIRect::MakeXYWH(xOff, yOff, xSize, ySize);

            SkASSERT(viewport.contains(clip));

            SkImageInfo tileII = SkImageInfo::MakeN32Premul(xSize, ySize);

            sk_sp<SkSurface> tileSurface = canvas->makeSurface(tileII);

            // TODO: this is here to deal w/ a resource allocator bug (skbug.com/8007). If all
            // the DDLs are flushed at the same time (w/o the composition draws) the allocator
            // feels free to reuse the backing GrSurfaces!
            tileSurface->flush();

            fTiles.push_back(TileData(std::move(tileSurface), clip));
        }
    }
}

void DDLTileHelper::createSKPPerTile(SkData* compressedPictureData,
                                     const DDLPromiseImageHelper& helper) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].createTileSpecificSKP(compressedPictureData, helper);
    }
}

void DDLTileHelper::createDDLsInParallel() {
#if 1
    SkTaskGroup().batch(fTiles.count(), [&](int i) { fTiles[i].createDDL(); });
    SkTaskGroup().wait();
#else
    // Use this code path to debug w/o threads
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].createDDL();
    }
#endif

}

void DDLTileHelper::drawAllTilesAndFlush(GrContext* context, bool flush) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].draw();
    }
    if (flush) {
        context->flush();
    }
}

void DDLTileHelper::composeAllTiles(SkCanvas* dstCanvas) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].compose(dstCanvas);
    }
}

void DDLTileHelper::resetAllTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].reset();
    }
}
