/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkTPin.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <unordered_set>

static SkPaint paint(SkColor color,
                     float strokeWidth = -1.f,
                     SkPaint::Join join = SkPaint::kMiter_Join) {
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    if (strokeWidth >= 0.f) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(strokeWidth);
        paint.setStrokeJoin(join);
    }
    return paint;
}

// Singular values for [a b][c d] 2x2 matrix, unordered.
static std::pair<float, float> singular_values(float a, float b, float c, float d) {
    float s1 = a*a + b*b + c*c + d*d;

    float e = a*a + b*b - c*c - d*d;
    float f = a*c + b*d;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    float singular1 = SkScalarSqrt(0.5f * (s1 + s2));
    float singular2 = SkScalarSqrt(0.5f * (s1 - s2));

    return {singular1, singular2};
}

static constexpr float kAARadius = 10.f;

//              [m00 m01 * m03]                                 [f(u,v)]
// Assuming M = [m10 m11 * m13], define the projected p'(u,v) = [g(u,v)] where
//              [ *   *  *  * ]
//              [m30 m31 * m33]
//                                                        [x]     [u]
// f(u,v) = x(u,v) / w(u,v), g(u,v) = y(u,v) / w(u,v) and [y] = M*[v]
//                                                        [*] =   [0]
//                                                        [w]     [1]
//
// x(u,v) = m00*u + m01*v + m03
// y(u,v) = m10*u + m11*v + m13
// w(u,v) = m30*u + m31*v + m33
//
// dx/du = m00, dx/dv = m01,
// dy/du = m10, dy/dv = m11
// dw/du = m30, dw/dv = m31
//
// df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2
// df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2
// dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2
// dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2
//
// Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
//                    [dg/du dg/dv]
// for M evaluated at  (u,v)
static float local_aa_radius(const SkM44& matrix, const SkV2& p) {
    SkV4 devP = matrix.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = matrix.rc(0,0);
    const float dxdv = matrix.rc(0,1);
    const float dydu = matrix.rc(1,0);
    const float dydv = matrix.rc(1,1);
    const float dwdu = matrix.rc(3,0);
    const float dwdv = matrix.rc(3,1);

    float invW2 = 1.f / (devP.w * devP.w);
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, this is the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    // and equivalent to SkMatrix::getMinmaxScales().
    auto [sv1, sv2] = singular_values(dfdu, dfdv, dgdu, dgdv);

    // The minimum and maximum singular values of the above matrix represent the min and maximum
    // scale factors that could be applied by the 'matrix'. So if 'p' is moved 1px locally it will
    // move between [min, max]px after transformation. Thus, moving 1/min px locally will move
    // between [1, max/min]px after transformation, ensuring the device-space offset exceeds the
    // minimum AA offset for analytic AA.
    float minScale = std::min(sv1, sv2);
    return kAARadius / minScale;
}

static constexpr float kMiterScale = 1.f;
static constexpr float kBevelScale = 0.0f;
static constexpr float kRoundScale = SK_FloatSqrt2 - 1.f;

struct LocalCornerVert {
    SkV2 fPosition;     // In unit square that each corner is normalized to
    SkV2 fNormal;       // Direction that AA outset is applied in

    float fStrokeScale; // Signed scale factor applied to external stroke radius, should be [-1,1]
    float fMirrorScale; // Scale fPosition.yx, along with external join-scale, should be [0,1].
    float fCenterWeight; // Added to external center scale, > 0 forces point to center instead.

