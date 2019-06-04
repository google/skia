/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrQuadUtils.h"

#include "include/core/SkRect.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkVx.h"
#include "src/gpu/geometry/GrQuad.h"

using V4f = skvx::Vec<4, float>;
using M4f = skvx::Vec<4, int32_t>;

// Since the local quad may not be type kRect, this uses the opposites for each vertex when
// interpolating, and calculates new ws in addition to new xs, ys.
static void interpolate_local(float alpha, int v0, int v1, int v2, int v3,
                              float lx[4], float ly[4], float lw[4]) {
    float beta = 1.f - alpha;
    lx[v0] = alpha * lx[v0] + beta * lx[v2];
    ly[v0] = alpha * ly[v0] + beta * ly[v2];
    lw[v0] = alpha * lw[v0] + beta * lw[v2];

    lx[v1] = alpha * lx[v1] + beta * lx[v3];
    ly[v1] = alpha * ly[v1] + beta * ly[v3];
    lw[v1] = alpha * lw[v1] + beta * lw[v3];
}

// Crops v0 to v1 based on the device rect. v2 is opposite of v0, v3 is opposite of v1.
// It is written to not modify coordinates if there's no intersection along the edge.
// Ideally this would have been detected earlier and the entire draw is skipped.
static bool crop_rect_edge(const SkRect& clipDevRect, int v0, int v1, int v2, int v3,
                           float x[4], float y[4], float lx[4], float ly[4], float lw[4]) {
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

// Updates x and y to intersect with clipDevRect, and applies clipAA policy to edgeFlags for each
// intersected edge. lx, ly, and lw are updated appropriately and may be null to skip calculations.
static void crop_rect(const SkRect& clipDevRect, GrAA clipAA, GrQuadAAFlags* edgeFlags,
                      float x[4], float y[4], float lx[4], float ly[4], float lw[4]) {
    // Filled in as if clipAA were true, will be inverted at the end if needed.
    GrQuadAAFlags clipEdgeFlags = GrQuadAAFlags::kNone;

    // However, the quad's left edge may not align with the SkRect notion of left due to 90 degree
    // rotations or mirrors. So, this processes the logical edges of the quad and clamps it to the 4
    // sides of clipDevRect.

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

    if (clipAA == GrAA::kYes) {
        // Turn on all edges that were clipped
        *edgeFlags |= clipEdgeFlags;
    } else {
        // Turn off all edges that were clipped
        *edgeFlags &= ~clipEdgeFlags;
    }
}

// Calculates barycentric coordinates for each point in (testX, testY) in the triangle formed by
// (x0,y0) - (x1,y1) - (x2, y2) and stores them in u, v, w.
static void barycentric_coords(float x0, float y0, float x1, float y1, float x2, float y2,
                               const V4f& testX, const V4f& testY,
                               V4f* u, V4f* v, V4f* w) {
    // Modeled after SkPathOpsQuad::pointInTriangle() but uses float instead of double, is
    // vectorized and outputs normalized barycentric coordinates instead of inside/outside test
    float v0x = x2 - x0;
    float v0y = y2 - y0;
    float v1x = x1 - x0;
    float v1y = y1 - y0;
    V4f v2x = testX - x0;
    V4f v2y = testY - y0;

    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    V4f   dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    V4f   dot12 = v1x * v2x + v1y * v2y;
    float invDenom = sk_ieee_float_divide(1.f, dot00 * dot11 - dot01 * dot01);
    *u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    *v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    *w = 1.f - *u - *v;
}

static M4f inside_triangle(const V4f& u, const V4f& v, const V4f& w) {
    return (u >= 0.f & u <= 1.f) & (v >= 0.f & v <= 1.f) & (w >= 0.f & v <= 1.f);
}

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
                // Turn off anti-aliasing
                *outAAType = GrAAType::kNone;
            } else {
                // For coverage AA, if the quad is a rect and it lines up with pixel boundaries
                // then overall aa and per-edge aa can be completely disabled
                if (quad.quadType() == GrQuad::Type::kAxisAligned && !quad.aaHasEffectOnRect()) {
                    *outAAType = GrAAType::kNone;
                    *outEdgeFlags = GrQuadAAFlags::kNone;
                }
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
        case GrAAType::kMixedSamples:
            SK_ABORT("Should not use mixed sample AA with edge AA flags");
            break;
    }
}

VertexHelper::VertexHelper(const GrQuad& deviceQuad, const GrQuad* localQuad)
        : fOriginalVertices(deviceQuad, localQuad)
        , fDeviceType(deviceQuad.quadType())
        , fProjectedCornersValid(false)
        , fEdgeVectorsValid(false)
        , fEdgeEquationsValid(false)
        , fOutsetInfoValid(false) { }

VertexHelper::Vertices::Vertices(const GrQuad& deviceQuad, const GrQuad* localQuad)
        : fX(deviceQuad.x4f())
        , fY(deviceQuad.y4f())
        , fW(deviceQuad.w4f())
        , fUVRCount(localQuad ? (localQuad->hasPerspective() ? 3 : 2) : 0) {
    if (localQuad) {
        fU = localQuad->x4f();
        fV = localQuad->y4f();
        fR = localQuad->w4f();
    }
}

