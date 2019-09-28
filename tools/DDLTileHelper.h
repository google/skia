/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DDLTileHelper_DEFINED
#define DDLTileHelper_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"

class DDLPromiseImageHelper;
class SkCanvas;
class SkData;
class SkDeferredDisplayList;
class SkPicture;
class SkSurface;
class SkSurfaceCharacterization;

class DDLTileHelper {
public:
    // TileData class encapsulates the information and behavior for a single tile/thread in
    // a DDL rendering.
    class TileData {
    public:
        TileData(sk_sp<SkSurface>, const SkIRect& clip);

        // This method can be invoked in parallel
        // In each thread we will reconvert the compressedPictureData into an SkPicture
        // replacing each image-index with a promise image.
        void createTileSpecificSKP(SkData* compressedPictureData,
                                   const DDLPromiseImageHelper& helper);

        // This method can be invoked in parallel
        // Create the per-tile DDL from the per-tile SKP
        void createDDL();

        // This method operates serially and replays the recorded DDL into the tile surface.
        void draw();

        // This method also operates serially and composes the results of replaying the DDL into
        // the final destination surface.
        void compose(SkCanvas* dst);

        void reset();

    private:
        sk_sp<SkSurface>                       fSurface;
        SkSurfaceCharacterization              fCharacterization;
        SkIRect                                fClip;    // in the device space of the dest canvas
        sk_sp<SkPicture>                       fReconstitutedPicture;
        SkTArray<sk_sp<SkImage>>               fPromiseImages; // All the promise images in the
                                                               // reconstituted picture
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
    };

    DDLTileHelper(SkCanvas* canvas, const SkIRect& viewport, int numDivisions);

    void createSKPPerTile(SkData* compressedPictureData, const DDLPromiseImageHelper& helper);

    void createDDLsInParallel();

    void drawAllTilesAndFlush(GrContext*, bool flush);

    void composeAllTiles(SkCanvas* dstCanvas);

    void resetAllTiles();

private:
    int                fNumDivisions; // number of tiles along a side
    SkTArray<TileData> fTiles;
};

#endif
