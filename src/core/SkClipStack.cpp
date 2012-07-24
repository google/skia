
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkClipStack.h"
#include "SkPath.h"
#include <new>


struct SkClipStack::Rec {
    enum State {
        kEmpty_State,
        kRect_State,
        kPath_State
    };

    SkPath          fPath;
    SkRect          fRect;
    int             fSaveCount;
    SkRegion::Op    fOp;
    State           fState;
    bool            fDoAA;

    // fFiniteBoundType and fFiniteBound are used to incrementally update
    // the clip stack's bound. When fFiniteBoundType is kNormal_BoundsType, 
    // fFiniteBound represents the  conservative bounding box of the pixels 
    // that aren't clipped (i.e., any pixels that can be drawn to are inside 
    // the bound). When fFiniteBoundType is kInsideOut_BoundsType (which occurs 
    // when a clip is inverse filled), fFiniteBound represents the 
    // conservative bounding box of the pixels that _are_ clipped (i.e., any 
    // pixels that cannot be drawn to are inside the bound). When 
    // fFiniteBoundType is kInsideOut_BoundsType the actual bound is
    // the infinite plane. This behavior of fFiniteBoundType and
    // fFiniteBound is required so that we can capture the cancelling out
    // of the extensions to infinity when two inverse filled clips are
    // Booleaned together.
    SkClipStack::BoundsType fFiniteBoundType;
    SkRect                  fFiniteBound;
    bool                    fIsIntersectionOfRects;

    Rec(int saveCount, const SkRect& rect, SkRegion::Op op, bool doAA) : fRect(rect) {
        fSaveCount = saveCount;
        fOp = op;
        fState = kRect_State;
        fDoAA = doAA;
        // bounding box members are updated in a following updateBound call
    }

    Rec(int saveCount, const SkPath& path, SkRegion::Op op, bool doAA) : fPath(path) {
        fRect.setEmpty();
        fSaveCount = saveCount;
        fOp = op;
        fState = kPath_State;
        fDoAA = doAA;
        // bounding box members are updated in a following updateBound call
    }

    bool operator==(const Rec& b) const {
        if (fSaveCount != b.fSaveCount || fOp != b.fOp || fState != b.fState ||
                fDoAA != b.fDoAA) {
            return false;
        }
        switch (fState) {
            case kEmpty_State:
                return true;
            case kRect_State:
                return fRect == b.fRect;
            case kPath_State:
                return fPath == b.fPath;
        }
        return false;  // Silence the compiler.
    }

    bool operator!=(const Rec& b) const {
        return !(*this == b);
    }


    /**
     *  Returns true if this Rec can be intersected in place with a new clip
     */
    bool canBeIntersected(int saveCount, SkRegion::Op op) const {
        if (kEmpty_State == fState && (
                    SkRegion::kDifference_Op == op ||
                    SkRegion::kIntersect_Op == op)) {
            return true;
        }
        return  fSaveCount == saveCount &&
                SkRegion::kIntersect_Op == fOp &&
                SkRegion::kIntersect_Op == op;
    }

    /**
     * The different combination of fill & inverse fill when combining
     * bounding boxes
     */ 
    enum FillCombo {
        kPrev_Cur_FillCombo,
        kPrev_InvCur_FillCombo,
        kInvPrev_Cur_FillCombo,
        kInvPrev_InvCur_FillCombo
    };

    // a mirror of CombineBoundsRevDiff
    void CombineBoundsDiff(FillCombo combination, const SkRect& prevFinite) {
        switch (combination) {
            case kInvPrev_InvCur_FillCombo:
                // In this case the only pixels that can remain set
                // are inside the current clip rect since the extensions
                // to infinity of both clips cancel out and whatever
                // is outside of the current clip is removed
                fFiniteBoundType = kNormal_BoundsType;
                break;
            case kInvPrev_Cur_FillCombo:
                // In this case the current op is finite so the only pixels
                // that aren't set are whatever isn't set in the previous
                // clip and whatever this clip carves out
                fFiniteBound.join(prevFinite);
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kPrev_InvCur_FillCombo:
                // In this case everything outside of this clip's bound
                // is erased, so the only pixels that can remain set 
                // occur w/in the intersection of the two finite bounds
                if (!fFiniteBound.intersect(prevFinite)) {
                    fFiniteBound.setEmpty();
                }
                fFiniteBoundType = kNormal_BoundsType;
                break;
            case kPrev_Cur_FillCombo:
                // The most conservative result bound is that of the 
                // prior clip. This could be wildly incorrect if the 
                // second clip either exactly matches the first clip 
                // (which should yield the empty set) or reduces the
                // size of the prior bound (e.g., if the second clip 
                // exactly matched the bottom half of the prior clip).
                // We ignore these two possibilities.
                fFiniteBound = prevFinite;
                break;
            default:
                SkDEBUGFAIL("SkClipStack::Rec::CombineBoundsDiff Invalid fill combination");
                break;
        }
    }

