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

    class MatrixClipState {
    public:
        class MatrixInfo {
        public:
            void reset() {
                fMatrixID = kIdentityMatID;
                fMatrix.reset();
            }

            bool preTranslate(SkScalar dx, SkScalar dy) {
                fMatrixID = -1;
                return fMatrix.preTranslate(dx, dy);
            }

            bool preScale(SkScalar sx, SkScalar sy) {
                fMatrixID = -1;
                return fMatrix.preScale(sx, sy);
            }

            bool preRotate(SkScalar degrees) {
                fMatrixID = -1;
                return fMatrix.preRotate(degrees);
            }

            bool preSkew(SkScalar sx, SkScalar sy) {
                fMatrixID = -1;
                return fMatrix.preSkew(sx, sy);
            }

            bool preConcat(const SkMatrix& matrix) {
                fMatrixID = -1;
                return fMatrix.preConcat(matrix);
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
        };

        class ClipInfo : public SkNoncopyable {
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
                newClip->fOffset = kInvalidJumpOffset;
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
                newClip->fOffset = kInvalidJumpOffset;
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
            void writeClip(int* curMatID,
                           SkMatrixClipStateMgr* mgr,
                           bool* overrideFirstOp);
            void fillInSkips(SkWriter32* writer, int32_t restoreOffset);

#ifdef SK_DEBUG
            void checkOffsetNotEqual(int32_t offset) {
                for (int i = 0; i < fClips.count(); ++i) {
                    ClipOp& curClip = fClips[i];
                    SkASSERT(offset != curClip.fOffset);
                }
            }
#endif
        private:
            enum ClipType {
                kRect_ClipType,
                kRRect_ClipType,
                kPath_ClipType,
                kRegion_ClipType
            };

            static const int kInvalidJumpOffset = -1;

            class ClipOp {
            public:
                ClipType     fClipType;

                union {
                    SkRRect fRRect;        // also stores clipRect
                    int     fPathID;
                    int     fRegionID;
                } fGeom;

                bool         fDoAA;
                SkRegion::Op fOp;

                // The CTM in effect when this clip call was issued
                int          fMatrixID;

                // The offset of this clipOp's "jump-to-offset" location in the skp.
                // -1 means the offset hasn't been written.
                int32_t      fOffset;
            };

            SkTDArray<ClipOp> fClips;

            typedef SkNoncopyable INHERITED;
        };

        MatrixClipState(MatrixClipState* prev, int flags)
#ifdef SK_DEBUG
            : fPrev(prev)
#endif
        {
            if (NULL == prev) {
                fLayerID = 0;

                fMatrixInfoStorage.reset();
                fMatrixInfo = &fMatrixInfoStorage;
                fClipInfo = &fClipInfoStorage;  // ctor handles init of fClipInfoStorage

                // The identity/wide-open-clip state is current by default
                fMCStateID = kIdentityWideOpenStateID;
            }
            else {
                fLayerID = prev->fLayerID;

                if (flags & SkCanvas::kMatrix_SaveFlag) {
                    fMatrixInfoStorage = *prev->fMatrixInfo;
                    fMatrixInfo = &fMatrixInfoStorage;
                } else {
                    fMatrixInfo = prev->fMatrixInfo;
                }

                if (flags & SkCanvas::kClip_SaveFlag) {
                    // We don't copy the ClipOps of the previous clip states
                    fClipInfo = &fClipInfoStorage;
                } else {
                    fClipInfo = prev->fClipInfo;
                }

                // Initially a new save/saveLayer represents the same MC state
                // as its predecessor.
                fMCStateID = prev->fMCStateID;
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

        // The next two fields are only valid when fIsSaveLayer is set.
        int32_t      fSaveLayerBaseStateID;
        bool         fSaveLayerBracketed;

#ifdef SK_DEBUG
        MatrixClipState* fPrev; // debugging aid
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

    int save(SkCanvas::SaveFlags flags);

    int saveLayer(const SkRect* bounds, const SkPaint* paint, SkCanvas::SaveFlags flags);

    bool isDrawingToLayer() const {
        return fCurMCState->fLayerID > 0;
    }

    void restore();

    bool translate(SkScalar dx, SkScalar dy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->preTranslate(dx, dy);
    }

    bool scale(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->preScale(sx, sy);
    }

    bool rotate(SkScalar degrees) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->preRotate(degrees);
    }

    bool skew(SkScalar sx, SkScalar sy) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->preSkew(sx, sy);
    }

    bool concat(const SkMatrix& matrix) {
        this->call(kMatrix_CallType);
        return fCurMCState->fMatrixInfo->preConcat(matrix);
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

    void fillInSkips(SkWriter32* writer, int32_t restoreOffset) {
        // Since we write out the entire clip stack at each block start we
        // need to update the skips for the entire stack each time too.
        SkDeque::F2BIter iter(fMatrixClipStack);

        for (const MatrixClipState* state = (const MatrixClipState*) iter.next();
             state != NULL;
             state = (const MatrixClipState*) iter.next()) {
            state->fClipInfo->fillInSkips(writer, restoreOffset);
        }
    }

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

    SkDEBUGCODE(void validate();)

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
};

#endif
