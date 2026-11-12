/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <utility>

using namespace skia_private;

class GrAppliedClip;
class GrCaps;
class GrDstProxyView;
class GrGLSLProgramDataManager;
class GrRecordingContext;
enum class GrXferBarrierFlags;
namespace skgpu { class KeyBuilder; }
struct GrContextOptions;
struct GrShaderCaps;

/**
 * This is a GPU-backend specific test for dynamic pipeline state. It draws boxes using dynamic
 * scissor rectangles then reads back the result to verify a successful test.
 */

static constexpr int kScreenSize = 6;
static constexpr int kNumMeshes = 4;
static constexpr int kScreenSplitX = kScreenSize/2;
static constexpr int kScreenSplitY = kScreenSize/2;

static const std::array<SkIRect, kNumMeshes> kDynamicScissors = {
        SkIRect::MakeLTRB(0,             0,             kScreenSplitX, kScreenSplitY),
        SkIRect::MakeLTRB(0,             kScreenSplitY, kScreenSplitX, kScreenSize),
        SkIRect::MakeLTRB(kScreenSplitX, 0,             kScreenSize,   kScreenSplitY),
        SkIRect::MakeLTRB(kScreenSplitX, kScreenSplitY, kScreenSize,   kScreenSize),
};

static const std::array<GrColor, kNumMeshes> kMeshColors = {
        GrColorPackRGBA(255, 0, 0, 255),
        GrColorPackRGBA(0, 255, 0, 255),
        GrColorPackRGBA(0, 0, 255, 255),
        GrColorPackRGBA(0, 0, 0, 255),
};

struct Vertex {
    float     fX;
    float     fY;
    GrColor   fColor;
};

namespace {
class PipelineDynamicStateTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena) {
        return arena->make(
                [&](void* ptr) { return new (ptr) PipelineDynamicStateTestProcessor(); });
    }

    const char* name() const override { return "GrPipelineDynamicStateTest Processor"; }

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const final {}

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

private:
    PipelineDynamicStateTestProcessor() : INHERITED(kGrPipelineDynamicStateTestProcessor_ClassID) {
        this->setVertexAttributesWithImplicitOffsets(kAttributes, std::size(kAttributes));
    }

    const Attribute& inVertex() const { return kAttributes[0]; }
    const Attribute& inColor() const { return kAttributes[1]; }

    inline static constexpr Attribute kAttributes[] = {
            {"vertex", kFloat2_GrVertexAttribType, SkSLType::kHalf2},
            {"color", kUByte4_norm_GrVertexAttribType, SkSLType::kHalf4},
    };

    friend class GLSLPipelineDynamicStateTestProcessor;
    using INHERITED = GrGeometryProcessor;
};
}  // anonymous namespace

std::unique_ptr<GrGeometryProcessor::ProgramImpl>
PipelineDynamicStateTestProcessor::makeProgramImpl(const GrShaderCaps&) const {
    class Impl : public GrGeometryProcessor::ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager&,
                     const GrShaderCaps&,
                     const GrGeometryProcessor&) final {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
            const PipelineDynamicStateTestProcessor& mp =
                    args.fGeomProc.cast<PipelineDynamicStateTestProcessor>();
            GrGLSLVertexBuilder* v = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            varyingHandler->emitAttributes(mp);
            f->codeAppendf("half4 %s;", args.fOutputColor);
            varyingHandler->addPassThroughAttribute(mp.inColor().asShaderVar(), args.fOutputColor);

            v->codeAppendf("float2 vertex = %s;", mp.inVertex().name());
            gpArgs->fPositionVar.set(SkSLType::kFloat2, "vertex");
            f->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
        }
    };
    return std::make_unique<Impl>();
}

namespace {
class GrPipelineDynamicStateTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrScissorTest scissorTest,
                            sk_sp<const GrBuffer> vbuff) {
        return GrOp::Make<GrPipelineDynamicStateTestOp>(context, scissorTest, std::move(vbuff));
    }

