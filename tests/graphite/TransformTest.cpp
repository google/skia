/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/src/geom/Transform_graphite.h"

DEF_GRAPHITE_TEST(TransformTest, reporter) {
    // TODO: Michael takes this over
    skgpu::Transform t{SkM44()};
    REPORTER_ASSERT(reporter, t.type() == skgpu::Transform::Type::kIdentity);
}
