/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMatrixClipStateMgr_DEFINED
#define SkMatrixClipStateMgr_DEFINED

#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkRegion.h"
#include "SkRRect.h"
#include "SkTypes.h"
#include "SkTDArray.h"

class SkPictureRecord;
class SkWriter32;

// The SkMatrixClipStateMgr collapses the matrix/clip state of an SkPicture into
// a series of save/restore blocks of consistent matrix clip state, e.g.:
//
//   save
//     clip(s)
//     concat
//     ... draw ops ...
//   restore
//
// SaveLayers simply add another level, e.g.:
//
//   save
//     clip(s)
//     concat
//     ... draw ops ...
//     saveLayer
//       save
//         clip(s)
//         concat
//         ... draw ops ...
//       restore
//     restore
//   restore
//
// As a side effect of this process all saves and saveLayers will become
// kMatrixClip_SaveFlag style saves/saveLayers.

// The SkMatrixClipStateMgr works by intercepting all the save*, restore, clip*,
// and matrix calls sent to SkCanvas in order to track the current matrix/clip
// state. All the other canvas calls get funnelled into a generic "call" entry
// point that signals that a state block is required.
class SkMatrixClipStateMgr {
public:
    static const int32_t kIdentityWideOpenStateID = 0;
    static const int kIdentityMatID = 0;

    class MatrixClipState : SkNoncopyable {
    public:
        class MatrixInfo {
        public:
            void reset() {
                fMatrixID = kIdentityMatID;
                fMatrix.reset();
            }

            void preTranslate(SkScalar dx, SkScalar dy) {
                fMatrixID = -1;
                fMatrix.preTranslate(dx, dy);
            }

            void preScale(SkScalar sx, SkScalar sy) {
                fMatrixID = -1;
                fMatrix.preScale(sx, sy);
            }

            void preRotate(SkScalar degrees) {
                fMatrixID = -1;
                fMatrix.preRotate(degrees);
            }

            void preSkew(SkScalar sx, SkScalar sy) {
                fMatrixID = -1;
                fMatrix.preSkew(sx, sy);
            }

            void preConcat(const SkMatrix& matrix) {
                fMatrixID = -1;
                fMatrix.preConcat(matrix);
            }

            void setMatrix(const SkMatrix& matrix) {
                fMatrixID = -1;
                fMatrix = matrix;
            }

            int getID(SkMatrixClipStateMgr* mgr) {
                if (fMatrixID >= 0) {
                    return fMatrixID;
                }

                fMatrixID = mgr->addMatToDict(fMatrix);
                return fMatrixID;
            }

        private:
            SkMatrix fMatrix;
            int      fMatrixID;

            typedef SkNoncopyable INHERITED;
        };

        class ClipInfo : SkNoncopyable {
        public:
            ClipInfo() {}

            bool clipRect(const SkRect& rect,
                          SkRegion::Op op,
                          bool doAA,
                          int matrixID) {
                ClipOp* newClip = fClips.append();
                newClip->fClipType = kRect_ClipType;
                newClip->fGeom.fRRect.setRect(rect);   // storing the clipRect in the RRect
                newClip->fOp = op;
                newClip->fDoAA = doAA;
                newClip->fMatrixID = matrixID;
                return false;
            }

            bool clipRRect(const SkRRect& rrect,
                           SkRegion::Op op,
                           bool doAA,
                           int matrixID) {
                ClipOp* newClip = fClips.append();
                newClip->fClipType = kRRect_ClipType;
                newClip->fGeom.fRRect = rrect;
                newClip->fOp = op;
                newClip->fDoAA = doAA;
                newClip->fMatrixID = matrixID;
                return false;
            }

            bool clipPath(SkPictureRecord* picRecord,
                          const SkPath& path,
                          SkRegion::Op op,
                          bool doAA,
                          int matrixID);
            bool clipRegion(SkPictureRecord* picRecord,
                            int regionID,
                            SkRegion::Op op,
                            int matrixID);
            void writeClip(int* curMatID, SkMatrixClipStateMgr* mgr);

            SkDEBUGCODE(int numClips() const { return fClips.count(); })

        private:
            enum ClipType {
                kRect_ClipType,
                kRRect_ClipType,
                kPath_ClipType,
                kRegion_ClipType
            };

            class ClipOp {
            public:
                ClipType     fClipType;

                union {
                    SkRRect fRRect;        // also stores clip rect
                    int     fPathID;
                    int     fRegionID;
                } fGeom;

                bool         fDoAA;
                SkRegion::Op fOp;

                // The CTM in effect when this clip call was issued
                int          fMatrixID;
            };

            SkTDArray<ClipOp> fClips;

            typedef SkNoncopyable INHERITED;
        };

        MatrixClipState(MatrixClipState* prev)
            : fPrev(prev)
        {
            fHasOpen = false;

            if (NULL == prev) {
                fLayerID = 0;

                fMatrixInfoStorage.reset();
                fMatrixInfo = &fMatrixInfoStorage;
                fClipInfo = &fClipInfoStorage;  // ctor handles init of fClipInfoStorage

                // The identity/wide-open-clip state is current by default
                fMCStateID = kIdentityWideOpenStateID;
#ifdef SK_DEBUG
                fExpectedDepth = 1;
#endif
            }
            else {
                fLayerID = prev->fLayerID;

                fMatrixInfoStorage = *prev->fMatrixInfo;
                fMatrixInfo = &fMatrixInfoStorage;

                // We don't copy the ClipOps of the previous clip states
                fClipInfo = &fClipInfoStorage;

                // Initially a new save/saveLayer represents the same MC state
                // as its predecessor.
                fMCStateID = prev->fMCStateID;
#ifdef SK_DEBUG
                fExpectedDepth = prev->fExpectedDepth;
#endif
            }

            fIsSaveLayer = false;
        }

