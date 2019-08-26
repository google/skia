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
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorSet.h"
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
    FwidthSquircleTestProcessor(const SkMatrix& viewMatrix)
            : GrGeometryProcessor(kFwidthSquircleTestProcessor_ClassID)
            , fViewMatrix(viewMatrix) {
        this->setVertexAttributes(&gVertex, 1);
    }
    const char* name() const override { return "FwidthSquircleTestProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const SkMatrix fViewMatrix;

    class Impl;
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
                 FPCoordTransformIter&& transformIter) override {
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
        this->setBounds(SkRect::MakeIWH(200, 200), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "ClockwiseTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        SkPoint vertices[4] = {
            {-1, -1},
            {+1, -1},
            {-1, +1},
            {+1, +1},
        };
        sk_sp<const GrBuffer> vertexBuffer(flushState->resourceProvider()->createBuffer(
                sizeof(vertices), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, vertices));
        if (!vertexBuffer) {
            return;
        }
        GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kSrcOver,
                            flushState->drawOpArgs().fOutputSwizzle);
        GrMesh mesh(GrPrimitiveType::kTriangleStrip);
        mesh.setNonIndexedNonInstanced(4);
        mesh.setVertexData(std::move(vertexBuffer));
        flushState->opsRenderPass()->draw(FwidthSquircleTestProcessor(fViewMatrix), pipeline,
                                          nullptr, nullptr, &mesh, 1, SkRect::MakeIWH(100, 100));
    }

    const SkMatrix fViewMatrix;

    friend class ::GrOpMemoryPool; // for ctor
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