    // 'cornerMapping' is a row-major 2x2 matrix [[x y], [z w]] to flip and rotate the normalized
    // positions into the local coord space.
    SkV3 transform(const SkM44& m, const SkV4& cornerMapping, const SkV2& cornerPt,
                   const SkV2& cornerRadii, const SkV4& devCenter, float centerWeight,
                   float strokeRadius, float joinScale, float localAARadius) const {
        const bool snapToCenter = centerWeight + fCenterWeight > 0.f;
        if (snapToCenter) {
            return {devCenter.x, devCenter.y, devCenter.w};
        } else {
            // Normalized position before any additional AA offsets
            SkV2 normalizedPos = fPosition + joinScale*fMirrorScale*SkV2{fPosition.y, fPosition.x};
            // scales the normalized unit corner to the actual radii of the corner, before any AA
            // offsets are added.
            SkV2 scale = cornerRadii + SkV2{fStrokeScale*strokeRadius, fStrokeScale*strokeRadius};
            normalizedPos = scale*normalizedPos - cornerRadii;

            if (fStrokeScale < 0.f) {
                // An inset, which means it might cross over or might be forced to the center
                SkV2 maxInset = scale - SkV2{localAARadius, localAARadius};
                if (maxInset.x < 0.f || maxInset.y < 0.f) {
                    normalizedPos =
                            SkV2{std::min(maxInset.x, 0.f), std::min(maxInset.y, 0.f)}
                            - cornerRadii;
                } else {
                    normalizedPos += localAARadius * fNormal;
                }
            } // else no normal offsetting, or device-space offsetting

            SkV2 localPos =
                   {cornerMapping.x*normalizedPos.x + cornerMapping.y*normalizedPos.y + cornerPt.x,
                    cornerMapping.z*normalizedPos.x + cornerMapping.w*normalizedPos.y + cornerPt.y};
            SkV4 devPos = m.map(localPos.x, localPos.y, 0.f, 1.f);

            const bool deviceSpaceNormal =
                    fStrokeScale > 0.f && (fNormal.x > 0.f || fNormal.y > 0.f);
            if (deviceSpaceNormal) {
                SkV2 devNorm;
                {
                    // To calculate a device-space normal, we use the normal matrix (A^-1)^T where
                    // A is CTM * T(cornerPt) * cornerMapping * scale. We inline the calculation
                    // of (T(cornerPt)*cornerMapping*scale)^-1^T * [nx, ny, 0, 0] = N', which means
                    // that CTM^-1^T * N' is equivalent to N'^T*CTM^-1, which can be calculated with
                    // two dot products if the CTM inverse is uploaded to the GPU.

                    // We add epsilon so that rectangular corners are not degenerate, and circular
                    // corners remain unmodified. This only slightly increases inaccuracy for
                    // elliptical corners.
                    float sx = (scale.y + SK_ScalarNearlyZero) / (scale.x + SK_ScalarNearlyZero);
                    // Needed to calculate intermediate W of transformed normal.
                    float px = cornerMapping.y*cornerPt.y - cornerMapping.w*cornerPt.x;
                    float py = cornerMapping.z*cornerPt.x - cornerMapping.x*cornerPt.y;
                    // Inverse CTM, presumably calculated once as a uniform
                    SkM44 inv;
                    SkAssertResult(m.invert(&inv));

                    SkV4 normX4 = { sx*cornerMapping.w*fNormal.x,
                                   -sx*cornerMapping.y*fNormal.x,
                                   0.f,
                                   sx*px*fNormal.x};
                    SkV4 normY4 = {-cornerMapping.z*fNormal.y,
                                    cornerMapping.x*fNormal.y,
                                   0.f,
                                   py*fNormal.y};

                    SkV2 normX = {inv.col(0).dot(normX4), inv.col(1).dot(normX4)};
                    SkV2 normY = {inv.col(0).dot(normY4), inv.col(1).dot(normY4)};

                    if (joinScale == kMiterScale && fNormal.x > 0.f && fNormal.y > 0.f) {
                        // normX and normY represent adjacent edges' normals, so if we normalize
                        // them before adding together, we'll have a vector that bisects the edge
                        // normals instead of a vector matching fNormal, which is what we want when
                        // we're at a miter corner.
                        normX = normX.normalize();
                        normY = normY.normalize();
                        if (normX.dot(normY) < -0.8) {
                            // Nearly opposite directions, so the sum could have cancellation, so
                            // instead bisect orthogonal vectors and flip to keep consistent
                            float sign = normX.cross(normY) >= 0.f ? 1.f : -1.f;
                            normX = sign*SkV2{-normX.y, normX.x};
                            normY = sign*SkV2{normY.y, -normY.x};
                        }
                    }

                    devNorm = (normX + normY).normalize();
                }

                // The local coordinates for a device-space AA outset are clamped to the non-outset
                // point, which means we don't care about remaining in the same pre-homogenous
                // divide plane. This makes it very easy to determine a homogenous coordinate that
                // projects to the correct device-space position.
                devPos.x += devPos.w * kAARadius * devNorm.x;
                devPos.y += devPos.w * kAARadius * devNorm.y;
            }

            return SkV3{devPos.x, devPos.y, devPos.w};
        }
    }
};

static constexpr float kHR2 = SK_ScalarRoot2Over2; // "half root 2"

