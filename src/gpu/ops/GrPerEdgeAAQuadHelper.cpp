/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPerEdgeAAQuadHelper.h"
#include "GrQuad.h"
#include "SkNx.h"

namespace {

// This computes the four edge equations for a quad, then outsets them and optionally computes a new
// quad as the intersection points of the outset edges. 'x' and 'y' contain the original points as
// input and the outset points as output. 'a', 'b', and 'c' are the edge equation coefficients on
// output. If outsetCorners is true then the x and y positions will be modified to expand half a
// pixel along the appropriate edges. If provided, 'u' and 'v' should hold the texture coordinates
// on input and will also be outset (if u is not null, it is assumed v is not null). The 'r' texture
// coordinate is optional when outsetCorners is true, and should be provided when perspective
// coordinates are needed (r is the local coordinate's w component).
static void compute_quad_edges_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y, Sk4f* a,
                                                   Sk4f* b, Sk4f* c, bool outsetCorners = false,
                                                   Sk4f* u = nullptr, Sk4f* v = nullptr,
                                                   Sk4f* r = nullptr) {
    static constexpr auto fma = SkNx_fma<4, float>;
    // These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
    // order.
    auto nextCW  = [](const Sk4f& v) { return SkNx_shuffle<2, 0, 3, 1>(v); };
    auto nextCCW = [](const Sk4f& v) { return SkNx_shuffle<1, 3, 0, 2>(v); };

    // Compute edge equations for the quad.
    auto xnext = nextCCW(*x);
    auto ynext = nextCCW(*y);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    auto xdiff = xnext - *x;
    auto ydiff = ynext - *y;
    auto invLengths = fma(xdiff, xdiff, ydiff * ydiff).rsqrt();
    xdiff *= invLengths;
    ydiff *= invLengths;

    // Use above vectors to compute edge equations.
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
        auto mask = Sk4f(GrQuadAAFlags::kLeft & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kTop & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        // Outset edge equations for masked out edges another pixel so that they always evaluate
        // >= 1.
        *c += (1.f - mask);
        if (outsetCorners) {
            // Do the vertex outset.
            mask *= 0.5f;
            auto maskCW = nextCW(mask);
            *x += maskCW * -xdiff + mask * nextCW(xdiff);
            *y += maskCW * -ydiff + mask * nextCW(ydiff);
            if (u) {
                // We want to extend the texture coords by the same proportion as the positions.
                maskCW *= invLengths;
                mask *= nextCW(invLengths);
                Sk4f udiff = nextCCW(*u) - *u;
                Sk4f vdiff = nextCCW(*v) - *v;
                *u += maskCW * -udiff + mask * nextCW(udiff);
                *v += maskCW * -vdiff + mask * nextCW(vdiff);
                if (r) {
                    Sk4f wdiff = nextCCW(*r) - *r;
                    *r += maskCW * -wdiff + mask * nextCW(wdiff);
                }
            }
        }
    } else if (outsetCorners) {
        *x += 0.5f * (-xdiff + nextCW(xdiff));
        *y += 0.5f * (-ydiff + nextCW(ydiff));
        if (u) {
            Sk4f t = 0.5f * invLengths;
            Sk4f udiff = nextCCW(*u) - *u;
            Sk4f vdiff = nextCCW(*v) - *v;
            *u += t * -udiff + nextCW(t) * nextCW(udiff);
            *v += t * -vdiff + nextCW(t) * nextCW(vdiff);
            if (r) {
                Sk4f wdiff = nextCCW(*r) - *r;
                *r += t * -wdiff + nextCW(t) * nextCW(wdiff);
            }
        }
    }
}

// Generalizes the above function to extrapolate local coords such that after perspective division
// of the device coordinate, the original local coordinate value is at the original un-outset
// device position. Unlike the above function, outsetting corners is not optional, it will always
// update x,y, and w as needed. (u,v) and r can be null if there are no local coordinates, or they
// are non-perspective. r is the local coordinate's w component.
static void compute_quad_edges_and_outset_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
                                                         Sk4f* w, Sk4f* a, Sk4f* b, Sk4f* c,
                                                         Sk4f* u = nullptr, Sk4f* v = nullptr,
                                                         Sk4f* r = nullptr) {
    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;
    // Don't compute outset corners in the normalized space
    compute_quad_edges_and_outset_vertices(aaFlags, &x2d, &y2d, a, b, c);
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

        if (u) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (r) {
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

        if (u) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (r) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }
}

