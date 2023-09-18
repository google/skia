/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkClipStack.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkRectPriv.h"
#include "src/shaders/SkShaderBase.h"

#include <array>
#include <atomic>
#include <new>

SkClipStack::Element::Element(const Element& that) {
    switch (that.getDeviceSpaceType()) {
        case DeviceSpaceType::kEmpty:
            fDeviceSpaceRRect.setEmpty();
            fDeviceSpacePath.reset();
            fShader.reset();
            break;
        case DeviceSpaceType::kRect:  // Rect uses rrect
        case DeviceSpaceType::kRRect:
            fDeviceSpacePath.reset();
            fShader.reset();
            fDeviceSpaceRRect = that.fDeviceSpaceRRect;
            break;
        case DeviceSpaceType::kPath:
            fShader.reset();
            fDeviceSpacePath.set(that.getDeviceSpacePath());
            break;
        case DeviceSpaceType::kShader:
            fDeviceSpacePath.reset();
            fShader = that.fShader;
            break;
    }

    fSaveCount = that.fSaveCount;
    fOp = that.fOp;
    fDeviceSpaceType = that.fDeviceSpaceType;
    fDoAA = that.fDoAA;
    fIsReplace = that.fIsReplace;
    fFiniteBoundType = that.fFiniteBoundType;
    fFiniteBound = that.fFiniteBound;
    fIsIntersectionOfRects = that.fIsIntersectionOfRects;
    fGenID = that.fGenID;
}

SkClipStack::Element::~Element() = default;

bool SkClipStack::Element::operator== (const Element& element) const {
    if (this == &element) {
        return true;
    }
    if (fOp != element.fOp || fDeviceSpaceType != element.fDeviceSpaceType ||
        fDoAA != element.fDoAA || fIsReplace != element.fIsReplace ||
        fSaveCount != element.fSaveCount) {
        return false;
    }
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kShader:
            return this->getShader() == element.getShader();
        case DeviceSpaceType::kPath:
            return this->getDeviceSpacePath() == element.getDeviceSpacePath();
        case DeviceSpaceType::kRRect:
            return fDeviceSpaceRRect == element.fDeviceSpaceRRect;
        case DeviceSpaceType::kRect:
            return this->getDeviceSpaceRect() == element.getDeviceSpaceRect();
        case DeviceSpaceType::kEmpty:
            return true;
        default:
            SkDEBUGFAIL("Unexpected type.");
            return false;
    }
}

const SkRect& SkClipStack::Element::getBounds() const {
    static const SkRect kEmpty = {0, 0, 0, 0};
    static const SkRect kInfinite = SkRectPriv::MakeLargeS32();
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kRect:  // fallthrough
        case DeviceSpaceType::kRRect:
            return fDeviceSpaceRRect.getBounds();
        case DeviceSpaceType::kPath:
            return fDeviceSpacePath->getBounds();
        case DeviceSpaceType::kShader:
            // Shaders have infinite bounds since any pixel could have clipped or full coverage
            // (which is different from wide-open, where every pixel has 1.0 coverage, or empty
            //  where every pixel has 0.0 coverage).
            return kInfinite;
        case DeviceSpaceType::kEmpty:
            return kEmpty;
        default:
            SkDEBUGFAIL("Unexpected type.");
            return kEmpty;
    }
}

bool SkClipStack::Element::contains(const SkRect& rect) const {
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kRect:
            return this->getDeviceSpaceRect().contains(rect);
        case DeviceSpaceType::kRRect:
            return fDeviceSpaceRRect.contains(rect);
        case DeviceSpaceType::kPath:
            return fDeviceSpacePath->conservativelyContainsRect(rect);
        case DeviceSpaceType::kEmpty:
        case DeviceSpaceType::kShader:
            return false;
        default:
            SkDEBUGFAIL("Unexpected type.");
            return false;
    }
}

bool SkClipStack::Element::contains(const SkRRect& rrect) const {
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kRect:
            return this->getDeviceSpaceRect().contains(rrect.getBounds());
        case DeviceSpaceType::kRRect:
            // We don't currently have a generalized rrect-rrect containment.
            return fDeviceSpaceRRect.contains(rrect.getBounds()) || rrect == fDeviceSpaceRRect;
        case DeviceSpaceType::kPath:
            return fDeviceSpacePath->conservativelyContainsRect(rrect.getBounds());
        case DeviceSpaceType::kEmpty:
        case DeviceSpaceType::kShader:
            return false;
        default:
            SkDEBUGFAIL("Unexpected type.");
            return false;
    }
}

