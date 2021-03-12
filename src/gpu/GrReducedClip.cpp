/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkClipOpPriv.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrReducedClip.h"
#include "src/gpu/GrStencilClip.h"
#include "src/gpu/GrStencilMaskHelper.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "src/gpu/effects/generated/GrDeviceSpaceEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/shaders/SkShaderBase.h"

/**
 * There are plenty of optimizations that could be added here. Maybe flips could be folded into
 * earlier operations. Or would inserting flips and reversing earlier ops ever be a win? Perhaps
 * for the case where the bounds are kInsideOut_BoundsType. We could restrict earlier operations
 * based on later intersect operations, and perhaps remove intersect-rects. We could optionally
 * take a rect in case the caller knows a bound on what is to be drawn through this clip.
 */
GrReducedClip::GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds,
                             const GrCaps* caps, int maxWindowRectangles, int maxAnalyticElements,
                             int maxCCPRClipPaths)
        : fCaps(caps)
        , fMaxWindowRectangles(maxWindowRectangles)
        , fMaxAnalyticElements(maxAnalyticElements)
        , fMaxCCPRClipPaths(maxCCPRClipPaths) {
    SkASSERT(!queryBounds.isEmpty());
    SkASSERT(fMaxWindowRectangles <= GrWindowRectangles::kMaxWindows);
    SkASSERT(fMaxCCPRClipPaths <= fMaxAnalyticElements);

    if (stack.isWideOpen()) {
        fInitialState = InitialState::kAllIn;
        return;
    }

    SkClipStack::BoundsType stackBoundsType;
    SkRect stackBounds;
    bool iior;
    stack.getBounds(&stackBounds, &stackBoundsType, &iior);

    if (GrClip::IsOutsideClip(stackBounds, queryBounds)) {
        bool insideOut = SkClipStack::kInsideOut_BoundsType == stackBoundsType;
        fInitialState = insideOut ? InitialState::kAllIn : InitialState::kAllOut;
        return;
    }

    if (iior) {
        // "Is intersection of rects" means the clip is a single rect indicated by the stack bounds.
        // This should only be true if aa/non-aa status matches among all elements.
        SkASSERT(SkClipStack::kNormal_BoundsType == stackBoundsType);

        if (GrClip::IsInsideClip(stackBounds, queryBounds)) {
            fInitialState = InitialState::kAllIn;
            return;
        }

        SkClipStack::Iter iter(stack, SkClipStack::Iter::kTop_IterStart);

        if (!iter.prev()->isAA() || GrClip::IsPixelAligned(stackBounds)) {
            // The clip is a non-aa rect. Here we just implement the entire thing using fScissor.
            stackBounds.round(&fScissor);
            fHasScissor = true;
            fInitialState = fScissor.isEmpty() ? InitialState::kAllOut : InitialState::kAllIn;
            return;
        }

        SkRect tightBounds;
        SkAssertResult(tightBounds.intersect(stackBounds, queryBounds));
        fScissor = GrClip::GetPixelIBounds(tightBounds);
        if (fScissor.isEmpty()) {
            fInitialState = InitialState::kAllOut;
            return;
        }
        fHasScissor = true;

        fAAClipRect = stackBounds;
        fAAClipRectGenID = stack.getTopmostGenID();
        SkASSERT(SK_InvalidGenID != fAAClipRectGenID);

        fInitialState = InitialState::kAllIn;
    } else {
        SkRect tighterQuery = queryBounds;
        if (SkClipStack::kNormal_BoundsType == stackBoundsType) {
            // Tighten the query by introducing a new clip at the stack's pixel boundaries. (This
            // new clip will be enforced by the scissor.)
            SkAssertResult(tighterQuery.intersect(GrClip::GetPixelBounds(stackBounds)));
        }

        fScissor = GrClip::GetPixelIBounds(tighterQuery);
        if (fScissor.isEmpty()) {
            fInitialState = InitialState::kAllOut;
            return;
        }
        fHasScissor = true;

        // Now that we have determined the bounds to use and filtered out the trivial cases, call
        // the helper that actually walks the stack.
        this->walkStack(stack, tighterQuery);

        if (fInitialState == InitialState::kAllOut && fMaskElements.isEmpty()) {
            // The clip starts with no coverage and there are no elements to add coverage with
            // expanding ops. We ignore the AAClipRectGenID since it is an implied intersection.
            this->makeEmpty();
            return;
        }
    }

    if (SK_InvalidGenID != fAAClipRectGenID && // Is there an AA clip rect?
        ClipResult::kNotClipped == this->addAnalyticRect(fAAClipRect, Invert::kNo, GrAA::kYes)) {
        if (fMaskElements.isEmpty()) {
            // Use a replace since it is faster than intersect.
            fMaskElements.addToHead(fAAClipRect, SkMatrix::I(), kReplace_SkClipOp, true /*doAA*/);
            fInitialState = InitialState::kAllOut;
        } else {
            fMaskElements.addToTail(fAAClipRect, SkMatrix::I(), kIntersect_SkClipOp, true /*doAA*/);
        }
        fMaskRequiresAA = true;
        fMaskGenID = fAAClipRectGenID;
    }
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
    if (relaxedQueryBounds.isEmpty()) {
        relaxedQueryBounds = queryBounds;
    }

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

        if (element->getDeviceSpaceType() == Element::DeviceSpaceType::kShader) {
            if (fShader) {
                // Combine multiple shaders together with src-in blending. This works because all
                // shaders are effectively intersections (difference ops have been modified to be
                // 1 - alpha already).
                fShader = SkShaders::Blend(SkBlendMode::kSrcIn, element->refShader(), fShader);
            } else {
                fShader = element->refShader();
            }
            continue;
        }

        bool skippable = false;
        bool isFlip = false; // does this op just flip the in/out state of every point in the bounds

        switch (element->getOp()) {
            case kDifference_SkClipOp:
                // check if the shape subtracted either contains the entire bounds (and makes
                // the clip empty) or is outside the bounds and therefore can be skipped.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (!embiggens) {
                        ClipResult result = this->clipInsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        skippable = (ClipResult::kClipped == result);
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    } else if (!embiggens) {
                        ClipResult result = this->clipOutsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        skippable = (ClipResult::kClipped == result);
                    }
                }
                if (!skippable) {
                    emsmallens = true;
                }
                break;
            case kIntersect_SkClipOp:
                // check if the shape intersected contains the entire bounds and therefore can
                // be skipped or it is outside the entire bounds and therefore makes the clip
                // empty.
                if (element->isInverseFilled()) {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        skippable = true;
                    } else if (!embiggens) {
                        ClipResult result = this->clipOutsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        skippable = (ClipResult::kClipped == result);
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (!embiggens) {
                        ClipResult result = this->clipInsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        skippable = (ClipResult::kClipped == result);
                    }
                }
                if (!skippable) {
                    emsmallens = true;
                }
                break;
            case kUnion_SkClipOp:
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
            case kXOR_SkClipOp:
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
            case kReverseDifference_SkClipOp:
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

            case kReplace_SkClipOp:
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
                    } else if (!embiggens) {
                        ClipResult result = this->clipOutsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        if (ClipResult::kClipped == result) {
                            initialTriState = InitialTriState::kAllIn;
                            skippable = true;
                        }
                    }
                } else {
                    if (element->contains(relaxedQueryBounds)) {
                        initialTriState = InitialTriState::kAllIn;
                        skippable = true;
                    } else if (GrClip::IsOutsideClip(element->getBounds(), queryBounds)) {
                        initialTriState = InitialTriState::kAllOut;
                        skippable = true;
                    } else if (!embiggens) {
                        ClipResult result = this->clipInsideElement(element);
                        if (ClipResult::kMadeEmpty == result) {
                            return;
                        }
                        if (ClipResult::kClipped == result) {
                            initialTriState = InitialTriState::kAllIn;
                            skippable = true;
                        }
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
            if (fMaskElements.isEmpty()) {
                // This will be the last element. Record the stricter genID.
                fMaskGenID = element->getGenID();
            }

            // if it is a flip, change it to a bounds-filling rect
            if (isFlip) {
                SkASSERT(kXOR_SkClipOp == element->getOp() ||
                         kReverseDifference_SkClipOp == element->getOp());
                fMaskElements.addToHead(SkRect::Make(fScissor), SkMatrix::I(),
                                        kReverseDifference_SkClipOp, false);
            } else {
                Element* newElement = fMaskElements.addToHead(*element);
                if (newElement->isAA()) {
                    ++numAAElements;
                }
                // Intersecting an inverse shape is the same as differencing the non-inverse shape.
                // Replacing with an inverse shape is the same as setting initialState=kAllIn and
                // differencing the non-inverse shape.
                bool isReplace = kReplace_SkClipOp == newElement->getOp();
                if (newElement->isInverseFilled() &&
                    (kIntersect_SkClipOp == newElement->getOp() || isReplace)) {
                    newElement->invertShapeFillType();
                    newElement->setOp(kDifference_SkClipOp);
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
        fMaskElements.reset();
        numAAElements = 0;
    } else {
        Element* element = fMaskElements.headIter().get();
        while (element) {
            bool skippable = false;
            switch (element->getOp()) {
                case kDifference_SkClipOp:
                    // subtracting from the empty set yields the empty set.
                    skippable = InitialTriState::kAllOut == initialTriState;
                    break;
                case kIntersect_SkClipOp:
                    // intersecting with the empty set yields the empty set
                    if (InitialTriState::kAllOut == initialTriState) {
                        skippable = true;
                    } else {
                        // We can clear to zero and then simply draw the clip element.
                        initialTriState = InitialTriState::kAllOut;
                        element->setOp(kReplace_SkClipOp);
                    }
                    break;
                case kUnion_SkClipOp:
                    if (InitialTriState::kAllIn == initialTriState) {
                        // unioning the infinite plane with anything is a no-op.
                        skippable = true;
                    } else {
                        // unioning the empty set with a shape is the shape.
                        element->setOp(kReplace_SkClipOp);
                    }
                    break;
                case kXOR_SkClipOp:
                    if (InitialTriState::kAllOut == initialTriState) {
                        // xor could be changed to diff in the kAllIn case, not sure it's a win.
                        element->setOp(kReplace_SkClipOp);
                    }
                    break;
                case kReverseDifference_SkClipOp:
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
                            element->setOp(kReplace_SkClipOp);
                        }
                    }
                    break;
                case kReplace_SkClipOp:
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
                fMaskElements.popHead();
                element = fMaskElements.headIter().get();
            }
        }
    }
    fMaskRequiresAA = numAAElements > 0;

    SkASSERT(InitialTriState::kUnknown != initialTriState);
    fInitialState = static_cast<GrReducedClip::InitialState>(initialTriState);
}

