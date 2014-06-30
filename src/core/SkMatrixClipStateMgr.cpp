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
    return false;
}

bool SkMatrixClipStateMgr::MatrixClipState::ClipInfo::clipRegion(SkPictureRecord* picRecord,
                                                                 int regionID,
                                                                 SkRegion::Op op,
                                                                 int matrixID) {
    ClipOp* newClip = fClips.append();
    newClip->fClipType = kRegion_ClipType;
    newClip->fGeom.fRegionID = regionID;
    newClip->fOp = op;
    newClip->fDoAA = true;      // not necessary but sanity preserving
    newClip->fMatrixID = matrixID;
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
                                                                SkMatrixClipStateMgr* mgr) {
    for (int i = 0; i < fClips.count(); ++i) {
        ClipOp& curClip = fClips[i];

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

        size_t offset = 0;

        switch (curClip.fClipType) {
        case kRect_ClipType:
            offset = mgr->getPicRecord()->recordClipRect(curClip.fGeom.fRRect.rect(),
                                                         curClip.fOp, curClip.fDoAA);
            break;
        case kRRect_ClipType:
            offset = mgr->getPicRecord()->recordClipRRect(curClip.fGeom.fRRect, curClip.fOp,
                                                         curClip.fDoAA);
            break;
        case kPath_ClipType:
            offset = mgr->getPicRecord()->recordClipPath(curClip.fGeom.fPathID, curClip.fOp,
                                                         curClip.fDoAA);
            break;
        case kRegion_ClipType: {
            const SkRegion* region = mgr->lookupRegion(curClip.fGeom.fRegionID);
            offset = mgr->getPicRecord()->recordClipRegion(*region, curClip.fOp);
            break;
        }
        default:
            SkASSERT(0);
        }

        mgr->addClipOffset(offset);
    }
}

SkMatrixClipStateMgr::SkMatrixClipStateMgr()
    : fPicRecord(NULL)
    , fMatrixClipStack(sizeof(MatrixClipState),
                       fMatrixClipStackStorage,
                       sizeof(fMatrixClipStackStorage))
    , fCurOpenStateID(kIdentityWideOpenStateID) {

    fSkipOffsets = SkNEW(SkTDArray<int>);

    // The first slot in the matrix dictionary is reserved for the identity matrix
    fMatrixDict.append()->reset();

    fCurMCState = (MatrixClipState*)fMatrixClipStack.push_back();
    new (fCurMCState) MatrixClipState(NULL);    // balanced in restore()

#ifdef SK_DEBUG
    fActualDepth = 0;
#endif
}

SkMatrixClipStateMgr::~SkMatrixClipStateMgr() {
    for (int i = 0; i < fRegionDict.count(); ++i) {
        SkDELETE(fRegionDict[i]);
    }

    SkDELETE(fSkipOffsets);
}


int SkMatrixClipStateMgr::MCStackPush() {
    MatrixClipState* newTop = (MatrixClipState*)fMatrixClipStack.push_back();
    new (newTop) MatrixClipState(fCurMCState); // balanced in restore()
    fCurMCState = newTop;

    SkDEBUGCODE(this->validate();)

    return fMatrixClipStack.count();
}

int SkMatrixClipStateMgr::save() {
    SkDEBUGCODE(this->validate();)

    return this->MCStackPush();
}

int SkMatrixClipStateMgr::saveLayer(const SkRect* bounds, const SkPaint* paint,
                                    SkCanvas::SaveFlags flags) {
#ifdef SK_DEBUG
    if (fCurMCState->fIsSaveLayer) {
        SkASSERT(0 == fSkipOffsets->count());
    }
#endif

    // Since the saveLayer call draws something we need to potentially dump
    // out the MC state
    SkDEBUGCODE(bool saved =) this->call(kOther_CallType);

    int result = this->MCStackPush();
    ++fCurMCState->fLayerID;
    fCurMCState->fIsSaveLayer = true;

#ifdef SK_DEBUG
    if (saved) {
        fCurMCState->fExpectedDepth++; // 1 for nesting save
    }
    fCurMCState->fExpectedDepth++;   // 1 for saveLayer
#endif

    *fStateIDStack.append() = fCurOpenStateID;
    fCurMCState->fSavedSkipOffsets = fSkipOffsets;

    // TODO: recycle these rather then new & deleting them on every saveLayer/
    // restore
    fSkipOffsets = SkNEW(SkTDArray<int>);

    fPicRecord->recordSaveLayer(bounds, paint, flags);
#ifdef SK_DEBUG
    fActualDepth++;
#endif
    return result;
}

