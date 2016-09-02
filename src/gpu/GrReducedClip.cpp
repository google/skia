/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReducedClip.h"

#include "GrClip.h"

typedef SkClipStack::Element Element;

/**
 * There are plenty of optimizations that could be added here. Maybe flips could be folded into
 * earlier operations. Or would inserting flips and reversing earlier ops ever be a win? Perhaps
 * for the case where the bounds are kInsideOut_BoundsType. We could restrict earlier operations
 * based on later intersect operations, and perhaps remove intersect-rects. We could optionally
 * take a rect in case the caller knows a bound on what is to be drawn through this clip.
 */
GrReducedClip::GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds) {
    SkASSERT(!queryBounds.isEmpty());
    fHasIBounds = false;

    if (stack.isWideOpen()) {
        fInitialState = InitialState::kAllIn;
        return;
    }

    SkClipStack::BoundsType stackBoundsType;
    SkRect stackBounds;
    bool iior;
    stack.getBounds(&stackBounds, &stackBoundsType, &iior);

    if (stackBounds.isEmpty() || GrClip::IsOutsideClip(stackBounds, queryBounds)) {
        bool insideOut = SkClipStack::kInsideOut_BoundsType == stackBoundsType;
        fInitialState = insideOut ? InitialState::kAllIn : InitialState::kAllOut;
        return;
    }

    if (iior) {
        // "Is intersection of rects" means the clip is a single rect indicated by the stack bounds.
        // This should only be true if aa/non-aa status matches among all elements.
        SkASSERT(SkClipStack::kNormal_BoundsType == stackBoundsType);
        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
        if (!iter.prev()->isAA() || GrClip::IsPixelAligned(stackBounds)) {
            // The clip is a non-aa rect. This is the one spot where we can actually implement the
            // clip (using fIBounds) rather than just telling the caller what it should be.
            stackBounds.round(&fIBounds);
            fHasIBounds = true;
            fInitialState = fIBounds.isEmpty() ? InitialState::kAllOut : InitialState::kAllIn;
            return;
        }
        if (GrClip::IsInsideClip(stackBounds, queryBounds)) {
            fInitialState = InitialState::kAllIn;
            return;
        }

        SkRect tightBounds;
        SkAssertResult(tightBounds.intersect(stackBounds, queryBounds));
        fIBounds = GrClip::GetPixelIBounds(tightBounds);
        SkASSERT(!fIBounds.isEmpty()); // Empty should have been blocked by IsOutsideClip above.
        fHasIBounds = true;

        // Implement the clip with an AA rect element.
        fElements.addToHead(stackBounds, SkRegion::kReplace_Op, true/*doAA*/);
        fElementsGenID = stack.getTopmostGenID();
        fRequiresAA = true;

        fInitialState = InitialState::kAllOut;
        return;
    }

    SkRect tighterQuery = queryBounds;
    if (SkClipStack::kNormal_BoundsType == stackBoundsType) {
        // Tighten the query by introducing a new clip at the stack's pixel boundaries. (This new
        // clip will be enforced by the scissor through fIBounds.)
        SkAssertResult(tighterQuery.intersect(GrClip::GetPixelBounds(stackBounds)));
    }

    fIBounds = GrClip::GetPixelIBounds(tighterQuery);
    SkASSERT(!fIBounds.isEmpty()); // Empty should have been blocked by IsOutsideClip above.
    fHasIBounds = true;

    // Now that we have determined the bounds to use and filtered out the trivial cases, call the
    // helper that actually walks the stack.
    this->walkStack(stack, tighterQuery);
}