GrReducedClip::ClipResult GrReducedClip::clipInsideElement(const Element* element) {
    SkASSERT(element->getDeviceSpaceType() != Element::DeviceSpaceType::kShader);

    SkIRect elementIBounds;
    if (!element->isAA()) {
        element->getBounds().round(&elementIBounds);
    } else {
        elementIBounds = GrClip::GetPixelIBounds(element->getBounds());
    }
    SkASSERT(fHasScissor);
    if (!fScissor.intersect(elementIBounds)) {
        this->makeEmpty();
        return ClipResult::kMadeEmpty;
    }

    switch (element->getDeviceSpaceType()) {
        case Element::DeviceSpaceType::kEmpty:
            return ClipResult::kMadeEmpty;

        case Element::DeviceSpaceType::kRect:
            SkASSERT(element->getBounds() == element->getDeviceSpaceRect());
            SkASSERT(!element->isInverseFilled());
            if (element->isAA()) {
                if (SK_InvalidGenID == fAAClipRectGenID) { // No AA clip rect yet?
                    fAAClipRect = element->getDeviceSpaceRect();
                    // fAAClipRectGenID is the value we should use for fMaskGenID if we end up
                    // moving the AA clip rect into the mask. The mask GenID is simply the topmost
                    // element's GenID. And since we walk the stack backwards, this means it's just
                    // the first element we don't skip during our walk.
                    fAAClipRectGenID = fMaskElements.isEmpty() ? element->getGenID() : fMaskGenID;
                    SkASSERT(SK_InvalidGenID != fAAClipRectGenID);
                } else if (!fAAClipRect.intersect(element->getDeviceSpaceRect())) {
                    this->makeEmpty();
                    return ClipResult::kMadeEmpty;
                }
            }
            return ClipResult::kClipped;

        case Element::DeviceSpaceType::kRRect:
            SkASSERT(!element->isInverseFilled());
            return this->addAnalyticRRect(element->getDeviceSpaceRRect(), Invert::kNo,
                                          GrAA(element->isAA()));

        case Element::DeviceSpaceType::kPath:
            return this->addAnalyticPath(element->getDeviceSpacePath(),
                                         Invert(element->isInverseFilled()), GrAA(element->isAA()));

        case Element::DeviceSpaceType::kShader:
            SkUNREACHABLE;
    }

    SK_ABORT("Unexpected DeviceSpaceType");
}