static constexpr LocalCornerVert kCornerTemplate[19] = {
    // Stroke-scale should be -1, 0, or 1.
    // Mirror-scale should be 0 or 1.
    // Center-weight should be -2 to never snap to center, -1 to snap when stroke coords would
    //     overlap, and 0 to snap for fill-style or overlapping coords.
    // Local-aa-scale should be 0 or 1.

    //  position,       normal,   stroke-scale   mirror-scale   center-weight
    // Device-space AA outsets from outer curve
    { {0.0f, 1.0f}, { 0.0f,  1.0f},  1.0f,          0.0f,           -2.f  },
    { {0.0f, 1.0f}, { 0.0f,  1.0f},  1.0f,          1.0f,           -2.f  },
    { {0.0f, 1.0f}, { kHR2,  kHR2},  1.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { kHR2,  kHR2},  1.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 1.0f,  0.0f},  1.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 1.0f,  0.0f},  1.0f,          0.0f,           -2.f  },

    // Outer anchors (no local or device-space normal outset)
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  1.0f,          0.0f,           -2.f  },
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  1.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  1.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  1.0f,          0.0f,           -2.f  },

    // Center of stroke (equivalent to outer anchors when filling)
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  0.0f,          0.0f,           -2.f  },
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  0.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  0.0f,          1.0f,           -2.f  },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  0.0f,          0.0f,           -2.f  },

    // Inner AA insets from inner curve
    { {0.0f, 1.0f}, { 0.0f, -1.0f}, -1.0f,          0.0f,           -1.f  },
    { {0.5f, 0.5f}, {-kHR2, -kHR2}, -1.0f,          1.0f,           -1.f  },
    { {1.0f, 0.0f}, {-1.0f,  0.0f}, -1.0f,          0.0f,           -1.f  },

    // Center filling vertices (equal to inner AA insets unless center-weight = 1)
    { {0.5f, 0.5f}, {-kHR2, -kHR2}, -1.0f,          1.0f,            0.f  },
    { {1.0f, 0.0f}, {-1.0f,  0.0f}, -1.0f,          0.0f,            0.f  },
};

static void compute_corner(SkV3 devPts[19], const SkM44& m, const SkV4& cornerMapping,
                           const SkV2& cornerPt, const SkV2& cornerRadii, const SkV4& center,
                           float centerWeight, float localAARadius, float strokeRadius,
                           SkPaint::Join join) {
    float joinScale;

    // TODO: checking against localAARadius can snap to rect corner unexpectedly under high skew
    // because localAARadius gets so big, but would be nice to be fuzzy here.
    if (cornerRadii.x <= 0.f || cornerRadii.y <= 0.f) {
        // Effectively a rectangular corner
        joinScale = kMiterScale; // default for rect corners
        if (strokeRadius > 0.f) {
            // Non-hairline strokes need to adjust the join scale factor to match style.
            if (join == SkPaint::kBevel_Join) {
                joinScale = kBevelScale;
            } else if (join == SkPaint::kRound_Join) {
                joinScale = kRoundScale;
            }
        }
    } else {
        // Rounded filled corner vertices are always positioned for a round join since the
        // underlying geometry has no real tangent discontinuity.
        joinScale = kRoundScale;
    }

    for (size_t i = 0; i < std::size(kCornerTemplate); ++i) {
        devPts[i] = kCornerTemplate[i].transform(m, cornerMapping, cornerPt, cornerRadii,
                                                 center, centerWeight, strokeRadius, joinScale,
                                                 localAARadius);
    }
}