static SkPoint3 compute_non_aa_persp_edge_coeffs(const GrPerspQuad& deviceQuad) {
    // Fast path for non-AA quads batched into an AA op. Since they are part of the AA op, the
    // vertices need to have valid edge equations that ensure coverage is set to 1. To get
    // perspective interpolation of the edge distance, the vertex shader outputs d*w and then
    // multiplies by 1/w in the fragment shader. For non-AA edges, the edge equation can be
    // simplified to 0*x/w + y/w + c >= 1, so the vertex shader outputs c*w. The quad is sent as two
    // triangles, so a fragment is the interpolation between 3 of the 4 vertices. If iX are the
    // weights for the 3 involved quad vertices, then the fragment shader's state is:
    //   f_cw = c * (iA*wA + iB*wB + iC*wC) and f_1/w = iA/wA + iB/wB + iC/wC
    //   (where A,B,C are chosen from {1,2,3, 4})
    // When there's no perspective, then f_cw*f_1/w = c and setting c = 1 guarantees a proper non-AA
    // edge. Unfortunately when there is perspective, f_cw*f_1/w != c unless the fragment is at a
    // vertex. We must pick a c such that f_cw*f_1/w >= 1 across the whole primitive.
    //Let n = min(w1,w2,w3,w4) and m = max(w1,w2,w3,w4) and rewrite
    //   f_1/w=(iA*wB*wC + iB*wA*wC + iC*wA*wB) / (wA*wB*wC)
    // Since the iXs are weights for the interior of the primitive, then we have:
    //   n <= (iA*wA + iB*wB + iC*wC) <= m and
    //   n^2 <= (iA*wB*wC + iB*wA*wC + iC*wA*wB) <= m^2 and
    //   n^3 <= wA*wB*wC <= m^3 regardless of the choice of A,B, and C
    // Thus if we set c = m^3/n^3, it guarantees f_cw*f_1/w >= 1 for any perspective.
    auto w = deviceQuad.w4f();
    float n = w.min();
    float m = w.max();
    return {0.f, 0.f, (m * m * m) / (n * n * n)};
}

// When there's guaranteed no perspective, the edge coefficients for non-AA quads is constant
static constexpr SkPoint3 kNonAANoPerspEdgeCoeffs = {0, 0, 1};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quads with no anti-aliasing - simple copies of persp quad positions and optionally local coords
//  - templated on domain, explicit definition for every other combination of pos/local coord type
///////////////////////////////////////////////////////////////////////////////////////////////////

// Pos = SkPoint, LocalPos = void, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, void, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    SkASSERT(!deviceQuad.hasPerspective());
    // Only need to set position, no local coords
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
    }
}

// Pos = SkPoint3, LocalPos = void, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, void, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    // Only need to set position, no local coords
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = deviceQuad.point(i);
    }
}

// Pos = SkPoint, LocalPos = SkPoint, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, SkPoint, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    SkASSERT(!deviceQuad.hasPerspective());
    SkASSERT(!srcQuad.hasPerspective());
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
        vertices[i].fLocalPos = {srcQuad.x(i), srcQuad.y(i)};
    }
}

// Pos = SkPoint3, LocalPos = SkPoint, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, SkPoint, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    SkASSERT(!srcQuad.hasPerspective());
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = deviceQuad.point(i);
        vertices[i].fLocalPos = {srcQuad.x(i), srcQuad.y(i)};
    }
}

// Pos = SkPoint, LocalPos = SkPoint3, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, SkPoint3, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    SkASSERT(!deviceQuad.hasPerspective());
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
        vertices[i].fLocalPos = srcQuad.point(i);
    }
}

// Pos = SkPoint3, LocalPos = SkPoint3, AA = No
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, SkPoint3, D, GrAA::kNo>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    // Should be kNone for non-AA and kAll for MSAA.
    SkASSERT(aaFlags == GrQuadAAFlags::kNone || aaFlags == GrQuadAAFlags::kAll);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = deviceQuad.point(i);
        vertices[i].fLocalPos = srcQuad.point(i);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quads with per-edge anti-aliasing - need to outset vertices and local coords (if present).
