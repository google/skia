/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ClipStack_graphite.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"

namespace skgpu {

namespace {

Rect subtract(const Rect& a, const Rect& b, bool exact) {
    SkRect diff;
    if (SkRectPriv::Subtract(a.asSkRect(), b.asSkRect(), &diff) || !exact) {
        // Either A-B is exactly the rectangle stored in diff, or we don't need an exact answer
        // and can settle for the subrect of A excluded from B (which is also 'diff')
        return Rect{diff};
    } else {
        // For our purposes, we want the original A when A-B cannot be exactly represented
        return a;
    }
}

static const Transform kIdentity{SkM44()};

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
// ClipStack::TransformedShape

// A flyweight object describing geometry, subject to a local-to-device transform.
// This can be used by SaveRecords, Elements, and draws to determine how two shape operations
// interact with each other, without needing to share a base class, friend each other, or have a
// template for each combination of two types.
struct ClipStack::TransformedShape {
    const Transform& fLocalToDevice;
    const Shape&     fShape;
    const Rect&      fOuterBounds;
    const Rect&      fInnerBounds;

    SkClipOp         fOp;

    // contains() performs a fair amount of work to be as accurate as possible since it can mean
    // greatly simplifying the clip stack. However, in some contexts this isn't worth doing because
    // the actual shape is only an approximation (save records), or there's no current way to take
    // advantage of knowing this shape contains another (draws containing a clip hypothetically
    // could replace their geometry to draw the clip directly, but that isn't implemented now).
    bool fContainsChecksOnlyBounds = false;

