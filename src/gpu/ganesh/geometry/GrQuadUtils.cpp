/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/geometry/GrQuadUtils.h"

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMacros.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkVx.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"

#include <algorithm>
#include <cmath>

using float4 = skvx::float4;
using mask4  = skvx::int4; // aliased to 'mask' to emphasize that it will hold boolean SIMD masks.

#define AI SK_ALWAYS_INLINE

// General tolerance used for denominators, checking div-by-0
static constexpr float kTolerance = 1e-9f;
// Increased slop when comparing signed distances / lengths
static constexpr float kDistTolerance = 1e-2f;
static constexpr float kDist2Tolerance = kDistTolerance * kDistTolerance;
static constexpr float kInvDistTolerance = 1.f / kDistTolerance;

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
template<typename T>
static AI skvx::Vec<4, T> next_cw(const skvx::Vec<4, T>& v) {
    return skvx::shuffle<2, 0, 3, 1>(v);
}

template<typename T>
static AI skvx::Vec<4, T> next_ccw(const skvx::Vec<4, T>& v) {
    return skvx::shuffle<1, 3, 0, 2>(v);
}

static AI float4 next_diag(const float4& v) {
    // Same as next_ccw(next_ccw(v)), or next_cw(next_cw(v)), e.g. two rotations either direction.
    return skvx::shuffle<3, 2, 1, 0>(v);
}

// Replaces zero-length 'bad' edge vectors with the reversed opposite edge vector.
// e3 may be null if only 2D edges need to be corrected for.
static AI void correct_bad_edges(const mask4& bad, float4* e1, float4* e2, float4* e3) {
    if (any(bad)) {
        // Want opposite edges, L B T R -> R T B L but with flipped sign to preserve winding
        *e1 = if_then_else(bad, -next_diag(*e1), *e1);
        *e2 = if_then_else(bad, -next_diag(*e2), *e2);
        if (e3) {
            *e3 = if_then_else(bad, -next_diag(*e3), *e3);
        }
    }
}

// Replace 'bad' coordinates by rotating CCW to get the next point. c3 may be null for 2D points.
static AI void correct_bad_coords(const mask4& bad, float4* c1, float4* c2, float4* c3) {
    if (any(bad)) {
        *c1 = if_then_else(bad, next_ccw(*c1), *c1);
        *c2 = if_then_else(bad, next_ccw(*c2), *c2);
        if (c3) {
            *c3 = if_then_else(bad, next_ccw(*c3), *c3);
        }
    }
}

// Since the local quad may not be type kRect, this uses the opposites for each vertex when
// interpolating, and calculates new ws in addition to new xs, ys.
static void interpolate_local(float alpha, int v0, int v1, int v2, int v3,
                              float lx[4], float ly[4], float lw[4]) {
    SkASSERT(v0 >= 0 && v0 < 4);
    SkASSERT(v1 >= 0 && v1 < 4);
    SkASSERT(v2 >= 0 && v2 < 4);
    SkASSERT(v3 >= 0 && v3 < 4);

    float beta = 1.f - alpha;
    lx[v0] = alpha * lx[v0] + beta * lx[v2];
    ly[v0] = alpha * ly[v0] + beta * ly[v2];
    lw[v0] = alpha * lw[v0] + beta * lw[v2];

    lx[v1] = alpha * lx[v1] + beta * lx[v3];
    ly[v1] = alpha * ly[v1] + beta * ly[v3];
    lw[v1] = alpha * lw[v1] + beta * lw[v3];
}