static const uint16_t kBR = 0*std::size(kCornerTemplate);
static const uint16_t kTR = 1*std::size(kCornerTemplate);
static const uint16_t kTL = 2*std::size(kCornerTemplate);
static const uint16_t kBL = 3*std::size(kCornerTemplate);
static const size_t kVertexCount = 4*std::size(kCornerTemplate);
static void compute_vertices(SkV3 devPts[kVertexCount],
                             const SkM44& m,
                             const SkRRect& rrect,
                             float strokeRadius,
                             SkPaint::Join join) {
    SkV4 devCenter = m.map(rrect.getBounds().centerX(), rrect.getBounds().centerY(), 0.f, 1.f);

    float localAARadius = std::max({
            local_aa_radius(m, {rrect.getBounds().fRight, rrect.getBounds().fBottom}),
            local_aa_radius(m, {rrect.getBounds().fRight, rrect.getBounds().fTop}),
            local_aa_radius(m, {rrect.getBounds().fLeft, rrect.getBounds().fTop}),
            local_aa_radius(m, {rrect.getBounds().fLeft, rrect.getBounds().fBottom})
        });

    float centerWeight = 0.f; // No center snapping
    if (strokeRadius < 0.f) {
        // A fill, so inner vertices need to snap to the center and then adjust the stroke radius
        // to 0 for later math to work out nicely.
        strokeRadius = 0.f;
        centerWeight = 1.f;
    }

    // Check if the inset amount (max stroke-radius + local-aa-radius) would interfere with the
    // opposite edge's inset or interfere with the adjacent corner's curve. When this happens, snap
    // all the interior vertices to the center and let the fragment shader work through it.
    // TODO: Could force centerWeight = 2 for filled rects and quads for simplicity around non
    // orthogonal inset overlap calculations.
    float maxInset = strokeRadius + localAARadius;
    if (maxInset >= rrect.width() - maxInset || // L/R stroke insets would cross over
        maxInset >= rrect.height() - maxInset || // T/B stroke insets would cross over
        maxInset >= rrect.width() - rrect.radii(SkRRect::kLowerLeft_Corner).fX || // X corner cross
        maxInset >= rrect.width() - rrect.radii(SkRRect::kLowerRight_Corner).fX ||
        maxInset >= rrect.width() - rrect.radii(SkRRect::kUpperLeft_Corner).fX ||
        maxInset >= rrect.width() - rrect.radii(SkRRect::kUpperRight_Corner).fX ||
        maxInset >= rrect.height() - rrect.radii(SkRRect::kLowerLeft_Corner).fY || // Y corner cross
        maxInset >= rrect.height() - rrect.radii(SkRRect::kLowerRight_Corner).fY ||
        maxInset >= rrect.height() - rrect.radii(SkRRect::kUpperLeft_Corner).fY ||
        maxInset >= rrect.height() - rrect.radii(SkRRect::kUpperRight_Corner).fY) {
        // All interior vertices need to snap to the center
        centerWeight = 2.f;
    }

    // The normalized corner template is defined relative to the quarter circle with positive X
    // and positive Y, with a counter clockwise winding (if +Y points down). This corresponds to
    // the bottom-right corner.
    static constexpr SkV4 kBRBasis = { 1.f,  0.f,  0.f,  1.f};
    static constexpr SkV4 kTRBasis = { 0.f,  1.f, -1.f,  0.f};
    static constexpr SkV4 kTLBasis = {-1.f,  0.f,  0.f, -1.f};
    static constexpr SkV4 kBLBasis = { 0.f, -1.f,  1.f,  0.f};

    compute_corner(devPts + kBR, m, kBRBasis,
                   {rrect.getBounds().fRight,
                    rrect.getBounds().fBottom},
                   {rrect.radii(SkRRect::kLowerRight_Corner).fX,
                    rrect.radii(SkRRect::kLowerRight_Corner).fY},
                   devCenter, centerWeight, localAARadius, strokeRadius, join);
    compute_corner(devPts + kTR, m, kTRBasis,
                   {rrect.getBounds().fRight,
                    rrect.getBounds().fTop},
                   {rrect.radii(SkRRect::kUpperRight_Corner).fY,
                    rrect.radii(SkRRect::kUpperRight_Corner).fX},
                   devCenter, centerWeight, localAARadius,strokeRadius, join);
    compute_corner(devPts + kTL, m, kTLBasis,
                   {rrect.getBounds().fLeft,
                    rrect.getBounds().fTop},
                   {rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                    rrect.radii(SkRRect::kUpperLeft_Corner).fY},
                   devCenter, centerWeight, localAARadius,strokeRadius, join);
    compute_corner(devPts + kBL, m, kBLBasis,
                   {rrect.getBounds().fLeft,
                    rrect.getBounds().fBottom},
                   {rrect.radii(SkRRect::kLowerLeft_Corner).fY,
                    rrect.radii(SkRRect::kLowerLeft_Corner).fX},
                   devCenter, centerWeight, localAARadius,strokeRadius, join);
}

