/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DDLTileHelper.h"

#include "DDLPromiseImageHelper.h"
#include "SkCanvas.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkImage_Gpu.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"
#include "SkTaskGroup.h"

DDLTileHelper::TileData::TileData(sk_sp<SkSurface> s, const SkIRect& clip)
        : fSurface(std::move(s))
        , fClip(clip) {
    SkAssertResult(fSurface->characterize(&fCharacterization));
}

void DDLTileHelper::TileData::createTileSpecificSKP(SkData* compressedPictureData,
                                                    const DDLPromiseImageHelper& helper) {
    SkASSERT(!fReconstitutedPicture && !fDDLRecorder);

    // We need the tile's DDL recorder at this point so the created promise images will
    // point at the correct GrContext
    fDDLRecorder.reset(new SkDeferredDisplayListRecorder(fCharacterization));

    fReconstitutedPicture = helper.reinflateSKP(fDDLRecorder.get(),
                                                compressedPictureData,
                                                &fPromiseImages);
}

void DDLTileHelper::TileData::createDDL() {
    SkASSERT(fReconstitutedPicture);
    SkASSERT(!fDisplayList);

    SkCanvas* subCanvas = fDDLRecorder->getCanvas();

    subCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
    subCanvas->translate(-fClip.fLeft, -fClip.fTop);

    // Note: in this use case we only render a picture to the deferred canvas
    // but, more generally, clients will use arbitrary draw calls.
    subCanvas->drawPicture(fReconstitutedPicture);

    fDisplayList = fDDLRecorder->detach();
    fDDLRecorder.reset();
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
