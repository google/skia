/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathStencilCoverOp_DEFINED
#define GrPathStencilCoverOp_DEFINED

#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

class GrPathTessellator;

// Draws paths using a standard Redbook "stencil then cover" method. Curves get linearized by either
// GPU tessellation shaders or indirect draws. This Op doesn't apply analytic AA, so it requires
// MSAA if AA is desired.
class GrPathStencilCoverOp : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    GrPathStencilCoverOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
                         GrAAType aaType, GrTessellationPathRenderer::PathFlags pathFlags,
                         const SkRect& devBounds)
            : GrDrawOp(ClassID())
            , fPathFlags(pathFlags)
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        this->setBounds(devBounds, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrPathStencilCoverOp"; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

    // Chooses the rendering method we will use and creates the corresponding tessellator and
    // stencil/cover programs.
    void prePreparePrograms(const GrTessellationShader::ProgramArgs&, GrAppliedClip&& clip);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrTessellationPathRenderer::PathFlags fPathFlags;
    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

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

    friend class GrOp;  // For ctor.
};

#endif