private:
    friend class GrOp;

    GrPipelineDynamicStateTestOp(GrScissorTest scissorTest, sk_sp<const GrBuffer> vbuff)
        : INHERITED(ClassID())
        , fScissorTest(scissorTest)
        , fVertexBuffer(std::move(vbuff)) {
        this->setBounds(SkRect::MakeIWH(kScreenSize, kScreenSize),
                        HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrPipelineDynamicStateTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrDstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        GrPipeline pipeline(fScissorTest, SkBlendMode::kSrc,
                            flushState->drawOpArgs().writeView().swizzle());
        STArray<kNumMeshes, GrSimpleMesh> meshes;
        for (int i = 0; i < kNumMeshes; ++i) {
            GrSimpleMesh& mesh = meshes.push_back();
            mesh.set(fVertexBuffer, 4, 4 * i);
        }

        auto geomProc = PipelineDynamicStateTestProcessor::Make(flushState->allocator());

        GrProgramInfo programInfo(flushState->caps(),
                                  flushState->writeView(),
                                  flushState->usesMSAASurface(),
                                  &pipeline,
                                  &GrUserStencilSettings::kUnused,
                                  geomProc,
                                  GrPrimitiveType::kTriangleStrip,
                                  flushState->renderPassBarriers(),
                                  flushState->colorLoadOp());

        flushState->bindPipeline(programInfo, SkRect::MakeIWH(kScreenSize, kScreenSize));
        for (int i = 0; i < 4; ++i) {
            if (fScissorTest == GrScissorTest::kEnabled) {
                flushState->setScissorRect(kDynamicScissors[i]);
            }
            flushState->drawMesh(meshes[i]);
        }
    }

    GrScissorTest               fScissorTest;
    const sk_sp<const GrBuffer> fVertexBuffer;

    using INHERITED = GrDrawOp;
};
}  // anonymous namespace

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrPipelineDynamicStateTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    GrResourceProvider* rp = dContext->priv().resourceProvider();

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kExact,
                                                       {kScreenSize, kScreenSize},
                                                       SkSurfaceProps(),
                                                       /*label=*/{});
    if (!sdc) {
        ERRORF(reporter, "could not create render target context.");
        return;
    }

    constexpr float d = (float) kScreenSize;
    Vertex vdata[kNumMeshes * 4] = {
        {0, 0, kMeshColors[0]},
        {0, d, kMeshColors[0]},
        {d, 0, kMeshColors[0]},
        {d, d, kMeshColors[0]},

        {0, 0, kMeshColors[1]},
        {0, d, kMeshColors[1]},
        {d, 0, kMeshColors[1]},
        {d, d, kMeshColors[1]},

        {0, 0, kMeshColors[2]},
        {0, d, kMeshColors[2]},
        {d, 0, kMeshColors[2]},
        {d, d, kMeshColors[2]},

        {0, 0, kMeshColors[3]},
        {0, d, kMeshColors[3]},
        {d, 0, kMeshColors[3]},
        {d, d, kMeshColors[3]}
    };

    sk_sp<const GrBuffer> vbuff(rp->createBuffer(vdata,
                                                 sizeof(vdata),
                                                 GrGpuBufferType::kVertex,
                                                 kDynamic_GrAccessPattern));
    if (!vbuff) {
        ERRORF(reporter, "vbuff is null.");
        return;
    }

    uint32_t resultPx[kScreenSize * kScreenSize];

    for (GrScissorTest scissorTest : {GrScissorTest::kEnabled, GrScissorTest::kDisabled}) {
        sdc->clear(SkPMColor4f::FromBytes_RGBA(0xbaaaaaad));
        sdc->addDrawOp(GrPipelineDynamicStateTestOp::Make(dContext, scissorTest, vbuff));
        auto ii = SkImageInfo::Make(kScreenSize, kScreenSize,
                                    kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        GrPixmap resultPM(ii, resultPx, kScreenSize*sizeof(uint32_t));
        sdc->readPixels(dContext, resultPM, {0, 0});
        for (int y = 0; y < kScreenSize; ++y) {
            for (int x = 0; x < kScreenSize; ++x) {
                int expectedColorIdx;
                if (GrScissorTest::kEnabled == scissorTest) {
                    expectedColorIdx = (x < kScreenSplitX ? 0 : 2) + (y < kScreenSplitY ? 0 : 1);
                } else {
                    expectedColorIdx = kNumMeshes - 1;
                }
                uint32_t expected = kMeshColors[expectedColorIdx];
                uint32_t actual = resultPx[y * kScreenSize + x];
                if (expected != actual) {
                    ERRORF(reporter, "[scissor=%s] pixel (%i,%i): got 0x%x expected 0x%x",
                           GrScissorTest::kEnabled == scissorTest ? "enabled" : "disabled", x, y,
                           actual, expected);
                    return;
                }
            }
        }
    }
}

