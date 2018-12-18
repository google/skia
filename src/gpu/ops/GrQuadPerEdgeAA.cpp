/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuadPerEdgeAA.h"
#include "GrQuad.h"
#include "GrVertexWriter.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "SkNx.h"

#define AI SK_ALWAYS_INLINE

namespace {

static AI Sk4f fma(const Sk4f& f, const Sk4f& m, const Sk4f& a) {
    return SkNx_fma<4, float>(f, m, a);
}

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
static AI Sk4f nextCW(const Sk4f& v) {
    return SkNx_shuffle<2, 0, 3, 1>(v);
}

static AI Sk4f nextCCW(const Sk4f& v) {
    return SkNx_shuffle<1, 3, 0, 2>(v);
}

// Fills Sk4f with 1f if edge bit is set, 0f otherwise. Edges are ordered LBTR to match CCW ordering
// of vertices in the quad.
static AI Sk4f compute_edge_mask(GrQuadAAFlags aaFlags) {
    return Sk4f((GrQuadAAFlags::kLeft & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kBottom & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kTop & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kRight & aaFlags) ? 1.f : 0.f);
}

// Outputs normalized edge vectors in xdiff and ydiff, as well as the reciprocal of the original
// edge lengths in invLengths
static AI void compute_edge_vectors(const Sk4f& x, const Sk4f& y, const Sk4f& xnext,
                                    const Sk4f& ynext, Sk4f* xdiff, Sk4f* ydiff, Sk4f* invLengths) {
    *xdiff = xnext - x;
    *ydiff = ynext - y;
    *invLengths = fma(*xdiff, *xdiff, *ydiff * *ydiff).rsqrt();
    *xdiff *= *invLengths;
    *ydiff *= *invLengths;
}

static AI void outset_masked_vertices(const Sk4f& xdiff, const Sk4f& ydiff, const Sk4f& invLengths,
                                      const Sk4f& mask, Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r,
                                      int uvrCount) {
    auto halfMask = 0.5f * mask;
    auto maskCW = nextCW(halfMask);
    *x += maskCW * -xdiff + halfMask * nextCW(xdiff);
    *y += maskCW * -ydiff + halfMask * nextCW(ydiff);
    if (uvrCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        maskCW *= invLengths;
        halfMask *= nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += maskCW * -udiff + halfMask * nextCW(udiff);
        *v += maskCW * -vdiff + halfMask * nextCW(vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += maskCW * -rdiff + halfMask * nextCW(rdiff);
        }
    }
}

static AI void outset_vertices(const Sk4f& xdiff, const Sk4f& ydiff, const Sk4f& invLengths,
                               Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    *x += 0.5f * (-xdiff + nextCW(xdiff));
    *y += 0.5f * (-ydiff + nextCW(ydiff));
    if (uvrCount > 0) {
        Sk4f t = 0.5f * invLengths;
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += t * -udiff + nextCW(t) * nextCW(udiff);
        *v += t * -vdiff + nextCW(t) * nextCW(vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += t * -rdiff + nextCW(t) * nextCW(rdiff);
        }
    }
}

static AI void compute_edge_distances(const Sk4f& a, const Sk4f& b, const Sk4f& c, const Sk4f& x,
                                      const Sk4f& y, const Sk4f& w, Sk4f edgeDistances[]) {
    for (int i = 0; i < 4; ++i) {
        edgeDistances[i] = a * x[i] + b * y[i] + c * w[i];
    }
}

// This computes the four edge equations for a quad, then outsets them and optionally computes a new
// quad as the intersection points of the outset edges. 'x' and 'y' contain the original points as
// input and the outset points as output. In order to be used as a component of perspective edge
// distance calculation, this exports edge equations in 'a', 'b', and 'c'. Use
// compute_edge_distances to turn these equations into the distances needed by the shader. The
// values in x, y, u, v, and r are possibly updated if outsetting is needed. r is the local
// position's w component if it exists.
static void compute_quad_edges_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
        Sk4f* a, Sk4f* b, Sk4f* c, Sk4f* u, Sk4f* v, Sk4f* r, int uvrChannelCount, bool outset) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);

    // Compute edge vectors for the quad.
    auto xnext = nextCCW(*x);
    auto ynext = nextCCW(*y);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(*x, *y, xnext, ynext, &xdiff, &ydiff, &invLengths);

    // Use above vectors to compute edge equations (importantly before we outset positions).
    *c = fma(xnext, *y,  -ynext * *x) * invLengths;
    // Make sure the edge equations have their normals facing into the quad in device space.
    auto test = fma(ydiff, nextCW(*x), fma(-xdiff, nextCW(*y), *c));
    if ((test < Sk4f(0)).anyTrue()) {
        *a = -ydiff;
        *b = xdiff;
        *c = -*c;
    } else {
        *a = ydiff;
        *b = -xdiff;
    }
    // Outset the edge equations so aa coverage evaluates to zero half a pixel away from the
    // original quad edge.
    *c += 0.5f;

    if (aaFlags != GrQuadAAFlags::kAll) {
        // This order is the same order the edges appear in xdiff/ydiff and therefore as the
        // edges in a/b/c.
        Sk4f mask = compute_edge_mask(aaFlags);

        // Outset edge equations for masked out edges another pixel so that they always evaluate
        // >= 1.
        *c += (1.f - mask);
        if (outset) {
            outset_masked_vertices(xdiff, ydiff, invLengths, mask, x, y, u, v, r, uvrChannelCount);
        }
    } else if (outset) {
        outset_vertices(xdiff, ydiff, invLengths, x, y, u, v, r, uvrChannelCount);
    }
}

