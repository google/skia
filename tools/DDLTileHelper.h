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
class PromiseImageCallbackContext;
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

        void init(int id,
                  GrContext* context,
                  const SkSurfaceCharacterization& dstChar,
                  const SkIRect& clip);

        // Convert the compressedPictureData into an SkPicture replacing each image-index
        // with a promise image.
        void createTileSpecificSKP(SkData* compressedPictureData,
                                   const DDLPromiseImageHelper& helper);

        // Create the DDL for this tile (i.e., fill in 'fDisplayList').
        void createDDL();

        // Precompile all the programs required to draw this tile's DDL
        void precompile(GrContext*);

        // Just draw the re-inflated per-tile SKP directly into this tile w/o going through a DDL
        // first. This is used for determining the overhead of using DDLs (i.e., it replaces
        // a 'createDDL' and 'draw' pair.
        void drawSKPDirectly(GrContext*);

        // Replay the recorded DDL into the tile surface - creating 'fImage'.
        void draw(GrContext*);

#if 0
        // Draw the result of replaying the DDL (i.e., 'fImage') into the
        // final destination surface ('fDstSurface').
        void compose(GrContext*);
#endif

        void reset();

        int id() const { return fID; }
        SkIRect clipRect() const { return fClip; }

        SkDeferredDisplayList* ddl() { return fDisplayList.get(); }

        sk_sp<SkImage> makePromiseImage(SkDeferredDisplayListRecorder*);
        void dropCallbackContext() { fCallbackContext1.reset(); }

        static void CreateBackendTexture(GrContext*, TileData*);
        static void DeleteBackendTexture(GrContext*, TileData*);

    private:
        sk_sp<SkSurface> makeWrappedTileDest(GrContext* context);

        sk_sp<PromiseImageCallbackContext> refCallbackContext() { return fCallbackContext1; }

        int                       fID = -1;
        sk_sp<SkSurface>          fDstSurface1;       // the ultimate target for composition

        sk_sp<PromiseImageCallbackContext> fCallbackContext1;
        GrBackendTexture          fBackendTexture;   // destination for this tile's content
        SkSurfaceCharacterization fCharacterization; // characterization for the tile's surface
        SkIRect                   fClip;             // in the device space of the 'fDstSurface'

        // 'fTileSurface' wraps 'fBackendTexture' and must exist until after 'fDisplayList'
        // has been flushed.
        sk_sp<SkSurface>          fTileSurface;
        sk_sp<SkPicture>          fReconstitutedPicture;
        SkTArray<sk_sp<SkImage>>  fPromiseImages;    // All the promise images in the
                                                     // reconstituted picture
        std::unique_ptr<SkDeferredDisplayList> fDisplayList;
    };

    DDLTileHelper(GrContext* context,
                  const SkSurfaceCharacterization& dstChar,
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

    // Create the DDL that will compose all the tile images into a final result.
    void createComposeDDL();
    SkDeferredDisplayList* composeDDL() const { return fComposeDDL.get(); }

    void precompileAndDrawAllTiles(GrContext*);

    // For each tile, create its DDL and then draw it - all on a single thread. This is to allow
    // comparison w/ just drawing the SKP directly (i.e., drawAllTilesDirectly). The
    // DDL creations and draws are interleaved to prevent starvation of the GPU.
    // Note: this is somewhat of a misuse/pessimistic-use of DDLs since they are supposed to
    // be created on a separate thread.
    void interleaveDDLCreationAndDraw(GrContext*);

    // This draws all the per-tile SKPs directly into all of the tiles w/o converting them to
    // DDLs first - all on a single thread.
    void drawAllTilesDirectly(GrContext*);

//    void composeAllTiles(GrContext*);

    void dropCallbackContexts();
    void resetAllTiles();

    int numTiles() const { return fNumDivisions * fNumDivisions; }

    void createBackendTextures(SkTaskGroup*, GrContext*);
    void deleteBackendTextures(SkTaskGroup*, GrContext*);

private:
    int                fNumDivisions; // number of tiles along a side
    TileData*          fTiles;        // 'fNumDivisions' x 'fNumDivisions'

    std::unique_ptr<SkDeferredDisplayList> fComposeDDL;

    const SkSurfaceCharacterization        fDstCharacterization;
};

#endif
