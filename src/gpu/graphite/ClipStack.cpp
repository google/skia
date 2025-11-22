/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/ClipStack.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/ClipAtlasManager.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/EdgeAAQuad.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"

#include <algorithm>
#include <atomic>
#include <utility>

namespace skgpu::graphite {

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

static constexpr uint32_t kInvalidGenID  = 0;
static constexpr uint32_t kEmptyGenID    = 1;
static constexpr uint32_t kWideOpenGenID = 2;

uint32_t next_gen_id() {
    // 0-2 are reserved for invalid, empty & wide-open
    static const uint32_t kFirstUnreservedGenID = 3;
    static std::atomic<uint32_t> nextID{kFirstUnreservedGenID};

    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id < kFirstUnreservedGenID);
    return id;
}

bool oriented_bbox_intersection(const Rect& a, const Transform& aXform,
                                const Rect& b, const Transform& bXform) {
    // NOTE: We intentionally exclude projected bounds for two reasons:
    //   1. We can skip the division by w and worring about clipping to w = 0.
    //   2. W/o the projective case, the separating axes are simpler to compute (see below).
    SkASSERT(aXform.type() != Transform::Type::kPerspective &&
             bXform.type() != Transform::Type::kPerspective);
    SkV4 quadA[4], quadB[4];

    aXform.mapPoints(a, quadA);
    bXform.mapPoints(b, quadB);

    // There are 4 separating axes, defined by the two normals from quadA and from quadB, but
    // since they were produced by transforming a rectangle by an affine transform, we know the
    // normals are orthoganal to the basis vectors of upper 2x2 of their two transforms.
    auto axesX = skvx::float4(-aXform.matrix().rc(1,0), -aXform.matrix().rc(1,1),
                              -bXform.matrix().rc(1,0), -bXform.matrix().rc(1,1));
    auto axesY = skvx::float4(aXform.matrix().rc(0,0), aXform.matrix().rc(0,1),
                              bXform.matrix().rc(0,0), bXform.matrix().rc(0,1));

    // Projections of the 4 corners of each quadrilateral vs. the 4 axes. For orthonormal
    // transforms, the projections of a quad's corners to its own normal axes should work out
    // to the original dimensions of the rectangle, but this code handles skew and scale factors
    // without branching.
    auto aProj0 = quadA[0].x * axesX + quadA[0].y * axesY;
    auto aProj1 = quadA[1].x * axesX + quadA[1].y * axesY;
    auto aProj2 = quadA[2].x * axesX + quadA[2].y * axesY;
    auto aProj3 = quadA[3].x * axesX + quadA[3].y * axesY;

    auto bProj0 = quadB[0].x * axesX + quadB[0].y * axesY;
    auto bProj1 = quadB[1].x * axesX + quadB[1].y * axesY;
    auto bProj2 = quadB[2].x * axesX + quadB[2].y * axesY;
    auto bProj3 = quadB[3].x * axesX + quadB[3].y * axesY;

    // Minimum and maximum projected values against the 4 axes, for both quadA and quadB, which
    // gives us four pairs of intervals to test for separation.
    auto minA = min(min(aProj0, aProj1), min(aProj2, aProj3));
    auto maxA = max(max(aProj0, aProj1), max(aProj2, aProj3));
    auto minB = min(min(bProj0, bProj1), min(bProj2, bProj3));
    auto maxB = max(max(bProj0, bProj1), max(bProj2, bProj3));

    auto overlaps = (minB <= maxA) & (minA <= maxB);
    return all(overlaps); // any non-overlapping interval would imply no intersection
}

// LTRB are set in returned bitmask if other's LTRB edge is coincident or inside `shape`'s edge.
SkEnumBitMask<EdgeAAQuad::Flags> clipped_edges(const Rect& shape, const Rect& other) {
    // Since RB are stored negated in vals(), this works out to
    //     [other.LT >= shape.LT, other.RB <= shape.RB]
    auto insideMask = other.vals() >= shape.vals();
    return (insideMask[0] ? EdgeAAQuad::Flags::kLeft   : EdgeAAQuad::Flags::kNone) |
           (insideMask[1] ? EdgeAAQuad::Flags::kTop    : EdgeAAQuad::Flags::kNone) |
           (insideMask[2] ? EdgeAAQuad::Flags::kRight  : EdgeAAQuad::Flags::kNone) |
           (insideMask[3] ? EdgeAAQuad::Flags::kBottom : EdgeAAQuad::Flags::kNone);
}

