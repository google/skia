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

// outset and outsetCW are provided separately to allow for different magnitude outsets for
// with-edge and "perpendicular" edge shifts. This is needed when one axis cannot be inset the full
// half pixel without crossing over the other side.
static AI void outset_masked_vertices(const Sk4f& outset, const Sk4f& outsetCW, const Sk4f& xdiff,
                                      const Sk4f& ydiff, const Sk4f& invLengths, const Sk4f& mask,
                                      Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    // The mask is rotated compared to the outsets and edge vectors, since if the edge is "on"
    // both its points need to be moved along their other edge vectors.
    auto maskedOutset = -outset * nextCW(mask);
    auto maskedOutsetCW = outsetCW * mask;
    // x = x + outsetCW * mask * nextCW(xdiff) - outset * nextCW(mask) * xdiff
    *x += fma(maskedOutsetCW, nextCW(xdiff), maskedOutset * xdiff);
    *y += fma(maskedOutsetCW, nextCW(ydiff), maskedOutset * ydiff);
    if (uvrCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        maskedOutset *= invLengths;
        maskedOutsetCW *= nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += fma(maskedOutsetCW, nextCW(udiff), maskedOutset * udiff);
        *v += fma(maskedOutsetCW, nextCW(vdiff), maskedOutset * vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += fma(maskedOutsetCW, nextCW(rdiff), maskedOutset * rdiff);
        }
    }
}

static AI void outset_vertices(const Sk4f& outset, const Sk4f& outsetCW, const Sk4f& xdiff,
                               const Sk4f& ydiff, const Sk4f& invLengths,
                               Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    // x = x + outsetCW * nextCW(xdiff) - outset * xdiff (as above, but where mask = (1,1,1,1))
    *x += fma(outsetCW, nextCW(xdiff), -outset * xdiff);
    *y += fma(outsetCW, nextCW(ydiff), -outset * ydiff);
    if (uvrCount > 0) {
        Sk4f t = -outset * invLengths; // Bake minus sign in here
        Sk4f tCW = outsetCW * nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += fma(tCW, nextCW(udiff), t * udiff);
        *v += fma(tCW, nextCW(vdiff), t * vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += fma(tCW, nextCW(rdiff), t * rdiff);
        }
    }
}

// Updates outset in place to account for non-90 degree angles of the quad edges stored in
// xdiff, ydiff (which are assumed to be normalized).
static void adjust_non_rectilinear_outset(const Sk4f& xdiff, const Sk4f& ydiff, Sk4f* outset) {
    // The distance the point needs to move is outset/sqrt(1-cos^2(theta)), where theta is the angle
    // between the two edges at that point. cos(theta) is equal to dot(xydiff, nextCW(xydiff)),
    Sk4f cosTheta = fma(xdiff, nextCW(xdiff), ydiff * nextCW(ydiff));
    *outset *= (1.f - cosTheta * cosTheta).rsqrt();
    // But clamp to make sure we don't expand by a giant amount if the sheer is really high
    *outset = Sk4f::Max(-3.f, Sk4f::Min(*outset, 3.f));
}