    void CombineBoundsXOR(int combination, const SkRect& prevFinite) {

        switch (combination) {
            case kInvPrev_Cur_FillCombo:       // fall through
            case kPrev_InvCur_FillCombo:
                // With only one of the clips inverted the result will always
                // extend to infinity. The only pixels that may be un-writeable
                // lie within the union of the two finite bounds
                fFiniteBound.join(prevFinite);
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kInvPrev_InvCur_FillCombo:
                // The only pixels that can survive are within the 
                // union of the two bounding boxes since the extensions
                // to infinity of both clips cancel out
                // fall through!
            case kPrev_Cur_FillCombo:
                // The most conservative bound for xor is the 
                // union of the two bounds. If the two clips exactly overlapped
                // the xor could yield the empty set. Similarly the xor 
                // could reduce the size of the original clip's bound (e.g., 
                // if the second clip exactly matched the bottom half of the 
                // first clip). We ignore these two cases.
                fFiniteBound.join(prevFinite);
                fFiniteBoundType = kNormal_BoundsType;
                break;
            default:
                SkDEBUGFAIL("SkClipStack::Rec::CombineBoundsXOR Invalid fill combination");
                break;
        }
    }

    // a mirror of CombineBoundsIntersection
    void CombineBoundsUnion(int combination, const SkRect& prevFinite) {

        switch (combination) {
            case kInvPrev_InvCur_FillCombo:
                if (!fFiniteBound.intersect(prevFinite)) {
                    fFiniteBound.setEmpty();
                }
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kInvPrev_Cur_FillCombo:
                // The only pixels that won't be drawable are inside
                // the prior clip's finite bound
                fFiniteBound = prevFinite;
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kPrev_InvCur_FillCombo:
                // The only pixels that won't be drawable are inside
                // this clip's finite bound
                break;
            case kPrev_Cur_FillCombo:
                fFiniteBound.join(prevFinite);
                break;
            default:
                SkDEBUGFAIL("SkClipStack::Rec::CombineBoundsUnion Invalid fill combination");
                break;
        }
    }

    // a mirror of CombineBoundsUnion
    void CombineBoundsIntersection(int combination, const SkRect& prevFinite) {

        switch (combination) {
            case kInvPrev_InvCur_FillCombo:
                // The only pixels that aren't writable in this case 
                // occur in the union of the two finite bounds
                fFiniteBound.join(prevFinite);
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kInvPrev_Cur_FillCombo:      
                // In this case the only pixels that will remain writeable
                // are within the current clip
                break;
            case kPrev_InvCur_FillCombo:
                // In this case the only pixels that will remain writeable
                // are with the previous clip
                fFiniteBound = prevFinite;
                fFiniteBoundType = kNormal_BoundsType;
                break;
            case kPrev_Cur_FillCombo:
                if (!fFiniteBound.intersect(prevFinite)) {
                    fFiniteBound.setEmpty();
                }
                break;
            default:
                SkDEBUGFAIL("SkClipStack::Rec::CombineBoundsIntersection Invalid fill combination");
                break;
        }
    }

    // a mirror of CombineBoundsDiff
    void CombineBoundsRevDiff(int combination, const SkRect& prevFinite) {

        switch (combination) {
            case kInvPrev_InvCur_FillCombo:
                // The only pixels that can survive are in the 
                // previous bound since the extensions to infinity in
                // both clips cancel out
                fFiniteBound = prevFinite;
                fFiniteBoundType = kNormal_BoundsType;
                break;
            case kInvPrev_Cur_FillCombo:
                if (!fFiniteBound.intersect(prevFinite)) {
                    fFiniteBound.setEmpty();
                }
                fFiniteBoundType = kNormal_BoundsType;
                break;
            case kPrev_InvCur_FillCombo:
                fFiniteBound.join(prevFinite);
                fFiniteBoundType = kInsideOut_BoundsType;
                break;
            case kPrev_Cur_FillCombo:
                // Fall through - as with the kDifference_Op case, the 
                // most conservative result bound is the bound of the
                // current clip. The prior clip could reduce the size of this 
                // bound (as in the kDifference_Op case) but we are ignoring 
                // those cases.
                break;
            default:
                SkDEBUGFAIL("SkClipStack::Rec::CombineBoundsRevDiff Invalid fill combination");
                break;
        }
    }

