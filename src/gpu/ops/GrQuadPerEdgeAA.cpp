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

static AI float get_max_coverage(const Sk4f& lengths) {
    float minWidth = SkMinScalar(lengths[0], lengths[3]);
    float minHeight = SkMinScalar(lengths[1], lengths[2]);
    // Calculate approximate area of the quad, pinning dimensions to 1 in case the quad is larger
    // than a pixel. Sub-pixel quads that are rotated may in fact have a different true maximum
    // coverage than this calculation, but this will be close and is stable.
    return SkMinScalar(minWidth, 1.f) * SkMinScalar(minHeight, 1.f);
}

// This computes the four edge equations for a quad, then outsets them and optionally computes a new
// quad as the intersection points of the outset edges. 'x' and 'y' contain the original points as
// input and the outset points as output. In order to be used as a component of perspective edge
// distance calculation, this exports edge equations in 'a', 'b', and 'c'. Use
// compute_edge_distances to turn these equations into the distances needed by the shader. The
// values in x, y, u, v, and r are possibly updated if outsetting is needed. r is the local
// position's w component if it exists.
//
// Returns maximum coverage allowed for any given pixel.
static float compute_quad_edges_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
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

    return get_max_coverage(invLengths.invert());
}

// A specialization of the above function that can compute edge distances very quickly when it knows
// that the edges intersect at right angles, i.e. any transform other than skew and perspective
// (GrQuadType::kRectilinear). Unlike the above function, this always outsets the corners since it
// cannot be reused in the perspective case.
static float compute_rectilinear_dists_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x,
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

    return get_max_coverage(lengths);
}

// Generalizes compute_quad_edge_distances_and_outset_vertices to extrapolate local coords such that
// after perspective division of the device coordinate, the original local coordinate value is at
// the original un-outset device position. r is the local coordinate's w component.
static float compute_quad_dists_and_outset_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x,
        Sk4f* y, Sk4f* w, Sk4f edgeDistances[4], Sk4f* u, Sk4f* v, Sk4f* r, int uvrChannelCount) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;
    Sk4f a, b, c;
    // Don't compute outset corners in the normalized space, which means u, v, and r don't need
    // to be provided here (outset separately below). Since this is computing distances for a
    // projected quad, there is a very good chance it's not rectilinear so use the general 2D path.
    float maxProjectedCoverage = compute_quad_edges_and_outset_vertices(aaFlags, &x2d, &y2d,
            &a, &b, &c, nullptr, nullptr, nullptr, /* uvr ct */ 0, /* outsetCorners */ false);

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

    return maxProjectedCoverage;
}

