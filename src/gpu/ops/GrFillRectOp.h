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

/**
 * A set of factory functions for drawing filled rectangles either coverage-antialiased, or
 * non-antialiased. The non-antialiased ops can be used with MSAA. As with other GrDrawOp factories,
 * the GrPaint is only consumed by these methods if a valid op is returned. If null is returned then
 * the paint is unmodified and may still be used.
 */
namespace GrFillRectOp {

// General purpose factory functions that handle per-edge anti-aliasing
std::unique_ptr<GrDrawOp> MakePerEdge(GrContext* context,
                                      GrPaint&& paint,
                                      GrAAType aaType,
                                      GrQuadAAFlags edgeAA,
                                      const SkMatrix& viewMatrix,
                                      const SkRect& rect,
                                      const GrUserStencilSettings* stencil = nullptr);

std::unique_ptr<GrDrawOp> MakePerEdgeWithLocalMatrix(GrContext* context,
                                                     GrPaint&& paint,
                                                     GrAAType aaType,
                                                     GrQuadAAFlags edgeAA,
                                                     const SkMatrix& viewMatrix,
                                                     const SkMatrix& localMatrix,
                                                     const SkRect& rect,
                                                     const GrUserStencilSettings* stl = nullptr);

std::unique_ptr<GrDrawOp> MakePerEdgeWithLocalRect(GrContext* context,
                                                   GrPaint&& paint,
                                                   GrAAType aaType,
                                                   GrQuadAAFlags edgeAA,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& rect,
                                                   const SkRect& localRect,
                                                   const GrUserStencilSettings* stencil = nullptr);

// Bulk API for drawing quads with a single op
// TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
std::unique_ptr<GrDrawOp> MakeSet(GrContext* context,
                                  GrPaint&& paint,
                                  GrAAType aaType,
                                  const SkMatrix& viewMatrix,
                                  const GrRenderTargetContext::QuadSetEntry quads[],
                                  int quadCount,
                                  const GrUserStencilSettings* stencil = nullptr);

// Specializations where all edges are treated the same. If the aa type is coverage, then the
// edges will be anti-aliased, otherwise per-edge AA will be disabled.
std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const GrUserStencilSettings* stencil = nullptr);

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect,
                                              const GrUserStencilSettings* stencil = nullptr);

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect,
                                            const GrUserStencilSettings* stencil = nullptr);

} // namespace GrFillRectOp

#endif // GrFillRectOp_DEFINED