    bool intersects(const TransformedShape&) const;
    bool contains(const TransformedShape&) const;
};

bool ClipStack::TransformedShape::intersects(const TransformedShape& o) const {
    if (!fOuterBounds.intersects(o.fOuterBounds)) {
        return false;
    }
    if (fLocalToDevice == o.fLocalToDevice) {
        // Since the two shape's local coordinate spaces are the same, we can compare shape
        // bounds directly for a more accurate intersection test. We intentionally do not go
        // further and do shape-specific intersection tests since these could have unknown
        // complexity (for paths) and limited utility (e.g. two round rects that are disjoint
        // solely from their corner curves).
        return fShape.bounds().intersects(o.fShape.bounds());
    } else if (fLocalToDevice.type() > Transform::Type::kRectStaysRect ||
               o.fLocalToDevice.type() > Transform::Type::kRectStaysRect) {
        // The shapes don't share the same coordinate system, and their approximate 'outer'
        // bounds in device space could have substantial outsetting to contain the transformed
        // shape (e.g. 45 degree rotation). This makes it worth mapping the corners of o'
        // into this shape's local space for a more accurate test.
        Rect bounds = fShape.bounds();
        SkV4 deviceQuad[4];
        o.fLocalToDevice.mapPoints(o.fShape.bounds(), deviceQuad);
        SkV4 localQuad[4];
        fLocalToDevice.inverseMapPoints(deviceQuad, localQuad, 4);
        for (int i = 0; i < 4; ++i) {
            // TODO: Would be nice to make this consistent with how the GPU clips NDC w.
            if (deviceQuad[i].w < SkPathPriv::kW0PlaneDistance ||
                localQuad[i].w < SkPathPriv::kW0PlaneDistance) {
                // Something in 'o' actually projects behind the W = 0 plane and would be
                // clipped to infinity, so pessimistically assume that they could intersect.
                return true;
            }
            if (bounds.contains(Rect::Point(float2::Load(localQuad + i) / localQuad[i].w))) {
                // If any corner of 'o's bounds are contained then it intersects our bounds
                return true;
            }
        }
        // Else no corners of 'o's bounds are inside our bounds, so no intersection
        return false;
    }

    // Else the two shape's coordinate spaces are different but both rect-stays-rect or simpler.
    // This means, though, that their outer bounds approximations are tight to their transormed
    // shape bounds. There's no point to do further tests given that and that we already found
    // that these outer bounds *do* intersect.
    return true;
}

bool ClipStack::TransformedShape::contains(const TransformedShape& o) const {
    if (fInnerBounds.contains(o.fOuterBounds)) {
        return true;
    }
    if (fContainsChecksOnlyBounds) {
        return false; // don't do any more work
    }

    if (fLocalToDevice == o.fLocalToDevice) {
        // Test the shapes directly against each other, with a special check for a rrect+rrect
        // containment (a intersect b == a implies b contains a) and paths (same gen ID, or same
        // path for small paths means they contain each other).
        static constexpr int kMaxPathComparePoints = 16;
        if (fShape.isRRect() && o.fShape.isRRect()) {
            return SkRRectPriv::ConservativeIntersect(fShape.rrect(), o.fShape.rrect())
                    == o.fShape.rrect();
        } else if (fShape.isPath() && o.fShape.isPath()) {
            // TODO: Is this worth doing still if clips only cost as much as a single draw?
            return (fShape.path().getGenerationID() == o.fShape.path().getGenerationID()) ||
                    (fShape.path().countPoints() <= kMaxPathComparePoints &&
                    fShape.path() == o.fShape.path());
        } else {
            return fShape.conservativeContains(o.fShape.bounds());
        }
    } else if (fLocalToDevice.type() <= Transform::Type::kRectStaysRect &&
               o.fLocalToDevice.type() <= Transform::Type::kRectStaysRect) {
        // Optimize the common case where o's bounds can be mapped tightly into this coordinate
        // space and then tested against our shape.
        Rect localBounds = fLocalToDevice.inverseMapRect(
                o.fLocalToDevice.mapRect(o.fShape.bounds()));
        return fShape.conservativeContains(localBounds);
    } else if (fShape.convex()) {
        // Since this shape is convex, if all four corners of o's bounding box are inside it
        // then the entirety of o is also guaranteed to be inside it.
        SkV4 deviceQuad[4];
        o.fLocalToDevice.mapPoints(o.fShape.bounds(), deviceQuad);
        SkV4 localQuad[4];
        fLocalToDevice.inverseMapPoints(deviceQuad, localQuad, 4);
        for (int i = 0; i < 4; ++i) {
            // TODO: Would be nice to make this consistent with how the GPU clips NDC w.
            if (deviceQuad[i].w < SkPathPriv::kW0PlaneDistance ||
                localQuad[i].w < SkPathPriv::kW0PlaneDistance) {
                // Something in O actually projects behind the W = 0 plane and would be clipped
                // to infinity, so it's extremely unlikely that this contains O.
                return false;
            }
            if (!fShape.conservativeContains(float2::Load(localQuad + i) / localQuad[i].w)) {
                return false;
            }
        }
        return true;
    }

    // Else not an easily comparable pair of shapes so assume this doesn't contain O
    return false;
}

ClipStack::SimplifyResult ClipStack::Simplify(const TransformedShape& a,
                                              const TransformedShape& b) {
    enum class ClipCombo {
        kDD = 0b00,
        kDI = 0b01,
        kID = 0b10,
        kII = 0b11
    };

    switch(static_cast<ClipCombo>(((int) a.fOp << 1) | (int) b.fOp)) {
        case ClipCombo::kII:
            // Intersect (A) + Intersect (B)
            if (!a.intersects(b)) {
                // Regions with non-zero coverage are disjoint, so intersection = empty
                return SimplifyResult::kEmpty;
            } else if (b.contains(a)) {
                // B's full coverage region contains entirety of A, so intersection = A
                return SimplifyResult::kAOnly;
            } else if (a.contains(b)) {
                // A's full coverage region contains entirety of B, so intersection = B
                return SimplifyResult::kBOnly;
            } else {
                // The shapes intersect in some non-trivial manner
                return SimplifyResult::kBoth;
            }
        case ClipCombo::kID:
            // Intersect (A) + Difference (B)
            if (!a.intersects(b)) {
                // A only intersects B's full coverage region, so intersection = A
                return SimplifyResult::kAOnly;
            } else if (b.contains(a)) {
                // B's zero coverage region completely contains A, so intersection = empty
                return SimplifyResult::kEmpty;
            } else {
                // Intersection cannot be simplified. Note that the combination of a intersect
                // and difference op in this order cannot produce kBOnly
                return SimplifyResult::kBoth;
            }
        case ClipCombo::kDI:
            // Difference (A) + Intersect (B) - the mirror of Intersect(A) + Difference(B),
            // but combining is commutative so this is equivalent barring naming.
            if (!b.intersects(a)) {
                // B only intersects A's full coverage region, so intersection = B
                return SimplifyResult::kBOnly;
            } else if (a.contains(b)) {
                // A's zero coverage region completely contains B, so intersection = empty
                return SimplifyResult::kEmpty;
            } else {
                // Cannot be simplified
                return SimplifyResult::kBoth;
            }
        case ClipCombo::kDD:
            // Difference (A) + Difference (B)
            if (a.contains(b)) {
                // A's zero coverage region contains B, so B doesn't remove any extra
                // coverage from their intersection.
                return SimplifyResult::kAOnly;
            } else if (b.contains(a)) {
                // Mirror of the above case, intersection = B instead
                return SimplifyResult::kBOnly;
            } else {
                // Intersection of the two differences cannot be simplified. Note that for
                // this op combination it is not possible to produce kEmpty.
                return SimplifyResult::kBoth;
            }
    }
}

///////////////////////////////////////////////////////////////////////////////
// ClipStack::Element

ClipStack::RawElement::RawElement(const Rect& deviceBounds,
                                  const Transform& localToDevice,
                                  const Shape& shape,
                                  SkClipOp op)
        : Element{shape, localToDevice, op}
        , fInvalidatedByIndex(-1) {
    // Discard shapes that don't have any area (including when a transform can't be inverted, since
    // it means the two dimensions are collapsed to 0 or 1 dimension in device space).
    if (fShape.isLine() || !localToDevice.valid()) {
        fShape.reset();
    }
    // Make sure the shape is not inverted. An inverted shape is equivalent to a non-inverted shape
    // with the clip op toggled.
    if (fShape.inverted()) {
        fOp = (fOp == SkClipOp::kIntersect) ? SkClipOp::kDifference : SkClipOp::kIntersect;
        fShape.setInverted(false);
    }

    fOuterBounds = fLocalToDevice.mapRect(fShape.bounds()).makeIntersect(deviceBounds);
    fInnerBounds = Rect::InfiniteInverted();

    // Apply rect-stays-rect transforms to rects and round rects to reduce the number of unique
    // local coordinate systems that are in play.
    if (!fOuterBounds.isEmptyNegativeOrNaN() &&
        fLocalToDevice.type() == Transform::Type::kRectStaysRect) {
        if (fShape.isRect()) {
            // The actual geometry can be updated to the device-intersected bounds and we know the
            // inner bounds are equal to the outer.
            fShape.setRect(fOuterBounds);
            fLocalToDevice = kIdentity;
            fInnerBounds = fOuterBounds;
        } else if (fShape.isRRect()) {
            // Can't transform in place and must still check transform result since some very
            // ill-formed scale+translate matrices can cause invalid rrect radii.
            SkRRect xformed;
            if (fShape.rrect().transform(fLocalToDevice, &xformed)) {
                fShape.setRRect(xformed);
                fLocalToDevice = kIdentity;
                // Refresh outer bounds to match the transformed round rect in case
                // SkRRect::transform produces slightly different results from Transform::mapRect.
                fOuterBounds = fShape.bounds().makeIntersect(deviceBounds);
                fInnerBounds = Rect{SkRRectPriv::InnerBounds(xformed)}.makeIntersect(fOuterBounds);
            }
        }
    }

    if (fOuterBounds.isEmptyNegativeOrNaN()) {
        // Either was already an empty shape or a non-empty shape is offscreen, so treat it as such.
        fShape.reset();
        fInnerBounds = Rect::InfiniteInverted();
    }

    // Post-conditions on inner and outer bounds
    SkASSERT(fShape.isEmpty() || deviceBounds.contains(fOuterBounds));
    this->validate();
}

ClipStack::RawElement::operator TransformedShape() const {
    return {fLocalToDevice, fShape, fOuterBounds, fInnerBounds, fOp};
}

void ClipStack::RawElement::validate() const {
    // If the shape type isn't empty, the outer bounds shouldn't be empty; if the inner bounds are
    // not empty, they must be contained in outer.
    SkASSERT((fShape.isEmpty() || !fOuterBounds.isEmptyNegativeOrNaN()) &&
             (fInnerBounds.isEmptyNegativeOrNaN() || fOuterBounds.contains(fInnerBounds)));
}

void ClipStack::RawElement::markInvalid(const SaveRecord& current) {
    SkASSERT(!this->isInvalid());
    fInvalidatedByIndex = current.firstActiveElementIndex();
}

void ClipStack::RawElement::restoreValid(const SaveRecord& current) {
    if (current.firstActiveElementIndex() < fInvalidatedByIndex) {
        fInvalidatedByIndex = -1;
    }
}

bool ClipStack::RawElement::combine(const RawElement& other, const SaveRecord& current) {
    // To reduce the number of possibilities, only consider intersect+intersect. Difference and
    // mixed op cases could be analyzed to simplify one of the shapes, but that is a rare
    // occurrence and the math is much more complicated.
    if (other.fOp != SkClipOp::kIntersect || fOp != SkClipOp::kIntersect) {
        return false;
    }

    // At the moment, only rect+rect or rrect+rrect are supported (although rect+rrect is
    // treated as a degenerate case of rrect+rrect).
    bool shapeUpdated = false;
    if (fShape.isRect() && other.fShape.isRect()) {
        if (fLocalToDevice == other.fLocalToDevice) {
            Rect intersection = fShape.rect().makeIntersect(other.fShape.rect());
            // Simplify() should have caught this case
            SkASSERT(!intersection.isEmptyNegativeOrNaN());
            fShape.setRect(intersection);
            shapeUpdated = true;
        }
    } else if ((fShape.isRect() || fShape.isRRect()) &&
               (other.fShape.isRect() || other.fShape.isRRect())) {
        if (fLocalToDevice == other.fLocalToDevice) {
            // Treat rrect+rect intersections as rrect+rrect
            SkRRect a = fShape.isRect() ? SkRRect::MakeRect(fShape.rect().asSkRect())
                                        : fShape.rrect();
            SkRRect b = other.fShape.isRect() ? SkRRect::MakeRect(other.fShape.rect().asSkRect())
                                              : other.fShape.rrect();

            SkRRect joined = SkRRectPriv::ConservativeIntersect(a, b);
            if (!joined.isEmpty()) {
                // Can reduce to a single element
                if (joined.isRect()) {
                    // And with a simplified type
                    fShape.setRect(joined.rect());
                } else {
                    fShape.setRRect(joined);
                }
                shapeUpdated = true;
            }
            // else the intersection isn't representable as a rrect, or doesn't actually intersect.
            // ConservativeIntersect doesn't disambiguate those two cases, and just testing bounding
            // boxes for non-intersection would have already been caught by Simplify(), so
            // just don't combine the two elements and let rasterization resolve the combination.
        }
    }

    if (shapeUpdated) {
        // This logic works under the assumption that both combined elements were intersect.
        SkASSERT(fOp == SkClipOp::kIntersect && other.fOp == SkClipOp::kIntersect);
        fOuterBounds.intersect(other.fOuterBounds);
        fInnerBounds.intersect(other.fInnerBounds);
        // Inner bounds can become empty, but outer bounds should not be able to.
        SkASSERT(!fOuterBounds.isEmptyNegativeOrNaN());
        return true;
    } else {
        return false;
    }
}

void ClipStack::RawElement::updateForElement(RawElement* added, const SaveRecord& current) {
    if (this->isInvalid()) {
        // Already doesn't do anything, so skip this element
        return;
    }

    // 'A' refers to this element, 'B' refers to 'added'.
    switch (Simplify(*this, *added)) {
        case SimplifyResult::kEmpty:
            // Mark both elements as invalid to signal that the clip is fully empty
            this->markInvalid(current);
            added->markInvalid(current);
            break;

        case SimplifyResult::kAOnly:
            // This element already clips more than 'added', so mark 'added' is invalid to skip it
            added->markInvalid(current);
            break;

        case SimplifyResult::kBOnly:
            // 'added' clips more than this element, so mark this as invalid
            this->markInvalid(current);
            break;

        case SimplifyResult::kBoth:
            // Else the bounds checks think we need to keep both, but depending on the combination
            // of the ops and shape kinds, we may be able to do better.
            if (added->combine(*this, current)) {
                // 'added' now fully represents the combination of the two elements
                this->markInvalid(current);
            }
            break;
    }
}

ClipStack::ClipState ClipStack::RawElement::clipType() const {
    // Map from the internal shape kind to the clip state enum
    switch (fShape.type()) {
        case Shape::Type::kEmpty:
            return ClipState::kEmpty;

        case Shape::Type::kRect:
            return fOp == SkClipOp::kIntersect &&
                   fLocalToDevice.type() == Transform::Type::kIdentity
                        ? ClipState::kDeviceRect : ClipState::kComplex;

        case Shape::Type::kRRect:
            return fOp == SkClipOp::kIntersect &&
                   fLocalToDevice.type() == Transform::Type::kIdentity
                        ? ClipState::kDeviceRRect : ClipState::kComplex;

        case Shape::Type::kLine:
            // These types should never become RawElements, but call them kComplex in release builds
            SkASSERT(false);
            [[fallthrough]];

        case Shape::Type::kPath:
            return ClipState::kComplex;
    }
    SkUNREACHABLE;
}

///////////////////////////////////////////////////////////////////////////////
// ClipStack::SaveRecord

ClipStack::SaveRecord::SaveRecord(const Rect& deviceBounds)
        : fInnerBounds(deviceBounds)
        , fOuterBounds(deviceBounds)
        , fShader(nullptr)
        , fStartingElementIndex(0)
        , fOldestValidIndex(0)
        , fDeferredSaveCount(0)
        , fStackOp(SkClipOp::kIntersect)
        , fState(ClipState::kWideOpen)  {}

ClipStack::SaveRecord::SaveRecord(const SaveRecord& prior,
                                  int startingElementIndex)
        : fInnerBounds(prior.fInnerBounds)
        , fOuterBounds(prior.fOuterBounds)
        , fShader(prior.fShader)
        , fStartingElementIndex(startingElementIndex)
        , fOldestValidIndex(prior.fOldestValidIndex)
        , fDeferredSaveCount(0)
        , fStackOp(prior.fStackOp)
        , fState(prior.fState) {
    // If the prior record added an element, this one will insert into the same index
    // (that's okay since we'll remove it when this record is popped off the stack).
    SkASSERT(startingElementIndex >= prior.fStartingElementIndex);
}

ClipStack::ClipState ClipStack::SaveRecord::state() const {
    if (fShader && fState != ClipState::kEmpty) {
        return ClipState::kComplex;
    } else {
        return fState;
    }
}

void ClipStack::SaveRecord::removeElements(RawElement::Stack* elements) {
    while (elements->count() > fStartingElementIndex) {
        elements->pop_back();
    }
}

void ClipStack::SaveRecord::restoreElements(RawElement::Stack* elements) {
    // Presumably this SaveRecord is the new top of the stack, and so it owns the elements
    // from its starting index to restoreCount - 1. Elements from the old save record have
    // been destroyed already, so their indices would have been >= restoreCount, and any
    // still-present element can be un-invalidated based on that.
    int i = elements->count() - 1;
    for (RawElement& e : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        e.restoreValid(*this);
        --i;
    }
}

void ClipStack::SaveRecord::addShader(sk_sp<SkShader> shader) {
    SkASSERT(shader);
    SkASSERT(this->canBeUpdated());
    if (!fShader) {
        fShader = std::move(shader);
    } else {
        // The total coverage is computed by multiplying the coverage from each element (shape or
        // shader), but since multiplication is associative, we can use kSrcIn blending to make
        // a new shader that represents 'shader' * 'fShader'
        fShader = SkShaders::Blend(SkBlendMode::kSrcIn, std::move(shader), fShader);
    }
}

bool ClipStack::SaveRecord::addElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // Validity check the element's state first
    toAdd.validate();

