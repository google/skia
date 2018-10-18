/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypesPriv.h"

class GrPaint;
class SkMatrix;
struct SkRect;

namespace GrRectOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               GrQuadAAFlags edgeAA,
                               GrUserStencilSettings* stencilSettings,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect);

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              GrQuadAAFlags edgeAA,
                                              GrUserStencilSettings* stencilSettings,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect);

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            GrQuadAAFlags edgeAA,
                                            GrUserStencilSettings* stencilSettings,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect);
}
