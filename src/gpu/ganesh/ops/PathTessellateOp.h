/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathTessellateOp_DEFINED
#define PathTessellateOp_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/tessellate/GrTessellationShader.h"
#include "src/gpu/ganesh/tessellate/PathTessellator.h"
#include "src/gpu/tessellate/Tessellation.h"

#include <utility>

class GrAppliedClip;
class GrDstProxyView;
class GrOpFlushState;
class GrProgramInfo;
class GrRecordingContext;
class GrSurfaceProxyView;
enum class GrXferBarrierFlags;
struct SkRect;

namespace skgpu::ganesh {

// Tessellates a path directly to the color buffer, using one single render pass. This currently
// only works for convex paths.
class PathTessellateOp final : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    using PatchAttribs = PathTessellator::PatchAttribs;
    using PathDrawList = PathTessellator::PathDrawList;

    PathTessellateOp(SkArenaAlloc* arena,
                     GrAAType aaType,
                     const GrUserStencilSettings* stencil,
                     const SkMatrix& viewMatrix,
                     const SkPath& path,
                     GrPaint&& paint,
                     const SkRect& drawBounds)
            : GrDrawOp(ClassID())
            , fAAType(aaType)
            , fStencil(stencil)
            , fTotalCombinedPathVerbCnt(path.countVerbs())
            , fPathDrawList(arena->make<PathDrawList>(SkMatrix::I(), path, paint.getColor4f()))
            , fPathDrawTail(&fPathDrawList->fNext)
            , fProcessors(std::move(paint))
            , fShaderMatrix(viewMatrix) {
        SkASSERT(!path.isInverseFillType());
        if (!this->headDraw().fColor.fitsInBytes()) {
            fPatchAttribs |= PatchAttribs::kWideColorIfEnabled;
        }
        this->setBounds(drawBounds, HasAABloat::kNo, IsHairline::kNo);
    }

    PathDrawList& headDraw() { return *fPathDrawList; }

    void prepareTessellator(const GrTessellationShader::ProgramArgs&, GrAppliedClip&& clip);

    // GrDrawOp overrides.
    const char* name() const override { return "PathTessellateOp"; }
    bool usesMSAA() const override { return fAAType == GrAAType::kMSAA; }
    void visitProxies(const GrVisitProxyFunc&) const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;
    bool usesStencil() const override { return !fStencil->isUnused(); }
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrDstProxyView&, GrXferBarrierFlags, GrLoadOp colorLoadOp) override;
    void onPrepare(GrOpFlushState*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrAAType fAAType;
    const GrUserStencilSettings* const fStencil;
    int fTotalCombinedPathVerbCnt;
    PatchAttribs fPatchAttribs = PatchAttribs::kNone;
    PathDrawList* const fPathDrawList;
    PathDrawList** fPathDrawTail;
    GrProcessorSet fProcessors;
    SkMatrix fShaderMatrix;

    // Decided during prepareTessellator.
    PathTessellator* fTessellator = nullptr;
    const GrProgramInfo* fTessellationProgram = nullptr;

    friend class GrOp;  // For ctor.
};

}  // namespace skgpu::ganesh

#endif // PathTessellateOp_DEFINED
