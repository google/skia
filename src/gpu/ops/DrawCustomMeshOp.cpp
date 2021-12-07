/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/DrawVerticesOp.h"

#include "include/core/SkCustomMesh.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkCustomMeshPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace {

GrPrimitiveType primitive_type(SkCustomMesh::Mode mode) {
    switch (mode) {
        case SkCustomMesh::Mode::kTriangles:     return GrPrimitiveType::kTriangles;
        case SkCustomMesh::Mode::kTriangleStrip: return GrPrimitiveType::kTriangleStrip;
    }
    SkUNREACHABLE;
}

class CustomMeshGP : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     sk_sp<SkCustomMeshSpecification> spec,
                                     sk_sp<GrColorSpaceXform> colorSpaceXform,
                                     const SkMatrix& viewMatrix,
                                     const skstd::optional<SkPMColor4f>& color) {
        return arena->make([&](void* ptr) {
            return new (ptr) CustomMeshGP(std::move(spec),
                                          std::move(colorSpaceXform),
                                          viewMatrix,
                                          std::move(color));
        });
    }

    const char* name() const override { return "CustomMeshGP"; }

    void addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override {
        b->add32(SkCustomMeshSpecificationPriv::Hash(*fSpec), "custom mesh spec hash");
        b->add32(ProgramImpl::ComputeMatrixKey(caps, fViewMatrix), "view matrix key");
        if (SkCustomMeshSpecificationPriv::GetColorType(*fSpec) !=
            SkCustomMeshSpecificationPriv::ColorType::kNone) {
            b->add32(GrColorSpaceXform::XformKey(fColorSpaceXform.get()), "colorspace xform key");
        }
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        return std::make_unique<Impl>();
    }

