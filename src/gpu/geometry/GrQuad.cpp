/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrQuad.h"

#include "include/private/GrTypesPriv.h"

using V4f = skvx::Vec<4, float>;

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
static GrQuadType quad_type_for_transformed_rect(const SkMatrix& matrix) {
    if (matrix.rectStaysRect()) {
        return GrQuadType::kRect;
    } else if (matrix.preservesRightAngles()) {
        return GrQuadType::kRectilinear;
    } else if (matrix.hasPerspective()) {
        return GrQuadType::kPerspective;
    } else {
        return GrQuadType::kStandard;
    }
}

// Perform minimal analysis of 'pts' (which are suitable for MakeFromSkQuad), and determine a
// quad type that will be as minimally general as possible.
static GrQuadType quad_type_for_points(const SkPoint pts[4], const SkMatrix& matrix) {
    if (matrix.hasPerspective()) {
        return GrQuadType::kPerspective;
    }
    // If 'pts' was formed by SkRect::toQuad() and not transformed further, it is safe to use the
    // quad type derived from 'matrix'. Otherwise don't waste any more time and assume kStandard
    // (most general 2D quad).
    if ((pts[0].fX == pts[3].fX && pts[1].fX == pts[2].fX) &&
        (pts[0].fY == pts[1].fY && pts[2].fY == pts[3].fY)) {
        return quad_type_for_transformed_rect(matrix);
    } else {
        return GrQuadType::kStandard;
    }
}

template <typename Q>
void GrResolveAATypeForQuad(GrAAType requestedAAType, GrQuadAAFlags requestedEdgeFlags,
                            const Q& quad, GrAAType* outAAType, GrQuadAAFlags* outEdgeFlags) {
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
                if (quad.quadType() == GrQuadType::kRect && !quad.aaHasEffectOnRect()) {
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

// Instantiate GrResolve... for GrQuad and GrPerspQuad
template void GrResolveAATypeForQuad(GrAAType, GrQuadAAFlags, const GrQuad&,
                                     GrAAType*, GrQuadAAFlags*);
template void GrResolveAATypeForQuad(GrAAType, GrQuadAAFlags, const GrPerspQuad&,
                                     GrAAType*, GrQuadAAFlags*);

GrQuad GrQuad::MakeFromRect(const SkRect& rect, const SkMatrix& m) {
    V4f x, y;
    SkMatrix::TypeMask tm = m.getType();
    GrQuadType type;
    if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        map_rect_translate_scale(rect, m, &x, &y);
        type = GrQuadType::kRect;
    } else {
        map_rect_general(rect, m, &x, &y, nullptr);
        type = quad_type_for_transformed_rect(m);
        if (type == GrQuadType::kPerspective) {
            // While the matrix created perspective, the coordinates were projected to a 2D quad
            // in map_rect_general since no w V4f was provided.
            type = GrQuadType::kStandard;
        }
    }
    return GrQuad(x, y, type);
}

GrQuad GrQuad::MakeFromSkQuad(const SkPoint pts[4], const SkMatrix& matrix) {
    V4f xs, ys;
    rearrange_sk_to_gr_points(pts, &xs, &ys);
    GrQuadType type = quad_type_for_points(pts, matrix);
    if (type == GrQuadType::kPerspective) {
        // While the matrix created perspective, the coordinates were projected to a 2D quad
        // in map_rect_general since no w V4f was provided.
        type = GrQuadType::kStandard;
    }

    if (matrix.isIdentity()) {
        return GrQuad(xs, ys, type);
    } else {
        V4f mx, my;
        map_quad_general(xs, ys, matrix, &mx, &my, nullptr);
        return GrQuad(mx, my, type);
    }
}

bool GrQuad::aaHasEffectOnRect() const {
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}

// Private constructor used by GrQuadList to quickly fill in a quad's values from the channel arrays
GrPerspQuad::GrPerspQuad(const float* xs, const float* ys, const float* ws, GrQuadType type)
        : fType(type) {
    memcpy(fX, xs, 4 * sizeof(float));
    memcpy(fY, ys, 4 * sizeof(float));
    memcpy(fW, ws, 4 * sizeof(float));
}

GrPerspQuad GrPerspQuad::MakeFromRect(const SkRect& rect, const SkMatrix& m) {
    V4f x, y, w;
    SkMatrix::TypeMask tm = m.getType();
    GrQuadType type;
    if (tm <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        map_rect_translate_scale(rect, m, &x, &y);
        w = 1.f;
        type = GrQuadType::kRect;
    } else {
        map_rect_general(rect, m, &x, &y, &w);
        type = quad_type_for_transformed_rect(m);
    }
    return GrPerspQuad(x, y, w, type);
}

GrPerspQuad GrPerspQuad::MakeFromSkQuad(const SkPoint pts[4], const SkMatrix& matrix) {
    V4f xs, ys;
    rearrange_sk_to_gr_points(pts, &xs, &ys);
    GrQuadType type = quad_type_for_points(pts, matrix);
    if (matrix.isIdentity()) {
        return GrPerspQuad(xs, ys, 1.f, type);
    } else {
        V4f mx, my, mw;
        map_quad_general(xs, ys, matrix, &mx, &my, &mw);
        return GrPerspQuad(mx, my, mw, type);
    }
}

bool GrPerspQuad::aaHasEffectOnRect() const {
    // If rect, ws must all be 1s so no need to divide
    return aa_affects_rect(fX[0], fY[0], fX[3], fY[3]);
}
