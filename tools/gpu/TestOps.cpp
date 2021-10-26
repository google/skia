/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestOps.h"

#include "src/core/SkPointPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
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

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        class Impl : public ProgramImpl {
        public:
            void setData(const GrGLSLProgramDataManager& pdman,
                         const GrShaderCaps& shaderCaps,
                         const GrGeometryProcessor& geomProc) override {
                const auto& gp = geomProc.cast<GP>();
                SetTransform(pdman, shaderCaps, fLocalMatrixUni, gp.fLocalMatrix);
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& gp = args.fGeomProc.cast<GP>();
                args.fVaryingHandler->emitAttributes(gp);
                GrGLSLVarying colorVarying(kHalf4_GrSLType);
                args.fVaryingHandler->addVarying("color", &colorVarying,
                                                 GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
                args.fVertBuilder->codeAppendf("%s = %s;", colorVarying.vsOut(), gp.fInColor.name());
                args.fFragBuilder->codeAppendf("half4 %s = %s;",
                                               args.fOutputColor, colorVarying.fsIn());
                args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
                WriteOutputPosition(args.fVertBuilder, gpArgs, gp.fInPosition.name());
                WriteLocalCoord(args.fVertBuilder,
                                args.fUniformHandler,
                                *args.fShaderCaps,
                                gpArgs,
                                gp.fInLocalCoords.asShaderVar(),
                                gp.fLocalMatrix,
                                &fLocalMatrixUni);
            }

            UniformHandle fLocalMatrixUni;
        };

        return std::make_unique<Impl>();
    }

    void addToKey(const GrShaderCaps& shaderCaps, GrProcessorKeyBuilder* b) const override {
        b->add32(ProgramImpl::ComputeMatrixKey(shaderCaps, fLocalMatrix));
    }

    bool wideColor() const { return fInColor.cpuType() != kUByte4_norm_GrVertexAttribType; }

private:
    Attribute fInPosition    = {   "inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInLocalCoords = {"inLocalCoords", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInColor;

    SkMatrix fLocalMatrix;
};

class TestRectOp final : public GrMeshDrawOp {
public:
    static GrOp::Owner Make(GrRecordingContext*,
                            GrPaint&&,
                            const SkRect& drawRect,
                            const SkRect& localRect,
                            const SkMatrix& localM);

    const char* name() const override { return "TestRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps&,
                                      const GrAppliedClip*,
                                      GrClampType) override;

    void visitProxies(const GrVisitProxyFunc& func) const override {
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
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override;

    void onPrepareDraws(GrMeshDrawTarget*) override;
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

    friend class ::GrOp;
};

GrOp::Owner TestRectOp::Make(GrRecordingContext* context,
                             GrPaint&& paint,
                             const SkRect& drawRect,
                             const SkRect& localRect,
                             const SkMatrix& localM) {
    const auto* caps = context->priv().caps();
    return GrOp::Make<TestRectOp>(context, caps, std::move(paint), drawRect, localRect, localM);
}

GrProcessorSet::Analysis TestRectOp::finalize(const GrCaps& caps,
                                              const GrAppliedClip* clip,
                                              GrClampType clampType) {
    return fProcessorSet.finalize(GrProcessorAnalysisColor::Opaque::kYes,
                                  GrProcessorAnalysisCoverage::kSingleChannel, clip,
                                  &GrUserStencilSettings::kUnused, caps, clampType, &fColor);
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
                                     const GrSurfaceProxyView& writeView,
                                     bool usesMSAASurface,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) {
    fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps,
                                                               arena,
                                                               writeView,
                                                               usesMSAASurface,
                                                               std::move(appliedClip),
                                                               dstProxyView,
                                                               &fGP,
                                                               std::move(fProcessorSet),
                                                               GrPrimitiveType::kTriangles,
                                                               renderPassXferBarriers,
                                                               colorLoadOp,
                                                               GrPipeline::InputFlags::kNone);
}

void TestRectOp::onPrepareDraws(GrMeshDrawTarget* target) {
    QuadHelper helper(target, fGP.vertexStride(), 1);
    skgpu::VertexWriter writer{helper.vertices()};
    auto pos = skgpu::VertexWriter::TriStripFromRect(fDrawRect);
    auto local = skgpu::VertexWriter::TriStripFromRect(fLocalRect);
    GrVertexColor color(fColor, fGP.wideColor());
    writer.writeQuad(pos, local, color);

    fMesh = helper.mesh();
}

void TestRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

}  // anonymous namespace

namespace sk_gpu_test::test_ops {

GrOp::Owner MakeRect(GrRecordingContext* context,
                     GrPaint&& paint,
                     const SkRect& drawRect,
                     const SkRect& localRect,
                     const SkMatrix& localM) {
    return TestRectOp::Make(context, std::move(paint), drawRect, localRect, localM);
}

GrOp::Owner MakeRect(GrRecordingContext* context,
                     std::unique_ptr<GrFragmentProcessor> fp,
                     const SkRect& drawRect,
                     const SkRect& localRect,
                     const SkMatrix& localM) {
    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));
    return TestRectOp::Make(context, std::move(paint), drawRect, localRect, localM);
}

GrOp::Owner MakeRect(GrRecordingContext* context,
                     GrPaint&& paint,
                     const SkRect& rect) {
    return TestRectOp::Make(context, std::move(paint), rect, rect, SkMatrix::I());
}

}  // namespace sk_gpu_test::test_ops