#if !defined(SK_LEGACY_SCISSOR)
namespace {
class GrPipelineDrawBoundsTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrScissorTest scissorTest,
                            const SkRect& drawBounds,
                            const SkIRect& scissorRect,
                            sk_sp<const GrBuffer> vbuff) {
        return GrOp::Make<GrPipelineDrawBoundsTestOp>(
                context, scissorTest, drawBounds, scissorRect, std::move(vbuff));
    }

private:
    friend class GrOp;

    GrPipelineDrawBoundsTestOp(GrScissorTest scissorTest,
                               const SkRect& drawBounds,
                               const SkIRect& scissorRect,
                               sk_sp<const GrBuffer> vbuff)
            : INHERITED(ClassID())
            , fScissorTest(scissorTest)
            , fDrawBounds(drawBounds)
            , fScissorRect(scissorRect)
            , fVertexBuffer(std::move(vbuff)) {
        // Keep the op's outer bounds covering the full logical target so SurfaceDrawContext does
        // not cull out-of-bounds test cases before onBindPipeline sees fDrawBounds.
        this->setBounds(
                SkRect::MakeIWH(kScreenSize, kScreenSize), HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrPipelineDrawBoundsTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView&,
                      GrAppliedClip*,
                      const GrDstProxyView&,
                      GrXferBarrierFlags,
                      GrLoadOp) override {}
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect&) override {
        GrPipeline pipeline(
                fScissorTest, SkBlendMode::kSrc, flushState->drawOpArgs().writeView().swizzle());
        GrSimpleMesh mesh;
        // Draw a fullscreen triangle strip whose geometry overshoots fDrawBounds.
        mesh.set(fVertexBuffer, 4, 0);

        auto geomProc = PipelineDynamicStateTestProcessor::Make(flushState->allocator());
        GrProgramInfo programInfo(flushState->caps(),
                                  flushState->writeView(),
                                  flushState->usesMSAASurface(),
                                  &pipeline,
                                  &GrUserStencilSettings::kUnused,
                                  geomProc,
                                  GrPrimitiveType::kTriangleStrip,
                                  flushState->renderPassBarriers(),
                                  flushState->colorLoadOp());

        flushState->bindPipeline(programInfo, fDrawBounds);
        if (fScissorTest == GrScissorTest::kEnabled) {
            flushState->setScissorRect(fScissorRect);
        }
        flushState->drawMesh(mesh);
    }

    GrScissorTest fScissorTest;
    SkRect fDrawBounds;
    SkIRect fScissorRect;
    const sk_sp<const GrBuffer> fVertexBuffer;

    using INHERITED = GrDrawOp;
};
}  // anonymous namespace

