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

class GrBatch;
class GrContext;
class SkRandom;

/*
 * This file defines some macros for testing batches, and also declares functions / objects which
 * are generally useful for GrBatch testing
 */

// Batches should define test functions using BATCH_TEST_DEFINE.  The other macros defined below
// are used exclusively by the test harness.
typedef GrBatch* (*BatchTestFunc)(SkRandom* random, GrContext* context);
#define BATCH_TEST_DEFINE(Batch) \
    GrBatch* Batch##__Test(SkRandom* random, GrContext* context)
#define BATCH_TEST_EXTERN(Batch) \
    extern GrBatch* Batch##__Test(SkRandom*, GrContext* context);
#define BATCH_TEST_ENTRY(Batch) \
    Batch##__Test
#define BATCH_TEST_FRIEND(Batch) \
    friend GrBatch* Batch##__Test(SkRandom* random, GrContext* context);

GrBatch* GrRandomBatch(SkRandom*, GrContext*);

#endif
#endif
