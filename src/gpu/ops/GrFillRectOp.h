/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRectOp_DEFINED
#define GrFillRectOp_DEFINED

#include "GrRenderTargetContext.h"
#include "GrTypesPriv.h"

class GrDrawOp;
class GrPaint;
struct GrUserStencilSettings;
class SkMatrix;
struct SkRect;

namespace GrFillRectOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               GrQuadAAFlags edgeAA,
                               const GrUserStencilSettings* stencilSettings,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect);

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              GrQuadAAFlags edgeAA,
                                              const GrUserStencilSettings* stencilSettings,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect);

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            GrQuadAAFlags edgeAA,
                                            const GrUserStencilSettings* stencilSettings,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect);

std::unique_ptr<GrDrawOp> MakeSet(GrContext* context,
                                  GrPaint&& paint,
                                  GrAAType aaType,
                                  const GrUserStencilSettings* stencilSettings,
                                  const SkMatrix& viewMatrix,
                                  const GrRenderTargetContext::QuadSetEntry quads[],
                                  int quadCount);
}

#endif // GrFillRectOp_DEFINED