//  - templated on domain, explicit definition for every other combination of pos/local coord type
///////////////////////////////////////////////////////////////////////////////////////////////////

// Pos = SkPoint, LocalPos = void, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, void, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    SkASSERT(!deviceQuad.hasPerspective());

    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = kNonAANoPerspEdgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    Sk4f a, b, c;
    // Outset corners, but don't pass any texture coordinates in since srcQuad is meaningless
    compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c, true);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], as[4], bs[4], cs[4];
    x.store(xs);
    y.store(ys);
    a.store(as);
    b.store(bs);
    c.store(cs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j]  = {as[j], bs[j], cs[j]};
        }
    }
}

// Pos = SkPoint3, LocalPos = void, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, void, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        SkPoint3 edgeCoeffs = compute_non_aa_persp_edge_coeffs(deviceQuad);
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = deviceQuad.point(i);
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = edgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    auto w = deviceQuad.w4f();
    Sk4f a, b, c;
    // No local coordinates to pass in
    compute_quad_edges_and_outset_persp_vertices(aaFlags, &x, &y, &w, &a, &b, &c);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], ws[4], as[4], bs[4], cs[4];
    x.store(xs);
    y.store(ys);
    w.store(ws);
    a.store(as);
    b.store(bs);
    c.store(cs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i], ws[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j] = {as[j], bs[j], cs[j]};
        }
    }
}

// Pos = SkPoint, LocalPos = SkPoint, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, SkPoint, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    SkASSERT(!deviceQuad.hasPerspective());
    SkASSERT(!srcQuad.hasPerspective());

    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
            vertices[i].fLocalPos = {srcQuad.x(i), srcQuad.y(i)};
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = kNonAANoPerspEdgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    auto u = srcQuad.x4f();
    auto v = srcQuad.y4f();
    Sk4f a, b, c;
    // No w since local positions are represented just as SkPoint
    compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c, true, &u, &v);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], as[4], bs[4], cs[4], us[4], vs[4];
    x.store(xs);
    y.store(ys);
    a.store(as);
    b.store(bs);
    c.store(cs);
    u.store(us);
    v.store(vs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i]};
        vertices[i].fLocalPos = {us[i], vs[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j]  = {as[j], bs[j], cs[j]};
        }
    }
}

// Pos = SkPoint3, LocalPos = SkPoint, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, SkPoint, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    SkASSERT(!srcQuad.hasPerspective());

    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        SkPoint3 edgeCoeffs = compute_non_aa_persp_edge_coeffs(deviceQuad);
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = deviceQuad.point(i);
            vertices[i].fLocalPos = {srcQuad.x(i), srcQuad.y(i)};
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = edgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    auto w = deviceQuad.w4f();
    auto u = srcQuad.x4f();
    auto v = srcQuad.y4f();
    Sk4f a, b, c;
    // Local coordinates are 2D so only provide u and v
    compute_quad_edges_and_outset_persp_vertices(aaFlags, &x, &y, &w, &a, &b, &c, &u, &v);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], ws[4], as[4], bs[4], cs[4], us[4], vs[4];
    x.store(xs);
    y.store(ys);
    w.store(ws);
    a.store(as);
    b.store(bs);
    c.store(cs);
    u.store(us);
    v.store(vs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i], ws[i]};
        vertices[i].fLocalPos = {us[i], vs[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j] = {as[j], bs[j], cs[j]};
        }
    }
}

// Pos = SkPoint, LocalPos = SkPoint3, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint, SkPoint3, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    SkASSERT(!deviceQuad.hasPerspective());

    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = {deviceQuad.x(i), deviceQuad.y(i)};
            vertices[i].fLocalPos = srcQuad.point(i);
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = kNonAANoPerspEdgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    auto u = srcQuad.x4f();
    auto v = srcQuad.y4f();
    auto r = srcQuad.w4f();
    Sk4f a, b, c;
    compute_quad_edges_and_outset_vertices(aaFlags, &x, &y, &a, &b, &c, true, &u, &v, &r);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], as[4], bs[4], cs[4], us[4], vs[4], rs[4];
    x.store(xs);
    y.store(ys);
    a.store(as);
    b.store(bs);
    c.store(cs);
    u.store(us);
    v.store(vs);
    r.store(rs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i]};
        vertices[i].fLocalPos = {us[i], vs[i], rs[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j]  = {as[j], bs[j], cs[j]};
        }
    }
}

