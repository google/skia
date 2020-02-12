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
        //TileData(sk_sp<SkSurface> dstSurface, const SkIRect& clip);
        TileData() {}
        ~TileData();

        void init(int id, sk_sp<SkSurface> dstSurface, const SkIRect& clip);

        // This method can be invoked in parallel
        // In each thread we will reconvert the compressedPictureData into an SkPicture
        // replacing each image-index with a promise image.
        void createTileSpecificSKP(SkData* compressedPictureData,
                                   const DDLPromiseImageHelper& helper);

        // This method can be invoked in parallel
        // Create the per-tile DDL from the per-tile SKP.
        void createDDL();

        // This method operates serially and replays the recorded DDL into the tile surface.
        void draw1(GrContext*);

        // This method also operates serially and composes the results of replaying the DDL into
        // the final destination surface.
        void compose();

        void reset();

        int id() const { return fID; }
        SkDeferredDisplayList* ddl() { return fDisplayList.get(); }

    private:
        int                                    fID = -1;
        sk_sp<SkSurface>                       fDstSurface;       // the ultimate target for composition
        SkSurfaceCharacterization              fCharacterization; // characterization for the tile's surface
        SkIRect                                fClip;             // in the device space of the 'fDstSurface'

        sk_sp<SkImage>                         fImage;            // the result of rendering the tile
        sk_sp<SkPicture>                       fReconstitutedPicture;
        SkTArray<sk_sp<SkImage>>               fPromiseImages;    // All the promise images in the
                                                                  // reconstituted picture
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
    };

    DDLTileHelper(sk_sp<SkSurface> dstSurface,
                  const SkIRect& viewport,
                  int numDivisions);

    void createSKPPerTile(SkData* compressedPictureData, const DDLPromiseImageHelper& helper);

    void createDDLsInParallel(SkTaskGroup* recordingTaskGroup = nullptr,
                              SkTaskGroup* gpuTaskGroup = nullptr,
                              GrContext* gpuThreadContext = nullptr);

    void drawAllTilesAndFlush(GrContext*, bool flush);

    void composeAllTiles();

    void resetAllTiles();

    int numTiles() const { return fNumDivisions * fNumDivisions; }

private:
    int                fNumDivisions; // number of tiles along a side
    TileData*          fTiles1;        // 'fNumDivisions' x 'fNumDivisions'
};

#endif
