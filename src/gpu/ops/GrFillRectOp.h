/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRectOp_DEFINED
#define GrFillRectOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

struct DrawQuad;
class GrClip;
class GrDrawOp;
class GrPaint;
class GrQuad;
struct GrQuadSetEntry;
class GrRecordingContext;
namespace skgpu { namespace v1 { class SurfaceDrawContext; }}
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

    // TODO: remove this guard once GrFillRectOp is made V1-only
#if SK_GPU_V1
    // Bulk API for drawing quads with a single op
    // TODO(michaelludwig) - remove if the bulk API is not useful for SkiaRenderer
    static void AddFillRectOps(skgpu::v1::SurfaceDrawContext*,
                               const GrClip*,
                               GrRecordingContext*,
                               GrPaint&&,
                               GrAAType,
                               const SkMatrix& viewMatrix,
                               const GrQuadSetEntry quads[],
                               int quadCount,
                               const GrUserStencilSettings* = nullptr);
#endif

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
                              const GrQuadSetEntry quads[],
                              int quadCount,
                              const GrUserStencilSettings*,
                              int* numConsumed);
};

#endif // GrFillRectOp_DEFINED
