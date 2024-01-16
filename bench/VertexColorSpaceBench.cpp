/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkString.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkHalf.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

enum Mode {
    kBaseline_Mode,  // Do the wrong thing, but quickly.
    kFloat_Mode,     // Transform colors on CPU, use float4 attributes.
    kHalf_Mode,      // Transform colors on CPU, use half4 attributes.
    kShader_Mode,    // Use ubyte4 attributes, transform colors on GPU (vertex shader).
};

class GP : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, Mode mode,
                                     sk_sp<GrColorSpaceXform> colorSpaceXform) {
        return arena->make([&](void* ptr) {
            return new (ptr) GP(mode, std::move(colorSpaceXform));
        });
    }

    const char* name() const override { return "VertexColorXformGP"; }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        class Impl : public ProgramImpl {
        public:
            void setData(const GrGLSLProgramDataManager& pdman,
                         const GrShaderCaps&,
                         const GrGeometryProcessor& geomProc) override {
                const GP& gp = geomProc.cast<GP>();
                fColorSpaceHelper.setData(pdman, gp.fColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const GP& gp = args.fGeomProc.cast<GP>();
                GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
                GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

                varyingHandler->emitAttributes(gp);

                // Setup color
                GrGLSLVarying varying(SkSLType::kHalf4);
                varyingHandler->addVarying("color", &varying);
                vertBuilder->codeAppendf("half4 color = %s;", gp.fInColor.name());

                if (kShader_Mode == gp.fMode) {
                    fColorSpaceHelper.emitCode(uniformHandler, gp.fColorSpaceXform.get(),
                                               kVertex_GrShaderFlag);
                    SkString xformedColor;
                    vertBuilder->appendColorGamutXform(&xformedColor, "color", &fColorSpaceHelper);
                    vertBuilder->codeAppendf("color = %s;", xformedColor.c_str());
                    vertBuilder->codeAppend("color = half4(color.rgb * color.a, color.a);");
                }

                vertBuilder->codeAppendf("%s = color;", varying.vsOut());
                fragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, varying.fsIn());

                // Position
                WriteOutputPosition(args.fVertBuilder, gpArgs, gp.fInPosition.name());

                // Coverage
                fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
            }

            GrGLSLColorSpaceXformHelper fColorSpaceHelper;
        };

        return std::make_unique<Impl>();
    }

    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const override {
        b->add32(fMode);
        b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()));
    }

private:
    GP(Mode mode, sk_sp<GrColorSpaceXform> colorSpaceXform)
            : INHERITED(kVertexColorSpaceBenchGP_ClassID)
            , fMode(mode)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
        switch (fMode) {
            case kBaseline_Mode:
            case kShader_Mode:
                fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, SkSLType::kHalf4};
                break;
            case kFloat_Mode:
                fInColor = {"inColor", kFloat4_GrVertexAttribType, SkSLType::kHalf4};
                break;
            case kHalf_Mode:
                fInColor = {"inColor", kHalf4_GrVertexAttribType, SkSLType::kHalf4};
                break;
        }
        this->setVertexAttributesWithImplicitOffsets(&fInPosition, 2);
    }

    Mode fMode;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    Attribute fInPosition;
    Attribute fInColor;

    using INHERITED = GrGeometryProcessor;
};

class Op : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "VertColorXformOp"; }

    Op(GrColor color)
            : INHERITED(ClassID())
            , fMode(kBaseline_Mode)
            , fColor(color) {
        this->setBounds(SkRect::MakeWH(100.f, 100.f), HasAABloat::kNo, IsHairline::kNo);
    }

    Op(const SkColor4f& color4f, Mode mode)
            : INHERITED(ClassID())
            , fMode(mode)
            , fColor4f(color4f) {
        SkASSERT(kFloat_Mode == fMode || kHalf_Mode == mode);
        this->setBounds(SkRect::MakeWH(100.f, 100.f), HasAABloat::kNo, IsHairline::kNo);
    }

    Op(GrColor color, sk_sp<GrColorSpaceXform> colorSpaceXform)
            : INHERITED(ClassID())
            , fMode(kShader_Mode)
            , fColor(color)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        this->setBounds(SkRect::MakeWH(100.f, 100.f), HasAABloat::kNo, IsHairline::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kNone;
    }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