// FIXME must bring in fma, nextCW, nextCCW, kTolerance

// FIXME must update calculation code to not just assume original edge distances are 0.5
// (that shgould just be in get_optimized_outset and in the c coeff adjustment done by compute_degenerate_quad)
// FIXME update get_exact_coverage math to use exact rectangle instead of pixel edges

void VertexHelper::Vertices::moveTo() {
    // this should be optimize_projected_vertices
}

void VertexHelper::Vertices::moveAlong() {
    // this should be optimize_vertices
}

int VertexHelper::areaOfIntersection(const SkRect& deviceRect) {
    // this should be get_exact_coverage
}

// FIXME maybe remove if only edgeVectors() and projectedVertices() needs this?
void VertexHelper::ensureProjectedCoords() {
    if (!fProjectedCornersValid) {
        if (fDeviceType == GrQuad::Type::kPerspective) {
            V4f iw = 1.0f / fOriginalVertices.fW;
            fX2D = fOriginalVertices.fX * iw;
            fY2D = fOriginalVertices.fY * iw;
        } else {
            fX2D = fOriginalVertices.fX;
            fY2D = fOriginalVertices.fY;
        }
        fProjectedCornersValid = true;
    }
}

// FIXME maybe remove if only adjustQuad needs this?
VertexHelper::Vertices VertexHelper::projectedVertices() {

}

const VertexHelper::EdgeVectors& VertexHelper::edgeVectors() {
    if (!fEdgeVectorsValid) {
        this->ensureProjectedCoords();

        fEdgeVectors.fDX = nextCCW(fX2D) - fX2D;
        fEdgeVectors.fDY = nextCCW(fY2D) - fY2D;
        fEdgeVectors.fInvLengths = rsqrt(fma(fEdgeVectors.fDX, fEdgeVectors.fDX,
                                             fEdgeVectors.fDY * fEdgeVectors.fDY));
        // Normalize dx and dy
        fEdgeVectors.fDX *= fEdgeVectors.fInvLengths;
        fEdgeVectors.fDY *= fEdgeVectors.fInvLengths;

    }
    return fEdgeVectors;
}

const VertexHelper::EdgeEquations& VertexHelper::edgeEquations() {
    if (!fEdgeEquationsValid) {
        const EdgeVectors& ev = this->edgeVectors();
        SkASSERT(fProjectedCornersValid); // should have been cached by edgeVectors()

        // Correct for bad edges by copying adjacent edge information into the bad component
        V4f dx = ev.fDX;
        V4f dy = ev.fDY;
        correct_bad_edges(ev.fInvLengths >= 1.f / kTolerance, &dx, &dy, nullptr);

        fEdgeEquations.fC = fma(dx, fY2D, -dy * fX2D);
        // Make sure normals point into the shape
        V4f test = fma(dy, nextCW(fX2D), fma(-dx, nextCW(fY2D), c));
        if (any(test < -kTolerance)) {
            fEdgeEquations.fA = -dy;
            fEdgeEquations.fB = dx;
            fEdgeEquations.fC *= -1.f;
            fEdgeEquations.fFlipped = true;
        } else {
            fEdgeEquations.fA = dy;
            fEdgeEquations.fB = -dx;
            fEdgeEquations.fFlipped = false;
        }
        fEdgeEquationsValid = true;
    }
    return fEdgeEquations;
}

const VertexHelper::OutsetInfo& VertexHelper::outsetInfo(const V4f& edgeDistances) {
    if (!fOutsetInfoValid || any(fOutsetInfo.fEdgeDistances != edgeDistances)) {
        // this should be get_optimized_outset
    }
    return fOutsetInfo;
}

int VertexHelper::calculateProjectedQuad(const V4f& signedEdgeDistances, const V4f& mask,
                                         V4f* x2d, V4f* y2d) const {
    // this should be compute_degenerate_quad()
}

