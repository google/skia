/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestOps.h"

#include "src/core/SkPointPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

class GP : public GrGeometryProcessor {
public:
    GP(const SkMatrix& localMatrix, bool wideColor)
            : GrGeometryProcessor(kTestRectOp_ClassID), fLocalMatrix(localMatrix) {
        fInColor = MakeColorAttribute("color", wideColor);
        this->setVertexAttributes(&fInPosition, 3);
    }

    const char* name() const override { return "TestRectOp::GP"; }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override;

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool wideColor() const { return fInColor.cpuType() != kUByte4_norm_GrVertexAttribType; }

private:
    Attribute fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInLocalCoords = {"inLocalCoords", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInColor;
    SkMatrix fLocalMatrix;
};

GrGLSLPrimitiveProcessor* GP::createGLSLInstance(const GrShaderCaps& caps) const {
    class GLSLGP : public GrGLSLGeometryProcessor {
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& pp,
                     const CoordTransformRange& transformRange) override {
            const auto& gp = pp.cast<GP>();
            this->setTransformDataHelper(gp.fLocalMatrix, pdman, transformRange);
        }

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const auto& gp = args.fGP.cast<GP>();
            args.fVaryingHandler->emitAttributes(gp);
            GrGLSLVarying colorVarying(kHalf4_GrSLType);
            args.fVaryingHandler->addVarying("color", &colorVarying,
                                             GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
            args.fVertBuilder->codeAppendf("%s = %s;", colorVarying.vsOut(), gp.fInColor.name());
            args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, colorVarying.fsIn());
            args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
            this->writeOutputPosition(args.fVertBuilder, gpArgs, gp.fInPosition.name());
            this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                                 gp.fInLocalCoords.asShaderVar(), gp.fLocalMatrix,
                                 args.fFPCoordTransformHandler);
        }
    };
    return new GLSLGP();
}

class TestRectOp final : public GrMeshDrawOp {
public:
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                          GrPaint&&,
                                          const SkRect& drawRect,
                                          const SkRect& localRect,
                                          const SkMatrix& localM);

    const char* name() const override { return "TestRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps&,
                                      const GrAppliedClip*,
                                      bool hasMixedSampledCoverage,
                                      GrClampType) override;

    void visitProxies(const VisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fProcessorSet.visitProxies(func);
        }
    }

private:
    DEFINE_OP_CLASS_ID

    TestRectOp(const GrCaps*,
               GrPaint&&,
               const SkRect& drawRect,
               const SkRect& localRect,
               const SkMatrix& localMatrix);

    GrProgramInfo* programInfo() override { return fProgramInfo; }
    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView* outputView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&) override;

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    SkRect         fDrawRect;
    SkRect         fLocalRect;
    SkPMColor4f    fColor;
    GP             fGP;
    GrProcessorSet fProcessorSet;

    // If this op is prePrepared the created programInfo will be stored here for use in
    // onExecute. In the prePrepared case it will have been stored in the record-time arena.
    GrProgramInfo* fProgramInfo = nullptr;
    GrSimpleMesh*  fMesh        = nullptr;

    friend class ::GrOpMemoryPool;
};

std::unique_ptr<GrDrawOp> TestRectOp::Make(GrRecordingContext* context,
                                           GrPaint&& paint,
                                           const SkRect& drawRect,
                                           const SkRect& localRect,
                                           const SkMatrix& localM) {
    auto* pool = context->priv().opMemoryPool();
    const auto* caps = context->priv().caps();
    return pool->allocate<TestRectOp>(caps, std::move(paint), drawRect, localRect, localM);
}

GrProcessorSet::Analysis TestRectOp::finalize(const GrCaps& caps,
                                              const GrAppliedClip* clip,
                                              bool hasMixedSampledCoverage,
                                              GrClampType clampType) {
    return fProcessorSet.finalize(GrProcessorAnalysisColor::Opaque::kYes,
                                  GrProcessorAnalysisCoverage::kSingleChannel, clip,
                                  &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
                                  clampType, &fColor);
}

static bool use_wide_color(const GrPaint& paint, const GrCaps* caps) {
    return !paint.getColor4f().fitsInBytes() && caps->halfFloatVertexAttributeSupport();
}
TestRectOp::TestRectOp(const GrCaps* caps,
                       GrPaint&& paint,
                       const SkRect& drawRect,
                       const SkRect& localRect,
                       const SkMatrix& localMatrix)
        : GrMeshDrawOp(ClassID())
        , fDrawRect(drawRect)
        , fLocalRect(localRect)
        , fColor(paint.getColor4f())
        , fGP(localMatrix, use_wide_color(paint, caps))
        , fProcessorSet(std::move(paint)) {
    this->setBounds(drawRect.makeSorted(), HasAABloat::kNo, IsHairline::kNo);
}

void TestRectOp::onCreateProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView* outputView,
                                     GrAppliedClip&& appliedClip,
                                     const GrXferProcessor::DstProxyView& dstProxyView) {
    fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps,
                                                               arena,
                                                               outputView,
                                                               std::move(appliedClip),
                                                               dstProxyView,
                                                               &fGP,
                                                               std::move(fProcessorSet),
                                                               GrPrimitiveType::kTriangles,
                                                               GrPipeline::InputFlags::kNone);
}

void TestRectOp::onPrepareDraws(Target* target) {
    QuadHelper helper(target, fGP.vertexStride(), 1);
    GrVertexWriter writer{helper.vertices()};
    auto pos = GrVertexWriter::TriStripFromRect(fDrawRect);
    auto local = GrVertexWriter::TriStripFromRect(fLocalRect);
    GrVertexColor color(fColor, fGP.wideColor());
    writer.writeQuad(pos, local, color);

    fMesh = helper.mesh();
}

void TestRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

}  // anonymous namespace

namespace sk_gpu_test::test_ops {

std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext* context,
                                   GrPaint&& paint,
                                   const SkRect& drawRect,
                                   const SkRect& localRect,
                                   const SkMatrix& localM) {
    return TestRectOp::Make(context, std::move(paint), drawRect, localRect, localM);
}

std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext* context,
                                   std::unique_ptr<GrFragmentProcessor> fp,
                                   const SkRect& drawRect,
                                   const SkRect& localRect,
                                   const SkMatrix& localM) {
    GrPaint paint;
    paint.addColorFragmentProcessor(std::move(fp));
    return TestRectOp::Make(context, std::move(paint), drawRect, localRect, localM);
}

std::unique_ptr<GrDrawOp> MakeRect(GrRecordingContext* context,
                                   GrPaint&& paint,
                                   const SkRect& rect) {
    return TestRectOp::Make(context, std::move(paint), rect, rect, SkMatrix::I());
}

}  // namespace sk_gpu_test::test_ops