// Tries to intersect `otherShape` transformed by `otherToDevice` directly into `shape` assuming
// that `shape` is transformed by localToDevice. If possible (true), `shape` represents the exact
// intersection of the two original shapes. Returns true if `shape` is modified, false otherwise.
bool intersect_shape(const Transform& otherToDevice, const Shape& otherShape,
                     const Transform& localToDevice, Shape* shape,
                     SkEnumBitMask<EdgeAAQuad::Flags>* edgeFlags) {
    // There are only a subset of shape types that we can analytically intersect with each other,
    // assuming a simple fill style (always the case for clip shapes):
    //
    //  rects, rrects, flood-fills (empty+inverse-fill)
    //
    // Flood-fills only appear as part of a draw, so it's only checked for `shape` and not
    // `otherShape`. In theory, per-edge AA quads could also be included but they do not appear as
    // clip shapes.
    //
    // Paths and arcs have complex intersection logic, so are skipped under the assumption that
    // simple cases have already been mapped to a rect or rrect. Lines are only ever stroked, so
    // are incompatible with this function.
    //
    // EdgeAAQuads that are rectangular can be intersected by being treated as a rect shape and
    // adjusting edge flags as non-AA edges are clipped out.
    bool shapeIntersectable = shape->isRect() ||
                              shape->isRRect() ||
                              shape->isFloodFill();
    bool otherIntersectable = otherShape.isRect() || otherShape.isRRect();
    // Only clip shapes are used for `otherShape`, so we shouldn't see any flood fills here
    SkASSERT(!otherShape.isFloodFill());
    // Only rects should have edge flags other than kAll
    SkASSERT(*edgeFlags == EdgeAAQuad::Flags::kAll || shape->isRect());

    if (!shapeIntersectable || !otherIntersectable) {
        // Technically if shapeIntersectable was true for empty+inverse, we could turn the flood
        // fill into `otherShape` regardless of its type, but those other types are more expensive
        // to render and in the situation where many draws fill against a clip path, we'd want to
        // draw the clip a single time vs. drawing the path multiple times.
        return false;
    }

    // In order to combine, otherShape must be able to map into `localToDevice` without changing
    // shape class (e.g. to a path when rotated) in order for shading to apply in the same
    // coordinate space. This is possible if the relative transform between otherToDevice and
    // localToDevice is rectStaysRect.
    Transform storage{SkM44::kUninitialized_Constructor};

    // We track `local` to `other` and use the `inverseMapRect` functions to map the `otherShape`
    // into local space when possible. Using `localToOther` instead of `otherToLocal` allows the
    // common case of a device-space clip (otherToDevice == I) and an axis-aligned draw to
    // simply use `localToDevice` as `localToOther`.
    const Transform* localToOther;

    if (otherToDevice == localToDevice) {
        // No coordinate space conversion, so set to null to signal identity mapping is skippable.
        // NOTE: This case arises in clip-clip combinations when both were axis-aligned and pre-
        // transformed to device space.
        localToOther = nullptr;
    } else if (otherToDevice.type() == Transform::Type::kIdentity &&
               localToDevice.type() <= Transform::Type::kRectStaysRect) {
        // Relative transform is (otherToDevice)^-1*localToDevice = localToDevice
        localToOther = &localToDevice;
    } else if (otherToDevice.type() <= Transform::Type::kRectStaysRect &&
               localToDevice.type() == Transform::Type::kIdentity) {
        // Relative transform is otherToDevice^-1*localToDevice = otherToDevice^-1
        // (which may not occur in a common scenario but is harmless). Inverse() is mostly
        // shuffling bytes around, not recomputing the inverse.
        storage = Transform::Inverse(otherToDevice);
        localToOther = &storage;
    } else {
        // Calculate (otherToDevice)^-1*localToDevice and see if the relative transform is
        // of the right type.
        storage = Transform(otherToDevice.inverse() * localToDevice);
        if (storage.type() <= Transform::Type::kRectStaysRect) {
            localToOther = &storage;
        } else {
            // `otherShape` can't be trivially mapped to the local coordinate space
            return false;
        }
    }

    // Since `otherShape` is either a rect or a round rect, bounds() is tight to the linear edges.
    Rect localOtherRect = otherShape.bounds();
    if (localToOther) {
        // In this block, `localOtherRect` is defined in the other coord space and `mapped` is in
        // the local coord space. At the end of the block, `localOtherRect` is set to `mapped` so
        // that afterwards it is always defined in local space.
        Rect mapped = localToOther->inverseMapRect(localOtherRect);
        // If we don't have enough precision, the other shape might not map back to the geometry.
        // Allow up to 1/1000th of a pixel in tolerance when mapping between coordinate spaces,
        // otherwise we'll have to clip the shapes independently.
        const float otherTol =
                Shape::kDefaultPixelTolerance * otherToDevice.localAARadius(localOtherRect);
        if (localOtherRect.isEmptyNegativeOrNaN() ||
            !localToOther->mapRect(mapped).nearlyEquals(localOtherRect, otherTol)) {
            return false;
        }
        localOtherRect = mapped;
    }
    // Remember the edges that get clipped by the intersection
    SkEnumBitMask<EdgeAAQuad::Flags> clippedEdges = clipped_edges(shape->bounds(), localOtherRect);
    if (!shape->isFloodFill()) {
        // And now it's tight to the intersection with `shape`, sans any corner rounding
        localOtherRect.intersect(shape->bounds());
    }
    // Make sure that the intersected shape does not become subpixel in size, since drawing a
    // subpixel/hairline shape produces a different result than something that's clipped.
    float localAARadius = localToDevice.localAARadius(localOtherRect);
    if (!std::isfinite(localAARadius) || any(localOtherRect.size() <= localAARadius)) {
        return false;
    }

    SkRRect localOtherRRect;
    if (otherShape.isRect()) {
        if (shape->isRect() || shape->isFloodFill()) {
            SkASSERT(*edgeFlags == EdgeAAQuad::Flags::kAll || !shape->isFloodFill());
            // Assuming that non-AA edges seam with non-AA edges other quads to create a uniform
            // coverage field, we turn on the AA edge flag when coincident or clipped. This will
            // create a nice AA edge from this draw while the other non-AA quad is discarded.
            *edgeFlags |= clippedEdges; // This is a no-op if shape was a flood fill
            shape->setRect(localOtherRect);
            return true;
        } else {
            // Fall back to rrect+rrect intersection
            localOtherRRect = SkRRect::MakeRect(localOtherRect.asSkRect());
        }
    } else {
        SkASSERT(otherShape.isRRect());
        if (localToOther) {
            if (auto rr = otherShape.rrect().transform(localToOther->inverse().asM33())) {
                localOtherRRect = *rr;
            } else {
                // Transformation produced invalid geometry
                return false;
            }
        } else {
            localOtherRRect = otherShape.rrect();
        }

        if (shape->isRect() && *edgeFlags != EdgeAAQuad::Flags::kAll) {
            // When combining a mixed edge AA quad with a rounded rectangle, we require that all
            // non-AA edges be clipped out entirely.
            if ((clippedEdges | *edgeFlags) != EdgeAAQuad::Flags::kAll) {
                // The intersection shows AA'ed round corners and non-AA'ed edges, which can't be
                // represented by just Geometry or Shape.
                return false;
            }
        } else if (shape->isFloodFill()) {
            SkASSERT(*edgeFlags == EdgeAAQuad::Flags::kAll);
            shape->setRRect(localOtherRRect);
            return true;
        } // Else continue with rrect+rrect intersection
    }

    // `shape` can only be rect or rrect at this point, flood fill should already have returned.
    // If we've made it this far, we've also determined that the edge flags should be set to kAll
    // on a successful rrect+rrect intersection.
    SkASSERT(shape->isRect() || shape->isRRect());

    SkRRect localRRect = SkRRectPriv::ConservativeIntersect(
            localOtherRRect,
            shape->isRect() ? SkRRect::MakeRect(shape->rect().asSkRect()) : shape->rrect());
    if (localRRect.isRect()) {
        // Valid shape that can be simplified to rect
        shape->setRect(localRRect.rect());
        *edgeFlags = EdgeAAQuad::Flags::kAll;
        return true;
    } else if (!localRRect.isEmpty()) {
        // Intersection is representable as a rrect still
        shape->setRRect(localRRect);
        *edgeFlags = EdgeAAQuad::Flags::kAll;
        return true;
    } else {
        // Intersection is complex, leave edge flags unmodified
        return false;
    }
}

Rect snap_scissor(const Rect& a, const Rect& deviceBounds) {
    // Snapping to 4 pixel boundaries seems to give a good tradeoff between rasterizing slightly
    // more (but being clipped by the depth test), vs. setting a tight scissor that forces a state
    // change.
    // NOTE: This rounds out to the *next* multiple of 4, so that if the input rectangle happens to
    // land on a multiple of 4 we still create some padding to avoid scissoring just AA outsets.
    static constexpr int kRes = 4;
    Rect snapped = a.makeOutset(kRes - 1.f);
    snapped = Rect::FromVals(snapped.vals() * (1.f / kRes)).makeRoundOut();
    return Rect::FromVals(snapped.vals() * kRes).makeIntersect(deviceBounds);
}

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

    if (fLocalToDevice.type() <= Transform::Type::kRectStaysRect &&
        o.fLocalToDevice.type() <= Transform::Type::kRectStaysRect) {
        // The two shape's coordinate spaces are different but both rect-stays-rect or simpler.
        // This means, though, that their outer bounds approximations are tight to their transormed
        // shape bounds. There's no point to do further tests given that and that we already found
        // that these outer bounds *do* intersect.
        return true;
    } else if (fLocalToDevice == o.fLocalToDevice) {
        // Since the two shape's local coordinate spaces are the same, we can compare shape
        // bounds directly for a more accurate intersection test. We intentionally do not go
        // further and do shape-specific intersection tests since these could have unknown
        // complexity (for paths) and limited utility (e.g. two round rects that are disjoint
        // solely from their corner curves).
        return fShape.bounds().intersects(o.fShape.bounds());
    } else if (fLocalToDevice.type() != Transform::Type::kPerspective &&
               o.fLocalToDevice.type() != Transform::Type::kPerspective) {
        // The shapes don't share the same coordinate system, and their approximate 'outer'
        // bounds in device space could have substantial outsetting to contain the transformed
        // shape (e.g. 45 degree rotation). Perform a more detailed check on their oriented
        // bounding boxes.
        return oriented_bbox_intersection(fShape.bounds(), fLocalToDevice,
                                          o.fShape.bounds(), o.fLocalToDevice);
    }
    // Else multiple perspective transforms are involved, so assume intersection and allow the
    // rasterizer to handle perspective clipping.
    return true;
}