// Pos = SkPoint3, LocalPos = SkPoint3, AA = Yes
template<GrPerEdgeAA::Domain D>
static void assign_positions_impl(GrPerEdgeAA::Vertex<SkPoint3, SkPoint3, D, GrAA::kYes>* vertices,
                                  const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                                  GrQuadAAFlags aaFlags) {
    if (aaFlags == GrQuadAAFlags::kNone) {
        // A non-AA quad batched into the AA op can be tessellated more efficiently
        SkPoint3 edgeCoeffs = compute_non_aa_persp_edge_coeffs(deviceQuad);
        for (int i = 0; i < 4; ++i) {
            vertices[i].fPosition = deviceQuad.point(i);
            vertices[i].fLocalPos = srcQuad.point(i);
            for (int j = 0; j < 4; ++j) {
                vertices[i].fEdges[j] = edgeCoeffs;
            }
        }
        return;
    }

    auto x = deviceQuad.x4f();
    auto y = deviceQuad.y4f();
    auto w = deviceQuad.w4f();
    auto u = srcQuad.x4f();
    auto v = srcQuad.y4f();
    auto r = srcQuad.w4f();
    Sk4f a, b, c;
    compute_quad_edges_and_outset_persp_vertices(aaFlags, &x, &y, &w, &a, &b, &c, &u, &v, &r);

    // Faster to store the Sk4fs all at once rather than element-by-element into vertices.
    float xs[4], ys[4], ws[4], as[4], bs[4], cs[4], us[4], vs[4], rs[4];
    x.store(xs);
    y.store(ys);
    w.store(ws);
    a.store(as);
    b.store(bs);
    c.store(cs);
    u.store(us);
    v.store(vs);
    r.store(rs);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fPosition = {xs[i], ys[i], ws[i]};
        vertices[i].fLocalPos = {us[i], vs[i], rs[i]};
        for (int j = 0; j < 4; ++j) {
            vertices[i].fEdges[j] = {as[j], bs[j], cs[j]};
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quad texture domain handlers (one definition per domain mode, templated on every other param)
///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Pos, GrAA AA>
static void assign_domain_impl(
        GrPerEdgeAA::Vertex<Pos, SkPoint, GrPerEdgeAA::Domain::kYes, AA>* vertices,
        GrPerEdgeAA::Domain domain, GrSamplerState::Filter filter, const SkRect& srcRect,
        GrSurfaceOrigin origin, float iw, float ih) {
    static constexpr SkRect kLargeRect = {-2, -2, 2, 2};
    SkRect domainRect;
    if (domain == GrPerEdgeAA::Domain::kYes) {
        auto ltrb = Sk4f::Load(&srcRect);
        if (filter == GrSamplerState::Filter::kBilerp) {
            auto rblt = SkNx_shuffle<2, 3, 0, 1>(ltrb);
            auto whwh = (rblt - ltrb).abs();
            auto c = (rblt + ltrb) * 0.5f;
            static const Sk4f kOffsets = {0.5f, 0.5f, -0.5f, -0.5f};
            ltrb = (whwh < 1.f).thenElse(c, ltrb + kOffsets);
        }
        ltrb *= Sk4f(iw, ih, iw, ih);
        if (origin == kBottomLeft_GrSurfaceOrigin) {
            static const Sk4f kMul = {1.f, -1.f, 1.f, -1.f};
            static const Sk4f kAdd = {0.f, 1.f, 0.f, 1.f};
            ltrb = SkNx_shuffle<0, 3, 2, 1>(kMul * ltrb + kAdd);
        }
        ltrb.store(&domainRect);
    } else {
        // The quad had a kNo domain but was merged into an op that requires a domain
        domainRect = kLargeRect;
    }
    for (int i = 0; i < 4; ++i) {
        vertices[i].fSrcDomain = domainRect;
    }
}

template <typename Pos, GrAA AA>
static void assign_domain_impl(
        GrPerEdgeAA::Vertex<Pos, SkPoint, GrPerEdgeAA::Domain::kNo, AA>* vertices,
        GrPerEdgeAA::Domain domain, GrSamplerState::Filter filter, const SkRect& srcRect,
        GrSurfaceOrigin origin, float iw, float ih) {
    SkASSERT(domain == GrPerEdgeAA::Domain::kNo);
}

} // anonymous namespace

namespace GrPerEdgeAA {

template <typename Pos, Domain D, GrAA AA>
void TesselateTexturedQuad(Vertex<Pos, SkPoint, D, AA>* vertices, GrQuadAAFlags aaFlags,
                           Domain constrainDomain, const GrPerspQuad& deviceQuad,
                           const SkRect& srcRect, GrColor color, GrSurfaceOrigin origin,
                           GrSamplerState::Filter filter, float iw, float ih) {
    SkRect texRect = {
        iw * srcRect.fLeft,
        ih * srcRect.fTop,
        iw * srcRect.fRight,
        ih * srcRect.fBottom
    };
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        texRect.fTop = 1.f - texRect.fTop;
        texRect.fBottom = 1.f - texRect.fBottom;
    }
    GrPerspQuad srcQuad(texRect, SkMatrix::I());
    assign_positions_impl(vertices, deviceQuad, srcQuad, aaFlags);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fColor = color;
    }
    assign_domain_impl(vertices, constrainDomain, filter, srcRect, origin, iw, ih);
}

