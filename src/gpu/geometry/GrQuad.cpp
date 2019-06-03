/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrQuad.h"

#include "include/core/SkMatrix.h"
#include "include/private/GrTypesPriv.h"

using V4f = skvx::Vec<4, float>;
using M4f = skvx::Vec<4, int32_t>;

static bool aa_affects_rect(float ql, float qt, float qr, float qb) {
    return !SkScalarIsInt(ql) || !SkScalarIsInt(qr) || !SkScalarIsInt(qt) || !SkScalarIsInt(qb);
}

static void map_rect_translate_scale(const SkRect& rect, const SkMatrix& m,
                                     V4f* xs, V4f* ys) {
    SkMatrix::TypeMask tm = m.getType();
    SkASSERT(tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask));

    V4f r = V4f::Load(&rect);
    if (tm > SkMatrix::kIdentity_Mask) {
        const V4f t{m.getTranslateX(), m.getTranslateY(), m.getTranslateX(), m.getTranslateY()};
        if (tm <= SkMatrix::kTranslate_Mask) {
            r += t;
        } else {
            const V4f s{m.getScaleX(), m.getScaleY(), m.getScaleX(), m.getScaleY()};
            r = r * s + t;
        }
    }
    *xs = skvx::shuffle<0, 0, 2, 2>(r);
    *ys = skvx::shuffle<1, 3, 1, 3>(r);
}

static void map_quad_general(const V4f& qx, const V4f& qy, const SkMatrix& m,
                             V4f* xs, V4f* ys, V4f* ws) {
    *xs = mad(m.getScaleX(), qx, mad(m.getSkewX(), qy, m.getTranslateX()));
    *ys = mad(m.getSkewY(), qx, mad(m.getScaleY(), qy, m.getTranslateY()));
    if (m.hasPerspective()) {
        V4f w = mad(m.getPerspX(), qx,
                    mad(m.getPerspY(), qy, m.get(SkMatrix::kMPersp2)));
        if (ws) {
            // Output the calculated w coordinates
            *ws = w;
        } else {
            // Apply perspective division immediately
            V4f iw = 1.f / w;
            *xs *= iw;
            *ys *= iw;
        }
    } else if (ws) {
        *ws = 1.f;
    }
}

static void map_rect_general(const SkRect& rect, const SkMatrix& matrix,
                             V4f* xs, V4f* ys, V4f* ws) {
    V4f rx{rect.fLeft, rect.fLeft, rect.fRight, rect.fRight};
    V4f ry{rect.fTop, rect.fBottom, rect.fTop, rect.fBottom};
    map_quad_general(rx, ry, matrix, xs, ys, ws);
}

// Rearranges (top-left, top-right, bottom-right, bottom-left) ordered skQuadPts into xs and ys
// ordered (top-left, bottom-left, top-right, bottom-right)
static void rearrange_sk_to_gr_points(const SkPoint skQuadPts[4], V4f* xs, V4f* ys) {
    *xs = V4f{skQuadPts[0].fX, skQuadPts[3].fX, skQuadPts[1].fX, skQuadPts[2].fX};
    *ys = V4f{skQuadPts[0].fY, skQuadPts[3].fY, skQuadPts[1].fY, skQuadPts[2].fY};
}

// If an SkRect is transformed by this matrix, what class of quad is required to represent it.
static GrQuad::Type quad_type_for_transformed_rect(const SkMatrix& matrix) {
    if (matrix.rectStaysRect()) {
        return GrQuad::Type::kAxisAligned;
    } else if (matrix.preservesRightAngles()) {
        return GrQuad::Type::kRectilinear;
    } else if (matrix.hasPerspective()) {
        return GrQuad::Type::kPerspective;
    } else {
        return GrQuad::Type::kGeneral;
    }
}

// Perform minimal analysis of 'pts' (which are suitable for MakeFromSkQuad), and determine a
// quad type that will be as minimally general as possible.
static GrQuad::Type quad_type_for_points(const SkPoint pts[4], const SkMatrix& matrix) {
    if (matrix.hasPerspective()) {
        return GrQuad::Type::kPerspective;
    }
    // If 'pts' was formed by SkRect::toQuad() and not transformed further, it is safe to use the
    // quad type derived from 'matrix'. Otherwise don't waste any more time and assume kStandard
    // (most general 2D quad).
    if ((pts[0].fX == pts[3].fX && pts[1].fX == pts[2].fX) &&
        (pts[0].fY == pts[1].fY && pts[2].fY == pts[3].fY)) {
        return quad_type_for_transformed_rect(matrix);
    } else {
        return GrQuad::Type::kGeneral;
    }
}