void GrReducedClip::walkStack(const SkClipStack& stack, const SkRect& queryBounds) {
    // walk backwards until we get to:
    //  a) the beginning
    //  b) an operation that is known to make the bounds all inside/outside
    //  c) a replace operation

    enum class InitialTriState {
        kUnknown = -1,
        kAllIn = (int)GrReducedClip::InitialState::kAllIn,
        kAllOut = (int)GrReducedClip::InitialState::kAllOut
    } initialTriState = InitialTriState::kUnknown;

    // During our backwards walk, track whether we've seen ops that either grow or shrink the clip.
    // TODO: track these per saved clip so that we can consider them on the forward pass.
    bool embiggens = false;
    bool emsmallens = false;

    // We use a slightly relaxed set of query bounds for element containment tests. This is to
    // account for floating point rounding error that may have occurred during coord transforms.
    SkRect relaxedQueryBounds = queryBounds.makeInset(GrClip::kBoundsTolerance,
                                                      GrClip::kBoundsTolerance);

    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
    int numAAElements = 0;
    while (InitialTriState::kUnknown == initialTriState) {
        const Element* element = iter.prev();
        if (nullptr == element) {
            initialTriState = InitialTriState::kAllIn;
            break;
        }
        if (SkClipStack::kEmptyGenID == element->getGenID()) {
            initialTriState = InitialTriState::kAllOut;
            break;
        }
        if (SkClipStack::kWideOpenGenID == element->getGenID()) {
            initialTriState = InitialTriState::kAllIn;
            break;
        }

        bool skippable = false;
        bool isFlip = false; // does this op just flip the in/out state of every point in the bounds

        switch (element->getOp()) {
            case SkRegion::kDifference_Op:
                // check if the shape subtracted either contains the entire bounds (and makes
                // the clip empty) or is outside the bounds and therefore can be skipped.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                }
                if (!skippable) {
                    emsmallens = true;
                }
                break;
            case SkRegion::kIntersect_Op:
                // check if the shape intersected contains the entire bounds and therefore can
                // be skipped or it is outside the entire bounds and therefore makes the clip
                // empty.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (!embiggens && !element->isAA() &&
                               Element::kRect_Type == element->getType()) {
                        // fIBounds and queryBounds have already acccounted for this element via
                        // clip stack bounds; here we just apply the non-aa rounding effect.
                        SkIRect nonaaRect;
                        element->getRect().round(&nonaaRect);
                        if (!this->intersectIBounds(nonaaRect)) {
                            return;
                        }
                        skippable = true;
                    }
                }
                if (!skippable) {
                    emsmallens = true;
                }
                break;
            case SkRegion::kUnion_Op:
                // If the union-ed shape contains the entire bounds then after this element
                // the bounds is entirely inside the clip. If the union-ed shape is outside the
                // bounds then this op can be skipped.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                }
                if (!skippable) {
                    embiggens = true;
                }
                break;
            case SkRegion::kXOR_Op:
                // If the bounds is entirely inside the shape being xor-ed then the effect is
                // to flip the inside/outside state of every point in the bounds. We may be
                // able to take advantage of this in the forward pass. If the xor-ed shape
                // doesn't intersect the bounds then it can be skipped.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        isFlip = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        isFlip = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                }
                if (!skippable) {
                    emsmallens = embiggens = true;
                }
                break;
            case SkRegion::kReverseDifference_Op:
                // When the bounds is entirely within the rev-diff shape then this behaves like xor
                // and reverses every point inside the bounds. If the shape is completely outside
                // the bounds then we know after this element is applied that the bounds will be
                // all outside the current clip.B
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        isFlip = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        isFlip = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    }
                }
                if (!skippable) {
                    emsmallens = embiggens = true;
                }
                break;

            case SkRegion::kReplace_Op:
                // Replace will always terminate our walk. We will either begin the forward walk
                // at the replace op or detect here than the shape is either completely inside
                // or completely outside the bounds. In this latter case it can be skipped by
                // setting the correct value for initialTriState.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (!embiggens && !element->isAA() &&
                               Element::kRect_Type == element->getType()) {
                        // fIBounds and queryBounds have already acccounted for this element via
                        // clip stack bounds; here we just apply the non-aa rounding effect.
                        SkIRect nonaaRect;
                        element->getRect().round(&nonaaRect);
                        if (!this->intersectIBounds(nonaaRect)) {
                            return;
                        }
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    }
                }
                if (!skippable) {
                    initialTriState = InitialTriState::kAllOut;
                    embiggens = emsmallens = true;
                }
                break;
            default:
                SkDEBUGFAIL("Unexpected op.");
                break;
        }
        if (!skippable) {
            if (0 == fElements.count()) {
                // This will be the last element. Record the stricter genID.
                fElementsGenID = element->getGenID();
            }

            // if it is a flip, change it to a bounds-filling rect
            if (isFlip) {
                SkASSERT(SkRegion::kXOR_Op == element->getOp() ||
                         SkRegion::kReverseDifference_Op == element->getOp());
                fElements.addToHead(SkRect::Make(fIBounds), SkRegion::kReverseDifference_Op, false);
            } else {
                Element* newElement = fElements.addToHead(*element);
                if (newElement->isAA()) {
                    ++numAAElements;
                }
                // Intersecting an inverse shape is the same as differencing the non-inverse shape.
                // Replacing with an inverse shape is the same as setting initialState=kAllIn and
                // differencing the non-inverse shape.
                bool isReplace = SkRegion::kReplace_Op == newElement->getOp();
                if (newElement->isInverseFilled() &&
                    (SkRegion::kIntersect_Op == newElement->getOp() || isReplace)) {
                    newElement->invertShapeFillType();
                    newElement->setOp(SkRegion::kDifference_Op);
                    if (isReplace) {
                        SkASSERT(InitialTriState::kAllOut == initialTriState);
                        initialTriState = InitialTriState::kAllIn;
                    }
                }
            }
        }
    }

    if ((InitialTriState::kAllOut == initialTriState && !embiggens) ||
        (InitialTriState::kAllIn == initialTriState && !emsmallens)) {
        fElements.reset();
        numAAElements = 0;
    } else {
        Element* element = fElements.headIter().get();
        while (element) {
            bool skippable = false;
            switch (element->getOp()) {
                case SkRegion::kDifference_Op:
                    // subtracting from the empty set yields the empty set.
                    skippable = InitialTriState::kAllOut == initialTriState;
                    break;
                case SkRegion::kIntersect_Op:
                    // intersecting with the empty set yields the empty set
                    if (InitialTriState::kAllOut == initialTriState) {
                        skippable = true;
                    } else {
                        // We can clear to zero and then simply draw the clip element.
                        initialTriState = InitialTriState::kAllOut;
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kUnion_Op:
                    if (InitialTriState::kAllIn == initialTriState) {
                        // unioning the infinite plane with anything is a no-op.
                        skippable = true;
                    } else {
                        // unioning the empty set with a shape is the shape.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kXOR_Op:
                    if (InitialTriState::kAllOut == initialTriState) {
                        // xor could be changed to diff in the kAllIn case, not sure it's a win.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kReverseDifference_Op:
                    if (InitialTriState::kAllIn == initialTriState) {
                        // subtracting the whole plane will yield the empty set.
                        skippable = true;
                        initialTriState = InitialTriState::kAllOut;
                    } else {
                        // this picks up flips inserted in the backwards pass.
                        skippable = element->isInverseFilled() ?
                            GrClip::IsOutsideClip(element->getBounds(), queryBounds) :
                            element->contains(relaxedQueryBounds);
                        if (skippable) {
                            initialTriState = InitialTriState::kAllIn;
                        } else {
                            element->setOp(SkRegion::kReplace_Op);
                        }
                    }
                    break;
                case SkRegion::kReplace_Op:
                    skippable = false; // we would have skipped it in the backwards walk if we
                                       // could've.
                    break;
                default:
                    SkDEBUGFAIL("Unexpected op.");
                    break;
            }
            if (!skippable) {
                break;
            } else {
                if (element->isAA()) {
                    --numAAElements;
                }
                fElements.popHead();
                element = fElements.headIter().get();
            }
        }
    }
    fRequiresAA = numAAElements > 0;

    SkASSERT(InitialTriState::kUnknown != initialTriState);
    fInitialState = static_cast<GrReducedClip::InitialState>(initialTriState);
}

inline bool GrReducedClip::intersectIBounds(const SkIRect& irect) {
    SkASSERT(fHasIBounds);
    if (!fIBounds.intersect(irect)) {
        fHasIBounds = false;
        fElements.reset();
        fRequiresAA = false;
        fInitialState = InitialState::kAllOut;
        return false;
    }
    return true;
}
