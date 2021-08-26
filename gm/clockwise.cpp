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
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
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
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "tools/gpu/ProxyUtils.h"

#include <memory>
#include <utility>

class GrAppliedClip;
class GrGLSLProgramDataManager;

namespace {

static constexpr GrGeometryProcessor::Attribute gVertex =
        {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkSL code.

class ClockwiseTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, bool readSkFragCoord) {
        return arena->make([&](void* ptr) {
            return new (ptr) ClockwiseTestProcessor(readSkFragCoord);
        });
    }

    const char* name() const final { return "ClockwiseTestProcessor"; }

    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(fReadSkFragCoord);
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    bool readSkFragCoord() const { return fReadSkFragCoord; }

private:
    ClockwiseTestProcessor(bool readSkFragCoord)
            : GrGeometryProcessor(kClockwiseTestProcessor_ClassID)
            , fReadSkFragCoord(readSkFragCoord) {
        this->setVertexAttributes(&gVertex, 1);
    }

    const bool fReadSkFragCoord;

    using INHERITED = GrGeometryProcessor;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> ClockwiseTestProcessor::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager&,
                     const GrShaderCaps&,
                     const GrGeometryProcessor&) override {}

    private:
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const ClockwiseTestProcessor& proc = args.fGeomProc.cast<ClockwiseTestProcessor>();
            args.fVaryingHandler->emitAttributes(proc);
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "position");
            args.fFragBuilder->codeAppendf(
                    "half4 %s = sk_Clockwise ? half4(0,1,0,1) : half4(1,0,0,1);",
                    args.fOutputColor);
            if (!proc.readSkFragCoord()) {
                args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
            } else {
                // Verify layout(origin_upper_left) on gl_FragCoord does not affect gl_FrontFacing.
                args.fFragBuilder->codeAppendf("half4 %s = half4(min(half(sk_FragCoord.y), 1));",
                                               args.fOutputCoverage);
            }
        }
    };

    return std::make_unique<Impl>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Draw Op.

class ClockwiseTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            bool readSkFragCoord, int y = 0) {
        return GrOp::Make<ClockwiseTestOp>(context, readSkFragCoord, y);
    }

