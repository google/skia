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
DRAW_BATCH_TEST_EXTERN(AAFillRectOp);
DRAW_BATCH_TEST_EXTERN(AAFillRectOpLocalMatrix);
DRAW_BATCH_TEST_EXTERN(AAHairlineBatch);
DRAW_BATCH_TEST_EXTERN(AAStrokeRectOp);
DRAW_BATCH_TEST_EXTERN(AnalyticRectOp);
DRAW_BATCH_TEST_EXTERN(DashBatch);
DRAW_BATCH_TEST_EXTERN(DefaultPathBatch);
DRAW_BATCH_TEST_EXTERN(CircleOp);
DRAW_BATCH_TEST_EXTERN(DIEllipseOp);
DRAW_BATCH_TEST_EXTERN(EllipseOp);
DRAW_BATCH_TEST_EXTERN(GrDrawAtlasBatch);
DRAW_BATCH_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_BATCH_TEST_EXTERN(RRectOp);
DRAW_BATCH_TEST_EXTERN(TesselatingPathBatch);
DRAW_BATCH_TEST_EXTERN(TextBlobBatch);
DRAW_BATCH_TEST_EXTERN(VerticesBatch);

static BatchTestFunc gTestBatches[] = {
    DRAW_BATCH_TEST_ENTRY(AAConvexPathBatch),
    DRAW_BATCH_TEST_ENTRY(AADistanceFieldPathBatch),
    DRAW_BATCH_TEST_ENTRY(AAFillRectOp),
    DRAW_BATCH_TEST_ENTRY(AAFillRectOpLocalMatrix),
    DRAW_BATCH_TEST_ENTRY(AAHairlineBatch),
    DRAW_BATCH_TEST_ENTRY(AAStrokeRectOp),
    DRAW_BATCH_TEST_ENTRY(AnalyticRectOp),
    DRAW_BATCH_TEST_ENTRY(DashBatch),
    DRAW_BATCH_TEST_ENTRY(DefaultPathBatch),
    DRAW_BATCH_TEST_ENTRY(CircleOp),
    DRAW_BATCH_TEST_ENTRY(DIEllipseOp),
    DRAW_BATCH_TEST_ENTRY(EllipseOp),
    DRAW_BATCH_TEST_ENTRY(GrDrawAtlasBatch),
    DRAW_BATCH_TEST_ENTRY(NonAAStrokeRectOp),
    DRAW_BATCH_TEST_ENTRY(RRectOp),
    DRAW_BATCH_TEST_ENTRY(TesselatingPathBatch),
    DRAW_BATCH_TEST_ENTRY(TextBlobBatch),
    DRAW_BATCH_TEST_ENTRY(VerticesBatch)
};

GrDrawOp* GrRandomDrawBatch(SkRandom* random, GrContext* context) {
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gTestBatches)));
    BatchTestFunc func = gTestBatches[index];
    return (*func)(random, context);
}
#endif
