/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkCanvas.h"
#include "SkClipStack.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkClipOpPriv.h"

#include <new>


// 0-2 are reserved for invalid, empty & wide-open
static const int32_t kFirstUnreservedGenID = 3;
int32_t SkClipStack::gGenID = kFirstUnreservedGenID;

SkClipStack::Element::Element(const Element& that) {
    switch (that.getType()) {
        case kEmpty_Type:
            fRRect.setEmpty();
            fPath.reset();
            break;
        case kRect_Type: // Rect uses rrect
        case kRRect_Type:
            fPath.reset();
            fRRect = that.fRRect;
            break;
        case kPath_Type:
            fPath.set(that.getPath());
            break;
    }

    fSaveCount = that.fSaveCount;
    fOp = that.fOp;
    fType = that.fType;
    fDoAA = that.fDoAA;
    fFiniteBoundType = that.fFiniteBoundType;
    fFiniteBound = that.fFiniteBound;
    fIsIntersectionOfRects = that.fIsIntersectionOfRects;
    fGenID = that.fGenID;
}

bool SkClipStack::Element::operator== (const Element& element) const {
    if (this == &element) {
        return true;
    }
    if (fOp != element.fOp ||
        fType != element.fType ||
        fDoAA != element.fDoAA ||
        fSaveCount != element.fSaveCount) {
        return false;
    }
    switch (fType) {
        case kPath_Type:
            return this->getPath() == element.getPath();
        case kRRect_Type:
            return fRRect == element.fRRect;
        case kRect_Type:
            return this->getRect() == element.getRect();
        case kEmpty_Type:
            return true;
        default:
            SkDEBUGFAIL("Unexpected type.");
            return false;
    }
}

void SkClipStack::Element::invertShapeFillType() {
    switch (fType) {
        case kRect_Type:
            fPath.init();
            fPath.get()->addRect(this->getRect());
            fPath.get()->setFillType(SkPath::kInverseEvenOdd_FillType);
            fType = kPath_Type;
            break;
        case kRRect_Type:
            fPath.init();
            fPath.get()->addRRect(fRRect);
            fPath.get()->setFillType(SkPath::kInverseEvenOdd_FillType);
            fType = kPath_Type;
            break;
        case kPath_Type:
            fPath.get()->toggleInverseFillType();
            break;
        case kEmpty_Type:
            // Should this set to an empty, inverse filled path?
            break;
    }
}

void SkClipStack::Element::initPath(int saveCount, const SkPath& path, SkClipOp op,
                                    bool doAA) {
    if (!path.isInverseFillType()) {
        SkRect r;
        if (path.isRect(&r)) {
            this->initRect(saveCount, r, op, doAA);
            return;
        }
        SkRect ovalRect;
        if (path.isOval(&ovalRect)) {
            SkRRect rrect;
            rrect.setOval(ovalRect);
            this->initRRect(saveCount, rrect, op, doAA);
            return;
        }
    }
    fPath.set(path);
    fPath.get()->setIsVolatile(true);
    fType = kPath_Type;
    this->initCommon(saveCount, op, doAA);
}

void SkClipStack::Element::asPath(SkPath* path) const {
    switch (fType) {
        case kEmpty_Type:
            path->reset();
            path->setIsVolatile(true);
            break;
        case kRect_Type:
            path->reset();
            path->addRect(this->getRect());
            path->setIsVolatile(true);
            break;
        case kRRect_Type:
            path->reset();
            path->addRRect(fRRect);
            path->setIsVolatile(true);
            break;
        case kPath_Type:
            *path = *fPath.get();
            break;
    }
    path->setIsVolatile(true);
}

void SkClipStack::Element::setEmpty() {
    fType = kEmpty_Type;
    fFiniteBound.setEmpty();
    fFiniteBoundType = kNormal_BoundsType;
    fIsIntersectionOfRects = false;
    fRRect.setEmpty();
    fPath.reset();
    fGenID = kEmptyGenID;
    SkDEBUGCODE(this->checkEmpty();)
}

void SkClipStack::Element::checkEmpty() const {
    SkASSERT(fFiniteBound.isEmpty());
    SkASSERT(kNormal_BoundsType == fFiniteBoundType);
    SkASSERT(!fIsIntersectionOfRects);
    SkASSERT(kEmptyGenID == fGenID);
    SkASSERT(fRRect.isEmpty());
    SkASSERT(!fPath.isValid());
}