GrReducedClip::ClipResult GrReducedClip::clipOutsideElement(const Element* element) {
    SkASSERT(element->getDeviceSpaceType() != Element::DeviceSpaceType::kShader);

    switch (element->getDeviceSpaceType()) {
        case Element::DeviceSpaceType::kEmpty:
            return ClipResult::kMadeEmpty;

        case Element::DeviceSpaceType::kRect:
            SkASSERT(!element->isInverseFilled());
            if (fWindowRects.count() < fMaxWindowRectangles) {
                // Clip out the inside of every rect. We won't be able to entirely skip the AA ones,
                // but it saves processing time.
                this->addWindowRectangle(element->getDeviceSpaceRect(), element->isAA());
                if (!element->isAA()) {
                    return ClipResult::kClipped;
                }
            }
            return this->addAnalyticRect(element->getDeviceSpaceRect(), Invert::kYes,
                                         GrAA(element->isAA()));

        case Element::DeviceSpaceType::kRRect: {
            SkASSERT(!element->isInverseFilled());
            const SkRRect& clipRRect = element->getDeviceSpaceRRect();
            ClipResult clipResult = this->addAnalyticRRect(clipRRect, Invert::kYes,
                                                           GrAA(element->isAA()));
            if (fWindowRects.count() >= fMaxWindowRectangles) {
                return clipResult;
            }

            // Clip out the interiors of round rects with two window rectangles in the shape of a
            // "plus". This doesn't let us skip the clip element, but still saves processing time.
            SkVector insetTL = clipRRect.radii(SkRRect::kUpperLeft_Corner);
            SkVector insetBR = clipRRect.radii(SkRRect::kLowerRight_Corner);
            if (SkRRect::kComplex_Type == clipRRect.getType()) {
                const SkVector& insetTR = clipRRect.radii(SkRRect::kUpperRight_Corner);
                const SkVector& insetBL = clipRRect.radii(SkRRect::kLowerLeft_Corner);
                insetTL.fX = std::max(insetTL.x(), insetBL.x());
                insetTL.fY = std::max(insetTL.y(), insetTR.y());
                insetBR.fX = std::max(insetBR.x(), insetTR.x());
                insetBR.fY = std::max(insetBR.y(), insetBL.y());
            }
            const SkRect& bounds = clipRRect.getBounds();
            if (insetTL.x() + insetBR.x() >= bounds.width() ||
                insetTL.y() + insetBR.y() >= bounds.height()) {
                return clipResult; // The interior "plus" is empty.
            }

            SkRect horzRect = SkRect::MakeLTRB(bounds.left(), bounds.top() + insetTL.y(),
                                               bounds.right(), bounds.bottom() - insetBR.y());
            this->addWindowRectangle(horzRect, element->isAA());

            if (fWindowRects.count() < fMaxWindowRectangles) {
                SkRect vertRect = SkRect::MakeLTRB(bounds.left() + insetTL.x(), bounds.top(),
                                                   bounds.right() - insetBR.x(), bounds.bottom());
                this->addWindowRectangle(vertRect, element->isAA());
            }

            return clipResult;
        }

        case Element::DeviceSpaceType::kPath:
            return this->addAnalyticPath(element->getDeviceSpacePath(),
                                         Invert(!element->isInverseFilled()),
                                         GrAA(element->isAA()));

        case Element::DeviceSpaceType::kShader:
            SkUNREACHABLE;
    }

    SK_ABORT("Unexpected DeviceSpaceType");
}