// All indices
static const uint16_t kIndices[] = {
    // Exterior AA ramp outset
    kBR+0,kBR+6,kBR+1,kBR+7,kBR+2,kBR+8,kBR+3,kBR+8,kBR+4,kBR+9,kBR+5,kBR+9,
    kTR+0,kTR+6,kTR+1,kTR+7,kTR+2,kTR+8,kTR+3,kTR+8,kTR+4,kTR+9,kTR+5,kTR+9,
    kTL+0,kTL+6,kTL+1,kTL+7,kTL+2,kTL+8,kTL+3,kTL+8,kTL+4,kTL+9,kTL+5,kTL+9,
    kBL+0,kBL+6,kBL+1,kBL+7,kBL+2,kBL+8,kBL+3,kBL+8,kBL+4,kBL+9,kBL+5,kBL+9,
    kBR+0,kBR+6,kBR+6, // close and extra vertex to jump to next strip
    // Outer to central curve
    kBR+6,kBR+10,kBR+7,kBR+11,kBR+8,kBR+12,kBR+9,kBR+13,
    kTR+6,kTR+10,kTR+7,kTR+11,kTR+8,kTR+12,kTR+9,kTR+13,
    kTL+6,kTL+10,kTL+7,kTL+11,kTL+8,kTL+12,kTL+9,kTL+13,
    kBL+6,kBL+10,kBL+7,kBL+11,kBL+8,kBL+12,kBL+9,kBL+13,
    kBR+6,kBR+10,kBR+10, // close and extra vertex to jump to next strip
    // Center to inner curve's insets
    kBR+10,kBR+14,kBR+11,kBR+15,kBR+12,kBR+16,kBR+13,kBR+16,
    kTR+10,kTR+14,kTR+11,kTR+15,kTR+12,kTR+16,kTR+13,kTR+16,
    kTL+10,kTL+14,kTL+11,kTL+15,kTL+12,kTL+16,kTL+13,kTL+16,
    kBL+10,kBL+14,kBL+11,kBL+15,kBL+12,kBL+16,kBL+13,kBL+16,
    kBR+10,kBR+14,kBR+14, // close and extra vertex to jump to next strip
    // Inner inset to center of shape
    kBR+14,kBR+17,kBR+15,kBR+17,kBR+16,kBR+16,kBR+18,kTR+14,
    kTR+14,kTR+17,kTR+15,kTR+17,kTR+16,kTR+16,kTR+18,kTL+14,
    kTL+14,kTL+17,kTL+15,kTL+17,kTL+16,kTL+16,kTL+18,kBL+14,
    kBL+14,kBL+17,kBL+15,kBL+17,kBL+16,kBL+16,kBL+18,kBR+14 // close
};

// Separated to draw with different colors (vs. duplicating vertices to change colors).
static const uint16_t kOuterCornerIndices[] = {
    kBR+0,  kBR+0,kBR+6,kBR+1,kBR+7,kBR+2,kBR+8,kBR+3,kBR+8,kBR+4,kBR+9,kBR+5,  kBR+5,
    kTR+0,  kTR+0,kTR+6,kTR+1,kTR+7,kTR+2,kTR+8,kTR+3,kTR+8,kTR+4,kTR+9,kTR+5,  kTR+5,
    kTL+0,  kTL+0,kTL+6,kTL+1,kTL+7,kTL+2,kTL+8,kTL+3,kTL+8,kTL+4,kTL+9,kTL+5,  kTL+5,
    kBL+0,  kBL+0,kBL+6,kBL+1,kBL+7,kBL+2,kBL+8,kBL+3,kBL+8,kBL+4,kBL+9,kBL+5,  kBL+5,

    kBR+6,  kBR+6,kBR+10,kBR+7,kBR+11,kBR+8,kBR+12,kBR+9,kBR+13,  kBR+13,
    kTR+6,  kTR+6,kTR+10,kTR+7,kTR+11,kTR+8,kTR+12,kTR+9,kTR+13,  kTR+13,
    kTL+6,  kTL+6,kTL+10,kTL+7,kTL+11,kTL+8,kTL+12,kTL+9,kTL+13,  kTL+13,
    kBL+6,  kBL+6,kBL+10,kBL+7,kBL+11,kBL+8,kBL+12,kBL+9,kBL+13,  kBL+13
};

static const uint16_t kInnerCornerIndices[] = {
    kBR+10,  kBR+10,kBR+14,kBR+11,kBR+15,kBR+12,kBR+16,kBR+13,  kBR+13,
    kTR+10,  kTR+10,kTR+14,kTR+11,kTR+15,kTR+12,kTR+16,kTR+13,  kTR+13,
    kTL+10,  kTL+10,kTL+14,kTL+11,kTL+15,kTL+12,kTL+16,kTL+13,  kTL+13,
    kBL+10,  kBL+10,kBL+14,kBL+11,kBL+15,kBL+12,kBL+16,kBL+13,  kBL+13,
};

