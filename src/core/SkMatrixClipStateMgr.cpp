/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixClipStateMgr.h"
#include "SkPictureRecord.h"

bool SkMatrixClipStateMgr::MatrixClipState::ClipInfo::clipPath(SkPictureRecord* picRecord,
                                                               const SkPath& path,
                                                               SkRegion::Op op,
                                                               bool doAA,
                                                               int matrixID) {
    int pathID = picRecord->addPathToHeap(path);

    ClipOp* newClip = fClips.append();
    newClip->fClipType = kPath_ClipType;
    newClip->fGeom.fPathID = pathID;
    newClip->fOp = op;
    newClip->fDoAA = doAA;
    newClip->fMatrixID = matrixID;
    newClip->fOffset = kInvalidJumpOffset;
    return false;
}

bool SkMatrixClipStateMgr::MatrixClipState::ClipInfo::clipRegion(SkPictureRecord* picRecord,
                                                                 int regionID,
                                                                 SkRegion::Op op,
                                                                 int matrixID) {
    // TODO: add a region dictionary so we don't have to copy the region in here
    ClipOp* newClip = fClips.append();
    newClip->fClipType = kRegion_ClipType;
    newClip->fGeom.fRegionID = regionID;
    newClip->fOp = op;
    newClip->fDoAA = true;      // not necessary but sanity preserving
    newClip->fMatrixID = matrixID;
    newClip->fOffset = kInvalidJumpOffset;
    return false;
}

void SkMatrixClipStateMgr::writeDeltaMat(int currentMatID, int desiredMatID) {
    const SkMatrix& current = this->lookupMat(currentMatID);
    const SkMatrix& desired = this->lookupMat(desiredMatID);

    SkMatrix delta;
    bool result = current.invert(&delta);
    if (result) {
        delta.preConcat(desired);
    }
    fPicRecord->recordConcat(delta);
}

// Note: this only writes out the clips for the current save state. To get the
// entire clip stack requires iterating of the entire matrix/clip stack.
void SkMatrixClipStateMgr::MatrixClipState::ClipInfo::writeClip(int* curMatID,
                                                                SkMatrixClipStateMgr* mgr,
                                                                bool* overrideFirstOp) {
    for (int i = 0; i < fClips.count(); ++i) {
        ClipOp& curClip = fClips[i];

        SkRegion::Op op = curClip.fOp;
        if (*overrideFirstOp) {
            op = SkRegion::kReplace_Op;
            *overrideFirstOp = false;
        }

        // TODO: use the matrix ID to skip writing the identity matrix
        // over and over, i.e.:
        //  if (*curMatID != curClip.fMatrixID) {
        //      mgr->writeDeltaMat...
        //      *curMatID...
        //  }
        // Right now this optimization would throw off the testing harness.
        // TODO: right now we're writing out the delta matrix from the prior
        // matrix state. This is a side-effect of writing out the entire
        // clip stack and should be resolved when that is fixed.
        mgr->writeDeltaMat(*curMatID, curClip.fMatrixID);
        *curMatID = curClip.fMatrixID;

        switch (curClip.fClipType) {
        case kRect_ClipType:
            curClip.fOffset = mgr->getPicRecord()->recordClipRect(curClip.fGeom.fRRect.rect(),
                                                                  op, curClip.fDoAA);
            break;
        case kRRect_ClipType:
            curClip.fOffset = mgr->getPicRecord()->recordClipRRect(curClip.fGeom.fRRect, op,
                                                                   curClip.fDoAA);
            break;
        case kPath_ClipType:
            curClip.fOffset = mgr->getPicRecord()->recordClipPath(curClip.fGeom.fPathID, op,
                                                                  curClip.fDoAA);
            break;
        case kRegion_ClipType: {
            const SkRegion* region = mgr->lookupRegion(curClip.fGeom.fRegionID);
            curClip.fOffset = mgr->getPicRecord()->recordClipRegion(*region, op);
            break;
        }
        default:
            SkASSERT(0);
        }
    }
}

// Fill in the skip offsets for all the clips written in the current block
void SkMatrixClipStateMgr::MatrixClipState::ClipInfo::fillInSkips(SkWriter32* writer,
                                                                  int32_t restoreOffset) {
    for (int i = 0; i < fClips.count(); ++i) {
        ClipOp& curClip = fClips[i];

        if (-1 == curClip.fOffset) {
            continue;
        }
//        SkDEBUGCODE(uint32_t peek = writer->read32At(curClip.fOffset);)
//        SkASSERT(-1 == peek);
        writer->overwriteTAt(curClip.fOffset, restoreOffset);
        SkDEBUGCODE(curClip.fOffset = -1;)
    }
}

SkMatrixClipStateMgr::SkMatrixClipStateMgr()
    : fPicRecord(NULL)
    , fMatrixClipStack(sizeof(MatrixClipState),
                       fMatrixClipStackStorage,
                       sizeof(fMatrixClipStackStorage))
    , fCurOpenStateID(kIdentityWideOpenStateID) {

    // The first slot in the matrix dictionary is reserved for the identity matrix
    fMatrixDict.append()->reset();

    fCurMCState = (MatrixClipState*)fMatrixClipStack.push_back();
    new (fCurMCState) MatrixClipState(NULL, 0);    // balanced in restore()
}

SkMatrixClipStateMgr::~SkMatrixClipStateMgr() {
    for (int i = 0; i < fRegionDict.count(); ++i) {
        SkDELETE(fRegionDict[i]);
    }
}