int VertexHelper::adjustQuad(const V4f& edgeDistances, bool outset,
                             GrQuad* deviceOut, GrQuad* localOut) {
    SkASSERT(localOut || fOriginalVertices.fUVRCount == 0);
    const OutsetInfo& outsets = this->outsetInfo(edgeDistances);

    // Get signed outsets after querying cached outset info (which is unsigned)
    V4f signedOutsets = outsets.fOutsets;
    if (!outset) {
        signedOutsets *= -1.f;
    }

    // Start with original coordinates and update in place
    Vertices adjusted = fOriginalVertices;
    int vertexCount = 4;

    if (fDeviceType == GrQuad::Type::kPerspective || outsets.fDegenerate) {
        this->ensureProjectedCoords();
        V4f x2d = fX2D;
        V4f y2d = fY2D;
        if (outsets.fDegenerate) {
            // In this case, signedOutsets will be equal to the signed edge distances
            vertexCount = this->calculateDegenerateQuad(signedOutsets, outsets.fMask, &x2d, &y2d);
        } else {
            // Only reach here in perspective, but the 2D projection is not degenerate so it can
            // at least be calculated quickly, albeit no point in local coords yet
            Vertices adjusted2D = { fX2D, fY2D, /*w*/ 1.f, 0.f, 0.f, 0.f, 0 }; // No local outsetting
            adjusted2D.moveAlong(this->edgeVectors(), signedOutsets, outsets.fMask);
            x2d = adjusted2D.fX;
            y2d = adjsuted2D.fY;
        }

        // Must use slow move function to update local coords to match 2D points (either we're
        // bringing them back into perspective, or interpolating them to match degenerate
        // intersections found by calculate)
        adjusted.moveTo(x2d, y2d, outsets.fMask);
    } else {
        // Quad starts 2D and outsets are not complicated, so we can directly adjust everything
        // as a fast path
        adjusted.moveAlong(this->edgeVectors(), signedOutsets, outsets.fMask);
    }

    // Push the state of adjusted into deviceOut and localOut
    adjusted.fX.store(deviceOut->fX);
    adjusted.fY.store(deviceOut->fY);
    adjusted.fW.store(deviceOut->fW); // FIXME only read from vertices fW if perspective, so would set deviceOut fW to 1s
    deviceOut->fType = fDeviceType;

    if (localOut && adjusted.fUVRCount > 0) {
        // fIXMe only read from uvr based on uvrcount
        adjusted.fU.store(localOut->fX);
        adjusted.fV.store(localOut->fY);
        adjusted.fR.store(localOut->fW);
        // FIXME what quad type to set? will it always be the same as the original local quad type? I'd think so
        localOut->fType = fLocalType;
    }

    return vertexCount;
}

bool CropToRect(const SkRect& cropRect, GrAA clipAA, GrQuadAAFlags* edgeFlags, GrQuad* device,
                GrQuad* local) {
    if (device->quadType() == GrQuad::Type::kAxisAligned) {
        // crop_rect keeps the rectangles as rectangles, so there's no need to modify types
        if (local) {
            crop_rect(cropRect, clipAA, edgeFlags, device->xs(), device->ys(),
                      local->xs(), local->ys(), local->ws());
        } else {
            crop_rect(cropRect, clipAA, edgeFlags, device->xs(), device->ys(),
                      nullptr, nullptr, nullptr);
        }
        return true;
    }

    if (local) {
        // FIXME (michaelludwig) Calculate cropped local coordinates when not kAxisAligned
        return false;
    }

    V4f devX = device->x4f();
    V4f devY = device->y4f();
    V4f devIW = device->iw4f();
    // Project the 3D coordinates to 2D
    if (device->quadType() == GrQuad::Type::kPerspective) {
        devX *= devIW;
        devY *= devIW;
    }

    V4f clipX = {cropRect.fLeft, cropRect.fLeft, cropRect.fRight, cropRect.fRight};
    V4f clipY = {cropRect.fTop, cropRect.fBottom, cropRect.fTop, cropRect.fBottom};

    // Calculate barycentric coordinates for the 4 rect corners in the 2 triangles that the quad
    // is tessellated into when drawn.
    V4f u1, v1, w1;
    barycentric_coords(devX[0], devY[0], devX[1], devY[1], devX[2], devY[2], clipX, clipY,
                       &u1, &v1, &w1);
    V4f u2, v2, w2;
    barycentric_coords(devX[1], devY[1], devX[3], devY[3], devX[2], devY[2], clipX, clipY,
                       &u2, &v2, &w2);

    // clipDevRect is completely inside this quad if each corner is in at least one of two triangles
    M4f inTri1 = inside_triangle(u1, v1, w1);
    M4f inTri2 = inside_triangle(u2, v2, w2);
    if (all(inTri1 | inTri2)) {
        // We can crop to exactly the clipDevRect.
        // FIXME (michaelludwig) - there are other ways to have determined quad covering the clip
        // rect, but the barycentric coords will be useful to derive local coordinates in the future

        // Since we are cropped to exactly clipDevRect, we have discarded any perspective and the
        // type becomes kRect. If updated locals were requested, they will incorporate perspective.
        // FIXME (michaelludwig) - once we have local coordinates handled, it may be desirable to
        // keep the draw as perspective so that the hardware does perspective interpolation instead
        // of pushing it into a local coord w and having the shader do an extra divide.
        clipX.store(device->xs());
        clipY.store(device->ys());
        device->ws()[0] = 1.f;
        device->ws()[1] = 1.f;
        device->ws()[2] = 1.f;
        device->ws()[3] = 1.f;
        device->setQuadType(GrQuad::Type::kAxisAligned);

        // Update the edge flags to match the clip setting since all 4 edges have been clipped
        *edgeFlags = clipAA == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;

        return true;
    }

    // FIXME (michaelludwig) - use the GrQuadPerEdgeAA tessellation inset/outset math to move
    // edges to the closest clip corner they are outside of

    return false;
}

}; // namespace GrQuadUtils