    // And we shouldn't be adding an element if we have a deferred save
    SkASSERT(this->canBeUpdated());

    if (fState == ClipState::kEmpty) {
        // The clip is already empty, and we only shrink, so there's no need to record this element.
        return false;
    } else if (toAdd.shape().isEmpty()) {
        // An empty difference op should have been detected earlier, since it's a no-op
        SkASSERT(toAdd.op() == SkClipOp::kIntersect);
        fState = ClipState::kEmpty;
        return true;
    }

    // Here we treat the SaveRecord as a "TransformedShape" with the identity transform, and a shape
    // equal to its outer bounds. This lets us get accurate intersection tests against the new
    // element, but we pass true to skip more detailed contains checks because the SaveRecord's
    // shape is potentially very different from its aggregate outer bounds.
    Shape outerSaveBounds{fOuterBounds};
    TransformedShape save{kIdentity, outerSaveBounds, fOuterBounds, fInnerBounds, fStackOp,
                          /*containsChecksBoundsOnly=*/true};

    // In this invocation, 'A' refers to the existing stack's bounds and 'B' refers to the new
    // element.
    switch (Simplify(save, toAdd)) {
        case SimplifyResult::kEmpty:
            // The combination results in an empty clip
            fState = ClipState::kEmpty;
            return true;

        case SimplifyResult::kAOnly:
            // The combination would not be any different than the existing clip
            return false;

        case SimplifyResult::kBOnly:
            // The combination would invalidate the entire existing stack and can be replaced with
            // just the new element.
            this->replaceWithElement(std::move(toAdd), elements);
            return true;

        case SimplifyResult::kBoth:
            // The new element combines in a complex manner, so update the stack's bounds based on
            // the combination of its and the new element's ops (handled below)
            break;
    }