    void updateBound(const Rec* prior) {

        // First, optimistically update the current Rec's bound information 
        // with the current clip's bound
        fIsIntersectionOfRects = false;
        if (kRect_State == fState) {
            fFiniteBound = fRect;
            fFiniteBoundType = kNormal_BoundsType;

            if (SkRegion::kReplace_Op == fOp ||
                (SkRegion::kIntersect_Op == fOp && NULL == prior) || 
                (SkRegion::kIntersect_Op == fOp && prior->fIsIntersectionOfRects)) {
                fIsIntersectionOfRects = true;
            }

        } else {
            fFiniteBound = fPath.getBounds();

            if (fPath.isInverseFillType()) {
                fFiniteBoundType = kInsideOut_BoundsType;
            } else {
                fFiniteBoundType = kNormal_BoundsType;
            }
        }

        if (!fDoAA) {
            // Here we mimic a non-anti-aliased scanline system. If there is
            // no anti-aliasing we can integerize the bounding box to exclude
            // fractional parts that won't be rendered.
            // Note: the left edge is handled slightly differently below. We
            // are a bit more generous in the rounding since we don't want to
            // risk missing the left pixels when fLeft is very close to .5
            fFiniteBound.set(SkIntToScalar(SkScalarFloorToInt(fFiniteBound.fLeft+0.45f)), 
                             SkIntToScalar(SkScalarRound(fFiniteBound.fTop)), 
                             SkIntToScalar(SkScalarRound(fFiniteBound.fRight)), 
                             SkIntToScalar(SkScalarRound(fFiniteBound.fBottom)));
        }

        // Now set up the previous Rec's bound information taking into
        // account that there may be no previous clip
        SkRect prevFinite;
        SkClipStack::BoundsType prevType;

        if (NULL == prior) {
            // no prior clip means the entire plane is writable
            prevFinite.setEmpty();   // there are no pixels that cannot be drawn to
            prevType = kInsideOut_BoundsType;
        } else {
            prevFinite = prior->fFiniteBound;
            prevType = prior->fFiniteBoundType;
        }

        FillCombo combination = kPrev_Cur_FillCombo;
        if (kInsideOut_BoundsType == fFiniteBoundType) {
            combination = (FillCombo) (combination | 0x01);
        }
        if (kInsideOut_BoundsType == prevType) {
            combination = (FillCombo) (combination | 0x02);
        }

        SkASSERT(kInvPrev_InvCur_FillCombo == combination || 
                 kInvPrev_Cur_FillCombo == combination ||
                 kPrev_InvCur_FillCombo == combination || 
                 kPrev_Cur_FillCombo == combination);

        // Now integrate with clip with the prior clips
        switch (fOp) {
            case SkRegion::kDifference_Op:
                this->CombineBoundsDiff(combination, prevFinite);
                break;
            case SkRegion::kXOR_Op:
                this->CombineBoundsXOR(combination, prevFinite);
                break;
            case SkRegion::kUnion_Op:
                this->CombineBoundsUnion(combination, prevFinite);
                break;
            case SkRegion::kIntersect_Op:
                this->CombineBoundsIntersection(combination, prevFinite);
                break;
            case SkRegion::kReverseDifference_Op:
                this->CombineBoundsRevDiff(combination, prevFinite);
                break;
            case SkRegion::kReplace_Op:
                // Replace just ignores everything prior
                // The current clip's bound information is already filled in
                // so nothing to do
                break;
            default:
                SkDebugf("SkRegion::Op error/n");
                SkASSERT(0);
                break;
        }
    }
};

SkClipStack::SkClipStack() : fDeque(sizeof(Rec)) {
    fSaveCount = 0;
}

SkClipStack::SkClipStack(const SkClipStack& b) : fDeque(sizeof(Rec)) {
    *this = b;
}

SkClipStack::~SkClipStack() {
    reset();
}

SkClipStack& SkClipStack::operator=(const SkClipStack& b) {
    if (this == &b) {
        return *this;
    }
    reset();

    fSaveCount = b.fSaveCount;
    SkDeque::F2BIter recIter(b.fDeque);
    for (const Rec* rec = (const Rec*)recIter.next();
         rec != NULL;
         rec = (const Rec*)recIter.next()) {
        new (fDeque.push_back()) Rec(*rec);
    }

    return *this;
}