void SkClipStack::Element::invertShapeFillType() {
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kRect:
            fDeviceSpacePath.init();
            fDeviceSpacePath->addRect(this->getDeviceSpaceRect());
            fDeviceSpacePath->setFillType(SkPathFillType::kInverseEvenOdd);
            fDeviceSpaceType = DeviceSpaceType::kPath;
            break;
        case DeviceSpaceType::kRRect:
            fDeviceSpacePath.init();
            fDeviceSpacePath->addRRect(fDeviceSpaceRRect);
            fDeviceSpacePath->setFillType(SkPathFillType::kInverseEvenOdd);
            fDeviceSpaceType = DeviceSpaceType::kPath;
            break;
        case DeviceSpaceType::kPath:
            fDeviceSpacePath->toggleInverseFillType();
            break;
        case DeviceSpaceType::kShader:
            fShader = as_SB(fShader)->makeInvertAlpha();
            break;
        case DeviceSpaceType::kEmpty:
            // Should this set to an empty, inverse filled path?
            break;
    }
}

void SkClipStack::Element::initCommon(int saveCount, SkClipOp op, bool doAA) {
    fSaveCount = saveCount;
    fOp = op;
    fDoAA = doAA;
    fIsReplace = false;
    // A default of inside-out and empty bounds means the bounds are effectively void as it
    // indicates that nothing is known to be outside the clip.
    fFiniteBoundType = kInsideOut_BoundsType;
    fFiniteBound.setEmpty();
    fIsIntersectionOfRects = false;
    fGenID = kInvalidGenID;
}

void SkClipStack::Element::initRect(int saveCount, const SkRect& rect, const SkMatrix& m,
                                    SkClipOp op, bool doAA) {
    if (m.rectStaysRect()) {
        SkRect devRect;
        m.mapRect(&devRect, rect);
        fDeviceSpaceRRect.setRect(devRect);
        fDeviceSpaceType = DeviceSpaceType::kRect;
        this->initCommon(saveCount, op, doAA);
        return;
    }
    SkPath path;
    path.addRect(rect);
    path.setIsVolatile(true);
    this->initAsPath(saveCount, path, m, op, doAA);
}

void SkClipStack::Element::initRRect(int saveCount, const SkRRect& rrect, const SkMatrix& m,
                                     SkClipOp op, bool doAA) {
    if (rrect.transform(m, &fDeviceSpaceRRect)) {
        SkRRect::Type type = fDeviceSpaceRRect.getType();
        if (SkRRect::kRect_Type == type || SkRRect::kEmpty_Type == type) {
            fDeviceSpaceType = DeviceSpaceType::kRect;
        } else {
            fDeviceSpaceType = DeviceSpaceType::kRRect;
        }
        this->initCommon(saveCount, op, doAA);
        return;
    }
    SkPath path;
    path.addRRect(rrect);
    path.setIsVolatile(true);
    this->initAsPath(saveCount, path, m, op, doAA);
}

void SkClipStack::Element::initPath(int saveCount, const SkPath& path, const SkMatrix& m,
                                    SkClipOp op, bool doAA) {
    if (!path.isInverseFillType()) {
        SkRect r;
        if (path.isRect(&r)) {
            this->initRect(saveCount, r, m, op, doAA);
            return;
        }
        SkRect ovalRect;
        if (path.isOval(&ovalRect)) {
            SkRRect rrect;
            rrect.setOval(ovalRect);
            this->initRRect(saveCount, rrect, m, op, doAA);
            return;
        }
    }
    this->initAsPath(saveCount, path, m, op, doAA);
}

void SkClipStack::Element::initAsPath(int saveCount, const SkPath& path, const SkMatrix& m,
                                      SkClipOp op, bool doAA) {
    path.transform(m, fDeviceSpacePath.init());
    fDeviceSpacePath->setIsVolatile(true);
    fDeviceSpaceType = DeviceSpaceType::kPath;
    this->initCommon(saveCount, op, doAA);
}

void SkClipStack::Element::initShader(int saveCount, sk_sp<SkShader> shader) {
    SkASSERT(shader);
    fDeviceSpaceType = DeviceSpaceType::kShader;
    fShader = std::move(shader);
    this->initCommon(saveCount, SkClipOp::kIntersect, false);
}