// A specialization of the above function that can compute edge distances very quickly when it knows
// that the edges intersect at right angles, i.e. any transform other than skew and perspective
// (GrQuadType::kRectilinear). Unlike the above function, this always outsets the corners since it
// cannot be reused in the perspective case.
static void compute_rectilinear_dists_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x,
        Sk4f* y,  Sk4f edgeDistances[4], Sk4f* u, Sk4f* v, Sk4f* r, int uvrChannelCount) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(*x, *y, nextCCW(*x), nextCCW(*y), &xdiff, &ydiff, &invLengths);
    Sk4f lengths = invLengths.invert();

    // Since the quad is rectilinear, the edge distances are predictable and independent of the
    // actual orientation of the quad. The lengths vector stores |p1-p0|, |p3-p1|, |p0-p2|, |p2-p3|,
    // matching the CCW order. For instance, edge distances for p0 are 0 for e0 and e2 since they
    // intersect at p0. Distance to e1 is the same as p0 to p1. Distance to e3 is p0 to p2 since
    // e3 goes through p2 and since the quad is rectilinear, we know that's the shortest distance.
    edgeDistances[0] = Sk4f(0.f, lengths[0], 0.f, lengths[2]);
    edgeDistances[1] = Sk4f(0.f, 0.f, lengths[0], lengths[1]);
    edgeDistances[2] = Sk4f(lengths[2], lengths[3], 0.f, 0.f);
    edgeDistances[3] = Sk4f(lengths[1], 0.f, lengths[3], 0.f);

    if (aaFlags != GrQuadAAFlags::kAll) {
        // This order is the same order the edges appear in xdiff/ydiff and therefore as the
        // edges in a/b/c.
        Sk4f mask = compute_edge_mask(aaFlags);

        // Update opposite corner distances by 1 (when enabled by the mask). The distance
        // calculations used in compute_quad_edges_... calculates the edge equations from original
        // positions and then shifts the coefficient by 0.5. If the opposite edges are also outset
        // then must add an additional 0.5 to account for its shift away from that edge.
        Sk4f maskWithOpposites = mask + SkNx_shuffle<3, 2, 1, 0>(mask);
        edgeDistances[0] += Sk4f(0.f, 0.5f, 0.f, 0.5f) * maskWithOpposites;
        edgeDistances[1] += Sk4f(0.f, 0.f, 0.5f, 0.5f) * maskWithOpposites;
        edgeDistances[2] += Sk4f(0.5f, 0.5f, 0.f, 0.f) * maskWithOpposites;
        edgeDistances[3] += Sk4f(0.5f, 0.f, 0.5f, 0.f) * maskWithOpposites;

        // Outset edge equations for masked out edges another pixel so that they always evaluate
        // So add 1-mask to each point's edge distances vector so that coverage >= 1 on non-aa
        for (int i = 0; i < 4; ++i) {
            edgeDistances[i] += (1.f - mask);
        }
        outset_masked_vertices(xdiff, ydiff, invLengths, mask, x, y, u, v, r, uvrChannelCount);
    } else {
        // Update opposite corner distances by 0.5 pixel and 0.5 edge shift, skipping the need for
        // mask since that's 1s
        edgeDistances[0] += Sk4f(0.f, 1.f, 0.f, 1.f);
        edgeDistances[1] += Sk4f(0.f, 0.f, 1.f, 1.f);
        edgeDistances[2] += Sk4f(1.f, 1.f, 0.f, 0.f);
        edgeDistances[3] += Sk4f(1.f, 0.f, 1.f, 0.f);

        outset_vertices(xdiff, ydiff, invLengths, x, y, u, v, r, uvrChannelCount);
    }
}