// Computes the vertices for the two nested quads used to create AA edges. The original single quad
// should be duplicated as input in x1 and x2, y1 and y2, and possibly u1|u2, v1|v2, [r1|r2]
// (controlled by uvrChannelCount).  While the values should be duplicated, they should be separate
// pointers. The outset quad is written in-place back to x1, y1, etc. and the inset inner quad is
// written to x2, y2, etc.
static float compute_nested_quad_vertices(GrQuadAAFlags aaFlags, Sk4f* x1, Sk4f* y1,
        Sk4f* u1, Sk4f* v1, Sk4f* r1, Sk4f* x2, Sk4f* y2, Sk4f* u2, Sk4f* v2, Sk4f* r2,
        int uvrCount, bool rectilinear) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    // Compute edge vectors for the quad.
    auto xnext = nextCCW(*x1);
    auto ynext = nextCCW(*y1);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(*x1, *y1, xnext, ynext, &xdiff, &ydiff, &invLengths);

    // When outsetting, we want the new edge to be .5px away from the old line, which means the
    // corners may need to be adjusted by more than .5px if the matrix had sheer.
    Sk4f outset = 0.5f;
    if (!rectilinear) {
        adjust_non_rectilinear_outset(xdiff, ydiff, &outset);
    }

    // When insetting, cap the inset amount to be half of the edge length, except that each edge
    // has to remain parallel, so we separately limit LR and TB to half of the smallest of the
    // opposing edges.
    Sk4f lengths = invLengths.invert();
    Sk2f sides(SkMinScalar(lengths[0], lengths[3]), SkMinScalar(lengths[1], lengths[2]));
    Sk4f edgeLimits = 0.5f * SkNx_shuffle<0, 1, 1, 0>(sides);

    if ((edgeLimits < 0.5f).anyTrue()) {
        // Dealing with a subpixel rectangle, so must calculate clamped insets and padded outsets.
        // The outsets are padded to ensure that the quad spans 2 pixels for improved interpolation.
        Sk4f inset = -Sk4f::Min(outset, edgeLimits);
        Sk4f insetCW = -Sk4f::Min(outset, nextCW(edgeLimits));

        // The parallel distance shift caused by outset is currently 0.5, but need to scale it up to
        // 0.5*(2 - side) so that (side + 2*shift) = 2px. Thus scale outsets for thin edges by
        // (2 - side) since it already has the 1/2.
        Sk4f outsetScale = 2.f - 2.f * Sk4f::Min(edgeLimits, 0.5f); // == 1 for non-thin edges
        Sk4f outsetCW = outset * nextCW(outsetScale);
        outset *= outsetScale;

        if (aaFlags != GrQuadAAFlags::kAll) {
            Sk4f mask = compute_edge_mask(aaFlags);
            outset_masked_vertices(outset, outsetCW, xdiff, ydiff, invLengths, mask, x1, y1,
                                   u1, v1, r1, uvrCount);
            outset_masked_vertices(inset, insetCW, xdiff, ydiff, invLengths, mask, x2, y2,
                                   u2, v2, r2, uvrCount);
        } else {
            outset_vertices(outset, outsetCW, xdiff, ydiff, invLengths, x1, y1, u1, v1, r1, uvrCount);
            outset_vertices(inset, insetCW, xdiff, ydiff, invLengths, x2, y2, u2, v2, r2, uvrCount);
        }
    } else {
        // Since it's not subpixel, the inset is just the opposite of the outset and there's no
        // difference between CCW and CW behavior.
        Sk4f inset = -outset;
        if (aaFlags != GrQuadAAFlags::kAll) {
            Sk4f mask = compute_edge_mask(aaFlags);
            outset_masked_vertices(outset, outset, xdiff, ydiff, invLengths, mask, x1, y1,
                                   u1, v1, r1, uvrCount);
            outset_masked_vertices(inset, inset, xdiff, ydiff, invLengths, mask, x2, y2,
                                   u2, v2, r2, uvrCount);
        } else {
            outset_vertices(outset, outset, xdiff, ydiff, invLengths, x1, y1, u1, v1, r1, uvrCount);
            outset_vertices(inset, inset, xdiff, ydiff, invLengths, x2, y2, u2, v2, r2, uvrCount);
        }
    }

    // An approximation of the pixel area covered by the quad
    sides = Sk2f::Min(1.f, sides);
    return sides[0] * sides[1];
}

// For each device space corner, devP, label its left/right or top/bottom opposite device space
// point opDevPt. The new device space point is opDevPt + s (devPt - opDevPt) where s is
// (length(devPt - opDevPt) + outset) / length(devPt - opDevPt); This returns the interpolant s,
// adjusted for any subpixel corrections. If subpixel, it also updates the max coverage.
static Sk4f get_projected_interpolant(const Sk4f& len, const Sk4f& outsets, float* maxCoverage) {
    if ((len < 1.f).anyTrue()) {
        *maxCoverage *= len.min();

        // When insetting, the amount is clamped to be half the minimum edge length to prevent
        // overlap. When outsetting, the amount is padded to cover 2 pixels.
        if ((outsets < 0.f).anyTrue()) {
            return (len - 0.5f * len.min()) / len;
        } else {
            return (len + outsets * (2.f - len.min())) / len;
        }
    } else {
        return (len + outsets) / len;
    }
}

