
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkClipStack.h"
#include "SkPath.h"
#include "SkThread.h"

#include <new>


// 0-2 are reserved for invalid, empty & wide-open
static const int32_t kFirstUnreservedGenID = 3;
int32_t SkClipStack::gGenID = kFirstUnreservedGenID;

void SkClipStack::Element::invertShapeFillType() {
    switch (fType) {
        case kRect_Type:
            fPath.reset();
            fPath.addRect(fRect);
            fPath.setFillType(SkPath::kInverseWinding_FillType);
            fType = kPath_Type;
            break;
        case kPath_Type:
            fPath.toggleInverseFillType();
        case kEmpty_Type:
            break;
    }
}

void SkClipStack::Element::checkEmpty() const {
    SkASSERT(fFiniteBound.isEmpty());
    SkASSERT(kNormal_BoundsType == fFiniteBoundType);
    SkASSERT(!fIsIntersectionOfRects);
    SkASSERT(kEmptyGenID == fGenID);
    SkASSERT(fPath.isEmpty());
}

bool SkClipStack::Element::canBeIntersectedInPlace(int saveCount, SkRegion::Op op) const {
    if (kEmpty_Type == fType &&
        (SkRegion::kDifference_Op == op || SkRegion::kIntersect_Op == op)) {
        return true;
    }
    // Only clips within the same save/restore frame (as captured by
    // the save count) can be merged
    return  fSaveCount == saveCount &&
            SkRegion::kIntersect_Op == op &&
            (SkRegion::kIntersect_Op == fOp || SkRegion::kReplace_Op == fOp);
}

bool SkClipStack::Element::rectRectIntersectAllowed(const SkRect& newR, bool newAA) const {
    SkASSERT(kRect_Type == fType);

    if (fDoAA == newAA) {
        // if the AA setting is the same there is no issue
        return true;
    }

    if (!SkRect::Intersects(fRect, newR)) {
        // The calling code will correctly set the result to the empty clip
        return true;
    }

    if (fRect.contains(newR)) {
        // if the new rect carves out a portion of the old one there is no
        // issue
        return true;
    }

    // So either the two overlap in some complex manner or newR contains oldR.
    // In the first, case the edges will require different AA. In the second,
    // the AA setting that would be carried forward is incorrect (e.g., oldR
    // is AA while newR is BW but since newR contains oldR, oldR will be
    // drawn BW) since the new AA setting will predominate.
    return false;
}

// a mirror of combineBoundsRevDiff
void SkClipStack::Element::combineBoundsDiff(FillCombo combination, const SkRect& prevFinite) {
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
                fGenID = kEmptyGenID;
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
            SkDEBUGFAIL("SkClipStack::Element::combineBoundsDiff Invalid fill combination");
            break;
    }
}

void SkClipStack::Element::combineBoundsXOR(int combination, const SkRect& prevFinite) {

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
            SkDEBUGFAIL("SkClipStack::Element::combineBoundsXOR Invalid fill combination");
            break;
    }
}

// a mirror of combineBoundsIntersection
void SkClipStack::Element::combineBoundsUnion(int combination, const SkRect& prevFinite) {

    switch (combination) {
        case kInvPrev_InvCur_FillCombo:
            if (!fFiniteBound.intersect(prevFinite)) {
                fFiniteBound.setEmpty();
                fGenID = kWideOpenGenID;
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
            SkDEBUGFAIL("SkClipStack::Element::combineBoundsUnion Invalid fill combination");
            break;
    }
}

// a mirror of combineBoundsUnion
void SkClipStack::Element::combineBoundsIntersection(int combination, const SkRect& prevFinite) {

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
                fGenID = kEmptyGenID;
            }
            break;
        default:
            SkDEBUGFAIL("SkClipStack::Element::combineBoundsIntersection Invalid fill combination");
            break;
    }
}

// a mirror of combineBoundsDiff
void SkClipStack::Element::combineBoundsRevDiff(int combination, const SkRect& prevFinite) {

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
                fGenID = kEmptyGenID;
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
            SkDEBUGFAIL("SkClipStack::Element::combineBoundsRevDiff Invalid fill combination");
            break;
    }
}

