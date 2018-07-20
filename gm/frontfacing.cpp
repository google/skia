/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkTypes.h"

#include <array>
#include <vector>
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGeometryProcessor.h"
#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "SkBitmap.h"
#include "SkMakeUnique.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

namespace skiagm {

////////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr GrGeometryProcessor::Attribute gVertex{"vertex", kFloat2_GrVertexAttribType};

/**
 * This is a GPU-backend specific test. Blah.
 */
class FrontFacingGM : public GM {
private:
    SkString onShortName() final { return SkString("frontfacing"); }
    SkISize onISize() override { return SkISize::Make(100, 100); }
    void onDraw(SkCanvas*) override;
};

class FrontFacingTestProcessor : public GrGeometryProcessor {
public:
    FrontFacingTestProcessor() : GrGeometryProcessor(kFrontFacingTestProcessor_ClassID) {
        this->setVertexAttributeCnt(1);
    }
    const char* name() const override { return "FrontFacingTestProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const Attribute& onVertexAttribute(int i) const override { return gVertex; }

    friend class GLSLFrontFacingTestProcessor;
};

class GLSLFrontFacingTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) override {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const FrontFacingTestProcessor& proc = args.fGP.cast<FrontFacingTestProcessor>();
        args.fVaryingHandler->emitAttributes(proc);
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");
        args.fFragBuilder->codeAppendf(
                "%s = sk_FrontFacing ? half4(0,1,0,1) : half4(1,0,0,1);", args.fOutputColor);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor* FrontFacingTestProcessor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new GLSLFrontFacingTestProcessor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class FrontFacingTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
        return pool->allocate<FrontFacingTestOp>();
    }

private:
    FrontFacingTestOp() : GrDrawOp(ClassID()) {
        this->setBounds(SkRect::MakeIWH(100, 100), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "FrontFacingTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState) override {
        static constexpr SkPoint vertices[4] = {
            {100, 0},
            {0, 0},
            {0, 100},
            {100, 100},
        };
        GrPipeline pipeline(flushState->drawOpArgs().fProxy, GrPipeline::ScissorState::kDisabled,
                            SkBlendMode::kPlus);
        GrMesh mesh(GrPrimitiveType::kTriangleStrip);
        mesh.setNonIndexedNonInstanced(4);
        mesh.setVertexData(flushState->resourceProvider()->createBuffer(
                sizeof(vertices), kVertex_GrBufferType, kStatic_GrAccessPattern,
                GrResourceProvider::kNone_Flag, vertices));
        flushState->rtCommandBuffer()->draw(FrontFacingTestProcessor(), pipeline, nullptr,
                                            nullptr, &mesh, 1, SkRect::MakeIWH(100, 100));
    }

    friend class ::GrOpMemoryPool; // for ctor
};

void FrontFacingGM::onDraw(SkCanvas* canvas) {
    GrContext* ctx = canvas->getGrContext();
    GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
    if (!ctx || !rtc) {
        DrawGpuOnlyMessage(canvas);
        return;
    }

    rtc->clear(nullptr, GrColorPackRGBA(0,0,0,255),
               GrRenderTargetContext::CanClearFullscreen::kYes);
    rtc->priv().testingOnly_addDrawOp(FrontFacingTestOp::Make(ctx));
}


DEF_GM( return new FrontFacingGM(); )

}
