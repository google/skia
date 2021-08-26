/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathStencilCoverOp_DEFINED
#define PathStencilCoverOp_DEFINED

#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/GrPathTessellator.h"
#include "src/gpu/tessellate/GrTessTypes.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

namespace skgpu::v1 {

// Draws paths using a standard Redbook "stencil then cover" method. Curves get linearized by either
// GPU tessellation shaders or indirect draws. This Op doesn't apply analytic AA, so it requires
// MSAA if AA is desired.
class PathStencilCoverOp final : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    using PathDrawList = GrPathTessellator::PathDrawList;

    // If the path is inverse filled, drawBounds must be the entire backing store dimensions of the
    // render target.
    PathStencilCoverOp(SkArenaAlloc* arena,
                       const SkMatrix& viewMatrix,
                       const SkPath& path,
                       GrPaint&& paint,
                       GrAAType aaType,
                       GrTessellationPathFlags pathFlags,
                       const SkRect& drawBounds)
            : GrDrawOp(ClassID())
            , fPathDrawList(arena->make<PathDrawList>(viewMatrix, path))
            , fTotalCombinedPathVerbCnt(path.countVerbs())
            , fPathCount(1)
            , fPathFlags(pathFlags)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
        SkDEBUGCODE(fOriginalDrawBounds = drawBounds;)
    }

    // Constructs a PathStencilCoverOp from an existing draw list.
    // FIXME: The only user of this method is the atlas. We should move the GrProgramInfos into
    // GrPathTessellator so the atlas can use that directly instead of going through this class.
    PathStencilCoverOp(const PathDrawList* pathDrawList,
                       int totalCombinedVerbCnt,
                       int pathCount,
                       GrPaint&& paint,
                       GrAAType aaType,
                       GrTessellationPathFlags pathFlags,
                       const SkRect& drawBounds)
            : GrDrawOp(ClassID())
            , fPathDrawList(pathDrawList)
            , fTotalCombinedPathVerbCnt(totalCombinedVerbCnt)
            , fPathCount(pathCount)
            , fPathFlags(pathFlags)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
        SkDEBUGCODE(fOriginalDrawBounds = drawBounds;)
    }

    const char* name() const override { return "PathStencilCoverOp"; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

    // All paths in fPathDrawList are required to have the same fill type.
    SkPathFillType pathFillType() const {
        return fPathDrawList->fPath.getFillType();
    }

    // Chooses the rendering method we will use and creates the corresponding tessellator and
    // stencil/cover programs.
    void prePreparePrograms(const GrTessellationShader::ProgramArgs&, GrAppliedClip&& clip);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const PathDrawList* fPathDrawList;
    const int fTotalCombinedPathVerbCnt;
    const int fPathCount;
    const GrTessellationPathFlags fPathFlags;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;
    SkDEBUGCODE(SkRect fOriginalDrawBounds;)

    // Decided during prePreparePrograms.
    GrPathTessellator* fTessellator = nullptr;
    const GrProgramInfo* fStencilFanProgram = nullptr;
    const GrProgramInfo* fStencilPathProgram = nullptr;
    const GrProgramInfo* fCoverBBoxProgram = nullptr;

    // Filled during onPrepare.
    sk_sp<const GrBuffer> fFanBuffer;
    int fFanBaseVertex = 0;
    int fFanVertexCount = 0;

    sk_sp<const GrBuffer> fBBoxBuffer;
    int fBBoxBaseInstance = 0;

    // Only used if sk_VertexID is not supported.
    sk_sp<const GrGpuBuffer> fBBoxVertexBufferIfNoIDSupport;

    friend class GrOp;  // For ctor.
};

} // namespace skgpu::v1

#endif // PathStencilCoverOp_DEFINED
