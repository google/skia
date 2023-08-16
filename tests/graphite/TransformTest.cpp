/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/geom/Transform_graphite.h"

DEF_GRAPHITE_TEST(TransformTest, reporter, CtsEnforcement::kNextRelease) {
    // TODO: Michael takes this over
    skgpu::graphite::Transform t{SkM44()};
    REPORTER_ASSERT(reporter, t.type() == skgpu::graphite::Transform::Type::kIdentity);
}