// Calculate safe edge distances for non-aa quads that have been batched with aa quads. Since the
// fragment shader multiples by 1/w, so the edge distance cannot just be set to 1. It cannot just
// be set to w either due to interpolation across the triangle. If iA, iB, and iC are the
// barycentric weights of the triangle, and we set the edge distance to w, the fragment shader
// actually sees d = (iA*wA + iB*wB + iC*wC) * (iA/wA + iB/wB + iC/wC). Without perspective this
// simplifies to 1 as necessary, but we must choose something other than w when there is perspective
// to ensure that d >= 1 and the edge shows as non-aa.
static float compute_nonaa_edge_distances(const Sk4f& w, bool hasPersp, Sk4f edgeDistances[4]) {
    // Let n = min(w1,w2,w3,w4) and m = max(w1,w2,w3,w4) and rewrite
    //   d = (iA*wA + iB*wB + iC*wC) * (iA*wB*wC + iB*wA*wC + iC*wA*wB) / (wA*wB*wC)
    //       |   e=attr from VS    |   |         fragCoord.w = 1/w                 |
    // Since the weights are the interior of the primitive then we have:
    //   n <= (iA*wA + iB*wB + iC*wC) <= m and
    //   n^2 <= (iA*wB*wC + iB*wA*wC + iC*wA*wB) <= m^2 and
    //   n^3 <= wA*wB*wC <= m^3 regardless of the choice of A, B, and C verts in the quad
    // Thus if we set e = m^3/n^3, it guarantees d >= 1 for any perspective.
    float e;
    if (hasPersp) {
        float m = w.max();
        float n = w.min();
        e = (m * m * m) / (n * n * n);
    } else {
        e = 1.f;
    }

    // All edge distances set to the same
    for (int i = 0; i < 4; ++i) {
        edgeDistances[i] = e;
    }

    // Non-aa, so always use full coverage
    return 1.f;
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

////////////////// Tessellate Implementation

void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                 const SkPMColor4f& color4f, const GrPerspQuad& localQuad, const SkRect& domain,
                 GrQuadAAFlags aaFlags) {
    bool deviceHasPerspective = spec.deviceQuadType() == GrQuadType::kPerspective;
    bool localHasPerspective = spec.localQuadType() == GrQuadType::kPerspective;
    GrVertexColor color(color4f, GrQuadPerEdgeAA::ColorType::kHalf == spec.colorType());

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
    float maxCoverage = 1.f;
    if (spec.usesCoverageAA()) {
        // Must calculate edges and possibly outside the positions
        if (aaFlags == GrQuadAAFlags::kNone) {
            // A non-AA quad that got batched into an AA group, so it should have full coverage
            maxCoverage = compute_nonaa_edge_distances(w, deviceHasPerspective, edgeDistances);
        } else if (deviceHasPerspective) {
            // For simplicity, pointers to u, v, and r are always provided, but the local dim param
            // ensures that only loaded Sk4fs are modified in the compute functions.
            maxCoverage = compute_quad_dists_and_outset_persp_vertices(aaFlags, &x, &y, &w,
                    edgeDistances, &u, &v, &r, spec.localDimensionality());
        } else if (spec.deviceQuadType() <= GrQuadType::kRectilinear) {
            maxCoverage = compute_rectilinear_dists_and_outset_vertices(aaFlags, &x, &y,
                    edgeDistances, &u, &v, &r, spec.localDimensionality());
        } else {
            Sk4f a, b, c;
            maxCoverage = compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c,
                    &u, &v, &r, spec.localDimensionality(), /*outset*/ true);
            compute_edge_distances(a, b, c, x, y, w, edgeDistances); // w holds 1.f as desired
        }
    }

    // Now rearrange the Sk4fs into the interleaved vertex layout:
    //  i.e. x1x2x3x4 y1y2y3y4 -> x1y1 x2y2 x3y3 x4y
    GrVertexWriter vb{vertices};
    for (int i = 0; i < 4; ++i) {
        // save position, always send a vec4 because we embed max coverage in the last component.
        // For 2D quads, we know w holds the correct 1.f, so just write it out without branching
        vb.write(x[i], y[i], w[i], maxCoverage);

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
            vb.write(edgeDistances[i]);
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
public:

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& spec) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(spec));
    }

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& vertexSpec, const GrShaderCaps& caps,
                                           GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState::Filter filter,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                vertexSpec, caps, textureType, textureConfig, filter,
                std::move(textureColorSpaceXform)));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // aa, domain, texturing are single bit flags
        uint32_t x = fAAEdgeDistances.isInitialized() ? 0 : 1;
        x |= fDomain.isInitialized() ? 0 : 2;
        x |= fSampler.isInitialized() ? 0 : 4;
        // regular position has two options as well
        x |= fNeedsPerspective ? 0 : 8;
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 16 : 32;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (this->fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 64 : 128;
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
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;

                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                       gp.fTextureColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(gp);

                // Extract effective position out of vec4 as a local variable in the vertex shader
                if (gp.fNeedsPerspective) {
                    args.fVertBuilder->codeAppendf("float3 position = %s.xyz;",
                                                   gp.fPositionWithCoverage.name());
                } else {
                    args.fVertBuilder->codeAppendf("float2 position = %s.xy;",
                                                   gp.fPositionWithCoverage.name());
                }
                gpArgs->fPositionVar = {"position",
                                        gp.fNeedsPerspective ? kFloat3_GrSLType : kFloat2_GrSLType,
                                        GrShaderVar::kNone_TypeModifier};

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

                // Solid color before any texturing gets modulated in
                if (gp.fColor.isInitialized()) {
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor,
                                                                  Interpolation::kCanBeFlat);
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

                // And lastly, output the coverage calculation code
                if (gp.fAAEdgeDistances.isInitialized()) {
                    GrGLSLVarying maxCoverage(kFloat_GrSLType);
                    args.fVaryingHandler->addVarying("maxCoverage", &maxCoverage);
                    args.fVertBuilder->codeAppendf("%s = %s.w;",
                                                   maxCoverage.vsOut(), gp.fPositionWithCoverage.name());

                    args.fFragBuilder->codeAppend("float4 edgeDists;");
                    args.fVaryingHandler->addPassThroughAttribute(gp.fAAEdgeDistances, "edgeDists");

                    args.fFragBuilder->codeAppend(
                            "float minDist = min(min(edgeDists.x, edgeDists.y),"
                            " min(edgeDists.z, edgeDists.w));");
                    if (gp.fNeedsPerspective) {
                        // The distance from edge equation e to homogeneous point p=sk_Position is
                        // e.x*p.x/p.w + e.y*p.y/p.w + e.z. However, we want screen space
                        // interpolation of this distance. We can do this by multiplying the vertex
                        // attribute by p.w and then multiplying by sk_FragCoord.w in the FS. So we
                        // output e.x*p.x + e.y*p.y + e.z * p.w
                        args.fFragBuilder->codeAppend("minDist *= sk_FragCoord.w;");
                    }
                    // Clamp to max coverage after the perspective divide since perspective quads
                    // calculated the max coverage in projected space.
                    args.fFragBuilder->codeAppendf("%s = float4(clamp(minDist, 0.0, %s));",
                                                   args.fOutputCoverage, maxCoverage.fsIn());
                } else {
                    // Set coverage to 1
                    args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
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
                                   GrSamplerState::Filter filter,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(textureType, textureConfig, filter) {
        SkASSERT(spec.hasVertexColors() && spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    void initializeAttrs(const VertexSpec& spec) {
        fNeedsPerspective = spec.deviceDimensionality() == 3;
        fPositionWithCoverage = {"posAndCoverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType};

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
            fAAEdgeDistances = {"aaEdgeDist", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }
        this->setVertexAttributes(&fPositionWithCoverage, 5);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPositionWithCoverage;
    Attribute fColor;
    Attribute fLocalCoord;
    Attribute fDomain;
    Attribute fAAEdgeDistances;

    // The positions attribute is always a vec4 and can't be used to encode perspectiveness
    bool fNeedsPerspective;

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
        const GrSamplerState::Filter filter, sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec, caps, textureType, textureConfig, filter,
                                                std::move(textureColorSpaceXform));
}

} // namespace GrQuadPerEdgeAA
