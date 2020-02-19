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
    // The TileData class encapsulates the information and behavior of a single tile when
    // rendering with DDLs.
    class TileData {
    public:
        TileData() {}
        ~TileData();

        void init(int id, sk_sp<SkSurface> dstSurface, const SkIRect& clip);

        // Convert the compressedPictureData into an SkPicture replacing each image-index
        // with a promise image.
        void createTileSpecificSKP(SkData* compressedPictureData,
                                   const DDLPromiseImageHelper& helper);

        // Create the DDL for this tile (i.e., fill in 'fDisplayList').
        void createDDL();

        // Replay the recorded DDL into the tile surface - creating 'fImage'.
        void draw(GrContext*);

        // Draw the result of replaying the DDL (i.e., 'fImage') into the
        // final destination surface ('fDstSurface').
        void compose();

        void reset();

        int id() const { return fID; }

        SkDeferredDisplayList* ddl() { return fDisplayList.get(); }

    private:
        int                       fID = -1;
        sk_sp<SkSurface>          fDstSurface;       // the ultimate target for composition
        SkSurfaceCharacterization fCharacterization; // characterization for the tile's surface
        SkIRect                   fClip;             // in the device space of the 'fDstSurface'

        sk_sp<SkImage>            fImage;            // the result of replaying the DDL
        sk_sp<SkPicture>          fReconstitutedPicture;
        SkTArray<sk_sp<SkImage>>  fPromiseImages;    // All the promise images in the
                                                     // reconstituted picture
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
    };

    DDLTileHelper(sk_sp<SkSurface> dstSurface,
                  const SkIRect& viewport,
                  int numDivisions);
    ~DDLTileHelper() {
        delete[] fTiles;
    }

    void createSKPPerTile(SkData* compressedPictureData, const DDLPromiseImageHelper& helper);

    void kickOffThreadedWork(SkTaskGroup* recordingTaskGroup,
                             SkTaskGroup* gpuTaskGroup,
                             GrContext* gpuThreadContext);

    void createDDLsInParallel();

    void drawAllTilesAndFlush(GrContext*, bool flush);

    void composeAllTiles();

    void resetAllTiles();

    int numTiles() const { return fNumDivisions * fNumDivisions; }

private:
    int                fNumDivisions; // number of tiles along a side
    TileData*          fTiles;        // 'fNumDivisions' x 'fNumDivisions'
};

#endif