void SkClipStack::Element::initReplaceRect(int saveCount, const SkRect& rect, bool doAA) {
    fDeviceSpaceRRect.setRect(rect);
    fDeviceSpaceType = DeviceSpaceType::kRect;
    this->initCommon(saveCount, SkClipOp::kIntersect, doAA);
    fIsReplace = true;
}

void SkClipStack::Element::asDeviceSpacePath(SkPath* path) const {
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kEmpty:
            path->reset();
            break;
        case DeviceSpaceType::kRect:
            path->reset();
            path->addRect(this->getDeviceSpaceRect());
            break;
        case DeviceSpaceType::kRRect:
            path->reset();
            path->addRRect(fDeviceSpaceRRect);
            break;
        case DeviceSpaceType::kPath:
            *path = *fDeviceSpacePath;
            break;
        case DeviceSpaceType::kShader:
            path->reset();
            path->addRect(SkRectPriv::MakeLargeS32());
            break;
    }
    path->setIsVolatile(true);
}

void SkClipStack::Element::setEmpty() {
    fDeviceSpaceType = DeviceSpaceType::kEmpty;
    fFiniteBound.setEmpty();
    fFiniteBoundType = kNormal_BoundsType;
    fIsIntersectionOfRects = false;
    fDeviceSpaceRRect.setEmpty();
    fDeviceSpacePath.reset();
    fShader.reset();
    fGenID = kEmptyGenID;
    SkDEBUGCODE(this->checkEmpty();)
}

void SkClipStack::Element::checkEmpty() const {
    SkASSERT(fFiniteBound.isEmpty());
    SkASSERT(kNormal_BoundsType == fFiniteBoundType);
    SkASSERT(!fIsIntersectionOfRects);
    SkASSERT(kEmptyGenID == fGenID);
    SkASSERT(fDeviceSpaceRRect.isEmpty());
    SkASSERT(!fDeviceSpacePath.isValid());
    SkASSERT(!fShader);
}

bool SkClipStack::Element::canBeIntersectedInPlace(int saveCount, SkClipOp op) const {
    if (DeviceSpaceType::kEmpty == fDeviceSpaceType &&
        (SkClipOp::kDifference == op || SkClipOp::kIntersect == op)) {
        return true;
    }
    // Only clips within the same save/restore frame (as captured by
    // the save count) can be merged
    return  fSaveCount == saveCount &&
            SkClipOp::kIntersect == op &&
            (SkClipOp::kIntersect == fOp || this->isReplaceOp());
}

