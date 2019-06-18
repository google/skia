/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrSamplerState.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
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
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"

#include <memory>
#include <utility>

class GrAppliedClip;
class GrGLSLProgramDataManager;

namespace skiagm {

static constexpr GrGeometryProcessor::Attribute gVertex =
        {"vertex", kFloat2_GrVertexAttribType, kFloat2_GrSLType};

/**
 * This is a GPU-backend specific test. It ensures that SkSL properly identifies clockwise-winding
 * triangles (sk_Clockwise), in terms of to Skia device space, in all backends and with all render
 * target origins. We draw clockwise triangles green and counter-clockwise red.
 */
class ClockwiseGM : public GpuGM {
private:
    SkString onShortName() final { return SkString("clockwise"); }
    SkISize onISize() override { return SkISize::Make(300, 200); }
    void onDraw(GrContext*, GrRenderTargetContext*, SkCanvas*) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class ClockwiseTestProcessor : public GrGeometryProcessor {
public:
    ClockwiseTestProcessor(bool readSkFragCoord)
            : GrGeometryProcessor(kClockwiseTestProcessor_ClassID)
            , fReadSkFragCoord(readSkFragCoord) {
        this->setVertexAttributes(&gVertex, 1);
    }
    const char* name() const override { return "ClockwiseTestProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(fReadSkFragCoord);
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const bool fReadSkFragCoord;

    friend class GLSLClockwiseTestProcessor;
};

class GLSLClockwiseTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) override {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const ClockwiseTestProcessor& proc = args.fGP.cast<ClockwiseTestProcessor>();
        args.fVaryingHandler->emitAttributes(proc);
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");
        args.fFragBuilder->codeAppendf(
                "%s = sk_Clockwise ? half4(0,1,0,1) : half4(1,0,0,1);", args.fOutputColor);
        if (!proc.fReadSkFragCoord) {
            args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
        } else {
            // Verify layout(origin_upper_left) on gl_FragCoord does not affect gl_FrontFacing.
            args.fFragBuilder->codeAppendf("%s = half4(min(half(sk_FragCoord.y), 1));",
                                           args.fOutputCoverage);
        }
    }
};

GrGLSLPrimitiveProcessor* ClockwiseTestProcessor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new GLSLClockwiseTestProcessor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

class ClockwiseTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          bool readSkFragCoord, int y = 0) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();
        return pool->allocate<ClockwiseTestOp>(readSkFragCoord, y);
    }

private:
    ClockwiseTestOp(bool readSkFragCoord, float y)
            : GrDrawOp(ClassID()), fReadSkFragCoord(readSkFragCoord), fY(y) {
        this->setBounds(SkRect::MakeIWH(300, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "ClockwiseTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        SkPoint vertices[4] = {
            {100, fY},
            {0, fY+100},
            {0, fY},
            {100, fY+100},
        };
        sk_sp<const GrBuffer> vertexBuffer(flushState->resourceProvider()->createBuffer(
                sizeof(vertices), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, vertices));
        if (!vertexBuffer) {
            return;
        }
        GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kPlus);
        GrMesh mesh(GrPrimitiveType::kTriangleStrip);
        mesh.setNonIndexedNonInstanced(4);
        mesh.setVertexData(std::move(vertexBuffer));
        flushState->rtCommandBuffer()->draw(ClockwiseTestProcessor(fReadSkFragCoord), pipeline,
                                            nullptr, nullptr, &mesh, 1, SkRect::MakeIWH(100, 100));
    }

    const bool fReadSkFragCoord;
    const float fY;

    friend class ::GrOpMemoryPool; // for ctor
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

void ClockwiseGM::onDraw(GrContext* ctx, GrRenderTargetContext* rtc, SkCanvas* canvas) {
    rtc->clear(nullptr, { 0, 0, 0, 1 }, GrRenderTargetContext::CanClearFullscreen::kYes);

    // Draw the test directly to the frame buffer.
    rtc->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, false, 0));
    rtc->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, true, 100));

    // Draw the test to an off-screen, top-down render target.
    if (auto topLeftRTC = ctx->priv().makeDeferredRenderTargetContext(
            rtc->asSurfaceProxy()->backendFormat(), SkBackingFit::kExact, 100, 200,
            rtc->asSurfaceProxy()->config(), nullptr, 1, GrMipMapped::kNo,
            kTopLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes)) {
        topLeftRTC->clear(nullptr, SK_PMColor4fTRANSPARENT,
                          GrRenderTargetContext::CanClearFullscreen::kYes);
        topLeftRTC->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, false, 0));
        topLeftRTC->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, true, 100));
        rtc->drawTexture(GrNoClip(), sk_ref_sp(topLeftRTC->asTextureProxy()),
                         GrSamplerState::Filter::kNearest, SkBlendMode::kSrcOver,
                         SK_PMColor4fWHITE, {0, 0, 100, 200},
                         {100, 0, 200, 200}, GrAA::kNo, GrQuadAAFlags::kNone,
                         SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint, SkMatrix::I(),
                         nullptr);
    }

    // Draw the test to an off-screen, bottom-up render target.
    if (auto topLeftRTC = ctx->priv().makeDeferredRenderTargetContext(
            rtc->asSurfaceProxy()->backendFormat(), SkBackingFit::kExact, 100, 200,
            rtc->asSurfaceProxy()->config(), nullptr, 1, GrMipMapped::kNo,
            kBottomLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes)) {
        topLeftRTC->clear(nullptr, SK_PMColor4fTRANSPARENT,
                          GrRenderTargetContext::CanClearFullscreen::kYes);
        topLeftRTC->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, false, 0));
        topLeftRTC->priv().testingOnly_addDrawOp(ClockwiseTestOp::Make(ctx, true, 100));
        rtc->drawTexture(GrNoClip(), sk_ref_sp(topLeftRTC->asTextureProxy()),
                         GrSamplerState::Filter::kNearest, SkBlendMode::kSrcOver,
                         SK_PMColor4fWHITE, {0, 0, 100, 200},
                         {200, 0, 300, 200}, GrAA::kNo, GrQuadAAFlags::kNone,
                         SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint, SkMatrix::I(),
                         nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ClockwiseGM(); )

}