void SkClipStack::Element::updateBoundAndGenID(const Element* prior) {
    // We set this first here but we may overwrite it later if we determine that the clip is
    // either wide-open or empty.
    fGenID = GetNextGenID();

    // First, optimistically update the current Element's bound information
    // with the current clip's bound
    fIsIntersectionOfRects = false;
    if (kRect_Type == fType) {
        fFiniteBound = fRect;
        fFiniteBoundType = kNormal_BoundsType;

        if (SkRegion::kReplace_Op == fOp ||
            (SkRegion::kIntersect_Op == fOp && NULL == prior) ||
            (SkRegion::kIntersect_Op == fOp && prior->fIsIntersectionOfRects &&
                prior->rectRectIntersectAllowed(fRect, fDoAA))) {
            fIsIntersectionOfRects = true;
        }

    } else {
        SkASSERT(kPath_Type == fType);

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

    // Now determine the previous Element's bound information taking into
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
            this->combineBoundsDiff(combination, prevFinite);
            break;
        case SkRegion::kXOR_Op:
            this->combineBoundsXOR(combination, prevFinite);
            break;
        case SkRegion::kUnion_Op:
            this->combineBoundsUnion(combination, prevFinite);
            break;
        case SkRegion::kIntersect_Op:
            this->combineBoundsIntersection(combination, prevFinite);
            break;
        case SkRegion::kReverseDifference_Op:
            this->combineBoundsRevDiff(combination, prevFinite);
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

// This constant determines how many Element's are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs.
// incurring allocation/deallocation thrashing. It should roughly correspond to
// the deepest save/restore stack we expect to see.
static const int kDefaultElementAllocCnt = 8;

SkClipStack::SkClipStack()
    : fDeque(sizeof(Element), kDefaultElementAllocCnt)
    , fSaveCount(0) {
}

SkClipStack::SkClipStack(const SkClipStack& b)
    : fDeque(sizeof(Element), kDefaultElementAllocCnt) {
    *this = b;
}

SkClipStack::SkClipStack(const SkRect& r)
    : fDeque(sizeof(Element), kDefaultElementAllocCnt)
    , fSaveCount(0) {
    if (!r.isEmpty()) {
        this->clipDevRect(r, SkRegion::kReplace_Op, false);
    }
}

SkClipStack::SkClipStack(const SkIRect& r)
    : fDeque(sizeof(Element), kDefaultElementAllocCnt)
    , fSaveCount(0) {
    if (!r.isEmpty()) {
        SkRect temp;
        temp.set(r);
        this->clipDevRect(temp, SkRegion::kReplace_Op, false);
    }
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
    for (const Element* element = (const Element*)recIter.next();
         element != NULL;
         element = (const Element*)recIter.next()) {
        new (fDeque.push_back()) Element(*element);
    }

    return *this;
}

bool SkClipStack::operator==(const SkClipStack& b) const {
    if (fSaveCount != b.fSaveCount ||
        fDeque.count() != b.fDeque.count()) {
        return false;
    }
    SkDeque::F2BIter myIter(fDeque);
    SkDeque::F2BIter bIter(b.fDeque);
    const Element* myElement = (const Element*)myIter.next();
    const Element* bElement = (const Element*)bIter.next();

    while (myElement != NULL && bElement != NULL) {
        if (*myElement != *bElement) {
            return false;
        }
        myElement = (const Element*)myIter.next();
        bElement = (const Element*)bIter.next();
    }
    return myElement == NULL && bElement == NULL;
}

void SkClipStack::reset() {
    // We used a placement new for each object in fDeque, so we're responsible
    // for calling the destructor on each of them as well.
    while (!fDeque.empty()) {
        Element* element = (Element*)fDeque.back();
        element->~Element();
        fDeque.pop_back();
    }

    fSaveCount = 0;
}

void SkClipStack::save() {
    fSaveCount += 1;
}

void SkClipStack::restore() {
    fSaveCount -= 1;
    restoreTo(fSaveCount);
}

void SkClipStack::restoreTo(int saveCount) {
    while (!fDeque.empty()) {
        Element* element = (Element*)fDeque.back();
        if (element->fSaveCount <= saveCount) {
            break;
        }
        this->purgeClip(element);
        element->~Element();
        fDeque.pop_back();
    }
}

void SkClipStack::getBounds(SkRect* canvFiniteBound,
                            BoundsType* boundType,
                            bool* isIntersectionOfRects) const {
    SkASSERT(NULL != canvFiniteBound && NULL != boundType);

    Element* element = (Element*)fDeque.back();

    if (NULL == element) {
        // the clip is wide open - the infinite plane w/ no pixels un-writeable
        canvFiniteBound->setEmpty();
        *boundType = kInsideOut_BoundsType;
        if (NULL != isIntersectionOfRects) {
            *isIntersectionOfRects = false;
        }
        return;
    }

    *canvFiniteBound = element->fFiniteBound;
    *boundType = element->fFiniteBoundType;
    if (NULL != isIntersectionOfRects) {
        *isIntersectionOfRects = element->fIsIntersectionOfRects;
    }
}

bool SkClipStack::intersectRectWithClip(SkRect* rect) const {
    SkASSERT(NULL != rect);

    SkRect bounds;
    SkClipStack::BoundsType bt;
    this->getBounds(&bounds, &bt);
    if (bt == SkClipStack::kInsideOut_BoundsType) {
        if (bounds.contains(*rect)) {
            return false;
        } else {
            // If rect's x values are both within bound's x range we
            // could clip here. Same for y. But we don't bother to check.
            return true;
        }
    } else {
        return rect->intersect(bounds);
    }
}

bool SkClipStack::quickContains(const SkRect& rect) const {

    Iter iter(*this, Iter::kTop_IterStart);
    const Element* element = iter.prev();
    while (element != NULL) {
        if (SkRegion::kIntersect_Op != element->getOp() && SkRegion::kReplace_Op != element->getOp())
            return false;
        if (element->isInverseFilled()) {
            // Part of 'rect' could be trimmed off by the inverse-filled clip element
            if (SkRect::Intersects(element->getBounds(), rect)) {
                return false;
            }
        } else {
            if (!element->contains(rect)) {
                return false;
            }
        }
        if (SkRegion::kReplace_Op == element->getOp()) {
            break;
        }
        element = iter.prev();
    }
    return true;
}

void SkClipStack::clipDevRect(const SkRect& rect, SkRegion::Op op, bool doAA) {

    // Use reverse iterator instead of back because Rect path may need previous
    SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
    Element* element = (Element*) iter.prev();

    if (NULL != element) {
        if (element->canBeIntersectedInPlace(fSaveCount, op)) {
            switch (element->fType) {
                case Element::kEmpty_Type:
                    element->checkEmpty();
                    return;
                case Element::kRect_Type:
                    if (element->rectRectIntersectAllowed(rect, doAA)) {
                        this->purgeClip(element);
                        if (!element->fRect.intersect(rect)) {
                            element->setEmpty();
                            return;
                        }

                        element->fDoAA = doAA;
                        Element* prev = (Element*) iter.prev();
                        element->updateBoundAndGenID(prev);
                        return;
                    }
                    break;
                case Element::kPath_Type:
                    if (!SkRect::Intersects(element->fPath.getBounds(), rect)) {
                        this->purgeClip(element);
                        element->setEmpty();
                        return;
                    }
                    break;
            }
        } else if (SkRegion::kReplace_Op == op) {
            this->restoreTo(fSaveCount - 1);
            element = (Element*) fDeque.back();
        }
    }
    new (fDeque.push_back()) Element(fSaveCount, rect, op, doAA);
    ((Element*) fDeque.back())->updateBoundAndGenID(element);

    if (element && element->fSaveCount == fSaveCount) {
        this->purgeClip(element);
    }
}

void SkClipStack::clipDevPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    SkRect alt;
    if (path.isRect(&alt) && !path.isInverseFillType()) {
        return this->clipDevRect(alt, op, doAA);
    }

    Element* element = (Element*)fDeque.back();
    if (NULL != element) {
        if (element->canBeIntersectedInPlace(fSaveCount, op)) {
            const SkRect& pathBounds = path.getBounds();
            switch (element->fType) {
                case Element::kEmpty_Type:
                    element->checkEmpty();
                    return;
                case Element::kRect_Type:
                    if (!SkRect::Intersects(element->fRect, pathBounds)) {
                        this->purgeClip(element);
                        element->setEmpty();
                        return;
                    }
                    break;
                case Element::kPath_Type:
                    if (!SkRect::Intersects(element->fPath.getBounds(), pathBounds)) {
                        this->purgeClip(element);
                        element->setEmpty();
                        return;
                    }
                    break;
            }
        } else if (SkRegion::kReplace_Op == op) {
            this->restoreTo(fSaveCount - 1);
            element = (Element*) fDeque.back();
        }
    }
    new (fDeque.push_back()) Element(fSaveCount, path, op, doAA);
    ((Element*) fDeque.back())->updateBoundAndGenID(element);

    if (element && element->fSaveCount == fSaveCount) {
        this->purgeClip(element);
    }
}

void SkClipStack::clipEmpty() {

    Element* element = (Element*) fDeque.back();

    if (element && element->canBeIntersectedInPlace(fSaveCount, SkRegion::kIntersect_Op)) {
        switch (element->fType) {
            case Element::kEmpty_Type:
                element->checkEmpty();
                return;
            case Element::kRect_Type:
            case Element::kPath_Type:
                this->purgeClip(element);
                element->setEmpty();
                return;
        }
    }
    new (fDeque.push_back()) Element(fSaveCount);

    if (element && element->fSaveCount == fSaveCount) {
        this->purgeClip(element);
    }
    ((Element*)fDeque.back())->fGenID = kEmptyGenID;
}

bool SkClipStack::isWideOpen() const {
    if (0 == fDeque.count()) {
        return true;
    }

    const Element* back = (const Element*) fDeque.back();
    return kWideOpenGenID == back->fGenID ||
           (kInsideOut_BoundsType == back->fFiniteBoundType && back->fFiniteBound.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

SkClipStack::Iter::Iter() : fStack(NULL) {
}

SkClipStack::Iter::Iter(const SkClipStack& stack, IterStart startLoc)
    : fStack(&stack) {
    this->reset(stack, startLoc);
}

const SkClipStack::Element* SkClipStack::Iter::next() {
    return (const SkClipStack::Element*)fIter.next();
}

const SkClipStack::Element* SkClipStack::Iter::prev() {
    return (const SkClipStack::Element*)fIter.prev();
}

const SkClipStack::Element* SkClipStack::Iter::skipToTopmost(SkRegion::Op op) {

    if (NULL == fStack) {
        return NULL;
    }

    fIter.reset(fStack->fDeque, SkDeque::Iter::kBack_IterStart);

    const SkClipStack::Element* element = NULL;

    for (element = (const SkClipStack::Element*) fIter.prev();
         NULL != element;
         element = (const SkClipStack::Element*) fIter.prev()) {

        if (op == element->fOp) {
            // The Deque's iterator is actually one pace ahead of the
            // returned value. So while "element" is the element we want to
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

    if (NULL == element) {
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
                                        SkRect* devBounds,
                                        bool* isIntersectionOfRects) const {
    SkASSERT(NULL != devBounds);

    devBounds->setLTRB(0, 0,
                       SkIntToScalar(maxWidth), SkIntToScalar(maxHeight));

    SkRect temp;
    SkClipStack::BoundsType boundType;

    // temp starts off in canvas space here
    this->getBounds(&temp, &boundType, isIntersectionOfRects);
    if (SkClipStack::kInsideOut_BoundsType == boundType) {
        return;
    }

    // but is converted to device space here
    temp.offset(SkIntToScalar(offsetX), SkIntToScalar(offsetY));

    if (!devBounds->intersect(temp)) {
        devBounds->setEmpty();
    }
}

void SkClipStack::addPurgeClipCallback(PFPurgeClipCB callback, void* data) const {
    ClipCallbackData temp = { callback, data };
    fCallbackData.append(1, &temp);
}

void SkClipStack::removePurgeClipCallback(PFPurgeClipCB callback, void* data) const {
    ClipCallbackData temp = { callback, data };
    int index = fCallbackData.find(temp);
    if (index >= 0) {
        fCallbackData.removeShuffle(index);
    }
}

// The clip state represented by 'element' will never be used again. Purge it.
void SkClipStack::purgeClip(Element* element) {
    SkASSERT(NULL != element);
    if (element->fGenID >= 0 && element->fGenID < kFirstUnreservedGenID) {
        return;
    }

    for (int i = 0; i < fCallbackData.count(); ++i) {
        (*fCallbackData[i].fCallback)(element->fGenID, fCallbackData[i].fData);
    }

    // Invalidate element's gen ID so handlers can detect already handled records
    element->fGenID = kInvalidGenID;
}

int32_t SkClipStack::GetNextGenID() {
    // TODO: handle overflow.
    return sk_atomic_inc(&gGenID);
}

int32_t SkClipStack::getTopmostGenID() const {

    if (fDeque.empty()) {
        return kInvalidGenID;
    }

    Element* element = (Element*)fDeque.back();
    return element->fGenID;
}