    if (fState == ClipState::kWideOpen) {
        // When the stack was wide open and the clip effect was kBoth, the "complex" manner is
        // simply to keep the element and update the stack bounds to be the element's intersected
        // with the device.
        this->replaceWithElement(std::move(toAdd), elements);
        return true;
    }

    // Some form of actual clip element(s) to combine with.
    if (fStackOp == SkClipOp::kIntersect) {
        if (toAdd.op() == SkClipOp::kIntersect) {
            // Intersect (stack) + Intersect (toAdd)
            //  - Bounds updates is simply the paired intersections of outer and inner.
            fOuterBounds.intersect(toAdd.outerBounds());
            fInnerBounds.intersect(toAdd.innerBounds());
            // Outer should not have become empty, but is allowed to if there's no intersection.
            SkASSERT(!fOuterBounds.isEmptyNegativeOrNaN());
        } else {
            // Intersect (stack) + Difference (toAdd)
            //  - Shrink the stack's outer bounds if the difference op's inner bounds completely
            //    cuts off an edge.
            //  - Shrink the stack's inner bounds to completely exclude the op's outer bounds.
            fOuterBounds = subtract(fOuterBounds, toAdd.innerBounds(), /* exact */ true);
            fInnerBounds = subtract(fInnerBounds, toAdd.outerBounds(), /* exact */ false);
        }
    } else {
        if (toAdd.op() == SkClipOp::kIntersect) {
            // Difference (stack) + Intersect (toAdd)
            //  - Bounds updates are just the mirror of Intersect(stack) + Difference(toAdd)
            Rect oldOuter = fOuterBounds;
            fOuterBounds = subtract(toAdd.outerBounds(), fInnerBounds, /* exact */ true);
            fInnerBounds = subtract(toAdd.innerBounds(), oldOuter,     /* exact */ false);
        } else {
            // Difference (stack) + Difference (toAdd)
            //  - The updated outer bounds is the union of outer bounds and the inner becomes the
            //    largest of the two possible inner bounds
            fOuterBounds.join(toAdd.outerBounds());
            if (toAdd.innerBounds().area() > fInnerBounds.area()) {
                fInnerBounds = toAdd.innerBounds();
            }
        }
    }