bool ClipStack::TransformedShape::contains(const TransformedShape& o) const {
    if (fInnerBounds.contains(o.fOuterBounds)) {
        return true;
    }
    // Skip more expensive contains() checks if configured not to, or if the extent of 'o' exceeds
    // this shape's outer bounds. When that happens there must be some part of 'o' that cannot be
    // contained in this shape.
    if (fContainsChecksOnlyBounds || !fOuterBounds.contains(o.fOuterBounds)) {
        return false;
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
            if (!fShape.conservativeContains(skvx::float2::Load(localQuad + i) / localQuad[i].w)) {
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
    SkUNREACHABLE;
}

ClipStack::DrawInfluence ClipStack::SimplifyForDraw(const TransformedShape& clip,
                                                    const TransformedShape& draw) {
    // Given the asserts below, we can just recast the SimplifyResult returned from
    // Simplify(A=clip, B=draw):
    //
    // If the result is kEmpty, the draw is clipped out.
    static_assert((int) SimplifyResult::kEmpty == (int) DrawInfluence::kClipsOutDraw);
    // If the result is kAOnly, only the clip's shape provides coverage and the draw could be
    // replaced with something that just covers the clip bounds.
    static_assert((int) SimplifyResult::kAOnly == (int) DrawInfluence::kReplacesDraw);
    // If the result is kBOnly, the clip's shape doesn't impact the draw's coverage at all.
    static_assert((int) SimplifyResult::kBOnly == (int) DrawInfluence::kNone);
    // If the result is kBoth, the clip and the draw combine in a complex manner
    static_assert((int) SimplifyResult::kBoth == (int) DrawInfluence::kComplexInteraction);

    SimplifyResult result = Simplify(clip, draw);
    return static_cast<DrawInfluence>(result);
}

///////////////////////////////////////////////////////////////////////////////
// ClipStack::Element

ClipStack::RawElement::RawElement(const Rect& deviceBounds,
                                  const Transform& localToDevice,
                                  const Shape& shape,
                                  SkClipOp op,
                                  PixelSnapping snapping)
        : Element{shape, localToDevice, op}
        , fUsageBounds{Rect::InfiniteInverted()}
        , fOrder(DrawOrder::kNoIntersection)
        , fMaxZ(DrawOrder::kClearDepth)
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
    }

    fOuterBounds = fLocalToDevice.mapRect(fShape.bounds()).makeIntersect(deviceBounds);
    fInnerBounds = Rect::InfiniteInverted();

    // Apply rect-stays-rect transforms to rects and round rects to reduce the number of unique
    // local coordinate systems that are in play.
    if (!fOuterBounds.isEmptyNegativeOrNaN() &&
        fLocalToDevice.type() <= Transform::Type::kRectStaysRect) {
        if (fShape.isRect()) {
            // The actual geometry can be updated to the device-intersected bounds and we know the
            // inner bounds are equal to the outer.
            if (snapping == PixelSnapping::kYes) {
                fOuterBounds.round();
            }
            fShape.setRect(fOuterBounds);
            fLocalToDevice = Transform::Identity();
            fInnerBounds = fOuterBounds;
        } else if (fShape.isRRect()) {
            // Can't transform in place and must still check transform result since some very
            // ill-formed scale+translate matrices can cause invalid rrect radii.
            if (auto xformed = fShape.rrect().transform(fLocalToDevice)) {
                if (snapping == PixelSnapping::kYes) {
                    // The rounded corners will still be anti-aliased, but snap the horizontal and
                    // vertical edges to pixel values.
                    xformed->setRectRadii(SkRect::Make(xformed->rect().round()),
                                          xformed->radii().data());
                }
                fShape.setRRect(*xformed);
                fLocalToDevice = Transform::Identity();
                // Refresh outer bounds to match the transformed round rect in case
                // SkRRect::transform produces slightly different results from Transform::mapRect.
                fOuterBounds = fShape.bounds().makeIntersect(deviceBounds);
                fInnerBounds = Rect{SkRRectPriv::InnerBounds(*xformed)}.makeIntersect(fOuterBounds);
            }
        }
    }

    if (fOuterBounds.isEmptyNegativeOrNaN()) {
        // Either was already an empty shape or a non-empty shape is offscreen, so treat it as such.
        fShape.reset();
        fInnerBounds = Rect::InfiniteInverted();
    }

    // Now that fOp and fShape are canonical, set the shape's fill type to match how it needs to be
    // drawn as a depth-only shape everywhere that is clipped out (intersect is thus inverse-filled)
    fShape.setInverted(fOp == SkClipOp::kIntersect);

    // Post-conditions on inner and outer bounds
    SkASSERT(fShape.isEmpty() || deviceBounds.contains(fOuterBounds));
    this->validate();
}

ClipStack::RawElement::operator ClipStack::TransformedShape() const {
    return {fLocalToDevice, fShape, fOuterBounds, fInnerBounds, fOp};
}

void ClipStack::RawElement::drawClip(Device* device) {
    this->validate();

    // Skip elements that have not affected any draws
    if (!this->hasPendingDraw()) {
        SkASSERT(fUsageBounds.isEmptyNegativeOrNaN());
        return;
    }

    SkASSERT(!fUsageBounds.isEmptyNegativeOrNaN());
    // For clip draws, the usage bounds is the scissor.
    const Rect deviceBounds = Rect::WH(device->width(), device->height());
    Rect scissor = fUsageBounds; // all joined usage bounds are pre-snapped

    // snappedOuterBounds was the rectangle used in updateForDraw() to query the Z order the clip's
    // draw will be inserted at. The scissor must enforce that rendering doesn't happen outside of
    // those bounds.
    Rect snappedOuterBounds = snap_scissor(fOuterBounds, deviceBounds);
    scissor.intersect(snappedOuterBounds);
    // But if the overlap is sufficiently large, just rasterize out to the snapped bounds instead of
    // adding a tight scissor. A factor of 1/2 is used because that corresponds to the area
    // change caused by a 45-degree rotation.
    if (0.5f * snappedOuterBounds.area() < scissor.area()) {
        scissor = snappedOuterBounds;
    }

    Rect drawBounds = fOp == SkClipOp::kIntersect ? scissor : fOuterBounds.makeIntersect(scissor);
    if (!drawBounds.isEmptyNegativeOrNaN()) {
        // Although we are recording this clip draw after all the draws it affects, 'fOrder' was
        // determined at the first usage, so after sorting by DrawOrder the clip draw will be in the
        // right place. Unlike regular draws that use their own "Z", by writing (1 + max Z this clip
        // affects), it will cause those draws to fail either GREATER and GEQUAL depth tests where
        // they need to be clipped.
        DrawOrder order{fMaxZ.next(), fOrder};
        // An element's clip op is encoded in the shape's fill type. Inverse fills are intersect ops
        // and regular fills are difference ops. This means fShape is already in the right state to
        // draw directly.
        SkASSERT((fOp == SkClipOp::kDifference && !fShape.inverted()) ||
                 (fOp == SkClipOp::kIntersect && fShape.inverted()));

        // NOTE: We use fOuterBounds as the transformed shape bounds as that hasn't been clipped by
        // the scissor. It has been clipped by the device bounds, but that shouldn't impact any
        // decisions at this point. If that becomes not the case, we can either recompute the
        // shape's device-space bounds (fLocalToDevice.mapRect(fShape.bounds())) or store a fully
        // unclipped shape bounds on the RawElement.
        device->drawClipShape(fLocalToDevice,
                              fShape,
                              Clip{drawBounds, fOuterBounds, scissor.asSkIRect(),
                                   /* nonMSAAClip= */ {}, /* shader= */ nullptr},
                              order);
    }

    // After the clip shape is drawn, reset its state. If the clip element is being popped off the
    // stack or overwritten because a new clip invalidated it, this won't matter. But if the clips
    // were drawn because the Device had to flush pending work while the clip stack was not empty,
    // subsequent draws will still need to be clipped to the elements. In this case, the usage
    // accumulation process will begin again and automatically use the Device's post-flush Z values
    // and BoundsManager state.
    fUsageBounds = Rect::InfiniteInverted();
    fOrder = DrawOrder::kNoIntersection;
    fMaxZ = DrawOrder::kClearDepth;
}