bool SkClipStack::Element::canBeIntersectedInPlace(int saveCount, SkClipOp op) const {
    if (kEmpty_Type == fType &&
        (kDifference_SkClipOp == op || kIntersect_SkClipOp == op)) {
        return true;
    }
    // Only clips within the same save/restore frame (as captured by
    // the save count) can be merged
    return  fSaveCount == saveCount &&
            kIntersect_SkClipOp == op &&
            (kIntersect_SkClipOp == fOp || kReplace_SkClipOp == fOp);
}

bool SkClipStack::Element::rectRectIntersectAllowed(const SkRect& newR, bool newAA) const {
    SkASSERT(kRect_Type == fType);

    if (fDoAA == newAA) {
        // if the AA setting is the same there is no issue
        return true;
    }

    if (!SkRect::Intersects(this->getRect(), newR)) {
        // The calling code will correctly set the result to the empty clip
        return true;
    }

    if (this->getRect().contains(newR)) {
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
                this->setEmpty();
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
                this->setEmpty();
            } else {
                fFiniteBoundType = kNormal_BoundsType;
            }
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
    switch (fType) {
        case kRect_Type:
            fFiniteBound = this->getRect();
            fFiniteBoundType = kNormal_BoundsType;

            if (kReplace_SkClipOp == fOp ||
                (kIntersect_SkClipOp == fOp && nullptr == prior) ||
                (kIntersect_SkClipOp == fOp && prior->fIsIntersectionOfRects &&
                    prior->rectRectIntersectAllowed(this->getRect(), fDoAA))) {
                fIsIntersectionOfRects = true;
            }
            break;
        case kRRect_Type:
            fFiniteBound = fRRect.getBounds();
            fFiniteBoundType = kNormal_BoundsType;
            break;
        case kPath_Type:
            fFiniteBound = fPath.get()->getBounds();

            if (fPath.get()->isInverseFillType()) {
                fFiniteBoundType = kInsideOut_BoundsType;
            } else {
                fFiniteBoundType = kNormal_BoundsType;
            }
            break;
        case kEmpty_Type:
            SkDEBUGFAIL("We shouldn't get here with an empty element.");
            break;
    }

    if (!fDoAA) {
        fFiniteBound.set(SkScalarFloorToScalar(fFiniteBound.fLeft+0.45f),
                         SkScalarRoundToScalar(fFiniteBound.fTop),
                         SkScalarRoundToScalar(fFiniteBound.fRight),
                         SkScalarRoundToScalar(fFiniteBound.fBottom));
    }

    // Now determine the previous Element's bound information taking into
    // account that there may be no previous clip
    SkRect prevFinite;
    SkClipStack::BoundsType prevType;

    if (nullptr == prior) {
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
        case kDifference_SkClipOp:
            this->combineBoundsDiff(combination, prevFinite);
            break;
        case kXOR_SkClipOp:
            this->combineBoundsXOR(combination, prevFinite);
            break;
        case kUnion_SkClipOp:
            this->combineBoundsUnion(combination, prevFinite);
            break;
        case kIntersect_SkClipOp:
            this->combineBoundsIntersection(combination, prevFinite);
            break;
        case kReverseDifference_SkClipOp:
            this->combineBoundsRevDiff(combination, prevFinite);
            break;
        case kReplace_SkClipOp:
            // Replace just ignores everything prior
            // The current clip's bound information is already filled in
            // so nothing to do
            break;
        default:
            SkDebugf("SkClipOp error\n");
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

SkClipStack::SkClipStack(void* storage, size_t size)
    : fDeque(sizeof(Element), storage, size, kDefaultElementAllocCnt)
    , fSaveCount(0) {
}

SkClipStack::SkClipStack(const SkClipStack& b)
    : fDeque(sizeof(Element), kDefaultElementAllocCnt) {
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
    for (const Element* element = (const Element*)recIter.next();
         element != nullptr;
         element = (const Element*)recIter.next()) {
        new (fDeque.push_back()) Element(*element);
    }

    return *this;
}

bool SkClipStack::operator==(const SkClipStack& b) const {
    if (this->getTopmostGenID() == b.getTopmostGenID()) {
        return true;
    }
    if (fSaveCount != b.fSaveCount ||
        fDeque.count() != b.fDeque.count()) {
        return false;
    }
    SkDeque::F2BIter myIter(fDeque);
    SkDeque::F2BIter bIter(b.fDeque);
    const Element* myElement = (const Element*)myIter.next();
    const Element* bElement = (const Element*)bIter.next();

    while (myElement != nullptr && bElement != nullptr) {
        if (*myElement != *bElement) {
            return false;
        }
        myElement = (const Element*)myIter.next();
        bElement = (const Element*)bIter.next();
    }
    return myElement == nullptr && bElement == nullptr;
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
        element->~Element();
        fDeque.pop_back();
    }
}

SkRect SkClipStack::bounds(const SkIRect& deviceBounds) const {
    // TODO: optimize this.
    SkRect r;
    SkClipStack::BoundsType bounds;
    this->getBounds(&r, &bounds);
    if (bounds == SkClipStack::kInsideOut_BoundsType) {
        return SkRect::Make(deviceBounds);
    }
    return r.intersect(SkRect::Make(deviceBounds)) ? r : SkRect::MakeEmpty();
}

// TODO: optimize this.
bool SkClipStack::isEmpty(const SkIRect& r) const { return this->bounds(r).isEmpty(); }

void SkClipStack::getBounds(SkRect* canvFiniteBound,
                            BoundsType* boundType,
                            bool* isIntersectionOfRects) const {
    SkASSERT(canvFiniteBound && boundType);

    Element* element = (Element*)fDeque.back();

    if (nullptr == element) {
        // the clip is wide open - the infinite plane w/ no pixels un-writeable
        canvFiniteBound->setEmpty();
        *boundType = kInsideOut_BoundsType;
        if (isIntersectionOfRects) {
            *isIntersectionOfRects = false;
        }
        return;
    }

    *canvFiniteBound = element->fFiniteBound;
    *boundType = element->fFiniteBoundType;
    if (isIntersectionOfRects) {
        *isIntersectionOfRects = element->fIsIntersectionOfRects;
    }
}

bool SkClipStack::internalQuickContains(const SkRect& rect) const {

    Iter iter(*this, Iter::kTop_IterStart);
    const Element* element = iter.prev();
    while (element != nullptr) {
        if (kIntersect_SkClipOp != element->getOp() && kReplace_SkClipOp != element->getOp())
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
        if (kReplace_SkClipOp == element->getOp()) {
            break;
        }
        element = iter.prev();
    }
    return true;
}

bool SkClipStack::internalQuickContains(const SkRRect& rrect) const {

    Iter iter(*this, Iter::kTop_IterStart);
    const Element* element = iter.prev();
    while (element != nullptr) {
        if (kIntersect_SkClipOp != element->getOp() && kReplace_SkClipOp != element->getOp())
            return false;
        if (element->isInverseFilled()) {
            // Part of 'rrect' could be trimmed off by the inverse-filled clip element
            if (SkRect::Intersects(element->getBounds(), rrect.getBounds())) {
                return false;
            }
        } else {
            if (!element->contains(rrect)) {
                return false;
            }
        }
        if (kReplace_SkClipOp == element->getOp()) {
            break;
        }
        element = iter.prev();
    }
    return true;
}

bool SkClipStack::asPath(SkPath *path) const {
    bool isAA = false;

    path->reset();
    path->setFillType(SkPath::kInverseEvenOdd_FillType);

    SkClipStack::Iter iter(*this, SkClipStack::Iter::kBottom_IterStart);
    while (const SkClipStack::Element* element = iter.next()) {
        SkPath operand;
        if (element->getType() != SkClipStack::Element::kEmpty_Type) {
            element->asPath(&operand);
        }

        SkClipOp elementOp = element->getOp();
        if (elementOp == kReplace_SkClipOp) {
            *path = operand;
        } else {
            Op(*path, operand, (SkPathOp)elementOp, path);
        }

        // if the prev and curr clips disagree about aa -vs- not, favor the aa request.
        // perhaps we need an API change to avoid this sort of mixed-signals about
        // clipping.
        isAA = (isAA || element->isAA());
    }

    return isAA;
}

void SkClipStack::pushElement(const Element& element) {
    // Use reverse iterator instead of back because Rect path may need previous
    SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
    Element* prior = (Element*) iter.prev();

    if (prior) {
        if (prior->canBeIntersectedInPlace(fSaveCount, element.getOp())) {
            switch (prior->fType) {
                case Element::kEmpty_Type:
                    SkDEBUGCODE(prior->checkEmpty();)
                    return;
                case Element::kRect_Type:
                    if (Element::kRect_Type == element.getType()) {
                        if (prior->rectRectIntersectAllowed(element.getRect(), element.isAA())) {
                            SkRect isectRect;
                            if (!isectRect.intersect(prior->getRect(), element.getRect())) {
                                prior->setEmpty();
                                return;
                            }

                            prior->fRRect.setRect(isectRect);
                            prior->fDoAA = element.isAA();
                            Element* priorPrior = (Element*) iter.prev();
                            prior->updateBoundAndGenID(priorPrior);
                            return;
                        }
                        break;
                    }
                    // fallthrough
                default:
                    if (!SkRect::Intersects(prior->getBounds(), element.getBounds())) {
                        prior->setEmpty();
                        return;
                    }
                    break;
            }
        } else if (kReplace_SkClipOp == element.getOp()) {
            this->restoreTo(fSaveCount - 1);
            prior = (Element*) fDeque.back();
        }
    }
    Element* newElement = new (fDeque.push_back()) Element(element);
    newElement->updateBoundAndGenID(prior);
}

void SkClipStack::clipRRect(const SkRRect& rrect, const SkMatrix& matrix, SkClipOp op,
                            bool doAA) {
    SkRRect transformedRRect;
    if (rrect.transform(matrix, &transformedRRect)) {
        Element element(fSaveCount, transformedRRect, op, doAA);
        this->pushElement(element);
        if (this->hasClipRestriction(op)) {
            Element element(fSaveCount, fClipRestrictionRect, kIntersect_SkClipOp, false);
            this->pushElement(element);
        }
        return;
    }
    SkPath path;
    path.addRRect(rrect);
    path.setIsVolatile(true);
    this->clipPath(path, matrix, op, doAA);
}

void SkClipStack::clipRect(const SkRect& rect, const SkMatrix& matrix, SkClipOp op,
                           bool doAA) {
    if (matrix.rectStaysRect()) {
        SkRect devRect;
        matrix.mapRect(&devRect, rect);
        if (this->hasClipRestriction(op)) {
            if (!devRect.intersect(fClipRestrictionRect)) {
                devRect.setEmpty();
            }
        }
        Element element(fSaveCount, devRect, op, doAA);
        this->pushElement(element);
        return;
    }
    SkPath path;
    path.addRect(rect);
    path.setIsVolatile(true);
    this->clipPath(path, matrix, op, doAA);
}

void SkClipStack::clipPath(const SkPath& path, const SkMatrix& matrix, SkClipOp op,
                           bool doAA) {
    SkPath devPath;
    path.transform(matrix, &devPath);
    Element element(fSaveCount, devPath, op, doAA);
    this->pushElement(element);
    if (this->hasClipRestriction(op)) {
        Element element(fSaveCount, fClipRestrictionRect, kIntersect_SkClipOp, false);
        this->pushElement(element);
    }
}

void SkClipStack::clipEmpty() {
    Element* element = (Element*) fDeque.back();

    if (element && element->canBeIntersectedInPlace(fSaveCount, kIntersect_SkClipOp)) {
        element->setEmpty();
    }
    new (fDeque.push_back()) Element(fSaveCount);

    ((Element*)fDeque.back())->fGenID = kEmptyGenID;
}

///////////////////////////////////////////////////////////////////////////////

SkClipStack::Iter::Iter() : fStack(nullptr) {
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

const SkClipStack::Element* SkClipStack::Iter::skipToTopmost(SkClipOp op) {

    if (nullptr == fStack) {
        return nullptr;
    }

    fIter.reset(fStack->fDeque, SkDeque::Iter::kBack_IterStart);

    const SkClipStack::Element* element = nullptr;

    for (element = (const SkClipStack::Element*) fIter.prev();
         element;
         element = (const SkClipStack::Element*) fIter.prev()) {

        if (op == element->fOp) {
            // The Deque's iterator is actually one pace ahead of the
            // returned value. So while "element" is the element we want to
            // return, the iterator is actually pointing at (and will
            // return on the next "next" or "prev" call) the element
            // in front of it in the deque. Bump the iterator forward a
            // step so we get the expected result.
            if (nullptr == fIter.next()) {
                // The reverse iterator has run off the front of the deque
                // (i.e., the "op" clip is the first clip) and can't
                // recover. Reset the iterator to start at the front.
                fIter.reset(fStack->fDeque, SkDeque::Iter::kFront_IterStart);
            }
            break;
        }
    }

    if (nullptr == element) {
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
    SkASSERT(devBounds);

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

bool SkClipStack::isRRect(const SkRect& bounds, SkRRect* rrect, bool* aa) const {
    // We limit to 5 elements. This means the back element will be bounds checked at most 4 times if
    // it is an rrect.
    int cnt = fDeque.count();
    if (!cnt || cnt > 5) {
        return false;
    }
    const Element* back = static_cast<const Element*>(fDeque.back());
    if (back->getType() != SkClipStack::Element::kRect_Type &&
        back->getType() != SkClipStack::Element::kRRect_Type) {
        return false;
    }
    if (back->getOp() == kReplace_SkClipOp) {
        *rrect = back->asRRect();
        *aa = back->isAA();
        return true;
    }

    if (back->getOp() == kIntersect_SkClipOp) {
        SkRect backBounds;
        if (!backBounds.intersect(bounds, back->asRRect().rect())) {
            return false;
        }
        if (cnt > 1) {
            SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
            SkAssertResult(static_cast<const Element*>(iter.prev()) == back);
            while (const Element* prior = (const Element*)iter.prev()) {
                if ((prior->getOp() != kIntersect_SkClipOp &&
                     prior->getOp() != kReplace_SkClipOp) ||
                    !prior->contains(backBounds)) {
                    return false;
                }
                if (prior->getOp() == kReplace_SkClipOp) {
                    break;
                }
            }
        }
        *rrect = back->asRRect();
        *aa = back->isAA();
        return true;
    }
    return false;
}

uint32_t SkClipStack::GetNextGenID() {
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gGenID));
    } while (id < kFirstUnreservedGenID);
    return id;
}

uint32_t SkClipStack::getTopmostGenID() const {
    if (fDeque.empty()) {
        return kWideOpenGenID;
    }

    const Element* back = static_cast<const Element*>(fDeque.back());
    if (kInsideOut_BoundsType == back->fFiniteBoundType && back->fFiniteBound.isEmpty()) {
        return kWideOpenGenID;
    }

    return back->getGenID();
}

#ifdef SK_DEBUG
void SkClipStack::Element::dump() const {
    static const char* kTypeStrings[] = {
        "empty",
        "rect",
        "rrect",
        "path"
    };
    static_assert(0 == kEmpty_Type, "type_str");
    static_assert(1 == kRect_Type, "type_str");
    static_assert(2 == kRRect_Type, "type_str");
    static_assert(3 == kPath_Type, "type_str");
    static_assert(SK_ARRAY_COUNT(kTypeStrings) == kTypeCnt, "type_str");

    static const char* kOpStrings[] = {
        "difference",
        "intersect",
        "union",
        "xor",
        "reverse-difference",
        "replace",
    };
    static_assert(0 == static_cast<int>(kDifference_SkClipOp), "op_str");
    static_assert(1 == static_cast<int>(kIntersect_SkClipOp), "op_str");
    static_assert(2 == static_cast<int>(kUnion_SkClipOp), "op_str");
    static_assert(3 == static_cast<int>(kXOR_SkClipOp), "op_str");
    static_assert(4 == static_cast<int>(kReverseDifference_SkClipOp), "op_str");
    static_assert(5 == static_cast<int>(kReplace_SkClipOp), "op_str");
    static_assert(SK_ARRAY_COUNT(kOpStrings) == SkRegion::kOpCnt, "op_str");

    SkDebugf("Type: %s, Op: %s, AA: %s, Save Count: %d\n", kTypeStrings[fType],
             kOpStrings[static_cast<int>(fOp)], (fDoAA ? "yes" : "no"), fSaveCount);
    switch (fType) {
        case kEmpty_Type:
            SkDebugf("\n");
            break;
        case kRect_Type:
            this->getRect().dump();
            SkDebugf("\n");
            break;
        case kRRect_Type:
            this->getRRect().dump();
            SkDebugf("\n");
            break;
        case kPath_Type:
            this->getPath().dump(nullptr, true, false);
            break;
    }
}

void SkClipStack::dump() const {
    B2TIter iter(*this);
    const Element* e;
    while ((e = iter.next())) {
        e->dump();
        SkDebugf("\n");
    }
}
#endif
