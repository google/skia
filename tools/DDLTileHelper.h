/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DDLTileHelper_DEFINED
#define DDLTileHelper_DEFINED

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "src/core/SkSpan.h"

class DDLPromiseImageHelper;
class PromiseImageCallbackContext;
class SkCanvas;
class SkData;
class SkDeferredDisplayListRecorder;
class SkPicture;
class SkSurface;
class SkSurfaceCharacterization;
class SkTaskGroup;

class DDLTileHelper {
public:
    // The TileData class encapsulates the information and behavior of a single tile when
    // rendering with DDLs.
    class TileData {
    public:
        TileData();
        ~TileData();

        bool initialized() const { return fID != -1; }

        void init(int id,
                  GrDirectContext*,
                  const SkSurfaceCharacterization& dstChar,
                  const SkIRect& clip,
                  const SkIRect& paddingOutsets);

        // Create the DDL for this tile (i.e., fill in 'fDisplayList').
        void createDDL(const SkPicture*);

        void dropDDL() { fDisplayList.reset(); }

        // Precompile all the programs required to draw this tile's DDL
        void precompile(GrDirectContext*);

        // Just draw the re-inflated per-tile SKP directly into this tile w/o going through a DDL
        // first. This is used for determining the overhead of using DDLs (i.e., it replaces
        // a 'createDDL' and 'draw' pair.
        void drawSKPDirectly(GrDirectContext*, const SkPicture*);

        // Replay the recorded DDL into the tile surface - filling in 'fBackendTexture'.
        void draw(GrDirectContext*);

        void reset();

        int id() const { return fID; }
        SkIRect clipRect() const { return fClip; }
        SkISize paddedRectSize() const {
            return { fClip.width() + fPaddingOutsets.fLeft + fPaddingOutsets.fRight,
                     fClip.height() + fPaddingOutsets.fTop + fPaddingOutsets.fBottom };
        }
        SkIVector padOffset() const { return { fPaddingOutsets.fLeft, fPaddingOutsets.fTop }; }

        SkDeferredDisplayList* ddl() { return fDisplayList.get(); }

        sk_sp<SkImage> makePromiseImageForDst(sk_sp<GrContextThreadSafeProxy>);
        void dropCallbackContext() { fCallbackContext.reset(); }

        static void CreateBackendTexture(GrDirectContext*, TileData*);
        static void DeleteBackendTexture(GrDirectContext*, TileData*);

    private:
        sk_sp<SkSurface> makeWrappedTileDest(GrRecordingContext* context);

        sk_sp<PromiseImageCallbackContext> refCallbackContext() { return fCallbackContext; }

        int                       fID = -1;
        SkIRect                   fClip;             // in the device space of the final SkSurface
        SkIRect                   fPaddingOutsets;   // random padding for the output surface
        SkSurfaceCharacterization fPlaybackChar;     // characterization for the tile's dst surface

        // The callback context holds (via its SkPromiseImageTexture) the backend texture
        // that is both wrapped in 'fTileSurface' and backs this tile's promise image
        // (i.e., the one returned by 'makePromiseImage').
        sk_sp<PromiseImageCallbackContext> fCallbackContext;
        // 'fTileSurface' wraps the backend texture in 'fCallbackContext' and must exist until
        // after 'fDisplayList' has been flushed (bc it owns the proxy the DDL's destination
        // trampoline points at).
        // TODO: fix the ref-order so we don't need 'fTileSurface' here
        sk_sp<SkSurface>              fTileSurface;

        sk_sp<SkDeferredDisplayList>  fDisplayList;
    };

    DDLTileHelper(GrDirectContext*,
                  const SkSurfaceCharacterization& dstChar,
                  const SkIRect& viewport,
                  int numXDivisions, int numYDivisions,
                  bool addRandomPaddingToDst);

    // TODO: Move this to PromiseImageHelper and have one method that does all the work and
    // returns the shared SkP.
    void createSKP(sk_sp<GrContextThreadSafeProxy>,
                   SkData* compressedPictureData,
                   const DDLPromiseImageHelper&);

    void kickOffThreadedWork(SkTaskGroup* recordingTaskGroup,
                             SkTaskGroup* gpuTaskGroup,
                             GrDirectContext*);

    void createDDLsInParallel();

    // Create the DDL that will compose all the tile images into a final result.
    void createComposeDDL();
    const sk_sp<SkDeferredDisplayList>& composeDDL() const { return fComposeDDL; }

    // For each tile, create its DDL and then draw it - all on a single thread. This is to allow
    // comparison w/ just drawing the SKP directly (i.e., drawAllTilesDirectly). The
    // DDL creations and draws are interleaved to prevent starvation of the GPU.
    // Note: this is somewhat of a misuse/pessimistic-use of DDLs since they are supposed to
    // be created on a separate thread.
    void interleaveDDLCreationAndDraw(GrDirectContext*);

    // This draws all the per-tile SKPs directly into all of the tiles w/o converting them to
    // DDLs first - all on a single thread.
    void drawAllTilesDirectly(GrDirectContext*);

    void dropCallbackContexts();
    void resetAllTiles();

    int numTiles() const { return fNumXDivisions * fNumYDivisions; }

    void createBackendTextures(SkTaskGroup*, GrDirectContext*);
    void deleteBackendTextures(SkTaskGroup*, GrDirectContext*);

private:
    int                                    fNumXDivisions; // number of tiles horizontally
    int                                    fNumYDivisions; // number of tiles vertically
    SkAutoTArray<TileData>                 fTiles;        // 'fNumXDivisions' x 'fNumYDivisions'

    sk_sp<SkDeferredDisplayList>           fComposeDDL;

    const SkSurfaceCharacterization        fDstCharacterization;
    sk_sp<SkPicture>                       fReconstitutedPicture;
    SkTArray<sk_sp<SkImage>>               fPromiseImages; // All the promise images in the
                                                           // reconstituted picture
};

#endif