// Verifies that all Ganesh backends restrict rasterization to the intersection of the render pass
// content bounds, bindPipeline's drawBounds, and (when enabled) setScissorRect's scissor.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrPipelineDrawBoundsTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    GrResourceProvider* rp = dContext->priv().resourceProvider();

    constexpr float d = static_cast<float>(kScreenSize);
    const GrColor kClearColor = GrColorPackRGBA(0xba, 0xaa, 0xaa, 0xad);
    const GrColor kDrawColor = kMeshColors[0];
    const Vertex vdata[4] = {
            {0, 0, kDrawColor},
            {0, d, kDrawColor},
            {d, 0, kDrawColor},
            {d, d, kDrawColor},
    };

    sk_sp<const GrBuffer> vbuff(rp->createBuffer(
            vdata, sizeof(vdata), GrGpuBufferType::kVertex, kDynamic_GrAccessPattern));
    if (!vbuff) {
        ERRORF(reporter, "vbuff is null.");
        return;
    }

    struct Case {
        const char* fLabel;
        GrScissorTest fScissorTest;
        SkRect fDrawBounds;
        SkIRect fScissorRect;
        SkIRect fExpectedPainted;
    };

    static const Case kCases[] = {
            // Unscissored pipeline with fractional sub-RT drawBounds -> rounds out to [1, 2, 5, 5].
            {"unscissored_fractional_subrect",
             GrScissorTest::kDisabled,
             SkRect::MakeLTRB(1.2f, 2.1f, 4.8f, 5.0f),
             SkIRect::MakeEmpty(),
             SkIRect::MakeLTRB(1, 2, 5, 5)},
            // Scissored pipeline with a loose scissor covering the full RT (e.g. kApprox saveLayer
            // or loose ClipStack outerBounds) -> must still clamp to drawBounds [1, 2, 5, 5].
            {"scissored_loose_full_rt",
             GrScissorTest::kEnabled,
             SkRect::MakeLTRB(1.2f, 2.1f, 4.8f, 5.0f),
             SkIRect::MakeWH(kScreenSize, kScreenSize),
             SkIRect::MakeLTRB(1, 2, 5, 5)},
            // Scissored pipeline with partially overlapping scissor -> intersection [3, 2, 5, 4].
            {"scissored_partial_overlap",
             GrScissorTest::kEnabled,
             SkRect::MakeLTRB(1.0f, 1.0f, 5.0f, 4.0f),
             SkIRect::MakeLTRB(3, 2, kScreenSize, kScreenSize),
             SkIRect::MakeLTRB(3, 2, 5, 4)},
            // Scissored pipeline whose scissor is disjoint from drawBounds -> draws nothing.
            {"scissored_disjoint_from_draw_bounds",
             GrScissorTest::kEnabled,
             SkRect::MakeLTRB(1.0f, 1.0f, 3.0f, 3.0f),
             SkIRect::MakeLTRB(4, 4, kScreenSize, kScreenSize),
             SkIRect::MakeEmpty()},
            // Out-of-bounds drawBounds (unscissored and scissored) -> draws nothing.
            {"unscissored_oob_positive",
             GrScissorTest::kDisabled,
             SkRect::MakeLTRB(10.0f, 10.0f, 12.0f, 12.0f),
             SkIRect::MakeEmpty(),
             SkIRect::MakeEmpty()},
            {"scissored_oob_negative",
             GrScissorTest::kEnabled,
             SkRect::MakeLTRB(-4.0f, -4.0f, -1.0f, -1.0f),
             SkIRect::MakeWH(kScreenSize, kScreenSize),
             SkIRect::MakeEmpty()},
    };

    const auto ii = SkImageInfo::Make(
            kScreenSize, kScreenSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    uint32_t resultPx[kScreenSize * kScreenSize];

    for (SkBackingFit fit : {SkBackingFit::kExact, SkBackingFit::kApprox}) {
        for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
            auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                               GrColorType::kRGBA_8888,
                                                               nullptr,
                                                               fit,
                                                               {kScreenSize, kScreenSize},
                                                               SkSurfaceProps(),
                                                               /*label=*/{},
                                                               /*sampleCnt=*/1,
                                                               skgpu::Mipmapped::kNo,
                                                               skgpu::Protected::kNo,
                                                               origin);
            if (!sdc) {
                ERRORF(reporter, "could not create render target context.");
                return;
            }

            for (const Case& c : kCases) {
                sdc->clear(SkPMColor4f::FromBytes_RGBA(kClearColor));
                sdc->addDrawOp(GrPipelineDrawBoundsTestOp::Make(
                        dContext, c.fScissorTest, c.fDrawBounds, c.fScissorRect, vbuff));
                GrPixmap resultPM(ii, resultPx, kScreenSize * sizeof(uint32_t));
                if (!sdc->readPixels(dContext, resultPM, {0, 0})) {
                    ERRORF(reporter, "readPixels failed for case %s", c.fLabel);
                    return;
                }

                for (int y = 0; y < kScreenSize; ++y) {
                    for (int x = 0; x < kScreenSize; ++x) {
                        uint32_t expected =
                                c.fExpectedPainted.contains(x, y) ? kDrawColor : kClearColor;
                        uint32_t actual = resultPx[y * kScreenSize + x];
                        if (expected != actual) {
                            ERRORF(reporter,
                                   "[%s fit=%s origin=%s] pixel (%d,%d): got 0x%08x expected 0x%08x",
                                   c.fLabel,
                                   fit == SkBackingFit::kExact ? "exact" : "approx",
                                   origin == kTopLeft_GrSurfaceOrigin ? "TopLeft" : "BottomLeft",
                                   x,
                                   y,
                                   actual,
                                   expected);
                            return;
                        }
                    }
                }
            }
        }
    }
}

static void suppress_inplace_dst_reads(GrContextOptions* options) {
#if defined(GPU_TEST_UTILS)
    options->fSuppressAdvancedBlendEquations = true;
    options->fSuppressFramebufferFetch = true;
#else
    (void)options;
#endif
}