bool SkClipStack::operator==(const SkClipStack& b) const {
    if (fSaveCount != b.fSaveCount || fDeque.count() != b.fDeque.count()) {
        return false;
    }
    SkDeque::F2BIter myIter(fDeque);
    SkDeque::F2BIter bIter(b.fDeque);
    const Rec* myRec = (const Rec*)myIter.next();
    const Rec* bRec = (const Rec*)bIter.next();

    while (myRec != NULL && bRec != NULL) {
        if (*myRec != *bRec) {
            return false;
        }
        myRec = (const Rec*)myIter.next();
        bRec = (const Rec*)bIter.next();
    }
    return myRec == NULL && bRec == NULL;
}

void SkClipStack::reset() {
    // We used a placement new for each object in fDeque, so we're responsible
    // for calling the destructor on each of them as well.
    while (!fDeque.empty()) {
        Rec* rec = (Rec*)fDeque.back();
        rec->~Rec();
        fDeque.pop_back();
    }

    fSaveCount = 0;
}

void SkClipStack::save() {
    fSaveCount += 1;
}

void SkClipStack::restore() {
    fSaveCount -= 1;
    while (!fDeque.empty()) {
        Rec* rec = (Rec*)fDeque.back();
        if (rec->fSaveCount <= fSaveCount) {
            break;
        }
        rec->~Rec();
        fDeque.pop_back();
    }
}

void SkClipStack::getBounds(SkRect* finiteBound, 
                            BoundsType* boundType,
                            bool* isIntersectionOfRects) const {
    SkASSERT(NULL != finiteBound && NULL != boundType);

    Rec* rec = (Rec*)fDeque.back();

    if (NULL == rec) {
        // the clip is wide open - the infinite plane w/ no pixels un-writeable
        finiteBound->setEmpty();
        *boundType = kInsideOut_BoundsType;
        if (NULL != isIntersectionOfRects) {
            *isIntersectionOfRects = false;
        }
        return;
    }

    *finiteBound = rec->fFiniteBound;
    *boundType = rec->fFiniteBoundType;
    if (NULL != isIntersectionOfRects) {
        *isIntersectionOfRects = rec->fIsIntersectionOfRects;
    }
}

void SkClipStack::clipDevRect(const SkRect& rect, SkRegion::Op op, bool doAA) {

    SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
    Rec* rec = (Rec*) iter.prev();

    if (rec && rec->canBeIntersected(fSaveCount, op)) {
        switch (rec->fState) {
            case Rec::kEmpty_State:
                SkASSERT(rec->fFiniteBound.isEmpty());
                SkASSERT(kNormal_BoundsType == rec->fFiniteBoundType);
                SkASSERT(!rec->fIsIntersectionOfRects);
                return;
            case Rec::kRect_State: {
                if (!rec->fRect.intersect(rect)) {
                    rec->fState = Rec::kEmpty_State;
                    rec->fFiniteBound.setEmpty();
                    rec->fFiniteBoundType = kNormal_BoundsType;
                    rec->fIsIntersectionOfRects = false;
                    return;
                }

                Rec* prev = (Rec*) iter.prev();
                rec->updateBound(prev);
                return;
            }
            case Rec::kPath_State:
                if (!SkRect::Intersects(rec->fPath.getBounds(), rect)) {
                    rec->fState = Rec::kEmpty_State;
                    rec->fFiniteBound.setEmpty();
                    rec->fFiniteBoundType = kNormal_BoundsType;
                    rec->fIsIntersectionOfRects = false;
                    return;
                }
                break;
        }
    }
    new (fDeque.push_back()) Rec(fSaveCount, rect, op, doAA);
    ((Rec*) fDeque.back())->updateBound(rec);
}

void SkClipStack::clipDevPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    SkRect alt;
    if (path.isRect(&alt)) {
        return this->clipDevRect(alt, op, doAA);
    }
    Rec* rec = (Rec*)fDeque.back();
    if (rec && rec->canBeIntersected(fSaveCount, op)) {
        const SkRect& pathBounds = path.getBounds();
        switch (rec->fState) {
            case Rec::kEmpty_State:
                SkASSERT(rec->fFiniteBound.isEmpty());
                SkASSERT(kNormal_BoundsType == rec->fFiniteBoundType);
                SkASSERT(!rec->fIsIntersectionOfRects);
                return;
            case Rec::kRect_State:
                if (!SkRect::Intersects(rec->fRect, pathBounds)) {
                    rec->fState = Rec::kEmpty_State;
                    rec->fFiniteBound.setEmpty();
                    rec->fFiniteBoundType = kNormal_BoundsType;
                    rec->fIsIntersectionOfRects = false;
                    return;
                }
                break;
            case Rec::kPath_State:
                if (!SkRect::Intersects(rec->fPath.getBounds(), pathBounds)) {
                    rec->fState = Rec::kEmpty_State;
                    rec->fFiniteBound.setEmpty();
                    rec->fFiniteBoundType = kNormal_BoundsType;
                    rec->fIsIntersectionOfRects = false;
                    return;
                }
                break;
        }
    }
    new (fDeque.push_back()) Rec(fSaveCount, path, op, doAA);
    ((Rec*) fDeque.back())->updateBound(rec);
}

