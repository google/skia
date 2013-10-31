
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReducedClip.h"

typedef SkClipStack::Element Element;
////////////////////////////////////////////////////////////////////////////////

namespace GrReducedClip {

// helper function
void reduced_stack_walker(const SkClipStack& stack,
                          const SkRect& queryBounds,
                          ElementList* result,
                          InitialState* initialState,
                          bool* requiresAA);

/*
There are plenty of optimizations that could be added here. Maybe flips could be folded into
earlier operations. Or would inserting flips and reversing earlier ops ever be a win? Perhaps
for the case where the bounds are kInsideOut_BoundsType. We could restrict earlier operations
based on later intersect operations, and perhaps remove intersect-rects. We could optionally
take a rect in case the caller knows a bound on what is to be drawn through this clip.
*/
void ReduceClipStack(const SkClipStack& stack,
                     const SkIRect& queryBounds,
                     ElementList* result,
                     InitialState* initialState,
                     SkIRect* tighterBounds,
                     bool* requiresAA) {
    result->reset();

    if (stack.isWideOpen()) {
        *initialState = kAllIn_InitialState;
        return;
    }


    // We initially look at whether the bounds alone is sufficient. We also use the stack bounds to
    // attempt to compute the tighterBounds.

    SkClipStack::BoundsType stackBoundsType;
    SkRect stackBounds;
    bool iior;
    stack.getBounds(&stackBounds, &stackBoundsType, &iior);

    const SkIRect* bounds = &queryBounds;

    SkRect scalarQueryBounds = SkRect::MakeFromIRect(queryBounds);

    if (iior) {
        SkASSERT(SkClipStack::kNormal_BoundsType == stackBoundsType);
        SkRect isectRect;
        if (stackBounds.contains(scalarQueryBounds)) {
            *initialState = kAllIn_InitialState;
            if (NULL != tighterBounds) {
                *tighterBounds = queryBounds;
            }
            if (NULL != requiresAA) {
               *requiresAA = false;
            }
        } else if (isectRect.intersect(stackBounds, scalarQueryBounds)) {
            if (NULL != tighterBounds) {
                isectRect.roundOut(tighterBounds);
                SkRect scalarTighterBounds = SkRect::MakeFromIRect(*tighterBounds);
                if (scalarTighterBounds == isectRect) {
                    // the round-out didn't add any area outside the clip rect.
                    *requiresAA = false;
                    *initialState = kAllIn_InitialState;
                    return;
                }
                *initialState = kAllOut_InitialState;
                // iior should only be true if aa/non-aa status matches among all elements.
                SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
                bool doAA = iter.prev()->isAA();
                SkNEW_INSERT_AT_LLIST_HEAD(result, Element, (isectRect, SkRegion::kReplace_Op, doAA));
                if (NULL != requiresAA) {
                    *requiresAA = doAA;
                }
            }
        } else {
            *initialState = kAllOut_InitialState;
             if (NULL != requiresAA) {
                *requiresAA = false;
             }
        }
        return;
    } else {
        if (SkClipStack::kNormal_BoundsType == stackBoundsType) {
            if (!SkRect::Intersects(stackBounds, scalarQueryBounds)) {
                *initialState = kAllOut_InitialState;
                if (NULL != requiresAA) {
                   *requiresAA = false;
                }
                return;
            }
            if (NULL != tighterBounds) {
                SkIRect stackIBounds;
                stackBounds.roundOut(&stackIBounds);
                tighterBounds->intersect(queryBounds, stackIBounds);
                bounds = tighterBounds;
            }
        } else {
            if (stackBounds.contains(scalarQueryBounds)) {
                *initialState = kAllOut_InitialState;
                if (NULL != requiresAA) {
                   *requiresAA = false;
                }
                return;
            }
            if (NULL != tighterBounds) {
                *tighterBounds = queryBounds;
            }
        }
    }

    SkRect scalarBounds = SkRect::MakeFromIRect(*bounds);

    // Now that we have determined the bounds to use and filtered out the trivial cases, call the
    // helper that actually walks the stack.
    reduced_stack_walker(stack, scalarBounds, result, initialState, requiresAA);
}

void reduced_stack_walker(const SkClipStack& stack,
                          const SkRect& queryBounds,
                          ElementList* result,
                          InitialState* initialState,
                          bool* requiresAA) {

    // walk backwards until we get to:
    //  a) the beginning
    //  b) an operation that is known to make the bounds all inside/outside
    //  c) a replace operation

    static const InitialState kUnknown_InitialState = static_cast<InitialState>(-1);
    *initialState = kUnknown_InitialState;

    // During our backwards walk, track whether we've seen ops that either grow or shrink the clip.
    // TODO: track these per saved clip so that we can consider them on the forward pass.
    bool embiggens = false;
    bool emsmallens = false;

    SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);
    int numAAElements = 0;
    while ((kUnknown_InitialState == *initialState)) {
        const Element* element = iter.prev();
        if (NULL == element) {
            *initialState = kAllIn_InitialState;
            break;
        }
        if (SkClipStack::kEmptyGenID == element->getGenID()) {
            *initialState = kAllOut_InitialState;
            break;
        }
        if (SkClipStack::kWideOpenGenID == element->getGenID()) {
            *initialState = kAllIn_InitialState;
            break;
        }

        bool skippable = false;
        bool isFlip = false; // does this op just flip the in/out state of every point in the bounds

        switch (element->getOp()) {
            case SkRegion::kDifference_Op:
                // check if the shape subtracted either contains the entire bounds (and makes
                // the clip empty) or is outside the bounds and therefore can be skipped.
                if (element->isInverseFilled()) {
                    if (element->contains(queryBounds)) {
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
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
                    if (element->contains(queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        skippable = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllOut_InitialState;
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
                    if (element->contains(queryBounds)) {
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllIn_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        *initialState = kAllIn_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
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
                    if (element->contains(queryBounds)) {
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        isFlip = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        isFlip = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
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
                    if (element->contains(queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        isFlip = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        isFlip = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllOut_InitialState;
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
                    if (element->contains(queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllIn_InitialState;
                        skippable = true;
                    }
                } else {
                    if (element->contains(queryBounds)) {
                        *initialState = kAllIn_InitialState;
                        skippable = true;
                    } else if (!SkRect::Intersects(element->getBounds(), queryBounds)) {
                        *initialState = kAllOut_InitialState;
                        skippable = true;
                    }
                }
                if (!skippable) {
                    *initialState = kAllOut_InitialState;
                    embiggens = emsmallens = true;
                }
                break;
            default:
                SkDEBUGFAIL("Unexpected op.");
                break;
        }
        if (!skippable) {
            // if it is a flip, change it to a bounds-filling rect
            if (isFlip) {
                SkASSERT(SkRegion::kXOR_Op == element->getOp() ||
                         SkRegion::kReverseDifference_Op == element->getOp());
                SkNEW_INSERT_AT_LLIST_HEAD(result,
                                           Element,
                                           (queryBounds, SkRegion::kReverseDifference_Op, false));
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
                        SkASSERT(kAllOut_InitialState == *initialState);
                        *initialState = kAllIn_InitialState;
                    }
                }
            }
        }
    }

    if ((kAllOut_InitialState == *initialState && !embiggens) ||
        (kAllIn_InitialState == *initialState && !emsmallens)) {
        result->reset();
    } else {
        Element* element = result->headIter().get();
        while (NULL != element) {
            bool skippable = false;
            switch (element->getOp()) {
                case SkRegion::kDifference_Op:
                    // subtracting from the empty set yields the empty set.
                    skippable = kAllOut_InitialState == *initialState;
                    break;
                case SkRegion::kIntersect_Op:
                    // intersecting with the empty set yields the empty set
                    if (kAllOut_InitialState == *initialState) {
                        skippable = true;
                    } else {
                        // We can clear to zero and then simply draw the clip element.
                        *initialState = kAllOut_InitialState;
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kUnion_Op:
                    if (kAllIn_InitialState == *initialState) {
                        // unioning the infinite plane with anything is a no-op.
                        skippable = true;
                    } else {
                        // unioning the empty set with a shape is the shape.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kXOR_Op:
                    if (kAllOut_InitialState == *initialState) {
                        // xor could be changed to diff in the kAllIn case, not sure it's a win.
                        element->setOp(SkRegion::kReplace_Op);
                    }
                    break;
                case SkRegion::kReverseDifference_Op:
                    if (kAllIn_InitialState == *initialState) {
                        // subtracting the whole plane will yield the empty set.
                        skippable = true;
                        *initialState = kAllOut_InitialState;
                    } else {
                        // this picks up flips inserted in the backwards pass.
                        skippable = element->isInverseFilled() ?
                            !SkRect::Intersects(element->getBounds(), queryBounds) :
                            element->contains(queryBounds);
                        if (skippable) {
                            *initialState = kAllIn_InitialState;
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
    if (NULL != requiresAA) {
        *requiresAA = numAAElements > 0;
    }
}
} // namespace GrReducedClip