    // If we get here, we're keeping the new element and the stack's bounds have been updated.
    // We ought to have caught the cases where the stack bounds resemble an empty or wide open
    // clip, so assert that's the case.
    SkASSERT(!fOuterBounds.isEmptyNegativeOrNaN() &&
             (fInnerBounds.isEmptyNegativeOrNaN() || fOuterBounds.contains(fInnerBounds)));

    return this->appendElement(std::move(toAdd), elements);
}

bool ClipStack::SaveRecord::appendElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // Update past elements to account for the new element
    int i = elements->count() - 1;

    // After the loop, elements between [max(youngestValid, startingIndex)+1, count-1] can be
    // removed from the stack (these are the active elements that have been invalidated by the
    // newest element; since it's the active part of the stack, no restore() can bring them back).
    int youngestValid = fStartingElementIndex - 1;
    // After the loop, elements between [0, oldestValid-1] are all invalid. The value of oldestValid
    // becomes the save record's new fLastValidIndex value.
    int oldestValid = elements->count();
    // After the loop, this is the earliest active element that was invalidated. It may be
    // older in the stack than earliestValid, so cannot be popped off, but can be used to store
    // the new element instead of allocating more.
    RawElement* oldestActiveInvalid = nullptr;
    int oldestActiveInvalidIndex = elements->count();