private:
    ClockwiseTestOp(bool readSkFragCoord, float y)
            : GrDrawOp(ClassID())
            , fReadSkFragCoord(readSkFragCoord)
            , fY(y) {
        this->setBounds(SkRect::MakeXYWH(0, fY, 100, 100), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "ClockwiseTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    GrProgramInfo* createProgramInfo(const GrCaps* caps,
                                     SkArenaAlloc* arena,
                                     const GrSurfaceProxyView& writeView,
                                     bool usesMSAASurface,
                                     GrAppliedClip&& appliedClip,
                                     const GrDstProxyView& dstProxyView,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp) const {
        GrGeometryProcessor* geomProc = ClockwiseTestProcessor::Make(arena, fReadSkFragCoord);

        return sk_gpu_test::CreateProgramInfo(caps, arena, writeView, usesMSAASurface,
                                              std::move(appliedClip), dstProxyView,
                                              geomProc, SkBlendMode::kPlus,
                                              GrPrimitiveType::kTriangleStrip,
                                              renderPassXferBarriers, colorLoadOp);
    }

    GrProgramInfo* createProgramInfo(GrOpFlushState* flushState) const {
        return this->createProgramInfo(&flushState->caps(),
                                       flushState->allocator(),
                                       flushState->writeView(),
                                       flushState->usesMSAASurface(),
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

        // DMSAA is not supported on DDL.
        bool usesMSAASurface = writeView.asRenderTargetProxy()->numSamples() > 1;

        // This is equivalent to a GrOpFlushState::detachAppliedClip
        GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip::Disabled();

        fProgramInfo = this->createProgramInfo(context->priv().caps(), arena, writeView,
                                               usesMSAASurface, std::move(appliedClip),
                                               dstProxyView, renderPassXferBarriers, colorLoadOp);

        context->priv().recordProgramInfo(fProgramInfo);
    }

    void onPrepare(GrOpFlushState* flushState) override {
        SkPoint vertices[4] = {
            {100, fY},
            {0, fY+100},
            {0, fY},
            {100, fY+100},
        };
        fVertexBuffer = flushState->resourceProvider()->createBuffer(
                sizeof(vertices), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, vertices);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fVertexBuffer) {
            return;
        }

        if (!fProgramInfo) {
            fProgramInfo = this->createProgramInfo(flushState);
        }

        flushState->bindPipeline(*fProgramInfo, SkRect::MakeXYWH(0, fY, 100, 100));
        flushState->bindBuffers(nullptr, nullptr, std::move(fVertexBuffer));
        flushState->draw(4, 0);
    }

    sk_sp<GrBuffer> fVertexBuffer;
    const bool      fReadSkFragCoord;
    const float     fY;

    // The program info (and both the GrPipeline and GrGeometryProcessor it relies on), when
    // allocated, are allocated in either the ddl-record-time or flush-time arena. It is the
    // arena's job to free up their memory so we just have a bare programInfo pointer here. We
    // don't even store the GrPipeline and GrGeometryProcessor pointers here bc they are
    // guaranteed to have the same lifetime as the program info.
    GrProgramInfo*  fProgramInfo = nullptr;

    friend class ::GrOp; // for ctor

    using INHERITED = GrDrawOp;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test.

namespace skiagm {

/**
 * This is a GPU-backend specific test. It ensures that SkSL properly identifies clockwise-winding
 * triangles (sk_Clockwise), in terms of to Skia device space, in all backends and with all render
 * target origins. We draw clockwise triangles green and counter-clockwise red.
 */
class ClockwiseGM : public GpuGM {
    SkString onShortName() override { return SkString("clockwise"); }
    SkISize onISize() override { return {300, 200}; }
    DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg) override;
};

DrawResult ClockwiseGM::onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) {
    auto sdc = SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);
    if (!sdc) {
        *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    sdc->clear(SK_PMColor4fBLACK);

    // Draw the test directly to the frame buffer.
    sdc->addDrawOp(ClockwiseTestOp::Make(rContext, false, 0));
    sdc->addDrawOp(ClockwiseTestOp::Make(rContext, true, 100));

    // Draw the test to an off-screen, top-down render target.
    GrColorType sdcColorType = sdc->colorInfo().colorType();
    if (auto topLeftSDC = skgpu::v1::SurfaceDrawContext::Make(
                rContext, sdcColorType, nullptr, SkBackingFit::kExact, {100, 200}, SkSurfaceProps(),
                1, GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin,
                SkBudgeted::kYes)) {
        topLeftSDC->clear(SK_PMColor4fTRANSPARENT);
        topLeftSDC->addDrawOp(ClockwiseTestOp::Make(rContext, false, 0));
        topLeftSDC->addDrawOp(ClockwiseTestOp::Make(rContext, true, 100));
        sdc->drawTexture(nullptr,
                         topLeftSDC->readSurfaceView(),
                         sdc->colorInfo().alphaType(),
                         GrSamplerState::Filter::kNearest,
                         GrSamplerState::MipmapMode::kNone,
                         SkBlendMode::kSrcOver,
                         SK_PMColor4fWHITE,
                         {0, 0, 100, 200},
                         {100, 0, 200, 200},
                         GrAA::kNo,
                         GrQuadAAFlags::kNone,
                         SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint,
                         SkMatrix::I(),
                         nullptr);
    }

    // Draw the test to an off-screen, bottom-up render target.
    if (auto topLeftSDC = skgpu::v1::SurfaceDrawContext::Make(
                rContext, sdcColorType, nullptr, SkBackingFit::kExact, {100, 200}, SkSurfaceProps(),
                1, GrMipmapped::kNo, GrProtected::kNo, kBottomLeft_GrSurfaceOrigin,
                SkBudgeted::kYes)) {
        topLeftSDC->clear(SK_PMColor4fTRANSPARENT);
        topLeftSDC->addDrawOp(ClockwiseTestOp::Make(rContext, false, 0));
        topLeftSDC->addDrawOp(ClockwiseTestOp::Make(rContext, true, 100));
        sdc->drawTexture(nullptr,
                         topLeftSDC->readSurfaceView(),
                         sdc->colorInfo().alphaType(),
                         GrSamplerState::Filter::kNearest,
                         GrSamplerState::MipmapMode::kNone,
                         SkBlendMode::kSrcOver,
                         SK_PMColor4fWHITE,
                         {0, 0, 100, 200},
                         {200, 0, 300, 200},
                         GrAA::kNo,
                         GrQuadAAFlags::kNone,
                         SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint,
                         SkMatrix::I(),
                         nullptr);
    }

    return DrawResult::kOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ClockwiseGM(); )

}  // namespace skiagm