// Generalizes compute_quad_edge_distances_and_outset_vertices to extrapolate local coords such that
// after perspective division of the device coordinate, the original local coordinate value is at
// the original un-outset device position. r is the local coordinate's w component.
static void compute_quad_dists_and_outset_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x,
        Sk4f* y, Sk4f* w, Sk4f edgeDistances[4], Sk4f* u, Sk4f* v, Sk4f* r, int uvrChannelCount) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;
    Sk4f a, b, c;
    // Don't compute outset corners in the normalized space, which means u, v, and r don't need
    // to be provided here (outset separately below). Since this is computing distances for a
    // projected quad, there is a very good chance it's not rectilinear so use the general 2D path.
    compute_quad_edges_and_outset_vertices(aaFlags, &x2d, &y2d, &a, &b, &c,
            nullptr, nullptr, nullptr, /* uvr ct */ 0, /* outsetCorners */ false);

    static const float kOutset = 0.5f;
    if ((GrQuadAAFlags::kLeft | GrQuadAAFlags::kRight) & aaFlags) {
        // For each entry in x the equivalent entry in opX is the left/right opposite and so on.
        Sk4f opX = SkNx_shuffle<2, 3, 0, 1>(*x);
        Sk4f opW = SkNx_shuffle<2, 3, 0, 1>(*w);
        Sk4f opY = SkNx_shuffle<2, 3, 0, 1>(*y);
        // vx/vy holds the device space left-to-right vectors along top and bottom of the quad.
        Sk2f vx = SkNx_shuffle<2, 3>(x2d) - SkNx_shuffle<0, 1>(x2d);
        Sk2f vy = SkNx_shuffle<2, 3>(y2d) - SkNx_shuffle<0, 1>(y2d);
        Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();
        // For each device space corner, devP, label its left/right opposite device space point
        // opDevPt. The new device space point is opDevPt + s (devPt - opDevPt) where s is
        // (length(devPt - opDevPt) + 0.5) / length(devPt - opDevPt);
        Sk4f s = SkNx_shuffle<0, 1, 0, 1>((len + kOutset) / len);
        // Compute t in homogeneous space from s using similar triangles so that we can produce
        // homogeneous outset vertices for perspective-correct interpolation.
        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));
        // mask is used to make the t values be 1 when the left/right side is not antialiased.
        Sk4f mask(GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrChannelCount > 0) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrChannelCount == 3) {
                Sk4f opR = SkNx_shuffle<2, 3, 0, 1>(*r);
                *r = opR + t * (*r - opR);
            }
        }

        if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
            // Update the 2D points for the top/bottom calculation.
            iw = (*w).invert();
            x2d = (*x) * iw;
            y2d = (*y) * iw;
        }
    }

    if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
        // This operates the same as above but for top/bottom rather than left/right.
        Sk4f opX = SkNx_shuffle<1, 0, 3, 2>(*x);
        Sk4f opW = SkNx_shuffle<1, 0, 3, 2>(*w);
        Sk4f opY = SkNx_shuffle<1, 0, 3, 2>(*y);

        Sk2f vx = SkNx_shuffle<1, 3>(x2d) - SkNx_shuffle<0, 2>(x2d);
        Sk2f vy = SkNx_shuffle<1, 3>(y2d) - SkNx_shuffle<0, 2>(y2d);
        Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();

        Sk4f s = SkNx_shuffle<0, 0, 1, 1>((len + kOutset) / len);

        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));

        Sk4f mask(GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrChannelCount > 0) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrChannelCount == 3) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }

    // Use the original edge equations with the outset homogeneous coordinates to get the edge
    // distance (technically multiplied by w, so that the fragment shader can do perspective
    // interpolation when it multiplies by 1/w later).
    compute_edge_distances(a, b, c, *x, *y, *w, edgeDistances);
}

