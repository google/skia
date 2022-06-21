/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/ganesh/v1/SurfaceDrawContext_v1.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

// Simple op that draws a vertex buffer with float2 positions as green triangles. We use this to
// draw GrGpuBuffers to test that the buffer contains the expected values as not all contexts will
// support buffer mapping.
class TestVertexOp final : public GrMeshDrawOp {
public:
    static GrOp::Owner Make(GrRecordingContext* context,
                            sk_sp<GrGpuBuffer> buffer,
                            int baseVertex,
                            int vertexCount,
                            const SkRect& bounds) {
        return GrOp::Make<TestVertexOp>(context,
                                        std::move(buffer),
                                        baseVertex,
                                        vertexCount,
                                        bounds);
    }

    const char* name() const override { return "TestVertexOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps& caps,
                                      const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        static constexpr SkPMColor4f kGreen{0, 1, 0, 1};
        SkPMColor4f color = kGreen;
        auto analysis = fProcessorSet.finalize(GrProcessorAnalysisColor::Opaque::kYes,
                                               GrProcessorAnalysisCoverage::kNone,
                                               clip,
                                               &GrUserStencilSettings::kUnused,
                                               caps,
                                               clampType,
                                               &color);
        SkASSERT(color == kGreen);
        return analysis;
    }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        }
    }

