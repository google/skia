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
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "tools/gpu/ProxyUtils.h"

#include <memory>
#include <utility>

class GrAppliedClip;

/**
 * This test ensures that fwidth() works properly on GPU configs by drawing a squircle.
 */
namespace skiagm {

static constexpr GrGeometryProcessor::Attribute gVertex =
        {"bboxcoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class FwidthSquircleTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix) {
        return arena->make<FwidthSquircleTestProcessor>(viewMatrix);
    }

    const char* name() const override { return "FwidthSquircleTestProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    friend class ::SkArenaAlloc; // for access to ctor

    FwidthSquircleTestProcessor(const SkMatrix& viewMatrix)
            : GrGeometryProcessor(kFwidthSquircleTestProcessor_ClassID)
            , fViewMatrix(viewMatrix) {
        this->setVertexAttributes(&gVertex, 1);
    }

    const SkMatrix fViewMatrix;

    class Impl;

    typedef GrGeometryProcessor INHERITED;
};

class FwidthSquircleTestProcessor::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<FwidthSquircleTestProcessor>();

        auto* uniforms = args.fUniformHandler;
        fViewMatrixHandle =
                uniforms->addUniform(kVertex_GrShaderFlag, kFloat3x3_GrSLType, "viewmatrix");

        auto* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);

        GrGLSLVarying squircleCoord(kFloat2_GrSLType);
        varyings->addVarying("bboxcoord", &squircleCoord);

        auto* v = args.fVertBuilder;
        v->codeAppendf("float2x2 R = float2x2(cos(.05), sin(.05), -sin(.05), cos(.05));");

        v->codeAppendf("%s = bboxcoord * 1.25;", squircleCoord.vsOut());
        v->codeAppendf("float3 vertexpos = float3(bboxcoord * 100 * R + 100, 1);");
        v->codeAppendf("vertexpos = %s * vertexpos;", uniforms->getUniformCStr(fViewMatrixHandle));
        gpArgs->fPositionVar.set(kFloat3_GrSLType, "vertexpos");

        auto* f = args.fFragBuilder;
        f->codeAppendf("float golden_ratio = 1.61803398875;");
        f->codeAppendf("float pi = 3.141592653589793;");
        f->codeAppendf("float x = abs(%s.x), y = abs(%s.y);",
                       squircleCoord.fsIn(), squircleCoord.fsIn());

        // Squircle function!
        f->codeAppendf("float fn = half(pow(x, golden_ratio*pi) + pow(y, golden_ratio*pi) - 1);");
        f->codeAppendf("float fnwidth = fwidth(fn);");
        f->codeAppendf("fnwidth += 1e-10;");  // Guard against divide-by-zero.
        f->codeAppendf("half coverage = clamp(half(.5 - fn/fnwidth), 0, 1);");

        f->codeAppendf("%s = half4(.51, .42, .71, 1) * .89;", args.fOutputColor);
        f->codeAppendf("%s = half4(coverage);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange&) override {
        const auto& proc = primProc.cast<FwidthSquircleTestProcessor>();
        pdman.setSkMatrix(fViewMatrixHandle, proc.fViewMatrix);
    }

    UniformHandle fViewMatrixHandle;
};

GrGLSLPrimitiveProcessor* FwidthSquircleTestProcessor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

class FwidthSquircleTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* ctx, const SkMatrix& viewMatrix) {
        GrOpMemoryPool* pool = ctx->priv().opMemoryPool();
        return pool->allocate<FwidthSquircleTestOp>(viewMatrix);
    }

private:
    FwidthSquircleTestOp(const SkMatrix& viewMatrix)
            : GrDrawOp(ClassID())
            , fViewMatrix(viewMatrix) {
        this->setBounds(SkRect::MakeIWH(kWidth, kHeight), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "FwidthSquircleTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView* outputView,
                                     GrAppliedClip&& appliedClip,
                                     const GrXferProcessor::DstProxyView& dstProxyView) const {
        GrGeometryProcessor* geomProc = FwidthSquircleTestProcessor::Make(arena, fViewMatrix);

        return sk_gpu_test::CreateProgramInfo(caps, arena, outputView,
                                              std::move(appliedClip), dstProxyView,
                                              geomProc, SkBlendMode::kSrcOver,
                                              GrPrimitiveType::kTriangleStrip);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->outputView(),
                                       flushState->detachAppliedClip(),
                                       flushState->dstProxyView());
    }

    void onPrePrepare(GrRecordingContext* context,
                      const GrSurfaceProxyView* outputView,
                      GrAppliedClip* clip,
                      const GrXferProcessor::DstProxyView& dstProxyView) final {
        SkArenaAlloc* arena = context->priv().recordTimeAllocator();

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip();

        fProgramInfo = this->createProgramInfo(context->priv().caps(), arena, outputView,
                                               std::move(appliedClip), dstProxyView);

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
        flushState->bindBuffers(nullptr, nullptr, fVertexBuffer.get());
        flushState->draw(4, 0);

    }

    static const int kWidth = 200;
    static const int kHeight = 200;

    sk_sp<GrBuffer> fVertexBuffer;
    const SkMatrix  fViewMatrix;

    // The program info (and both the GrPipeline and GrPrimitiveProcessor it relies on), when
    // allocated, are allocated in either the ddl-record-time or flush-time arena. It is the
    // arena's job to free up their memory so we just have a bare programInfo pointer here. We
    // don't even store the GrPipeline and GrPrimitiveProcessor pointers here bc they are
    // guaranteed to have the same lifetime as the program info.
    GrProgramInfo*  fProgramInfo = nullptr;

    friend class ::GrOpMemoryPool; // for ctor

    typedef GrDrawOp INHERITED;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

DEF_SIMPLE_GPU_GM_CAN_FAIL(fwidth_squircle, ctx, rtc, canvas, errorMsg, 200, 200) {
    if (!ctx->priv().caps()->shaderCaps()->shaderDerivativeSupport()) {
        *errorMsg = "Shader derivatives not supported.";
        return DrawResult::kSkip;
    }

    // Draw the test directly to the frame buffer.
    canvas->clear(SK_ColorWHITE);
    rtc->priv().testingOnly_addDrawOp(FwidthSquircleTestOp::Make(ctx, canvas->getTotalMatrix()));
    return skiagm::DrawResult::kOk;
}

}
