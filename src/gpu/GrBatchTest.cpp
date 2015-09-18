/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTest.h"
#include "SkRandom.h"
#include "SkTypes.h"

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_EXTERN(AAConvexPathBatch);
DRAW_BATCH_TEST_EXTERN(AADistanceFieldPathBatch);
DRAW_BATCH_TEST_EXTERN(AAFillRectBatch);
DRAW_BATCH_TEST_EXTERN(AAFillRectBatchLocalMatrix);
DRAW_BATCH_TEST_EXTERN(AAHairlineBatch);
DRAW_BATCH_TEST_EXTERN(AAStrokeRectBatch);
DRAW_BATCH_TEST_EXTERN(DashBatch);
DRAW_BATCH_TEST_EXTERN(DefaultPathBatch);
DRAW_BATCH_TEST_EXTERN(CircleBatch);
DRAW_BATCH_TEST_EXTERN(DIEllipseBatch);
DRAW_BATCH_TEST_EXTERN(EllipseBatch);
DRAW_BATCH_TEST_EXTERN(GrDrawAtlasBatch);
DRAW_BATCH_TEST_EXTERN(NonAAStrokeRectBatch);
DRAW_BATCH_TEST_EXTERN(RRectBatch);
DRAW_BATCH_TEST_EXTERN(TesselatingPathBatch);
DRAW_BATCH_TEST_EXTERN(TextBlobBatch);
DRAW_BATCH_TEST_EXTERN(VerticesBatch);

static BatchTestFunc gTestBatches[] = {
    DRAW_BATCH_TEST_ENTRY(AAConvexPathBatch),
    DRAW_BATCH_TEST_ENTRY(AADistanceFieldPathBatch),
    DRAW_BATCH_TEST_ENTRY(AAFillRectBatch),
    DRAW_BATCH_TEST_ENTRY(AAFillRectBatchLocalMatrix),
    DRAW_BATCH_TEST_ENTRY(AAHairlineBatch),
    DRAW_BATCH_TEST_ENTRY(AAStrokeRectBatch),
    DRAW_BATCH_TEST_ENTRY(DashBatch),
    DRAW_BATCH_TEST_ENTRY(DefaultPathBatch),
    DRAW_BATCH_TEST_ENTRY(CircleBatch),
    DRAW_BATCH_TEST_ENTRY(DIEllipseBatch),
    DRAW_BATCH_TEST_ENTRY(EllipseBatch),
    DRAW_BATCH_TEST_ENTRY(GrDrawAtlasBatch),
    DRAW_BATCH_TEST_ENTRY(NonAAStrokeRectBatch),
    DRAW_BATCH_TEST_ENTRY(RRectBatch),
    DRAW_BATCH_TEST_ENTRY(TesselatingPathBatch),
    DRAW_BATCH_TEST_ENTRY(TextBlobBatch),
    DRAW_BATCH_TEST_ENTRY(VerticesBatch)
};

GrDrawBatch* GrRandomDrawBatch(SkRandom* random, GrContext* context) {
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gTestBatches)));
    BatchTestFunc func = gTestBatches[index];
    return (*func)(random, context);
}
#endif
