/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathTessellateOp_DEFINED
#define PathTessellateOp_DEFINED

#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

namespace skgpu {

class PathTessellator;

};

namespace skgpu::v1 {

// Tessellates a path directly to the color buffer, using one single render pass. This currently
// only works for convex paths.
class PathTessellateOp final : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    PathTessellateOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
                     GrAAType aaType, const GrUserStencilSettings* stencil,
                     const SkRect& drawBounds)
            : GrDrawOp(ClassID())
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fStencil(stencil)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkASSERT(!fPath.isInverseFillType());
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "PathTessellateOp"; }
    bool usesMSAA() const override { return fAAType == GrAAType::kMSAA; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;
    bool usesStencil() const override { return !fStencil->isUnused(); }

    void prepareTessellator(const GrTessellationShader::ProgramArgs&, GrAppliedClip&& clip);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    const GrUserStencilSettings* const fStencil;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    // Decided during prepareTessellator.
    PathTessellator* fTessellator = nullptr;
    const GrProgramInfo* fTessellationProgram = nullptr;

    friend class GrOp;  // For ctor.
};

} // namespace skgpu::v1

#endif // PathTessellateOp_DEFINED
