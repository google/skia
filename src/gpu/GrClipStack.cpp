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
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrStencilMaskHelper.h"
#include "src/gpu/GrSWMaskHelper.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/GrXfermodeFragmentProcessor.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "src/gpu/effects/generated/GrDeviceSpaceEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"

namespace {

enum class ClipGeometry {
    kEmpty,
    kAOnly,
    kBOnly,
    kBoth
};

// A and B can be Element, SaveRecord, or Draw. Supported combinations are, order not mattering,
// (Element, Element), (Element, SaveRecord), (Element, Draw), and (SaveRecord, Draw).
template<typename A, typename B>
static ClipGeometry get_clip_geometry(const A& a, const B& b) {
    // NOTE: SkIRect::Intersects() returns false when two rectangles touch at an edge (so the result
    // is empty). This behavior is desired for the following clip effect policies.
    if (a.op() == SkClipOp::kIntersect) {
        if (b.op() == SkClipOp::kIntersect) {
            // Intersect (A) + Intersect (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // Regions with non-zero coverage are disjoint, so intersection = empty
                return ClipGeometry::kEmpty;
            } else if (a.contains(b)) {
                // A's full coverage region contains entirety of B, so intersection = B
                return ClipGeometry::kBOnly;
            } else if (b.contains(a)) {
                // B's full coverage region contains entirety of A, so intersection = A
                return ClipGeometry::kAOnly;
            } else {
                // The shapes intersect in some non-trivial manner
                return ClipGeometry::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Intersect (A) + Difference (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // A only intersects B's full coverage region, so intersection = A
                return ClipGeometry::kAOnly;
            } else if (b.contains(a)) {
                // B's zero coverage region completely contains A, so intersection = empty
                return ClipGeometry::kEmpty;
            } else {
                // Intersection cannot be simplified. Note that the combination of a intersect
                // and difference op in this order cannot produce kBOnly
                return ClipGeometry::kBoth;
            }
        }
    } else {
        SkASSERT(a.op() == SkClipOp::kDifference);
        if (b.op() == SkClipOp::kIntersect) {
            // Difference (A) + Intersect (B) - the mirror of Intersect(A) + Difference(B),
            // but combining is commutative so this is equivalent barring naming.
            if (!SkIRect::Intersects(b.outerBounds(), a.outerBounds())) {
                // B only intersects A's full coverage region, so intersection = B
                return ClipGeometry::kBOnly;
            } else if (a.contains(b)) {
                // A's zero coverage region completely contains B, so intersection = empty
                return ClipGeometry::kEmpty;
            } else {
                // Cannot be simplified
                return ClipGeometry::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Difference (A) + Difference (B)
            if (a.contains(b)) {
                // A's zero coverage region contains B, so B doesn't remove any extra
                // coverage from their intersection.
                return ClipGeometry::kAOnly;
            } else if (b.contains(a)) {
                // Mirror of the above case, intersection = B instead
                return ClipGeometry::kBOnly;
            } else {
                // Intersection of the two differences cannot be simplified. Note that for
                // this op combination it is not possible to produce kEmpty.
                return ClipGeometry::kBoth;
            }
        }
    }
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

static GrClipEdgeType get_clip_edge_type(SkClipOp op, GrAA aa) {
    if (op == SkClipOp::kIntersect) {
        return aa == GrAA::kYes ? GrClipEdgeType::kFillAA : GrClipEdgeType::kFillBW;
    } else {
        return aa == GrAA::kYes ? GrClipEdgeType::kInverseFillAA : GrClipEdgeType::kInverseFillBW;
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

// Functions for rendering / applying clip shapes in various ways
// The general strategy is:
//  - Represent the clip element as an analytic FP that tests sk_FragCoord vs. its device-space shape
//  - Render the clip element to the stencil, if stencil is allowed and supports the AA, and the
//    size of the element indicates stenciling will be worth it, vs. making a mask.
//  - Try to put the individual element into a clip atlas, which is then sampled during the draw
//  - Render the element into a SW mask and upload it. If possible, the SW rasterization happens
//    in parallel.
static constexpr GrSurfaceOrigin kMaskOrigin = kTopLeft_GrSurfaceOrigin;

static GrFPResult analytic_clip_fp(const GrClipStack::Element& e,
                                   const GrShaderCaps& caps,
                                   std::unique_ptr<GrFragmentProcessor> fp) {
    // All analytic clip shape FPs need to be in device space
    GrClipEdgeType edgeType = get_clip_edge_type(e.fOp, e.fAA);
    if (e.fLocalToDevice.isIdentity()) {
        if (e.fShape.isRect()) {
            return GrFPSuccess(GrAARectEffect::Make(std::move(fp), edgeType, e.fShape.rect()));
        } else if (e.fShape.isRRect()) {
            return GrRRectEffect::Make(std::move(fp), edgeType, e.fShape.rrect(), caps);
        }
    }

    // A convex hull can be transformed into device space (this will handle rect shapes with a
    // non-identity transform).
    if (e.fShape.segmentMask() == SkPath::kLine_SegmentMask && e.fShape.convex()) {
        SkPath devicePath;
        e.fShape.asPath(&devicePath);
        devicePath.transform(e.fLocalToDevice);
        return GrConvexPolyEffect::Make(std::move(fp), edgeType, devicePath);
    }

    return GrFPFailure(std::move(fp));
}

static void draw_to_sw_mask(GrSWMaskHelper* helper, const GrClipStack::Element& e, bool clearMask) {
    // If the first element to draw is an intersect, we clear to 0 and will draw it directly with
    // coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
    // If the first element to draw is a difference, we clear to 1, and in all cases we draw the
    // difference element directly with coverage 0.
    if (clearMask) {
        helper->clear(e.fOp == SkClipOp::kIntersect ? 0x00 : 0xFF);
    }

    uint8_t alpha;
    bool invert;
    if (e.fOp == SkClipOp::kIntersect) {
        // Intersect modifies pixels outside of its geometry. If this isn't the first op, we
        // draw the inverse-filled shape with 0 coverage to erase everything outside the element
        // But if we are the first element, we can draw directly with coverage 1 since we
        // cleared to 0.
        if (clearMask) {
            alpha = 0xFF;
            invert = false;
        } else {
            alpha = 0x00;
            invert = true;
        }
    } else {
        // For difference ops, can always just subtract the shape directly by drawing 0 coverage
        SkASSERT(e.fOp == SkClipOp::kDifference);
        alpha = 0x00;
        invert = false;
    }

    // Draw the shape; based on how we've initialized the buffer and chosen alpha+invert,
    // every element is drawn with the kReplace_Op
    if (invert) {
        // Must invert the path
        SkASSERT(!e.fShape.inverted());
        // TODO: this is an extra copy effectively, just so we can toggle inversion; would be
        // better perhaps to just call a drawPath() since we know it'll use path rendering w/
        // the inverse fill type.
        GrShape inverted = e.fShape;
        inverted.setInverted(true);
        helper->drawShape(inverted, e.fLocalToDevice, SkRegion::kReplace_Op, e.fAA, alpha);
    } else {
        helper->drawShape(e.fShape, e.fLocalToDevice, SkRegion::kReplace_Op, e.fAA, alpha);
    }
}

// TODO: Currently this only works with CCPR because CCPR owns and manages the clip atlas. The
// high-level concept should be generalized to support any path renderer going into a shared atlas.
static std::unique_ptr<GrFragmentProcessor> clip_atlas_fp(GrCoverageCountingPathRenderer* ccpr,
                                                          uint32_t opsTaskID,
                                                          const SkIRect& bounds,
                                                          const GrClipStack::Element& e,
                                                          const GrCaps& caps,
                                                          std::unique_ptr<GrFragmentProcessor> fp) {
    // TODO: Currently the atlas manages device-space paths, so we have to transform by the ctm.
    // In the future, it the atlas manager should see the local path and the ctm so that it can
    // cache across integer-only translations (internally, it already does this, just not exposed).
    SkPath devicePath;
    e.fShape.asPath(&devicePath);
    devicePath.transform(e.fLocalToDevice);

    SkASSERT(!devicePath.isInverseFillType());
    if (e.fOp == SkClipOp::kIntersect) {
        return ccpr->makeClipProcessor(std::move(fp), opsTaskID, devicePath, bounds, caps);
    } else {
        // Use kSrcIn to convert the non-inverted mask alpha into (1-alpha), but this way the
        // clip atlas only ever renders non-inverse filled paths.
        return GrXfermodeFragmentProcessor::Make(
                ccpr->makeClipProcessor(nullptr, opsTaskID, devicePath, bounds, caps),
                std::move(fp),
                SkBlendMode::kSrcIn,
                GrXfermodeFragmentProcessor::ComposeBehavior::kSkModeBehavior);
    }
}

} // anonymous namespace

class GrClipStack::Draw {
public:
    Draw(const SkRect& pixelBounds, GrAAType aa)
            : fBounds(GrClip::GetPixelIBounds(pixelBounds, aa)) {}
    Draw(const SkIRect& pixelBounds) : fBounds(pixelBounds) {}

    // Common clip type interface
    SkClipOp op() const { return SkClipOp::kIntersect; }
    const SkIRect& outerBounds() const { return fBounds; }
    bool contains(const RawElement& e) const { return false; }
    bool contains(const SaveRecord& s) const { return false; }

    bool applyDeviceBounds(const SkIRect& deviceBounds) {
        return fBounds.intersect(deviceBounds);
    }

private:
    SkIRect  fBounds;
};

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::Element

GrClipStack::RawElement::RawElement(const SkMatrix& localToDevice, const GrShape& shape,
                                    GrAA aa, SkClipOp op)
        : Element{shape, localToDevice, op, aa}
        , fInvalidatedByIndex(-1) {}

GrClipStack::RawElement::RawElement(const RawElement& e) {
    *this = e;
}

GrClipStack::RawElement& GrClipStack::RawElement::operator=(const RawElement& e) {
    fShape = e.fShape;
    fLocalToDevice = e.fLocalToDevice;
    fOp = e.fOp;
    fAA = e.fAA;
    fInnerBounds = e.fInnerBounds;
    fOuterBounds = e.fOuterBounds;
    fInvalidatedByIndex = e.fInvalidatedByIndex;
    return *this;
}

/*
bool GrClipStack::Element::contains(const Draw& d) const {
    if (d.isRect()) {

    } else if (d.isRRect()) {

    } else {
        return this->contains(d.outerBounds());
    }
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

            // FIXME this logic is not correct; it is assuming that every corner touches an edge
            // and that is just not right for skew/persp matrices. They have corners that are not
            // an extreme relative to the device bounds, but do probably only need 1 point to map to

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
*/

void GrClipStack::RawElement::simplify(const SkIRect& deviceBounds) {
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

    // Transform rects and rrects with scale+translate matrices directly to device space, which
    // lets us combine clip elements more generally since they'll now share the identity transform.
    bool deviceBoundsApplied = false;
    if (!fLocalToDevice.isIdentity() && fLocalToDevice.isScaleTranslate()) {
        if (fShape.isRect()) {
            fShape.rect() = fLocalToDevice.mapRect(fShape.rect());
            fLocalToDevice = SkMatrix::I();

            if (!fShape.rect().intersect(SkRect::Make(deviceBounds))) {
                fShape.reset();
            }
            deviceBoundsApplied = true;
        } else if (fShape.isRRect()) {
            SkRRect src = fShape.rrect();
            SkAssertResult(src.transform(fLocalToDevice, &fShape.rrect()));
            fLocalToDevice = SkMatrix::I();

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

void GrClipStack::RawElement::updateBounds() {
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
    if (!fLocalToDevice.isIdentity()) {
        // TODO: could theoretically compute an inner mapped rect for the updated inner bounds
        inner = SkRect::MakeEmpty();
        outer = fLocalToDevice.mapRect(outer);
    }

    // When the clip is non-AA, the pixel bounds are simply rounded. If the clip is AA, the outer
    // bounds are rounded out and the inner bounds are rounded in. We apply a little fudge tolerance
    // to the bounds before rounding.
    if (fAA == GrAA::kYes) {
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

bool GrClipStack::RawElement::combine(const RawElement& other) {
    // To reduce the number of possibilities, only consider intersect+intersect. Difference and
    // mixed op cases could be analyzed to simplify one of the shapes, but that is a rare
    // occurrence and the math is much more complicated.
    if (fOp != other.fOp || fOp != SkClipOp::kIntersect) {
        return false;
    }
    // TODO depending on if these are rects and we track edge flags, or if the edges are on
    // pixel boundaries w/o needing rounded corners anymore, we could mix aa
    if (fAA != other.fAA) {
        return false;
    }
    // Shapes must be in the same coordinate space to be combined together
    if (fLocalToDevice != other.fLocalToDevice) {
        return false;
    }

    // Can combine rects with rects, and rrects with rrects. rect+rrect is promoted to rrect+rrect
    bool shapeUpdated = false;
    if (fShape.isRect() && other.fShape.isRect()) {
        SkAssertResult(fShape.rect().intersect(other.fShape.rect()));
        shapeUpdated = true;
    } else if ((fShape.isRect() || fShape.isRRect()) &&
               (other.fShape.isRect() || other.fShape.isRRect())) {
        // Treat rrect+rect intersections as rrect+rrect
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

void GrClipStack::RawElement::updateForElement(RawElement* added, int addedIndex) {
    if (fInvalidatedByIndex >= 0) {
        // Already doesn't do anything, so skip this element
        return;
    }

    // A refers to this element, B refers to 'added'.
    switch(get_clip_geometry(*this, *added)) {
        case ClipGeometry::kEmpty:
            // Mark both elements as invalid to signal that the clip is fully empty
            fInvalidatedByIndex = addedIndex;
            added->fInvalidatedByIndex = addedIndex;
            break;

        case ClipGeometry::kAOnly:
            // This element already clips more than 'added', so mark 'added' is invalid so it's skipped
            added->fInvalidatedByIndex = addedIndex;
            break;

        case ClipGeometry::kBOnly:
            // 'added' clips more than this element, so mark this as invalid
            fInvalidatedByIndex = addedIndex;
            break;

        case ClipGeometry::kBoth:
            // Else the bounds checks think we need to keep both, but depending on the combination
            // of the ops and shape kinds, we may be able to do better.
            if (added->combine(*this)) {
                // 'added' now fully represents the combination of the two elements
                fInvalidatedByIndex = addedIndex;
            }
            break;
    }
}

GrClipStack::ClipState GrClipStack::RawElement::clipType() const {
    // Map from the internal shape kind to the clip state enum
    switch(fShape.type()) {
        case GrShape::Type::kEmpty:
            return ClipState::kEmpty;

        case GrShape::Type::kRect:
            return fLocalToDevice.isIdentity() ? ClipState::kDeviceRect : ClipState::kComplex;

        case GrShape::Type::kRRect:
            return fLocalToDevice.isIdentity() ? ClipState::kDeviceRRect : ClipState::kComplex;

        case GrShape::Type::kArc:
        case GrShape::Type::kLine:
        case GrShape::Type::kPoint:
            // These types should never become RawElements
            SkASSERT(false);
            [[fallthrough]];

        case GrShape::Type::kPath:
            return ClipState::kComplex;
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
        , fShader(nullptr)
        , fStartingMaskIndex(0)
        , fStartingElementIndex(0)
        , fOldestValidIndex(0)
        , fDeferredSaveCount(0)
        , fStackOp(SkClipOp::kIntersect)
        , fState(ClipState::kWideOpen)
        , fGenID(kInvalidGenID) {}

GrClipStack::SaveRecord::SaveRecord(const SaveRecord& prior,
                                    int startingMaskIndex,
                                    int startingElementIndex)
        : fInnerBounds(prior.fInnerBounds)
        , fOuterBounds(prior.fOuterBounds)
        , fShader(prior.fShader)
        , fStartingMaskIndex(startingMaskIndex)
        , fStartingElementIndex(startingElementIndex)
        , fOldestValidIndex(prior.fOldestValidIndex)
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

GrClipStack::ClipState GrClipStack::SaveRecord::state() const {
    if (fShader && fState != ClipState::kEmpty) {
        return ClipState::kComplex;
    } else {
        return fState;
    }
}

bool GrClipStack::SaveRecord::contains(const GrClipStack::Draw& draw) const {
    return fInnerBounds.contains(draw.outerBounds());
}

bool GrClipStack::SaveRecord::contains(const GrClipStack::RawElement& element) const {
    return fInnerBounds.contains(element.outerBounds());
}

void GrClipStack::SaveRecord::removeElements(RawElement::Stack* elements) {
    // Should only be called when we're about to be destroyed
    SkASSERT(fDeferredSaveCount == -1);
    while(elements->count() > fStartingElementIndex) {
        elements->pop_back();
    }
}

void GrClipStack::SaveRecord::restoreElements(RawElement::Stack* elements) {
    // Presumably this SaveRecord is the new top of the stack, and so it owns the elements
    // from its starting index to restoreCount - 1. Elements from the old save record have
    // been destroyed already, so their indices would have been >= restoreCount, and any
    // still-present element can be un-invalidated based on that.
    int restoreCount = elements->count();
    int i = restoreCount - 1;
    for (RawElement& e : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        e.restoreValid(restoreCount);
        --i;
    }
}

void GrClipStack::SaveRecord::invalidateMasks(GrProxyProvider* proxyProvider,
                                              Mask::Stack* masks) {
    // Must explicitly invalidate the key before removing the mask object from the stack
    while(masks->count() > fStartingMaskIndex) {
        SkASSERT(masks->back().genID() == fGenID && proxyProvider);
        masks->back().invalidate(proxyProvider);
        masks->pop_back();
    }
    SkASSERT(masks->empty() || masks->back().genID() != fGenID);
}

void GrClipStack::SaveRecord::reset(const SkIRect& bounds) {
    SkASSERT(this->canBeUpdated());
    fOldestValidIndex = fStartingElementIndex;
    fOuterBounds = bounds;
    fInnerBounds = bounds;
    fStackOp = SkClipOp::kIntersect;
    fState = ClipState::kWideOpen;
    fShader = nullptr;
}

void GrClipStack::SaveRecord::addShader(sk_sp<SkShader> shader) {
    SkASSERT(shader);
    SkASSERT(this->canBeUpdated());
    if (!fShader) {
        fShader = std::move(shader);
    } else {
        fShader = SkShaders::Blend(SkBlendMode::kSrcIn, std::move(shader), fShader);
    }
}

bool GrClipStack::SaveRecord::addElement(RawElement&& toAdd, RawElement::Stack* elements) {
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
    switch(get_clip_geometry(*this, toAdd)) {
        case ClipGeometry::kEmpty:
            // The combination results in an empty clip
            fState = ClipState::kEmpty;
            return true;

        case ClipGeometry::kAOnly:
            // The combination would not be any different than the existing clip
            return false;

        case ClipGeometry::kBOnly:
            // The combination would invalidate the entire existing stack and can be replaced with
            // just the new element.
            this->replaceWithElement(std::move(toAdd), elements);
            return true;

        case ClipGeometry::kBoth:
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
            //  - The updated outer bounds is the union of outer bounds and the inner becomes the
            //    largest of the two possible inner bounds
            fOuterBounds.join(toAdd.outerBounds());
            if (toAdd.innerBounds().width() * toAdd.innerBounds().height() >
                fInnerBounds.width() * fInnerBounds.height()) {
                fInnerBounds = toAdd.innerBounds();
            }
        }
    }

    // If we get here, we're keeping the new element and the stack's bounds have been updated.
    // We ought to have caught the cases where the stack bounds resemble an empty or wide open
    // clip, so assert that's the case.
    SkASSERT(!fOuterBounds.isEmpty() &&
             (fInnerBounds.isEmpty() || fOuterBounds.contains(fInnerBounds)));

    return this->appendElement(std::move(toAdd), elements);
}

bool GrClipStack::SaveRecord::appendElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // Update past elements to account for the new element
    int i = elements->count() - 1;

    // After the loop, elements between [max(youngestValid, startingIndex)+1, count-1] can be
    // removed from the stack (these are the active elements that have been invalidated by the
    // newest element; since its the active part of the stack, no restore() can bring them back).
    int youngestValid = fStartingElementIndex - 1;
    // After the loop, elements between [0, oldestValid-1] are all invalid. The value of oldestValid
    // becomes the save record's new fLastValidIndex value.
    int oldestValid = i + 1;
    // After the loop, this is the earliest active element that was invalidated. It may be
    // older in the stack than earliestValid, so cannot be popped off, but can be used to store
    // the new element instead of allocating more.
    RawElement* oldestActiveInvalid = nullptr;
    int oldestActiveInvalidIndex = i + 1;

    for (RawElement& existing : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        // We don't need to pass the actual index that toAdd will be saved to; just the minimum
        // index of this save record, since that will result in the same restoration behavior later.
        existing.updateForElement(&toAdd, fStartingElementIndex);

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

    // Sanity check the indices just computed during the loop
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
    if (!oldestActiveInvalid || oldestActiveInvalidIndex > targetCount) {
        // toAdd will be stored right after youngestValid
        targetCount++;
        oldestActiveInvalid = nullptr;
    }
    while(elements->count() > targetCount) {
        elements->pop_back();
    }
    if (oldestActiveInvalid) {
        *oldestActiveInvalid = std::move(toAdd);
    } else if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back() = std::move(toAdd);
    }

    // Changing this will prompt GrClipStack to invalidate any masks associated with this record.
    fGenID = next_gen_id();
    return true;
}

void GrClipStack::SaveRecord::replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // The aggregate state of the save record mirrors the element
    fInnerBounds = toAdd.innerBounds();
    fOuterBounds = toAdd.outerBounds();
    fStackOp = toAdd.op();
    fState = toAdd.clipType();

    // All prior active element can be removed from the stack: [startingIndex, count - 1]
    int targetCount = fStartingElementIndex + 1;
    while(elements->count() > targetCount) {
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
    fGenID = next_gen_id();
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack

GrClipStack::GrClipStack(const SkIRect& deviceBounds, const SkMatrixProvider* matrixProvider)
        : fElements()
        , fSaves()
        , fMasks()
        , fProxyProvider(nullptr)
        , fDeviceBounds(deviceBounds)
        , fMatrixProvider(matrixProvider) {
    // Start with a save record that is wide open
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
        // This was just a deferred save being undone, so the record doesn't need to be removed yet
        return;
    }

    // When we remove a save record, we delete all elements >= its starting index and any masks
    // that were rasterized for it.
    current.removeElements(&fElements);
    SkASSERT(fProxyProvider || fMasks.empty());
    if (fProxyProvider) {
        current.invalidateMasks(fProxyProvider, &fMasks);
    }
    fSaves.pop_back();
    // Restore any remaining elements that were only invalidated by the now-removed save record.
    fSaves.back().restoreElements(&fElements);
}

SkIRect GrClipStack::getConservativeBounds() const {
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

GrClip::PreClipResult GrClipStack::preApply(const SkRect& bounds, GrAAType aa) const {
    Draw draw(bounds, aa);
    if (!draw.applyDeviceBounds(fDeviceBounds)) {
        return GrClip::Effect::kClippedOut;
    }

    const SaveRecord& cs = this->currentSaveRecord();
    // Early out if we know a priori that the clip is full 0s or full 1s.
    if (cs.state() == ClipState::kEmpty) {
        return GrClip::Effect::kClippedOut;
    } else if (cs.state() == ClipState::kWideOpen) {
        SkASSERT(!cs.shader());
        return GrClip::Effect::kUnclipped;
    }

    // Given argument order, A == current clip, B == draw
    switch(get_clip_geometry(cs, draw)) {
        case ClipGeometry::kEmpty:
            return GrClip::Effect::kClippedOut;

        case ClipGeometry::kBOnly:
            // Geometrically, the draw is unclipped, but can't ignore a shader
            return cs.shader() ? GrClip::Effect::kClipped : GrClip::Effect::kUnclipped;

        case ClipGeometry::kAOnly:
            // Shouldn't happen since the inner bounds of a draw are unknown
            SkASSERT(false);
            [[fallthrough]];

        case ClipGeometry::kBoth: {
            SkASSERT(fElements.count() > 0);
            const RawElement& back = fElements.back();
            if (cs.state() == ClipState::kDeviceRect) {
                SkASSERT(back.clipType() == ClipState::kDeviceRect);
                return GrClip::PreClipResult(back.shape().rect(), back.aa());
            } else if (cs.state() == ClipState::kDeviceRRect) {
                SkASSERT(back.clipType() == ClipState::kDeviceRRect);
                return GrClip::PreClipResult(back.shape().rrect(), back.aa());
            } else {
                // The clip stack has complex shapes or multiple elements; we could iterate per
                // element like we would in apply(), but preApply() is meant to be conservative
                // and efficient.
                SkASSERT(cs.state() == ClipState::kComplex);
                return GrClip::Effect::kClipped;
            }
            break;
        }
    }
}

GrClip::Effect GrClipStack::apply(GrRecordingContext* context, GrRenderTargetContext* rtc,
                                  GrAAType aa, bool hasUserStencilSettings,
                                  GrAppliedClip* out, SkRect* bounds) const {
    // TODO: Once we no longer store SW masks, we don't need to sneak the provider in like this
    if (!fProxyProvider) {
        fProxyProvider = context->priv().proxyProvider();
    }
    SkASSERT(fProxyProvider == context->priv().proxyProvider());
    const GrCaps* caps = context->priv().caps();

    // Convert the bounds to a Draw and apply device bounds clipping, making our query as tight
    // as possible.
    Draw draw(*bounds, aa);
    if (!draw.applyDeviceBounds(fDeviceBounds)) {
        return Effect::kClippedOut;
    }

    const SaveRecord& cs = this->currentSaveRecord();
    // Early out if we know a priori that the clip is full 0s or full 1s.
    if (cs.state() == ClipState::kEmpty) {
        return Effect::kClippedOut;
    } else if (cs.state() == ClipState::kWideOpen) {
        SkASSERT(!cs.shader());
        return Effect::kUnclipped;
    }

    // Convert any clip shader first, since it's not geometrically related to the draw bounds
    std::unique_ptr<GrFragmentProcessor> clipFP = nullptr;
    if (cs.shader()) {
        static const GrColorInfo kCoverageColorInfo{GrColorType::kUnknown, kPremul_SkAlphaType,
                                                    nullptr};
        GrFPArgs args(context, *fMatrixProvider, kNone_SkFilterQuality, &kCoverageColorInfo);
        clipFP = as_SB(cs.shader())->asFragmentProcessor(args);
        if (clipFP) {
            clipFP = GrFragmentProcessor::SwizzleOutput(std::move(clipFP), GrSwizzle::AAAA());
        }
    }

    // A refers to the entire clip stack, B refers to the draw
    switch(get_clip_geometry(cs, draw)) {
        case ClipGeometry::kEmpty:
            return Effect::kClippedOut;

        case ClipGeometry::kBOnly:
            // Geometrically unclipped, but may need to add the shader as a coverage FP
            if (clipFP) {
                out->addCoverageFP(std::move(clipFP));
                return Effect::kClipped;
            } else {
                return Effect::kUnclipped;
            }

        case ClipGeometry::kAOnly:
            // Shouldn't happen since draws don't report inner bounds
            SkASSERT(false);
            [[fallthrough]];

        case ClipGeometry::kBoth:
            // The draw is combined with the saved clip elements; the below logic tries to skip
            // as many elements as possible.
            SkASSERT(cs.state() == ClipState::kDeviceRect ||
                     cs.state() == ClipState::kDeviceRRect ||
                     cs.state() == ClipState::kComplex);
            break;
    }

    // We can determine a scissor based on the draw and the overall stack bounds.
    SkIRect scissorBounds;
    if (cs.op() == SkClipOp::kIntersect) {
        scissorBounds = draw.outerBounds();
        SkAssertResult(scissorBounds.intersect(cs.outerBounds()));
    } else {
        scissorBounds = subtract(draw.outerBounds(), cs.innerBounds(), /* exact */ true);
    }
    Draw scissoredDraw(scissorBounds);

    // We mark this true once we have a coverage FP (since complex clipping is occurring), or we
    // have an element that wouldn't affect the scissored draw bounds, but does affect the regular
    // draw bounds. In that case, the scissor is sufficient for clipping and we can skip the
    // element but definitely cannot then drop the scissor.
    bool scissorIsNeeded = SkToBool(cs.shader());

    // An default count of 4 was chosen because of the common pattern in Blink of:
    //   isect RR
    //   diff  RR
    //   isect convex_poly
    //   isect convex_poly
    // when drawing rounded div borders. Any clip shader FP is not counted to this limit.
    int remainingAnalyticFPs = 4;
    if (rtc->numSamples() > 1 || aa == GrAAType::kMSAA || hasUserStencilSettings) {
        // Disable analytic clips when we have MSAA. In MSAA we never conflate coverage and opacity.
        remainingAnalyticFPs = 0;
        // We disable MSAA when avoiding stencil so shouldn't get here.
        SkASSERT(!context->priv().caps()->avoidStencilBuffers());
    }

    // If window rectangles are supported, we can use them to exclude inner bounds of difference ops
    int maxWindowRectangles = rtc->priv().maxWindowRectangles();
    GrWindowRectangles windowRects;

    // Stack size determined empirically, the maximum number of elements put in a SW mask was 4
    // across our set of GMs, SKPs, and SVGs used for testing. Elements not represented as an
    // analytic FP or skipped will be collected here and later applied by using the stencil buffer,
    // CCPR clip atlas, or a cached SW mask.
    SkSTArray<4, const RawElement*> elementsForMask;
    bool maskRequiresAA = false;
    int i = fElements.count() - 1;
    for (const RawElement& e : fElements.ritems()) {
        if (i < cs.oldestElementIndex()) {
            // All earlier elements have been invalidated by elements already processed
            break;
        } else if (e.isInvalid()) {
            continue;
        }

        out->fElementsConsidered++;

        switch(get_clip_geometry(e, scissoredDraw)) {
            case ClipGeometry::kEmpty:
                // This can happen for difference op elements that have a larger fInnerBounds than
                // can be preserved at the net level.
                return Effect::kClippedOut;

            case ClipGeometry::kBOnly:
                // We don't need to produce a coverage FP or mask for the element, but we do need
                // to test if this requires the scissor
                if (!scissorIsNeeded) {
                    ClipGeometry withoutScissor = get_clip_geometry(e, draw);
                    SkASSERT(withoutScissor == ClipGeometry::kBoth ||
                             withoutScissor == ClipGeometry::kBOnly);
                    out->fElementsConsidered++;
                    scissorIsNeeded = withoutScissor == ClipGeometry::kBoth;
                }
                break;

            case ClipGeometry::kAOnly:
                // Shouldn't happen for draws, fall through to regular element processing
                SkASSERT(false);
                [[fallthrough]];

            case ClipGeometry::kBoth: {
                // The element must apply coverage to the draw, enable the scissor to limit overdraw
                scissorIsNeeded = true;

                bool success = false;
                if (remainingAnalyticFPs > 0) {
                    std::tie(success, clipFP) = analytic_clip_fp(e.asElement(), *caps->shaderCaps(),
                                                                 std::move(clipFP));
                    if (success) {
                        out->fAnalyticElements++;
                        remainingAnalyticFPs--;
                    }
                }

                if (!success) {
                    // TODO - eventually an element will always produce an FP, but for now,
                    // unhandled elements are accumulated and rasterized into a mask
                    elementsForMask.push_back(&e);
                    maskRequiresAA |= (e.aa() == GrAA::kYes);
                }

                // If we have room for it, use window rects to exclude pixels that are definitely
                // clipped by a difference op (the intersect case is handled by the scissor).
                if (e.op() == SkClipOp::kDifference && !e.innerBounds().isEmpty() &&
                    out->windowRectsState().numWindows() < maxWindowRectangles) {
                    windowRects.addWindow(e.innerBounds());
                }
                break;
            }
        }
    }

    if (!scissorIsNeeded) {
        // More detailed analysis of the element shapes determined no clip is needed
        SkASSERT(elementsForMask.empty() && !clipFP);
        return Effect::kUnclipped;
    }

    // Fill out the GrAppliedClip with what we know so far
    out->hardClip().setScissor(scissorBounds);
    if (!windowRects.empty()) {
        out->hardClip().addWindowRectangles(windowRects, GrWindowRectsState::Mode::kExclusive);
    }
    if (clipFP) {
        out->addCoverageFP(std::move(clipFP));
    }

    // Now rasterize any remaining elements, either to the stencil, a SW mask, or an atlas
    if (!elementsForMask.empty()) {
        // First if we have CCPR and any remaining FP slots, put them in the clip atlas, although
        // we can't create those FPs until after the stencil/SW mask rendering is done, so that the
        // ops task id is stable.
        auto* ccpr = context->priv().drawingManager()->getCoverageCountingPathRenderer();
        int ccprCount = ccpr ? std::min(elementsForMask.count(), remainingAnalyticFPs) : 0;

        // Handle flattened mask generation
        int flattenCount = elementsForMask.count() - ccprCount;
        out->fMaskElements = flattenCount;
        out->fCCPRElements = ccprCount;
        if (flattenCount > 0) {
            std::unique_ptr<GrFragmentProcessor> maskFP = nullptr;
            bool stencilUnavailable = context->priv().caps()->avoidStencilBuffers() ||
                                      rtc->wrapsVkSecondaryCB();
            if ((rtc->numSamples() <= 1 && maskRequiresAA) || stencilUnavailable) {
                // Must use a texture mask to represent the combined clip elements since the stencil
                // cannot be used, or cannot handle smooth clips.
                maskFP = GetSWMaskFP(context, &fMasks, cs.genID(), scissorBounds,
                                     elementsForMask.begin() + ccprCount, flattenCount,
                                     &out->fMaskInCache);
            }

            if (maskFP) {
                out->addCoverageFP(std::move(maskFP));
            } else if (stencilUnavailable) {
                // Mask generation failed and we can't rely on the stencil to satisfy it
                SkDebugf("WARNING: Clip mask requires stencil, but stencil unavailable. "
                         "Draw will be ignored.\n");
                return Effect::kClippedOut;
            } else {
                // Rasterize the remaining elements to the stencil buffer
                GrStencilMaskHelper helper(context, rtc);
                if (helper.init(scissorBounds, cs.genID(), out->windowRectsState().windows(), 0)) {
                    helper.clear(cs.op() == SkClipOp::kDifference);
                    for (int i = ccprCount; i < elementsForMask.count(); ++i) {
                        const Element& e = elementsForMask[i]->asElement();
                        SkRegion::Op op = e.fOp == SkClipOp::kIntersect ? SkRegion::kIntersect_Op
                                                                        : SkRegion::kDifference_Op;
                        helper.drawShape(e.fShape, e.fLocalToDevice, op, e.fAA);
                    }
                    helper.finish();
                } else {
                    out->fMaskInCache = true;
                }
                out->hardClip().addStencilClip(cs.genID());
            }
        }

        // Finish CCPR paths now that the render target's ops task is stable.
        if (ccprCount) {
            uint32_t opsTaskID = rtc->getOpsTask()->uniqueID();
            std::unique_ptr<GrFragmentProcessor> ccprFP = nullptr;
            for (int i = 0; i < ccprCount; ++i) {
                ccprFP = clip_atlas_fp(ccpr, opsTaskID, scissorBounds,
                                       elementsForMask[i]->asElement(), *caps, std::move(ccprFP));
            }
            out->addCoverageFP(std::move(ccprFP));
        }
    }

    SkASSERT(out->doesClip());
    SkAssertResult(bounds->intersect(SkRect::Make(scissorBounds)));
    return Effect::kClipped;
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

void GrClipStack::clipShader(sk_sp<SkShader> shader) {
    // Shaders can't bring additional coverage
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    bool wasDeferred;
    this->writableSaveRecord(&wasDeferred).addShader(std::move(shader));
    // Masks and geometry elements are not invalidated by updating the clip shader
}

void GrClipStack::replaceClip(const SkIRect& rect) {
    bool wasDeferred;
    SaveRecord& save = this->writableSaveRecord(&wasDeferred);

    if (!wasDeferred) {
        save.removeElements(&fElements);
        save.invalidateMasks(fProxyProvider, &fMasks);
    }

    save.reset(fDeviceBounds);
    if (rect != fDeviceBounds) {
        this->clipRect(SkMatrix::I(), SkRect::Make(rect), GrAA::kNo, SkClipOp::kIntersect);
    }
}

void GrClipStack::clip(RawElement&& element) {
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    // Reduce the path to anything simpler, will apply the transform if it's a scale+translate
    // and we don't need to worry about a path ID anymore, and ensures the element's
    // bounds are clipped to the device (or current limits of the clip, which is even tighter).
    element.simplify(this->getConservativeBounds());
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
    if (!save.addElement(std::move(element), &fElements)) {
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

std::unique_ptr<GrFragmentProcessor> GrClipStack::GetSWMaskFP(GrRecordingContext* context,
                                                              Mask::Stack* masks,
                                                              uint32_t genID, const SkIRect& bounds,
                                                              const RawElement** elements,
                                                              int count, bool* cached) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrSurfaceProxyView maskProxy;

    // Check the existing masks from this save record for compatibility
    for (const Mask& m : masks->ritems()) {
        if (m.genID() != genID) {
            break;
        }
        if (m.appliesToDraw(genID, bounds)) {
            maskProxy = proxyProvider->findCachedProxyWithColorTypeFallback(
                    m.key(), kMaskOrigin, GrColorType::kAlpha_8, 1);
            if (maskProxy) {
                *cached = true;
                break;
            }
        }
    }

    if (!maskProxy) {
        // No existing mask was found, so need to render a new one
        maskProxy = RenderSWMask(context, bounds, elements, count);
        if (!maskProxy) {
            // If we still don't have one, there's nothing we can do
            return nullptr;
        }

        // Register the mask for later invalidation
        Mask& mask = masks->emplace_back(genID, bounds);
        proxyProvider->assignUniqueKeyToProxy(mask.key(), maskProxy.asTextureProxy());
    }

    // Wrap the mask in an FP that samples it for coverage
    SkASSERT(maskProxy && maskProxy.origin() == kMaskOrigin);

    GrSamplerState samplerState(GrSamplerState::WrapMode::kClampToBorder,
                                GrSamplerState::Filter::kNearest);
    // FIXME not ocrrect, this should translate based on the masks bounds top-left, and the
    // domain/subset should be the query bounds adjusted to match the masks coord space.
    auto m = SkMatrix::Translate(-bounds.fLeft, -bounds.fTop);
    auto subset = SkRect::Make(bounds.size());
    // We scissor to bounds. The mask's texel centers are aligned to device space
    // pixel centers. Hence this domain of texture coordinates.
    auto domain = subset.makeInset(0.5, 0.5);
    auto fp = GrTextureEffect::MakeSubset(std::move(maskProxy), kPremul_SkAlphaType, m,
                                          samplerState, subset, domain, *context->priv().caps());
    return GrDeviceSpaceEffect::Make(std::move(fp));
}

GrSurfaceProxyView GrClipStack::RenderSWMask(GrRecordingContext* context,
                                             const SkIRect& bounds,
                                             const RawElement** elements,
                                             int count) {
    SkASSERT(count > 0);

    SkTaskGroup* taskGroup = nullptr;
    if (auto direct = context->asDirectContext()) {
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
            const RawElement& e = *(elements[i]);
            uploader->data().push_back(e.asElement());
        }

        GrTDeferredProxyUploader<SkTArray<Element>>* uploaderRaw = uploader.get();
        auto drawAndUploadMask = [uploaderRaw, bounds] {
            TRACE_EVENT0("skia.gpu", "Threaded SW Clip Mask Render");
            GrSWMaskHelper helper(uploaderRaw->getPixels());
            if (helper.init(bounds)) {
                for (int i = 0; i < uploaderRaw->data().count(); ++i) {
                    draw_to_sw_mask(&helper, uploaderRaw->data()[i], i == 0);
                }
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

        for (int i = 0; i < count; ++i) {
            const RawElement& e = *(elements[i]);
            draw_to_sw_mask(&helper, e.asElement(), i == 0);
        }

        return helper.toTextureView(context, SkBackingFit::kApprox);
    }
}