///////////////////////////////////////////////////////////////////////////////

SkClipStack::Iter::Iter() : fStack(NULL) {
}

bool operator==(const SkClipStack::Iter::Clip& a,
                const SkClipStack::Iter::Clip& b) {
    return a.fOp == b.fOp && a.fDoAA == b.fDoAA &&
           ((a.fRect == NULL && b.fRect == NULL) ||
               (a.fRect != NULL && b.fRect != NULL && *a.fRect == *b.fRect)) &&
           ((a.fPath == NULL && b.fPath == NULL) ||
               (a.fPath != NULL && b.fPath != NULL && *a.fPath == *b.fPath));
}

bool operator!=(const SkClipStack::Iter::Clip& a,
                const SkClipStack::Iter::Clip& b) {
    return !(a == b);
}

SkClipStack::Iter::Iter(const SkClipStack& stack, IterStart startLoc)
    : fStack(&stack) {
    this->reset(stack, startLoc);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::updateClip(
                        const SkClipStack::Rec* rec) {
    switch (rec->fState) {
        case SkClipStack::Rec::kEmpty_State:
            fClip.fRect = NULL;
            fClip.fPath = NULL;
            break;
        case SkClipStack::Rec::kRect_State:
            fClip.fRect = &rec->fRect;
            fClip.fPath = NULL;
            break;
        case SkClipStack::Rec::kPath_State:
            fClip.fRect = NULL;
            fClip.fPath = &rec->fPath;
            break;
    }
    fClip.fOp = rec->fOp;
    fClip.fDoAA = rec->fDoAA;
    return &fClip;
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::next() {
    const SkClipStack::Rec* rec = (const SkClipStack::Rec*)fIter.next();
    if (NULL == rec) {
        return NULL;
    }

    return this->updateClip(rec);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::prev() {
    const SkClipStack::Rec* rec = (const SkClipStack::Rec*)fIter.prev();
    if (NULL == rec) {
        return NULL;
    }

    return this->updateClip(rec);
}

const SkClipStack::Iter::Clip* SkClipStack::Iter::skipToTopmost(SkRegion::Op op) {

    if (NULL == fStack) {
        return NULL;
    }

    fIter.reset(fStack->fDeque, SkDeque::Iter::kBack_IterStart);

    const SkClipStack::Rec* rec = NULL;

    for (rec = (const SkClipStack::Rec*) fIter.prev();
         NULL != rec;
         rec = (const SkClipStack::Rec*) fIter.prev()) {

        if (op == rec->fOp) {
            // The Deque's iterator is actually one pace ahead of the
            // returned value. So while "rec" is the element we want to 
            // return, the iterator is actually pointing at (and will
            // return on the next "next" or "prev" call) the element
            // in front of it in the deque. Bump the iterator forward a 
            // step so we get the expected result.
            if (NULL == fIter.next()) {
                // The reverse iterator has run off the front of the deque
                // (i.e., the "op" clip is the first clip) and can't
                // recover. Reset the iterator to start at the front.
                fIter.reset(fStack->fDeque, SkDeque::Iter::kFront_IterStart);
            }
            break;
        }
    }

    if (NULL == rec) {
        // There were no "op" clips
        fIter.reset(fStack->fDeque, SkDeque::Iter::kFront_IterStart);
    }

    return this->next();
}

void SkClipStack::Iter::reset(const SkClipStack& stack, IterStart startLoc) {
    fStack = &stack;
    fIter.reset(stack.fDeque, static_cast<SkDeque::Iter::IterStart>(startLoc));
}

// helper method
void SkClipStack::getConservativeBounds(int offsetX,
                                        int offsetY,
                                        int maxWidth,
                                        int maxHeight,
                                        SkRect* bounds,
                                        bool* isIntersectionOfRects) const {
    SkASSERT(NULL != bounds);

    bounds->setLTRB(0, 0, 
                    SkIntToScalar(maxWidth), SkIntToScalar(maxHeight));

    SkRect temp;
    SkClipStack::BoundsType boundType;
    
    this->getBounds(&temp, &boundType, isIntersectionOfRects);
    if (SkClipStack::kInsideOut_BoundsType == boundType) {
        return;
    }

    temp.offset(SkIntToScalar(offsetX), SkIntToScalar(offsetY));

    if (!bounds->intersect(temp)) {
        bounds->setEmpty();
    }
}