void ClipStack::RawElement::validate() const {
    // If the shape type isn't empty, the outer bounds shouldn't be empty; if the inner bounds are
    // not empty, they must be contained in outer.
    SkASSERT((fShape.isEmpty() || !fOuterBounds.isEmptyNegativeOrNaN()) &&
             (fInnerBounds.isEmptyNegativeOrNaN() || fOuterBounds.contains(fInnerBounds)));
    SkASSERT((fOp == SkClipOp::kDifference && !fShape.inverted()) ||
             (fOp == SkClipOp::kIntersect && fShape.inverted()));
    SkASSERT(!this->hasPendingDraw() || !fUsageBounds.isEmptyNegativeOrNaN());
}

void ClipStack::RawElement::markInvalid(const SaveRecord& current) {
    SkASSERT(!this->isInvalid());
    fInvalidatedByIndex = current.firstActiveElementIndex();
    // NOTE: We don't draw the accumulated clip usage when the element is marked invalid. Some
    // invalidated elements are part of earlier save records so can become re-active after a restore
    // in which case they should continue to accumulate. Invalidated elements that are part of the
    // active save record are removed at the end of the stack modification, which is when they are
    // explicitly drawn.
}

void ClipStack::RawElement::restoreValid(const SaveRecord& current) {
    if (current.firstActiveElementIndex() < fInvalidatedByIndex) {
        fInvalidatedByIndex = -1;
    }
}

