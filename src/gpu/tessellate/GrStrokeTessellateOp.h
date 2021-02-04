/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateOp_DEFINED
#define GrStrokeTessellateOp_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrSTArenaList.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrPathShader.h"

class GrRecordingContext;

// Prepares GPU data for, and then draws a stroke's tessellated geometry.
class GrStrokeTessellator {
public:
    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate.
    virtual void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const GrSTArenaList<SkPath>&,
                         const SkStrokeRec&, int totalCombinedVerbCnt) = 0;

    // Issues draw calls for the tessellated stroie. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    virtual ~GrStrokeTessellator() {}
};

// Base class for ops that render opaque, constant-color strokes by linearizing them into sorted
// "parametric" and "radial" edges. See GrStrokeTessellateShader.
class GrStrokeTessellateOp : public GrDrawOp {
public:
    // The provided matrix must be a similarity matrix for the time being. This is so we can
    // bootstrap this Op on top of GrStrokeGeometry with minimal modifications.
    //
    // Patches can overlap, so until a stencil technique is implemented, the provided paint must be
    // a constant blended color.
    GrStrokeTessellateOp(GrAAType, const SkMatrix&, const SkPath&, const SkStrokeRec&, GrPaint&&);

protected:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "GrStrokeTessellateOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override;
    FixedFunctionFlags fixedFunctionFlags() const override;
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    // Creates the tessellator and the stencil/fill program(s) we will use with it.
    void prePrepareTessellator(GrPathShader::ProgramArgs&&, GrAppliedClip&&);

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView&, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&, GrXferBarrierFlags,
                      GrLoadOp colorLoadOp) override;

    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const GrAAType fAAType;
    const SkMatrix fViewMatrix;
    const SkStrokeRec fStroke;
    SkPMColor4f fColor;
    bool fNeedsStencil = false;
    GrProcessorSet fProcessors;

    GrSTArenaList<SkPath> fPathList;
    int fTotalCombinedVerbCnt = 0;
    bool fHasConics = false;

    GrStrokeTessellator* fTessellator = nullptr;
    const GrProgramInfo* fStencilProgram = nullptr;  // Only used if the stroke has transparency.
    const GrProgramInfo* fFillProgram = nullptr;
};

#endif
