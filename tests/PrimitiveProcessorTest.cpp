/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include <memory>

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/core/SkString.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {
class Op : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "Dummy Op"; }

    static GrOp::Owner Make(GrRecordingContext* rContext, int numAttribs) {
        return GrOp::Make<Op>(rContext, numAttribs);
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kNone;
    }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

private:
    friend class ::GrOp;

    Op(int numAttribs) : INHERITED(ClassID()), fNumAttribs(numAttribs) {
        this->setBounds(SkRect::MakeWH(1.f, 1.f), HasAABloat::kNo, IsHairline::kNo);
    }

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             GrAppliedClip&& appliedClip,
                             const GrXferProcessor::DstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        class GP : public GrGeometryProcessor {
        public:
            static GrGeometryProcessor* Make(SkArenaAlloc* arena, int numAttribs) {
                return arena->make([&](void* ptr) {
                    return new (ptr) GP(numAttribs);
                });
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
                        fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputColor);
                        fragBuilder->codeAppendf("const half4 %s = half4(1);",
                                                 args.fOutputCoverage);
                    }
                    void setData(const GrGLSLProgramDataManager& pdman,
                                 const GrPrimitiveProcessor& primProc) override {}
                };
                return new GLSLGP();
            }
            void getGLSLProcessorKey(const GrShaderCaps&,
                                     GrProcessorKeyBuilder* builder) const override {
                builder->add32(fNumAttribs);
            }

        private:
            GP(int numAttribs) : INHERITED(kGP_ClassID), fNumAttribs(numAttribs) {
                SkASSERT(numAttribs > 1);
                fAttribNames = std::make_unique<SkString[]>(numAttribs);
                fAttributes = std::make_unique<Attribute[]>(numAttribs);
                for (auto i = 0; i < numAttribs; ++i) {
                    fAttribNames[i].printf("attr%d", i);
                    // This gives us more of a mix of attribute types, and allows the
                    // component count to fit within the limits for iOS Metal.
                    if (i & 0x1) {
                        fAttributes[i] = {fAttribNames[i].c_str(), kFloat_GrVertexAttribType,
                                                                   kFloat_GrSLType};
                    } else {
                        fAttributes[i] = {fAttribNames[i].c_str(), kFloat2_GrVertexAttribType,
                                                                   kFloat2_GrSLType};
                    }
                }
                this->setVertexAttributes(fAttributes.get(), numAttribs);
            }

            int fNumAttribs;
            std::unique_ptr<SkString[]> fAttribNames;
            std::unique_ptr<Attribute[]> fAttributes;

            using INHERITED = GrGeometryProcessor;
        };

        GrGeometryProcessor* gp = GP::Make(arena, fNumAttribs);

        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps,
                                                                   arena,
                                                                   writeView,
                                                                   std::move(appliedClip),
                                                                   dstProxyView,
                                                                   gp,
                                                                   GrProcessorSet::MakeEmptySet(),
                                                                   GrPrimitiveType::kTriangles,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   GrPipeline::InputFlags::kNone);
    }

    void onPrepareDraws(Target* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
        }

        size_t vertexStride = fProgramInfo->primProc().vertexStride();
        QuadHelper helper(target, vertexStride, 1);
        SkPoint* vertices = reinterpret_cast<SkPoint*>(helper.vertices());
        SkPointPriv::SetRectTriStrip(vertices, 0.f, 0.f, 1.f, 1.f, vertexStride);
        fMesh = helper.mesh();
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    int            fNumAttribs;
    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};
}  // namespace

DEF_GPUTEST_FOR_ALL_CONTEXTS(VertexAttributeCount, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
#if GR_GPU_STATS
    GrGpu* gpu = context->priv().getGpu();
#endif

    auto surfaceDrawContext = GrSurfaceDrawContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kApprox, {1, 1});
    if (!surfaceDrawContext) {
        ERRORF(reporter, "Could not create render target context.");
        return;
    }
    int attribCnt = context->priv().caps()->maxVertexAttributes();
    if (!attribCnt) {
        ERRORF(reporter, "No attributes allowed?!");
        return;
    }
    context->flushAndSubmit();
    context->priv().resetGpuStats();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    // Adding discard to appease vulkan validation warning about loading uninitialized data on draw
    surfaceDrawContext->discard();

    GrPaint grPaint;
    // This one should succeed.
    surfaceDrawContext->addDrawOp(Op::Make(context, attribCnt));
    context->flushAndSubmit();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 1);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    context->priv().resetGpuStats();
    surfaceDrawContext->addDrawOp(Op::Make(context, attribCnt + 1));
    context->flushAndSubmit();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 1);
#endif
}