static const uint16_t kInteriorIndices[] = {
    kBR+14,kBR+17,kBR+15,kBR+17,kBR+16,kBR+16,kBR+18,kTR+14,
    kTR+14,kTR+17,kTR+15,kTR+17,kTR+16,kTR+16,kTR+18,kTL+14,
    kTL+14,kTL+17,kTL+15,kTL+17,kTL+16,kTL+16,kTL+18,kBL+14,
    kBL+14,kBL+17,kBL+15,kBL+17,kBL+16,kBL+16,kBL+18,kBR+14 // close
};

// Implicit in the original mesh from the tri-strip connections between corners
static const uint16_t kEdgeIndices[] = {
    kBR+5,   kBR+5,kBR+9,kTR+0,kTR+6,      kTR+6,
    kBR+9,   kBR+9,kBR+13,kTR+6,kTR+10,    kTR+10,
    kBR+13,  kBR+13,kBR+16,kTR+10,kTR+14,  kTR+14,

    kTR+5,   kTR+5,kTR+9,kTL+0,kTL+6,      kTL+6,
    kTR+9,   kTR+9,kTR+13,kTL+6,kTL+10,    kTL+10,
    kTR+13,  kTR+13,kTR+16,kTL+10,kTL+14,  kTL+14,

    kTL+5,   kTL+5,kTL+9,kBL+0,kBL+6,      kBL+6,
    kTL+9,   kTL+9,kTL+13,kBL+6,kBL+10,    kBL+10,
    kTL+13,  kTL+13,kTL+16,kBL+10,kBL+14,  kBL+14,

    kBL+5,   kBL+5,kBL+9,kBR+0,kBR+6,      kBR+6,
    kBL+9,   kBL+9,kBL+13,kBR+6,kBR+10,    kBR+10,
    kBL+13,  kBL+13,kBL+16,kBR+10,kBR+14,  kBR+14,
};

class GraphitePrimitivesSlide : public ClickHandlerSlide {
    static constexpr float kControlPointRadius = 3.f;
    static constexpr float kBaseScale = 50.f;

public:
    GraphitePrimitivesSlide()
        : fOrigin{300.f, 300.f}
        , fXAxisPoint{300.f + kBaseScale, 300.f}
        , fYAxisPoint{300.f, 300.f + kBaseScale}
        , fStrokeWidth{10.f}
        , fJoinMode(SkPaint::kMiter_Join)
        , fMode(PrimitiveMode::kFillRect) {
        fName = "GraphitePrimitives";
    }

    void draw(SkCanvas* canvas) override {
        canvas->save();
        SkM44 viewMatrix = canvas->getLocalToDevice();

        canvas->concat(this->basisMatrix());

        SkM44 totalMatrix = canvas->getLocalToDevice();

            // Base shape + style
            SkRRect rrect = this->primitiveShape();
            canvas->drawRRect(rrect, paint(SK_ColorBLUE, this->strokeWidth(), fJoinMode));
        canvas->restore();

        canvas->save();
        canvas->resetMatrix();
            // Draw the full mesh directly in device space
            this->drawVertices(canvas, totalMatrix);
            // Draw the controls in device space so we get consistent circles for the click points.
            SkV4 origin = viewMatrix.map(fOrigin.x, fOrigin.y, 0.f, 1.f);
            SkV4 xAxis = viewMatrix.map(fXAxisPoint.x, fXAxisPoint.y, 0.f, 1.f);
            SkV4 yAxis = viewMatrix.map(fYAxisPoint.x, fYAxisPoint.y, 0.f, 1.f);

            // Axes
            canvas->drawLine({origin.x/origin.w, origin.y/origin.w},
                             {xAxis.x/xAxis.w, xAxis.y/xAxis.w}, paint(SK_ColorRED, 0.f));
            canvas->drawLine({origin.x/origin.w, origin.y/origin.w},
                             {yAxis.x/yAxis.w, yAxis.y/yAxis.w}, paint(SK_ColorGREEN, 0.f));

            // Control points
            canvas->drawCircle({origin.x/origin.w, origin.y/origin.w},
                               kControlPointRadius, paint(SK_ColorBLACK));
            canvas->drawCircle({xAxis.x/xAxis.w, xAxis.y/xAxis.w},
                               kControlPointRadius, paint(SK_ColorRED));
            canvas->drawCircle({yAxis.x/yAxis.w, yAxis.y/yAxis.w},
                               kControlPointRadius, paint(SK_ColorGREEN));
        canvas->restore();
    }