    for (RawElement& existing : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        // We don't need to pass the actual index that toAdd will be saved to; just the minimum
        // index of this save record, since that will result in the same restoration behavior later.
        existing.updateForElement(&toAdd, *this);

        if (toAdd.isInvalid()) {
            if (existing.isInvalid()) {
                // Both new and old invalid implies the entire clip becomes empty
                fState = ClipState::kEmpty;
                return true;
            } else {
                // The new element doesn't change the clip beyond what the old element already does
                return false;
            }
        } else if (existing.isInvalid()) {
            // The new element cancels out the old element. The new element may have been modified
            // to account for the old element's geometry.
            if (i >= fStartingElementIndex) {
                // Still active, so the invalidated index could be used to store the new element
                oldestActiveInvalid = &existing;
                oldestActiveInvalidIndex = i;
            }
        } else {
            // Keep both new and old elements
            oldestValid = i;
            if (i > youngestValid) {
                youngestValid = i;
            }
        }

        --i;
    }

    // Post-iteration validity check
    SkASSERT(oldestValid == elements->count() ||
             (oldestValid >= fOldestValidIndex && oldestValid < elements->count()));
    SkASSERT(youngestValid == fStartingElementIndex - 1 ||
             (youngestValid >= fStartingElementIndex && youngestValid < elements->count()));
    SkASSERT((oldestActiveInvalid && oldestActiveInvalidIndex >= fStartingElementIndex &&
              oldestActiveInvalidIndex < elements->count()) || !oldestActiveInvalid);

