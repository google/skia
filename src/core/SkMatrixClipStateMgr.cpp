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
                                                               const SkMatrix& matrix) {
    int pathID = picRecord->addPathToHeap(path);

    ClipOp& newClip = fClips.push_back();
    newClip.fClipType = kPath_ClipType;
    newClip.fGeom.fPathID = pathID;
    newClip.fOp = op;
    newClip.fDoAA = doAA;
    newClip.fMatrix = matrix;
    newClip.fOffset = kInvalidJumpOffset;
    return false;
}

bool SkMatrixClipStateMgr::MatrixClipState::ClipInfo::clipRegion(SkPictureRecord* picRecord, 
                                                                 const SkRegion& region, 
                                                                 SkRegion::Op op, 
                                                                 const SkMatrix& matrix) {
    // TODO: add a region dictionary so we don't have to copy the region in here                                                                    
    ClipOp& newClip = fClips.push_back();
    newClip.fClipType = kRegion_ClipType;
    newClip.fGeom.fRegion = SkNEW(SkRegion(region));
    newClip.fOp = op;
    newClip.fDoAA = true;      // not necessary but sanity preserving
    newClip.fMatrix = matrix;
    newClip.fOffset = kInvalidJumpOffset;
    return false;
}

void SkMatrixClipStateMgr::WriteDeltaMat(SkPictureRecord* picRecord,
                                         const SkMatrix& current, 
                                         const SkMatrix& desired) {
    SkMatrix delta;
    current.invert(&delta);
    delta.preConcat(desired);
    picRecord->recordConcat(delta);
}

// Note: this only writes out the clips for the current save state. To get the
// entire clip stack requires iterating of the entire matrix/clip stack.
void SkMatrixClipStateMgr::MatrixClipState::ClipInfo::writeClip(SkMatrix* curMat, 
                                                                SkPictureRecord* picRecord, 
                                                                bool* overrideFirstOp) {
    for (int i = 0; i < fClips.count(); ++i) {
        ClipOp& curClip = fClips[i];

        SkRegion::Op op = curClip.fOp;
        if (*overrideFirstOp) {
            op = SkRegion::kReplace_Op;
            *overrideFirstOp = false;
        }

        // TODO: re-add an internal matrix dictionary to not write out
        // redundant matrices.
        // TODO: right now we're writing out the delta matrix from the prior
        // matrix state. This is a side-effect of writing out the entire
        // clip stack and should be resolved when that is fixed.
        SkMatrixClipStateMgr::WriteDeltaMat(picRecord, *curMat, curClip.fMatrix);
        *curMat = curClip.fMatrix;
            
        switch (curClip.fClipType) {
        case kRect_ClipType:
            curClip.fOffset = picRecord->recordClipRect(curClip.fGeom.fRRect.rect(), 
                                                        op, curClip.fDoAA);
            break;
        case kRRect_ClipType:
            curClip.fOffset = picRecord->recordClipRRect(curClip.fGeom.fRRect, op, 
                                                         curClip.fDoAA);
            break;
        case kPath_ClipType:
            curClip.fOffset = picRecord->recordClipPath(curClip.fGeom.fPathID, op, 
                                                        curClip.fDoAA);
            break;
        case kRegion_ClipType:
            curClip.fOffset = picRecord->recordClipRegion(*curClip.fGeom.fRegion, op);
            break;
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
        SkDEBUGCODE(uint32_t peek = writer->read32At(curClip.fOffset);)
        SkASSERT(-1 == peek);
        writer->write32At(curClip.fOffset, restoreOffset);
        SkDEBUGCODE(curClip.fOffset = -1;)
    }
}

SkMatrixClipStateMgr::SkMatrixClipStateMgr()
    : fPicRecord(NULL)
    , fCurOpenStateID(kIdentityWideOpenStateID)
    , fMatrixClipStack(sizeof(MatrixClipState), 
                       fMatrixClipStackStorage, 
                       sizeof(fMatrixClipStackStorage)) {
    fCurMCState = (MatrixClipState*)fMatrixClipStack.push_back();
    new (fCurMCState) MatrixClipState(NULL, 0);    // balanced in restore()
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

    SkMatrix curMat = SkMatrix::I();
    for (const MatrixClipState* state = (const MatrixClipState*) iter.next();
         state != NULL;
         state = (const MatrixClipState*) iter.next()) {
         state->fClipInfo->writeClip(&curMat, fPicRecord, &firstClip);
    }

    // write out matrix
    if (!fCurMCState->fMatrixInfo->fMatrix.isIdentity()) {
        // TODO: writing out the delta matrix here is an artifact of the writing 
        // out of the entire clip stack (with its matrices). Ultimately we will 
        // write out the CTM here when the clip state is collapsed to a single path.
        WriteDeltaMat(fPicRecord, curMat, fCurMCState->fMatrixInfo->fMatrix);
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