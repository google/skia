/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DDLTileHelper.h"

#include "PromiseImageHelper.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"
#include "SkTaskGroup.h"

DDLTileHelper::TileData::TileData(sk_sp<SkSurface> s,
                                  const SkSurfaceCharacterization& c,
                                  const SkIRect& clip)
        : fSurface(std::move(s))
        , fRecorder(c)
        , fClip(clip) {
}

void DDLTileHelper::TileData::createTileSpecificSKP(SkData* compressedPictureData,
                                                    const PromiseImageHelper& helper) {
    SkASSERT(!fReconstitutedPicture);

    fReconstitutedPicture = helper.reinflateSKP(&fRecorder, compressedPictureData);
}

void DDLTileHelper::TileData::createDDL() {
    SkASSERT(fReconstitutedPicture);
    SkASSERT(!fDisplayList);

    // DDL TODO: the DDLRecorder's GrContext isn't initialized until getCanvas is called.
    // Maybe set it up in the ctor?
    SkCanvas* subCanvas = fRecorder.getCanvas();

    subCanvas->clipRect(SkRect::MakeWH(fClip.width(), fClip.height()));
    subCanvas->translate(-fClip.fLeft, -fClip.fTop);

    // Note: in this use case we only render a picture to the deferred canvas
    // but, more generally, clients will use arbitrary draw calls.
    subCanvas->drawPicture(fReconstitutedPicture);

    fDisplayList = fRecorder.detach();
}

void DDLTileHelper::TileData::draw() {
    SkASSERT(fDisplayList);

    fSurface->draw(fDisplayList.get());
}

void DDLTileHelper::TileData::compose(SkCanvas* dst) {

    this->print();

    sk_sp<SkImage> img = fSurface->makeImageSnapshot();
    dst->save();
    dst->clipRect(SkRect::Make(fClip));
    dst->drawImage(std::move(img), fClip.fLeft, fClip.fTop);
    dst->restore();
}

#ifdef SK_DEBUG
void DDLTileHelper::TileData::print() const {
    SkDebugf("tile: %dx%d [ %d %d %d %d]\n",
            fSurface->width(), fSurface->height(),
            fClip.fLeft, fClip.fTop, fClip.fRight, fClip.fBottom);
}
#endif

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

            SkSurfaceCharacterization tileCharacterization;
            SkAssertResult(tileSurface->characterize(&tileCharacterization));

            fTiles.push_back(TileData(std::move(tileSurface), tileCharacterization, clip));

            fTiles.back().print();
        }
    }
}

void DDLTileHelper::createSKPPerTile(SkData* compressedPictureData,
                                     const PromiseImageHelper& helper) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].createTileSpecificSKP(compressedPictureData, helper);
    }
}

void DDLTileHelper::createDDLsInParallel() {
    SkTaskGroup().batch(fTiles.count(), [&](int i) {
        fTiles[i].createDDL();
    });
}

void DDLTileHelper::drawAllTilesAndFlush(GrContext* context) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].draw();
    }
    context->flush();
}

void DDLTileHelper::composeAllTiles(SkCanvas* dstCanvas) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].compose(dstCanvas);
    }
}