    // Update final state
    SkASSERT(oldestValid >= fOldestValidIndex);
    fOldestValidIndex = std::min(oldestValid, oldestActiveInvalidIndex);
    fState = oldestValid == elements->count() ? toAdd.clipType() : ClipState::kComplex;
    if (fStackOp == SkClipOp::kDifference && toAdd.op() == SkClipOp::kIntersect) {
        // The stack remains in difference mode only as long as all elements are difference
        fStackOp = SkClipOp::kIntersect;
    }

    int targetCount = youngestValid + 1;
    if (!oldestActiveInvalid || oldestActiveInvalidIndex >= targetCount) {
        // toAdd will be stored right after youngestValid
        targetCount++;
        oldestActiveInvalid = nullptr;
    }
    while (elements->count() > targetCount) {
        SkASSERT(oldestActiveInvalid != &elements->back()); // shouldn't delete what we'll reuse
        elements->pop_back();
    }
    if (oldestActiveInvalid) {
        *oldestActiveInvalid = std::move(toAdd);
    } else if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back() = std::move(toAdd);
    }

    return true;
}

void ClipStack::SaveRecord::replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // The aggregate state of the save record mirrors the element
    fInnerBounds = toAdd.innerBounds();
    fOuterBounds = toAdd.outerBounds();
    fStackOp = toAdd.op();
    fState = toAdd.clipType();

    // All prior active element can be removed from the stack: [startingIndex, count - 1]
    int targetCount = fStartingElementIndex + 1;
    while (elements->count() > targetCount) {
        elements->pop_back();
    }
    if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back() = std::move(toAdd);
    }

    SkASSERT(elements->count() == fStartingElementIndex + 1);

    // This invalidates all older elements that are owned by save records lower in the clip stack.
    fOldestValidIndex = fStartingElementIndex;
}

///////////////////////////////////////////////////////////////////////////////
// ClipStack

// NOTE: Based on draw calls in all GMs, SKPs, and SVGs as of 08/20, 98% use a clip stack with
// one Element and up to two SaveRecords, thus the inline size for RawElement::Stack and
// SaveRecord::Stack (this conveniently keeps the size of ClipStack manageable). The max
// encountered element stack depth was 5 and the max save depth was 6. Using an increment of 8 for
// these stacks means that clip management will incur a single allocation for the remaining 2%
// of the draws, with extra head room for more complex clips encountered in the wild.
static constexpr int kElementStackIncrement = 8;
static constexpr int kSaveStackIncrement = 8;

