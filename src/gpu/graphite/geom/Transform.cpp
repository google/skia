/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Transform_graphite.h"

#include "src/base/SkVx.h"
#include "src/core/SkMatrixInvert.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <tuple>

namespace skgpu::graphite {

namespace {

Rect map_rect(const SkM44& m, const Rect& r) {
    // TODO: Can Rect's (l,t,-r,-b) structure be used to optimize mapRect?
    // TODO: Can take this opportunity to implement 100% accurate perspective plane clipping since
    //       it doesn't have to match raster/ganesh rendering behavior.
    return SkMatrixPriv::MapRect(m, r.asSkRect());
}

void map_points(const SkM44& m, const SkV4* in, SkV4* out, int count) {
    // TODO: These maybe should go into SkM44, since bulk point mapping seems generally useful
    auto c0 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 0);
    auto c1 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 4);
    auto c2 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 8);
    auto c3 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 12);

    for (int i = 0; i < count; ++i) {
        auto p = (c0 * in[i].x) + (c1 * in[i].y) + (c2 * in[i].z) + (c3 * in[i].w);
        p.store(out + i);
    }
}

// Returns singular value decomposition of the 2x2 matrix [m00 m01] as {min, max}
//                                                        [m10 m11]
std::pair<float, float> compute_svd(float m00, float m01, float m10, float m11) {
    // no-persp, these are the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    // and equivalent to SkMatrix::getMinmaxScales().
    float s1 = m00*m00 + m01*m01 + m10*m10 + m11*m11;

    float e = m00*m00 + m01*m01 - m10*m10 - m11*m11;
    float f = m00*m10 + m01*m11;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    // s2 >= 0, so (s1 - s2) <= (s1 + s2) so this always returns {min, max}.
    return {SkScalarSqrt(0.5f * (s1 - s2)),
            SkScalarSqrt(0.5f * (s1 + s2))};
}

std::pair<float, float> sort_scale(float sx, float sy) {
    float min = std::abs(sx);
    float max = std::abs(sy);
    if (min > max) {
        return {max, min};
    } else {
        return {min, max};
    }
}

} // anonymous namespace

Transform::Transform(const SkM44& m) : fM(m) {
    static constexpr SkV4 kNoPerspective = {0.f, 0.f, 0.f, 1.f};
    static constexpr SkV4 kNoZ           = {0.f, 0.f, 1.f, 0.f};
    if (m.row(3) != kNoPerspective) {
        // Perspective matrices will have per-location scale factors calculated, so cached scale
        // factors will not be used.
        if (m.invert(&fInvM)) {
            fType = Type::kPerspective;
        } else {
            fType = Type::kInvalid;
        }
        return;
    } else if (m.col(2) != kNoZ || m.row(2) != kNoZ) {
        // Orthographic matrices are lumped into the kAffine type although we use SkM44::invert()
        // instead of taking short cuts.
        if (m.invert(&fInvM)) {
            fType = Type::kAffine;
            // These scale factors are valid for the case where Z=0, which is the case for all
            // local geometry that's drawn.
            std::tie(fMinScaleFactor, fMaxScaleFactor) = compute_svd(m.rc(0,0), m.rc(0,1),
                                                                     m.rc(1,0), m.rc(1,1));
        } else {
            fType = Type::kInvalid;
        }
        return;
    }

    //                                              [sx kx 0 tx]
    // At this point, we know that m is of the form [ky sy 0 ty]
    //                                              [0  0  1 0 ]
    //                                              [0  0  0 1 ]
    // Other than kIdentity, none of the types depend on (tx, ty). The remaining types are
    // identified by considering the upper 2x2 (tx and ty are still used to compute the inverse).
    const float sx = m.rc(0, 0);
    const float sy = m.rc(1, 1);
    const float kx = m.rc(0, 1);
    const float ky = m.rc(1, 0);
    const float tx = m.rc(0, 3);
    const float ty = m.rc(1, 3);
    if (kx == 0.f && ky == 0.f) {
        // 2x2 is a diagonal matrix
        if (sx == 0.f || sy == 0.f) {
            // Not invertible
            fType = Type::kInvalid;
        } else if (sx == 1.f && sy == 1.f && tx == 0.f && ty == 0.f) {
            fType = Type::kIdentity;
            fInvM.setIdentity();
        } else {
            const float ix = 1.f / sx;
            const float iy = 1.f / sy;
            fType = sx > 0.f && sy > 0.f ? Type::kSimpleRectStaysRect
                                         : Type::kRectStaysRect;
            fInvM = SkM44(ix, 0.f, 0.f, -ix*tx,
                          0.f, iy, 0.f, -iy*ty,
                          0.f, 0.f, 1.f, 0.f,
                          0.f, 0.f, 0.f, 1.f);
            std::tie(fMinScaleFactor, fMaxScaleFactor) = sort_scale(sx, sy);
        }
    } else if (sx == 0.f && sy == 0.f) {
        // 2x2 is an anti-diagonal matrix and represents a 90 or 270 degree rotation plus optional
        // scale and translate.
        if (kx == 0.f || ky == 0.f) {
            // Not invertible
            fType = Type::kInvalid;
        } else {
            const float ix = 1.f / kx;
            const float iy = 1.f / ky;
            fType = Type::kRectStaysRect;
            fInvM = SkM44(0.f, iy, 0.f, -iy*ty,
                          ix, 0.f, 0.f, -ix*tx,
                          0.f, 0.f, 1.f, 0.f,
                          0.f, 0.f, 0.f, 1.f);
            std::tie(fMinScaleFactor, fMaxScaleFactor) = sort_scale(kx, ky);
        }
    } else {
        // Invert just the upper 2x2 and derive inverse translation from that
        float upper[4] = {sx, ky, kx, sy}; // col-major
        float invUpper[4];
        if (SkInvert2x2Matrix(upper, invUpper) == 0.f) {
            // 2x2 was not invertible, so the original matrix won't be invertible either
            fType = Type::kInvalid;
        } else {
            fType = Type::kAffine;
            fInvM = SkM44(invUpper[0], invUpper[2], 0.f, -invUpper[0]*tx - invUpper[2]*ty,
                          invUpper[1], invUpper[3], 0.f, -invUpper[1]*tx - invUpper[3]*ty,
                          0.f, 0.f, 1.f, 0.f,
                          0.f, 0.f, 0.f, 1.f);
            std::tie(fMinScaleFactor, fMaxScaleFactor) = compute_svd(sx, kx, ky, sy);
        }
    }
}

