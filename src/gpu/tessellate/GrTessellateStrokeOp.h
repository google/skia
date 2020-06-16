/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellateStrokeOp_DEFINED
#define GrTessellateStrokeOp_DEFINED

#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class GrAppliedHardClip;
class GrStencilPathShader;
class GrResolveLevelCounter;

// Blah
class GrTessellateStrokeOp : public GrDrawOp {
private:
    DEFINE_OP_CLASS_ID

    GrTessellateStrokeOp(const SkMatrix& viewMatrix, const SkPath& path,
                         const SkStrokeRec& strokeRec, GrPaint&& paint, GrAAType aaType,
                         GrTessellationPathRenderer::OpFlags opFlags)
            : GrDrawOp(ClassID())
            , fOpFlags(opFlags)
            , fViewMatrix(viewMatrix)
            , fMatrixScale(fViewMatrix.getMaxScale())
            , fPath(path)
            , fStrokeRec(strokeRec)
            , fAAType(aaType)
            , fNeedsStencil(fAAType == GrAAType::kCoverage ||
                            !paint.isConstantBlendedColor() ||
                            paint.numCoverageFragmentProcessors() ||
                            (opFlags & GrTessellationPathRenderer::OpFlags::kStencilOnly))
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkRect devBounds = path.getBounds();
        float inflationRadius = strokeRec.getInflationRadius();
        devBounds.outset(inflationRadius, inflationRadius);
        fViewMatrix.mapRect(&devBounds, path.getBounds());
        this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
    }

    const char* name() const override { return "GrTessellateStrokeOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      bool hasMixedSampledCoverage,
                                      GrClampType clampType) override {
        return fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
                hasMixedSampledCoverage, caps, clampType, &fColor);
    }

    FixedFunctionFlags fixedFunctionFlags() const override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override;
    void onPrepare(GrOpFlushState* state) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
    void drawStencilPass(GrOpFlushState*);
    void drawCoverPass(GrOpFlushState*);

    const GrTessellationPathRenderer::OpFlags fOpFlags;
    const SkMatrix fViewMatrix;
    const float fMatrixScale;
    const SkPath fPath;
    const SkStrokeRec fStrokeRec;
    const GrAAType fAAType;
    const bool fNeedsStencil;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    sk_sp<const GrBuffer> fIndirectDrawBuffer;
    size_t fIndirectDrawOffset;
    int fIndirectDrawCount = 0;
    sk_sp<const GrBuffer> fCubicsBuffer;
    int fCubicVertexCount = 0;
    int fBaseCubicVertex;
    sk_sp<const GrBuffer> fVertexBuffer;

    friend class GrOpMemoryPool;  // For ctor.
};

#endif