// Calculate safe edge distances for non-aa quads that have been batched with aa quads. Since the
// fragment shader multiples by 1/w, so the edge distance cannot just be set to 1. It cannot just
// be set to w either due to interpolation across the triangle. If iA, iB, and iC are the
// barycentric weights of the triangle, and we set the edge distance to w, the fragment shader
// actually sees d = (iA*wA + iB*wB + iC*wC) * (iA/wA + iB/wB + iC/wC). Without perspective this
// simplifies to 1 as necessary, but we must choose something other than w when there is perspective
// to ensure that d >= 1 and the edge shows as non-aa.
static void compute_nonaa_edge_distances(const Sk4f& w, bool hasPersp, Sk4f edgeDistances[4]) {
    // Let n = min(w1,w2,w3,w4) and m = max(w1,w2,w3,w4) and rewrite
    //   d = (iA*wA + iB*wB + iC*wC) * (iA*wB*wC + iB*wA*wC + iC*wA*wB) / (wA*wB*wC)
    //       |   e=attr from VS    |   |         fragCoord.w = 1/w                 |
    // Since the weights are the interior of the primitive then we have:
    //   n <= (iA*wA + iB*wB + iC*wC) <= m and
    //   n^2 <= (iA*wB*wC + iB*wA*wC + iC*wA*wB) <= m^2 and
    //   n^3 <= wA*wB*wC <= m^3 regardless of the choice of A, B, and C verts in the quad
    // Thus if we set e = m^3/n^3, it guarantees d >= 1 for any perspective. However, the vertex
    // shader that calculates max coverage for a pixel subtracts 1 from the edge distances to
    // account for the outsetting done for AA quads, thus we return 2*e to make sure it's > 1 after
    // the subtraction.
    float e;
    if (hasPersp) {
        float m = w.max();
        float n = w.min();
        e = 2.f * (m * m * m) / (n * n * n);
    } else {
        e = 2.f;
    }

    // All edge distances set to the same
    for (int i = 0; i < 4; ++i) {
        edgeDistances[i] = e;
    }
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

////////////////// Tessellate Implementation

void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                 const SkPMColor4f& color4f, const GrPerspQuad& localQuad, const SkRect& domain,
                 GrQuadAAFlags aaFlags) {
    bool deviceHasPerspective = spec.deviceQuadType() == GrQuadType::kPerspective;
    bool localHasPerspective = spec.localQuadType() == GrQuadType::kPerspective;
            // The internals of the packed edge distance shader variant require the color values to
        // be unpremul'ed on the CPU so that when it modulates alpha and coverage together, the
        // alpha is not doubly-applied to the RGB components.
    bool wide = GrQuadPerEdgeAA::ColorType::kHalf == spec.colorType();
    GrVertexColor color = spec.packEdgeDistances() ? GrVertexColor(color4f.unpremul(), wide) :
                                                     GrVertexColor(color4f, wide);

    // Load position data into Sk4fs (always x, y, and load w to avoid branching down the road)
    Sk4f x = deviceQuad.x4f();
    Sk4f y = deviceQuad.y4f();
    Sk4f w = deviceQuad.w4f(); // Guaranteed to be 1f if it's not perspective

    // Load local position data into Sk4fs (either none, just u,v or all three)
    Sk4f u, v, r;
    if (spec.hasLocalCoords()) {
        u = localQuad.x4f();
        v = localQuad.y4f();

        if (localHasPerspective) {
            r = localQuad.w4f();
        }
    }

    // Index into array refers to vertex. Index into particular Sk4f refers to edge.
    Sk4f edgeDistances[4];
    if (spec.usesCoverageAA()) {
        // Must calculate edges and possibly outside the positions
        if (aaFlags == GrQuadAAFlags::kNone) {
            // A non-AA quad that got batched into an AA group, so it should have full coverage
            compute_nonaa_edge_distances(w, deviceHasPerspective, edgeDistances);
        } else if (deviceHasPerspective) {
            // For simplicity, pointers to u, v, and r are always provided, but the local dim param
            // ensures that only loaded Sk4fs are modified in the compute functions.
            compute_quad_dists_and_outset_persp_vertices(aaFlags, &x, &y, &w, edgeDistances,
                    &u, &v, &r, spec.localDimensionality());
        } else if (spec.deviceQuadType() <= GrQuadType::kRectilinear) {
            compute_rectilinear_dists_and_outset_vertices(aaFlags, &x, &y, edgeDistances,
                    &u, &v, &r, spec.localDimensionality());
        } else {
            Sk4f a, b, c;
            compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c, &u, &v, &r,
                                                   spec.localDimensionality(), /*outset*/ true);
            compute_edge_distances(a, b, c, x, y, w, edgeDistances); // w holds 1.f as desired
        }
    }

    // Now rearrange the Sk4fs into the interleaved vertex layout:
    //  i.e. x1x2x3x4 y1y2y3y4 -> x1y1 x2y2 x3y3 x4y
    GrVertexWriter vb{vertices};
    uint32_t halfDists[2];
    for (int i = 0; i < 4; ++i) {
        // save position
        if (deviceHasPerspective) {
            vb.write(x[i], y[i], w[i]);
        } else {
            vb.write(x[i], y[i]);
        }

        // save color
        if (spec.hasVertexColors()) {
            vb.write(color);
        }

        // save local position
        if (spec.hasLocalCoords()) {
            if (localHasPerspective) {
                vb.write<SkPoint3>({u[i], v[i], r[i]});
            } else {
                vb.write<SkPoint>({u[i], v[i]});
            }
        }

        // save the domain
        if (spec.hasDomain()) {
            vb.write(domain);
        }

        // save the edges
        if (spec.usesCoverageAA()) {
            if (spec.packEdgeDistances()) {
                // 65504 is the smallest of the boundary values that round to infinity, depending
                // on the round mode. Quads this big should be detected and handled without packed
                // edge distances.
                SkASSERT(edgeDistances[i].max() < 65504);
                SkFloatToHalf_finite_ftz(edgeDistances[i]).store(&halfDists);
                vb.write(halfDists[0], halfDists[1]);
            } else {
                vb.write(edgeDistances[i]);
            }
        }
    }

    return vb.fPtr;
}