std::pair<float, float> Transform::scaleFactors(const SkV2& p) const {
    SkASSERT(this->valid());
    if (fType < Type::kPerspective) {
        return {fMinScaleFactor, fMaxScaleFactor};
    }

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
    // df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2 = (m00 - m30*f)/w
    // df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2 = (m01 - m31*f)/w
    // dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2 = (m10 - m30*g)/w
    // dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2 = (m11 - m31*g)/w
    //
    // Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
    //                    [dg/du dg/dv]
    // for M evaluated at  (u,v)
    SkV4 devP = fM.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = fM.rc(0,0);
    const float dxdv = fM.rc(0,1);
    const float dydu = fM.rc(1,0);
    const float dydv = fM.rc(1,1);
    const float dwdu = fM.rc(3,0);
    const float dwdv = fM.rc(3,1);

    float invW2 = sk_ieee_float_divide(1.f, (devP.w * devP.w));
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, these are the singular values of [m00,m01][m10,m11], which was already calculated
    // in get_matrix_info.
    return compute_svd(dfdu, dfdv, dgdu, dgdv);
}

float Transform::localAARadius(const Rect& bounds) const {
    SkASSERT(this->valid());

    float min;
    if (fType < Type::kPerspective) {
        // The scale factor is constant
        min = fMinScaleFactor;
    } else {
        // Calculate the minimum scale factor over the 4 corners of the bounding box
        float tl = std::get<0>(this->scaleFactors(SkV2{bounds.left(), bounds.top()}));
        float tr = std::get<0>(this->scaleFactors(SkV2{bounds.right(), bounds.top()}));
        float br = std::get<0>(this->scaleFactors(SkV2{bounds.right(), bounds.bot()}));
        float bl = std::get<0>(this->scaleFactors(SkV2{bounds.left(), bounds.bot()}));
        min = std::min(std::min(tl, tr), std::min(br, bl));
    }

    // Moving 1 from 'p' before transforming will move at least 'min' and at most 'max' from
    // the transformed point. Thus moving between [1/max, 1/min] pre-transformation means post
    // transformation moves between [1,max/min] so using 1/min as the local AA radius ensures that
    // the post-transformed point is at least 1px away from the original.
    float aaRadius = sk_ieee_float_divide(1.f, min);
    if (SkIsFinite(aaRadius)) {
        return aaRadius;
    } else {
        return SK_FloatInfinity;
    }
}

Rect Transform::mapRect(const Rect& rect) const {
    SkASSERT(this->valid());
    return map_rect(fM, rect);
}
Rect Transform::inverseMapRect(const Rect& rect) const {
    SkASSERT(this->valid());
    return map_rect(fInvM, rect);
}

void Transform::mapPoints(const Rect& localRect, SkV4 deviceOut[4]) const {
    SkASSERT(this->valid());
    SkV2 localCorners[4] = {{localRect.left(),  localRect.top()},
                            {localRect.right(), localRect.top()},
                            {localRect.right(), localRect.bot()},
                            {localRect.left(),  localRect.bot()}};
    this->mapPoints(localCorners, deviceOut, 4);
}

void Transform::mapPoints(const SkV2* localIn, SkV4* deviceOut, int count) const {
    SkASSERT(this->valid());
    // TODO: These maybe should go into SkM44, since bulk point mapping seems generally useful
    auto c0 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 0);
    auto c1 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 4);
    // skip c2 since localIn's z is assumed to be 0
    auto c3 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 12);

    for (int i = 0; i < count; ++i) {
        auto p = c0 * localIn[i].x + c1 * localIn[i].y /* + c2*0.f */ + c3 /* *1.f */;
        p.store(deviceOut + i);
    }
}

void Transform::mapPoints(const SkV4* localIn, SkV4* deviceOut, int count) const {
    SkASSERT(this->valid());
    return map_points(fM, localIn, deviceOut, count);
}

void Transform::inverseMapPoints(const SkV4* deviceIn, SkV4* localOut, int count) const {
    SkASSERT(this->valid());
    return map_points(fInvM, deviceIn, localOut, count);
}

} // namespace skgpu::graphite