// Generalizes compute_nested_quad_vertices to extrapolate local coords such that
// after perspective division of the device coordinate, the original local coordinate value is at
// the original un-outset device position. r is the local coordinate's w component. However, since
// the projected edges will be different for inner and outer quads, there isn't much reuse between
// the calculations, so it's easier to just have this operate on one quad a time.
static float compute_quad_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
        Sk4f* w, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount, bool inset) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;

    // Must compute non-rectilinear outset quantity using the projected 2d edge vectors
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(x2d, y2d, nextCCW(x2d), nextCCW(y2d), &xdiff, &ydiff, &invLengths);
    Sk4f outset = inset ? -0.5f : 0.5f;
    adjust_non_rectilinear_outset(xdiff, ydiff, &outset);

    float maxProjectedCoverage = 1.f;

    if ((GrQuadAAFlags::kLeft | GrQuadAAFlags::kRight) & aaFlags) {
        // For each entry in x the equivalent entry in opX is the left/right opposite and so on.
        Sk4f opX = SkNx_shuffle<2, 3, 0, 1>(*x);
        Sk4f opW = SkNx_shuffle<2, 3, 0, 1>(*w);
        Sk4f opY = SkNx_shuffle<2, 3, 0, 1>(*y);
        // vx/vy holds the device space left-to-right vectors along top and bottom of the quad.
        Sk2f vx = SkNx_shuffle<2, 3>(x2d) - SkNx_shuffle<0, 1>(x2d);
        Sk2f vy = SkNx_shuffle<2, 3>(y2d) - SkNx_shuffle<0, 1>(y2d);
        Sk4f len = SkNx_shuffle<0, 1, 0, 1>(SkNx_fma(vx, vx, vy * vy).sqrt());

        // Compute t in homogeneous space from s using similar triangles so that we can produce
        // homogeneous outset vertices for perspective-correct interpolation.
        Sk4f s = get_projected_interpolant(len, outset, &maxProjectedCoverage);
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

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
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
        Sk4f len = SkNx_shuffle<0, 0, 1, 1>(SkNx_fma(vx, vx, vy * vy).sqrt());

        Sk4f s = get_projected_interpolant(len, outset, &maxProjectedCoverage);
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

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }

    return maxProjectedCoverage;
}

enum class CoverageMode {
    kNone,
    kWithPosition,
    kWithColor
};

static CoverageMode get_mode_for_spec(const GrQuadPerEdgeAA::VertexSpec& spec) {
    if (spec.usesCoverageAA()) {
        if (spec.compatibleWithAlphaAsCoverage() && spec.hasVertexColors()) {
            return CoverageMode::kWithColor;
        } else {
            return CoverageMode::kWithPosition;
        }
    } else {
        return CoverageMode::kNone;
    }
}

// Writes four vertices in triangle strip order, including the additional data for local
// coordinates, domain, color, and coverage as needed to satisfy the vertex spec.
static void write_quad(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                       CoverageMode mode, float coverage,
                       SkPMColor4f color4f, bool wideColor,
                       const SkRect& domain,
                       const Sk4f& x, const Sk4f& y, const Sk4f& w,
                       const Sk4f& u, const Sk4f& v, const Sk4f& r) {
    static constexpr auto If = GrVertexWriter::If<float>;

    if (mode == CoverageMode::kWithColor) {
        // Multiply the color by the coverage up front
        SkASSERT(spec.hasVertexColors());
        color4f = color4f * coverage;
    }
    GrVertexColor color(color4f, wideColor);

    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        vb->write(x[i], y[i], If(spec.deviceQuadType() == GrQuadType::kPerspective, w[i]),
                  If(mode == CoverageMode::kWithPosition, coverage));

        // save color
        if (spec.hasVertexColors()) {
            vb->write(color);
        }

        // save local position
        if (spec.hasLocalCoords()) {
            vb->write(u[i], v[i], If(spec.localQuadType() == GrQuadType::kPerspective, r[i]));
        }

        // save the domain
        if (spec.hasDomain()) {
            vb->write(domain);
        }
    }
}

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static const int kVertsPerAAFillRect = 8;
static const int kIndicesPerAAFillRect = 30;