    bool onChar(SkUnichar) override;

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Click*) override;

private:
    class Click;

    enum class PrimitiveMode {
        kFillRect,
        kFillRRect,
        kStrokeRect,
        kStrokeRRect
    };

    // Computed from 3 control points. Concat with CTM to get total matrix.
    SkM44 basisMatrix() const {
        SkV2 xAxis = (fXAxisPoint - fOrigin) / kBaseScale;
        SkV2 yAxis = (fYAxisPoint - fOrigin) / kBaseScale;

        return SkM44::Cols({xAxis.x, xAxis.y, 0.f, 0.f},
                           {yAxis.x, yAxis.y, 0.f, 0.f},
                           {0.f, 0.f, 1.f, 0.f},
                           {fOrigin.x, fOrigin.y, 0.f, 1.f});
    }

    float strokeWidth() const {
        if (fMode == PrimitiveMode::kFillRect || fMode == PrimitiveMode::kFillRRect) {
            return -1.f;
        }
        return fStrokeWidth;
    }

    SkRRect primitiveShape() const {
        static const SkRect kOuterBounds = SkRect::MakeLTRB(-kBaseScale, -kBaseScale,
                                                            kBaseScale, kBaseScale);
        // Filled rounded rects can have arbitrary corners
        static const SkVector kOuterRadii[4] = { { 0.25f * kBaseScale, 0.75f * kBaseScale },
                                                 { 0.f, 0.f},
                                                 { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                 { 0.75f * kBaseScale, 0.25f * kBaseScale } };
        // // Stroked rounded rects will only have circular corners
        static const SkVector kStrokeRadii[4] = { { 0.25f * kBaseScale, 0.25f * kBaseScale },
                                                  { 0.f, 0.f },
                                                  { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                  { 0.75f * kBaseScale, 0.75f * kBaseScale } };

        float strokeRadius = 0.5f * fStrokeWidth;
        switch(fMode) {
            case PrimitiveMode::kFillRect:
                return SkRRect::MakeRect(kOuterBounds.makeOutset(strokeRadius, strokeRadius));
            case PrimitiveMode::kFillRRect: {
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds, kOuterRadii);
                rrect.outset(strokeRadius, strokeRadius);
                return rrect; }
            case PrimitiveMode::kStrokeRect:
                return SkRRect::MakeRect(kOuterBounds);
            case PrimitiveMode::kStrokeRRect: {
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds, kStrokeRadii);
                return rrect;
            }
        }

        SkUNREACHABLE;
    }

    void drawVertices(SkCanvas* canvas, const SkM44& ctm) {
        SkRRect rrect = this->primitiveShape();
        float strokeRadius = 0.5f * this->strokeWidth();

        SkV3 points[kVertexCount];
        SkPoint vertices[kVertexCount];
        compute_vertices(points, ctm, rrect, strokeRadius, fJoinMode);
        // SkCanvas::drawVertices() wants SkPoint, but normally we'd let the GPU handle the
        // perspective division and clipping.
        for (size_t i = 0; i < kVertexCount; ++i) {
            vertices[i] = SkPoint{points[i].x/points[i].z, points[i].y/points[i].z};
        }

        auto drawMeshSubset = [vertices, canvas](SkColor color,
                                                 const uint16_t* indices,
                                                 size_t indexCount) {
            sk_sp<SkVertices> mesh = SkVertices::MakeCopy(
                    SkVertices::kTriangleStrip_VertexMode, kVertexCount, vertices,
                    nullptr, nullptr, (int) indexCount, indices);
            SkPaint meshPaint;
            meshPaint.setColor(color);
            meshPaint.setAlphaf(0.5f);
            canvas->drawVertices(mesh, SkBlendMode::kSrc, meshPaint);
        };
        if (fColorize) {
            drawMeshSubset(SK_ColorGRAY,
                           kEdgeIndices,
                           std::size(kEdgeIndices));
            drawMeshSubset(SK_ColorDKGRAY,
                           kInteriorIndices,
                           std::size(kInteriorIndices));
            drawMeshSubset(SK_ColorMAGENTA,
                           kInnerCornerIndices,
                           std::size(kInnerCornerIndices));
            drawMeshSubset(SK_ColorCYAN,
                           kOuterCornerIndices,
                           std::size(kOuterCornerIndices));
        } else {
            drawMeshSubset(SK_ColorGRAY, kIndices, std::size(kIndices));
        }

        // Draw the edges over the triangle strip mesh, but keep track of edges already drawn so
        // that we don't oversaturate AA on edges shared by multiple triangles.
        std::unordered_set<uint32_t> edges;
        auto drawEdge = [&edges, vertices, canvas](uint16_t e0, uint16_t e1) {
            uint32_t edgeID = (std::max(e0, e1) << 16) | std::min(e0, e1);
            if (edges.find(edgeID) == edges.end()) {
                edges.insert(edgeID);
                if (SkScalarNearlyEqual(vertices[e0].fX, vertices[e1].fX) &&
                    SkScalarNearlyEqual(vertices[e0].fY, vertices[e1].fY)) {
                    return;
                }
                canvas->drawLine(vertices[e0], vertices[e1], paint(SK_ColorBLACK, 0.f));
            }
        };
        for (size_t i = 2; i < std::size(kIndices); ++i) {
            drawEdge(kIndices[i-1], kIndices[i]);
            drawEdge(kIndices[i-2], kIndices[i]);
        }
    }

    // This Sample is responsive to the entire transform of the viewer slide, including the
    // transform (rotation, scale, and perspective) selected from the widget. The 3 points below
    // define the location and basis of the local coordinate space, relative to the viewer's
    // coordinate space. This is used instead of the root canvas coordinate space because it aligns
    // with the coordinate space that the click handler operates in.
    SkV2 fOrigin;
    SkV2 fXAxisPoint;
    SkV2 fYAxisPoint;

    float fStrokeWidth;
    SkPaint::Join fJoinMode;
    PrimitiveMode fMode;
    bool fColorize = true;
};

