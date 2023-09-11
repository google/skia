/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static initializers to work

#include "include/core/SkColorSpace.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstddef>
#include <memory>
#include <utility>

class GrDstProxyView;
class GrGLSLProgramDataManager;
class GrMeshDrawTarget;
class GrRecordingContext;
class GrSurfaceProxyView;
enum class GrXferBarrierFlags;
struct GrContextOptions;
struct GrShaderCaps;
struct GrSimpleMesh;
struct SkPoint;

namespace {
class Op : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "Test Op"; }

    static GrOp::Owner Make(GrRecordingContext* rContext, int numAttribs) {
        return GrOp::Make<Op>(rContext, numAttribs);
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kNone;
    }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
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
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        class GP : public GrGeometryProcessor {
        public:
            static GrGeometryProcessor* Make(SkArenaAlloc* arena, int numAttribs) {
                return arena->make([&](void* ptr) {
                    return new (ptr) GP(numAttribs);
                });
            }

            const char* name() const override { return "Test GP"; }

            std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
                class Impl : public ProgramImpl {
                public:
                    void setData(const GrGLSLProgramDataManager&,
                                 const GrShaderCaps&,
                                 const GrGeometryProcessor&) override {}

                private:
                    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                        const GP& gp = args.fGeomProc.cast<GP>();
                        args.fVaryingHandler->emitAttributes(gp);
                        WriteOutputPosition(args.fVertBuilder, gpArgs, gp.fAttributes[0].name());
                        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                        fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputColor);
                        fragBuilder->codeAppendf("const half4 %s = half4(1);",
                                                 args.fOutputCoverage);
                    }
                };

                return std::make_unique<Impl>();
            }
            void addToKey(const GrShaderCaps&, skgpu::KeyBuilder* builder) const override {
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
                                                                   SkSLType::kFloat};
                    } else {
                        fAttributes[i] = {fAttribNames[i].c_str(), kFloat2_GrVertexAttribType,
                                                                   SkSLType::kFloat2};
                    }
                }
                this->setVertexAttributesWithImplicitOffsets(fAttributes.get(), numAttribs);
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
                                                                   usesMSAASurface,
                                                                   std::move(appliedClip),
                                                                   dstProxyView,
                                                                   gp,
                                                                   GrProcessorSet::MakeEmptySet(),
                                                                   GrPrimitiveType::kTriangles,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   GrPipeline::InputFlags::kNone);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
        }

        size_t vertexStride = fProgramInfo->geomProc().vertexStride();
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
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    int            fNumAttribs;
    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};
}  // namespace

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(VertexAttributeCount,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
#if GR_GPU_STATS
    GrGpu* gpu = dContext->priv().getGpu();
#endif

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {1, 1},
                                                       SkSurfaceProps(),
                                                       /*label=*/{});
    if (!sdc) {
        ERRORF(reporter, "Could not create render target context.");
        return;
    }
    int attribCnt = dContext->priv().caps()->maxVertexAttributes();
    if (!attribCnt) {
        ERRORF(reporter, "No attributes allowed?!");
        return;
    }
    dContext->flushAndSubmit();
    dContext->priv().resetGpuStats();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    // Adding discard to appease vulkan validation warning about loading uninitialized data on draw
    sdc->discard();

    GrPaint grPaint;
    // This one should succeed.
    sdc->addDrawOp(Op::Make(dContext, attribCnt));
    dContext->flushAndSubmit();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 1);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 0);
#endif
    dContext->priv().resetGpuStats();
    sdc->addDrawOp(Op::Make(dContext, attribCnt + 1));
    dContext->flushAndSubmit();
#if GR_GPU_STATS
    REPORTER_ASSERT(reporter, gpu->stats()->numDraws() == 0);
    REPORTER_ASSERT(reporter, gpu->stats()->numFailedDraws() == 1);
#endif
}