private:
    friend class ::GrMemoryPool;

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp = GP::Make(arena, fMode, fColorSpaceXform);

        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps,
                                                                   arena,
                                                                   writeView,
                                                                   usesMSAASurface,
                                                                   std::move(appliedClip),
                                                                   dstProxyView,
                                                                   gp,
                                                                   GrProcessorSet::MakeEmptySet(),
                                                                   GrPrimitiveType::kTriangleStrip,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   GrPipeline::InputFlags::kNone);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
        }

        size_t vertexStride = fProgramInfo->geomProc().vertexStride();
        const int kVertexCount = 1024;
        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex = 0;
        void* verts = target->makeVertexSpace(vertexStride, kVertexCount, &vertexBuffer,
                                              &firstVertex);
        if (!verts) {
            return;
        }

        const float dx = 100.0f / kVertexCount;
        if (kFloat_Mode == fMode) {
            struct V {
                SkPoint fPos;
                SkColor4f fColor;
            };
            SkASSERT(sizeof(V) == vertexStride);
            V* v = (V*)verts;
            for (int i = 0; i < kVertexCount; i += 2) {
                v[i + 0].fPos.set(dx * i, 0.0f);
                v[i + 0].fColor = fColor4f;
                v[i + 1].fPos.set(dx * i, 100.0f);
                v[i + 1].fColor = fColor4f;
            }
        } else if (kHalf_Mode == fMode) {
            struct V {
                SkPoint fPos;
                uint64_t fColor;
            };
            SkASSERT(sizeof(V) == vertexStride);
            uint64_t color;
            to_half(skvx::float4::Load(&fColor4f)).store(&color);
            V* v = (V*)verts;
            for (int i = 0; i < kVertexCount; i += 2) {
                v[i + 0].fPos.set(dx * i, 0.0f);
                v[i + 0].fColor = color;
                v[i + 1].fPos.set(dx * i, 100.0f);
                v[i + 1].fColor = color;
            }
        } else {
            struct V {
                SkPoint fPos;
                GrColor fColor;
            };
            SkASSERT(sizeof(V) == vertexStride);
            V* v = (V*)verts;
            for (int i = 0; i < kVertexCount; i += 2) {
                v[i + 0].fPos.set(dx * i, 0.0f);
                v[i + 0].fColor = fColor;
                v[i + 1].fPos.set(dx * i, 100.0f);
                v[i + 1].fColor = fColor;
            }
        }

        fMesh = target->allocMesh();
        fMesh->set(std::move(vertexBuffer), kVertexCount, firstVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    Mode fMode;
    GrColor fColor;
    SkColor4f fColor4f;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};
}  // namespace

class VertexColorSpaceBench : public Benchmark {
public:
    VertexColorSpaceBench(Mode mode, const char* name) : fMode(mode) {
        fName = "vertexcolorspace";
        fName.appendf("_%s", name);
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto context = canvas->recordingContext()->asDirectContext();
        SkASSERT(context);

        if (kHalf_Mode == fMode &&
            !context->priv().caps()->halfFloatVertexAttributeSupport()) {
            return;
        }

        auto p3 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                        SkNamedGamut::kDisplayP3);
        auto xform = GrColorSpaceXform::Make(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                             p3.get(),            kUnpremul_SkAlphaType);

        SkRandom r;
        const int kDrawsPerLoop = 32;

        for (int i = 0; i < loops; ++i) {
            auto sdc =
                    skgpu::ganesh::SurfaceDrawContext::Make(context,
                                                            GrColorType::kRGBA_8888,
                                                            p3,
                                                            SkBackingFit::kApprox,
                                                            {100, 100},
                                                            SkSurfaceProps(),
                                                            /*label=*/"DrawVertexColorSpaceBench");
            SkASSERT(sdc);

            for (int j = 0; j < kDrawsPerLoop; ++j) {
                SkColor c = r.nextU();
                GrOp::Owner op = nullptr;
                GrRecordingContext* rContext = canvas->recordingContext();
                switch (fMode) {
                    case kBaseline_Mode:
                        op = GrOp::Make<Op>(rContext, SkColorToPremulGrColor(c));
                        break;
                    case kShader_Mode:
                        op = GrOp::Make<Op>(rContext, SkColorToUnpremulGrColor(c), xform);
                        break;
                    case kHalf_Mode:
                    case kFloat_Mode: {
                        SkColor4f c4f = SkColor4f::FromColor(c);
                        c4f = xform->apply(c4f);
                        op = GrOp::Make<Op>(rContext, c4f, fMode);
                    }
                }
                sdc->addDrawOp(std::move(op));
            }

            context->flushAndSubmit();
        }
    }

private:
    SkString fName;
    Mode fMode;

    using INHERITED = Benchmark;
};

DEF_BENCH(return new VertexColorSpaceBench(kBaseline_Mode, "baseline"));
DEF_BENCH(return new VertexColorSpaceBench(kFloat_Mode,    "float"));
DEF_BENCH(return new VertexColorSpaceBench(kHalf_Mode,     "half"));
DEF_BENCH(return new VertexColorSpaceBench(kShader_Mode,   "shader"));
