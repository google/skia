/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DDLTileHelper_DEFINED
#define DDLTileHelper_DEFINED

#include "SkDeferredDisplayListRecorder.h"
#include "SkRect.h"

#if SK_SUPPORT_GPU

class PromiseImageHelper;
class SkDeferredDisplayList;
class SkPicture;
class SkSurfaceCharacterization;

class DDLTileHelper {
public:
    // TileData class encapsulates the information and behavior for a single tile/thread in
    // a DDL rendering.
    class TileData {
    public:
        TileData(sk_sp<SkSurface>, const SkSurfaceCharacterization&, const SkIRect& clip);

        // This method can be invoked in parallel
        // In each thread we will reconvert the compressedPictureData into an SkPicture
        // replacing each image-index with a promise image.
        void createTileSpecificSKP(SkData* compressedPictureData, const PromiseImageHelper& helper);

        // This method can be invoked in parallel
        // Create the per-tile DDL from the per-tile SKP
        void createDDL();

        // This method operates serially and replays the recorded DDL into the tile surface.
        void draw();

        // This method also operates serially and composes the results of replaying the DDL into
        // the final destination surface.
        void compose(SkCanvas* dst);

#ifdef SK_DEBUG
        void print() const;
#endif

    private:
        sk_sp<SkSurface>                       fSurface;
        SkDeferredDisplayListRecorder          fRecorder;
        SkIRect                                fClip;    // in the device space of the dest canvas
        sk_sp<SkPicture>                       fReconstitutedPicture;
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
    };

    DDLTileHelper(SkCanvas* canvas, const SkIRect& viewport, int numDivisions);

    void createSKPPerTile(SkData* compressedPictureData, const PromiseImageHelper& helper);

    void createDDLsInParallel();

    void drawAllTilesAndFlush(GrContext*);

    void composeAllTiles(SkCanvas* dstCanvas);

private:
    int fNumDivisions; // number of tiles along a side
    SkTArray<TileData> fTiles;
};

#endif
#endif
