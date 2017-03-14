/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrReducedClip.h"

#include "GrAppliedClip.h"
#include "GrClip.h"
#include "GrColor.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrDrawingManager.h"
#include "GrFixedClip.h"
#include "GrPathRenderer.h"
#include "GrStencilSettings.h"
#include "GrStyle.h"
#include "GrUserStencilSettings.h"
#include "SkClipOpPriv.h"

typedef SkClipStack::Element Element;

/**
 * There are plenty of optimizations that could be added here. Maybe flips could be folded into
 * earlier operations. Or would inserting flips and reversing earlier ops ever be a win? Perhaps
 * for the case where the bounds are kInsideOut_BoundsType. We could restrict earlier operations
 * based on later intersect operations, and perhaps remove intersect-rects. We could optionally
 * take a rect in case the caller knows a bound on what is to be drawn through this clip.
 */
GrReducedClip::GrReducedClip(const SkClipStack& stack, const SkRect& queryBounds,
                             int maxWindowRectangles) {
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
        fElements.addToHead(stackBounds, kReplace_SkClipOp, true/*doAA*/);
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
    this->walkStack(stack, tighterQuery, maxWindowRectangles);

    if (fWindowRects.count() < maxWindowRectangles) {
        this->addInteriorWindowRectangles(maxWindowRectangles);
    }
}

void GrReducedClip::walkStack(const SkClipStack& stack, const SkRect& queryBounds,
                              int maxWindowRectangles) {
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
            case kDifference_SkClipOp:
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
                    } else if (fWindowRects.count() < maxWindowRectangles && !embiggens &&
                               !element->isAA() && Element::kRect_Type == element->getType()) {
                        this->addWindowRectangle(element->getRect(), false);
                        skippable = true;
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
                SkASSERT(kXOR_SkClipOp == element->getOp() ||
                         kReverseDifference_SkClipOp == element->getOp());
                fElements.addToHead(SkRect::Make(fIBounds), kReverseDifference_SkClipOp, false);
            } else {
                Element* newElement = fElements.addToHead(*element);
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
        fElements.reset();
        numAAElements = 0;
    } else {
        Element* element = fElements.headIter().get();
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
                fElements.popHead();
                element = fElements.headIter().get();
            }
        }
    }
    fRequiresAA = numAAElements > 0;

    SkASSERT(InitialTriState::kUnknown != initialTriState);
    fInitialState = static_cast<GrReducedClip::InitialState>(initialTriState);
}

static bool element_is_pure_subtract(SkClipOp op) {
    SkASSERT(static_cast<int>(op) >= 0);
    return static_cast<int>(op) <= static_cast<int>(kIntersect_SkClipOp);

    GR_STATIC_ASSERT(0 == static_cast<int>(kDifference_SkClipOp));
    GR_STATIC_ASSERT(1 == static_cast<int>(kIntersect_SkClipOp));
}

