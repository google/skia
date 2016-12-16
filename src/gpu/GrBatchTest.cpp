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

DRAW_BATCH_TEST_EXTERN(AAConvexPathOp);
DRAW_BATCH_TEST_EXTERN(AADistanceFieldPathOp);
DRAW_BATCH_TEST_EXTERN(AAFillRectOp);
DRAW_BATCH_TEST_EXTERN(AAFillRectOpLocalMatrix);
DRAW_BATCH_TEST_EXTERN(AAFlatteningConvexPathOp)
DRAW_BATCH_TEST_EXTERN(AAHairlineOp);
DRAW_BATCH_TEST_EXTERN(AAStrokeRectOp);
DRAW_BATCH_TEST_EXTERN(AnalyticRectOp);
DRAW_BATCH_TEST_EXTERN(DashOp);
DRAW_BATCH_TEST_EXTERN(DefaultPathOp);
DRAW_BATCH_TEST_EXTERN(CircleOp);
DRAW_BATCH_TEST_EXTERN(DIEllipseOp);
DRAW_BATCH_TEST_EXTERN(EllipseOp);
DRAW_BATCH_TEST_EXTERN(GrDrawAtlasOp);
DRAW_BATCH_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_BATCH_TEST_EXTERN(PLSPathOp);
DRAW_BATCH_TEST_EXTERN(RRectOp);
DRAW_BATCH_TEST_EXTERN(TesselatingPathOp);
DRAW_BATCH_TEST_EXTERN(TextBlobBatch);
DRAW_BATCH_TEST_EXTERN(VerticesOp);

static BatchTestFunc gTestBatches[] = {
    DRAW_BATCH_TEST_ENTRY(AAConvexPathOp),
    DRAW_BATCH_TEST_ENTRY(AADistanceFieldPathOp),
    DRAW_BATCH_TEST_ENTRY(AAFillRectOp),
    DRAW_BATCH_TEST_ENTRY(AAFillRectOpLocalMatrix),
    DRAW_BATCH_TEST_ENTRY(AAFlatteningConvexPathOp),
    DRAW_BATCH_TEST_ENTRY(AAHairlineOp),
    DRAW_BATCH_TEST_ENTRY(AAStrokeRectOp),
    DRAW_BATCH_TEST_ENTRY(AnalyticRectOp),
    DRAW_BATCH_TEST_ENTRY(DashOp),
    DRAW_BATCH_TEST_ENTRY(DefaultPathOp),
    DRAW_BATCH_TEST_ENTRY(CircleOp),
    DRAW_BATCH_TEST_ENTRY(DIEllipseOp),
    DRAW_BATCH_TEST_ENTRY(EllipseOp),
    DRAW_BATCH_TEST_ENTRY(GrDrawAtlasOp),
    DRAW_BATCH_TEST_ENTRY(NonAAStrokeRectOp),
    // This currently hits an assert when the GrDisableColorXPFactory is randomly selected.
    // DRAW_BATCH_TEST_ENTRY(PLSPathOp),
    DRAW_BATCH_TEST_ENTRY(RRectOp),
    DRAW_BATCH_TEST_ENTRY(TesselatingPathOp),
    DRAW_BATCH_TEST_ENTRY(TextBlobBatch),
    DRAW_BATCH_TEST_ENTRY(VerticesOp)
};

GrDrawOp* GrRandomDrawBatch(SkRandom* random, GrContext* context) {
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gTestBatches)));
    BatchTestFunc func = gTestBatches[index];
    return (*func)(random, context);
}
#endif
