/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathInnerTriangulateOp_DEFINED
#define PathInnerTriangulateOp_DEFINED

#include "include/core/SkTypes.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/geometry/GrInnerFanTriangulator.h"
#include "src/gpu/ganesh/geometry/GrTriangulator.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/tessellate/GrTessellationShader.h"

#include <utility>

class GrAppliedClip;
class GrDstProxyView;
class GrOpFlushState;
class GrPipeline;
class GrProgramInfo;
class GrRecordingContext;
class GrSurfaceProxyView;
enum class GrXferBarrierFlags;
struct GrUserStencilSettings;
struct SkRect;

namespace skgpu::ganesh {

class PathCurveTessellator;
enum class FillPathFlags;

// This op is a 3-pass twist on the standard Redbook "stencil then cover" algorithm:
//
// 1) Tessellate the path's outer curves into the stencil buffer.
// 2) Triangulate the path's inner fan and fill it with a stencil test against the curves.
// 3) Draw convex hulls around each curve that fill in remaining samples.
//
// In practice, a path's inner fan takes up a large majority of its pixels. So from a GPU load
// perspective, this op is effectively as fast as a single-pass algorithm.
class PathInnerTriangulateOp final : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    PathInnerTriangulateOp(const SkMatrix& viewMatrix,
                           const SkPath& path,
                           GrPaint&& paint,
                           GrAAType aaType,
                           FillPathFlags pathFlags,
                           const SkRect& drawBounds)
            : GrDrawOp(ClassID())
            , fPathFlags(pathFlags)
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkASSERT(!fPath.isInverseFillType());
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "PathInnerTriangulateOp"; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

    // These calls set up the stencil & fill programs we will use prior to preparing and executing.
    void pushFanStencilProgram(const GrTessellationShader::ProgramArgs&,
                               const GrPipeline* pipelineForStencils, const GrUserStencilSettings*);
    void pushFanFillProgram(const GrTessellationShader::ProgramArgs&, const GrUserStencilSettings*);
    void prePreparePrograms(const GrTessellationShader::ProgramArgs&, GrAppliedClip&&);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const FillPathFlags fPathFlags;
    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    // Triangulates the inner fan.
    GrInnerFanTriangulator* fFanTriangulator = nullptr;
    GrTriangulator::Poly* fFanPolys = nullptr;
    GrInnerFanTriangulator::BreadcrumbTriangleList fFanBreadcrumbs;

    // This pipeline is shared by all programs that do filling.
    const GrPipeline* fPipelineForFills = nullptr;

    // Tessellates the outer curves.
    PathCurveTessellator* fTessellator = nullptr;

    // Pass 1: Tessellate the outer curves into the stencil buffer.
    const GrProgramInfo* fStencilCurvesProgram = nullptr;

    // Pass 2: Fill the path's inner fan with a stencil test against the curves. (In extenuating
    // circumstances this might require two separate draws.)
    skia_private::STArray<2, const GrProgramInfo*> fFanPrograms;

    // Pass 3: Draw convex hulls around each curve.
    const GrProgramInfo* fCoverHullsProgram = nullptr;

    // This buffer gets created by fFanTriangulator during onPrepare.
    sk_sp<const GrBuffer> fFanBuffer;
    int fBaseFanVertex = 0;
    int fFanVertexCount = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fHullVertexBufferIfNoIDSupport;

    friend class GrOp;  // For ctor.
};

}  // namespace skgpu::ganesh

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // PathInnerTriangulateOp_DEFINED
