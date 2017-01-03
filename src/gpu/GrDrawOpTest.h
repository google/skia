/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpTest_DEFINED
#define GrDrawOpTest_DEFINED

#include "GrTestUtils.h"
#include "SkRefCnt.h"

#ifdef GR_TEST_UTILS

class GrDrawOp;
class GrContext;
class SkRandom;

/**  This function returns a randomly configured GrDrawOp for testing purposes. */
std::unique_ptr<GrDrawOp> GrRandomDrawOp(SkRandom*, GrContext*);

/** GrDrawOp subclasses should define test factory functions using this macro. */
#define DRAW_OP_TEST_DEFINE(Op) \
    std::unique_ptr<GrDrawOp> Op##__Test(SkRandom* random, GrContext* context)

/** This macro may be used if the test factory function must be made a friend of a class. */
#define DRAW_OP_TEST_FRIEND(Op) \
    friend std::unique_ptr<GrDrawOp> Op##__Test(SkRandom* random, GrContext* context);

#endif
#endif