inline void GrReducedClip::addWindowRectangle(const SkRect& elementInteriorRect, bool elementIsAA) {
    SkIRect window;
    if (!elementIsAA) {
        elementInteriorRect.round(&window);
    } else {
        elementInteriorRect.roundIn(&window);
    }
    if (!window.isEmpty()) { // Skip very thin windows that round to zero or negative dimensions.
        fWindowRects.addWindow(window);
    }
}

GrClipEdgeType GrReducedClip::GetClipEdgeType(Invert invert, GrAA aa) {
    if (Invert::kNo == invert) {
        return (GrAA::kYes == aa) ? GrClipEdgeType::kFillAA : GrClipEdgeType::kFillBW;
    } else {
        return (GrAA::kYes == aa) ? GrClipEdgeType::kInverseFillAA : GrClipEdgeType::kInverseFillBW;
    }
}

GrReducedClip::ClipResult GrReducedClip::addAnalyticRect(const SkRect& deviceSpaceRect,
                                                         Invert invert, GrAA aa) {
    if (this->numAnalyticElements() >= fMaxAnalyticElements) {
        return ClipResult::kNotClipped;
    }

    fAnalyticFP = GrAARectEffect::Make(std::move(fAnalyticFP), GetClipEdgeType(invert, aa),
                                       deviceSpaceRect);

    SkASSERT(fAnalyticFP != nullptr);
    ++fNumAnalyticElements;

    return ClipResult::kClipped;
}

