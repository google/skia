/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"
#include "Test.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "SkPointPriv.h"
#include "SkString.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "ops/GrMeshDrawOp.h"

namespace {
class Op : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "Dummy Op"; }

    static std::unique_ptr<GrDrawOp> Make(GrContext* context, int numAttribs) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<Op>(numAttribs);
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kNone;
    }

    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

private:
    friend class ::GrOpMemoryPool;

    Op(int numAttribs) : INHERITED(ClassID()), fNumAttribs(numAttribs) {
        this->setBounds(SkRect::MakeWH(1.f, 1.f), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override {
        class GP : public GrGeometryProcessor {
        public:
            GP(int numAttribs) : INHERITED(kGP_ClassID), fNumAttribs(numAttribs) {
                SkASSERT(numAttribs > 1);
                fAttribNames.reset(new SkString[numAttribs]);
                fAttributes.reset(new Attribute[numAttribs]);
                for (auto i = 0; i < numAttribs; ++i) {
                    fAttribNames[i].printf("attr%d", i);
                    fAttributes[i] = {fAttribNames[i].c_str(), kFloat2_GrVertexAttribType,
                                                               kFloat2_GrSLType};
                }
                this->setVertexAttributes(fAttributes.get(), numAttribs);
            }
            const char* name() const override { return "Dummy GP"; }

            GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
                class GLSLGP : public GrGLSLGeometryProcessor {
                public:
                    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                        const GP& gp = args.fGP.cast<GP>();
                        args.fVaryingHandler->emitAttributes(gp);
                        this->writeOutputPosition(args.fVertBuilder, gpArgs,
                                                  gp.fAttributes[0].name());
                        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                        fragBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
                        fragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                    }
                    void setData(const GrGLSLProgramDataManager& pdman,
                                 const GrPrimitiveProcessor& primProc,
                                 FPCoordTransformIter&&) override {}
                };
                return new GLSLGP();
            }
            void getGLSLProcessorKey(const GrShaderCaps&,
                                     GrProcessorKeyBuilder* builder) const override {
                builder->add32(fNumAttribs);
            }

        private:
            int fNumAttribs;
            std::unique_ptr<SkString[]> fAttribNames;
            std::unique_ptr<Attribute[]> fAttributes;

            typedef GrGeometryProcessor INHERITED;
        };
        sk_sp<GrGeometryProcessor> gp(new GP(fNumAttribs));
        size_t vertexStride = gp->vertexStride();
        QuadHelper helper(target, vertexStride, 1);
        SkPoint* vertices = reinterpret_cast<SkPoint*>(helper.vertices());
        SkPointPriv::SetRectTriStrip(vertices, 0.f, 0.f, 1.f, 1.f, vertexStride);
        helper.recordDraw(target, std::move(gp));
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                this, chainBounds, GrProcessorSet::MakeEmptySet());
    }

    int fNumAttribs;

    typedef GrMeshDrawOp INHERITED;
};
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(VertexAttributeCount, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
#if GR_GPU_STATS
    GrGpu* gpu = context->priv().getGpu();
#endif

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    sk_sp<GrRenderTargetContext> renderTargetContext(
            context->priv().makeDeferredRenderTargetContext(format, SkBackingFit::kApprox,
                                                            1, 1, kRGBA_8888_GrPixelConfig,
                                                            nullptr));
    if (!renderTargetContext) {
        ERRORF(reporter, "Could not create render target context.");
        return;
    }
    int attribCnt = context->priv().caps()->maxVertexAttributes();
    if (!attribCnt) {
        ERRORF(reporter, "No attributes allowed?!");
        return;
    }
    context->flush();
    context->priv().resetGpuStats();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    // Adding discard to appease vulkan validation warning about loading uninitialized data on draw
    renderTargetContext->discard();

    GrPaint grPaint;
    // This one should succeed.
    renderTargetContext->priv().testingOnly_addDrawOp(Op::Make(context, attribCnt));
    context->flush();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 1);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    context->priv().resetGpuStats();
    renderTargetContext->priv().testingOnly_addDrawOp(Op::Make(context, attribCnt + 1));
    context->flush();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 1);
#endif
}