bool ClipStack::RawElement::combine(const RawElement& other, const SaveRecord& current) {
    // Don't combine elements that have collected draw usage, since that changes their geometry.
    if (this->hasPendingDraw() || other.hasPendingDraw()) {
        return false;
    }
    // To reduce the number of possibilities, only consider intersect+intersect. Difference and
    // mixed op cases could be analyzed to simplify one of the shapes, but that is a rare
    // occurrence and the math is much more complicated.
    if (other.fOp != SkClipOp::kIntersect || fOp != SkClipOp::kIntersect) {
        return false;
    }

    // NOTE: intersect_shape operates on the underlying geometry and ignores the fill rule, which
    // because these are intersect clip ops, is the inverse fill. If the shape is updated, the
    // resulting geometry is set to a regular fill so it must be re-inverted to represent the
    // pixels rasterized for a depth-only clip draw.
    SkEnumBitMask<EdgeAAQuad::Flags> edgeFlags = EdgeAAQuad::Flags::kAll;
    const bool shapeUpdated = intersect_shape(other.fLocalToDevice, other.fShape,
                                              fLocalToDevice, &fShape, &edgeFlags);
    SkASSERT(edgeFlags == EdgeAAQuad::Flags::kAll);

    if (shapeUpdated) {
        // This logic works under the assumption that both combined elements were intersect.
        SkASSERT(fOp == SkClipOp::kIntersect && other.fOp == SkClipOp::kIntersect);
        fOuterBounds.intersect(other.fOuterBounds);
        fInnerBounds.intersect(other.fInnerBounds);
        // Inner bounds can become empty, but outer bounds should not be able to.
        SkASSERT(!fOuterBounds.isEmptyNegativeOrNaN());
        fShape.setInverted(true); // Undo intersect_shape setting it to non-inverse
        this->validate();
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

ClipStack::DrawInfluence ClipStack::RawElement::testForDraw(const TransformedShape& draw) const {
    if (this->isInvalid()) {
        // Cannot affect the draw
        return DrawInfluence::kNone;
    }

    return SimplifyForDraw(*this, draw);
}

CompressedPaintersOrder ClipStack::RawElement::updateForDraw(const BoundsManager* boundsManager,
                                                             const Rect& deviceBounds,
                                                             const Rect& drawBounds,
                                                             PaintersDepth drawZ) {
    SkASSERT(!this->isInvalid());
    SkASSERT(!drawBounds.isEmptyNegativeOrNaN());

    // Always record snapped draw bounds to avoid scissor thrashing since these bounds will be used
    // to determine the scissor applied to the depth-only draw for the clip element.
    Rect snappedDrawBounds = snap_scissor(drawBounds, deviceBounds);

    if (!this->hasPendingDraw()) {
        // No usage yet so we need an order that we will use when drawing to just the depth
        // attachment. It is sufficient to use the next CompressedPaintersOrder after the
        // most recent draw under this clip's outer bounds. It is necessary to use the
        // entire clip's outer bounds because the order has to be determined before the
        // final usage bounds are known and a subsequent draw could require a completely
        // different portion of the clip than this triggering draw.
        //
        // Lazily determining the order has several benefits to computing it when the clip
        // element was first created:
        //  - Elements that are invalidated by nested clips before draws are made do not
        //    waste time in the BoundsManager.
        //  - Elements that never actually modify a draw (e.g. a defensive clip) do not
        //    waste time in the BoundsManager.
        //  - A draw that triggers clip usage on multiple elements will more likely assign
        //    the same order to those elements, meaning their depth-only draws are more
        //    likely to batch in the final DrawPass.
        //
        // However, it does mean that clip elements can have the same order as each other,
        // or as later draws (e.g. after the clip has been popped off the stack). Any
        // overlap between clips or draws is addressed when the clip is drawn by selecting
        // an appropriate DisjointStencilIndex value. Stencil-aside, this order assignment
        // logic, max Z tracking, and the depth test during rasterization are able to
        // resolve everything correctly even if clips have the same order value.
        // See go/clip-stack-order for a detailed analysis of why this works.
        Rect snappedOuterBounds = snap_scissor(fOuterBounds, deviceBounds);
        fOrder = boundsManager->getMostRecentDraw(snappedOuterBounds).next();
        fUsageBounds = snappedDrawBounds;
        fMaxZ = drawZ;
    } else {
        // Earlier draws have already used this element so we cannot change where the
        // depth-only draw will be sorted to, but we need to ensure we cover the new draw's
        // bounds and use a Z value that will clip out its pixels as appropriate.
        fUsageBounds.join(snappedDrawBounds);
        if (drawZ > fMaxZ) {
            fMaxZ = drawZ;
        }
    }

    return fOrder;
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

        case Shape::Type::kArc:
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
        , fState(ClipState::kWideOpen)
        , fGenID(kInvalidGenID) {}

ClipStack::SaveRecord::SaveRecord(const SaveRecord& prior,
                                  int startingElementIndex)
        : fInnerBounds(prior.fInnerBounds)
        , fOuterBounds(prior.fOuterBounds)
        , fShader(prior.fShader)
        , fStartingElementIndex(startingElementIndex)
        , fOldestValidIndex(prior.fOldestValidIndex)
        , fDeferredSaveCount(0)
        , fStackOp(prior.fStackOp)
        , fState(prior.fState)
        , fGenID(kInvalidGenID) {
    // If the prior record added an element, this one will insert into the same index
    // (that's okay since we'll remove it when this record is popped off the stack).
    SkASSERT(startingElementIndex >= prior.fStartingElementIndex);
}

uint32_t ClipStack::SaveRecord::genID() const {
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

ClipStack::ClipState ClipStack::SaveRecord::state() const {
    if (fShader && fState != ClipState::kEmpty) {
        return ClipState::kComplex;
    } else {
        return fState;
    }
}

ClipStack::DrawInfluence ClipStack::SaveRecord::testForDraw(const TransformedShape& draw) const {
    Transform identity = Transform::Identity();
    Shape outerSaveBounds{fOuterBounds};
    TransformedShape save{identity, outerSaveBounds, fOuterBounds, fInnerBounds, fStackOp,
                          /*containsChecksOnlyBounds=*/true};

    return SimplifyForDraw(save, draw);
}

void ClipStack::SaveRecord::removeElements(RawElement::Stack* elements, Device* device) {
    while (elements->count() > fStartingElementIndex) {
        // Since the element is being deleted now, it won't be in the ClipStack when the Device
        // calls recordDeferredClipDraws(). Record the clip's draw now (if it needs it).
        elements->back().drawClip(device);
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

bool ClipStack::SaveRecord::addElement(RawElement&& toAdd,
                                       RawElement::Stack* elements,
                                       Device* device) {
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
        this->removeElements(elements, device);
        return true;
    }

    // Here we treat the SaveRecord as a "TransformedShape" with the identity transform, and a shape
    // equal to its outer bounds. This lets us get accurate intersection tests against the new
    // element, but we pass true to skip more detailed contains checks because the SaveRecord's
    // shape is potentially very different from its aggregate outer bounds.
    Shape outerSaveBounds{fOuterBounds};
    Transform identity = Transform::Identity();
    TransformedShape save{identity, outerSaveBounds, fOuterBounds, fInnerBounds, fStackOp,
                          /*containsChecksOnlyBounds=*/true};

    // In this invocation, 'A' refers to the existing stack's bounds and 'B' refers to the new
    // element.
    switch (Simplify(save, toAdd)) {
        case SimplifyResult::kEmpty:
            // The combination results in an empty clip
            fState = ClipState::kEmpty;
            this->removeElements(elements, device);
            return true;

        case SimplifyResult::kAOnly:
            // The combination would not be any different than the existing clip
            return false;

        case SimplifyResult::kBOnly:
            // The combination would invalidate the entire existing stack and can be replaced with
            // just the new element.
            this->replaceWithElement(std::move(toAdd), elements, device);
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
        this->replaceWithElement(std::move(toAdd), elements, device);
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

    return this->appendElement(std::move(toAdd), elements, device);
}

bool ClipStack::SaveRecord::appendElement(RawElement&& toAdd,
                                          RawElement::Stack* elements,
                                          Device* device) {
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
        elements->back().drawClip(device);
        elements->pop_back();
    }
    if (oldestActiveInvalid) {
        oldestActiveInvalid->drawClip(device);
        *oldestActiveInvalid = std::move(toAdd);
    } else if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back().drawClip(device);
        elements->back() = std::move(toAdd);
    }

    // Changing this will prompt ClipStack to invalidate any masks associated with this record.
    fGenID = next_gen_id();
    return true;
}

void ClipStack::SaveRecord::replaceWithElement(RawElement&& toAdd,
                                               RawElement::Stack* elements,
                                               Device* device) {
    // The aggregate state of the save record mirrors the element
    fInnerBounds = toAdd.innerBounds();
    fOuterBounds = toAdd.outerBounds();
    fStackOp = toAdd.op();
    fState = toAdd.clipType();

    // All prior active element can be removed from the stack: [startingIndex, count - 1]
    int targetCount = fStartingElementIndex + 1;
    while (elements->count() > targetCount) {
        elements->back().drawClip(device);
        elements->pop_back();
    }
    if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back().drawClip(device);
        elements->back() = std::move(toAdd);
    }

    SkASSERT(elements->count() == fStartingElementIndex + 1);

    // This invalidates all older elements that are owned by save records lower in the clip stack.
    fOldestValidIndex = fStartingElementIndex;
    fGenID = next_gen_id();
}

///////////////////////////////////////////////////////////////////////////////
// ClipStack::DrawShape

/**
 * DrawShape represents the approximate shape that is being drawn in order to compare it against
 * the clip stack's RawElements. It is able to map to a `TransformedShape` to be simplified with
 * either the SaveRecord or each element. For non-Shape geometries and stroked shapes, it
 * represents the oriented bounding box. For filled Shapes, it preserves the original shape for
 * more accurate contains/intersect checks and geometrically combining RawElements into the
 * shape.
 */
class ClipStack::DrawShape {
public:
    DrawShape(const Transform& localToDevice, const Geometry& geometry)
            : fLocalToDevice(&localToDevice)
            , fEdgeFlags(EdgeAAQuad::Flags::kAll)
            , fScissor(Rect::Infinite())
            , fShapeWasModified(false) {
        if (geometry.isShape()) {
            fShape = geometry.shape();
            fShapeMatchesGeometry = true;
        } else {
            // The geometry is something special like text or vertices, in which case it's
            // definitely not a shape that could simplify cleanly with the clip stack, so just track
            // its bounds. The exception is EdgeAA quads that are rectangular, in which case we can
            // clip its edges and adjust edge flags.
            fShape.setRect(geometry.bounds());
            if (geometry.isEdgeAAQuad()) {
                fEdgeFlags = geometry.edgeAAQuad().edgeFlags();
                fShapeMatchesGeometry = geometry.edgeAAQuad().isRect();
            } else {
                fShapeMatchesGeometry = false;
            }
            // If geometry is not a shape, it is not inverted.
            SkASSERT(!fShape.inverted());
        }

        fShapeCompatibleWithIntersectShape = fShape.isFloodFill() ||
                                            (!fShape.inverted() && (fShape.isRect() ||
                                                                    fShape.isRRect()));
    }

    operator TransformedShape() const {
        // A regular draw is a transformed shape that "intersects" the clip. An inverse-filled draw
        // is equivalent to "difference". For simple convex shapes we provide an inner bounds
        // because we can geometrically intersect clip elements with the draw geometry and not
        // really impact the choice of Renderer (given the family of renderers used for simple
        // shapes). In theory any convex shape could provide an inner bounds and/or use the detailed
        // contains check, but that would cause path rendering draws to potentially change in hard
        // to predict ways.
        SkClipOp op = fShape.inverted() ? SkClipOp::kDifference : SkClipOp::kIntersect;
        return TransformedShape{*fLocalToDevice, fShape, fOuterBounds, fInnerBounds, op,
                                /*containsChecksOnlyBounds=*/!this->shapeCanBeModified()};
    }

    bool shapeCanBeModified() const {
        return fShapeCompatibleWithIntersectShape && fShapeMatchesGeometry;
    }

    bool applyStyle(const SkStrokeRec& style, const Rect& deviceBounds);
    void applyScissor(const Rect& scissor);

    void resetToFloodFill();
    bool intersectClipElement(const RawElement& clip);

    // Sync any modifications back to `geometry` and return a Clip object encapsulating the
    // tracked bounds of the now-clipped draw.
    Clip toClip(Geometry* geometry, const NonMSAAClip& analyticClip, const SkShader* clipShader);

private:
    const Transform* fLocalToDevice;

    // When 'style' isn't fill, the original geometry describes the pre-stroked shape, so 'fShape'
    // is updated to include the bounds post-stroking. `fShape` may also include local AA outsets
    // under certain circumstances:
    //  1. If it's a hairline, the AA outset can be added in local space to preserve a tighter
    //     oriented bbox compared to device bounds outset by 1px.
    //  2. If it's subpixel, the rendered geometry is often treated as a hairline with an adjusted
    //     coverage ramp.
    // Notably, the local AA outset is not included in `styledShape` for other cases to maximize the
    // cases where a draw is contained in a clip, or can be clipped geometrically. This assumes that
    // rendering an AA'ed non-hairline/subpixel edge produces a 1px feathered edge that's not
    // qualitatively different from the 1px feathered edge a clip would enforce.
    Shape fShape;
    SkEnumBitMask<EdgeAAQuad::Flags> fEdgeFlags;

    // Not valid until after applyStyle() is called, although applyScissor() can shrink the inner
    // and outer bounds.
    Rect fTransformedShapeBounds;
    Rect fOuterBounds;
    Rect fInnerBounds;

    Rect fScissor;

    // Whether or not the shape matches the original geometry to draw (with style)
    bool fShapeMatchesGeometry;
    // Whether or not the clip stack can modify this shape in place (and if it has already done so).
    bool fShapeCompatibleWithIntersectShape;
    bool fShapeWasModified;
};

bool ClipStack::DrawShape::applyStyle(const SkStrokeRec& style, const Rect& deviceBounds) {
    // For overriding fLocalToDevice when the shape is only tracking device-space bounds
    static const Transform kIdentity = Transform::Identity();

    fTransformedShapeBounds = fShape.bounds(); // not scissor'ed, regular fill rule bounds
    auto origSize = fTransformedShapeBounds.size();
    if (!SkIsFinite(origSize.x(), origSize.y())) {
        // Discard all non-finite geometry as if it were clipped out
        return false;
    }

    // Discard fills and strokes that cannot produce any coverage: an empty fill, or a
    // zero-length stroke that has butt caps. Otherwise the stroke style applies to a vertical
    // or horizontal line (making it non-empty), or it's a zero-length path segment that
    // must produce round or square caps (making it non-empty):
    //     https://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes
    if (!fShape.inverted() && (fShape.isLine() || any(origSize == 0.f))) {
        if (style.isFillStyle() || (style.getCap() == SkPaint::kButt_Cap && all(origSize == 0.f))) {
            return false;
        }
    }

    // Anti-aliasing makes shapes larger than their original coordinates, but we only care about
    // that for local clip checks in certain cases (see above).
    // NOTE: After this if-else block, `transformedShapeBounds` will be in device space.
    float localAAOutset = fLocalToDevice->localAARadius(fTransformedShapeBounds);
    if (!SkIsFinite(localAAOutset)) SK_UNLIKELY {
        // We cannot calculate an accurate local shape bounds, and transformedShapeBounds is meant
        // to be unclipped. This is to maximize atlas reuse for mostly unclipped draws and to detect
        // when a scissor state change is required. Setting transformedShapeBounds to deviceBounds
        // is harmless in this case as these benefits are unlikely to apply for this transform.
        fTransformedShapeBounds = deviceBounds;
        fShape.setRect(deviceBounds);
        fLocalToDevice = &kIdentity;
        fShapeMatchesGeometry = false;
    } else {
        // SkStrokeRec::GetInflationRadius() returns a device-space inflation for hairlines.
        float localOutset = 0.f;
        if (!style.isFillStyle() && !style.isHairlineStyle()) {
            // Rectangles, rounded rectangles, and lines do not produce miters so don't count the
            // pessimistic limit against their draw bounds.
            const float effectiveMiterLimit = fShape.isPath() ? style.getMiter() : 1.f;
            // Rectangles and rounded rectangles don't have caps, so don't count that against their
            // draw bounds (if we could efficiently know a path was a closed contour, it could
            // be included here too).
            const SkPaint::Cap effectiveCap = fShape.isRect() || fShape.isRRect()
                    ? SkPaint::kButt_Cap : style.getCap();
            localOutset = SkStrokeRec::GetInflationRadius(style.getJoin(),
                                                          effectiveMiterLimit,
                                                          effectiveCap,
                                                          style.getWidth());
        }

        if (style.isHairlineStyle() ||
            (!style.isFillStyle() && style.getWidth() < localAAOutset) ||
            (style.isFillStyle() && !fShape.inverted() && any(origSize < localAAOutset))) {
            // The geometry is a hairline or projects to a subpixel shape, so rendering will not
            // follow the typical 1/2px outset anti-aliasing that is compatible with clipping.
            // In this case, apply the local AA radius to the shape to have a conservative clip
            // query while preserving the oriented bounding box.
            localOutset += localAAOutset;
        }

        if (localOutset > 0.f) {
            // Propagate style and AA outset into styledShape so clip queries reflect style.
            fTransformedShapeBounds.outset(localOutset);

            bool inverted = fShape.inverted();
            if (fShape.isRRect()) {
                // Try to preserve the rounded corners, which can reduce the chance of clipping
                // stroked rounded rects that are clipped to a round rect matching their outer edge.
                fShape.rrect().outset(localOutset, localOutset, &fShape.rrect());
            } else
            {
                fShape.setRect(fTransformedShapeBounds); // it's still local at this point
            }
            fShape.setInverted(inverted);  // preserve original inversion state
            fShapeMatchesGeometry = false;
        }

        fTransformedShapeBounds = fLocalToDevice->mapRect(fTransformedShapeBounds);
    }

    fOuterBounds = fTransformedShapeBounds;
    fInnerBounds = Rect::InfiniteInverted();

    if (this->shapeCanBeModified() && fLocalToDevice->type() <= Transform::Type::kRectStaysRect) {
        if (fShape.isRect()) {
            fInnerBounds = fOuterBounds;
        } else if (fShape.isRRect()) {
            SkRect rrectInnerBounds = SkRRectPriv::InnerBounds(fShape.rrect());
            if (!rrectInnerBounds.isEmpty()) {
                fInnerBounds = fLocalToDevice->mapRect(rrectInnerBounds);
            }
        }
        // Otherwise it's a flood fill, but should have empty bounds anyways
    }
    // Otherwise we either don't need the inner bounds, or the inner bounds can't be computed
    // for a non-axis-aligned transform

     return true; // Something can be drawn based on style (might still be clipped out)
}

void ClipStack::DrawShape::applyScissor(const Rect& scissor) {
    // Apply the scissor to the outer bounds because it restricts rasterization and will allow
    // the SaveRecord::testForDraw() case to detect no clip influence if only the scissor is
    // needed.
    SkASSERT(scissor == Rect(scissor.asSkIRect())); // `scissor` must be integer valued
    fScissor.intersect(scissor); // For first call, fScissor is infinite so this is assignment
    fOuterBounds.intersect(scissor);
    fInnerBounds.intersect(scissor);
}

Clip ClipStack::DrawShape::toClip(Geometry* geometry,
                                  const NonMSAAClip& analyticClip,
                                  const SkShader* clipShader) {
    if (fShapeWasModified) {
        // Sync back to the geometry that will be drawn
        SkASSERT(this->shapeCanBeModified());
        if (geometry->isEdgeAAQuad() && fShape.isRect()) {
            // Preserve the EdgeAAQuad geometry type and sync updated edge flags
            SkASSERT(geometry->edgeAAQuad().isRect());
            geometry->setEdgeAAQuad(EdgeAAQuad(fShape.rect(), fEdgeFlags));
        } else {
            SkASSERT(fEdgeFlags == EdgeAAQuad::Flags::kAll);
            geometry->setShape(fShape);
        }
        // Reconstruct new transformedShapeBounds and outer bounds
        fTransformedShapeBounds = fLocalToDevice->mapRect(fShape.bounds());
        fOuterBounds = fTransformedShapeBounds.makeIntersect(fScissor);
    }

    Rect drawBounds = fShape.inverted() ? fScissor : fOuterBounds;
    // If the draw isn't clipped out (empty drawBounds), it should be in the scissor rect
    SkASSERT(drawBounds.isEmptyNegativeOrNaN() || fScissor.contains(drawBounds));
    // If the scissor is empty, the draw bounds must also be empty
    SkASSERT(!fScissor.isEmptyNegativeOrNaN() || drawBounds.isEmptyNegativeOrNaN());
    // fScissor.asSkIRect() must be equivalent
    SkASSERT(fScissor == Rect(fScissor.asSkIRect()));
    return Clip(drawBounds, fTransformedShapeBounds,
                fScissor.asSkIRect(), analyticClip, clipShader);
}

void ClipStack::DrawShape::resetToFloodFill() {
    if (this->shapeCanBeModified() && !fShape.isFloodFill()) {
        fShape.reset();
        fShape.setInverted(true);
        fEdgeFlags = EdgeAAQuad::Flags::kAll;
        fOuterBounds = fInnerBounds = Rect::InfiniteInverted();
        fShapeWasModified = true;
    }
}

bool ClipStack::DrawShape::intersectClipElement(const RawElement& clip) {
    SkASSERT(clip.op() == SkClipOp::kIntersect);
    if (this->shapeCanBeModified() &&
        intersect_shape(clip.localToDevice(), clip.shape(),
                        *fLocalToDevice, &fShape, &fEdgeFlags)) {
        SkASSERT(!fShape.inverted());
        if (fOuterBounds.isEmptyNegativeOrNaN()) {
            // Changing from a flood fill to the clip's shape
            fOuterBounds = clip.outerBounds();
            fInnerBounds = clip.innerBounds();
        } else {
            // Restricting the shape's geometry by the clip
            fOuterBounds.intersect(clip.outerBounds());
            fInnerBounds.intersect(clip.innerBounds());
            SkASSERT(!fOuterBounds.isEmptyNegativeOrNaN()); // Should have been caught earlier
        }

        fShapeWasModified = true;
        return true;
    }

    return false;
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

ClipStack::ClipStack(Device* owningDevice)
        : fElements(kElementStackIncrement)
        , fSaves(kSaveStackIncrement)
        , fDevice(owningDevice) {
    // Start with a save record that is wide open
    fSaves.emplace_back(this->deviceBounds());
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
    current.removeElements(&fElements, fDevice);

    fSaves.pop_back();
    // Restore any remaining elements that were only invalidated by the now-removed save record.
    fSaves.back().restoreElements(&fElements);
}

Rect ClipStack::deviceBounds() const {
    return Rect::WH(fDevice->width(), fDevice->height());
}

Rect ClipStack::conservativeBounds() const {
    const SaveRecord& current = this->currentSaveRecord();
    if (current.state() == ClipState::kEmpty) {
        return Rect::InfiniteInverted();
    } else if (current.state() == ClipState::kWideOpen) {
        return this->deviceBounds();
    } else {
        if (current.op() == SkClipOp::kDifference) {
            // The outer/inner bounds represent what's cut out, so full bounds remains the device
            // bounds, minus any fully clipped content that spans the device edge.
            return subtract(this->deviceBounds(), current.innerBounds(), /* exact */ true);
        } else {
            SkASSERT(this->deviceBounds().contains(current.outerBounds()));
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
    // TODO(b/238763003): Integrating clipShader into graphite needs more thought, particularly how
    // to handle the shader explosion and where to put the effects in the GraphicsPipelineDesc.
    // One idea is to use sample locations and draw the clipShader into the depth buffer.
    // Another is resolve the clip shader into an alpha mask image that is sampled by the draw.
}

void ClipStack::clipShape(const Transform& localToDevice,
                          const Shape& shape,
                          SkClipOp op,
                          PixelSnapping snapping) {
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    // This will apply the transform if it's shape-type preserving, and clip the element's bounds
    // to the device bounds (NOT the conservative clip bounds, since those are based on the net
    // effect of all elements while device bounds clipping happens implicitly. During addElement,
    // we may still be able to invalidate some older elements).
    // NOTE: Does not try to simplify the shape type by inspecting the SkPath.
    RawElement element{this->deviceBounds(), localToDevice, shape, op, snapping};

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
    if (!save.addElement(std::move(element), &fElements, fDevice)) {
        if (wasDeferred) {
            // We made a new save record, but ended up not adding an element to the stack.
            // So instead of keeping an empty save record around, pop it off and restore the counter
            SkASSERT(elementCount == fElements.count());
            fSaves.pop_back();
            fSaves.back().pushSave();
        }
    }
}

// Decide whether we can use this shape to do analytic clipping. Only rects and certain
// rrects are supported. We assume these have been pre-transformed by the RawElement
// constructor, so only identity transforms are allowed.
namespace {

AnalyticClip can_apply_analytic_clip(const Shape& shape, const Transform& localToDevice) {
    if (localToDevice.type() != Transform::Type::kIdentity) {
        return {};
    }

    // The circular rrect clip only handles rrect radii >= kRadiusMin, circular radii less than
    // this are coerced to be rectangular.
    static constexpr float kRadiusMin = SK_ScalarHalf;

    // Can handle Rect directly.
    if (shape.isRect()) {
        return {shape.rect(), kRadiusMin, AnalyticClip::kNone_EdgeFlag, shape.inverted()};
    }

    // Otherwise we only handle certain kinds of RRects, specifically only approximately simple
    // circular rrects (e.g. all 4 corners can be described by a single radius value).
    if (!shape.isRRect()) {
        return {};
    }

    const SkRRect& rrect = shape.rrect();
    if (rrect.isOval() || rrect.isSimple()) {
        SkVector radii = SkRRectPriv::GetSimpleRadii(rrect);
        if (radii.fX < kRadiusMin || radii.fY < kRadiusMin) {
            // In this case the corners are extremely close to rectangular and we collapse the
            // clip to a rectangular clip.
            return {rrect.rect(), kRadiusMin, AnalyticClip::kNone_EdgeFlag, shape.inverted()};
        }
        if (SkRRectPriv::IsRelativelyCircular(radii.fX, radii.fY, Shape::kDefaultPixelTolerance)) {
            return {rrect.rect(), radii.fX, AnalyticClip::kAll_EdgeFlag, shape.inverted()};
        } else {
            return {};
        }
    }

    // If rrect is not an oval or simple, it's either empty, rect, 9-patch, or complex. However,
    // empty should have been handled by the clip stack, and rect ought to have been simplified
    // into an explicit Rect shape (already handled above). That leaves 9-patch and complex,
    // so we check for the "tab" cases - two adjacent circular corners and two square corners.
    // It just so happens that if a rect RRect slipped through the cracks, we detect it here too.
    constexpr uint32_t kCornerFlags[4] = {
        AnalyticClip::kTop_EdgeFlag | AnalyticClip::kLeft_EdgeFlag,
        AnalyticClip::kTop_EdgeFlag | AnalyticClip::kRight_EdgeFlag,
        AnalyticClip::kBottom_EdgeFlag | AnalyticClip::kRight_EdgeFlag,
        AnalyticClip::kBottom_EdgeFlag | AnalyticClip::kLeft_EdgeFlag,
    };
    SkScalar circularRadius = 0;
    uint32_t edgeFlags = 0;
    int squareCount = 0;
    for (int corner = 0; corner < 4; ++corner) {
        SkVector radii = rrect.radii((SkRRect::Corner)corner);
        // Can only handle circular radii.
        // Also applies to corners with both zero and non-zero radii.
        if (!SkRRectPriv::IsRelativelyCircular(radii.fX, radii.fY, Shape::kDefaultPixelTolerance)) {
            return {};
        }
        if (radii.fX < kRadiusMin || radii.fY < kRadiusMin) {
            // The corner is square, so no need to flag as circular.
            squareCount++;
            continue;
        }
        // First circular corner seen
        if (!edgeFlags) {
            circularRadius = radii.fX;
        } else if (!SkRRectPriv::IsRelativelyCircular(radii.fX,
                                                      circularRadius,
                                                      Shape::kDefaultPixelTolerance)) {
            // Radius doesn't match previously seen circular radius
            return {};
        }
        edgeFlags |= kCornerFlags[corner];
    }

    if (edgeFlags == AnalyticClip::kNone_EdgeFlag) {
        // It's a rect (or coerced to a rect)
        return {rrect.rect(), kRadiusMin, edgeFlags, shape.inverted()};
    } else if (edgeFlags == AnalyticClip::kAll_EdgeFlag && squareCount != 0) {
        // If any rounded corner pairs are non-adjacent or if there are three rounded corners all
        // edge flags will be set, which is not valid.
        return {};
    } else {
        // At least one corner is rounded, or two adjacent corners are rounded, or all corners
        // are approximately the same (but not classified as simple due to inexactness).
        return {rrect.rect(), circularRadius, edgeFlags, shape.inverted()};
    }
}

}  // anonymous namespace

Clip ClipStack::visitClipStackForDraw(const Transform& localToDevice,
                                      Geometry* geometry,
                                      const SkStrokeRec& style,
                                      bool msaaSupported,
                                      ClipStack::ElementList* outEffectiveElements) const {
    static const Clip kClippedOut = {
            Rect::InfiniteInverted(), Rect::InfiniteInverted(), SkIRect::MakeEmpty(),
            /* nonMSAAClip= */ {}, /* shader= */ nullptr};

    const SaveRecord& cs = this->currentSaveRecord();
    if (cs.state() == ClipState::kEmpty) {
        // We know the draw is clipped out so don't bother computing the base draw bounds.
        return kClippedOut;
    }
    // Compute draw bounds, clipped only to our device bounds since we need to return that even if
    // the clip stack is known to be wide-open.
    const Rect deviceBounds = this->deviceBounds();

    DrawShape draw{localToDevice, *geometry};
    if (!draw.applyStyle(style, deviceBounds)) {
        return kClippedOut;
    }

    // For intersect clips, the scissor rectangle is snapped outer bounds (to loosely restrict
    // rasterization if absolutely necessary). Cases where the draw is fully inside the scissor are
    // automatically handled during GPU command generation.
    //
    // For difference clips, a tight scissor could be `subtract(drawBounds, cs.innerBounds())`
    // but this is only useful when the clip spans across an axis of the draw and can otherwise
    // lead to scissor state thrashing since it's connected to the draw's bounds as well. So just
    // use the device bounds for simplicity.
    draw.applyScissor(cs.op() == SkClipOp::kIntersect ? snap_scissor(cs.outerBounds(), deviceBounds)
                                                      : deviceBounds);

    switch (cs.testForDraw(draw)) {
        case DrawInfluence::kClipsOutDraw:
            // The draw is offscreen or clipped out, so there is no need to visit the clip elements.
            return kClippedOut;

        case DrawInfluence::kNone:
            // The draw is unaffected by the clip stack (except possibly `scissor`), and there's no
            // need to visit each clip element.
            return draw.toClip(geometry, {}, cs.shader());

        case DrawInfluence::kReplacesDraw:
            // The draw covers the clip entirely. Replace the shape with a flood fill, which can
            // intersect with shapes efficiently.
            draw.resetToFloodFill();
            [[fallthrough]];

        case DrawInfluence::kComplexInteraction:
            // Check each element's influence on the draw below
            break;
    }

    SkASSERT(outEffectiveElements);
    SkASSERT(outEffectiveElements->empty());
    int i = fElements.count();
    NonMSAAClip nonMSAAClip;
    for (const RawElement& e : fElements.ritems()) {
        --i;
        if (i < cs.oldestElementIndex()) {
            // All earlier elements have been invalidated by elements already processed so the draw
            // can't be affected by them and cannot contribute to their usage bounds.
            break;
        }

        switch (e.testForDraw(draw)) {
            case DrawInfluence::kClipsOutDraw:
                // Per-element check was able to completely reject the draw.
                outEffectiveElements->clear();
                return kClippedOut;

            case DrawInfluence::kNone:
                // This element does not interact, so continue to the next
                continue;

            case DrawInfluence::kReplacesDraw:
                // This element is covered entirely by the draw, so the draw's geometry can be
                // replaced assuming the coordinate spaces are compatible. To facilitate this, we
                // switch the drawn geometry to a flood fill and then fall through to intersection.
                // Even if the coordinate spaces aren't in alignment, this eliminates the draw's
                // source of analytic coverage.
                draw.resetToFloodFill();

                [[fallthrough]];

            case DrawInfluence::kComplexInteraction:
                // First try to handle the clip geometrically
                if (e.op() == SkClipOp::kIntersect && draw.intersectClipElement(e)) {
                    continue;
                }
                // Second try to tighten the scissor, which is lighter weight than adding an
                // analytic clip pipeline variation or triggering MSAA.
                if (e.clipType() == ClipState::kDeviceRect) {
                    Rect scissor = e.shape().rect().makeRound();
                    if (e.shape().rect().nearlyEquals(scissor, Shape::kDefaultPixelTolerance)) {
                        // Pass in `scissor` since these need to be integral values while
                        // nearlyEquals allows the original rect coordinates to be slightly
                        // different (causing problems later with asSkIRect()).
                        draw.applyScissor(scissor);
                        continue;
                    }
                }
                // // Third try to handle the clip analytically in the shader
                if (nonMSAAClip.fAnalyticClip.isEmpty()) {
                    nonMSAAClip.fAnalyticClip = can_apply_analytic_clip(e.shape(),
                                                                        e.localToDevice());
                    if (!nonMSAAClip.fAnalyticClip.isEmpty()) {
                        continue;
                    }
                }
                // Fourth, remember the element for later, either to be a depth-only draw or to be
                // flattened into a clip mask.
                // Otherwise, accumulate it for later. Depending on how many elements are collected
                // we may use the scissor, analytic clip, or MSAA/atlas.
                outEffectiveElements->push_back(&e);
                break;
        }
    }

#if !defined(SK_DISABLE_GRAPHITE_CLIP_ATLAS)
    // If there is no MSAA supported, rasterize any remaining elements by flattening them
    // into a single mask and storing in an atlas. Otherwise these will be handled by
    // Device::drawClip().
    AtlasProvider* atlasProvider = fDevice->recorder()->priv().atlasProvider();
    if (!msaaSupported && !outEffectiveElements->empty()) {
        ClipAtlasManager* clipAtlas = atlasProvider->getClipAtlasManager();
        SkASSERT(clipAtlas);
        AtlasClip* atlasClip = &nonMSAAClip.fAtlasClip;

        SkIRect iMaskBounds = cs.outerBounds().makeRoundOut().asSkIRect();
        sk_sp<TextureProxy> proxy = clipAtlas->findOrCreateEntry(cs.genID(),
                                                                 outEffectiveElements,
                                                                 iMaskBounds,
                                                                 &atlasClip->fOutPos);
        if (proxy) {
            // Add to Clip
            atlasClip->fMaskBounds = iMaskBounds;
            atlasClip->fAtlasTexture = std::move(proxy);

            // Elements are represented in the clip atlas, discard.
            outEffectiveElements->clear();
        }
    }
#endif

    return draw.toClip(geometry, nonMSAAClip, cs.shader());
}

CompressedPaintersOrder ClipStack::updateClipStateForDraw(const Clip& clip,
                                                          const ElementList& effectiveElements,
                                                          const BoundsManager* boundsManager,
                                                          PaintersDepth z) {
    if (clip.isClippedOut()) {
        return DrawOrder::kNoIntersection;
    }

    SkDEBUGCODE(const SaveRecord& cs = this->currentSaveRecord();)
    SkASSERT(cs.state() != ClipState::kEmpty);

    Rect deviceBounds = this->deviceBounds();
    CompressedPaintersOrder maxClipOrder = DrawOrder::kNoIntersection;
    for (int i = 0; i < effectiveElements.size(); ++i) {
        // ClipStack owns the elements in the `clipState` so it's OK to downcast and cast away
        // const.
        // TODO: Enforce the ownership? In debug builds we could invalidate a `ClipStateForDraw` if
        // its element pointers become dangling and assert validity here.
        const RawElement* e = static_cast<const RawElement*>(effectiveElements[i]);
        CompressedPaintersOrder order =  const_cast<RawElement*>(e)->updateForDraw(
                boundsManager, deviceBounds, clip.drawBounds(), z);
        maxClipOrder = std::max(order, maxClipOrder);
    }

    return maxClipOrder;
}

void ClipStack::recordDeferredClipDraws() {
    for (auto& e : fElements.items()) {
        // When a Device requires all clip elements to be recorded, we have to iterate all elements,
        // and will draw clip shapes for elements that are still marked as invalid from the clip
        // stack, including those that are older than the current save record's oldest valid index,
        // because they could have accumulated draw usage prior to being invalidated, but weren't
        // flushed when they were invalidated because of an intervening save.
        e.drawClip(fDevice);
    }
}

}  // namespace skgpu::graphite
