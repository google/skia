/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrColor.h"
#include "GrGeometryProcessor.h"
#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "SkMakeUnique.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

/**
 * This is a GPU-backend specific test for dynamic pipeline state. It draws boxes using dynamic
 * scissor rectangles then reads back the result to verify a successful test.
 */

using ScissorState = GrPipeline::ScissorState;

static constexpr int kScreenSize = 6;
static constexpr int kNumMeshes = 4;
static constexpr int kScreenSplitX = kScreenSize/2;
static constexpr int kScreenSplitY = kScreenSize/2;

static const GrPipeline::DynamicState kDynamicStates[kNumMeshes] = {
    {SkIRect::MakeLTRB(0,              0,              kScreenSplitX,  kScreenSplitY)},
    {SkIRect::MakeLTRB(0,              kScreenSplitY,  kScreenSplitX,  kScreenSize)},
    {SkIRect::MakeLTRB(kScreenSplitX,  0,              kScreenSize,    kScreenSplitY)},
    {SkIRect::MakeLTRB(kScreenSplitX,  kScreenSplitY,  kScreenSize,    kScreenSize)},
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
        : fVertex(this->addVertexAttrib("vertex", kVec2f_GrVertexAttribType))
        , fColor(this->addVertexAttrib("color", kVec4ub_GrVertexAttribType)) {
        this->initClassID<GrPipelineDynamicStateTestProcessor>();
    }

    const char* name() const override { return "GrPipelineDynamicStateTest Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

protected:
    const Attribute& fVertex;
    const Attribute& fColor;

    friend class GLSLPipelineDynamicStateTestProcessor;
    typedef GrGeometryProcessor INHERITED;
};

class GLSLPipelineDynamicStateTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrPipelineDynamicStateTestProcessor& mp =
            args.fGP.cast<GrPipelineDynamicStateTestProcessor>();

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        varyingHandler->emitAttributes(mp);
        varyingHandler->addPassThroughAttribute(&mp.fColor, args.fOutputColor);

        GrGLSLVertexBuilder* v = args.fVertBuilder;
        v->codeAppendf("vec2 vertex = %s;", mp.fVertex.fName);
        gpArgs->fPositionVar.set(kVec2f_GrSLType, "vertex");

        GrGLSLPPFragmentBuilder* f = args.fFragBuilder;
        f->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor*
GrPipelineDynamicStateTestProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLPipelineDynamicStateTestProcessor;
}

class GrPipelineDynamicStateTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    GrPipelineDynamicStateTestOp(ScissorState scissorState, sk_sp<const GrBuffer> vbuff)
        : INHERITED(ClassID())
        , fScissorState(scissorState)
        , fVertexBuffer(std::move(vbuff)) {
        this->setBounds(SkRect::MakeIWH(kScreenSize, kScreenSize),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

private:
    const char* name() const override { return "GrPipelineDynamicStateTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state) override {
        GrRenderTarget* rt = state->drawOpArgs().fRenderTarget;
        GrPipeline pipeline(rt, fScissorState, SkBlendMode::kSrc);
        SkSTArray<kNumMeshes, GrMesh> meshes;
        for (int i = 0; i < kNumMeshes; ++i) {
            GrMesh& mesh = meshes.emplace_back(GrPrimitiveType::kTriangleStrip);
            mesh.setNonIndexedNonInstanced(4);
            mesh.setVertexData(fVertexBuffer.get(), 4 * i);
        }
        state->commandBuffer()->draw(pipeline, GrPipelineDynamicStateTestProcessor(),
                                     meshes.begin(), kDynamicStates, 4,
                                     SkRect::MakeIWH(kScreenSize, kScreenSize));
    }

    ScissorState                fScissorState;
    const sk_sp<const GrBuffer> fVertexBuffer;

    typedef GrDrawOp INHERITED;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrPipelineDynamicStateTest, reporter, ctxInfo) {
    GrContext* const context = ctxInfo.grContext();
    GrResourceProvider* rp = context->resourceProvider();

    sk_sp<GrRenderTargetContext> rtc(
        context->makeDeferredRenderTargetContext(SkBackingFit::kExact, kScreenSize, kScreenSize,
                                                 kRGBA_8888_GrPixelConfig, nullptr));
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

    sk_sp<const GrBuffer> vbuff(rp->createBuffer(sizeof(vdata), kVertex_GrBufferType,
                                                 kDynamic_GrAccessPattern,
                                                 GrResourceProvider::kNoPendingIO_Flag |
                                                 GrResourceProvider::kRequireGpuMemory_Flag,
                                                 vdata));
    if (!vbuff) {
        ERRORF(reporter, "vbuff is null.");
        return;
    }

    uint32_t resultPx[kScreenSize * kScreenSize];

    for (ScissorState scissorState : {ScissorState::kEnabled, ScissorState::kDisabled}) {
        rtc->clear(nullptr, 0xbaaaaaad, true);
        rtc->priv().testingOnly_addDrawOp(
            skstd::make_unique<GrPipelineDynamicStateTestOp>(scissorState, vbuff));
        rtc->readPixels(SkImageInfo::Make(kScreenSize, kScreenSize,
                                          kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                        resultPx, 4 * kScreenSize, 0, 0, 0);
        for (int y = 0; y < kScreenSize; ++y) {
            for (int x = 0; x < kScreenSize; ++x) {
                int expectedColorIdx;
                if (ScissorState::kEnabled == scissorState) {
                    expectedColorIdx = (x < kScreenSplitX ? 0 : 2) + (y < kScreenSplitY ? 0 : 1);
                } else {
                    expectedColorIdx = kNumMeshes - 1;
                }
                uint32_t expected = kMeshColors[expectedColorIdx];
                uint32_t actual = resultPx[y * kScreenSize + x];
                if (expected != actual) {
                    ERRORF(reporter, "[scissor=%s] pixel (%i,%i): got 0x%x expected 0x%x",
                           ScissorState::kEnabled == scissorState ? "enabled" : "disabled", x, y,
                           actual, expected);
                    return;
                }
            }
        }
    }
}

#endif
