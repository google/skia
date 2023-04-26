/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Transform_graphite.h"

#include "src/base/SkVx.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/graphite/geom/Rect.h"

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

Transform::Type get_matrix_info(const SkM44& m, SkM44* inverse, SkV2* scale) {
    // First compute the inverse.
    // TODO: Alternatively we could compute type first and have type-specific inverses, but it seems
    // useful to ensure any non-invalid matrix returns true from SkM44::invert.
    if (!m.invert(inverse)) {
        *scale = {1.f, 1.f};
        return Transform::Type::kInvalid;
    }

    static constexpr SkV4 kNoPerspective = {0.f, 0.f, 0.f, 1.f};
    static constexpr SkV4 kNoZ = {0.f, 0.f, 1.f, 0.f};
    if (m.row(3) != kNoPerspective || m.col(2) != kNoZ || m.row(2) != kNoZ) {
        // TODO: Use SkMatrixPriv::DifferentialAreaScale, but we need a representative point then.
        // Or something like lengths of upper 2x2 divided by w?
        *scale = {1.f, 1.f};
        return Transform::Type::kProjection;
    }

    //                                              [sx kx 0 tx]
    // At this point, we know that m is of the form [ky sy 0 ty]
    //                                              [0  0  1 0 ]
    //                                              [0  0  0 1 ]
    // Other than kIdentity, none of the types depend on (tx, ty). The remaining types are
    // identified by considering the upper 2x2.
    float sx = m.rc(0, 0);
    float sy = m.rc(1, 1);
    float kx = m.rc(0, 1);
    float ky = m.rc(1, 0);
    if (kx == 0.f && ky == 0.f) {
        // 2x2 is a diagonal matrix
        *scale = {std::abs(sx), std::abs(sy)};
        if (sx == 1.f && sy == 1.f && m.rc(0, 3) == 0.f && m.rc(1, 3) == 0.f) {
            return Transform::Type::kIdentity;
        } else if (sx > 0.f && sy > 0.f) {
            return Transform::Type::kSimpleRectStaysRect;
        } else {
            // We don't need to worry about sx or sy being 0 here because that would imply the
            // matrix wasn't invertible and that was already tested.
            SkASSERT(sx != 0.f && sy != 0.f);
            return Transform::Type::kRectStaysRect;
        }
    } else if (sx == 0.f && sy == 0.f) {
        // 2x2 is an anti-diagonal matrix and represents a 90 or 270 degree rotation plus optional
        // scale and translate. Similar to before, kx and ky can't be 0 or m wouldn't be invertible.
        SkASSERT(kx != 0.f && ky != 0.f);
        *scale = {std::abs(ky), std::abs(kx)};
        return Transform::Type::kRectStaysRect;
    } else {
        *scale = {SkV2{sx, ky}.length(), SkV2{kx, sy}.length()};
        return Transform::Type::kAffine;
    }
}

} // anonymous namespace

Transform::Transform(const SkM44& m) : fM(m) {
    fType = get_matrix_info(m, &fInvM, &fScale);
}

bool Transform::operator==(const Transform& t) const {
    // Checking fM should be sufficient as all other values are computed from it.
    SkASSERT(fM != t.fM || (fInvM == t.fInvM && fType == t.fType && fScale == t.fScale));
    return fM == t.fM;
}

Rect Transform::mapRect(const Rect& rect) const { return map_rect(fM, rect); }
Rect Transform::inverseMapRect(const Rect& rect) const { return map_rect(fInvM, rect); }

void Transform::mapPoints(const Rect& localRect, SkV4 deviceOut[4]) const {
    SkV2 localCorners[4] = {{localRect.left(),  localRect.top()},
                            {localRect.right(), localRect.top()},
                            {localRect.right(), localRect.bot()},
                            {localRect.left(),  localRect.bot()}};
    this->mapPoints(localCorners, deviceOut, 4);
}

void Transform::mapPoints(const SkV2* localIn, SkV4* deviceOut, int count) const {
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
    return map_points(fM, localIn, deviceOut, count);
}

void Transform::inverseMapPoints(const SkV4* deviceIn, SkV4* localOut, int count) const {
    return map_points(fInvM, deviceIn, localOut, count);
}

Transform Transform::preTranslate(float x, float y) const {
    Transform t = *this;
    t.fM.preTranslate(x, y);
    t.fInvM.postTranslate(-x, -y);

    // Under normal conditions, type and scale won't change, but if we've overflown the translation
    // components, mark the matrix as invalid.
    if (!t.fM.isFinite() || !t.fInvM.isFinite()) {
        t.fType = Type::kInvalid;
    }
    return t;
}

Transform Transform::postTranslate(float x, float y) const {
    Transform t = *this;
    t.fM.postTranslate(x, y);
    t.fInvM.preTranslate(-x, -y);
    if (!t.fM.isFinite() || !t.fInvM.isFinite()) {
        t.fType = Type::kInvalid;
    }
    return t;
}

Transform Transform::concat(const Transform& t) const {
    Transform c = {fM * t.fM, t.fInvM * fInvM, std::max(fType, t.fType), {fScale * t.fScale}};
    if (!c.fM.isFinite() || !c.fInvM.isFinite()) {
        c.fType = Type::kInvalid;
    }
    return c;
}

Transform Transform::concatInverse(const Transform& t) const {
    Transform c = {fM * t.fInvM, t.fM * fInvM, std::max(fType, t.fType), {fScale * (1.f/t.fScale)}};
    if (!c.fM.isFinite() || !c.fInvM.isFinite()) {
        c.fType = Type::kInvalid;
    }
    return c;
}

Transform Transform::concatInverse(const SkM44& t) const {
    // saves a multiply compared to inverting just t and then computing fM*t^-1 and t*fInvM, if we
    // instead start with (t*fInvM) and swap definition of computed fM and fInvM.
    Transform inverse{t * fInvM};
    return {inverse.fInvM, inverse.fM, inverse.fType, 1.f / inverse.fScale};
}

} // namespace skgpu::graphite
