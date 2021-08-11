/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/gpu/ProxyUtils.h"

#include <memory>
#include <utility>

class GrAppliedClip;

/**
 * This test ensures that fwidth() works properly on GPU configs by drawing a squircle.
 */
namespace {

static constexpr GrGeometryProcessor::Attribute gVertex =
        {"bboxcoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class FwidthSquircleTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix) {
        return arena->make([&](void* ptr) {
            return new (ptr) FwidthSquircleTestProcessor(viewMatrix);
        });
    }

    const char* name() const override { return "FwidthSquircleTestProcessor"; }

    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

private:
    FwidthSquircleTestProcessor(const SkMatrix& viewMatrix)
            : GrGeometryProcessor(kFwidthSquircleTestProcessor_ClassID)
            , fViewMatrix(viewMatrix) {
        this->setVertexAttributes(&gVertex, 1);
    }

    const SkMatrix fViewMatrix;

    using INHERITED = GrGeometryProcessor;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> FwidthSquircleTestProcessor::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrShaderCaps&,
                     const GrGeometryProcessor& geomProc) override {
            const auto& proc = geomProc.cast<FwidthSquircleTestProcessor>();
            pdman.setSkMatrix(fViewMatrixHandle, proc.fViewMatrix);
        }

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const auto& proc = args.fGeomProc.cast<FwidthSquircleTestProcessor>();

            auto* uniforms = args.fUniformHandler;
            fViewMatrixHandle = uniforms->addUniform(nullptr,
                                                     kVertex_GrShaderFlag,
                                                     kFloat3x3_GrSLType,
                                                     "viewmatrix");

            auto* varyings = args.fVaryingHandler;
            varyings->emitAttributes(proc);

            GrGLSLVarying squircleCoord(kFloat2_GrSLType);
            varyings->addVarying("bboxcoord", &squircleCoord);

            auto* v = args.fVertBuilder;
            v->codeAppendf("float2x2 R = float2x2(cos(.05), sin(.05), -sin(.05), cos(.05));");

            v->codeAppendf("%s = bboxcoord * 1.25;", squircleCoord.vsOut());
            v->codeAppendf("float3 vertexpos = float3(bboxcoord * 100 * R + 100, 1);");
            v->codeAppendf("vertexpos = %s * vertexpos;",
                           uniforms->getUniformCStr(fViewMatrixHandle));
            gpArgs->fPositionVar.set(kFloat3_GrSLType, "vertexpos");

            auto* f = args.fFragBuilder;
            f->codeAppendf("float golden_ratio = 1.61803398875;");
            f->codeAppendf("float pi = 3.141592653589793;");
            f->codeAppendf("float x = abs(%s.x), y = abs(%s.y);",
                           squircleCoord.fsIn(), squircleCoord.fsIn());

            // Squircle function!
            f->codeAppendf("float fn = half(pow(x, golden_ratio*pi) + "
                           "pow(y, golden_ratio*pi) - 1);");
            f->codeAppendf("float fnwidth = fwidth(fn);");
            f->codeAppendf("fnwidth += 1e-10;");  // Guard against divide-by-zero.
            f->codeAppendf("half coverage = clamp(half(.5 - fn/fnwidth), 0, 1);");

            f->codeAppendf("half4 %s = half4(.51, .42, .71, 1) * .89;", args.fOutputColor);
            f->codeAppendf("half4 %s = half4(coverage);", args.fOutputCoverage);
        }

        UniformHandle fViewMatrixHandle;
    };

    return std::make_unique<Impl>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

class FwidthSquircleTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* ctx, const SkMatrix& viewMatrix) {
        return GrOp::Make<FwidthSquircleTestOp>(ctx, viewMatrix);
    }

private:
    FwidthSquircleTestOp(const SkMatrix& viewMatrix)
            : GrDrawOp(ClassID())
            , fViewMatrix(viewMatrix) {
        this->setBounds(SkRect::MakeIWH(kWidth, kHeight), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "FwidthSquircleTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) const {
        GrGeometryProcessor* geomProc = FwidthSquircleTestProcessor::Make(arena, fViewMatrix);

        return sk_gpu_test::CreateProgramInfo(caps, arena, writeView,
                                              std::move(appliedClip), dstProxyView,
                                              geomProc, SkBlendMode::kSrcOver,
                                              GrPrimitiveType::kTriangleStrip,
                                              renderPassXferBarriers, colorLoadOp);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->writeView(),
                                       flushState->detachAppliedClip(),
                                       flushState->dstProxyView(),
                                       flushState->renderPassBarriers(),
                                       flushState->colorLoadOp());
    }

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip* clip,
                      const GrDstProxyView& dstProxyView,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {
        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip::Disabled();

        fProgramInfo = this->createProgramInfo(context->priv().caps(), arena, writeView,
                                               std::move(appliedClip), dstProxyView,
                                               renderPassXferBarriers, colorLoadOp);

        context->priv().recordProgramInfo(fProgramInfo);
    }

    void onPrepare(GrOpFlushState* flushState) final {
        SkPoint vertices[4] = {
            {-1, -1},
            {+1, -1},
            {-1, +1},
            {+1, +1},
        };
        fVertexBuffer = flushState->resourceProvider()->createBuffer(
                sizeof(vertices), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, vertices);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) final {
        if (!fVertexBuffer) {
            return;
        }

        if (!fProgramInfo) {
            fProgramInfo = this->createProgramInfo(flushState);
        }

        flushState->bindPipeline(*fProgramInfo, SkRect::MakeIWH(kWidth, kHeight));
        flushState->bindBuffers(nullptr, nullptr, std::move(fVertexBuffer));
        flushState->draw(4, 0);

    }

    static const int kWidth = 200;
    static const int kHeight = 200;

    sk_sp<GrBuffer> fVertexBuffer;
    const SkMatrix  fViewMatrix;

    // The program info (and both the GrPipeline and GrGeometryProcessor it relies on), when
    // allocated, are allocated in either the ddl-record-time or flush-time arena. It is the
    // arena's job to free up their memory so we just have a bare programInfo pointer here. We
    // don't even store the GrPipeline and GrGeometryProcessor pointers here bc they are
    // guaranteed to have the same lifetime as the program info.
    GrProgramInfo*  fProgramInfo = nullptr;

    friend class ::GrOp; // for ctor

    using INHERITED = GrDrawOp;
};

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

namespace skiagm {

DEF_SIMPLE_GPU_GM_CAN_FAIL(fwidth_squircle, rContext, canvas, errorMsg, 200, 200) {
    if (!rContext->priv().caps()->shaderCaps()->shaderDerivativeSupport()) {
        *errorMsg = "Shader derivatives not supported.";
        return DrawResult::kSkip;
    }

    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    // Draw the test directly to the frame buffer.
    canvas->clear(SK_ColorWHITE);
    sdc->addDrawOp(FwidthSquircleTestOp::Make(rContext, canvas->getTotalMatrix()));
    return skiagm::DrawResult::kOk;
}

}  // namespace skiagm