bool SkClipStack::Element::rectRectIntersectAllowed(const SkRect& newR, bool newAA) const {
    SkASSERT(DeviceSpaceType::kRect == fDeviceSpaceType);

    if (fDoAA == newAA) {
        // if the AA setting is the same there is no issue
        return true;
    }

    if (!SkRect::Intersects(this->getDeviceSpaceRect(), newR)) {
        // The calling code will correctly set the result to the empty clip
        return true;
    }

    if (this->getDeviceSpaceRect().contains(newR)) {
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

void SkClipStack::Element::updateBoundAndGenID(const Element* prior) {
    // We set this first here but we may overwrite it later if we determine that the clip is
    // either wide-open or empty.
    fGenID = GetNextGenID();

    // First, optimistically update the current Element's bound information
    // with the current clip's bound
    fIsIntersectionOfRects = false;
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kRect:
            fFiniteBound = this->getDeviceSpaceRect();
            fFiniteBoundType = kNormal_BoundsType;

            if (this->isReplaceOp() ||
                (SkClipOp::kIntersect == fOp && nullptr == prior) ||
                (SkClipOp::kIntersect == fOp && prior->fIsIntersectionOfRects &&
                 prior->rectRectIntersectAllowed(this->getDeviceSpaceRect(), fDoAA))) {
                fIsIntersectionOfRects = true;
            }
            break;
        case DeviceSpaceType::kRRect:
            fFiniteBound = fDeviceSpaceRRect.getBounds();
            fFiniteBoundType = kNormal_BoundsType;
            break;
        case DeviceSpaceType::kPath:
            fFiniteBound = fDeviceSpacePath->getBounds();

            if (fDeviceSpacePath->isInverseFillType()) {
                fFiniteBoundType = kInsideOut_BoundsType;
            } else {
                fFiniteBoundType = kNormal_BoundsType;
            }
            break;
        case DeviceSpaceType::kShader:
            // A shader is infinite. We don't act as wide-open here (which is an empty bounds with
            // the inside out type). This is because when the bounds is empty and inside-out, we
            // know there's full coverage everywhere. With a shader, there's *unknown* coverage
            // everywhere.
            fFiniteBound = SkRectPriv::MakeLargeS32();
            fFiniteBoundType = kNormal_BoundsType;
            break;
        case DeviceSpaceType::kEmpty:
            SkDEBUGFAIL("We shouldn't get here with an empty element.");
            break;
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
    if (!this->isReplaceOp()) {
        switch (fOp) {
            case SkClipOp::kDifference:
                this->combineBoundsDiff(combination, prevFinite);
                break;
            case SkClipOp::kIntersect:
                this->combineBoundsIntersection(combination, prevFinite);
                break;
            default:
                SkDebugf("SkClipOp error\n");
                SkASSERT(0);
                break;
        }
    } // else Replace just ignores everything prior and should already have filled in bounds.
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
        // TODO: Once expanding ops are removed, this condition is equiv. to op == kDifference.
        if (SkClipOp::kIntersect != element->getOp() && !element->isReplaceOp()) {
            return false;
        }
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
        if (element->isReplaceOp()) {
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
        // TODO: Once expanding ops are removed, this condition is equiv. to op == kDifference.
        if (SkClipOp::kIntersect != element->getOp() && !element->isReplaceOp()) {
            return false;
        }
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
        if (element->isReplaceOp()) {
            break;
        }
        element = iter.prev();
    }
    return true;
}

void SkClipStack::pushElement(const Element& element) {
    // Use reverse iterator instead of back because Rect path may need previous
    SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
    Element* prior = (Element*) iter.prev();

    if (prior) {
        if (element.isReplaceOp()) {
            this->restoreTo(fSaveCount - 1);
            prior = (Element*) fDeque.back();
        } else if (prior->canBeIntersectedInPlace(fSaveCount, element.getOp())) {
            switch (prior->fDeviceSpaceType) {
                case Element::DeviceSpaceType::kEmpty:
                    SkDEBUGCODE(prior->checkEmpty();)
                    return;
                case Element::DeviceSpaceType::kShader:
                    if (Element::DeviceSpaceType::kShader == element.getDeviceSpaceType()) {
                        prior->fShader = SkShaders::Blend(SkBlendMode::kSrcIn,
                                                          element.fShader, prior->fShader);
                        Element* priorPrior = (Element*) iter.prev();
                        prior->updateBoundAndGenID(priorPrior);
                        return;
                    }
                    break;
                case Element::DeviceSpaceType::kRect:
                    if (Element::DeviceSpaceType::kRect == element.getDeviceSpaceType()) {
                        if (prior->rectRectIntersectAllowed(element.getDeviceSpaceRect(),
                                                            element.isAA())) {
                            SkRect isectRect;
                            if (!isectRect.intersect(prior->getDeviceSpaceRect(),
                                                     element.getDeviceSpaceRect())) {
                                prior->setEmpty();
                                return;
                            }

                            prior->fDeviceSpaceRRect.setRect(isectRect);
                            prior->fDoAA = element.isAA();
                            Element* priorPrior = (Element*) iter.prev();
                            prior->updateBoundAndGenID(priorPrior);
                            return;
                        }
                        break;
                    }
                    [[fallthrough]];
                default:
                    if (!SkRect::Intersects(prior->getBounds(), element.getBounds())) {
                        prior->setEmpty();
                        return;
                    }
                    break;
            }
        }
    }
    Element* newElement = new (fDeque.push_back()) Element(element);
    newElement->updateBoundAndGenID(prior);
}

void SkClipStack::clipRRect(const SkRRect& rrect, const SkMatrix& matrix, SkClipOp op, bool doAA) {
    Element element(fSaveCount, rrect, matrix, op, doAA);
    this->pushElement(element);
}

void SkClipStack::clipRect(const SkRect& rect, const SkMatrix& matrix, SkClipOp op, bool doAA) {
    Element element(fSaveCount, rect, matrix, op, doAA);
    this->pushElement(element);
}

void SkClipStack::clipPath(const SkPath& path, const SkMatrix& matrix, SkClipOp op,
                           bool doAA) {
    Element element(fSaveCount, path, matrix, op, doAA);
    this->pushElement(element);
}

void SkClipStack::clipShader(sk_sp<SkShader> shader) {
    Element element(fSaveCount, std::move(shader));
    this->pushElement(element);
}

void SkClipStack::replaceClip(const SkRect& rect, bool doAA) {
    Element element(fSaveCount, rect, doAA);
    this->pushElement(element);
}

void SkClipStack::clipEmpty() {
    Element* element = (Element*) fDeque.back();

    if (element && element->canBeIntersectedInPlace(fSaveCount, SkClipOp::kIntersect)) {
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
    const Element* back = static_cast<const Element*>(fDeque.back());
    if (!back) {
        // TODO: return bounds?
        return false;
    }
    // First check if the entire stack is known to be a rect by the top element.
    if (back->fIsIntersectionOfRects && back->fFiniteBoundType == BoundsType::kNormal_BoundsType) {
        rrect->setRect(back->fFiniteBound);
        *aa = back->isAA();
        return true;
    }

    if (back->getDeviceSpaceType() != SkClipStack::Element::DeviceSpaceType::kRect &&
        back->getDeviceSpaceType() != SkClipStack::Element::DeviceSpaceType::kRRect) {
        return false;
    }
    if (back->isReplaceOp()) {
        *rrect = back->asDeviceSpaceRRect();
        *aa = back->isAA();
        return true;
    }

    if (back->getOp() == SkClipOp::kIntersect) {
        SkRect backBounds;
        if (!backBounds.intersect(bounds, back->asDeviceSpaceRRect().rect())) {
            return false;
        }
        // We limit to 17 elements. This means the back element will be bounds checked at most 16
        // times if it is an rrect.
        int cnt = fDeque.count();
        if (cnt > 17) {
            return false;
        }
        if (cnt > 1) {
            SkDeque::Iter iter(fDeque, SkDeque::Iter::kBack_IterStart);
            SkAssertResult(static_cast<const Element*>(iter.prev()) == back);
            while (const Element* prior = (const Element*)iter.prev()) {
                // TODO: Once expanding clip ops are removed, this is equiv. to op == kDifference
                if ((prior->getOp() != SkClipOp::kIntersect && !prior->isReplaceOp()) ||
                    !prior->contains(backBounds)) {
                    return false;
                }
                if (prior->isReplaceOp()) {
                    break;
                }
            }
        }
        *rrect = back->asDeviceSpaceRRect();
        *aa = back->isAA();
        return true;
    }
    return false;
}

uint32_t SkClipStack::GetNextGenID() {
    // 0-2 are reserved for invalid, empty & wide-open
    static const uint32_t kFirstUnreservedGenID = 3;
    static std::atomic<uint32_t> nextID{kFirstUnreservedGenID};

    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id < kFirstUnreservedGenID);
    return id;
}

uint32_t SkClipStack::getTopmostGenID() const {
    if (fDeque.empty()) {
        return kWideOpenGenID;
    }

    const Element* back = static_cast<const Element*>(fDeque.back());
    if (kInsideOut_BoundsType == back->fFiniteBoundType && back->fFiniteBound.isEmpty() &&
        Element::DeviceSpaceType::kShader != back->fDeviceSpaceType) {
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
        "path",
        "shader"
    };
    static_assert(0 == static_cast<int>(DeviceSpaceType::kEmpty), "enum mismatch");
    static_assert(1 == static_cast<int>(DeviceSpaceType::kRect), "enum mismatch");
    static_assert(2 == static_cast<int>(DeviceSpaceType::kRRect), "enum mismatch");
    static_assert(3 == static_cast<int>(DeviceSpaceType::kPath), "enum mismatch");
    static_assert(4 == static_cast<int>(DeviceSpaceType::kShader), "enum mismatch");
    static_assert(std::size(kTypeStrings) == kTypeCnt, "enum mismatch");

    const char* opName = this->isReplaceOp() ? "replace" :
            (fOp == SkClipOp::kDifference ? "difference" : "intersect");
    SkDebugf("Type: %s, Op: %s, AA: %s, Save Count: %d\n", kTypeStrings[(int)fDeviceSpaceType],
             opName, (fDoAA ? "yes" : "no"), fSaveCount);
    switch (fDeviceSpaceType) {
        case DeviceSpaceType::kEmpty:
            SkDebugf("\n");
            break;
        case DeviceSpaceType::kRect:
            this->getDeviceSpaceRect().dump();
            SkDebugf("\n");
            break;
        case DeviceSpaceType::kRRect:
            this->getDeviceSpaceRRect().dump();
            SkDebugf("\n");
            break;
        case DeviceSpaceType::kPath:
            this->getDeviceSpacePath().dump(nullptr, false);
            break;
        case DeviceSpaceType::kShader:
            // SkShaders don't provide much introspection that's worth while.
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
