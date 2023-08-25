/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillRectOp_DEFINED
#define FillRectOp_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"

struct DrawQuad;
class GrClip;
class GrDrawOp;
class GrPaint;
class GrQuad;
struct GrQuadSetEntry;
class GrRecordingContext;
struct GrUserStencilSettings;
class SkMatrix;
struct SkRect;

namespace skgpu::ganesh {

class SurfaceDrawContext;

/**
 * A set of factory functions for drawing filled rectangles either coverage-antialiased, or
 * non-antialiased. The non-antialiased ops can be used with MSAA. As with other GrDrawOp factories,
 * the GrPaint is only consumed by these methods if a valid op is returned. If null is returned then
 * the paint is unmodified and may still be used.
 */
class FillRectOp {
public:
    using InputFlags = GrSimpleMeshDrawOpHelper::InputFlags;

    static GrOp::Owner Make(GrRecordingContext*,
                            GrPaint&&,
                            GrAAType,
                            DrawQuad*,
                            const GrUserStencilSettings* = nullptr,
                            InputFlags = InputFlags::kNone);

    // Utility function to create a non-AA rect transformed by view. This is used commonly enough
    // in testing and GMs that manage ops without going through GrRTC that it's worth the
    // convenience.
    static GrOp::Owner MakeNonAARect(GrRecordingContext*,
                                     GrPaint&&,
                                     const SkMatrix& view,
                                     const SkRect&,
                                     const GrUserStencilSettings* = nullptr);

    // Bulk API for drawing quads with a single op
    // TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
    static void AddFillRectOps(SurfaceDrawContext*,
                               const GrClip*,
                               GrRecordingContext*,
                               GrPaint&&,
                               GrAAType,
                               const SkMatrix& viewMatrix,
                               const GrQuadSetEntry quads[],
                               int quadCount,
                               const GrUserStencilSettings* = nullptr);

#if defined(GR_TEST_UTILS)
    static uint32_t ClassID();
#endif

private:
    // Create a FillRectOp that uses as many quads as possible from 'quads' w/o exceeding
    // any index buffer size limits.
    static GrOp::Owner MakeOp(GrRecordingContext*,
                              GrPaint&&,
                              GrAAType,
                              const SkMatrix& viewMatrix,
                              const GrQuadSetEntry quads[],
                              int quadCount,
                              const GrUserStencilSettings*,
                              int* numConsumed);
};

}  // namespace skgpu::ganesh

#endif // FillRectOp_DEFINED
