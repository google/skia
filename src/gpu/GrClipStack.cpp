/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClipStack.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSWMaskHelper.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "src/gpu/effects/generated/GrDeviceSpaceEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"

namespace {

enum class ClipEffect {
    kEmpty,
    kAOnly,
    kBOnly,
    kBoth
};

// A and B can be Element, SaveRecord, or Draw. Supported combinations are, order not mattering,
// (Element, Element), (Element, SaveRecord), (Element, Draw), and (SaveRecord, Draw).
template<typename A, typename B>
static ClipEffect get_clip_effect(const A& a, const B& b) {
    // NOTE: SkIRect::Intersects() returns false when two rectangles touch at an edge (so the result
    // is empty). This behavior is desired for the following clip effect policies.
    if (a.op() == SkClipOp::kIntersect) {
        if (b.op() == SkClipOp::kIntersect) {
            // Intersect (A) + Intersect (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // Regions with non-zero coverage are disjoint, so intersection = empty
                return ClipEffect::kEmpty;
            } else if (a.contains(b)) {
                // A's full coverage region contains entirety of B, so intersection = B
                return ClipEffect::kBOnly;
            } else if (b.contains(a)) {
                // B's full coverage region contains entirety of A, so intersection = A
                return ClipEffect::kAOnly;
            } else {
                // The shapes intersect in some non-trivial manner
                return ClipEffect::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Intersect (A) + Difference (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // A only intersects B's full coverage region, so intersection = A
                return ClipEffect::kAOnly;
            } else if (b.contains(a)) {
                // B's zero coverage region completely contains A, so intersection = empty
                return ClipEffect::kEmpty;
            } else {
                // Intersection cannot be simplified. Note that the combination of a intersect
                // and difference op in this order cannot produce kBOnly
                return ClipEffect::kBoth;
            }
        }
    } else {
        SkASSERT(a.op() == SkClipOp::kDifference);
        if (b.op() == SkClipOp::kIntersect) {
            // Difference (A) + Intersect (B) - the mirror of Intersect(A) + Difference(B),
            // but combining is commutative so this is equivalent barring naming.
            if (!SkIRect::Intersects(b.outerBounds(), a.outerBounds())) {
                // B only intersects A's full coverage region, so intersection = B
                return ClipEffect::kBOnly;
            } else if (a.contains(b)) {
                // A's zero coverage region completely contains B, so intersection = empty
                return ClipEffect::kEmpty;
            } else {
                // Cannot be simplified
                return ClipEffect::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Difference (A) + Difference (B)
            if (a.contains(b)) {
                // A's zero coverage region contains B, so B doesn't remove any extra
                // coverage from their intersection.
                return ClipEffect::kAOnly;
            } else if (b.contains(a)) {
                // Mirror of the above case, intersection = B instead
                return ClipEffect::kBOnly;
            } else {
                // Intersection of the two differences cannot be simplified. Note that for
                // this op combination it is not possible to produce kEmpty.
                return ClipEffect::kBoth;
            }
        }
    }
}

// Returns the largest rect that is contained in A u B. If A and B do not intersect, then this will
// return the larger of A and B. If they do intersect, rectangles spanning the horizontal and
// vertical unions of the two rectangles are considered as well.
//
// If the returned rectangle contains both A and B, it means the union of A and B can be exactly
// represented as a single rectangle.
template<typename R, typename C>
static R largest_rect_in_union(const R& a, const R& b) {
    static constexpr C kZ = C(0);
    C aArea = a.isEmpty() ? kZ : a.width() * a.height();
    C bArea = b.isEmpty() ? kZ : b.width() * b.height();

    if (!R::Intersects(a, b)) {
        return aArea > bArea ? a : b;
    }

    SkASSERT(!a.isEmpty() && !b.isEmpty());
    // Two additional rectangles to consider:
    // 1. horizontal span: (min(a.l, b.l), max(a.t, b.t), max(a.r, b.r), min(a.b, b.b))
    // 2. vertical span: (max(a.l, b.l), min(a.t, b.t), min(a.r, b.r), max(a.b, b.b))
    auto lSpan = std::minmax(a.fLeft, b.fLeft);
    auto tSpan = std::minmax(a.fTop, b.fTop);
    auto rSpan = std::minmax(a.fRight, b.fRight);
    auto bSpan = std::minmax(a.fBottom, b.fBottom);

    // first stores the min, second stores the max
    R horiz = R::MakeLTRB(lSpan.first, tSpan.second, rSpan.second, bSpan.first);
    R vert = R::MakeLTRB(lSpan.second, tSpan.first, rSpan.first, bSpan.second);
    C horizArea = horiz.width() * horiz.height();
    C vertArea = vert.width() * vert.height();

    if (horizArea > vertArea && horizArea > aArea && horizArea > bArea) {
        // Horizontal span is the largest contiguous rect in A u B
        return horiz;
    } else if (vertArea > aArea && vertArea > bArea) {
        // Vertical span is the largest contiguous rect
        return vert;
    } else if (aArea > bArea) {
        // A remains the largest subrect in A u B
        return a;
    } else {
        // B is the largest
        return b;
    }
}
static SkRect largest_rect_in_union(const SkRect& a, const SkRect& b) {
    return largest_rect_in_union<SkRect, SkScalar>(a, b);
}
static SkIRect largest_rect_in_union(const SkIRect& a, const SkIRect& b) {
    return largest_rect_in_union<SkIRect, int>(a, b);
}

static SkIRect subtract(const SkIRect& a, const SkIRect& b, bool exact) {
    SkIRect diff;
    if (SkRectPriv::Subtract(a, b, &diff) || !exact) {
        // Either A-B is exactly the rectangle stored in diff, or we don't need an exact answer
        // and can settle for the subrect of A excluded from B (which is also 'diff')
        return diff;
    } else {
        // For our purposes, we want the original A when A-B cannot be exactly represented
        return a;
    }
}

static GrClipEdgeType get_clip_edge_type(SkClipOp op, bool aa) {
    if (op == SkClipOp::kIntersect) {
        return aa ? GrClipEdgeType::kFillAA : GrClipEdgeType::kFillBW;
    } else {
        return aa ? GrClipEdgeType::kInverseFillAA : GrClipEdgeType::kInverseFillBW;
    }
}

template<typename T, int N>
static void pop_to_count(GrTAllocator<T, N>* allocator, int newCount) {
    while(allocator->count() > newCount) {
        allocator->pop_back();
    }
}

template<typename T, int N>
static void set_item(GrTAllocator<T, N>* allocator, int index, T&& item) {
    if (index < allocator->count()) {
        // Overwrite the existing item
        allocator->item(index) = std::move(item);
    } else {
        // Append to the end
        SkASSERT(index == allocator->count());
        allocator->push_back(std::move(item));
    }
}

static uint32_t kInvalidGenID  = 0;
static uint32_t kEmptyGenID    = 1;
static uint32_t kWideOpenGenID = 2;

static uint32_t next_gen_id() {
    // 0-1 are reserved for invalid, empty & wide-open
    static const uint32_t kFirstUnreservedGenID = 3;
    static std::atomic<uint32_t> nextID{kFirstUnreservedGenID};

    uint32_t id;
    do {
        id = nextID++;
    } while (id < kFirstUnreservedGenID);
    return id;
}

static constexpr GrSurfaceOrigin kMaskOrigin = kTopLeft_GrSurfaceOrigin;

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::Element

GrClipStack::Element::Element(const SkMatrix& ctm, const GrShape& shape, bool aa, SkClipOp op)
        : fShape(shape)
        , fCTM(ctm)
        , fInvalidatedByIndex(-1)
        , fOp(op)
        , fAA(aa) {}

GrClipStack::Element::Element(const Element& e) {
    *this = e;
}

GrClipStack::Element& GrClipStack::Element::operator=(const Element& e) {
    fShape = e.fShape;
    fCTM = e.fCTM;
    fInnerBounds = e.fInnerBounds;
    fOuterBounds = e.fOuterBounds;
    fInvalidatedByIndex = e.fInvalidatedByIndex;
    fAA = e.fAA;
    fOp = e.fOp;
    return *this;
}

bool GrClipStack::Element::contains(const Element& e) const {
    if (e.fCTM.isIdentity()) {
        // If e is already in device space, there's no need to map its local points to device
        // space, and processing its outer bounds is sufficient.
        return this->contains(e.fOuterBounds);
    }
    // If the anti-alias modes differ between the elements, switch to testing pixel boundaries
    // instead of exact geometry. When the AA state matches, (BW or not), an exact geometry contains
    // will be consistent with what is rasterized, even if the rasterized geometry is outset or
    // snapped
    bool mustDoPixelSnapping = e.fAA != fAA;

    // Quick test before doing any coord space transformation
    if (fInnerBounds.contains(e.fOuterBounds)) {
        return true;
    } else if (!fShape.convex()) {
        // Since the shape isn't convex, we can't just consider the points independently. If we're
        // in device-space, however, use the shape's contains() function to process the rect
        SkRect query = mustDoPixelSnapping ? SkRect::Make(e.fOuterBounds) : e.fShape.bounds();
        return fCTM.isIdentity() && fShape.contains(query);
    }

    // First map the 4 corners of e's local bounds into device space.
    SkPoint corners[4];
    e.fShape.bounds().toQuad(corners);
    e.fCTM.mapPoints(corners, 4);

    SkMatrix invCTM;
    if (!fCTM.invert(&invCTM)) {
        return false;
    }

    if (!mustDoPixelSnapping) {
        // Can map directly into this space and query per-point
        // FIXME if we knew whether or not a draw was AA, and had the draw's CTM, we could do the
        // same exact test here, which would make it easier to unclip a clipRect(r), drawRect(r)
        // sequence. But for now, I think it's okay to have those special-cased in GrRTC.
        invCTM.mapPoints(corners, 4);
        for (int i = 0; i < 4; ++i) {
            if (!fShape.contains(corners[i])) {
                return false;
            }
        }
        return true;
    } else {
        // We round these points to different extreme pixel edges in device space before mapping
        // into this' local space. Each floating point point produces 1 to 2 pixel-snapped points.
        // If the corner's coordinate is an extreme edge, it gets rounded in or out depending on the
        // side. If it is not an extreme edge, we test both rounded in and out.
        SkRect deviceBounds = e.fCTM.mapRect(e.fShape.bounds());
        SkSTArray<8, SkPoint> deviceCorners;
        for (int i = 0; i < 4; ++i) {
            bool floorY = corners[i].fY == deviceBounds.fTop ||
                          corners[i].fY != deviceBounds.fBottom;
            bool ceilY = corners[i].fY == deviceBounds.fBottom ||
                         corners[i].fY != deviceBounds.fTop;
            // Floor X
            if (corners[i].fX == deviceBounds.fLeft || corners[i].fX != deviceBounds.fRight) {
                SkScalar x = SkScalarFloorToScalar(corners[i].fX);
                if (floorY) {
                    deviceCorners.push_back(SkPoint{x, SkScalarFloorToScalar(corners[i].fY)});
                }
                if(ceilY) {
                    deviceCorners.push_back(SkPoint{x, SkScalarCeilToScalar(corners[i].fY)});
                }
            }
            // Ceil X
            if (corners[i].fX == deviceBounds.fRight || corners[i].fX != deviceBounds.fLeft) {
                SkScalar x = SkScalarCeilToScalar(corners[i].fX);
                if (floorY) {
                    deviceCorners.push_back(SkPoint{x, SkScalarFloorToScalar(corners[i].fY)});
                }
                if(ceilY) {
                    deviceCorners.push_back(SkPoint{x, SkScalarCeilToScalar(corners[i].fY)});
                }
            }
        }

        SkASSERT(deviceCorners.size() >= 4 && deviceCorners.size() <= 8);
        invCTM.mapPoints(deviceCorners.begin(), deviceCorners.size());
        for (size_t i = 0; deviceCorners.size(); ++i) {
            if (!fShape.contains(deviceCorners[i])) {
                return false;
            }
        }
        return true;
    }
}

bool GrClipStack::Element::contains(const SkIRect& bounds) const {
    if (fInnerBounds.contains(bounds)) {
        return true;
    }

    SkRect boundsF = SkRect::Make(bounds);
    if (fCTM.isIdentity()) {
        // 'bounds' is already in the clips coordinate space
        return fShape.contains(boundsF);
    } else if (fShape.convex()) {
        // Since the shape is convex, we can map each corner of the bounds into the element's
        // coordinate space and test the point for containment. If all pass, the bounds are
        // are inside the shape still.
        SkPoint corners[4];
        boundsF.toQuad(corners);

        // FIXME is it worth caching the inverse matrix on the element?
        SkMatrix invCTM;
        if (!fCTM.invert(&invCTM)) {
            return false;
        }
        invCTM.mapPoints(corners, 4);

        for (int i = 0; i < 4; ++i) {
            if (!fShape.contains(corners[i])) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

void GrClipStack::Element::simplify(const SkIRect& deviceBounds) {
    // First step is to simplify the base shape
    fShape.simplify();

    // Lines and points should have been turned into empty since we assume everything is filled
    SkASSERT(!fShape.isPoint() && !fShape.isLine());
    // Sanity check, we have no public API to create an arc at the moment
    SkASSERT(!fShape.isArc());

    // Make sure the shape is not inverted. An inverted shape is equivalent to a non-inverted shape
    // with the clip op toggled.
    if (fShape.inverted()) {
        fOp = fOp == SkClipOp::kIntersect ? SkClipOp::kDifference : SkClipOp::kIntersect;
        fShape.setInverted(false);
    }

    // Then try to apply the ctm if it results in a nice shape still; this lets us combine elements
    // across CTMs into a single simpler element when possible, and possibly use the device bounds
    // to further simplify the geometry.
    bool deviceBoundsApplied = false;
    if (!fCTM.isIdentity() && fCTM.isScaleTranslate()) {
        if (fShape.isRect()) {
            fShape.rect() = fCTM.mapRect(fShape.rect());
            fCTM = SkMatrix::I();

            if (!fShape.rect().intersect(SkRect::Make(deviceBounds))) {
                fShape.reset();
            }
            deviceBoundsApplied = true;
        } else if (fShape.isRRect()) {
            SkRRect src = fShape.rrect();
            SkAssertResult(src.transform(fCTM, &fShape.rrect()));
            fCTM = SkMatrix::I();

            src = SkRRectPriv::ConservativeIntersect(
                    fShape.rrect(), SkRRect::MakeRect(SkRect::Make(deviceBounds)));
            // If src is non-empty the resulting intersection was a valid rrect. When src is empty
            // the shapes could intersect, or there is no intersection. This will be disambiguated
            // later on when the outer bounds are compared to the device bounds.
            if (!src.isEmpty()) {
                fShape.rrect() = src;
                deviceBoundsApplied = true;
            }
        }
        // otherwise, don't transform the path since that will modify its gen ID and we'll lose
        // out on any caching across frames
    }

    this->updateBounds();

    // Apply the device bounds to the pixel bounds if it wasn't already accounted for directly
    if (!deviceBoundsApplied && !fShape.isEmpty()) {
        if (!fOuterBounds.intersect(deviceBounds)) {
            // We've been clipped off screen
            fShape.reset();
            fOuterBounds = SkIRect::MakeEmpty();
            fInnerBounds = SkIRect::MakeEmpty();
        } else if (!fInnerBounds.isEmpty() && !fInnerBounds.intersect(deviceBounds)) {
            // Depending on positioning, it's entirely possible for the mixed coverage area of a
            // clip to be on screen, but the known full 1/0 coverage to be offscreen, so it's okay
            // for the inner bounds to not intersect the device bounds.
            fInnerBounds = SkIRect::MakeEmpty();
        }
    }
}

void GrClipStack::Element::updateBounds() {
    // Compute initial inner and outer bounds in local space, no rounding yet
    SkRect inner, outer;
    if (fShape.isPath()) {
        inner = SkRect::MakeEmpty();
        outer = fShape.path().getBounds();
    } else if (fShape.isRect()) {
        inner = fShape.bounds();
        outer = inner;
    } else if (fShape.isRRect()) {
        inner = SkRRectPriv::InnerBounds(fShape.rrect());
        outer = fShape.rrect().getBounds();
    } else {
        SkASSERT(fShape.isEmpty());
    }

    // Map to device space
    if (!fCTM.isIdentity()) {
        // FIXME could theoretically compute an inner mapped rect for the updated inner bounds
        inner = SkRect::MakeEmpty();
        outer = fCTM.mapRect(outer);
    }

    // When the clip is non-AA, the pixel bounds are simply rounded. If the clip is AA, the outer
    // bounds are rounded out and the inner bounds are rounded in. We apply a little fudge tolerance
    // to the bounds before rounding.
    if (fAA) {
        outer.inset(GrClip::kBoundsTolerance, GrClip::kBoundsTolerance);
        if (!inner.isEmpty()) {
            inner.outset(GrClip::kBoundsTolerance, GrClip::kBoundsTolerance);
        }

        outer.roundOut(&fOuterBounds);
        inner.roundIn(&fInnerBounds);
    } else {
        // FIXME if the float boundaries are close to half pixels, we should maybe adjust the edge
        // to round out/in as if it were AA to account for the unpredictability of GPU non-AA rasterization
        outer.round(&fOuterBounds);
        inner.round(&fInnerBounds);
    }

    if (fOuterBounds.isEmpty()) {
        // FIXME is it right to just make the whole element empty? We can end up in this case when
        // a non-aa draw that is thinner than a pixel is used. If it were to land on a pixel center
        // we'd want to show it, but plain round() doesn't do that. (but if we switched to conservative
        // round-in + round-out like for AA, we'd get it being a pixel wide, so then it would not
        // be empty). More evidence to do that.
        fShape.reset();
    }
}

bool GrClipStack::Element::combine(const Element& other) {
    // To reduce the number of possibilities, only consider intersect+intersect and
    // difference+difference combinations. Mixed op cases could be analyzed to simplify one of the
    // shapes, but they cannot collapse into a single element so are already of reduced value.
    if (fOp != other.fOp) {
        return false;
    }
    // FIXME depending on if these are rects and we track edge flags, or if the edges are on
    // pixel boundaries w/o needing rounded corners anymore, we could mix aa
    if (fAA != other.fAA) {
        return false;
    }

    // Shapes must be in the same coordinate space to be combined together
    if (fCTM != other.fCTM) {
        return false;
    }

    // Can combine rects with rects, and rrects with rrects. rect+rrect is promoted to rrect+rrect
    bool shapeUpdated = false;
    if (fShape.isRect() && other.fShape.isRect()) {
        if (fOp == SkClipOp::kIntersect) {
            SkAssertResult(fShape.rect().intersect(other.fShape.rect()));
            shapeUpdated = true;
        } else {
            // (1 \ A) n (1 \ B) == 1 \ (A u B), so if the two rects' union can be stored as
            // a single rect, we can replace this with a single difference element
            SkRect joined = largest_rect_in_union(fShape.rect(), other.fShape.rect());
            if (joined.contains(fShape.rect()) && joined.contains(other.fShape.rect())) {
                fShape.rect() = joined;
                shapeUpdated = true;
            }
        }
    } else if ((fShape.isRect() || fShape.isRRect()) &&
               (other.fShape.isRect() || other.fShape.isRRect()) &&
               fOp == SkClipOp::kIntersect) {
        // Only rrect intersections are implemented, map rects back to rrects
        SkRRect a = fShape.isRect() ? SkRRect::MakeRect(fShape.rect()) : fShape.rrect();
        SkRRect b = other.fShape.isRect() ? SkRRect::MakeRect(other.fShape.rect())
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
    }

    if (shapeUpdated) {
        this->updateBounds();
        return true;
    } else {
        return false;
    }
}

void GrClipStack::Element::updateForElement(Element* added, int addedIndex) {
    if (fInvalidatedByIndex >= 0) {
        // Already doesn't do anything, so skip this element
        return;
    }

    // A refers to this element, B refers to 'added'.
    ClipEffect effect = get_clip_effect(*this, *added);

    if (effect == ClipEffect::kEmpty) {
        // Mark both elements as invalid to signal that the clip is fully empty
        fInvalidatedByIndex = addedIndex;
        added->fInvalidatedByIndex = addedIndex;
        return;
    } else if (effect == ClipEffect::kAOnly) {
        // This element already clips more than 'added', so mark 'added' is invalid so it's skipped
        added->fInvalidatedByIndex = addedIndex;
        return;
    } else if (effect == ClipEffect::kBOnly) {
        // 'added' clips more than this element, so mark this as invalid
        fInvalidatedByIndex = addedIndex;
        return;
    }

    // Else the bounds checks think we need to keep both, but depending on the combination of the
    // ops and shape kinds, we may be able to do better. Currently, we can reason about the
    // combination of rects and round rects.
    if (added->combine(*this)) {
        // 'added' now fully represents the combination of the two elements so invalidate this
        fInvalidatedByIndex = addedIndex;
    }
}

GrClipStack::DrawEffect GrClipStack::Element::affectsDraw(const Draw& draw) const {
    if (fInvalidatedByIndex >= 0) {
        // The element has been disabled, so it can never affect a draw (presumably there is another
        // element that will affect the draw in an equivalent or greater manner).
        return DrawEffect::kUnclipped;
    }

    ClipEffect effect = get_clip_effect(*this, draw);
    if (effect == ClipEffect::kEmpty) {
        return DrawEffect::kNoDraw;
    } else if (effect == ClipEffect::kBOnly) {
        return DrawEffect::kUnclipped;
    }
    SkASSERT(effect != ClipEffect::kAOnly);

    return DrawEffect::kClipped;
}

GrClipStack::ClipState GrClipStack::Element::clipType() const {
    // Map from the internal shape kind to the clip state enum
    if (fShape.isEmpty()) {
        return ClipState::kEmpty;
    } else if (fShape.isRect()) {
        return ClipState::kRect;
    } else if (fShape.isRRect()) {
        return ClipState::kRRect;
    } else {
        return ClipState::kComplex;
    }
}

std::unique_ptr<GrFragmentProcessor> GrClipStack::Element::asCoverageFP(const SkIRect& drawBounds,
                                                                        GrRecordingContext* context,
                                                                        GrRenderTargetContext* rtc,
                                                                        bool useHWAA,
                                                                        bool hasUserStencilSettings) const {
    GrClipEdgeType edgeType = get_clip_edge_type(fOp, fAA);
    std::unique_ptr<GrFragmentProcessor> op;
    if (fShape.isRect()) {
        if (fCTM.isIdentity()) {
            // This effect only works in device space
            op = GrAARectEffect::Make(edgeType, fShape.rect());
        } else {
            // The rectangle mapped by the CTM produces a convex hull
            // FIXME could compute this more efficiently by mapping rect edge equations
            SkPath p;
            fShape.asPath(&p);
            p.transform(fCTM);
            op = GrConvexPolyEffect::Make(edgeType, p);
        }
    } else if (fShape.isRRect()) {
        if (fCTM.isIdentity()) {
            // This effect only works in device space
            op = GrRRectEffect::Make(edgeType, fShape.rrect(), *context->priv().caps()->shaderCaps());
        }
    } else {
        SkASSERT(fShape.isPath()); // other types result in an empty clip that should not need an FP
        if (fCTM.isIdentity()) {
            // FIXME if this is a bottleneck, could be worth specializing convex hulls and save
            // the computed edge equations in GrShape.
            op = GrConvexPolyEffect::Make(edgeType, fShape.path());
        }
    }

    if (!op) {
        // SkPath p;
        // fShape.asPath(&p);
        // p.transform(fCTM); // FIXME shouldn't pre-transform the path, that should be part of atlas query

        // FIXME use CCPR or other dynamic atlas of path masks

    }
    return op;
}

// FIXME do we need the initial state, probs
GrSurfaceProxyView GrClipStack::Element::RenderSWMask(const SkIRect& bounds,
                                                      const Element** elements,
                                                      int count,
                                                      GrRecordingContext* context) {
    SkASSERT(count > 0);

    SkTaskGroup* taskGroup = nullptr;
    if (auto direct = context->priv().asDirectContext()) {
        taskGroup = direct->priv().getTaskGroup();
    }

    if (taskGroup) {
        const GrCaps* caps = context->priv().caps();
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        // Create our texture proxy
        GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                               GrRenderable::kNo);

        GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, GrColorType::kAlpha_8);

        // MDB TODO: We're going to fill this proxy with an ASAP upload (which is out of order wrt
        // to ops), so it can't have any pending IO.
        auto proxy = proxyProvider->createProxy(format, bounds.size(), GrRenderable::kNo, 1,
                                                GrMipMapped::kNo, SkBackingFit::kApprox,
                                                SkBudgeted::kYes, GrProtected::kNo);

        // Since this will be rendered on another thread, make a copy of the elements in case
        // the clip stack is modified on the main thread
        auto uploader = std::make_unique<GrTDeferredProxyUploader<SkTArray<Element>>>(count);
        for (int i = 0; i < count; ++i) {
            uploader->data().push_back(*(elements[i]));
        }

        GrTDeferredProxyUploader<SkTArray<Element>>* uploaderRaw = uploader.get();
        auto drawAndUploadMask = [uploaderRaw, bounds] {
            TRACE_EVENT0("skia.gpu", "Threaded SW Clip Mask Render");
            GrSWMaskHelper helper(uploaderRaw->getPixels());
            if (helper.init(bounds)) {
                // TODO: Kind of ugly to then go back to Element** just to reuse RenderToSWHelper.
                SkTArray<const Element*> addrs(uploaderRaw->data().count());
                for (int i = 0; i < uploaderRaw->data().count(); ++i) {
                    addrs.push_back(&(uploaderRaw->data()[i]));
                }
                RenderToSWHelper(&helper, addrs.begin(), addrs.count());
            } else {
                SkDEBUGFAIL("Unable to allocate SW clip mask.");
            }
            uploaderRaw->signalAndFreeData();
        };

        taskGroup->add(std::move(drawAndUploadMask));
        proxy->texPriv().setDeferredUploader(std::move(uploader));

        return {std::move(proxy), kMaskOrigin, swizzle};
    } else {
        GrSWMaskHelper helper;
        if (!helper.init(bounds)) {
            return {};
        }
        RenderToSWHelper(&helper, elements, count);
        return helper.toTextureView(context, SkBackingFit::kApprox);
    }
}

void GrClipStack::Element::RenderToSWHelper(GrSWMaskHelper* helper,
                                            const Element** elements,
                                            int count) {
    // If the first element to draw is an intersect, we clear to 0 and will draw it directly with
    // coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
    // If the first element to draw is a difference, we clear to 1, and in all cases we draw the
    // difference element directly with coverage 0.
    helper->clear(elements[0]->fOp == SkClipOp::kIntersect ? 0x00 : 0xFF);

    for (int i = 0; i < count; ++i) {
        const Element* e = elements[i];
        uint8_t alpha;
        bool invert;
        if (e->fOp == SkClipOp::kIntersect) {
            // Intersect modifies pixels outside of its geometry. If this isn't the first op, we
            // draw the inverse-filled shape with 0 coverage to erase everything outside the element
            // But if we are the first element, we can draw directly with coverage 1 since we
            // cleared to 0.
            if (i == 0) {
                alpha = 0xFF;
                invert = false;
            } else {
                alpha = 0x00;
                invert = true;
            }
        } else {
            // For difference ops, can always just subtract the shape directly by drawing 0 coverage
            SkASSERT(e->fOp == SkClipOp::kDifference);
            alpha = 0x00;
            invert = false;
        }

        // Draw the shape; based on how we've initialized the buffer and chosen alpha+invert,
        // every element is drawn with the kReplace_Op
        GrAA aa = e->fAA ? GrAA::kYes : GrAA::kNo;
        if (invert) {
            // Must need to invert the path
            SkASSERT(!e->fShape.inverted());
            SkPath p;
            e->fShape.asPath(&p);
            p.toggleInverseFillType();
            // FIXME this helper should have an option that just takes a GrShape directly
            helper->drawShape(GrStyledShape(p, GrStyle::SimpleFill()), e->fCTM,
                              SkRegion::kReplace_Op, aa, alpha);
        } else {
            // Try to draw primitive shapes directly
            if (e->fShape.isRect()) {
                helper->drawRect(e->fShape.rect(), e->fCTM, SkRegion::kReplace_Op, aa, alpha);
            } else {
                // Must go through path rendering still
                // FIXME Should expose a drawRRect variant since SkDraw already has it
                // FIXME and if it takes GrShape directly, we definitely don't need to extract a path
                SkPath p;
                e->fShape.asPath(&p);
                helper->drawShape(GrStyledShape(p, GrStyle::SimpleFill()), e->fCTM,
                                  SkRegion::kReplace_Op, aa, alpha);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::Mask

GrClipStack::Mask::Mask(uint32_t genID, const SkIRect& drawBounds)
        : fBounds(drawBounds)
        , fGenID(genID) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    // The gen ID should not be invalid, empty, or wide open, since those do not require masks
    SkASSERT(genID != kInvalidGenID && genID != kEmptyGenID && genID != kWideOpenGenID);

    GrUniqueKey::Builder builder(&fKey, kDomain, 3, "clip_mask");
    builder[0] = genID;
    // SkToS16 because image filters outset layers to a size indicated by the filter, which can
    // sometimes result in negative coordinates from device space.
    builder[1] = SkToS16(drawBounds.fLeft) | (SkToS16(drawBounds.fRight) << 16);
    builder[2] = SkToS16(drawBounds.fTop) | (SkToS16(drawBounds.fBottom) << 16);
    SkASSERT(fKey.isValid());
}

void GrClipStack::Mask::invalidate(GrProxyProvider* proxyProvider) {
    SkASSERT(proxyProvider);
    SkASSERT(fKey.isValid()); // Should only be invalidated once
    proxyProvider->processInvalidUniqueKey(
            fKey, nullptr, GrProxyProvider::InvalidateGPUResource::kYes);
    fKey.reset();
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::SaveRecord

GrClipStack::SaveRecord::SaveRecord(const SkIRect& deviceBounds)
        : fInnerBounds(deviceBounds)
        , fOuterBounds(deviceBounds)
        , fStartingMaskIndex(0)
        , fStartingElementIndex(0)
        , fLastValidIndex(0)
        , fDeferredSaveCount(0)
        , fStackOp(SkClipOp::kIntersect)
        , fState(ClipState::kWideOpen)
        , fGenID(kInvalidGenID) {}

GrClipStack::SaveRecord::SaveRecord(const SaveRecord& prior,
                                    int startingMaskIndex,
                                    int startingElementIndex)
        : fInnerBounds(prior.fInnerBounds)
        , fOuterBounds(prior.fOuterBounds)
        , fStartingMaskIndex(startingMaskIndex)
        , fStartingElementIndex(startingElementIndex)
        , fLastValidIndex(prior.fLastValidIndex)
        , fDeferredSaveCount(0)
        , fStackOp(prior.fStackOp)
        , fState(prior.fState)
        , fGenID(kInvalidGenID) {
    // If the prior record never needed a mask, this one will insert into the same index
    // (that's okay since we'll remove it when this record is popped off the stack).
    SkASSERT(startingMaskIndex >= prior.fStartingMaskIndex);
    // The same goes for elements (the prior could have been wide open).
    SkASSERT(startingElementIndex >= prior.fStartingElementIndex);
}

uint32_t GrClipStack::SaveRecord::genID() const {
    if (fState == ClipState::kEmpty) {
        return kEmptyGenID;
    } else if (fState == ClipState::kWideOpen) {
        return kWideOpenGenID;
    } else {
        // The gen ID shouldn't be empty or wide open, since they are reserved for the above
        // if-cases. It may be kInvalid if the record hasn't had any elements added to it yet.
        SkASSERT(fGenID != kEmptyGenID && fGenID != kWideOpenGenID);
        return fGenID;
    }
}

void GrClipStack::SaveRecord::removeElements(Element::Allocator* elements) {
    // Should only be called when we're about to be destroyed
    SkASSERT(fDeferredSaveCount == -1);
    pop_to_count(elements, fStartingElementIndex);
}

void GrClipStack::SaveRecord::restoreElements(Element::Allocator* elements) {
    // Presumably this SaveRecord is the new top of the stack, and so it owns the elements
    // from its starting index to restoreCount - 1. Elements from the old save record have
    // been destroyed already, so their indices would have been >= restoreCount, and any
    // still-present element can be un-invalidated based on that.
    int restoreCount = elements->count();
    int i = restoreCount - 1;
    for (Element& e : elements->ritems()) {
        if (i < fLastValidIndex) {
            break;
        }
        e.restoreValid(restoreCount);
        --i;
    }
}

void GrClipStack::SaveRecord::invalidateMasks(GrProxyProvider* proxyProvider,
                                              Mask::Allocator* masks) {
    // Must explicitly invalidate the key before removing the mask object from the stack
    while(masks->count() > fStartingMaskIndex) {
        SkASSERT(masks->back().genID() == fGenID);
        masks->back().invalidate(proxyProvider);
        masks->pop_back();
    }
    SkASSERT(masks->empty() || masks->back().genID() != fGenID);
}

bool GrClipStack::SaveRecord::updateForElement(Element&& toAdd, Element::Allocator* elements) {
    // Sanity check the element's state first; if the shape class isn't empty, the outer bounds
    // shouldn't be empty; if the inner bounds are not empty, they must be contained in outer.
    SkASSERT((toAdd.shape().isEmpty() || !toAdd.outerBounds().isEmpty()) &&
             (toAdd.innerBounds().isEmpty() || toAdd.outerBounds().contains(toAdd.innerBounds())));
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

    // In this invocation, A refers to the existing stack's bounds and B refers to the new element.
    ClipEffect effect = get_clip_effect(*this, toAdd);

    if (effect == ClipEffect::kEmpty) {
        // The combination results in an empty clip
        fState = ClipState::kEmpty;
        return true;
    } else if (effect == ClipEffect::kAOnly) {
        // The combination would not be any different than the existing clip
        return false;
    } else if (effect == ClipEffect::kBOnly) {
        // The combination would invalidate the entire existing stack and can be replaced with
        // just the new element.
        this->replaceWithElement(std::move(toAdd), elements);
        return true;
    }

    // The new element combines in a complex manner, so update the stack's bounds based on the
    // combination of its and the new element's ops.
    SkASSERT(effect == ClipEffect::kBoth);

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
            SkAssertResult(fOuterBounds.intersect(toAdd.outerBounds()));
            if (!fInnerBounds.intersect(toAdd.innerBounds())) {
                // NOTE: this does the right thing if either rect is empty, since we set the
                // inner bounds to empty here
                fInnerBounds = SkIRect::MakeEmpty();
            }
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
            SkIRect oldOuter = fOuterBounds;
            fOuterBounds = subtract(toAdd.outerBounds(), fInnerBounds, /* exact */ true);
            fInnerBounds = subtract(toAdd.innerBounds(), oldOuter,     /* exact */ false);
        } else {
            // Difference (stack) + Difference (toAdd)
            //  - The updated bounds is the union of outer bounds and largest contiguous piece
            //    of the combined inner bounds.
            fOuterBounds.join(toAdd.outerBounds());
            fInnerBounds = largest_rect_in_union(fInnerBounds, toAdd.innerBounds());
        }
    }

    // If we get here, we're keeping the new element and the stack's bounds have been updated.
    // We ought to have caught the cases where the stack bounds resemble an empty or wide open
    // clip, so assert that's the case.
    SkASSERT(!fOuterBounds.isEmpty() &&
             (fInnerBounds.isEmpty() || fOuterBounds.contains(fInnerBounds)));

    return this->appendElement(std::move(toAdd), elements);
}

bool GrClipStack::SaveRecord::appendElement(Element&& toAdd, Element::Allocator* elements) {
    // Update past elements to account for the new element
    int i = elements->count() - 1;

    // After the loop, elements between [max(earliestValid, startingIndex)+1, count-1] can be
    // removed from the stack (these are the active elements that have been invalidated by the
    // newest element; since its the active part of the stack, no restore() can bring them back).
    int earliestValid = fStartingElementIndex - 1;
    // After the loop, elements between [0, latestValid-1] are all invalid. The value of latestValid
    // becomes the save record's new fLastValidIndex value.
    int latestValid = i + 1;
    // After the loop, this is the earliest active element index that was invalidated. It may be
    // later in the stack than earliestValid, so cannot be popped off, but can be used to store
    // the new element instead of pushing more storage on to the allocator.
    int latestActiveInvalid = i + 1;

    for (Element& existing : elements->ritems()) {
        if (i < fLastValidIndex) {
            break;
        }
        // We don't need to pass the actual index that toAdd will be saved to; just the minimum
        // index of this save record, since that will result in the same restoration behavior later.
        existing.updateForElement(&toAdd, fStartingElementIndex);

        if (toAdd.isInvalid()) {
            if (existing.isInvalid()) {
                // When both the existing and the new element are invalid, it signals that their
                // combination produces an empty clip
                fState = ClipState::kEmpty;
                return true;
            } else {
                // The existing op supercedes the new element, so there's no need to continue
                // evaluating against other elements.
                return false;
            }
        } else if (existing.isInvalid()) {
            // The new element cancels out the old element. The new element may have been modified
            // to account for the old element's geometry.
            if (i >= fStartingElementIndex) {
                // Still active, so the invalidated index could be used to store the new element
                latestActiveInvalid = i;
            }
        } else {
            // The old element remains valid, and the new element has a novel contribution to the
            // clip, so continue on
            latestValid = i;
            if (i > earliestValid) {
                earliestValid = i;
            }
        }

        --i;
    }

    // Sanity check the indices just computed during the loop
    SkASSERT(latestValid == elements->count() ||
             (latestValid >= fLastValidIndex && latestValid < elements->count()));
    SkASSERT(earliestValid == fStartingElementIndex - 1 ||
             (earliestValid >= fStartingElementIndex && earliestValid < elements->count()));
    SkASSERT(latestActiveInvalid >= fStartingElementIndex &&
             latestActiveInvalid <= elements->count());

    // Update final state
    SkASSERT(latestValid >= fLastValidIndex);
    fLastValidIndex = std::min(latestValid, latestActiveInvalid);
    // The specific clip shape types are only appropriate when there's a single element. If
    // the new element was combined with earlier elements, they would have become invalid already.
    // So if there are no current invalid elements, the clip state matches the new element.
    // Otherwise, there's a combination of elements so it must be complex.
    fState = latestValid == elements->count() ? toAdd.clipType() : ClipState::kComplex;
    // fMaskGenerated = false;
    // - this would be where we'd reset the proxy clip key listeners I think
    if (fStackOp == SkClipOp::kDifference && toAdd.op() == SkClipOp::kIntersect) {
        // The stack remains in difference mode only as long as all elements are difference
        fStackOp = SkClipOp::kIntersect;
    }

    int newElementCount = earliestValid + 2;
    SkASSERT(latestActiveInvalid < earliestValid || latestActiveInvalid == earliestValid + 1);
    if (latestActiveInvalid < earliestValid) {
        // The new element will be stored in 'latestActiveInvalid', not the index after
        // 'earliestValid', so no need to keep an extra element available.
        newElementCount--;
    }

    pop_to_count(elements, newElementCount);
    set_item(elements, latestActiveInvalid, std::move(toAdd));

    // Any prior mask for the record doesn't count anymore, since the content has canged
    fGenID = next_gen_id();

    SkASSERT(elements->count() == newElementCount);
    return true;
}

void GrClipStack::SaveRecord::replaceWithElement(Element&& toAdd, Element::Allocator* elements) {
    // The aggregate state of the save record mirrors the element
    fInnerBounds = toAdd.innerBounds();
    fOuterBounds = toAdd.outerBounds();
    fStackOp = toAdd.op();
    fState = toAdd.clipType();

    // All prior active element can be removed from the stack: [startingIndex, count - 1]
    pop_to_count(elements, fStartingElementIndex + 1);
    set_item(elements, fStartingElementIndex, std::move(toAdd));
    SkASSERT(elements->count() == fStartingElementIndex + 1);

    // In addition, all inactive elements are invalidated, but this can be easily handled
    // by shifting the last valid index of the save record w/o iterating over old records.
    fLastValidIndex = fStartingElementIndex;

    // Any prior mask for the record doesn't count anymore, since the content has canged
    fGenID = next_gen_id();
}

bool GrClipStack::SaveRecord::apply(const Element::Allocator& elements,
                                    Mask::Allocator* masks,
                                    GrRecordingContext* context,
                                    GrRenderTargetContext* rtc,
                                    bool useHWAA,
                                    bool hasUserStencilSettings,
                                    GrAppliedClip* out,
                                    SkIRect* bounds,
                                    ApplyState* state) const {
    // Early out if we know a priori that the clip is full 0s or full 1s.
    if (fState == ClipState::kEmpty) {
        return false;
    } else if (fState == ClipState::kWideOpen) {
        return true;
    }

    // A refers to the clip stack, B refers to the draw
    ClipEffect overallEffect = get_clip_effect(*this, Draw(*bounds));
    if (overallEffect == ClipEffect::kBOnly) {
        // Draw not affected by the clip
        return true;
    } else if (overallEffect == ClipEffect::kEmpty) {
        // Draw fully clipped
        return false;
    }
    // Since draws don't provide inner bounds for contains checks, we don't expect kAOnly. If we
    // supported that and had sufficient plumbing, kAOnly could be used to change the draw to use
    // just the clip's final geometry directly.
    SkASSERT(overallEffect == ClipEffect::kBoth && (fState == ClipState::kRect ||
                                                    fState == ClipState::kRRect ||
                                                    fState == ClipState::kComplex));

    // We can determine a scissor based on the draw and the overall stack bounds.
    SkIRect scissoredDrawBounds = *bounds;
    if (fStackOp == SkClipOp::kIntersect) {
        // The save record's outer bounds is already the intersection of every
        // element's outer bounds, so we can apply it now.
        SkAssertResult(scissoredDrawBounds.intersect(fOuterBounds));
    } else {
        // If the difference ops' inner bounds fully cuts an edge of the draw,
        // we can shrink it down.
        scissoredDrawBounds = subtract(*bounds, fInnerBounds, /* exact */ true);
    }

    // We mark this true once we have a coverage FP (since complex clipping is occurring), or
    // we have an element that wouldn't affect the scissored draw bounds, but does affect the
    // regular draw bounds. In that case, the scissor is sufficient for clipping and we can skip the
    // element but definitely cannot then drop the scissor.
    bool scissorIsNeeded = false;

    // Stack size determined imperically, the maximum number of elements put in a SW mask was 4
    // across our set of GMs, SKPs, and SVGs used for testing.
    SkSTArray<4, const Element*> elementsForMask;
    int i = elements.count() - 1;
    for (const Element& e : elements.ritems()) {
        if (i < fLastValidIndex) {
            // All earlier elements have been invalidated by elements already processed
            break;
        }

        DrawEffect effect = e.affectsDraw(scissoredDrawBounds);
        if (effect == DrawEffect::kUnclipped) {
            // We don't need to produce a coverage FP from the element, but do another query
            // to determine if the scissor needs to be kept
            if (!scissorIsNeeded) {
                DrawEffect withoutScissor = e.affectsDraw(*bounds);
                SkASSERT(withoutScissor == DrawEffect::kClipped ||
                         withoutScissor == DrawEffect::kUnclipped);
                scissorIsNeeded = withoutScissor == DrawEffect::kClipped;
            }
            continue;
        } else if (effect == DrawEffect::kNoDraw) {
            // This can happen for difference op elements that have a larger fInnerBounds than
            // can be preserved at the net level.
            return false;
        } else {
            // The element must apply a coverage FP to the draw, so since we're doing some form
            // of shader-based clipping, also enable the scissor to limit overdraw.
            scissorIsNeeded = true;
            std::unique_ptr<GrFragmentProcessor> coverageFP = e.asCoverageFP(
                    scissoredDrawBounds, context, rtc, useHWAA, hasUserStencilSettings);
            if (coverageFP) {
                out->addCoverageFP(std::move(coverageFP));
            } else {
                // TODO - eventually an element will always produce an FP, but for now,
                // unhandled elements are accumulated and rasterized into a mask for later upload
                elementsForMask.push_back(&e);
            }
        }
    }

    if (!scissorIsNeeded) {
        // More detailed analysis of the element shapes determined no clip is needed
        SkASSERT(elementsForMask.empty() && !out->doesClip());
        return true;
    }

    out->hardClip().setScissor(scissoredDrawBounds);
    *bounds = scissoredDrawBounds;

    // Create a mask image that will handle all additional elements not explicitly represented
    // as coverage FPs
    if (!elementsForMask.empty()) {
        // FIXME knowing AA'ness of these elements is also handy since we could decide between
        // stencil and SW mask generation. For now just use the SW mask all the time.
        std::unique_ptr<GrFragmentProcessor> maskFP = this->getSWMaskFP(
                scissoredDrawBounds, elementsForMask.begin(), elementsForMask.count(),
                masks, context);
        if (maskFP) {
            out->addCoverageFP(std::move(maskFP));
        } else {
            // Mask generation failed, so don't draw
            return false;
        }
    }

    SkASSERT(out->doesClip());
    return true;
}

std::unique_ptr<GrFragmentProcessor> GrClipStack::SaveRecord::getSWMaskFP(
        const SkIRect& bounds, const Element** elements, int count, Mask::Allocator* masks,
        GrRecordingContext* context) const {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrSurfaceProxyView maskProxy;

    // Check the existing masks from this save record for compatibility
    for (const Mask& m : masks->ritems()) {
        if (m.genID() != fGenID) {
            break;
        }
        if (m.appliesToDraw(fGenID, bounds)) {
            maskProxy = proxyProvider->findCachedProxyWithColorTypeFallback(
                    m.key(), kMaskOrigin, GrColorType::kAlpha_8, 1);
            if (maskProxy) {
                break;
            }
        }
    }

    if (!maskProxy) {
        // No existing mask was found, so need to render a new one
        maskProxy = Element::RenderSWMask(bounds, elements, count, context);
        if (!maskProxy) {
            // If we still don't have one, there's nothing we can do
            return nullptr;
        }

        // Register the mask for later invalidation
        Mask& mask = masks->emplace_back(fGenID, bounds);
        proxyProvider->assignUniqueKeyToProxy(mask.key(), maskProxy.asTextureProxy());
    }

    // Wrap the mask in an FP that samples it for coverage
    SkASSERT(maskProxy && maskProxy.origin() == kMaskOrigin);

    GrSamplerState samplerState(GrSamplerState::WrapMode::kClampToBorder,
                                GrSamplerState::Filter::kNearest);
    auto m = SkMatrix::MakeTrans(-bounds.fLeft, -bounds.fTop);
    auto subset = SkRect::Make(bounds.size());
    // We scissor to bounds. The mask's texel centers are aligned to device space
    // pixel centers. Hence this domain of texture coordinates.
    auto domain = subset.makeInset(0.5, 0.5);
    auto fp = GrTextureEffect::MakeSubset(std::move(maskProxy), kPremul_SkAlphaType, m,
                                          samplerState, subset, domain, *context->priv().caps());
    return GrDeviceSpaceEffect::Make(std::move(fp));
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack

GrClipStack::GrClipStack(const SkIRect& deviceBounds)
        : fElements()
        , fSaves()
        , fMasks()
        , fProxyProvider(nullptr)
        , fDeviceBounds(deviceBounds) {
    fSaves.emplace_back(deviceBounds);
}

GrClipStack::~GrClipStack() {
    // Invalidate all mask keys that remain. Since we're tearing the clip stack down, we don't need
    // to go through SaveRecord.
    SkASSERT(fProxyProvider || fMasks.empty());
    if (fProxyProvider) {
        for (Mask& m : fMasks.ritems()) {
            m.invalidate(fProxyProvider);
        }
    }
}

void GrClipStack::save() {
    SkASSERT(!fSaves.empty());
    fSaves.back().pushSave();
}

void GrClipStack::restore() {
    SkASSERT(!fSaves.empty());
    SaveRecord& current = fSaves.back();
    if (current.popSave()) {
        // The save record still refers to elements, don't clean it up yet
        return;
    }

    // When we remove a save record, we delete all elements >= its starting index.
    current.removeElements(&fElements);
    SkASSERT(fProxyProvider || fMasks.empty());
    if (fProxyProvider) {
        current.invalidateMasks(fProxyProvider, &fMasks);
    }
    fSaves.pop_back();
    // With the new save record, restore any remaining elements that were invalidated
    // by current's elements.
    fSaves.back().restoreElements(&fElements);
}

SkIRect GrClipStack::bounds() const {
    const SaveRecord& current = this->currentSaveRecord();
    if (current.state() == ClipState::kEmpty) {
        return SkIRect::MakeEmpty();
    } else if (current.state() == ClipState::kWideOpen) {
        return fDeviceBounds;
    } else {
        if (current.op() == SkClipOp::kDifference) {
            // The outer/inner bounds represent what's cut out, so full bounds remains the device
            // bounds, minus any fully clipped content that spans the device edge.
            return subtract(fDeviceBounds, current.innerBounds(), /* exact */ true);
        } else {
            return current.outerBounds();
        }
    }
}

bool GrClipStack::apply(GrRecordingContext* context, GrRenderTargetContext* rtc, bool useHWAA,
                        bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds,
                        ApplyState* state) const {
    SkIRect pixelBounds;

    bounds->inset(GrClip::kBoundsTolerance, GrClip::kBoundsTolerance);
    // FIXME If we knew if the draw was AA or non-AA, we could do round() instead, but this
    // is conservative.
    bounds->roundOut(&pixelBounds);

    // Clip to device bounds now, since that clipping happens automatically, even if 'out'
    // remains wide open. This makes our query as tight as possible for contains checks later.
    if (!pixelBounds.intersect(fDeviceBounds)) {
        // The draw is offscreen
        return false;
    }

    if (!fProxyProvider) {
        fProxyProvider = context->priv().proxyProvider();
    }
    SkASSERT(fProxyProvider == context->priv().proxyProvider());

    bool shouldDraw = this->currentSaveRecord().apply(
            fElements, &fMasks, context, rtc, useHWAA, hasUserStencilSettings,
            out, &pixelBounds, state);
    *bounds = SkRect::Make(pixelBounds);
    return shouldDraw;
}

GrClipStack::SaveRecord& GrClipStack::writableSaveRecord(bool* wasDeferred) {
    SaveRecord& current = fSaves.back();
    if (current.canBeUpdated()) {
        // Current record is still open, so it can be modified directly
        *wasDeferred = false;
        return current;
    } else {
        // Must undefer the save to get a new record.
        SkAssertResult(current.popSave());
        *wasDeferred = true;
        return fSaves.emplace_back(current, fMasks.count(), fElements.count());
    }
}

void GrClipStack::clip(Element&& element) {
    // If we're already empty, a new element won't do anything so just exit early
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    // Reduce the path to anything simpler, will apply the transform if it's a scale+translate
    // and we don't need to worry about a path ID anymore, and ensures the element's
    // bounds are clipped to the device (or current limits of the clip, which is even tighter).
    element.simplify(this->bounds());

    // simplify() should have normalized this and corrected anything by changing the element's op
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
    SkDEBUGCODE(uint32_t oldGenID = save.genID();)
    SkDEBUGCODE(int elementCount = fElements.count();)
    if (!save.updateForElement(std::move(element), &fElements)) {
        if (wasDeferred) {
            // We made a new save record, but ended up not adding an element to the stack.
            // So instead of keeping an empty save record around, pop it off and restore the counter
            SkASSERT(elementCount == fElements.count());
            fSaves.pop_back();
            fSaves.back().pushSave();
        } else {
            // Should not have changed gen ID if the element and save were not modified
            SkASSERT(oldGenID == save.genID());
        }
    } else {
        // The gen ID should be new, and should not be invalid
        SkASSERT(oldGenID != save.genID() && save.genID() != kInvalidGenID);
        if (fProxyProvider && !wasDeferred) {
            // We modified an active save record so any old masks it had can be invalidated
            save.invalidateMasks(fProxyProvider, &fMasks);
        }
    }
}