int SkMatrixClipStateMgr::save(SkCanvas::SaveFlags flags) {
    SkDEBUGCODE(this->validate();)

    MatrixClipState* newTop = (MatrixClipState*)fMatrixClipStack.push_back();
    new (newTop) MatrixClipState(fCurMCState, flags); // balanced in restore()
    fCurMCState = newTop;

    SkDEBUGCODE(this->validate();)

    return fMatrixClipStack.count();
}

int SkMatrixClipStateMgr::saveLayer(const SkRect* bounds, const SkPaint* paint,
                                    SkCanvas::SaveFlags flags) {
    int result = this->save(flags);
    ++fCurMCState->fLayerID;
    fCurMCState->fIsSaveLayer = true;

    fCurMCState->fSaveLayerBracketed = this->call(kOther_CallType);
    fCurMCState->fSaveLayerBaseStateID = fCurOpenStateID;
    fPicRecord->recordSaveLayer(bounds, paint,
                                (SkCanvas::SaveFlags)(flags| SkCanvas::kMatrixClip_SaveFlag));
    return result;
}

void SkMatrixClipStateMgr::restore() {
    SkDEBUGCODE(this->validate();)

    if (fCurMCState->fIsSaveLayer) {
        if (fCurMCState->fSaveLayerBaseStateID != fCurOpenStateID) {
            fPicRecord->recordRestore(); // Close the open block
        }
        // The saveLayer's don't carry any matrix or clip state in the
        // new scheme so make sure the saveLayer's recordRestore doesn't
        // try to finalize them (i.e., fill in their skip offsets).
        fPicRecord->recordRestore(false); // close of saveLayer

        // Close the Save that brackets the saveLayer. TODO: this doesn't handle
        // the skip offsets correctly
        if (fCurMCState->fSaveLayerBracketed) {
            fPicRecord->recordRestore(false);
        }

        // MC states can be allowed to fuse across saveLayer/restore boundaries
        fCurOpenStateID = kIdentityWideOpenStateID;
    }

    fCurMCState->~MatrixClipState();       // balanced in save()
    fMatrixClipStack.pop_back();
    fCurMCState = (MatrixClipState*)fMatrixClipStack.back();

    SkDEBUGCODE(this->validate();)
}

// kIdentityWideOpenStateID (0) is reserved for the identity/wide-open clip state
int32_t SkMatrixClipStateMgr::NewMCStateID() {
    // TODO: guard against wrap around
    // TODO: make uint32_t
    static int32_t gMCStateID = kIdentityWideOpenStateID;
    ++gMCStateID;
    return gMCStateID;
}

bool SkMatrixClipStateMgr::call(CallType callType) {
    SkDEBUGCODE(this->validate();)

    if (kMatrix_CallType == callType || kClip_CallType == callType) {
        fCurMCState->fMCStateID = NewMCStateID();
        SkDEBUGCODE(this->validate();)
        return false;
    }

    SkASSERT(kOther_CallType == callType);

    if (fCurMCState->fMCStateID == fCurOpenStateID) {
        // Required MC state is already active one - nothing to do
        SkDEBUGCODE(this->validate();)
        return false;
    }

    if (kIdentityWideOpenStateID != fCurOpenStateID) {
        fPicRecord->recordRestore();    // Close the open block
    }

    // Install the required MC state as the active one
    fCurOpenStateID = fCurMCState->fMCStateID;

    fPicRecord->recordSave(SkCanvas::kMatrixClip_SaveFlag);

    // write out clips
    SkDeque::F2BIter iter(fMatrixClipStack);
    bool firstClip = true;

    int curMatID = kIdentityMatID;
    for (const MatrixClipState* state = (const MatrixClipState*) iter.next();
         state != NULL;
         state = (const MatrixClipState*) iter.next()) {
         state->fClipInfo->writeClip(&curMatID, this, &firstClip);
    }

    // write out matrix
    if (kIdentityMatID != fCurMCState->fMatrixInfo->getID(this)) {
        // TODO: writing out the delta matrix here is an artifact of the writing
        // out of the entire clip stack (with its matrices). Ultimately we will
        // write out the CTM here when the clip state is collapsed to a single path.
        this->writeDeltaMat(curMatID, fCurMCState->fMatrixInfo->getID(this));
    }

    SkDEBUGCODE(this->validate();)

    return true;
}

void SkMatrixClipStateMgr::finish() {
    if (kIdentityWideOpenStateID != fCurOpenStateID) {
        fPicRecord->recordRestore();    // Close the open block
        fCurOpenStateID = kIdentityWideOpenStateID;
    }
}

#ifdef SK_DEBUG
void SkMatrixClipStateMgr::validate() {
    if (fCurOpenStateID == fCurMCState->fMCStateID) {
        // The current state is the active one so all its skip offsets should
        // still be -1
        SkDeque::F2BIter iter(fMatrixClipStack);

        for (const MatrixClipState* state = (const MatrixClipState*) iter.next();
             state != NULL;
             state = (const MatrixClipState*) iter.next()) {
            state->fClipInfo->checkOffsetNotEqual(-1);
        }
    }
}
#endif

int SkMatrixClipStateMgr::addRegionToDict(const SkRegion& region) {
    int index = fRegionDict.count();
    *fRegionDict.append() = SkNEW(SkRegion(region));
    return index;
}

int SkMatrixClipStateMgr::addMatToDict(const SkMatrix& mat) {
    if (mat.isIdentity()) {
        return kIdentityMatID;
    }

    *fMatrixDict.append() = mat;
    return fMatrixDict.count()-1;
}