// Crops v0 to v1 based on the clipDevRect. v2 is opposite of v0, v3 is opposite of v1.
// It is written to not modify coordinates if there's no intersection along the edge.
// Ideally this would have been detected earlier and the entire draw is skipped.
static bool crop_rect_edge(const SkRect& clipDevRect, int v0, int v1, int v2, int v3,
                           float x[4], float y[4], float lx[4], float ly[4], float lw[4]) {
    SkASSERT(v0 >= 0 && v0 < 4);
    SkASSERT(v1 >= 0 && v1 < 4);
    SkASSERT(v2 >= 0 && v2 < 4);
    SkASSERT(v3 >= 0 && v3 < 4);

    if (SkScalarNearlyEqual(x[v0], x[v1])) {
        // A vertical edge
        if (x[v0] < clipDevRect.fLeft && x[v2] >= clipDevRect.fLeft) {
            // Overlapping with left edge of clipDevRect
            if (lx) {
                float alpha = (x[v2] - clipDevRect.fLeft) / (x[v2] - x[v0]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            x[v0] = clipDevRect.fLeft;
            x[v1] = clipDevRect.fLeft;
            return true;
        } else if (x[v0] > clipDevRect.fRight && x[v2] <= clipDevRect.fRight) {
            // Overlapping with right edge of clipDevRect
            if (lx) {
                float alpha = (clipDevRect.fRight - x[v2]) / (x[v0] - x[v2]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            x[v0] = clipDevRect.fRight;
            x[v1] = clipDevRect.fRight;
            return true;
        }
    } else {
        // A horizontal edge
        SkASSERT(SkScalarNearlyEqual(y[v0], y[v1]));
        if (y[v0] < clipDevRect.fTop && y[v2] >= clipDevRect.fTop) {
            // Overlapping with top edge of clipDevRect
            if (lx) {
                float alpha = (y[v2] - clipDevRect.fTop) / (y[v2] - y[v0]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            y[v0] = clipDevRect.fTop;
            y[v1] = clipDevRect.fTop;
            return true;
        } else if (y[v0] > clipDevRect.fBottom && y[v2] <= clipDevRect.fBottom) {
            // Overlapping with bottom edge of clipDevRect
            if (lx) {
                float alpha = (clipDevRect.fBottom - y[v2]) / (y[v0] - y[v2]);
                interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            }
            y[v0] = clipDevRect.fBottom;
            y[v1] = clipDevRect.fBottom;
            return true;
        }
    }

    // No overlap so don't crop it
    return false;
}

// Updates x and y to intersect with clipDevRect.  lx, ly, and lw are updated appropriately and may
// be null to skip calculations. Returns bit mask of edges that were clipped.
static GrQuadAAFlags crop_rect(const SkRect& clipDevRect, float x[4], float y[4],
                               float lx[4], float ly[4], float lw[4]) {
    GrQuadAAFlags clipEdgeFlags = GrQuadAAFlags::kNone;

    // The quad's left edge may not align with the SkRect notion of left due to 90 degree rotations
    // or mirrors. So, this processes the logical edges of the quad and clamps it to the 4 sides of
    // clipDevRect.

    // Quad's left is v0 to v1 (op. v2 and v3)
    if (crop_rect_edge(clipDevRect, 0, 1, 2, 3, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kLeft;
    }
    // Quad's top edge is v0 to v2 (op. v1 and v3)
    if (crop_rect_edge(clipDevRect, 0, 2, 1, 3, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kTop;
    }
    // Quad's right edge is v2 to v3 (op. v0 and v1)
    if (crop_rect_edge(clipDevRect, 2, 3, 0, 1, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kRight;
    }
    // Quad's bottom edge is v1 to v3 (op. v0 and v2)
    if (crop_rect_edge(clipDevRect, 1, 3, 0, 2, x, y, lx, ly, lw)) {
        clipEdgeFlags |= GrQuadAAFlags::kBottom;
    }

    return clipEdgeFlags;
}

// Similar to crop_rect, but assumes that both the device coordinates and optional local coordinates
// geometrically match the TL, BL, TR, BR vertex ordering, i.e. axis-aligned but not flipped, etc.
static GrQuadAAFlags crop_simple_rect(const SkRect& clipDevRect, float x[4], float y[4],
                                      float lx[4], float ly[4]) {
    GrQuadAAFlags clipEdgeFlags = GrQuadAAFlags::kNone;

    // Update local coordinates proportionately to how much the device rect edge was clipped
    const SkScalar dx = lx ? (lx[2] - lx[0]) / (x[2] - x[0]) : 0.f;
    const SkScalar dy = ly ? (ly[1] - ly[0]) / (y[1] - y[0]) : 0.f;
    if (clipDevRect.fLeft > x[0]) {
        if (lx) {
            lx[0] += (clipDevRect.fLeft - x[0]) * dx;
            lx[1] = lx[0];
        }
        x[0] = clipDevRect.fLeft;
        x[1] = clipDevRect.fLeft;
        clipEdgeFlags |= GrQuadAAFlags::kLeft;
    }
    if (clipDevRect.fTop > y[0]) {
        if (ly) {
            ly[0] += (clipDevRect.fTop - y[0]) * dy;
            ly[2] = ly[0];
        }
        y[0] = clipDevRect.fTop;
        y[2] = clipDevRect.fTop;
        clipEdgeFlags |= GrQuadAAFlags::kTop;
    }
    if (clipDevRect.fRight < x[2]) {
        if (lx) {
            lx[2] -= (x[2] - clipDevRect.fRight) * dx;
            lx[3] = lx[2];
        }
        x[2] = clipDevRect.fRight;
        x[3] = clipDevRect.fRight;
        clipEdgeFlags |= GrQuadAAFlags::kRight;
    }
    if (clipDevRect.fBottom < y[1]) {
        if (ly) {
            ly[1] -= (y[1] - clipDevRect.fBottom) * dy;
            ly[3] = ly[1];
        }
        y[1] = clipDevRect.fBottom;
        y[3] = clipDevRect.fBottom;
        clipEdgeFlags |= GrQuadAAFlags::kBottom;
    }

    return clipEdgeFlags;
}
// Consistent with GrQuad::asRect()'s return value but requires fewer operations since we don't need
// to calculate the bounds of the quad.
static bool is_simple_rect(const GrQuad& quad) {
    if (quad.quadType() != GrQuad::Type::kAxisAligned) {
        return false;
    }
    // v0 at the geometric top-left is unique, so we only need to compare x[0] < x[2] for left
    // and y[0] < y[1] for top, but add a little padding to protect against numerical precision
    // on R90 and R270 transforms tricking this check.
    return ((quad.x(0) + SK_ScalarNearlyZero) < quad.x(2)) &&
           ((quad.y(0) + SK_ScalarNearlyZero) < quad.y(1));
}

// Calculates barycentric coordinates for each point in (testX, testY) in the triangle formed by
// (x0,y0) - (x1,y1) - (x2, y2) and stores them in u, v, w.
static bool barycentric_coords(float x0, float y0, float x1, float y1, float x2, float y2,
                               const float4& testX, const float4& testY,
                               float4* u, float4* v, float4* w) {
    // The 32-bit calculations can have catastrophic cancellation if the device-space coordinates
    // are really big, and this code needs to handle that because we evaluate barycentric coords
    // pre-cropping to the render target bounds. This preserves some precision by shrinking the
    // coordinate space if the bounds are large.
    static constexpr float kCoordLimit = 1e7f; // Big but somewhat arbitrary, fixes crbug:10141204
    float scaleX = std::max(std::max(x0, x1), x2) - std::min(std::min(x0, x1), x2);
    float scaleY = std::max(std::max(y0, y1), y2) - std::min(std::min(y0, y1), y2);
    if (scaleX > kCoordLimit) {
        scaleX = kCoordLimit / scaleX;
        x0 *= scaleX;
        x1 *= scaleX;
        x2 *= scaleX;
    } else {
        // Don't scale anything
        scaleX = 1.f;
    }
    if (scaleY > kCoordLimit) {
        scaleY = kCoordLimit / scaleY;
        y0 *= scaleY;
        y1 *= scaleY;
        y2 *= scaleY;
    } else {
        scaleY = 1.f;
    }

    // Modeled after SkPathOpsQuad::pointInTriangle() but uses float instead of double, is
    // vectorized and outputs normalized barycentric coordinates instead of inside/outside test
    float v0x = x2 - x0;
    float v0y = y2 - y0;
    float v1x = x1 - x0;
    float v1y = y1 - y0;

    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot11 = v1x * v1x + v1y * v1y;

    // Not yet 1/d, first check d != 0 with a healthy tolerance (worst case is we end up not
    // cropping something we could have, which is better than cropping something we shouldn't have).
    // The tolerance is partly so large because these comparisons operate in device px^4 units,
    // with plenty of subtractions thrown in. The SkPathOpsQuad code's use of doubles helped, and
    // because it only needed to return "inside triangle", it could compare against [0, denom] and
    // skip the normalization entirely.
    float invDenom = dot00 * dot11 - dot01 * dot01;
    static constexpr SkScalar kEmptyTriTolerance = SK_Scalar1 / (1 << 5);
    if (SkScalarNearlyZero(invDenom, kEmptyTriTolerance)) {
        // The triangle was degenerate/empty, which can cause the following UVW calculations to
        // return (0,0,1) for every test point. This in turn makes the cropping code think that the
        // empty triangle contains the crop rect and we turn the draw into a fullscreen clear, which
        // is definitely the utter opposite of what we'd expect for an empty shape.
        return false;
    } else {
        // Safe to divide
        invDenom = sk_ieee_float_divide(1.f, invDenom);
    }

    float4 v2x = (scaleX * testX) - x0;
    float4 v2y = (scaleY * testY) - y0;

    float4 dot02 = v0x * v2x + v0y * v2y;
    float4 dot12 = v1x * v2x + v1y * v2y;

    // These are relative to the vertices, so there's no need to undo the scale factor
    *u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    *v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    *w = 1.f - *u - *v;

    return true;
}

static mask4 inside_triangle(const float4& u, const float4& v, const float4& w) {
    return ((u >= 0.f) & (u <= 1.f)) & ((v >= 0.f) & (v <= 1.f)) & ((w >= 0.f) & (w <= 1.f));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRect GrQuad::projectedBounds() const {
    float4 xs = this->x4f();
    float4 ys = this->y4f();
    float4 ws = this->w4f();
    mask4 clipW = ws < SkPathPriv::kW0PlaneDistance;
    if (any(clipW)) {
        float4 x2d = xs / ws;
        float4 y2d = ys / ws;
        // Bounds of just the projected points in front of w = epsilon
        SkRect frontBounds = {
            min(if_then_else(clipW, float4(SK_ScalarInfinity), x2d)),
            min(if_then_else(clipW, float4(SK_ScalarInfinity), y2d)),
            max(if_then_else(clipW, float4(SK_ScalarNegativeInfinity), x2d)),
            max(if_then_else(clipW, float4(SK_ScalarNegativeInfinity), y2d))
        };
        // Calculate clipped coordinates by following CCW edges, only keeping points where the w
        // actually changes sign between the vertices.
        float4 t = (SkPathPriv::kW0PlaneDistance - ws) / (next_ccw(ws) - ws);
        x2d = (t * next_ccw(xs) + (1.f - t) * xs) / SkPathPriv::kW0PlaneDistance;
        y2d = (t * next_ccw(ys) + (1.f - t) * ys) / SkPathPriv::kW0PlaneDistance;
        // True if (w < e) xor (ccw(w) < e), i.e. crosses the w = epsilon plane
        clipW = clipW ^ (next_ccw(ws) < SkPathPriv::kW0PlaneDistance);
        return {
            min(if_then_else(clipW, x2d, float4(frontBounds.fLeft))),
            min(if_then_else(clipW, y2d, float4(frontBounds.fTop))),
            max(if_then_else(clipW, x2d, float4(frontBounds.fRight))),
            max(if_then_else(clipW, y2d, float4(frontBounds.fBottom)))
        };
    } else {
        // Nothing is behind the viewer, so the projection is straight forward and valid
        ws = 1.f / ws;
        float4 x2d = xs * ws;
        float4 y2d = ys * ws;
        return {min(x2d), min(y2d), max(x2d), max(y2d)};
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace GrQuadUtils {

void ResolveAAType(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags, const GrQuad& quad,
                   GrAAType* outAAType, GrQuadAAFlags* outEdgeFlags) {
    // Most cases will keep the requested types unchanged
    *outAAType = requestedAAType;
    *outEdgeFlags = requestedEdgeFlags;

    switch (requestedAAType) {
        // When aa type is coverage, disable AA if the edge configuration doesn't actually need it
        case GrAAType::kCoverage:
            if (requestedEdgeFlags == GrQuadAAFlags::kNone) {
                // This can happen when quads are drawn in bulk, where the requestedAAType was
                // conservatively enabled and the edge flags are per-entry.
                *outAAType = GrAAType::kNone;
            } else if (quad.quadType() == GrQuad::Type::kAxisAligned &&
                       !quad.aaHasEffectOnRect(requestedEdgeFlags)) {
                // For coverage AA, if the quad is a rect and AA-enabled edges line up with pixel
                // boundaries, then overall AA and per-edge AA can be completely disabled.
                *outAAType = GrAAType::kNone;
                *outEdgeFlags = GrQuadAAFlags::kNone;
            }

            break;
        // For no or msaa anti aliasing, override the edge flags since edge flags only make sense
        // when coverage aa is being used.
        case GrAAType::kNone:
            *outEdgeFlags = GrQuadAAFlags::kNone;
            break;
        case GrAAType::kMSAA:
            *outEdgeFlags = GrQuadAAFlags::kAll;
            break;
    }
}

int ClipToW0(DrawQuad* quad, DrawQuad* extraVertices) {
    using Vertices = TessellationHelper::Vertices;

    SkASSERT(quad && extraVertices);

    if (quad->fDevice.quadType() < GrQuad::Type::kPerspective) {
        // W implicitly 1s for each vertex, so nothing to do but draw unmodified 'quad'
        return 1;
    }

    mask4 validW = quad->fDevice.w4f() >= SkPathPriv::kW0PlaneDistance;
    if (all(validW)) {
        // Nothing to clip, can proceed normally drawing just 'quad'
        return 1;
    } else if (!any(validW)) {
        // Everything is clipped, so draw nothing
        return 0;
    }

    // The clipped local coordinates will most likely not remain rectilinear
    GrQuad::Type localType = quad->fLocal.quadType();
    if (localType < GrQuad::Type::kGeneral) {
        localType = GrQuad::Type::kGeneral;
    }

    // If we got here, there are 1, 2, or 3 points behind the w = 0 plane. If 2 or 3 points are
    // clipped we can define a new quad that covers the clipped shape directly. If there's 1 clipped
    // out, the new geometry is a pentagon.
    Vertices v;
    v.reset(quad->fDevice, &quad->fLocal);

    int clipCount = (validW[0] ? 0 : 1) + (validW[1] ? 0 : 1) +
                    (validW[2] ? 0 : 1) + (validW[3] ? 0 : 1);
    SkASSERT(clipCount >= 1 && clipCount <= 3);

    // FIXME de-duplicate from the projectedBounds() calculations.
    float4 t = (SkPathPriv::kW0PlaneDistance - v.fW) / (next_ccw(v.fW) - v.fW);

    Vertices clip;
    clip.fX = (t * next_ccw(v.fX) + (1.f - t) * v.fX);
    clip.fY = (t * next_ccw(v.fY) + (1.f - t) * v.fY);
    clip.fW = SkPathPriv::kW0PlaneDistance;

    clip.fU = (t * next_ccw(v.fU) + (1.f - t) * v.fU);
    clip.fV = (t * next_ccw(v.fV) + (1.f - t) * v.fV);
    clip.fR = (t * next_ccw(v.fR) + (1.f - t) * v.fR);

    mask4 ccwValid = next_ccw(v.fW) >= SkPathPriv::kW0PlaneDistance;
    mask4 cwValid  = next_cw(v.fW)  >= SkPathPriv::kW0PlaneDistance;

    if (clipCount != 1) {
        // Simplest case, replace behind-w0 points with their clipped points by following CCW edge
        // or CW edge, depending on if the edge crosses from neg. to pos. w or pos. to neg.
        SkASSERT(clipCount == 2 || clipCount == 3);

        // NOTE: when 3 vertices are clipped, this results in a degenerate quad where one vertex
        // is replicated. This is preferably to inserting a 3rd vertex on the w = 0 intersection
        // line because two parallel edges make inset/outset math unstable for large quads.
        v.fX = if_then_else(validW, v.fX,
                       if_then_else((!ccwValid) & (!cwValid), next_ccw(clip.fX),
                               if_then_else(ccwValid, clip.fX, /* cwValid */ next_cw(clip.fX))));
        v.fY = if_then_else(validW, v.fY,
                       if_then_else((!ccwValid) & (!cwValid), next_ccw(clip.fY),
                               if_then_else(ccwValid, clip.fY, /* cwValid */ next_cw(clip.fY))));
        v.fW = if_then_else(validW, v.fW, clip.fW);

        v.fU = if_then_else(validW, v.fU,
                       if_then_else((!ccwValid) & (!cwValid), next_ccw(clip.fU),
                               if_then_else(ccwValid, clip.fU, /* cwValid */ next_cw(clip.fU))));
        v.fV = if_then_else(validW, v.fV,
                       if_then_else((!ccwValid) & (!cwValid), next_ccw(clip.fV),
                               if_then_else(ccwValid, clip.fV, /* cwValid */ next_cw(clip.fV))));
        v.fR = if_then_else(validW, v.fR,
                       if_then_else((!ccwValid) & (!cwValid), next_ccw(clip.fR),
                               if_then_else(ccwValid, clip.fR, /* cwValid */ next_cw(clip.fR))));

        // For 2 or 3 clipped vertices, the resulting shape is a quad or a triangle, so it can be
        // entirely represented in 'quad'.
        v.asGrQuads(&quad->fDevice, GrQuad::Type::kPerspective,
                    &quad->fLocal, localType);
        return 1;
    } else {
        // The clipped geometry is a pentagon, so it will be represented as two quads connected by
        // a new non-AA edge. Use the midpoint along one of the unclipped edges as a split vertex.
        Vertices mid;
        mid.fX = 0.5f * (v.fX + next_ccw(v.fX));
        mid.fY = 0.5f * (v.fY + next_ccw(v.fY));
        mid.fW = 0.5f * (v.fW + next_ccw(v.fW));

        mid.fU = 0.5f * (v.fU + next_ccw(v.fU));
        mid.fV = 0.5f * (v.fV + next_ccw(v.fV));
        mid.fR = 0.5f * (v.fR + next_ccw(v.fR));

        // Make a quad formed by the 2 clipped points, the inserted mid point, and the good vertex
        // that is CCW rotated from the clipped vertex.
        Vertices v2;
        v2.fUVRCount = v.fUVRCount;
        v2.fX = if_then_else((!validW) | (!ccwValid), clip.fX,
                        if_then_else(cwValid, next_cw(mid.fX), v.fX));
        v2.fY = if_then_else((!validW) | (!ccwValid), clip.fY,
                        if_then_else(cwValid, next_cw(mid.fY), v.fY));
        v2.fW = if_then_else((!validW) | (!ccwValid), clip.fW,
                        if_then_else(cwValid, next_cw(mid.fW), v.fW));

        v2.fU = if_then_else((!validW) | (!ccwValid), clip.fU,
                        if_then_else(cwValid, next_cw(mid.fU), v.fU));
        v2.fV = if_then_else((!validW) | (!ccwValid), clip.fV,
                        if_then_else(cwValid, next_cw(mid.fV), v.fV));
        v2.fR = if_then_else((!validW) | (!ccwValid), clip.fR,
                        if_then_else(cwValid, next_cw(mid.fR), v.fR));
        // The non-AA edge for this quad is the opposite of the clipped vertex's edge
        GrQuadAAFlags v2EdgeFlag = (!validW[0] ? GrQuadAAFlags::kRight  : // left clipped -> right
                                   (!validW[1] ? GrQuadAAFlags::kTop    : // bottom clipped -> top
                                   (!validW[2] ? GrQuadAAFlags::kBottom : // top clipped -> bottom
                                                 GrQuadAAFlags::kLeft))); // right clipped -> left
        extraVertices->fEdgeFlags = quad->fEdgeFlags & ~v2EdgeFlag;

        // Make a quad formed by the remaining two good vertices, one clipped point, and the
        // inserted mid point.
        v.fX = if_then_else(!validW, next_cw(clip.fX),
                       if_then_else(!cwValid, mid.fX, v.fX));
        v.fY = if_then_else(!validW, next_cw(clip.fY),
                       if_then_else(!cwValid, mid.fY, v.fY));
        v.fW = if_then_else(!validW, clip.fW,
                       if_then_else(!cwValid, mid.fW, v.fW));

        v.fU = if_then_else(!validW, next_cw(clip.fU),
                       if_then_else(!cwValid, mid.fU, v.fU));
        v.fV = if_then_else(!validW, next_cw(clip.fV),
                       if_then_else(!cwValid, mid.fV, v.fV));
        v.fR = if_then_else(!validW, next_cw(clip.fR),
                       if_then_else(!cwValid, mid.fR, v.fR));
        // The non-AA edge for this quad is the clipped vertex's edge
        GrQuadAAFlags v1EdgeFlag = (!validW[0] ? GrQuadAAFlags::kLeft   :
                                   (!validW[1] ? GrQuadAAFlags::kBottom :
                                   (!validW[2] ? GrQuadAAFlags::kTop    :
                                                 GrQuadAAFlags::kRight)));

        v.asGrQuads(&quad->fDevice, GrQuad::Type::kPerspective,
                    &quad->fLocal, localType);
        quad->fEdgeFlags &= ~v1EdgeFlag;

        v2.asGrQuads(&extraVertices->fDevice, GrQuad::Type::kPerspective,
                     &extraVertices->fLocal, localType);
        // Caller must draw both 'quad' and 'extraVertices' to cover the clipped geometry
        return 2;
    }
}

bool CropToRect(const SkRect& cropRect, GrAA cropAA, DrawQuad* quad, bool computeLocal) {
    SkASSERT(quad->fDevice.isFinite());

    if (quad->fDevice.quadType() == GrQuad::Type::kAxisAligned) {
        // crop_rect and crop_rect_simple keep the rectangles as rectangles, so the intersection
        // of the crop and quad can be calculated exactly. Some care must be taken if the quad
        // is axis-aligned but does not satisfy asRect() due to flips, etc.
        GrQuadAAFlags clippedEdges;
        if (computeLocal) {
            if (is_simple_rect(quad->fDevice) && is_simple_rect(quad->fLocal)) {
                clippedEdges = crop_simple_rect(cropRect, quad->fDevice.xs(), quad->fDevice.ys(),
                                                quad->fLocal.xs(), quad->fLocal.ys());
            } else {
                clippedEdges = crop_rect(cropRect, quad->fDevice.xs(), quad->fDevice.ys(),
                                         quad->fLocal.xs(), quad->fLocal.ys(), quad->fLocal.ws());
            }
        } else {
            if (is_simple_rect(quad->fDevice)) {
                clippedEdges = crop_simple_rect(cropRect, quad->fDevice.xs(), quad->fDevice.ys(),
                                                nullptr, nullptr);
            } else {
                clippedEdges = crop_rect(cropRect, quad->fDevice.xs(), quad->fDevice.ys(),
                                         nullptr, nullptr, nullptr);
            }
        }

        // Apply the clipped edge updates to the original edge flags
        if (cropAA == GrAA::kYes) {
            // Turn on all edges that were clipped
            quad->fEdgeFlags |= clippedEdges;
        } else {
            // Turn off all edges that were clipped
            quad->fEdgeFlags &= ~clippedEdges;
        }
        return true;
    }

    if (computeLocal || quad->fDevice.quadType() == GrQuad::Type::kPerspective) {
        // FIXME (michaelludwig) Calculate cropped local coordinates when not kAxisAligned
        // FIXME (michaelludwig) crbug.com/1204347 and skbug.com/40041027 - disable this when there's
        // perspective; it does not prove numerical robust enough in the wild and should be
        // revisited.
        return false;
    }

    float4 devX = quad->fDevice.x4f();
    float4 devY = quad->fDevice.y4f();

    float4 clipX = {cropRect.fLeft, cropRect.fLeft, cropRect.fRight, cropRect.fRight};
    float4 clipY = {cropRect.fTop, cropRect.fBottom, cropRect.fTop, cropRect.fBottom};

    // Calculate barycentric coordinates for the 4 rect corners in the 2 triangles that the quad
    // is tessellated into when drawn.
    float4 u1, v1, w1;
    float4 u2, v2, w2;
    if (!barycentric_coords(devX[0], devY[0], devX[1], devY[1], devX[2], devY[2], clipX, clipY,
                            &u1, &v1, &w1) ||
        !barycentric_coords(devX[1], devY[1], devX[3], devY[3], devX[2], devY[2], clipX, clipY,
                            &u2, &v2, &w2)) {
        // Bad triangles, skip cropping
        return false;
    }

    // clipDevRect is completely inside this quad if each corner is in at least one of two triangles
    mask4 inTri1 = inside_triangle(u1, v1, w1);
    mask4 inTri2 = inside_triangle(u2, v2, w2);
    if (all(inTri1 | inTri2)) {
        // We can crop to exactly the clipDevRect.
        // FIXME (michaelludwig) - there are other ways to have determined quad covering the clip
        // rect, but the barycentric coords will be useful to derive local coordinates in the future

        // Since we are cropped to exactly clipDevRect, we have discarded any perspective and the
        // type becomes kRect. If updated locals were requested, they will incorporate perspective.
        // FIXME (michaelludwig) - once we have local coordinates handled, it may be desirable to
        // keep the draw as perspective so that the hardware does perspective interpolation instead
        // of pushing it into a local coord w and having the shader do an extra divide.
        clipX.store(quad->fDevice.xs());
        clipY.store(quad->fDevice.ys());
        quad->fDevice.setQuadType(GrQuad::Type::kAxisAligned);

        // Update the edge flags to match the clip setting since all 4 edges have been clipped
        quad->fEdgeFlags = cropAA == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;

        return true;
    }

    // FIXME (michaelludwig) - use TessellationHelper's inset/outset math to move
    // edges to the closest clip corner they are outside of

    return false;
}

bool WillUseHairline(const GrQuad& quad, GrAAType aaType, GrQuadAAFlags edgeFlags) {
    if (aaType != GrAAType::kCoverage || edgeFlags != GrQuadAAFlags::kAll) {
        // Non-aa or msaa don't do any outsetting so they will not be hairlined; mixed edge flags
        // could be hairlined in theory, but applying hairline bloat would extend beyond the
        // original tiled shape.
        return false;
    }

    if (quad.quadType() == GrQuad::Type::kAxisAligned) {
        // Fast path that avoids computing edge properties via TessellationHelper.
        // Taking the absolute value of the diagonals always produces the minimum of width or
        // height given that this is axis-aligned, regardless of mirror or 90/180-degree rotations.
        float d = std::min(std::abs(quad.x(3) - quad.x(0)), std::abs(quad.y(3) - quad.y(0)));
        return d < 1.f;
    } else {
        TessellationHelper helper;
        helper.reset(quad, nullptr);
        return helper.isSubpixel();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TessellationHelper implementation and helper struct implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

//** EdgeVectors implementation

void TessellationHelper::EdgeVectors::reset(const skvx::Vec<4, float>& xs,
                                            const skvx::Vec<4, float>& ys,
                                            const skvx::Vec<4, float>& ws,
                                            GrQuad::Type quadType) {
    // Calculate all projected edge vector values for this quad.
    if (quadType == GrQuad::Type::kPerspective) {
        float4 iw = 1.f / ws;
        fX2D = xs * iw;
        fY2D = ys * iw;
    } else {
        fX2D = xs;
        fY2D = ys;
    }

    fDX = next_ccw(fX2D) - fX2D;
    fDY = next_ccw(fY2D) - fY2D;
    fInvLengths = 1.f / sqrt(fDX*fDX + fDY*fDY);

    // Normalize edge vectors
    fDX *= fInvLengths;
    fDY *= fInvLengths;

    // Calculate angles between vectors
    if (quadType <= GrQuad::Type::kRectilinear) {
        fCosTheta = 0.f;
        fInvSinTheta = 1.f;
    } else {
        fCosTheta = fDX*next_cw(fDX) + fDY*next_cw(fDY);
        // NOTE: if cosTheta is close to 1, inset/outset math will avoid the fast paths that rely
        // on thefInvSinTheta since it will approach infinity.
        fInvSinTheta = 1.f / sqrt(1.f - fCosTheta * fCosTheta);
    }
}

//** EdgeEquations implementation

void TessellationHelper::EdgeEquations::reset(const EdgeVectors& edgeVectors) {
    float4 dx = edgeVectors.fDX;
    float4 dy = edgeVectors.fDY;
    // Correct for bad edges by copying adjacent edge information into the bad component
    correct_bad_edges(edgeVectors.fInvLengths >= kInvDistTolerance, &dx, &dy, nullptr);

    float4 c = dx*edgeVectors.fY2D - dy*edgeVectors.fX2D;
    // Make sure normals point into the shape
    float4 test = dy * next_cw(edgeVectors.fX2D) + (-dx * next_cw(edgeVectors.fY2D) + c);
    if (any(test < -kDistTolerance)) {
        fA = -dy;
        fB = dx;
        fC = -c;
    } else {
        fA = dy;
        fB = -dx;
        fC = c;
    }
}

float4 TessellationHelper::EdgeEquations::estimateCoverage(const float4& x2d,
                                                           const float4& y2d) const {
    // Calculate distance of the 4 inset points (px, py) to the 4 edges
    float4 d0 = fA[0]*x2d + (fB[0]*y2d + fC[0]);
    float4 d1 = fA[1]*x2d + (fB[1]*y2d + fC[1]);
    float4 d2 = fA[2]*x2d + (fB[2]*y2d + fC[2]);
    float4 d3 = fA[3]*x2d + (fB[3]*y2d + fC[3]);

    // For each point, pretend that there's a rectangle that touches e0 and e3 on the horizontal
    // axis, so its width is "approximately" d0 + d3, and it touches e1 and e2 on the vertical axis
    // so its height is d1 + d2. Pin each of these dimensions to [0, 1] and approximate the coverage
    // at each point as clamp(d0+d3, 0, 1) x clamp(d1+d2, 0, 1). For rectilinear quads this is an
    // accurate calculation of its area clipped to an aligned pixel. For arbitrary quads it is not
    // mathematically accurate but qualitatively provides a stable value proportional to the size of
    // the shape.
    float4 w = max(0.f, min(1.f, d0 + d3));
    float4 h = max(0.f, min(1.f, d1 + d2));
    return w * h;
}

bool TessellationHelper::EdgeEquations::isSubpixel(const float4& x2d, const float4& y2d) const {
    // Compute the minimum distances from vertices to opposite edges. If all 4 minimum distances
    // are less than 1px, then the inset geometry would be a point or line and quad rendering
    // will switch to hairline mode.
    float4 d = min(x2d * skvx::shuffle<1,2,1,2>(fA) + y2d * skvx::shuffle<1,2,1,2>(fB)
                           + skvx::shuffle<1,2,1,2>(fC),
                   x2d * skvx::shuffle<3,3,0,0>(fA) + y2d * skvx::shuffle<3,3,0,0>(fB)
                           + skvx::shuffle<3,3,0,0>(fC));
    return all(d < 1.f);
}

int TessellationHelper::EdgeEquations::computeDegenerateQuad(const float4& signedEdgeDistances,
                                                             float4* x2d, float4* y2d,
                                                             mask4* aaMask) const {
    // If the original points form a line in the 2D projection then give up on antialiasing.
    for (int i = 0; i < 4; ++i) {
        float4 d = (*x2d)*fA[i] + (*y2d)*fB[i] + fC[i];
        if (all(abs(d) < kDistTolerance)) {
            *aaMask = mask4(0);
            return 4;
        }
    }

    *aaMask = signedEdgeDistances != 0.f;

    // Move the edge by the signed edge adjustment.
    float4 oc = fC + signedEdgeDistances;

    // There are 6 points that we care about to determine the final shape of the polygon, which
    // are the intersections between (e0,e2), (e1,e0), (e2,e3), (e3,e1) (corresponding to the
    // 4 corners), and (e1, e2), (e0, e3) (representing the intersections of opposite edges).
    float4 denom = fA * next_cw(fB) - fB * next_cw(fA);
    float4 px = (fB * next_cw(oc) - oc * next_cw(fB)) / denom;
    float4 py = (oc * next_cw(fA) - fA * next_cw(oc)) / denom;
    correct_bad_coords(abs(denom) < kTolerance, &px, &py, nullptr);

    // Calculate the signed distances from these 4 corners to the other two edges that did not
    // define the intersection. So p(0) is compared to e3,e1, p(1) to e3,e2 , p(2) to e0,e1, and
    // p(3) to e0,e2
    float4 dists1 = px * skvx::shuffle<3, 3, 0, 0>(fA) +
                    py * skvx::shuffle<3, 3, 0, 0>(fB) +
                    skvx::shuffle<3, 3, 0, 0>(oc);
    float4 dists2 = px * skvx::shuffle<1, 2, 1, 2>(fA) +
                    py * skvx::shuffle<1, 2, 1, 2>(fB) +
                    skvx::shuffle<1, 2, 1, 2>(oc);

    // If all the distances are >= 0, the 4 corners form a valid quadrilateral, so use them as
    // the 4 points. If any point is on the wrong side of both edges, the interior has collapsed
    // and we need to use a central point to represent it. If all four points are only on the
    // wrong side of 1 edge, one edge has crossed over another and we use a line to represent it.
    // Otherwise, use a triangle that replaces the bad points with the intersections of
    // (e1, e2) or (e0, e3) as needed.
    mask4 d1v0 = dists1 < kDistTolerance;
    mask4 d2v0 = dists2 < kDistTolerance;
    mask4 d1And2 = d1v0 & d2v0;
    mask4 d1Or2 = d1v0 | d2v0;

    if (!any(d1Or2)) {
        // Every dists1 and dists2 >= kTolerance so it's not degenerate, use all 4 corners as-is
        // and use full coverage
        *x2d = px;
        *y2d = py;
        return 4;
    } else if (any(d1And2)) {
        // A point failed against two edges, so reduce the shape to a single point, which we take as
        // the center of the original quad to ensure it is contained in the intended geometry. Since
        // it has collapsed, we know the shape cannot cover a pixel so update the coverage.
        SkPoint center = {0.25f * ((*x2d)[0] + (*x2d)[1] + (*x2d)[2] + (*x2d)[3]),
                          0.25f * ((*y2d)[0] + (*y2d)[1] + (*y2d)[2] + (*y2d)[3])};
        *x2d = center.fX;
        *y2d = center.fY;
        *aaMask = any(*aaMask);
        return 1;
    } else if (all(d1Or2)) {
        // Degenerates to a line. Compare p[2] and p[3] to edge 0. If they are on the wrong side,
        // that means edge 0 and 3 crossed, and otherwise edge 1 and 2 crossed.
        if (dists1[2] < kDistTolerance && dists1[3] < kDistTolerance) {
            // Edges 0 and 3 have crossed over, so make the line from average of (p0,p2) and (p1,p3)
            *x2d = 0.5f * (skvx::shuffle<0, 1, 0, 1>(px) + skvx::shuffle<2, 3, 2, 3>(px));
            *y2d = 0.5f * (skvx::shuffle<0, 1, 0, 1>(py) + skvx::shuffle<2, 3, 2, 3>(py));
            // If edges 0 and 3 crossed then one must have AA but we moved both 2D points on the
            // edge so we need moveTo() to be able to move both 3D points along the shared edge. So
            // ensure both have AA.
            *aaMask = *aaMask | mask4({1, 0, 0, 1});
        } else {
            // Edges 1 and 2 have crossed over, so make the line from average of (p0,p1) and (p2,p3)
            *x2d = 0.5f * (skvx::shuffle<0, 0, 2, 2>(px) + skvx::shuffle<1, 1, 3, 3>(px));
            *y2d = 0.5f * (skvx::shuffle<0, 0, 2, 2>(py) + skvx::shuffle<1, 1, 3, 3>(py));
            *aaMask = *aaMask | mask4({0, 1, 1, 0});
        }
        return 2;
    } else {
        // This turns into a triangle. Replace corners as needed with the intersections between
        // (e0,e3) and (e1,e2), which must now be calculated. Because of kDistTolarance we can
        // have cases where the intersection lies far outside the quad. For example, consider top
        // and bottom edges that are nearly parallel and their intersections with the right edge are
        // nearly but not quite swapped (top edge intersection is barely above bottom edge
        // intersection). In this case we replace the point with the average of itself and the point
        // calculated using the edge equation it failed (in the example case this would be the
        // average of the points calculated by the top and bottom edges intersected with the right
        // edge.)
        using V2f = skvx::Vec<2, float>;
        V2f eDenom = skvx::shuffle<0, 1>(fA) * skvx::shuffle<3, 2>(fB) -
                     skvx::shuffle<0, 1>(fB) * skvx::shuffle<3, 2>(fA);
        V2f ex = (skvx::shuffle<0, 1>(fB) * skvx::shuffle<3, 2>(oc) -
                  skvx::shuffle<0, 1>(oc) * skvx::shuffle<3, 2>(fB)) / eDenom;
        V2f ey = (skvx::shuffle<0, 1>(oc) * skvx::shuffle<3, 2>(fA) -
                  skvx::shuffle<0, 1>(fA) * skvx::shuffle<3, 2>(oc)) / eDenom;

        float4 avgX = 0.5f * (skvx::shuffle<0, 1, 0, 2>(px) + skvx::shuffle<2, 3, 1, 3>(px));
        float4 avgY = 0.5f * (skvx::shuffle<0, 1, 0, 2>(py) + skvx::shuffle<2, 3, 1, 3>(py));
        for (int i = 0; i < 4; ++i) {
            // Note that we would not have taken this branch if any point failed both of its edges
            // tests. That is, it can't be the case that d1v0[i] and d2v0[i] are both true.
            if (dists1[i] < -kDistTolerance && std::abs(eDenom[0]) > kTolerance) {
                px[i] = ex[0];
                py[i] = ey[0];
            } else if (d1v0[i]) {
                px[i] = avgX[i % 2];
                py[i] = avgY[i % 2];
            } else if (dists2[i] < -kDistTolerance && std::abs(eDenom[1]) > kTolerance) {
                px[i] = ex[1];
                py[i] = ey[1];
            } else if (d2v0[i]) {
                px[i] = avgX[i / 2 + 2];
                py[i] = avgY[i / 2 + 2];
            }
        }

        // If we replace a vertex with an intersection then it will not fall along the
        // edges that intersect at the original vertex. When we apply AA later to the
        // original points we move along the original 3d edges to move towards the 2d
        // points we're computing here. If we have an AA edge and a non-AA edge we
        // can only move along 1 edge, but now the point we're moving toward isn't
        // on that edge. Thus, we provide an additional degree of freedom by turning
        // AA on for both edges if either edge is AA at each point.
        *aaMask = *aaMask | (d1Or2 & next_cw(*aaMask)) | (next_ccw(d1Or2) & next_ccw(*aaMask));
        *x2d = px;
        *y2d = py;
        return 3;
    }
}

//** OutsetRequest implementation

void TessellationHelper::OutsetRequest::reset(const EdgeVectors& edgeVectors, GrQuad::Type quadType,
                                              const skvx::Vec<4, float>& edgeDistances) {
    fEdgeDistances = edgeDistances;

    // Based on the edge distances, determine if it's acceptable to use fInvSinTheta to
    // calculate the inset or outset geometry.
    if (quadType <= GrQuad::Type::kRectilinear) {
        // Since it's rectangular, the width (edge[1] or edge[2]) collapses if subtracting
        // (dist[0] + dist[3]) makes the new width negative (minus for inset, outsetting will
        // never be degenerate in this case). The same applies for height (edge[0] or edge[3])
        // and (dist[1] + dist[2]).
        fOutsetDegenerate = false;
        float widthChange = edgeDistances[0] + edgeDistances[3];
        float heightChange = edgeDistances[1] + edgeDistances[2];
        // (1/len > 1/(edge sum) implies len - edge sum < 0.
        fInsetDegenerate =
                (widthChange > 0.f  && edgeVectors.fInvLengths[1] > 1.f / widthChange) ||
                (heightChange > 0.f && edgeVectors.fInvLengths[0] > 1.f / heightChange);
    } else if (any(edgeVectors.fInvLengths >= kInvDistTolerance)) {
        // Have an edge that is effectively length 0, so we're dealing with a triangle, which
        // must always go through the degenerate code path.
        fOutsetDegenerate = true;
        fInsetDegenerate = true;
    } else {
        // If possible, the corners will move +/-edgeDistances * 1/sin(theta). The entire
        // request is degenerate if 1/sin(theta) -> infinity (or cos(theta) -> 1).
        if (any(abs(edgeVectors.fCosTheta) >= 0.9f)) {
            fOutsetDegenerate = true;
            fInsetDegenerate = true;
        } else {
            // With an edge-centric view, an edge's length changes by
            // edgeDistance * cos(pi - theta) / sin(theta) for each of its corners (the second
            // corner uses ccw theta value). An edge's length also changes when its adjacent
            // edges move, in which case it's updated by edgeDistance / sin(theta)
            // (or cos(theta) for the other edge).

            // cos(pi - theta) = -cos(theta)
            float4 halfTanTheta = -edgeVectors.fCosTheta * edgeVectors.fInvSinTheta;
            float4 edgeAdjust = edgeDistances * (halfTanTheta + next_ccw(halfTanTheta)) +
                                next_ccw(edgeDistances) * next_ccw(edgeVectors.fInvSinTheta) +
                                next_cw(edgeDistances) * edgeVectors.fInvSinTheta;

            // If either outsetting (plus edgeAdjust) or insetting (minus edgeAdjust) make
            // the edge lengths negative, then it's degenerate.
            float4 threshold = 0.1f - (1.f / edgeVectors.fInvLengths);
            fOutsetDegenerate = any(edgeAdjust < threshold);
            fInsetDegenerate = any(edgeAdjust > -threshold);
        }
    }
}

//** Vertices implementation

void TessellationHelper::Vertices::reset(const GrQuad& deviceQuad, const GrQuad* localQuad) {
    // Set vertices to match the device and local quad
    fX = deviceQuad.x4f();
    fY = deviceQuad.y4f();
    fW = deviceQuad.w4f();

    if (localQuad) {
        fU = localQuad->x4f();
        fV = localQuad->y4f();
        fR = localQuad->w4f();
        fUVRCount = localQuad->hasPerspective() ? 3 : 2;
    } else {
        fUVRCount = 0;
    }
}

void TessellationHelper::Vertices::asGrQuads(GrQuad* deviceOut, GrQuad::Type deviceType,
                                             GrQuad* localOut, GrQuad::Type localType) const {
    SkASSERT(deviceOut);
    SkASSERT(fUVRCount == 0 || localOut);

    fX.store(deviceOut->xs());
    fY.store(deviceOut->ys());
    if (deviceType == GrQuad::Type::kPerspective) {
        fW.store(deviceOut->ws());
    }
    deviceOut->setQuadType(deviceType); // This sets ws == 1 when device type != perspective

    if (fUVRCount > 0) {
        fU.store(localOut->xs());
        fV.store(localOut->ys());
        if (fUVRCount == 3) {
            fR.store(localOut->ws());
        }
        localOut->setQuadType(localType);
    }
}

void TessellationHelper::Vertices::moveAlong(const EdgeVectors& edgeVectors,
                                             const float4& signedEdgeDistances) {
    // This shouldn't be called if fInvSinTheta is close to infinity (cosTheta close to 1).
    // FIXME (michaelludwig) - Temporarily allow NaNs on debug builds here, for crbug:224618's GM
    // Once W clipping is implemented, shouldn't see NaNs unless it's actually time to fail.
    SkASSERT(all(abs(edgeVectors.fCosTheta) < 0.9f) ||
             any(edgeVectors.fCosTheta != edgeVectors.fCosTheta));

    // When the projected device quad is not degenerate, the vertex corners can move
    // cornerOutsetLen along their edge and their cw-rotated edge. The vertex's edge points
    // inwards and the cw-rotated edge points outwards, hence the minus-sign.
    // The edge distances are rotated compared to the corner outsets and (dx, dy), since if
    // the edge is "on" both its corners need to be moved along their other edge vectors.
    float4 signedOutsets = -edgeVectors.fInvSinTheta * next_cw(signedEdgeDistances);
    float4 signedOutsetsCW = edgeVectors.fInvSinTheta * signedEdgeDistances;

    // x = x + outset * mask * next_cw(xdiff) - outset * next_cw(mask) * xdiff
    fX += signedOutsetsCW * next_cw(edgeVectors.fDX) + signedOutsets * edgeVectors.fDX;
    fY += signedOutsetsCW * next_cw(edgeVectors.fDY) + signedOutsets * edgeVectors.fDY;
    if (fUVRCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        signedOutsets *= edgeVectors.fInvLengths;
        signedOutsetsCW *= next_cw(edgeVectors.fInvLengths);
        float4 du = next_ccw(fU) - fU;
        float4 dv = next_ccw(fV) - fV;
        fU += signedOutsetsCW * next_cw(du) + signedOutsets * du;
        fV += signedOutsetsCW * next_cw(dv) + signedOutsets * dv;
        if (fUVRCount == 3) {
            float4 dr = next_ccw(fR) - fR;
            fR += signedOutsetsCW * next_cw(dr) + signedOutsets * dr;
        }
    }
}

void TessellationHelper::Vertices::moveTo(const float4& x2d, const float4& y2d, const mask4& mask) {
    // Left to right, in device space, for each point
    float4 e1x = skvx::shuffle<2, 3, 2, 3>(fX) - skvx::shuffle<0, 1, 0, 1>(fX);
    float4 e1y = skvx::shuffle<2, 3, 2, 3>(fY) - skvx::shuffle<0, 1, 0, 1>(fY);
    float4 e1w = skvx::shuffle<2, 3, 2, 3>(fW) - skvx::shuffle<0, 1, 0, 1>(fW);
    mask4 e1Bad = e1x*e1x + e1y*e1y < kDist2Tolerance;
    correct_bad_edges(e1Bad, &e1x, &e1y, &e1w);

    // // Top to bottom, in device space, for each point
    float4 e2x = skvx::shuffle<1, 1, 3, 3>(fX) - skvx::shuffle<0, 0, 2, 2>(fX);
    float4 e2y = skvx::shuffle<1, 1, 3, 3>(fY) - skvx::shuffle<0, 0, 2, 2>(fY);
    float4 e2w = skvx::shuffle<1, 1, 3, 3>(fW) - skvx::shuffle<0, 0, 2, 2>(fW);
    mask4 e2Bad = e2x*e2x + e2y*e2y < kDist2Tolerance;
    correct_bad_edges(e2Bad, &e2x, &e2y, &e2w);

    // Can only move along e1 and e2 to reach the new 2D point, so we have
    // x2d = (x + a*e1x + b*e2x) / (w + a*e1w + b*e2w) and
    // y2d = (y + a*e1y + b*e2y) / (w + a*e1w + b*e2w) for some a, b
    // This can be rewritten to a*c1x + b*c2x + c3x = 0; a * c1y + b*c2y + c3y = 0, where
    // the cNx and cNy coefficients are:
    float4 c1x = e1w * x2d - e1x;
    float4 c1y = e1w * y2d - e1y;
    float4 c2x = e2w * x2d - e2x;
    float4 c2y = e2w * y2d - e2y;
    float4 c3x = fW * x2d - fX;
    float4 c3y = fW * y2d - fY;

    // Solve for a and b
    float4 a, b, denom;
    if (all(mask)) {
        // When every edge is outset/inset, each corner can use both edge vectors
        denom = c1x * c2y - c2x * c1y;
        a = (c2x * c3y - c3x * c2y) / denom;
        b = (c3x * c1y - c1x * c3y) / denom;
    } else {
        // Force a or b to be 0 if that edge cannot be used due to non-AA
        mask4 aMask = skvx::shuffle<0, 0, 3, 3>(mask);
        mask4 bMask = skvx::shuffle<2, 1, 2, 1>(mask);

        // When aMask[i]&bMask[i], then a[i], b[i], denom[i] match the kAll case.
        // When aMask[i]&!bMask[i], then b[i] = 0, a[i] = -c3x/c1x or -c3y/c1y, using better denom
        // When !aMask[i]&bMask[i], then a[i] = 0, b[i] = -c3x/c2x or -c3y/c2y, ""
        // When !aMask[i]&!bMask[i], then both a[i] = 0 and b[i] = 0
        mask4 useC1x = abs(c1x) > abs(c1y);
        mask4 useC2x = abs(c2x) > abs(c2y);

        denom = if_then_else(aMask,
                        if_then_else(bMask,
                                c1x * c2y - c2x * c1y,            /* A & B   */
                                if_then_else(useC1x, c1x, c1y)),  /* A & !B  */
                        if_then_else(bMask,
                                if_then_else(useC2x, c2x, c2y),   /* !A & B  */
                                float4(1.f)));                    /* !A & !B */

        a = if_then_else(aMask,
                    if_then_else(bMask,
                            c2x * c3y - c3x * c2y,                /* A & B   */
                            if_then_else(useC1x, -c3x, -c3y)),    /* A & !B  */
                    float4(0.f)) / denom;                         /* !A      */
        b = if_then_else(bMask,
                    if_then_else(aMask,
                            c3x * c1y - c1x * c3y,                /* A & B   */
                            if_then_else(useC2x, -c3x, -c3y)),    /* !A & B  */
                    float4(0.f)) / denom;                         /* !B      */
    }

    fX += a * e1x + b * e2x;
    fY += a * e1y + b * e2y;
    fW += a * e1w + b * e2w;

    // If fW has gone negative, flip the point to the other side of w=0. This only happens if the
    // edge was approaching a vanishing point and it was physically impossible to outset 1/2px in
    // screen space w/o going behind the viewer and being mirrored. Scaling by -1 preserves the
    // computed screen space position but moves the 3D point off of the original quad. So far, this
    // seems to be a reasonable compromise.
    if (any(fW < 0.f)) {
        float4 scale = if_then_else(fW < 0.f, float4(-1.f), float4(1.f));
        fX *= scale;
        fY *= scale;
        fW *= scale;
    }

    correct_bad_coords(abs(denom) < kTolerance, &fX, &fY, &fW);

    if (fUVRCount > 0) {
        // Calculate R here so it can be corrected with U and V in case it's needed later
        float4 e1u = skvx::shuffle<2, 3, 2, 3>(fU) - skvx::shuffle<0, 1, 0, 1>(fU);
        float4 e1v = skvx::shuffle<2, 3, 2, 3>(fV) - skvx::shuffle<0, 1, 0, 1>(fV);
        float4 e1r = skvx::shuffle<2, 3, 2, 3>(fR) - skvx::shuffle<0, 1, 0, 1>(fR);
        correct_bad_edges(e1Bad, &e1u, &e1v, &e1r);

        float4 e2u = skvx::shuffle<1, 1, 3, 3>(fU) - skvx::shuffle<0, 0, 2, 2>(fU);
        float4 e2v = skvx::shuffle<1, 1, 3, 3>(fV) - skvx::shuffle<0, 0, 2, 2>(fV);
        float4 e2r = skvx::shuffle<1, 1, 3, 3>(fR) - skvx::shuffle<0, 0, 2, 2>(fR);
        correct_bad_edges(e2Bad, &e2u, &e2v, &e2r);

        fU += a * e1u + b * e2u;
        fV += a * e1v + b * e2v;
        if (fUVRCount == 3) {
            fR += a * e1r + b * e2r;
            correct_bad_coords(abs(denom) < kTolerance, &fU, &fV, &fR);
        } else {
            correct_bad_coords(abs(denom) < kTolerance, &fU, &fV, nullptr);
        }
    }
}

//** TessellationHelper implementation

void TessellationHelper::reset(const GrQuad& deviceQuad, const GrQuad* localQuad) {
    // Record basic state that isn't recorded on the Vertices struct itself
    fDeviceType = deviceQuad.quadType();
    fLocalType = localQuad ? localQuad->quadType() : GrQuad::Type::kAxisAligned;

    // Reset metadata validity
    fOutsetRequestValid = false;
    fEdgeEquationsValid = false;

    // Compute vertex properties that are always needed for a quad, so no point in doing it lazily.
    fOriginal.reset(deviceQuad, localQuad);
    fEdgeVectors.reset(fOriginal.fX, fOriginal.fY, fOriginal.fW, fDeviceType);

    fVerticesValid = true;
}

float4 TessellationHelper::inset(const skvx::Vec<4, float>& edgeDistances,
                                 GrQuad* deviceInset, GrQuad* localInset) {
    SkASSERT(fVerticesValid);

    Vertices inset = fOriginal;
    const OutsetRequest& request = this->getOutsetRequest(edgeDistances);
    int vertexCount;
    if (request.fInsetDegenerate) {
        vertexCount = this->adjustDegenerateVertices(-request.fEdgeDistances, &inset);
    } else {
        this->adjustVertices(-request.fEdgeDistances, &inset);
        vertexCount = 4;
    }

    inset.asGrQuads(deviceInset, fDeviceType, localInset, fLocalType);
    if (vertexCount < 3) {
        // The interior has less than a full pixel's area so estimate reduced coverage using
        // the distance of the inset's projected corners to the original edges.
        return this->getEdgeEquations().estimateCoverage(inset.fX / inset.fW,
                                                         inset.fY / inset.fW);
    } else {
        return 1.f;
    }
}

void TessellationHelper::outset(const skvx::Vec<4, float>& edgeDistances,
                                GrQuad* deviceOutset, GrQuad* localOutset) {
    SkASSERT(fVerticesValid);

    Vertices outset = fOriginal;
    const OutsetRequest& request = this->getOutsetRequest(edgeDistances);
    if (request.fOutsetDegenerate) {
        this->adjustDegenerateVertices(request.fEdgeDistances, &outset);
    } else {
        this->adjustVertices(request.fEdgeDistances, &outset);
    }

    outset.asGrQuads(deviceOutset, fDeviceType, localOutset, fLocalType);
}

void TessellationHelper::getEdgeEquations(skvx::Vec<4, float>* a,
                                          skvx::Vec<4, float>* b,
                                          skvx::Vec<4, float>* c) {
    SkASSERT(a && b && c);
    SkASSERT(fVerticesValid);
    const EdgeEquations& eq = this->getEdgeEquations();
    *a = eq.fA;
    *b = eq.fB;
    *c = eq.fC;
}

skvx::Vec<4, float> TessellationHelper::getEdgeLengths() {
    SkASSERT(fVerticesValid);
    return 1.f / fEdgeVectors.fInvLengths;
}

const TessellationHelper::OutsetRequest& TessellationHelper::getOutsetRequest(
        const skvx::Vec<4, float>& edgeDistances) {
    // Much of the code assumes that we start from positive distances and apply it unmodified to
    // create an outset; knowing that it's outset simplifies degeneracy checking.
    SkASSERT(all(edgeDistances >= 0.f));

    // Rebuild outset request if invalid or if the edge distances have changed.
    if (!fOutsetRequestValid || any(edgeDistances != fOutsetRequest.fEdgeDistances)) {
        fOutsetRequest.reset(fEdgeVectors, fDeviceType, edgeDistances);
        fOutsetRequestValid = true;
    }
    return fOutsetRequest;
}

bool TessellationHelper::isSubpixel() {
    SkASSERT(fVerticesValid);
    if (fDeviceType <= GrQuad::Type::kRectilinear) {
        // Check the edge lengths, if the shortest is less than 1px it's degenerate, which is the
        // same as if the max 1/length is greater than 1px.
        return any(fEdgeVectors.fInvLengths > 1.f);
    } else {
        // Compute edge equations and then distance from each vertex to the opposite edges.
        return this->getEdgeEquations().isSubpixel(fEdgeVectors.fX2D, fEdgeVectors.fY2D);
    }
}

const TessellationHelper::EdgeEquations& TessellationHelper::getEdgeEquations() {
    if (!fEdgeEquationsValid) {
        fEdgeEquations.reset(fEdgeVectors);
        fEdgeEquationsValid = true;
    }
    return fEdgeEquations;
}

void TessellationHelper::adjustVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                        Vertices* vertices) {
    SkASSERT(vertices);
    SkASSERT(vertices->fUVRCount == 0 || vertices->fUVRCount == 2 || vertices->fUVRCount == 3);

    if (fDeviceType < GrQuad::Type::kPerspective) {
        // For non-perspective, non-degenerate quads, moveAlong is correct and most efficient
        vertices->moveAlong(fEdgeVectors, signedEdgeDistances);
    } else {
        // For perspective, non-degenerate quads, use moveAlong for the projected points and then
        // reconstruct Ws with moveTo.
        Vertices projected = { fEdgeVectors.fX2D, fEdgeVectors.fY2D, /*w*/ 1.f, 0.f, 0.f, 0.f, 0 };
        projected.moveAlong(fEdgeVectors, signedEdgeDistances);
        vertices->moveTo(projected.fX, projected.fY, signedEdgeDistances != 0.f);
    }
}

int TessellationHelper::adjustDegenerateVertices(const skvx::Vec<4, float>& signedEdgeDistances,
                                                 Vertices* vertices) {
    SkASSERT(vertices);
    SkASSERT(vertices->fUVRCount == 0 || vertices->fUVRCount == 2 || vertices->fUVRCount == 3);

    if (fDeviceType <= GrQuad::Type::kRectilinear) {
        // For rectilinear, degenerate quads, can use moveAlong if the edge distances are adjusted
        // to not cross over each other.
        SkASSERT(all(signedEdgeDistances <= 0.f)); // Only way rectilinear can degenerate is insets
        float4 halfLengths = -0.5f / next_cw(fEdgeVectors.fInvLengths); // Negate to inset
        mask4 crossedEdges = halfLengths > signedEdgeDistances;
        float4 safeInsets = if_then_else(crossedEdges, halfLengths, signedEdgeDistances);
        vertices->moveAlong(fEdgeVectors, safeInsets);

        // A degenerate rectilinear quad is either a point (both w and h crossed), or a line
        return all(crossedEdges) ? 1 : 2;
    } else {
        // Degenerate non-rectangular shape, must go through slowest path (which automatically
        // handles perspective).
        float4 x2d = fEdgeVectors.fX2D;
        float4 y2d = fEdgeVectors.fY2D;

        mask4 aaMask;
        int vertexCount = this->getEdgeEquations().computeDegenerateQuad(signedEdgeDistances,
                                                                         &x2d, &y2d, &aaMask);
        vertices->moveTo(x2d, y2d, aaMask);
        return vertexCount;
    }
}

} // namespace GrQuadUtils