// End-to-end regression test: an acute conic hairline whose bloated miter geometry overshoots
// opBounds, drawn with a dst-reading blend mode that triggers a partial kApprox dst copy. Pixels
// outside the curve's conservative op bounds must not be overwritten with uninitialized/stale
// scratch-texture texels, whether drawn directly, inside a kApprox saveLayer, or inside a loose
// clipRect.
DEF_GANESH_TEST_FOR_CONTEXTS(GrDrawBoundsHairlineDstCopyTest,
                             &skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             suppress_inplace_dst_reads,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    constexpr int kSize = 96;
    const SkImageInfo ii =
            SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Prime the scratch-texture pool with non-green sentinel pixels so any out-of-bounds sample
    // from an SkBackingFit::kApprox dst-copy texture produces visibly wrong colors if rasterized.
    {
        sk_sp<SkSurface> scratchPrimer =
                SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, ii);
        if (scratchPrimer) {
            scratchPrimer->getCanvas()->clear(SK_ColorMAGENTA);
            dContext->flushAndSubmit(scratchPrimer.get());
        }
    }

    // Control points lie inside [10, 10, 28, 11], so opBounds + 1px dst-copy padding stays well
    // inside [0, 0, 32, 24], while bloat_quad's acute miter vertex shoots past x = 60.
    const SkPath sharpConic =
            SkPathBuilder().moveTo(10.0f, 10.0f).conicTo(28.0f, 10.3f, 10.0f, 10.6f, 8.0f).detach();

    SkPaint hairlinePaint;
    hairlinePaint.setAntiAlias(true);
    hairlinePaint.setStyle(SkPaint::kStroke_Style);
    hairlinePaint.setStrokeWidth(0);
    hairlinePaint.setColor(SK_ColorBLUE);
    hairlinePaint.setBlendMode(SkBlendMode::kColorBurn);

    enum class DrawMode { kDirect, kSaveLayer, kLooseClip };

    SkBitmap readback;
    readback.allocPixels(ii);

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, ii, 0, origin, nullptr);
        if (!surface) {
            ERRORF(reporter, "Failed to create surface.");
            return;
        }

        for (DrawMode mode : {DrawMode::kDirect, DrawMode::kSaveLayer, DrawMode::kLooseClip}) {
            SkCanvas* canvas = surface->getCanvas();
            canvas->clear(SK_ColorGREEN);

            if (mode == DrawMode::kSaveLayer) {
                SkRect layerBounds = SkRect::MakeWH(80, 80);
                canvas->saveLayer(&layerBounds, nullptr);
                canvas->drawPath(sharpConic, hairlinePaint);
                canvas->restore();
            } else if (mode == DrawMode::kLooseClip) {
                canvas->save();
                canvas->clipRect(SkRect::MakeWH(80, 80));
                canvas->drawPath(sharpConic, hairlinePaint);
                canvas->restore();
            } else {
                canvas->drawPath(sharpConic, hairlinePaint);
            }

            if (!surface->readPixels(readback, 0, 0)) {
                ERRORF(reporter, "readPixels failed.");
                return;
            }

            // Every pixel outside the conservative bounds [8, 8, 31, 14] must remain untouched
            // solid green.
            const SkIRect allowedBounds = SkIRect::MakeLTRB(8, 8, 31, 14);
            for (int y = 0; y < kSize; ++y) {
                for (int x = 0; x < kSize; ++x) {
                    if (allowedBounds.contains(x, y)) {
                        continue;
                    }
                    SkColor actual = readback.getColor(x, y);
                    if (actual != SK_ColorGREEN) {
                        const char* modeName = mode == DrawMode::kDirect      ? "direct"
                                               : mode == DrawMode::kSaveLayer ? "saveLayer"
                                                                              : "looseClip";
                        ERRORF(reporter,
                               "[mode=%s origin=%s] out-of-bounds pixel (%d,%d) corrupted: "
                               "got 0x%08x expected 0x%08x",
                               modeName,
                               origin == kTopLeft_GrSurfaceOrigin ? "TopLeft" : "BottomLeft",
                               x,
                               y,
                               actual,
                               SK_ColorGREEN);
                        return;
                    }
                }
            }
        }
    }
}
#endif  // !defined(SK_LEGACY_SCISSOR)
