/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeRectBatch.h"
#include "GrBatchTest.h"
#include "SkRandom.h"

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(GrStrokeRectBatch) {
    GrStrokeRectBatch::Geometry geometry;
    geometry.fViewMatrix = GrTest::TestMatrix(random);
    geometry.fColor = GrRandomColor(random);
    geometry.fRect = GrTest::TestRect(random);
    geometry.fStrokeWidth = random->nextBool() ? 0.0f : 1.0f;

    return GrStrokeRectBatch::Create(geometry, random->nextBool());
}

#endif