void SkMatrixClipStateMgr::restore() {
    SkDEBUGCODE(this->validate();)

    if (fCurMCState->fIsSaveLayer) {
        if (fCurMCState->fHasOpen) {
            fCurMCState->fHasOpen = false;
            fPicRecord->recordRestore(); // Close the open block inside the saveLayer
#ifdef SK_DEBUG
            SkASSERT(fActualDepth > 0);
            fActualDepth--;
#endif
        } else {
            SkASSERT(0 == fSkipOffsets->count());
        }

        // The saveLayer's don't carry any matrix or clip state in the
        // new scheme so make sure the saveLayer's recordRestore doesn't
        // try to finalize them (i.e., fill in their skip offsets).
        fPicRecord->recordRestore(false); // close of saveLayer
#ifdef SK_DEBUG
        SkASSERT(fActualDepth > 0);
        fActualDepth--;
#endif

        SkASSERT(fStateIDStack.count() >= 1);
        fCurOpenStateID = fStateIDStack[fStateIDStack.count()-1];
        fStateIDStack.pop();

        SkASSERT(0 == fSkipOffsets->count());
        SkASSERT(NULL != fCurMCState->fSavedSkipOffsets);

        SkDELETE(fSkipOffsets);
        fSkipOffsets = fCurMCState->fSavedSkipOffsets;
    }

    bool prevHadOpen = fCurMCState->fHasOpen;
    bool prevWasSaveLayer = fCurMCState->fIsSaveLayer;

    fCurMCState->~MatrixClipState();       // balanced in save()
    fMatrixClipStack.pop_back();
    fCurMCState = (MatrixClipState*)fMatrixClipStack.back();

    if (!prevWasSaveLayer) {
        fCurMCState->fHasOpen = prevHadOpen;
    }

    if (fCurMCState->fIsSaveLayer) {
        if (0 != fSkipOffsets->count()) {
            SkASSERT(fCurMCState->fHasOpen);
        }
    }

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

bool SkMatrixClipStateMgr::isNestingMCState(int stateID) {
    return fStateIDStack.count() > 0 && fStateIDStack[fStateIDStack.count()-1] == fCurOpenStateID;
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

    if (kIdentityWideOpenStateID != fCurOpenStateID &&
        !this->isNestingMCState(fCurOpenStateID)) {
        // Don't write a restore if the open state is one in which a saveLayer
        // is nested. The save after the saveLayer's restore will close it.
        fPicRecord->recordRestore();    // Close the open block
        fCurMCState->fHasOpen = false;
#ifdef SK_DEBUG
        SkASSERT(fActualDepth > 0);
        fActualDepth--;
#endif
    }

    // Install the required MC state as the active one
    fCurOpenStateID = fCurMCState->fMCStateID;

    if (kIdentityWideOpenStateID == fCurOpenStateID) {
        SkASSERT(0 == fActualDepth);
        SkASSERT(!fCurMCState->fHasOpen);
        SkASSERT(0 == fSkipOffsets->count());
        return false;
    }

    SkASSERT(!fCurMCState->fHasOpen);
    SkASSERT(0 == fSkipOffsets->count());
    fCurMCState->fHasOpen = true;
    fPicRecord->recordSave();
#ifdef SK_DEBUG
    fActualDepth++;
    SkASSERT(fActualDepth == fCurMCState->fExpectedDepth);
#endif

    // write out clips
    SkDeque::Iter iter(fMatrixClipStack, SkDeque::Iter::kBack_IterStart);
    const MatrixClipState* state;
    // Loop back across the MC states until the last saveLayer. The MC
    // state in front of the saveLayer has already been written out.
    for (state = (const MatrixClipState*) iter.prev();
         state != NULL;
         state = (const MatrixClipState*) iter.prev()) {
        if (state->fIsSaveLayer) {
            break;
        }
    }

    int curMatID;

    if (NULL == state) {
        // There was no saveLayer in the MC stack so we need to output them all
        iter.reset(fMatrixClipStack, SkDeque::Iter::kFront_IterStart);
        state = (const MatrixClipState*) iter.next();
        curMatID = kIdentityMatID;
    } else {
        // SkDeque's iterators actually return the previous location so we
        // need to reverse and go forward one to get back on track.
        iter.next();
        SkDEBUGCODE(const MatrixClipState* test = (const MatrixClipState*)) iter.next();
        SkASSERT(test == state);

        curMatID = state->fMatrixInfo->getID(this);

        // TODO: this assumes that, in the case of Save|SaveLayer when the SaveLayer
        // doesn't save the clip, that the SaveLayer doesn't add any additional clip state.
        // This assumption will be removed when we explicitly store the clip state in
        // self-contained objects. It is valid for the small set of skps.
        if (NULL != state->fPrev && state->fClipInfo == state->fPrev->fClipInfo) {
            // By the above assumption the SaveLayer's MC state has already been
            // written out by the prior Save so don't output it again.
            state = (const MatrixClipState*) iter.next();
        }
    }

    for ( ; state != NULL; state = (const MatrixClipState*) iter.next()) {
         state->fClipInfo->writeClip(&curMatID, this);
    }

    // write out matrix
    // TODO: this test isn't quite right. It should be:
    //   if (curMatID != fCurMCState->fMatrixInfo->getID(this)) {
    // but right now the testing harness always expects a matrix if
    // the matrices are non-I
    if (kIdentityMatID != fCurMCState->fMatrixInfo->getID(this)) {
        // TODO: writing out the delta matrix here is an artifact of the writing
        // out of the entire clip stack (with its matrices). Ultimately we will
        // write out the CTM here when the clip state is collapsed to a single path.
        this->writeDeltaMat(curMatID, fCurMCState->fMatrixInfo->getID(this));
    }

    SkDEBUGCODE(this->validate();)
    return true;
}

// Fill in the skip offsets for all the clips written in the current block
void SkMatrixClipStateMgr::fillInSkips(SkWriter32* writer, int32_t restoreOffset) {
    for (int i = 0; i < fSkipOffsets->count(); ++i) {
        SkDEBUGCODE(int32_t peek = writer->readTAt<int32_t>((*fSkipOffsets)[i]);)
        SkASSERT(-1 == peek);
        writer->overwriteTAt<int32_t>((*fSkipOffsets)[i], restoreOffset);
    }

    fSkipOffsets->rewind();
    SkASSERT(0 == fSkipOffsets->count());
}

void SkMatrixClipStateMgr::finish() {
    if (kIdentityWideOpenStateID != fCurOpenStateID) {
        fPicRecord->recordRestore();    // Close the open block
        fCurMCState->fHasOpen = false;
#ifdef SK_DEBUG
        SkASSERT(fActualDepth > 0);
        fActualDepth--;
#endif
        fCurOpenStateID = kIdentityWideOpenStateID;
        SkASSERT(!fCurMCState->fHasOpen);
    }
}

#ifdef SK_DEBUG
void SkMatrixClipStateMgr::validate() {
    if (fCurOpenStateID == fCurMCState->fMCStateID && !this->isNestingMCState(fCurOpenStateID)) {
        // The current state is the active one so it should have a skip
        // offset for each clip
        SkDeque::Iter iter(fMatrixClipStack, SkDeque::Iter::kBack_IterStart);
        int clipCount = 0;
        for (const MatrixClipState* state = (const MatrixClipState*) iter.prev();
             state != NULL;
             state = (const MatrixClipState*) iter.prev()) {
            if (NULL == state->fPrev || state->fPrev->fClipInfo != state->fClipInfo) {
                clipCount += state->fClipInfo->numClips();
            }
            if (state->fIsSaveLayer) {
                break;
            }
        }

        SkASSERT(fSkipOffsets->count() == clipCount);
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
