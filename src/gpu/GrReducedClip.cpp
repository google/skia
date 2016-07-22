/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReducedClip.h"

#include "GrClip.h"

typedef SkClipStack::Element Element;

static GrReducedClip::InitialState reduced_stack_walker(const SkClipStack& stack,
                                                        const SkRect& queryBounds,
                                                        const SkIRect& clipIBounds,
                                                        GrReducedClip::ElementList* result,
                                                        int32_t* resultGenID,
                                                        bool* requiresAA) {

    // walk backwards until we get to:
    //  a) the beginning
    //  b) an operation that is known to make the bounds all inside/outside
    //  c) a replace operation

    static const GrReducedClip::InitialState kUnknown_InitialState =
        static_cast<GrReducedClip::InitialState>(-1);
    GrReducedClip::InitialState initialState = kUnknown_InitialState;

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
    while (kUnknown_InitialState == initialState) {
        const Element* element = iter.prev();
        if (nullptr == element) {
            initialState = GrReducedClip::kAllIn_InitialState;
            break;
        }
        if (SkClipStack::kEmptyGenID == element->getGenID()) {
            initialState = GrReducedClip::kAllOut_InitialState;
            break;
        }
        if (SkClipStack::kWideOpenGenID == element->getGenID()) {
            initialState = GrReducedClip::kAllIn_InitialState;
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
                        initialState = GrReducedClip::kAllOut_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialState = GrReducedClip::kAllOut_InitialState;
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
                        initialState = GrReducedClip::kAllOut_InitialState;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialState = GrReducedClip::kAllOut_InitialState;
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
                        initialState = GrReducedClip::kAllIn_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialState = GrReducedClip::kAllIn_InitialState;
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
                        initialState = GrReducedClip::kAllOut_InitialState;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        isFlip = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        isFlip = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialState = GrReducedClip::kAllOut_InitialState;
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
                // setting the correct value for initialState.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        initialState = GrReducedClip::kAllOut_InitialState;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialState = GrReducedClip::kAllIn_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialState = GrReducedClip::kAllIn_InitialState;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialState = GrReducedClip::kAllOut_InitialState;
                        skippable = true;
                    }
                }
                if (!skippable) {
                    initialState = GrReducedClip::kAllOut_InitialState;
                    embiggens = emsmallens = true;
                }
                break;
            default:
                SkDEBUGFAIL("Unexpected op.");
                break;
        }
        if (!skippable) {
            if (0 == result->count()) {
                // This will be the last element. Record the stricter genID.
                *resultGenID = element->getGenID();
            }

            // if it is a flip, change it to a bounds-filling rect
            if (isFlip) {
                SkASSERT(SkRegion::kXOR_Op == element->getOp() ||
                         SkRegion::kReverseDifference_Op == element->getOp());
                result->addToHead(SkRect::Make(clipIBounds), SkRegion::kReverseDifference_Op,
                                  false);
            } else {
                Element* newElement = result->addToHead(*element);
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
                        SkASSERT(GrReducedClip::kAllOut_InitialState == initialState);
                        initialState = GrReducedClip::kAllIn_InitialState;
                    }
                }
            }
        }
    }

    if ((GrReducedClip::kAllOut_InitialState == initialState && !embiggens) ||
        (GrReducedClip::kAllIn_InitialState == initialState && !emsmallens)) {
        result->reset();
        numAAElements = 0;
    } else {
        Element* element = result->headIter().get();
        while (element) {
            bool skippable = false;
            switch (element->getOp()) {
                case SkRegion::kDifference_Op:
                    // subtracting from the empty set yields the empty set.
                    skippable = GrReducedClip::kAllOut_InitialState == initialState;
                    break;
                case SkRegion::kIntersect_Op:
                    // intersecting with the empty set yields the empty set
                    if (GrReducedClip::kAllOut_InitialState == initialState) {
                        skippable = true;
                    } else {
                        // We can clear to zero and then simply draw the clip element.
                        initialState = GrReducedClip::kAllOut_InitialState;
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kUnion_Op:
                    if (GrReducedClip::kAllIn_InitialState == initialState) {
                        // unioning the infinite plane with anything is a no-op.
                        skippable = true;
                    } else {
                        // unioning the empty set with a shape is the shape.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kXOR_Op:
                    if (GrReducedClip::kAllOut_InitialState == initialState) {
                        // xor could be changed to diff in the kAllIn case, not sure it's a win.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kReverseDifference_Op:
                    if (GrReducedClip::kAllIn_InitialState == initialState) {
                        // subtracting the whole plane will yield the empty set.
                        skippable = true;
                        initialState = GrReducedClip::kAllOut_InitialState;
                    } else {
                        // this picks up flips inserted in the backwards pass.
                        skippable = element->isInverseFilled() ?
                            GrClip::IsOutsideClip(element->getBounds(), queryBounds) :
                            element->contains(relaxedQueryBounds);
                        if (skippable) {
                            initialState = GrReducedClip::kAllIn_InitialState;
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
                result->popHead();
                element = result->headIter().get();
            }
        }
    }
    *requiresAA = numAAElements > 0;

    if (0 == result->count()) {
        if (initialState == GrReducedClip::kAllIn_InitialState) {
            *resultGenID = SkClipStack::kWideOpenGenID;
        } else {
            *resultGenID = SkClipStack::kEmptyGenID;
        }
    }

    SkASSERT(SkClipStack::kInvalidGenID != *resultGenID);
    return initialState;
}

/*
There are plenty of optimizations that could be added here. Maybe flips could be folded into
earlier operations. Or would inserting flips and reversing earlier ops ever be a win? Perhaps
for the case where the bounds are kInsideOut_BoundsType. We could restrict earlier operations
based on later intersect operations, and perhaps remove intersect-rects. We could optionally
take a rect in case the caller knows a bound on what is to be drawn through this clip.
*/
GrReducedClip::InitialState GrReducedClip::ReduceClipStack(const SkClipStack& stack,
                                                           const SkRect& queryBounds,
                                                           ElementList* result,
                                                           int32_t* resultGenID,
                                                           SkIRect* clipIBounds,
                                                           bool* requiresAA) {
    SkASSERT(!queryBounds.isEmpty());
    result->reset();

    // The clip established by the element list might be cached based on the last
    // generation id. When we make early returns, we do not know what was the generation
    // id that lead to the state. Make a conservative guess.
    *resultGenID = stack.getTopmostGenID();

    // TODO: instead devise a way of telling the caller to disregard some or all of the clip bounds.
    *clipIBounds = GrClip::GetPixelIBounds(queryBounds);

    if (stack.isWideOpen()) {
        return kAllIn_InitialState;
    }

    SkClipStack::BoundsType stackBoundsType;
    SkRect stackBounds;
    bool iior;
    stack.getBounds(&stackBounds, &stackBoundsType, &iior);

    if (stackBounds.isEmpty() || GrClip::IsOutsideClip(stackBounds, queryBounds)) {
        bool insideOut = SkClipStack::kInsideOut_BoundsType == stackBoundsType;
        return insideOut ? kAllIn_InitialState : kAllOut_InitialState;
    }

    if (iior) {
        // "Is intersection of rects" means the clip is a single rect indicated by the stack bounds.
        // This should only be true if aa/non-aa status matches among all elements.
        SkASSERT(SkClipStack::kNormal_BoundsType == stackBoundsType);
        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
        if (!iter.prev()->isAA() || GrClip::IsPixelAligned(stackBounds)) {
            // The clip is a non-aa rect. This is the one spot where we can actually implement the
            // clip (using clipIBounds) rather than just telling the caller what it should be.
            stackBounds.round(clipIBounds);
            return kAllIn_InitialState;
        }
        if (GrClip::IsInsideClip(stackBounds, queryBounds)) {
            return kAllIn_InitialState;
        }

        // Implement the clip with an AA rect element.
        result->addToHead(stackBounds, SkRegion::kReplace_Op, true/*doAA*/);
        *requiresAA = true;

        SkAssertResult(clipIBounds->intersect(GrClip::GetPixelIBounds(stackBounds)));
        return kAllOut_InitialState;
    }

    SkRect tighterQuery = queryBounds;
    if (SkClipStack::kNormal_BoundsType == stackBoundsType) {
        // Tighten the query by introducing a new clip at the stack's pixel boundaries. (This new
        // clip will be enforced by the scissor through clipIBounds.)
        SkAssertResult(tighterQuery.intersect(GrClip::GetPixelBounds(stackBounds)));
        *clipIBounds = GrClip::GetPixelIBounds(tighterQuery);
    }

    // Now that we have determined the bounds to use and filtered out the trivial cases, call the
    // helper that actually walks the stack.
    return reduced_stack_walker(stack, tighterQuery, *clipIBounds, result, resultGenID, requiresAA);
}
