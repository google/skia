/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

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

class GrPipelineDynamicStateTestProcessor : public GrGeometryProcessor {
public:
    GrPipelineDynamicStateTestProcessor()
            : INHERITED(kGrPipelineDynamicStateTestProcessor_ClassID) {
        this->setVertexAttributes(kAttributes, SK_ARRAY_COUNT(kAttributes));
    }

    const char* name() const override { return "GrPipelineDynamicStateTest Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    const Attribute& inVertex() const { return kAttributes[0]; }
    const Attribute& inColor() const { return kAttributes[1]; }

private:
    static constexpr Attribute kAttributes[] = {
        {"vertex", kFloat2_GrVertexAttribType, kHalf2_GrSLType},
        {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType},
    };

    friend class GLSLPipelineDynamicStateTestProcessor;
    typedef GrGeometryProcessor INHERITED;
};
constexpr GrPrimitiveProcessor::Attribute GrPipelineDynamicStateTestProcessor::kAttributes[];

class GLSLPipelineDynamicStateTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrPipelineDynamicStateTestProcessor& mp =
            args.fGP.cast<GrPipelineDynamicStateTestProcessor>();

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        varyingHandler->emitAttributes(mp);
        varyingHandler->addPassThroughAttribute(mp.inColor(), args.fOutputColor);

        GrGLSLVertexBuilder* v = args.fVertBuilder;
        v->codeAppendf("float2 vertex = %s;", mp.inVertex().name());
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");

        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
        f->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor*
GrPipelineDynamicStateTestProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPipelineDynamicStateTestProcessor;
}

class GrPipelineDynamicStateTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrScissorTest scissorTest,
                                          sk_sp<const GrBuffer> vbuff) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<GrPipelineDynamicStateTestOp>(scissorTest, std::move(vbuff));
    }

private:
    friend class GrOpMemoryPool;

    GrPipelineDynamicStateTestOp(GrScissorTest scissorTest, sk_sp<const GrBuffer> vbuff)
        : INHERITED(ClassID())
        , fScissorTest(scissorTest)
        , fVertexBuffer(std::move(vbuff)) {
        this->setBounds(SkRect::MakeIWH(kScreenSize, kScreenSize),
                        HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrPipelineDynamicStateTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        GrPipeline pipeline(fScissorTest, SkBlendMode::kSrc,
                            flushState->drawOpArgs().outputSwizzle());
        SkSTArray<kNumMeshes, GrMesh> meshes;
        for (int i = 0; i < kNumMeshes; ++i) {
            GrMesh& mesh = meshes.emplace_back(GrPrimitiveType::kTriangleStrip);
            mesh.setNonIndexedNonInstanced(4);
            mesh.setVertexData(fVertexBuffer, 4 * i);
        }
        GrPipeline::DynamicStateArrays dynamicState;
        dynamicState.fScissorRects = kDynamicScissors;

        GrPipelineDynamicStateTestProcessor primProc;

        GrProgramInfo programInfo(flushState->drawOpArgs().numSamples(),
                                  flushState->drawOpArgs().origin(),
                                  pipeline,
                                  primProc,
                                  nullptr,
                                  &dynamicState);

        flushState->opsRenderPass()->draw(programInfo, meshes.begin(), 4,
                                          SkRect::MakeIWH(kScreenSize, kScreenSize));
    }

    GrScissorTest               fScissorTest;
    const sk_sp<const GrBuffer> fVertexBuffer;

    typedef GrDrawOp INHERITED;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrPipelineDynamicStateTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrResourceProvider* rp = context->priv().resourceProvider();

    auto rtc = context->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kScreenSize, kScreenSize, GrColorType::kRGBA_8888, nullptr);
    if (!rtc) {
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
        rtc->clear(nullptr, SkPMColor4f::FromBytes_RGBA(0xbaaaaaad),
                   GrRenderTargetContext::CanClearFullscreen::kYes);
        rtc->priv().testingOnly_addDrawOp(
            GrPipelineDynamicStateTestOp::Make(context, scissorTest, vbuff));
        rtc->readPixels(SkImageInfo::Make(kScreenSize, kScreenSize,
                                          kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                        resultPx, 4 * kScreenSize, {0, 0});
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
