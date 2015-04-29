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

BATCH_TEST_EXTERN(AAFillRectBatch);
BATCH_TEST_EXTERN(AAStrokeRectBatch);

static BatchTestFunc gTestBatches[] = {
    BATCH_TEST_ENTRY(AAFillRectBatch),
    BATCH_TEST_ENTRY(AAStrokeRectBatch),
};

GrBatch* GrRandomBatch(SkRandom* random, GrContext* context) {
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(SK_ARRAY_COUNT(gTestBatches)));
    BatchTestFunc func = gTestBatches[index];
    return (*func)(random, context);
}
#endif
