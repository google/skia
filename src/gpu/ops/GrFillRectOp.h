/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRectOp_DEFINED
#define GrFillRectOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

class GrDrawOp;
class GrPaint;
class GrQuad;
class GrRecordingContext;
struct GrUserStencilSettings;
class SkMatrix;
struct SkRect;

/**
 * A set of factory functions for drawing filled rectangles either coverage-antialiased, or
 * non-antialiased. The non-antialiased ops can be used with MSAA. As with other GrDrawOp factories,
 * the GrPaint is only consumed by these methods if a valid op is returned. If null is returned then
 * the paint is unmodified and may still be used.
 */
class GrFillRectOp {
public:
    using InputFlags = GrSimpleMeshDrawOpHelper::InputFlags;

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            GrAAType aaType,
                            DrawQuad* quad,
                            const GrUserStencilSettings* stencil = nullptr,
                            InputFlags = InputFlags::kNone);

    // Utility function to create a non-AA rect transformed by view. This is used commonly enough
    // in testing and GMs that manage ops without going through GrRTC that it's worth the
    // convenience.
    static GrOp::Owner MakeNonAARect(GrRecordingContext* context,
                                     GrPaint&& paint,
                                     const SkMatrix& view,
                                     const SkRect& rect,
                                     const GrUserStencilSettings* stencil = nullptr);

    // Bulk API for drawing quads with a single op
    // TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
    static void AddFillRectOps(GrSurfaceDrawContext*,
                               const GrClip* clip,
                               GrRecordingContext*,
                               GrPaint&&,
                               GrAAType,
                               const SkMatrix& viewMatrix,
                               const GrSurfaceDrawContext::QuadSetEntry quads[],
                               int quadCount,
                               const GrUserStencilSettings* = nullptr);

#if GR_TEST_UTILS
    static uint32_t ClassID();
#endif

private:
    // Create a GrFillRectOp that uses as many quads as possible from 'quads' w/o exceeding
    // any index buffer size limits.
    static GrOp::Owner MakeOp(GrRecordingContext*,
                              GrPaint&&,
                              GrAAType,
                              const SkMatrix& viewMatrix,
                              const GrSurfaceDrawContext::QuadSetEntry quads[],
                              int quadCount,
                              const GrUserStencilSettings*,
                              int* numConsumed);
};

#endif // GrFillRectOp_DEFINED