ClipStack::ClipStack(const SkIRect& deviceBounds)
        : fElements(kElementStackIncrement)
        , fSaves(kSaveStackIncrement)
        , fDeviceBounds(SkRect::Make(deviceBounds)) {
    // Start with a save record that is wide open
    fSaves.emplace_back(fDeviceBounds);
}

ClipStack::~ClipStack() = default;

void ClipStack::save() {
    SkASSERT(!fSaves.empty());
    fSaves.back().pushSave();
}

void ClipStack::restore() {
    SkASSERT(!fSaves.empty());
    SaveRecord& current = fSaves.back();
    if (current.popSave()) {
        // This was just a deferred save being undone, so the record doesn't need to be removed yet
        return;
    }

    // When we remove a save record, we delete all elements >= its starting index and any masks
    // that were rasterized for it.
    current.removeElements(&fElements);

    fSaves.pop_back();
    // Restore any remaining elements that were only invalidated by the now-removed save record.
    fSaves.back().restoreElements(&fElements);
}

Rect ClipStack::conservativeBounds() const {
    const SaveRecord& current = this->currentSaveRecord();
    if (current.state() == ClipState::kEmpty) {
        return Rect::InfiniteInverted();
    } else if (current.state() == ClipState::kWideOpen) {
        return fDeviceBounds;
    } else {
        if (current.op() == SkClipOp::kDifference) {
            // The outer/inner bounds represent what's cut out, so full bounds remains the device
            // bounds, minus any fully clipped content that spans the device edge.
            return subtract(fDeviceBounds, current.innerBounds(), /* exact */ true);
        } else {
            SkASSERT(fDeviceBounds.contains(current.outerBounds()));
            return current.outerBounds();
        }
    }
}

ClipStack::SaveRecord& ClipStack::writableSaveRecord(bool* wasDeferred) {
    SaveRecord& current = fSaves.back();
    if (current.canBeUpdated()) {
        // Current record is still open, so it can be modified directly
        *wasDeferred = false;
        return current;
    } else {
        // Must undefer the save to get a new record.
        SkAssertResult(current.popSave());
        *wasDeferred = true;
        return fSaves.emplace_back(current, fElements.count());
    }
}

void ClipStack::clipShader(sk_sp<SkShader> shader) {
    // Shaders can't bring additional coverage
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    bool wasDeferred;
    this->writableSaveRecord(&wasDeferred).addShader(std::move(shader));
    // Geometry elements are not invalidated by updating the clip shader
    // TODO: Integrating clipShader into graphite needs more thought, particularly around how to
    // handle the shader explosion and where to put the effects in the GraphicsPipelineDesc.
    // One idea is to use sample locations and draw the clipShader into the depth buffer.
    // Another is resolve the clip shader into an alpha mask image that is sampled by the draw.
}

void ClipStack::clipShape(const Transform& localToDevice, const Shape& shape, SkClipOp op) {
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    // This will apply the transform if it's shape-type preserving, and clip the element's bounds
    // to the device bounds (NOT the conservative clip bounds, since those are based on the net
    // effect of all elements while device bounds clipping happens implicitly. During addElement,
    // we may still be able to invalidate some older elements).
    // NOTE: Does not try to simplify the shape type by inspecting the SkPath.
    RawElement element{fDeviceBounds, localToDevice, shape, op};
    SkASSERT(!element.shape().inverted());

    // An empty op means do nothing (for difference), or close the save record, so we try and detect
    // that early before doing additional unnecessary save record allocation.
    if (element.shape().isEmpty()) {
        if (element.op() == SkClipOp::kDifference) {
            // If the shape is empty and we're subtracting, this has no effect on the clip
            return;
        }
        // else we will make the clip empty, but we need a new save record to record that change
        // in the clip state; fall through to below and updateForElement() will handle it.
    }

    bool wasDeferred;
    SaveRecord& save = this->writableSaveRecord(&wasDeferred);
    SkDEBUGCODE(int elementCount = fElements.count();)
    if (!save.addElement(std::move(element), &fElements)) {
        if (wasDeferred) {
            // We made a new save record, but ended up not adding an element to the stack.
            // So instead of keeping an empty save record around, pop it off and restore the counter
            SkASSERT(elementCount == fElements.count());
            fSaves.pop_back();
            fSaves.back().pushSave();
        }
    }
}

} // namespace skgpu
