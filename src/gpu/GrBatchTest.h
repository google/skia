/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchTest_DEFINED
#define GrBatchTest_DEFINED

#include "GrTestUtils.h"

#ifdef GR_TEST_UTILS

class GrDrawOp;
class GrContext;
class SkRandom;

/*
 * This file defines some macros for testing batches, and also declares functions / objects which
 * are generally useful for GrOp testing
 */

// Batches should define test functions using DRAW_BATCH_TEST_DEFINE.  The other macros defined
// below are used exclusively by the test harness.
typedef GrDrawOp* (*BatchTestFunc)(SkRandom* random, GrContext* context);
#define DRAW_BATCH_TEST_DEFINE(Batch) \
    GrDrawOp* Batch##__Test(SkRandom* random, GrContext* context)
#define DRAW_BATCH_TEST_EXTERN(Batch) \
    extern GrDrawOp* Batch##__Test(SkRandom*, GrContext* context);
#define DRAW_BATCH_TEST_ENTRY(Batch) \
    Batch##__Test
#define DRAW_BATCH_TEST_FRIEND(Batch) \
    friend GrDrawOp* Batch##__Test(SkRandom* random, GrContext* context);

GrDrawOp* GrRandomDrawBatch(SkRandom*, GrContext*);

#endif
#endif
