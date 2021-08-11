/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

/**
 * This is a GPU-backend specific test for dynamic pipeline state. It draws boxes using dynamic
 * scissor rectangles then reads back the result to verify a successful test.
 */

static constexpr int kScreenSize = 6;
static constexpr int kNumMeshes = 4;
static constexpr int kScreenSplitX = kScreenSize/2;
static constexpr int kScreenSplitY = kScreenSize/2;

static const SkIRect kDynamicScissors[kNumMeshes] = {
    SkIRect::MakeLTRB(0,              0,              kScreenSplitX,  kScreenSplitY),
    SkIRect::MakeLTRB(0,              kScreenSplitY,  kScreenSplitX,  kScreenSize),
    SkIRect::MakeLTRB(kScreenSplitX,  0,              kScreenSize,    kScreenSplitY),
    SkIRect::MakeLTRB(kScreenSplitX,  kScreenSplitY,  kScreenSize,    kScreenSize),
};

static const GrColor kMeshColors[kNumMeshes] {
    GrColorPackRGBA(255, 0, 0, 255),
    GrColorPackRGBA(0, 255, 0, 255),
    GrColorPackRGBA(0, 0, 255, 255),
    GrColorPackRGBA(0, 0, 0, 255)
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

    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

private:
    PipelineDynamicStateTestProcessor() : INHERITED(kGrPipelineDynamicStateTestProcessor_ClassID) {
        this->setVertexAttributes(kAttributes, SK_ARRAY_COUNT(kAttributes));
    }

    const Attribute& inVertex() const { return kAttributes[0]; }
    const Attribute& inColor() const { return kAttributes[1]; }

    static constexpr Attribute kAttributes[] = {
            {"vertex", kFloat2_GrVertexAttribType, kHalf2_GrSLType},
            {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType},
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
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");
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
        SkSTArray<kNumMeshes, GrSimpleMesh> meshes;
        for (int i = 0; i < kNumMeshes; ++i) {
            GrSimpleMesh& mesh = meshes.push_back();
            mesh.set(fVertexBuffer, 4, 4 * i);
        }

        auto geomProc = PipelineDynamicStateTestProcessor::Make(flushState->allocator());

        GrProgramInfo programInfo(flushState->writeView(),
                                  &pipeline,
                                  &GrUserStencilSettings::kUnused,
                                  geomProc,
                                  GrPrimitiveType::kTriangleStrip, 0,
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrPipelineDynamicStateTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    GrResourceProvider* rp = dContext->priv().resourceProvider();

    auto sdc = skgpu::v1::SurfaceDrawContext::Make(
            dContext, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
            {kScreenSize, kScreenSize}, SkSurfaceProps());
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

    sk_sp<const GrBuffer> vbuff(rp->createBuffer(sizeof(vdata), GrGpuBufferType::kVertex,
                                                 kDynamic_GrAccessPattern, vdata));
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
