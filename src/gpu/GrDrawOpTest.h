/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpTest_DEFINED
#define GrDrawOpTest_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrTestUtils.h"

#if GR_TEST_UTILS

class GrContext_Base;
class GrDrawOp;
class GrPaint;
class GrRecordingContext;
namespace skgpu { namespace v1 { class SurfaceDrawContext; }}
struct GrUserStencilSettings;
class SkRandom;

/**  This function draws a randomly configured GrDrawOp for testing purposes. */
void GrDrawRandomOp(SkRandom*, skgpu::v1::SurfaceDrawContext*, GrPaint&&);

/** GrDrawOp subclasses should define test factory functions using this macro. */
#define GR_DRAW_OP_TEST_DEFINE(Op)                                                              \
    GrOp::Owner Op##__Test(GrPaint&& paint,                                                     \
                           SkRandom* random,                                                    \
                           GrRecordingContext* context,                                         \
                           skgpu::v1::SurfaceDrawContext* sdc,                                  \
                           int numSamples)
#define GR_DRAW_OP_TEST_FRIEND(Op)                                                              \
    friend GrOp::OpOwner Op##__Test(GrPaint&&,                                                  \
                                    SkRandom*,                                                  \
                                    GrRecordingContext*,                                        \
                                    skgpu::v1::SurfaceDrawContext*,                             \
                                    int)

/** Helper for op test factories to pick a random stencil state. */
const GrUserStencilSettings* GrGetRandomStencil(SkRandom* random, GrContext_Base*);

#endif
#endif