template <typename Pos, typename LocalPos, GrAA AA>
void TesselateQuad(Vertex<Pos, LocalPos, Domain::kNo, AA>* vertices, GrQuadAAFlags aaFlags,
                   const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad, GrColor color) {
    assign_positions_impl(vertices, deviceQuad, srcQuad, aaFlags);
    for (int i = 0; i < 4; ++i) {
        vertices[i].fColor = color;
    }
}

// Provide instantiations of the templates for all of their parameter permutations.
#define INSTANTIATE_TEXTURED(Pos, TheDomain, AA) \
    template void TesselateTexturedQuad<Pos, TheDomain, AA>( \
            Vertex<Pos, SkPoint, TheDomain, AA>*, GrQuadAAFlags, Domain, const GrPerspQuad&, \
            const SkRect&, GrColor, GrSurfaceOrigin, GrSamplerState::Filter, float, float)
#define INSTANTIATE_SIMPLE(Pos, LocalPos, AA) \
    template void TesselateQuad<Pos, LocalPos, AA>( \
            Vertex<Pos, LocalPos, Domain::kNo, AA>*, GrQuadAAFlags, const GrPerspQuad&, \
            const GrPerspQuad&, GrColor);

INSTANTIATE_TEXTURED(SkPoint, Domain::kNo, GrAA::kNo);
INSTANTIATE_TEXTURED(SkPoint, Domain::kNo, GrAA::kYes);
INSTANTIATE_TEXTURED(SkPoint, Domain::kYes, GrAA::kNo);
INSTANTIATE_TEXTURED(SkPoint, Domain::kYes, GrAA::kYes);
INSTANTIATE_TEXTURED(SkPoint3, Domain::kNo, GrAA::kNo);
INSTANTIATE_TEXTURED(SkPoint3, Domain::kNo, GrAA::kYes);
INSTANTIATE_TEXTURED(SkPoint3, Domain::kYes, GrAA::kNo);
INSTANTIATE_TEXTURED(SkPoint3, Domain::kYes, GrAA::kYes);

INSTANTIATE_SIMPLE(SkPoint, void, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint, void, GrAA::kYes);
INSTANTIATE_SIMPLE(SkPoint, SkPoint, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint, SkPoint, GrAA::kYes);
INSTANTIATE_SIMPLE(SkPoint, SkPoint3, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint, SkPoint3, GrAA::kYes);
INSTANTIATE_SIMPLE(SkPoint3, void, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint3, void, GrAA::kYes);
INSTANTIATE_SIMPLE(SkPoint3, SkPoint, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint3, SkPoint, GrAA::kYes);
INSTANTIATE_SIMPLE(SkPoint3, SkPoint3, GrAA::kNo);
INSTANTIATE_SIMPLE(SkPoint3, SkPoint3, GrAA::kYes);

#undef INSTANTIATE_TEXTURED
#undef INSTANTIATE_SIMPLE

} // GrPerEdgeAA namespace
