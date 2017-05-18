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

#if GR_TEST_UTILS

class GrDrawOp;
class GrLegacyMeshDrawOp;
class GrPaint;
class GrRenderTargetContext;
struct GrUserStencilSettings;
class SkRandom;

/**  This function draws a randomly configured GrDrawOp for testing purposes. */
void GrDrawRandomOp(SkRandom*, GrRenderTargetContext*, GrPaint&&);

/** GrDrawOp subclasses should define test factory functions using this macro. */
#define GR_DRAW_OP_TEST_DEFINE(Op)                                                              \
    std::unique_ptr<GrDrawOp> Op##__Test(GrPaint&& paint, SkRandom* random, GrContext* context, \
                                         GrFSAAType fsaaType)

/** Variations for GrLegacyMeshDrawOps. To be deleted. */
#define GR_LEGACY_MESH_DRAW_OP_TEST_DEFINE(Op) \
    std::unique_ptr<GrLegacyMeshDrawOp> Op##__Test(SkRandom* random, GrContext* context)
#define GR_LEGACY_MESH_DRAW_OP_TEST_FRIEND(Op) \
    friend std::unique_ptr<GrLegacyMeshDrawOp> Op##__Test(SkRandom* random, GrContext* context);

/** Helper for op test factories to pick a random stencil state. */
const GrUserStencilSettings* GrGetRandomStencil(SkRandom* random, GrContext*);

#endif
#endif