GrReducedClip::ClipResult GrReducedClip::addAnalyticRRect(const SkRRect& deviceSpaceRRect,
                                                          Invert invert, GrAA aa) {
    if (this->numAnalyticElements() >= fMaxAnalyticElements) {
        return ClipResult::kNotClipped;
    }

    // Combine this analytic effect with the previous effect in the stack.
    bool success;
    std::tie(success, fAnalyticFP) = GrRRectEffect::Make(std::move(fAnalyticFP),
                                                         GetClipEdgeType(invert, aa),
                                                         deviceSpaceRRect, *fCaps->shaderCaps());
    if (success) {
        ++fNumAnalyticElements;
        return ClipResult::kClipped;
    }

    SkPathBuilder deviceSpacePath;
    deviceSpacePath.setIsVolatile(true);
    deviceSpacePath.addRRect(deviceSpaceRRect);
    return this->addAnalyticPath(deviceSpacePath.detach(), invert, aa);
}

GrReducedClip::ClipResult GrReducedClip::addAnalyticPath(const SkPath& deviceSpacePath,
                                                         Invert invert, GrAA aa) {
    if (this->numAnalyticElements() >= fMaxAnalyticElements) {
        return ClipResult::kNotClipped;
    }

    // Combine this analytic effect with the previous effect in the stack.
    bool success;
    std::tie(success, fAnalyticFP) = GrConvexPolyEffect::Make(std::move(fAnalyticFP),
                                                              GetClipEdgeType(invert, aa),
                                                              deviceSpacePath);
    if (success) {
        ++fNumAnalyticElements;
        return ClipResult::kClipped;
    }

    if (fCCPRClipPaths.count() < fMaxCCPRClipPaths && GrAA::kYes == aa) {
        const SkRect& bounds = deviceSpacePath.getBounds();
        if (bounds.height() * bounds.width() <= GrCoverageCountingPathRenderer::kMaxClipPathArea) {
            // Set aside CCPR paths for later. We will create their clip FPs once we know the ID of
            // the opsTask they will operate in.
            SkPath& ccprClipPath = fCCPRClipPaths.push_back(deviceSpacePath);
            if (Invert::kYes == invert) {
                ccprClipPath.toggleInverseFillType();
            }
            return ClipResult::kClipped;
        }
    }

    return ClipResult::kNotClipped;
}