static sk_sp<const GrGpuBuffer> get_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

    // clang-format off
    static const uint16_t gFillAARectIdx[] = {
        0, 1, 2, 1, 3, 2,
        0, 4, 1, 4, 5, 1,
        0, 6, 4, 0, 2, 6,
        2, 3, 6, 3, 7, 6,
        1, 5, 3, 3, 5, 7,
    };
    // clang-format on

    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
            gFillAARectIdx, kIndicesPerAAFillRect, GrQuadPerEdgeAA::kNumAAQuadsInIndexBuffer,
            kVertsPerAAFillRect, gAAFillRectIndexBufferKey);
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

////////////////// Tessellate Implementation

void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                 const SkPMColor4f& color4f, const GrPerspQuad& localQuad, const SkRect& domain,
                 GrQuadAAFlags aaFlags) {
    bool wideColor = GrQuadPerEdgeAA::ColorType::kHalf == spec.colorType();
    CoverageMode mode = get_mode_for_spec(spec);

    // Load position data into Sk4fs (always x, y, and load w to avoid branching down the road)
    Sk4f oX = deviceQuad.x4f();
    Sk4f oY = deviceQuad.y4f();
    Sk4f oW = deviceQuad.w4f(); // Guaranteed to be 1f if it's not perspective

    // Load local position data into Sk4fs (either none, just u,v or all three)
    Sk4f oU, oV, oR;
    if (spec.hasLocalCoords()) {
        oU = localQuad.x4f();
        oV = localQuad.y4f();
        oR = localQuad.w4f(); // Will be ignored if the local quad type isn't perspective
    }

    GrVertexWriter vb{vertices};
    if (spec.usesCoverageAA()) {
        SkASSERT(mode == CoverageMode::kWithPosition || mode == CoverageMode::kWithColor);

        // Must calculate two new quads, an outset and inset by .5 in projected device space, so
        // duplicate the original quad into new Sk4fs for the inset.
        Sk4f iX = oX, iY = oY, iW = oW;
        Sk4f iU = oU, iV = oV, iR = oR;

        float maxCoverage = 1.f;
        if (aaFlags != GrQuadAAFlags::kNone) {
            if (spec.deviceQuadType() == GrQuadType::kPerspective) {
                // Outset and inset the quads independently because perspective makes each shift
                // unique. Since iX copied pre-outset oX, this will compute the proper inset too.
                compute_quad_persp_vertices(aaFlags, &oX, &oY, &oW, &oU, &oV, &oW,
                                            spec.localDimensionality(), /* inset */ false);
                // Save coverage limit when computing inset quad
                maxCoverage = compute_quad_persp_vertices(aaFlags, &iX, &iY, &iW, &iU, &iV, &iW,
                                                          spec.localDimensionality(), true);
            } else {
                // In the 2D case, insetting and outsetting can reuse the edge vectors, so the
                // nested quads are computed together
                maxCoverage = compute_nested_quad_vertices(aaFlags, &oX, &oY, &oU, &oV, &oR,
                        &iX, &iY, &iU, &iV, &iR, spec.localDimensionality(),
                        spec.deviceQuadType() <= GrQuadType::kRectilinear);
            }
            // NOTE: could provide an even more optimized tessellation function for axis-aligned
            // rects since the positions can be outset by constants without doing vector math,
            // except it must handle identifying the winding of the quad vertices if the transform
            // applied a mirror, etc. The current 2D case is already adequately fast.
        } // else don't adjust any positions, let the outer quad form degenerate triangles

        // Write two quads for inner and outer, inner will use the
        write_quad(&vb, spec, mode, maxCoverage, color4f, wideColor, domain,
                   iX, iY, iW, iU, iV, iR);
        write_quad(&vb, spec, mode, 0.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(mode == CoverageMode::kNone);
        write_quad(&vb, spec, mode, 1.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
    }

    return vb.fPtr;
}

bool ConfigureMeshIndices(GrMeshDrawOp::Target* target, GrMesh* mesh, const VertexSpec& spec,
                          int quadCount) {
    if (spec.usesCoverageAA()) {
        // AA quads use 8 vertices, basically nested rectangles
        sk_sp<const GrGpuBuffer> ibuffer = get_index_buffer(target->resourceProvider());
        if (!ibuffer) {
            return false;
        }

        mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
        mesh->setIndexedPatterned(std::move(ibuffer), kIndicesPerAAFillRect, kVertsPerAAFillRect,
                                  quadCount, kNumAAQuadsInIndexBuffer);
    } else {
        // Non-AA quads use 4 vertices, and regular triangle strip layout
        if (quadCount > 1) {
            sk_sp<const GrGpuBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                return false;
            }

            mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
            mesh->setIndexedPatterned(std::move(ibuffer), 6, 4, quadCount,
                                      GrResourceProvider::QuadCountOfQuadBuffer());
        } else {
            mesh->setPrimitiveType(GrPrimitiveType::kTriangleStrip);
            mesh->setNonIndexedNonInstanced(4);
        }
    }

    return true;
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
                                           const GrSamplerState& samplerState,
                                           uint32_t extraSamplerKey,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                vertexSpec, caps, textureType, textureConfig, samplerState, extraSamplerKey,
                std::move(textureColorSpaceXform)));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // domain, texturing, device-dimensions are single bit flags
        uint32_t x = fDomain.isInitialized() ? 0 : 1;
        x |= fSampler.isInitialized() ? 0 : 2;
        x |= fNeedsPerspective ? 0 : 4;
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 8 : 16;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 32 : 64;
        }
        // and coverage mode, 00 for none, 01 for withposition, 10 for withcolor
        if (fCoverageMode != CoverageMode::kNone) {
            x |= CoverageMode::kWithPosition == fCoverageMode ? 128 : 256;
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

                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    // Strip last channel from the vertex attribute to remove coverage and get the
                    // actual position
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("float3 position = %s.xyz;",
                                                       gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("float2 position = %s.xy;",
                                                       gp.fPosition.name());
                    }
                    gpArgs->fPositionVar = {"position",
                                            gp.fNeedsPerspective ? kFloat3_GrSLType
                                                                 : kFloat2_GrSLType,
                                            GrShaderVar::kNone_TypeModifier};
                } else {
                    // No coverage to eliminate
                    gpArgs->fPositionVar = gp.fPosition.asShaderVar();
                }

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
                    // The color cannot be flat if the varying coverage has been modulated into it
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor,
                            gp.fCoverageMode == CoverageMode::kWithColor ?
                            Interpolation::kInterpolated : Interpolation::kCanBeFlat);
                } else {
                    // Output color must be initialized to something
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
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
                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    GrGLSLVarying coverage(kFloat_GrSLType);
                    args.fVaryingHandler->addVarying("coverage", &coverage);
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("%s = %s.w;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("%s = %s.z;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    }

                    args.fFragBuilder->codeAppendf("%s = half4(half(%s));",
                                                   args.fOutputCoverage, coverage.fsIn());
                } else {
                    // Set coverage to 1, since it's either non-AA or the coverage was already
                    // folded into the output color
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
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
        SkASSERT(!spec.hasDomain());
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
        SkASSERT(spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    void initializeAttrs(const VertexSpec& spec) {
        fNeedsPerspective = spec.deviceDimensionality() == 3;
        fCoverageMode = get_mode_for_spec(spec);

        if (fCoverageMode == CoverageMode::kWithPosition) {
            if (fNeedsPerspective) {
                fPosition = {"positionWithCoverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            } else {
                fPosition = {"positionWithCoverage", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            }
        } else {
            if (fNeedsPerspective) {
                fPosition = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            } else {
                fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
            }
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

        this->setVertexAttributes(&fPosition, 4);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition; // May contain coverage as last channel
    Attribute fColor; // May have coverage modulated in if the FPs support it
    Attribute fLocalCoord;
    Attribute fDomain;

    // The positions attribute may have coverage built into it, so float3 is an ambiguous type
    // and may mean 2d with coverage, or 3d with no coverage
    bool fNeedsPerspective;
    CoverageMode fCoverageMode;

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
