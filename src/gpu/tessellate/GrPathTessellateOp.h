/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellateOp_DEFINED
#define GrPathTessellateOp_DEFINED

#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class GrAppliedHardClip;
class GrEagerVertexAllocator;
class GrStencilPathShader;
class GrResolveLevelCounter;

// Renders paths using a hybrid "Red Book" (stencil, then cover) method. Curves get linearized by
// either GPU tessellation shaders or indirect draws. This Op doesn't apply analytic AA, so it
// requires a render target that supports either MSAA or mixed samples if AA is desired.
class GrPathTessellateOp : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    GrPathTessellateOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
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

    const char* name() const override { return "GrPathTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override;
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      bool hasMixedSampledCoverage,
                                      GrClampType clampType) override {
        return fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
                hasMixedSampledCoverage, caps, clampType, &fColor);
    }

    FixedFunctionFlags fixedFunctionFlags() const override;

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;

    struct PrePrepareArgs {
        SkArenaAlloc* fArena;
        const GrSurfaceProxyView& fWriteView;
        const GrAppliedHardClip* fHardClip;
        GrAppliedClip* fClip;
        const GrXferProcessor::DstProxyView* fDstProxyView;
        GrXferBarrierFlags fXferBarrierFlags;
        GrLoadOp fColorLoadOp;
        const GrCaps* fCaps;
    };

    // Chooses the rendering method we will use and creates the corresponding stencil and fill
    // programs up front.
    void prePreparePrograms(const PrePrepareArgs&);

    // Produces a non-overlapping triangulation of the path's inner polygon(s). The inner polygons
    // connect the endpoints of each verb. (i.e., they are the path that would result from
    // collapsing all curves to single lines.) If this succeeds, then we will be able to fill the
    // triangles directly and bypass stencilling them.
    //
    // Returns false if the inner triangles do not form a simple polygon (e.g., self intersection,
    // double winding). Non-simple polygons would need to split edges in order to avoid overlap,
    // and this is not an option as it would introduce T-junctions with the outer cubics.
    bool prePrepareInnerPolygonTriangulation(const PrePrepareArgs&, bool* isLinear);

    void prePrepareStencilTrianglesProgram(const PrePrepareArgs&);
    template<typename ShaderType> void prePrepareStencilCubicsProgram(const PrePrepareArgs&);
    void prePreparePipelineForStencils(const PrePrepareArgs&);

    void prePrepareFillTrianglesProgram(const PrePrepareArgs&, bool isLinear);
    void prePrepareFillCubicHullsProgram(const PrePrepareArgs&);
    void prePrepareFillBoundingBoxProgram(const PrePrepareArgs&);
    void prePreparePipelineForFills(const PrePrepareArgs&);

    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
    void drawStencilPass(GrOpFlushState*);
    void drawCoverPass(GrOpFlushState*);

    const GrTessellationPathRenderer::OpFlags fOpFlags;
    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    GrInnerFanTriangulator* fInnerFanTriangulator = nullptr;
    GrTriangulator::Poly* fInnerFanPolys = nullptr;
    sk_sp<const GrBuffer> fTriangleBuffer;
    int fBaseTriangleVertex = 0;
    int fTriangleVertexCount = 0;

    // This pipeline is used by all programInfos in the stencil step.
    const GrPipeline* fPipelineForStencils = nullptr;

    // This pipeline is used by all programInfos in the fill step.
    const GrPipeline* fPipelineForFills = nullptr;

    // These switches specify how the above fTriangleBuffer should be drawn (if at all).
    //
    // If stencil and !fill:
    //
    //     We just stencil the triangles (with cubics) during the stencil step. The path gets filled
    //     later using a simple bounding box if needed.
    //
    // If stencil and fill:
    //
    //     We still stencil the triangles and cubics normally, but during the *fill* step we fill
    //     the triangles plus local convex hulls around each cubic instead of a bounding box.
    //
    // If !stencil and fill:
    //
    //     This means that fTriangleBuffer contains non-overlapping geometry that can be filled
    //     directly to the final render target. We only need to stencil *curves*, and during the
    //     fill step we draw the triangles directly with a stencil test that accounts for curves
    //     (see drawCoverPass()), and then finally fill the curves with local convex hulls.
    const GrProgramInfo* fStencilTrianglesProgram = nullptr;
    const GrProgramInfo* fFillTrianglesProgram = nullptr;

    const GrProgramInfo* fStencilCubicsProgram = nullptr;

    // This will draw either a bounding box, or if fFillTrianglesProgram exists, individual convex
    // hulls covering each cubic.
    const GrProgramInfo* fFillPathProgram = nullptr;

    GrPathTessellator* fTessellator = nullptr;

    friend class GrOp;  // For ctor.

public:
    // This serves as a base class for benchmarking individual methods on GrPathTessellateOp.
    class TestingOnly_Benchmark;
};

#endif