void GrReducedClip::addInteriorWindowRectangles(int maxWindowRectangles) {
    SkASSERT(fWindowRects.count() < maxWindowRectangles);
    // Walk backwards through the element list and add window rectangles to the interiors of
    // "difference" elements. Quit if we encounter an element that may grow the clip.
    ElementList::Iter iter(fElements, ElementList::Iter::kTail_IterStart);
    for (; iter.get() && element_is_pure_subtract(iter.get()->getOp()); iter.prev()) {
        const Element* element = iter.get();
        if (kDifference_SkClipOp != element->getOp()) {
            continue;
        }

        if (Element::kRect_Type == element->getType()) {
            SkASSERT(element->isAA());
            this->addWindowRectangle(element->getRect(), true);
            if (fWindowRects.count() >= maxWindowRectangles) {
                return;
            }
            continue;
        }

        if (Element::kRRect_Type == element->getType()) {
            // For round rects we add two overlapping windows in the shape of a plus.
            const SkRRect& clipRRect = element->getRRect();
            SkVector insetTL = clipRRect.radii(SkRRect::kUpperLeft_Corner);
            SkVector insetBR = clipRRect.radii(SkRRect::kLowerRight_Corner);
            if (SkRRect::kComplex_Type == clipRRect.getType()) {
                const SkVector& insetTR = clipRRect.radii(SkRRect::kUpperRight_Corner);
                const SkVector& insetBL = clipRRect.radii(SkRRect::kLowerLeft_Corner);
                insetTL.fX = SkTMax(insetTL.x(), insetBL.x());
                insetTL.fY = SkTMax(insetTL.y(), insetTR.y());
                insetBR.fX = SkTMax(insetBR.x(), insetTR.x());
                insetBR.fY = SkTMax(insetBR.y(), insetBL.y());
            }
            const SkRect& bounds = clipRRect.getBounds();
            if (insetTL.x() + insetBR.x() >= bounds.width() ||
                insetTL.y() + insetBR.y() >= bounds.height()) {
                continue; // The interior "plus" is empty.
            }

            SkRect horzRect = SkRect::MakeLTRB(bounds.left(), bounds.top() + insetTL.y(),
                                               bounds.right(), bounds.bottom() - insetBR.y());
            this->addWindowRectangle(horzRect, element->isAA());
            if (fWindowRects.count() >= maxWindowRectangles) {
                return;
            }

            SkRect vertRect = SkRect::MakeLTRB(bounds.left() + insetTL.x(), bounds.top(),
                                               bounds.right() - insetBR.x(), bounds.bottom());
            this->addWindowRectangle(vertRect, element->isAA());
            if (fWindowRects.count() >= maxWindowRectangles) {
                return;
            }
            continue;
        }
    }
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

inline bool GrReducedClip::intersectIBounds(const SkIRect& irect) {
    SkASSERT(fHasIBounds);
    if (!fIBounds.intersect(irect)) {
        fHasIBounds = false;
        fWindowRects.reset();
        fElements.reset();
        fRequiresAA = false;
        fInitialState = InitialState::kAllOut;
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Create a 8-bit clip mask in alpha

static bool stencil_element(GrRenderTargetContext* rtc,
                            const GrFixedClip& clip,
                            const GrUserStencilSettings* ss,
                            const SkMatrix& viewMatrix,
                            const SkClipStack::Element* element) {
    GrAA aa = GrBoolToAA(element->isAA());
    switch (element->getType()) {
        case Element::kEmpty_Type:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case Element::kRect_Type:
            return rtc->priv().drawAndStencilRect(clip, ss,
                                                  (SkRegion::Op)element->getOp(),
                                                  element->isInverseFilled(), aa, viewMatrix,
                                                  element->getRect());
            break;
        default: {
            SkPath path;
            element->asPath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            return rtc->priv().drawAndStencilPath(clip, ss, (SkRegion::Op)element->getOp(),
                                                  element->isInverseFilled(), aa, viewMatrix, path);
            break;
        }
    }

    return false;
}

static void draw_element(GrRenderTargetContext* rtc,
                         const GrClip& clip,  // TODO: can this just always be WideOpen?
                         GrPaint&& paint,
                         GrAA aa,
                         const SkMatrix& viewMatrix,
                         const SkClipStack::Element* element) {
    // TODO: Draw rrects directly here.
    switch (element->getType()) {
        case Element::kEmpty_Type:
            SkDEBUGFAIL("Should never get here with an empty element.");
            break;
        case Element::kRect_Type:
            rtc->drawRect(clip, std::move(paint), aa, viewMatrix, element->getRect());
            break;
        default: {
            SkPath path;
            element->asPath(&path);
            if (path.isInverseFillType()) {
                path.toggleInverseFillType();
            }

            rtc->drawPath(clip, std::move(paint), aa, viewMatrix, path, GrStyle::SimpleFill());
            break;
        }
    }
}

bool GrReducedClip::drawAlphaClipMask(GrRenderTargetContext* rtc) const {
    // The texture may be larger than necessary, this rect represents the part of the texture
    // we populate with a rasterization of the clip.
    GrFixedClip clip(SkIRect::MakeWH(fIBounds.width(), fIBounds.height()));

    if (!fWindowRects.empty()) {
        clip.setWindowRectangles(fWindowRects.makeOffset(-fIBounds.left(), -fIBounds.top()),
                                 GrWindowRectsState::Mode::kExclusive);
    }

    // The scratch texture that we are drawing into can be substantially larger than the mask. Only
    // clear the part that we care about.
    GrColor initialCoverage = InitialState::kAllIn == this->initialState() ? -1 : 0;
    rtc->priv().clear(clip, initialCoverage, true);

    // Set the matrix so that rendered clip elements are transformed to mask space from clip space.
    SkMatrix translate;
    translate.setTranslate(SkIntToScalar(-fIBounds.left()), SkIntToScalar(-fIBounds.top()));

    // walk through each clip element and perform its set op
    for (ElementList::Iter iter(fElements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        SkRegion::Op op = (SkRegion::Op)element->getOp();
        GrAA aa = GrBoolToAA(element->isAA());
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
            if (!rtc->priv().drawAndStencilRect(clip, &kDrawOutsideElement, op, !invert, GrAA::kNo,
                                                translate, SkRect::Make(fIBounds))) {
                return false;
            }
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

class StencilClip final : public GrClip {
public:
    StencilClip(const SkIRect& scissorRect) : fFixedClip(scissorRect) {}
    const GrFixedClip& fixedClip() const { return fFixedClip; }

    void setWindowRectangles(const GrWindowRectangles& windows, GrWindowRectsState::Mode mode) {
        fFixedClip.setWindowRectangles(windows, mode);
    }

private:
    bool quickContains(const SkRect&) const override {
        return false;
    }
    void getConservativeBounds(int width, int height, SkIRect* bounds, bool* iior) const override {
        fFixedClip.getConservativeBounds(width, height, bounds, iior);
    }
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA*) const override {
        return false;
    }
    bool apply(GrContext* context, GrRenderTargetContext* renderTargetContext, bool useHWAA,
               bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const override {
        if (!fFixedClip.apply(context, renderTargetContext, useHWAA, hasUserStencilSettings, out,
                              bounds)) {
            return false;
        }
        out->addStencilClip();
        return true;
    }

    GrFixedClip fFixedClip;

    typedef GrClip INHERITED;
};

bool GrReducedClip::drawStencilClipMask(GrContext* context,
                                        GrRenderTargetContext* renderTargetContext) const {
    // We set the current clip to the bounds so that our recursive draws are scissored to them.
    StencilClip stencilClip(fIBounds);

    if (!fWindowRects.empty()) {
        stencilClip.setWindowRectangles(fWindowRects, GrWindowRectsState::Mode::kExclusive);
    }

    bool initialState = InitialState::kAllIn == this->initialState();
    renderTargetContext->priv().clearStencilClip(stencilClip.fixedClip(), initialState);

    // walk through each clip element and perform its set op with the existing clip.
    for (ElementList::Iter iter(fElements); iter.get(); iter.next()) {
        const Element* element = iter.get();
        GrAAType aaType = GrAAType::kNone;
        if (element->isAA() && renderTargetContext->isStencilBufferMultisampled()) {
            aaType = GrAAType::kMSAA;
        }

        bool fillInverted = false;

        // This will be used to determine whether the clip shape can be rendered into the
        // stencil with arbitrary stencil settings.
        GrPathRenderer::StencilSupport stencilSupport;

        SkRegion::Op op = (SkRegion::Op)element->getOp();

        GrPathRenderer* pr = nullptr;
        SkPath clipPath;
        if (Element::kRect_Type == element->getType()) {
            stencilSupport = GrPathRenderer::kNoRestriction_StencilSupport;
            fillInverted = false;
        } else {
            element->asPath(&clipPath);
            fillInverted = clipPath.isInverseFillType();
            if (fillInverted) {
                clipPath.toggleInverseFillType();
            }

            GrShape shape(clipPath, GrStyle::SimpleFill());
            GrPathRenderer::CanDrawPathArgs canDrawArgs;
            canDrawArgs.fShaderCaps = context->caps()->shaderCaps();
            canDrawArgs.fViewMatrix = &SkMatrix::I();
            canDrawArgs.fShape = &shape;
            canDrawArgs.fAAType = aaType;
            canDrawArgs.fHasUserStencilSettings = false;

            GrDrawingManager* dm = context->contextPriv().drawingManager();
            pr = dm->getPathRenderer(canDrawArgs, false, GrPathRendererChain::DrawType::kStencil,
                                     &stencilSupport);
            if (!pr) {
                return false;
            }
        }

        bool canRenderDirectToStencil =
            GrPathRenderer::kNoRestriction_StencilSupport == stencilSupport;
        bool drawDirectToClip; // Given the renderer, the element,
                               // fill rule, and set operation should
                               // we render the element directly to
                               // stencil bit used for clipping.
        GrUserStencilSettings const* const* stencilPasses =
            GrStencilSettings::GetClipPasses(op, canRenderDirectToStencil, fillInverted,
                                             &drawDirectToClip);

        // draw the element to the client stencil bits if necessary
        if (!drawDirectToClip) {
            static constexpr GrUserStencilSettings kDrawToStencil(
                 GrUserStencilSettings::StaticInit<
                     0x0000,
                     GrUserStencilTest::kAlways,
                     0xffff,
                     GrUserStencilOp::kIncMaybeClamp,
                     GrUserStencilOp::kIncMaybeClamp,
                     0xffff>()
            );
            if (Element::kRect_Type == element->getType()) {
                renderTargetContext->priv().stencilRect(stencilClip.fixedClip(), &kDrawToStencil,
                                                        aaType, SkMatrix::I(), element->getRect());
            } else {
                if (!clipPath.isEmpty()) {
                    GrShape shape(clipPath, GrStyle::SimpleFill());
                    if (canRenderDirectToStencil) {
                        GrPaint paint;
                        paint.setXPFactory(GrDisableColorXPFactory::Get());

                        GrPathRenderer::DrawPathArgs args{context,
                                                          std::move(paint),
                                                          &kDrawToStencil,
                                                          renderTargetContext,
                                                          &stencilClip.fixedClip(),
                                                          &SkMatrix::I(),
                                                          &shape,
                                                          aaType,
                                                          false};
                        pr->drawPath(args);
                    } else {
                        GrPathRenderer::StencilPathArgs args;
                        args.fContext = context;
                        args.fRenderTargetContext = renderTargetContext;
                        args.fClip = &stencilClip.fixedClip();
                        args.fViewMatrix = &SkMatrix::I();
                        args.fAAType = aaType;
                        args.fShape = &shape;
                        pr->stencilPath(args);
                    }
                }
            }
        }

        // now we modify the clip bit by rendering either the clip
        // element directly or a bounding rect of the entire clip.
        for (GrUserStencilSettings const* const* pass = stencilPasses; *pass; ++pass) {
            if (drawDirectToClip) {
                if (Element::kRect_Type == element->getType()) {
                    renderTargetContext->priv().stencilRect(stencilClip, *pass, aaType,
                                                            SkMatrix::I(), element->getRect());
                } else {
                    GrShape shape(clipPath, GrStyle::SimpleFill());
                    GrPaint paint;
                    paint.setXPFactory(GrDisableColorXPFactory::Get());
                    GrPathRenderer::DrawPathArgs args{context,
                                                      std::move(paint),
                                                      *pass,
                                                      renderTargetContext,
                                                      &stencilClip,
                                                      &SkMatrix::I(),
                                                      &shape,
                                                      aaType,
                                                      false};
                    pr->drawPath(args);
                }
            } else {
                // The view matrix is setup to do clip space -> stencil space translation, so
                // draw rect in clip space.
                renderTargetContext->priv().stencilRect(stencilClip, *pass, aaType, SkMatrix::I(),
                                                        SkRect::Make(fIBounds));
            }
        }
    }
    return true;
}