private:
    DEFINE_OP_CLASS_ID

    TestVertexOp(sk_sp<GrGpuBuffer> buffer,
                 int baseVertex,
                 int vertexCount,
                 const SkRect& bounds)
                 : GrMeshDrawOp(ClassID())
                 , fBuffer(std::move(buffer))
                 , fProcessorSet(SkBlendMode::kSrc)
                 , fBaseVertex(baseVertex)
                 , fVertexCount(vertexCount) {
        this->setBounds(bounds, HasAABloat::kNo, GrOp::IsHairline::kNo);
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
        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(
                caps,
                arena,
                writeView,
                usesMSAASurface,
                std::move(appliedClip),
                dstProxyView,
                &fGP,
                std::move(fProcessorSet),
                GrPrimitiveType::kTriangles,
                renderPassXferBarriers,
                colorLoadOp,
                GrPipeline::InputFlags::kNone);
    }

    class GP : public GrGeometryProcessor {
    public:
        GP() : GrGeometryProcessor(kTestFP_ClassID) {
            this->setVertexAttributesWithImplicitOffsets(&kPos, 1);
        }

        const char* name() const override { return "TestVertexOp::GP"; }

        std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
            class Impl : public ProgramImpl {
            public:
                void setData(const GrGLSLProgramDataManager&,
                             const GrShaderCaps&,
                             const GrGeometryProcessor&) override {}

            private:
                void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                    const auto& gp = args.fGeomProc.cast<GP>();
                    args.fVaryingHandler->emitAttributes(gp);
                    args.fFragBuilder->codeAppendf("half4 %s = half4(0, 1, 0, 1);",
                                                   args.fOutputColor);
                    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);",
                                                   args.fOutputCoverage);
                    WriteOutputPosition(args.fVertBuilder, gpArgs, kPos.name());
                }

                UniformHandle fLocalMatrixUni;
            };

            return std::make_unique<Impl>();
        }

        void addToKey(const GrShaderCaps &caps, skgpu::KeyBuilder *builder) const override {}

    private:
        static constexpr Attribute kPos = {"pos", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    };

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        fMesh = target->allocMesh();
        fMesh->set(fBuffer, fVertexCount, fBaseVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo) {
            this->createProgramInfo(flushState);
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    sk_sp<GrGpuBuffer> fBuffer;

    GP fGP;

    GrProcessorSet fProcessorSet;

    int fBaseVertex;
    int fVertexCount;

    GrProgramInfo* fProgramInfo = nullptr;
    GrSimpleMesh*  fMesh        = nullptr;

    friend class ::GrOp;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrGpuBufferTransferTest, reporter, ctxInfo) {
    if (!ctxInfo.directContext()->priv().caps()->transferFromBufferToBufferSupport()) {
        return;
    }

    GrDirectContext* dc = ctxInfo.directContext();

    GrResourceProvider* rp = ctxInfo.directContext()->priv().resourceProvider();

    GrGpu* gpu = ctxInfo.directContext()->priv().getGpu();

    auto create_cpu_to_gpu_buffer = [&](int baseVertex) {
        // Ensure any extra vertices are offscreen
        int totalVertices = baseVertex + 6;
        auto points = std::make_unique<SkPoint[]>(totalVertices);
        SkPoint offscreenPt{-10000, -10000};
        std::fill_n(points.get(), totalVertices, offscreenPt);

        // set the quad at the desired base vertex
        static constexpr SkPoint kUnitQuad[] {{0, 0}, {0, 1}, {1, 0},
                                              {1, 0}, {0, 1}, {1, 1}};
        std::copy_n(kUnitQuad, 6, points.get() + baseVertex);

        return rp->createBuffer(points.get(),
                                totalVertices*sizeof(SkPoint),
                                GrGpuBufferType::kXferCpuToGpu,
                                kStream_GrAccessPattern);
    };

    auto create_vertex_buffer = [&](sk_sp<GrGpuBuffer> srcBuffer,
                                    int srcBaseVertex,
                                    int vbBaseVertex,
                                    bool minSizedTransfers) {
        // make initialization data of offscreen points.
        int dstVertexCount = vbBaseVertex + 6;
        auto points = std::make_unique<SkPoint[]>(dstVertexCount);
        SkPoint offscreenPt{-10000, -10000};
        std::fill_n(points.get(), dstVertexCount, offscreenPt);

        sk_sp<GrGpuBuffer> vb = rp->createBuffer(points.get(),
                                                 dstVertexCount*sizeof(SkPoint),
                                                 GrGpuBufferType::kVertex,
                                                 kDynamic_GrAccessPattern);

        // copy actual quad data from the source buffer to our new vb.

        static constexpr size_t kTotalSize = 6*sizeof(SkPoint);

        size_t srcOffset = srcBaseVertex*sizeof(SkPoint);
        size_t  vbOffset =  vbBaseVertex*sizeof(SkPoint);

        size_t alignment = gpu->caps()->transferFromBufferToBufferAlignment();
        SkASSERT(kTotalSize      % alignment == 0);
        SkASSERT(sizeof(SkPoint) % alignment == 0);

        if (minSizedTransfers) {
            for (size_t n = kTotalSize/alignment, i = 0; i < n; ++i) {
                gpu->transferFromBufferToBuffer(srcBuffer,
                                                srcOffset + i*alignment,
                                                vb,
                                                vbOffset + i*alignment,
                                                alignment);
            }
        } else {
            gpu->transferFromBufferToBuffer(srcBuffer, srcOffset, vb, vbOffset, kTotalSize);
        }
        return vb;
    };

    auto sdc = skgpu::v1::SurfaceDrawContext::Make(dc,
                                                   GrColorType::kRGBA_8888,
                                                   nullptr,
                                                   SkBackingFit::kExact,
                                                   {1, 1},
                                                   SkSurfaceProps{},
                                                   std::string_view{});
    if (!sdc) {
        ERRORF(reporter, "Could not create draw context");
        return;
    }

    auto pm = GrPixmap::Allocate(sdc->imageInfo().makeColorType(GrColorType::kRGBA_F32));

    for (bool byteAtATime : {false, true}) {
        for (int srcBaseVertex : {0, 5}) {
            auto src = create_cpu_to_gpu_buffer(srcBaseVertex);
            if (!src) {
                ERRORF(reporter, "Could not create src buffer");
                return;
            }
            for (int vbBaseVertex : {0, 2}) {
                auto vb  = create_vertex_buffer(src, srcBaseVertex, vbBaseVertex, byteAtATime);
                if (!vb) {
                    ERRORF(reporter, "Could not create vertex buffer");
                    return;
                }

                static constexpr SkColor4f kRed{1, 0, 0, 1};

                static constexpr SkRect kBounds{0, 0, 1, 1};

                sdc->clear(kRed);

                sdc->addDrawOp(nullptr, TestVertexOp::Make(dc,
                                                           vb,
                                                           vbBaseVertex,
                                                           /*vertexCount=*/6,
                                                           kBounds));

                auto color = static_cast<SkPMColor4f*>(pm.addr());
                *color = kRed.premul();
                if (!sdc->readPixels(dc, pm, {0, 0})) {
                    ERRORF(reporter, "Read back failed.");
                    return;
                }

                static constexpr SkPMColor4f kGreen{0, 1, 0, 1};

                REPORTER_ASSERT(reporter, *color == kGreen, "src base vertex: %d, "
                                                            "vb base vertex: %d, "
                                                            "byteAtATime: %d",
                                                            srcBaseVertex,
                                                            vbBaseVertex,
                                                            byteAtATime);
            }
        }
    }
}