void GrReducedClip::makeEmpty() {
    fHasScissor = false;
    fAAClipRectGenID = SK_InvalidGenID;
    fWindowRects.reset();
    fMaskElements.reset();
    fShader.reset();
    fInitialState = InitialState::kAllOut;
    fAnalyticFP = nullptr;
    fNumAnalyticElements = 0;
    fCCPRClipPaths.reset();
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static bool stencil_element(GrSurfaceDrawContext* rtc,
                            const GrFixedClip& clip,
                            const GrUserStencilSettings* ss,
                            const SkMatrix& viewMatrix,
                            const SkClipStack::Element* element) {
    GrAA aa = GrAA(element->isAA());
    switch (element->getDeviceSpaceType()) {
        case SkClipStack::Element::DeviceSpaceType::kEmpty:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case SkClipStack::Element::DeviceSpaceType::kRect: {
            GrPaint paint;
            paint.setCoverageSetOpXPFactory((SkRegion::Op)element->getOp(),
                                            element->isInverseFilled());
            rtc->stencilRect(&clip, ss, std::move(paint), aa, viewMatrix,
                             element->getDeviceSpaceRect());
            return true;
        }
        default: {
            SkPath path;
            element->asDeviceSpacePath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            return rtc->drawAndStencilPath(&clip, ss, (SkRegion::Op)element->getOp(),
                                           element->isInverseFilled(), aa, viewMatrix, path);
        }
    }

    return false;
}

static void draw_element(GrSurfaceDrawContext* rtc,
                         const GrClip& clip,  // TODO: can this just always be WideOpen?
                         GrPaint&& paint,
                         GrAA aa,
                         const SkMatrix& viewMatrix,
                         const SkClipStack::Element* element) {
    // TODO: Draw rrects directly here.
    switch (element->getDeviceSpaceType()) {
        case SkClipStack::Element::DeviceSpaceType::kEmpty:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case SkClipStack::Element::DeviceSpaceType::kRect:
            rtc->drawRect(&clip, std::move(paint), aa, viewMatrix, element->getDeviceSpaceRect());
            break;
        default: {
            SkPath path;
            element->asDeviceSpacePath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            rtc->drawPath(&clip, std::move(paint), aa, viewMatrix, path, GrStyle::SimpleFill());
            break;
        }
    }
}

bool GrReducedClip::drawAlphaClipMask(GrSurfaceDrawContext* rtc) const {
    // The texture may be larger than necessary, this rect represents the part of the texture
    // we populate with a rasterization of the clip.
    GrFixedClip clip(rtc->dimensions(), SkIRect::MakeWH(fScissor.width(), fScissor.height()));

    if (!fWindowRects.empty()) {
        clip.setWindowRectangles(fWindowRects.makeOffset(-fScissor.left(), -fScissor.top()),
                                 GrWindowRectsState::Mode::kExclusive);
    }

    // The scratch texture that we are drawing into can be substantially larger than the mask. Only
    // clear the part that we care about.
    SkPMColor4f initialCoverage =
        InitialState::kAllIn == this->initialState() ? SK_PMColor4fWHITE : SK_PMColor4fTRANSPARENT;
    if (clip.hasWindowRectangles()) {
        GrPaint paint;
        paint.setColor4f(initialCoverage);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        rtc->drawRect(&clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                      SkRect::Make(clip.scissorRect()));
    } else {
        rtc->clearAtLeast(clip.scissorRect(), initialCoverage);
    }

    // Set the matrix so that rendered clip elements are transformed to mask space from clip space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-fScissor.left()), SkIntToScalar(-fScissor.top()));

    // walk through each clip element and perform its set op
    for (ElementList::Iter iter(fMaskElements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = (SkRegion::Op)element->getOp();
        GrAA aa = GrAA(element->isAA());
        bool invert = element->isInverseFilled();
        if (invert || SkRegion::kIntersect_Op == op || SkRegion::kReverseDifference_Op == op) {
            // draw directly into the result with the stencil set to make the pixels affected
            // by the clip shape be non-zero.
            static constexpr GrUserStencilSettings kStencilInElement(
                 GrUserStencilSettings::StaticInit<
                     0xffff,
                     GrUserStencilTest::kAlways,
                     0xffff,
                     GrUserStencilOp::kReplace,
                     GrUserStencilOp::kReplace,
                     0xffff>()
            );
            if (!stencil_element(rtc, clip, &kStencilInElement, translate, element)) {
                return false;
            }

            // Draw to the exterior pixels (those with a zero stencil value).
            static constexpr GrUserStencilSettings kDrawOutsideElement(
                 GrUserStencilSettings::StaticInit<
                     0x0000,
                     GrUserStencilTest::kEqual,
                     0xffff,
                     GrUserStencilOp::kZero,
                     GrUserStencilOp::kZero,
                     0xffff>()
            );

            GrPaint paint;
            paint.setCoverageSetOpXPFactory(op, !invert);
            rtc->stencilRect(&clip, &kDrawOutsideElement, std::move(paint), GrAA::kNo, translate,
                             SkRect::Make(fScissor));
        } else {
            // all the remaining ops can just be directly draw into the accumulation buffer
            GrPaint paint;
            paint.setCoverageSetOpXPFactory(op, false);

            draw_element(rtc, clip, std::move(paint), aa, translate, element);
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 1-bit clip mask in the stencil buffer.

bool GrReducedClip::drawStencilClipMask(GrRecordingContext* context,
                                        GrSurfaceDrawContext* surfaceDrawContext) const {
    GrStencilMaskHelper helper(context, surfaceDrawContext);
    if (!helper.init(fScissor, this->maskGenID(), fWindowRects, this->numAnalyticElements())) {
        // The stencil mask doesn't need updating
        return true;
    }

    helper.clear(InitialState::kAllIn == this->initialState());

    // walk through each clip element and perform its set op with the existing clip.
    for (ElementList::Iter iter(fMaskElements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = (SkRegion::Op)element->getOp();
        GrAA aa = element->isAA() ? GrAA::kYes : GrAA::kNo;

        if (Element::DeviceSpaceType::kRect == element->getDeviceSpaceType()) {
            helper.drawRect(element->getDeviceSpaceRect(), SkMatrix::I(), op, aa);
        } else {
            SkPath path;
            element->asDeviceSpacePath(&path);
            if (!helper.drawPath(path, SkMatrix::I(), op, aa)) {
                return false;
            }
        }
    }

    helper.finish();
    return true;
}

int GrReducedClip::numAnalyticElements() const {
    return fCCPRClipPaths.size() + fNumAnalyticElements;
}

std::unique_ptr<GrFragmentProcessor> GrReducedClip::finishAndDetachAnalyticElements(
        GrRecordingContext* context, const SkMatrixProvider& matrixProvider,
        GrCoverageCountingPathRenderer* ccpr, uint32_t opsTaskID) {
    // Combine the analytic FP with any CCPR clip processors.
    std::unique_ptr<GrFragmentProcessor> clipFP = std::move(fAnalyticFP);
    fNumAnalyticElements = 0;

    for (const SkPath& ccprClipPath : fCCPRClipPaths) {
        SkASSERT(ccpr);
        SkASSERT(fHasScissor);
        clipFP = ccpr->makeClipProcessor(std::move(clipFP), opsTaskID, ccprClipPath,
                                         fScissor, *fCaps);
    }
    fCCPRClipPaths.reset();

    // Create the shader.
    std::unique_ptr<GrFragmentProcessor> shaderFP;
    if (fShader != nullptr) {
        static const GrColorInfo kCoverageColorInfo{GrColorType::kUnknown, kPremul_SkAlphaType,
                                                    nullptr};
        GrFPArgs args(context, matrixProvider, SkSamplingOptions(), &kCoverageColorInfo);
        shaderFP = as_SB(fShader)->asFragmentProcessor(args);
        if (shaderFP != nullptr) {
            shaderFP = GrFragmentProcessor::MulInputByChildAlpha(std::move(shaderFP));
        }
    }

    // Compose the clip and shader FPs.
    return GrFragmentProcessor::Compose(std::move(shaderFP), std::move(clipFP));
}