GrQuad GrQuad::MakeFromRect(const SkRect& rect, const SkMatrix& m) {
    V4f x, y, w;
    SkMatrix::TypeMask tm = m.getType();
    Type type;
    if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        map_rect_translate_scale(rect, m, &x, &y);
        w = 1.f;
        type = Type::kAxisAligned;
    } else {
        map_rect_general(rect, m, &x, &y, &w);
        type = quad_type_for_transformed_rect(m);
    }
    return GrQuad(x, y, w, type);
}

GrQuad GrQuad::MakeFromSkQuad(const SkPoint pts[4], const SkMatrix& matrix) {
    V4f xs, ys;
    rearrange_sk_to_gr_points(pts, &xs, &ys);
    Type type = quad_type_for_points(pts, matrix);
    if (matrix.isIdentity()) {
        return GrQuad(xs, ys, 1.f, type);
    } else {
        V4f mx, my, mw;
        map_quad_general(xs, ys, matrix, &mx, &my, &mw);
        return GrQuad(mx, my, mw, type);
    }
}

bool GrQuad::aaHasEffectOnRect() const {
    SkASSERT(this->quadType() == Type::kAxisAligned);
    // If rect, ws must all be 1s so no need to divide
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

// Since the local quad may not be type kRect, this uses the opposites for each vertex when
// interpolating, and calculates new ws in addition to new xs, ys.
static void interpolate_local(float alpha, int v0, int v1, int v2, int v3,
                              float lx[4], float ly[4], float lw[4]) {
    // Don't compute local coords if not requested
    if (!lx) {
        SkASSERT(!ly && !lw);
        return;
    }
    SkASSERT(ly && lw);

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
            float alpha = (x[v2] - clipDevRect.fLeft) / (x[v2] - x[v0]);
            interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            x[v0] = clipDevRect.fLeft;
            x[v1] = clipDevRect.fLeft;
            return true;
        } else if (x[v0] > clipDevRect.fRight && x[v2] <= clipDevRect.fRight) {
            // Overlapping with right edge of clipDevRect
            float alpha = (clipDevRect.fRight - x[v2]) / (x[v0] - x[v2]);
            interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            x[v0] = clipDevRect.fRight;
            x[v1] = clipDevRect.fRight;
            return true;
        }
    } else {
        // A horizontal edge
        if (y[v0] < clipDevRect.fTop && y[v2] >= clipDevRect.fTop) {
            // Overlapping with top edge of clipDevRect
            float alpha = (y[v2] - clipDevRect.fTop) / (y[v2] - y[v0]);
            interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
            y[v0] = clipDevRect.fTop;
            y[v1] = clipDevRect.fTop;
            return true;
        } else if (y[v0] > clipDevRect.fBottom && y[v2] <= clipDevRect.fBottom) {
            // Overlapping with bottom edge of clipDevRect
            float alpha = (clipDevRect.fBottom - y[v2]) / (y[v0] - y[v2]);
            interpolate_local(alpha, v0, v1, v2, v3, lx, ly, lw);
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

bool GrQuad::crop(const SkRect& clipDevRect, GrAA clipAA, GrQuadAAFlags* edgeFlags,
                       GrQuad* local) {
    if (fType == Type::kAxisAligned) {
        if (local) {
            crop_rect(clipDevRect, clipAA, edgeFlags, this->fX, this->fY,
                      local->fX, local->fY, local->fW);
        } else {
            crop_rect(clipDevRect, clipAA, edgeFlags, this->fX, this->fY,
                      nullptr, nullptr, nullptr);
        }
        return true;
    }

    if (local) {
        // FIXME (michaelludwig) Calculate cropped local coordinates when not kRect
        return false;
    }

    V4f devX = this->x4f();
    V4f devY = this->y4f();
    V4f devIW = this->iw4f();
    // Project the 3D coordinates to 2D
    if (this->quadType() == Type::kPerspective) {
        devX *= devIW;
        devY *= devIW;
    }

    V4f clipX = {clipDevRect.fLeft, clipDevRect.fLeft, clipDevRect.fRight, clipDevRect.fRight};
    V4f clipY = {clipDevRect.fTop, clipDevRect.fBottom, clipDevRect.fTop, clipDevRect.fBottom};

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
        clipX.store(fX);
        clipY.store(fY);
        fW[0] = 1.f;
        fW[1] = 1.f;
        fW[2] = 1.f;
        fW[3] = 1.f;
        fType = Type::kAxisAligned;

        return true;
    }

    // FIXME (michaelludwig) - use the GrQuadPerEdgeAA tessellation inset/outset math to move
    // edges to the closest clip corner they are outside of

    return false;
}