class GraphitePrimitivesSlide::Click : public ClickHandlerSlide::Click {
public:
    Click(SkV2* point) : fPoint(point) {}

    void drag() {
        SkVector delta = fCurr - fPrev;
        *fPoint += {delta.fX, delta.fY};
    }

private:
    SkV2* fPoint;
};

ClickHandlerSlide::Click* GraphitePrimitivesSlide::onFindClickHandler(SkScalar x, SkScalar y,
                                                                      skui::ModifierKey) {
    auto selected = [x,y](const SkV2& p) {
        return ((p - SkV2{x,y}).length() < kControlPointRadius);
    };

    if (selected(fOrigin)) {
        return new Click(&fOrigin);
    } else if (selected(fXAxisPoint)) {
        return new Click(&fXAxisPoint);
    } else if (selected(fYAxisPoint)) {
        return new Click(&fYAxisPoint);
    } else {
        return nullptr;
    }
}

bool GraphitePrimitivesSlide::onClick(ClickHandlerSlide::Click* click) {
    Click* myClick = (Click*) click;
    myClick->drag();
    return true;
}

bool GraphitePrimitivesSlide::onChar(SkUnichar code) {
        switch(code) {
            case '1':
                fMode = PrimitiveMode::kFillRect;
                return true;
            case '2':
                fMode = PrimitiveMode::kFillRRect;
                return true;
            case '3':
                fMode = PrimitiveMode::kStrokeRect;
                return true;
            case '4':
                fMode = PrimitiveMode::kStrokeRRect;
                return true;
            case '-':
                fStrokeWidth = std::max(0.f, fStrokeWidth - 0.4f);
                return true;
            case '=':
                fStrokeWidth = std::min(5 * kBaseScale, fStrokeWidth + 0.4f);
                return true;
            case 'q':
                fJoinMode = SkPaint::kRound_Join;
                return true;
            case 'w':
                fJoinMode = SkPaint::kBevel_Join;
                return true;
            case 'e':
                fJoinMode = SkPaint::kMiter_Join;
                return true;
            case 'r':
                fStrokeWidth = 10.f;
                fOrigin = {300.f, 300.f};
                fXAxisPoint = {300.f + kBaseScale, 300.f};
                fYAxisPoint = {300.f, 300.f + kBaseScale};
                return true;
            case 'c':
                fColorize = !fColorize;
                return true;
        }
        return false;
}

DEF_SLIDE(return new GraphitePrimitivesSlide();)