////////////////// VertexSpec Implementation

int VertexSpec::deviceDimensionality() const {
    return this->deviceQuadType() == GrQuadType::kPerspective ? 3 : 2;
}

int VertexSpec::localDimensionality() const {
    return fHasLocalCoords ? (this->localQuadType() == GrQuadType::kPerspective ? 3 : 2) : 0;
}

////////////////// Geometry Processor Implementation

class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

public:

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& spec) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(spec));
    }

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& vertexSpec, const GrShaderCaps& caps,
                                           GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState& samplerState,
                                           uint32_t extraSamplerKey,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                vertexSpec, caps, textureType, textureConfig, samplerState, extraSamplerKey,
                std::move(textureColorSpaceXform)));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // aa, domain, texturing are single bit flags
        uint32_t x = fAAEdgeDistances.isInitialized() ? 0 : 1;
        x |= fDomain.isInitialized() ? 0 : 2;
        x |= fSampler.isInitialized() ? 0 : 4;
        // regular position has two options as well
        x |= kFloat3_GrVertexAttribType == fPosition.cpuType() ? 0 : 8;
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 16 : 32;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 64 : 128;
        }
        // shaders handle edge distances differently if they are packed
        if (fAAEdgeDistances.isInitialized()) {
            x |= kHalf4_GrVertexAttribType == fAAEdgeDistances.cpuType() ? 0 : 256;
        }

        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(x);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& gp = proc.cast<QuadPerEdgeAAGeometryProcessor>();
                if (gp.fLocalCoord.isInitialized()) {
                    this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                }
                fTextureColorSpaceXformHelper.setData(pdman, gp.fTextureColorSpaceXform.get());
            }

        private:
            void emitMaxCoverage(const QuadPerEdgeAAGeometryProcessor& gp, EmitArgs& args,
                                 const char* outputName) {
                if (gp.fPosition.gpuType() == kFloat3_GrSLType) {
                    args.fVertBuilder->codeAppendf(
                            "%s = min(1.0, max(%s.x, %s.w) / %s.z - 1) * "
                            "min(1.0, max(%s.y, %s.z) / %s.z - 1);",
                            outputName, gp.fAAEdgeDistances.name(), gp.fAAEdgeDistances.name(),
                            gp.fPosition.name(), gp.fAAEdgeDistances.name(),
                            gp.fAAEdgeDistances.name(), gp.fPosition.name());
                } else {
                    args.fVertBuilder->codeAppendf(
                            "%s = min(1.0, max(%s.x, %s.w) - 1) * min(1.0, max(%s.y, %s.z) - 1);",
                            outputName, gp.fAAEdgeDistances.name(), gp.fAAEdgeDistances.name(),
                            gp.fAAEdgeDistances.name(), gp.fAAEdgeDistances.name());
                }
            }

            void emitFinalCoverage(const QuadPerEdgeAAGeometryProcessor& gp, EmitArgs& args,
                                   const char* distTypeName, const char* edgeDistName,
                                   const char* maxCoverageName, const char* outputName,
                                   bool outputVector) {
                args.fFragBuilder->codeAppendf("%s minDist = min(min(%s.x, %s.y), min(%s.z, %s.w));",
                        distTypeName, edgeDistName, edgeDistName, edgeDistName, edgeDistName);
                if (gp.fPosition.gpuType() == kFloat3_GrSLType) {
                    // The distance from edge equation e to homogeneous point p=sk_Position is
                    // e.x*p.x/p.w + e.y*p.y/p.w + e.z. However, we want screen space
                    // interpolation of this distance. We can do this by multiplying the vertex
                    // attribute by p.w and then multiplying by sk_FragCoord.w in the FS. So we
                    // output e.x*p.x + e.y*p.y + e.z * p.w
                    args.fFragBuilder->codeAppend("minDist *= sk_FragCoord.w;");
                }
                args.fFragBuilder->codeAppendf("%s = %s%s(clamp(minDist, 0.0, %s));",
                        outputName, distTypeName, outputVector ? "4" : "", maxCoverageName);
            }

            // In the packed version, the edge distances are premultiplied by the color's alpha
            // and the color's alpha stores max-coverage*alpha, and edge distances are encoded
            // as halfs instead of floats.
            void emitPackedAACode(const QuadPerEdgeAAGeometryProcessor& gp, EmitArgs& args) {
                // First compute the maximum coverage based on the edge distances (include type so
                // it's a declaration in addition to the assignment emitMaxCoverage writes)
                this->emitMaxCoverage(gp, args, "half maxCoverage");

                // When using packed coverage, always add a half4 varying, even if color isn't
                // initialized, since it is also the vehicle by which the max coverage is sent to
                // the fragment shader.
                GrGLSLVarying color(kHalf4_GrSLType);
                // The color itself is flat, but the max coverage calculation stuffed in the
                // alpha might not be for high perspective rectangles.
                args.fVaryingHandler->addVarying("color", &color);
                if (gp.fColor.isInitialized()) {
                    // Store maxCoverage * real alpha in the color's alpha channel so that coverage
                    // as alpha is respected when clamped to this value. This assumes that the
                    // vertex colors have been sent UNpremul.
                    args.fVertBuilder->codeAppendf("%s = half4(%s.rgb, %s.a * maxCoverage);",
                                                   color.vsOut(), gp.fColor.name(),
                                                   gp.fColor.name());
                } else {
                    // No color, so set rgb to 1s and alpha to just max coverage
                    args.fVertBuilder->codeAppendf("%s = half4(1, 1, 1, maxCoverage);",
                                                   color.vsOut());
                }
                // The alpha of the output color will be modified once final coverage is calculated,
                // and then the UNpremul RGB values will be modulated by the combined alpha and
                // coverage to be the correct premul RGB with coverage.
                args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, color.fsIn());

                // Send the edge distances as half floats, modulated by the color's alpha (this is
                // the major difference from the regular shader flow). By modulating with alpha, the
                // fragment shader calculates the proper coverage*alpha.
                GrGLSLVarying edgeDists(kHalf4_GrSLType);
                args.fVaryingHandler->addVarying("edgeDists", &edgeDists);
                args.fVertBuilder->codeAppendf("%s = %s.a * %s;", edgeDists.vsOut(),
                                               gp.fColor.name(), gp.fAAEdgeDistances.name());

                // The maximum coverage is in the alpha channel of the output color, and we want to
                // store the final coverage back into the output's alpha channel since coverage and
                // alpha have been merged.
                SkString coverage(args.fOutputColor);
                coverage.append(".a");
                this->emitFinalCoverage(gp, args, "half", edgeDists.fsIn(), coverage.c_str(),
                                        coverage.c_str(), /* vector output */ false);
                // Since coverage is baked into the output color's alpha channel, write 1.0 for the
                // output coverage value and make the RGB premul again (which will properly include
                // the original alpha and the determined coverage).
                args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                args.fFragBuilder->codeAppendf("%s.rgb *= %s.a;", args.fOutputColor,
                                               args.fOutputColor);
            }

            // In the normal AA code, the edge distances are passed as float4's, color is not
            // manipulated and a separate varying is created to hold on to the calculated max
            // coverage.
            void emitNormalAACode(const QuadPerEdgeAAGeometryProcessor& gp, EmitArgs& args) {
                if (gp.fColor.isInitialized()) {
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor);
                }

                args.fFragBuilder->codeAppend("float4 edgeDists;");
                args.fVaryingHandler->addPassThroughAttribute(gp.fAAEdgeDistances, "edgeDists");

                GrGLSLVarying maxCoverage(kFloat_GrSLType);
                args.fVaryingHandler->addVarying("maxCoverage", &maxCoverage);

                this->emitMaxCoverage(gp, args, maxCoverage.vsOut());
                this->emitFinalCoverage(gp, args, "float", "edgeDists", maxCoverage.fsIn(),
                                        args.fOutputCoverage, /* vector output */ true);
            }

            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {

                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                       gp.fTextureColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(gp);
                gpArgs->fPositionVar = gp.fPosition.asShaderVar();

                // Handle local coordinates if they exist
                if (gp.fLocalCoord.isInitialized()) {
                    // NOTE: If the only usage of local coordinates is for the inline texture fetch
                    // before FPs, then there are no registered FPCoordTransforms and this ends up
                    // emitting nothing, so there isn't a duplication of local coordinates
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fLocalCoord.asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                // Output coverage calculation code and output color code. Because the max coverage
                // is packed into the color varyings alpha and edge distances are weighted by
                // the true alpha when the spec requests packing, the coverage and color code is
                // tightly coupled. Handle it prior to texture modulation, so that that doesn't need
                // to worry about how the final color was arrived at.
                if (gp.fAAEdgeDistances.isInitialized()) {
                    if (gp.fAAEdgeDistances.gpuType() == kHalf4_GrSLType) {
                        this->emitPackedAACode(gp, args);
                    } else {
                        this->emitNormalAACode(gp, args);
                    }
                } else {
                    // Set coverage to 1. Since there's no AA, there's no need for max coverage
                    // and the output color can be passed through as-is, or hard-coded.
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                    if (gp.fColor.isInitialized()) {
                        args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor);
                    } else {
                        args.fVertBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
                    }
                }

                // If there is a texture, must also handle texture coordinates and reading from
                // the texture in the fragment shader before continuing to fragment processors.
                if (gp.fSampler.isInitialized()) {
                    // Texture coordinates clamped by the domain on the fragment shader; if the GP
                    // has a texture, it's guaranteed to have local coordinates
                    args.fFragBuilder->codeAppend("float2 texCoord;");
                    if (gp.fLocalCoord.cpuType() == kFloat3_GrVertexAttribType) {
                        // Can't do a pass through since we need to perform perspective division
                        GrGLSLVarying v(gp.fLocalCoord.gpuType());
                        args.fVaryingHandler->addVarying(gp.fLocalCoord.name(), &v);
                        args.fVertBuilder->codeAppendf("%s = %s;",
                                                       v.vsOut(), gp.fLocalCoord.name());
                        args.fFragBuilder->codeAppendf("texCoord = %s.xy / %s.z;",
                                                       v.fsIn(), v.fsIn());
                    } else {
                        args.fVaryingHandler->addPassThroughAttribute(gp.fLocalCoord, "texCoord");
                    }

                    // Clamp the now 2D localCoordName variable by the domain if it is provided
                    if (gp.fDomain.isInitialized()) {
                        args.fFragBuilder->codeAppend("float4 domain;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fDomain, "domain",
                                                                      Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "texCoord = clamp(texCoord, domain.xy, domain.zw);");
                    }

                    // Now modulate the starting output color by the texture lookup
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                    args.fFragBuilder->codeAppend(";");
                }
            }

            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(nullptr) {
        SkASSERT(spec.hasVertexColors() && !spec.hasDomain());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(0);
    }

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
                                   GrTextureType textureType, GrPixelConfig textureConfig,
                                   const GrSamplerState& samplerState,
                                   uint32_t extraSamplerKey,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(textureType, textureConfig, samplerState, extraSamplerKey) {
        SkASSERT(spec.hasVertexColors() && spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    void initializeAttrs(const VertexSpec& spec) {
        if (spec.deviceDimensionality() == 3) {
            fPosition = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else {
            fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        }

        int localDim = spec.localDimensionality();
        if (localDim == 3) {
            fLocalCoord = {"localCoord", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else if (localDim == 2) {
            fLocalCoord = {"localCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        } // else localDim == 0 and attribute remains uninitialized

        if (ColorType::kByte == spec.colorType()) {
            fColor = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
        } else if (ColorType::kHalf == spec.colorType()) {
            fColor = {"color", kHalf4_GrVertexAttribType, kHalf4_GrSLType};
        }

        if (spec.hasDomain()) {
            fDomain = {"domain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        if (spec.usesCoverageAA()) {
            if (spec.packEdgeDistances()) {
                fAAEdgeDistances = {"aaEdgeDist", kHalf4_GrVertexAttribType, kHalf4_GrSLType};
            } else {
                fAAEdgeDistances = {"aaEdgeDist", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            }
        }
        this->setVertexAttributes(&fPosition, 5);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition;
    Attribute fColor;
    Attribute fLocalCoord;
    Attribute fDomain;
    Attribute fAAEdgeDistances;

    // Color space will be null and fSampler.isInitialized() returns false when the GP is configured
    // to skip texturing.
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

sk_sp<GrGeometryProcessor> MakeProcessor(const VertexSpec& spec) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec);
}

sk_sp<GrGeometryProcessor> MakeTexturedProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
        GrTextureType textureType, GrPixelConfig textureConfig,
        const GrSamplerState& samplerState, uint32_t extraSamplerKey,
        sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec, caps, textureType, textureConfig,
                                                samplerState, extraSamplerKey,
                                                std::move(textureColorSpaceXform));
}

} // namespace GrQuadPerEdgeAA
