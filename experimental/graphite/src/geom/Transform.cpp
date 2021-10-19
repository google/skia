/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "experimental/graphite/src/geom/Rect.h"
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

} // namespace skgpu
