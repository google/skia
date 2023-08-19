/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/geom/Shape.h"

DEF_GRAPHITE_TEST(ShapeTest, reporter, CtsEnforcement::kNextRelease) {
    // TODO: Michael takes this over
    skgpu::graphite::Shape s;
    REPORTER_ASSERT(reporter, s.type() == skgpu::graphite::Shape::Type::kEmpty);
}
