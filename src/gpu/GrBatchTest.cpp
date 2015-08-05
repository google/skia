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

BATCH_TEST_EXTERN(AAConvexPathBatch);
BATCH_TEST_EXTERN(AADistanceFieldPathBatch);
BATCH_TEST_EXTERN(AAFillRectBatch);
BATCH_TEST_EXTERN(AAHairlineBatch);
BATCH_TEST_EXTERN(AAStrokeRectBatch);
BATCH_TEST_EXTERN(DashBatch);
BATCH_TEST_EXTERN(DefaultPathBatch);
BATCH_TEST_EXTERN(CircleBatch);
BATCH_TEST_EXTERN(DIEllipseBatch);
BATCH_TEST_EXTERN(EllipseBatch);
BATCH_TEST_EXTERN(GrStrokeRectBatch);
BATCH_TEST_EXTERN(RRectBatch);
BATCH_TEST_EXTERN(TesselatingPathBatch);
BATCH_TEST_EXTERN(TextBlobBatch);
BATCH_TEST_EXTERN(VerticesBatch);

static BatchTestFunc gTestBatches[] = {
    BATCH_TEST_ENTRY(AAConvexPathBatch),
    BATCH_TEST_ENTRY(AADistanceFieldPathBatch),
    BATCH_TEST_ENTRY(AAFillRectBatch),
    BATCH_TEST_ENTRY(AAHairlineBatch),
    BATCH_TEST_ENTRY(AAStrokeRectBatch),
    BATCH_TEST_ENTRY(DashBatch),
    BATCH_TEST_ENTRY(DefaultPathBatch),
    BATCH_TEST_ENTRY(CircleBatch),
    BATCH_TEST_ENTRY(DIEllipseBatch),
    BATCH_TEST_ENTRY(EllipseBatch),
    BATCH_TEST_ENTRY(GrStrokeRectBatch),
    BATCH_TEST_ENTRY(RRectBatch),
    BATCH_TEST_ENTRY(TesselatingPathBatch),
    BATCH_TEST_ENTRY(TextBlobBatch),
    BATCH_TEST_ENTRY(VerticesBatch)
};

GrBatch* GrRandomBatch(SkRandom* random, GrContext* context) {
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gTestBatches)));
    BatchTestFunc func = gTestBatches[index];
    return (*func)(random, context);
}
#endif
