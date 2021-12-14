/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "experimental/graphite/src/geom/Rect.h"
#include "experimental/graphite/src/geom/VectorTypes.h"
#include "src/core/SkMatrixPriv.h"

namespace skgpu {

namespace {

Rect map_rect(const SkM44& m, const Rect& r) {
    // TODO: Can Rect's (l,t,-r,-b) structure be used to optimize mapRect?
    // TODO: Can take this opportunity to implement 100% accurate perspective plane clipping since
    //       it doesn't have to match raster/ganesh rendering behavior.
    return SkMatrixPriv::MapRect(m, r.asSkRect());
}

} // anonymous namespace

Transform::Transform(const SkM44& m)
        : fM(m) {
    if (fM.invert(&fInvM)) {
        // TODO: actually detect these
        fType = (fM == SkM44()) ? Type::kIdentity : Type::kPerspective;
        fScale = {1.f, 1.f};
    } else {
        fType = Type::kInvalid;
        fInvM = SkM44();
        fScale = {1.f, 1.f};
    }
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
    float4 c0 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 0);
    float4 c1 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 4);
    // skip c2 since localIn's z is assumed to be 0
    float4 c3 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 12);

    for (int i = 0; i < count; ++i) {
        float4 p = c0 * localIn[i].x + c1 * localIn[i].y /* + c2*0.f */ + c3 /* *1.f */;
        p.store(deviceOut + i);
    }
}

} // namespace skgpu