private:
    class Impl : public ProgramImpl {
    public:
        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrShaderCaps& shaderCaps,
                     const GrGeometryProcessor& geomProc) override {
            const auto& cmgp = geomProc.cast<CustomMeshGP>();
            SetTransform(pdman, shaderCaps, fViewMatrixUniform, cmgp.fViewMatrix, &fViewMatrix);
            fColorSpaceHelper.setData(pdman, cmgp.fColorSpaceXform.get());
            if (fColorUniform.isValid()) {
                pdman.set4fv(fColorUniform, 1, cmgp.fColor.vec());
            }
        }

    private:
        class MeshCallbacks : public SkSL::PipelineStage::Callbacks {
        public:
            MeshCallbacks(Impl* self,
                          GrGLSLShaderBuilder* builder,
                          const char* mainName,
                          const SkSL::Context& context)
                    : fSelf(self), fBuilder(builder), fMainName(mainName), fContext(context) {}

            using String = SkSL::String;

            String declareUniform(const SkSL::VarDeclaration* decl) override {
                SK_ABORT("uniforms not allowed");
            }

            String getMangledName(const char* name) override {
                return String(fBuilder->getMangledFunctionName(name).c_str());
            }

            String getMainName() override { return fMainName; }

            void defineFunction(const char* decl, const char* body, bool isMain) override {
                fBuilder->emitFunction(decl, body);
            }

            void declareFunction(const char* decl) override {
                fBuilder->emitFunctionPrototype(decl);
            }

            void defineStruct(const char* definition) override {
                fBuilder->definitionAppend(definition);
            }

            void declareGlobal(const char* declaration) override {
                fBuilder->definitionAppend(declaration);
            }

            String sampleShader(int index, String coords) override {
                SK_ABORT("No children allowed.");
            }

            String sampleColorFilter(int index, String color) override {
                SK_ABORT("No children allowed.");
            }

            String sampleBlender(int index, String src, String dst) override {
                SK_ABORT("No children allowed.");
            }

            String toLinearSrgb(String color) override {
                SK_ABORT("Color transform intrinsics not allowed.");
            }

            String fromLinearSrgb(String Color) override {
                SK_ABORT("Color transform intrinsics not allowed.");
            }

            Impl*                 fSelf;
            GrGLSLShaderBuilder*  fBuilder;
            const char*           fMainName;
            const SkSL::Context&  fContext;
        };

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const CustomMeshGP& cmgp = args.fGeomProc.cast<CustomMeshGP>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            ////// VS

            // emit attributes
            varyingHandler->emitAttributes(cmgp);

            // Define the user's vert function.
            SkString userVertName = vertBuilder->getMangledFunctionName("custom_mesh_vs");
            const SkSL::Program* customVS = SkCustomMeshSpecificationPriv::VS(*cmgp.fSpec);
            MeshCallbacks vsCallbacks(this, vertBuilder, userVertName.c_str(), *customVS->fContext);
            SkSL::PipelineStage::ConvertProgram(*customVS,
                                                /*sampleCoords=*/"",
                                                /*inputColor=*/"",
                                                /*destColor=*/"",
                                                &vsCallbacks);

            // Copy the individual attributes into a struct
            vertBuilder->codeAppendf("%s attributes;",
                                     vsCallbacks.getMangledName("Attributes").c_str());
            size_t i = 0;
            SkASSERT(cmgp.vertexAttributes().count() == (int)cmgp.fSpec->attributes().size());
            for (auto attr : cmgp.vertexAttributes()) {
                vertBuilder->codeAppendf("attributes.%s = %s;",
                                         cmgp.fSpec->attributes()[i++].name.c_str(),
                                         attr.name());
            }

            // Call the user's vert function.
            vertBuilder->codeAppendf("%s varyings;",
                                     vsCallbacks.getMangledName("Varyings").c_str());
            vertBuilder->codeAppendf("float2 pos = %s(attributes, varyings);",
                                     userVertName.c_str());

            // Unpack the varyings from the struct into individual varyings.
            std::vector<GrGLSLVarying> varyings;
            varyings.reserve(SkCustomMeshSpecificationPriv::Varyings(*cmgp.fSpec).size());
            for (const auto& v : SkCustomMeshSpecificationPriv::Varyings(*cmgp.fSpec)) {
                varyings.emplace_back(SkCustomMeshSpecificationPriv::VaryingTypeAsSLType(v.type));
                varyingHandler->addVarying(v.name.c_str(), &varyings.back());
                vertBuilder->codeAppendf("%s = varyings.%s;",
                                         varyings.back().vsOut(),
                                         v.name.c_str());
            }

            // Setup position
            WriteOutputPosition(vertBuilder,
                                uniformHandler,
                                *args.fShaderCaps,
                                gpArgs,
                                "pos",
                                cmgp.fViewMatrix,
                                &fViewMatrixUniform);

            ////// FS

            fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);

            // Define the user's frag function.
            SkString userFragName = fragBuilder->getMangledFunctionName("custom_mesh_fs");
            const SkSL::Program* customFS = SkCustomMeshSpecificationPriv::FS(*cmgp.fSpec);
            MeshCallbacks fsCallbacks(this, fragBuilder, userFragName.c_str(), *customFS->fContext);
            SkSL::PipelineStage::ConvertProgram(*customFS,
                                                /*sampleCoords=*/"",
                                                /*inputColor=*/"",
                                                /*destColor=*/"",
                                                &fsCallbacks);

            // Pack the varyings into a struct to call the user's frag code.
            fragBuilder->codeAppendf("%s varyings;",
                                     fsCallbacks.getMangledName("Varyings").c_str());
            i = 0;
            for (const auto& varying : SkCustomMeshSpecificationPriv::Varyings(*cmgp.fSpec)) {
                fragBuilder->codeAppendf("varyings.%s = %s;",
                                         varying.name.c_str(),
                                         varyings[i++].vsOut());
            }
            SkCustomMeshSpecificationPriv::ColorType meshColorType =
                    SkCustomMeshSpecificationPriv::GetColorType(*cmgp.fSpec);
            const char* uniformColorName = nullptr;
            if (cmgp.fColor != SK_PMColor4fILLEGAL) {
                fColorUniform = uniformHandler->addUniform(nullptr,
                                                           kFragment_GrShaderFlag,
                                                           kHalf4_GrSLType,
                                                           "color",
                                                           &uniformColorName);
            }
            if (meshColorType == SkCustomMeshSpecificationPriv::ColorType::kNone) {
                fragBuilder->codeAppendf("float2 local = %s(varyings);", userFragName.c_str());
                SkASSERT(uniformColorName);
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, uniformColorName);
            } else {
                fColorSpaceHelper.emitCode(uniformHandler,
                                           cmgp.fColorSpaceXform.get(),
                                           kFragment_GrShaderFlag);
                if (meshColorType == SkCustomMeshSpecificationPriv::ColorType::kFloat4) {
                    fragBuilder->codeAppendf("float4 color;");
                } else {
                    SkASSERT(meshColorType == SkCustomMeshSpecificationPriv::ColorType::kHalf4);
                    fragBuilder->codeAppendf("half4 color;");
                }

                fragBuilder->codeAppendf("float2 local = %s(varyings, color);",
                                         userFragName.c_str());
                // We ignore the user's color if analysis told us to emit a specific color. The user
                // color might be float4 and we expect a half4 in the colorspace helper.
                const char* color = uniformColorName ? uniformColorName : "half4(color)";
                SkString xformedColor;
                fragBuilder->appendColorGamutXform(&xformedColor, color, &fColorSpaceHelper);
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, xformedColor.c_str());
            }
            gpArgs->fLocalCoordVar = GrShaderVar("local", kFloat2_GrSLType);
            gpArgs->fLocalCoordShader = kFragment_GrShaderType;
        }

    private:
        SkMatrix fViewMatrix = SkMatrix::InvalidMatrix();

        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;

        GrGLSLColorSpaceXformHelper fColorSpaceHelper;
    };

    CustomMeshGP(sk_sp<SkCustomMeshSpecification> spec,
                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                 const SkMatrix& viewMatrix,
                 const skstd::optional<SkPMColor4f>& color)
            : INHERITED(kVerticesGP_ClassID)
            , fSpec(std::move(spec))
            , fViewMatrix(viewMatrix)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        fColor = color.value_or(SK_PMColor4fILLEGAL);
        for (const auto& srcAttr : fSpec->attributes()) {
            fAttributes.emplace_back(
                    srcAttr.name.c_str(),
                    SkCustomMeshSpecificationPriv::AttrTypeAsVertexAttribType(srcAttr.type),
                    SkCustomMeshSpecificationPriv::AttrTypeAsSLType(srcAttr.type),
                    srcAttr.offset);
        }
        this->setVertexAttributes(fAttributes.data(), fAttributes.size(), fSpec->stride());
    }

    sk_sp<SkCustomMeshSpecification> fSpec;
    std::vector<Attribute>           fAttributes;
    SkMatrix                         fViewMatrix;
    SkPMColor4f                      fColor;
    sk_sp<GrColorSpaceXform>         fColorSpaceXform;

    using INHERITED = GrGeometryProcessor;
};

class CustomMeshOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    CustomMeshOp(GrProcessorSet*,
                 const SkPMColor4f&,
                 SkCustomMesh,
                 GrAAType,
                 sk_sp<GrColorSpaceXform>,
                 const SkMatrixProvider&);

    const char* name() const override { return "CustomMeshOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override;

    void onPrepareDraws(GrMeshDrawTarget*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif

    GrGeometryProcessor* makeGP(SkArenaAlloc*);

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps&) override;

    struct Mesh {
        std::unique_ptr<const char[]>     vb;
        std::unique_ptr<const uint16_t[]> indices;
        int                               vcount;
        int                               icount;
    };

    Helper                           fHelper;
    sk_sp<SkCustomMeshSpecification> fSpecification;
    bool                             fIgnoreSpecColor = false;
    SkCustomMesh::Mode               fMode;
    SkSTArray<1, Mesh>               fMeshes;
    sk_sp<GrColorSpaceXform>         fColorSpaceXform;
    SkPMColor4f                      fColor; // Used if no color from spec or analysis overrides.
    SkMatrix                         fViewMatrix;
    GrSimpleMesh*                    fMesh = nullptr;
    GrProgramInfo*                   fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

CustomMeshOp::CustomMeshOp(GrProcessorSet* processorSet,
                           const SkPMColor4f& color,
                           SkCustomMesh cm,
                           GrAAType aaType,
                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                           const SkMatrixProvider& matrixProvider)
        : INHERITED(ClassID())
        , fHelper(processorSet, aaType)
        , fMode(cm.mode)
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fColor(color)
        , fViewMatrix(matrixProvider.localToDevice()) {
    Mesh m;
    m.vb      = SkCopyCustomMeshVB(cm);
    m.vcount  = cm.vcount;
    m.indices = SkCopyCustomMeshIB(cm);
    m.icount  = cm.icount;
    fMeshes.push_back(std::move(m));

    fSpecification = std::move(cm.spec);

    this->setTransformedBounds(cm.bounds, fViewMatrix, HasAABloat::kNo, IsHairline::kNo);
}

#if GR_TEST_UTILS
SkString CustomMeshOp::onDumpInfo() const { return {}; }
#endif

GrDrawOp::FixedFunctionFlags CustomMeshOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrProcessorSet::Analysis CustomMeshOp::finalize(const GrCaps& caps,
                                                const GrAppliedClip* clip,
                                                GrClampType clampType) {
    GrProcessorAnalysisColor gpColor;
    gpColor.setToUnknown();
    auto result = fHelper.finalizeProcessors(caps,
                                             clip,
                                             clampType,
                                             GrProcessorAnalysisCoverage::kNone,
                                             &gpColor);
    if (gpColor.isConstant(&fColor)) {
        fIgnoreSpecColor = true;
    }
    return result;
}

GrGeometryProcessor* CustomMeshOp::makeGP(SkArenaAlloc* arena) {
    skstd::optional<SkPMColor4f> color;
    if (fIgnoreSpecColor || !SkCustomMeshSpecificationPriv::HasColors(*fSpecification)) {
        color.emplace(fColor);
    }
    return CustomMeshGP::Make(arena, fSpecification, fColorSpaceXform, fViewMatrix, color);
}

void CustomMeshOp::onCreateProgramInfo(const GrCaps* caps,
                                       SkArenaAlloc* arena,
                                       const GrSurfaceProxyView& writeView,
                                       bool usesMSAASurface,
                                       GrAppliedClip&& appliedClip,
                                       const GrDstProxyView& dstProxyView,
                                       GrXferBarrierFlags renderPassXferBarriers,
                                       GrLoadOp colorLoadOp) {
    fProgramInfo = fHelper.createProgramInfo(caps,
                                             arena,
                                             writeView,
                                             usesMSAASurface,
                                             std::move(appliedClip),
                                             dstProxyView,
                                             this->makeGP(arena),
                                             primitive_type(fMode),
                                             renderPassXferBarriers,
                                             colorLoadOp);
}

void CustomMeshOp::onPrepareDraws(GrMeshDrawTarget* target) {
    size_t vertexStride = fSpecification->stride();
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex = 0;
    size_t vcount = 0;
    for (const auto& m : fMeshes) {
        vcount += m.vcount;
    }
    skgpu::VertexWriter verts{target->makeVertexSpace(vertexStride,
                                                      vcount,
                                                      &vertexBuffer,
                                                      &firstVertex)};
    if (!verts) {
        SkDebugf("Could not allocate vertices.\n");
        return;
    }

    for (const auto& m : fMeshes) {
        verts << skgpu::VertexWriter::Array(static_cast<const char*>(m.vb.get()),
                                            vertexStride * m.vcount);
    }

    sk_sp<const GrBuffer> indexBuffer;
    int firstIndex = 0;
    uint16_t* indices = nullptr;
    if (fMeshes[0].icount) {
        SkASSERT(fMeshes.size() == 1);
        indices = target->makeIndexSpace(fMeshes[0].icount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices.\n");
            return;
        }
        std::copy_n(fMeshes[0].indices.get(), fMeshes[0].icount, indices);
    }

    SkASSERT(!fMesh);
    fMesh = target->allocMesh();

    if (indices) {
        fMesh->setIndexed(std::move(indexBuffer),
                          fMeshes[0].icount,
                          firstIndex,
                          /*minIndexValue=*/0,
                          fMeshes[0].vcount,
                          GrPrimitiveRestart::kNo,
                          std::move(vertexBuffer),
                          firstVertex);
    } else {
        fMesh->set(std::move(vertexBuffer), vcount, firstVertex);
    }
}

void CustomMeshOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    if (!fProgramInfo || !fMesh) {
        return;
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

GrOp::CombineResult CustomMeshOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) {
    auto that = t->cast<CustomMeshOp>();

    if (fMode       == SkCustomMesh::Mode::kTriangleStrip ||
        that->fMode == SkCustomMesh::Mode::kTriangleStrip) {
        return CombineResult::kCannotCombine;
    }

    if (this->fMeshes[0].indices || that->fMeshes[0].indices) {
        return CombineResult::kCannotCombine;
    }

    int thisHash = SkCustomMeshSpecificationPriv::Hash(*fSpecification);
    int thatHash = SkCustomMeshSpecificationPriv::Hash(*fSpecification);

    if (thisHash != thatHash                                                                   ||
        (!SkCustomMeshSpecificationPriv::HasColors(*fSpecification) && fColor != that->fColor) ||
        fViewMatrix != that->fViewMatrix) {
        return CombineResult::kCannotCombine;
    }

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // NOTE: The source color space is part of the spec, and the destination gamut is determined by
    // the render target context. A mis-match should be impossible.
    SkASSERT(GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get()));

    fMeshes.move_back_n(that->fMeshes.count(), that->fMeshes.begin());
    return CombineResult::kMerged;
}

}  // anonymous namespace

namespace skgpu::v1::DrawCustomMeshOp {

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 SkCustomMesh cm,
                 const SkMatrixProvider& matrixProvider,
                 GrAAType aaType,
                 sk_sp<GrColorSpaceXform> colorSpaceXform) {
    return GrSimpleMeshDrawOpHelper::FactoryHelper<CustomMeshOp>(context,
                                                                 std::move(paint),
                                                                 std::move(cm),
                                                                 aaType,
                                                                 std::move(colorSpaceXform),
                                                                 matrixProvider);
}

}  // namespace skgpu::v1::DrawCustomMeshOp
