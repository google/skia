/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Transform_graphite.h"

namespace skgpu {

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

} // namespace skgpu