        MatrixInfo*  fMatrixInfo;
        MatrixInfo   fMatrixInfoStorage;

        ClipInfo*    fClipInfo;
        ClipInfo     fClipInfoStorage;

        // Tracks the current depth of saveLayers to support the isDrawingToLayer call
        int          fLayerID;
        // Does this MC state represent a saveLayer call?
        bool         fIsSaveLayer;

        // The next field is only valid when fIsSaveLayer is set.
        SkTDArray<int>* fSavedSkipOffsets;

        // Does the MC state have an open block in the skp?
        bool         fHasOpen;

        MatrixClipState* fPrev;

#ifdef SK_DEBUG
        int              fExpectedDepth;    // debugging aid
#endif

        int32_t     fMCStateID;
    };

    enum CallType {
        kMatrix_CallType,
        kClip_CallType,
        kOther_CallType
    };

    SkMatrixClipStateMgr();
    ~SkMatrixClipStateMgr();

    void init(SkPictureRecord* picRecord) {
        // Note: we're not taking a ref here. It is expected that the SkMatrixClipStateMgr
        // is owned by the SkPictureRecord object
        fPicRecord = picRecord;
    }

    SkPictureRecord* getPicRecord() { return fPicRecord; }

    // TODO: need to override canvas' getSaveCount. Right now we pass the
    // save* and restore calls on to the base SkCanvas in SkPictureRecord but
    // this duplicates effort.
    int getSaveCount() const { return fMatrixClipStack.count(); }

    int save();

    int saveLayer(const SkRect* bounds, const SkPaint* paint, SkCanvas::SaveFlags flags);

    bool isDrawingToLayer() const {
        return fCurMCState->fLayerID > 0;
    }

    void restore();

    void translate(SkScalar dx, SkScalar dy) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->preTranslate(dx, dy);
    }

    void scale(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->preScale(sx, sy);
    }

    void rotate(SkScalar degrees) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->preRotate(degrees);
    }

    void skew(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->preSkew(sx, sy);
    }

    void concat(const SkMatrix& matrix) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->preConcat(matrix);
    }

    void setMatrix(const SkMatrix& matrix) {
        this->call(kMatrix_CallType);
        fCurMCState->fMatrixInfo->setMatrix(matrix);
    }

    bool clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipRect(rect, op, doAA,
                                                fCurMCState->fMatrixInfo->getID(this));
    }

    bool clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipRRect(rrect, op, doAA,
                                                 fCurMCState->fMatrixInfo->getID(this));
    }

    bool clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        return fCurMCState->fClipInfo->clipPath(fPicRecord, path, op, doAA,
                                                fCurMCState->fMatrixInfo->getID(this));
    }

    bool clipRegion(const SkRegion& region, SkRegion::Op op) {
        this->call(SkMatrixClipStateMgr::kClip_CallType);
        int regionID = this->addRegionToDict(region);
        return fCurMCState->fClipInfo->clipRegion(fPicRecord, regionID, op,
                                                  fCurMCState->fMatrixInfo->getID(this));
    }

    bool call(CallType callType);

    void fillInSkips(SkWriter32* writer, int32_t restoreOffset);

    void finish();

protected:
    SkPictureRecord* fPicRecord;

    uint32_t         fMatrixClipStackStorage[43]; // sized to fit 2 clip states
    SkDeque          fMatrixClipStack;
    MatrixClipState* fCurMCState;

    // This dictionary doesn't actually de-duplicate the matrices (except for the
    // identity matrix). It merely stores the matrices and allows them to be looked
    // up by ID later. The de-duplication mainly falls upon the matrix/clip stack
    // which stores the ID so a revisited clip/matrix (via popping the stack) will
    // use the same ID.
    SkTDArray<SkMatrix> fMatrixDict;

    SkTDArray<SkRegion*> fRegionDict;

    // The MCStateID of the state currently in effect in the byte stream. 0 if none.
    int32_t          fCurOpenStateID;
    // The skip offsets for the current open state. These are the locations in the
    // skp that must be filled in when the current open state is closed. These are
    // here rather then distributed across the MatrixClipState's because saveLayers
    // can cause MC states to be nested.
    SkTDArray<int32_t>  *fSkipOffsets;  // TODO: should we store u32 or size_t instead?

    SkDEBUGCODE(void validate();)

    int MCStackPush();

    void addClipOffset(size_t offset) {
        SkASSERT(NULL != fSkipOffsets);
        SkASSERT(kIdentityWideOpenStateID != fCurOpenStateID);
        SkASSERT(fCurMCState->fHasOpen);
        SkASSERT(!fCurMCState->fIsSaveLayer);

        *fSkipOffsets->append() = SkToS32(offset);
    }

    void writeDeltaMat(int currentMatID, int desiredMatID);
    static int32_t   NewMCStateID();

    int addRegionToDict(const SkRegion& region);
    const SkRegion* lookupRegion(int index) {
        SkASSERT(index >= 0 && index < fRegionDict.count());
        return fRegionDict[index];
    }

    // TODO: add stats to check if the dictionary really does
    // reduce the size of the SkPicture.
    int addMatToDict(const SkMatrix& mat);
    const SkMatrix& lookupMat(int index) {
        SkASSERT(index >= 0 && index < fMatrixDict.count());
        return fMatrixDict[index];
    }

    bool isNestingMCState(int stateID);

#ifdef SK_DEBUG
    int fActualDepth;
#endif

    // save layers are nested within a specific MC state. This stack tracks
    // the nesting MC state's ID as save layers are pushed and popped.
    SkTDArray<int> fStateIDStack;
};

#endif
