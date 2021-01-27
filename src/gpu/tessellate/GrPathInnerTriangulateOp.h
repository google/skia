/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathInnerTriangulateOp_DEFINED
#define GrPathInnerTriangulateOp_DEFINED

#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/GrPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class GrPathTessellator;

// This op is a 3-pass twist on the standard Redbook "stencil then fill" algorithm:
//
// 1) Tessellate the path's outer curves into the stencil buffer.
// 2) Triangulate the path's inner fan and fill it with a stencil test against the curves.
// 3) Draw convex hulls around each curve that fill in remaining samples.
//
// In practice, a path's inner fan takes up a large majority of its pixels. So from a GPU load
// perspective, this op is effectively as fast as a single-pass algorithm.
class GrPathInnerTriangulateOp : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    GrPathInnerTriangulateOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
                       GrAAType aaType, GrTessellationPathRenderer::OpFlags opFlags)
            : GrDrawOp(ClassID())
            , fOpFlags(opFlags)
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkRect devBounds;
        fViewMatrix.mapRect(&devBounds, path.getBounds());
        this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
    }

    const char* name() const override { return "GrPathInnerTriangulateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;

    // These calls set up the stencil & fill programs we will use prior to preparing and executing.
    void pushFanStencilProgram(const GrPathShader::ProgramArgs&,
                               const GrPipeline* pipelineForStencils, const GrUserStencilSettings*);
    void pushFanFillProgram(const GrPathShader::ProgramArgs&, const GrUserStencilSettings*);
    void prePreparePrograms(const GrPathShader::ProgramArgs&, GrAppliedClip&&);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrTessellationPathRenderer::OpFlags fOpFlags;
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
    GrPathTessellator* fTessellator = nullptr;

    // Pass 1: Tessellate the outer curves into the stencil buffer.
    const GrProgramInfo* fStencilCurvesProgram = nullptr;

    // Pass 2: Fill the path's inner fan with a stencil test against the curves. (In extenuating
    // circumstances this might require two separate draws.)
    SkSTArray<2, const GrProgramInfo*> fFanPrograms;

    // Pass 3: Draw convex hulls around each curve.
    const GrProgramInfo* fFillHullsProgram = nullptr;

    // This buffer gets created by fFanTriangulator during onPrepare.
    sk_sp<const GrBuffer> fFanBuffer;
    int fBaseFanVertex = 0;
    int fFanVertexCount = 0;

    friend class GrOp;  // For ctor.
};

#endif
