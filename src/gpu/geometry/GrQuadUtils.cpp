/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrQuadUtils.h"

#include "include/private/GrTypesPriv.h"

using V4f = skvx::Vec<4, float>;

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
};

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

}; // namespace GrQuadUtils
