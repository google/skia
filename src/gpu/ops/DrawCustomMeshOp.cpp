/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/DrawCustomMeshOp.h"

#include "include/core/SkCustomMesh.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkCustomMeshPriv.h"
#include "src/core/SkVerticesPriv.h"
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
                                     const skstd::optional<SkPMColor4f>& color,
                                     bool needsLocalCoords) {
        return arena->make([&](void* ptr) {
            return new (ptr) CustomMeshGP(std::move(spec),
                                          std::move(colorSpaceXform),
                                          viewMatrix,
                                          std::move(color),
                                          needsLocalCoords);
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
            SkString localCoordAssignment;
            if (SkCustomMeshSpecificationPriv::HasLocalCoords(*cmgp.fSpec) &&
                cmgp.fNeedsLocalCoords) {
                localCoordAssignment = "float2 local =";
            }
            if (meshColorType == SkCustomMeshSpecificationPriv::ColorType::kNone) {
                fragBuilder->codeAppendf("%s %s(varyings);",
                                         localCoordAssignment.c_str(),
                                         userFragName.c_str());
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

                fragBuilder->codeAppendf("%s %s(varyings, color);",
                                         localCoordAssignment.c_str(),
                                         userFragName.c_str());
                // We ignore the user's color if analysis told us to emit a specific color. The user
                // color might be float4 and we expect a half4 in the colorspace helper.
                const char* color = uniformColorName ? uniformColorName : "half4(color)";
                SkString xformedColor;
                fragBuilder->appendColorGamutXform(&xformedColor, color, &fColorSpaceHelper);
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, xformedColor.c_str());
            }
            if (cmgp.fNeedsLocalCoords) {
                if (SkCustomMeshSpecificationPriv::HasLocalCoords(*cmgp.fSpec)) {
                    gpArgs->fLocalCoordVar = GrShaderVar("local", kFloat2_GrSLType);
                    gpArgs->fLocalCoordShader = kFragment_GrShaderType;
                } else {
                    gpArgs->fLocalCoordVar = GrShaderVar("pos", kFloat2_GrSLType);
                    gpArgs->fLocalCoordShader = kVertex_GrShaderType;
                }
            }
        }

    private:
        SkMatrix fViewMatrix = SkMatrix::InvalidMatrix();

        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;

        GrGLSLColorSpaceXformHelper fColorSpaceHelper;
    };

    CustomMeshGP(sk_sp<SkCustomMeshSpecification>    spec,
                 sk_sp<GrColorSpaceXform>            colorSpaceXform,
                 const SkMatrix&                     viewMatrix,
                 const skstd::optional<SkPMColor4f>& color,
                 bool                                needsLocalCoords)
            : INHERITED(kVerticesGP_ClassID)
            , fSpec(std::move(spec))
            , fViewMatrix(viewMatrix)
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fNeedsLocalCoords(needsLocalCoords) {
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
    bool                             fNeedsLocalCoords;

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

    CustomMeshOp(GrProcessorSet*,
                 const SkPMColor4f&,
                 sk_sp<SkVertices>,
                 const GrPrimitiveType*,
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

    /**
     * Built either from a SkCustomMesh or a SkVertices. In the former case the data is owned
     * by Mesh and in the latter it is not. Meshes made from SkVertices can contain a SkMatrix
     * to enable CPU-based transformation but Meshes made from SkCustomMesh cannot.
     */
    class Mesh {
    public:
        Mesh() = delete;
        Mesh(const SkCustomMesh& cm);
        Mesh(sk_sp<SkVertices>, const SkMatrix& viewMatrix);
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& m);

        Mesh& operator=(const Mesh&) = delete;
        Mesh& operator=(Mesh&&) = delete; // not used by SkSTArray but could be implemented.

        ~Mesh();

        bool isFromVertices() const { return SkToBool(fVertices); }

        void writeVertices(skgpu::VertexWriter& writer,
                           const SkCustomMeshSpecification& spec,
                           bool transform) const;

        int vertexCount() const {
            return this->isFromVertices() ? fVertices->priv().vertexCount() : fCMData.vcount;
        }

        const uint16_t* indices() const {
            return this->isFromVertices() ? fVertices->priv().indices() : fCMData.indices;
        }

        int indexCount() const {
            return this->isFromVertices() ? fVertices->priv().indexCount() : fCMData.icount;
        }

    private:
        struct CMData {
            const char*     vb;
            const uint16_t* indices;
            int             vcount;
            int             icount;
        };

        sk_sp<SkVertices> fVertices;

        union {
            SkMatrix fViewMatrix;
            CMData   fCMData;
        };
    };

    Helper                           fHelper;
    sk_sp<SkCustomMeshSpecification> fSpecification;
    bool                             fIgnoreSpecColor = false;
    GrPrimitiveType                  fPrimitiveType;
    SkSTArray<1, Mesh>               fMeshes;
    sk_sp<GrColorSpaceXform>         fColorSpaceXform;
    SkPMColor4f                      fColor; // Used if no color from spec or analysis overrides.
    SkMatrix                         fViewMatrix;
    int                              fVertexCount;
    int                              fIndexCount;
    GrSimpleMesh*                    fMesh = nullptr;
    GrProgramInfo*                   fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

CustomMeshOp::Mesh::Mesh(const SkCustomMesh& cm) {
    fCMData.vb       = SkCopyCustomMeshVB(cm).release();
    fCMData.indices  = SkCopyCustomMeshIB(cm).release();
    fCMData.vcount   = cm.vcount;
    fCMData.icount   = cm.icount;
}

CustomMeshOp::Mesh::Mesh(sk_sp<SkVertices> vertices, const SkMatrix& viewMatrix)
        : fVertices(std::move(vertices)), fViewMatrix(viewMatrix) {
    SkASSERT(fVertices);
}

CustomMeshOp::Mesh::Mesh(Mesh&& m) {
    fVertices = std::move(m.fVertices);
    if (fVertices) {
        fViewMatrix = m.fViewMatrix;
    } else {
        fCMData = m.fCMData;
    }
    m.fCMData.vb      = nullptr;
    m.fCMData.indices = nullptr;
}

CustomMeshOp::Mesh::~Mesh() {
    if (!this->isFromVertices()) {
        delete[] fCMData.indices;
        delete[] fCMData.vb;
    }
}

void CustomMeshOp::Mesh::writeVertices(skgpu::VertexWriter& writer,
                                       const SkCustomMeshSpecification& spec,
                                       bool transform) const {
    SkASSERT(!transform || this->isFromVertices());
    if (this->isFromVertices()) {
        int vertexCount = fVertices->priv().vertexCount();
        for (int i = 0; i < vertexCount; ++i) {
            SkPoint pos = fVertices->priv().positions()[i];
            if (transform) {
                SkASSERT(!fViewMatrix.hasPerspective());
                fViewMatrix.mapPoints(&pos, 1);
            }
            writer << pos;
            if (SkCustomMeshSpecificationPriv::HasColors(spec)) {
                SkASSERT(fVertices->priv().hasColors());
                writer << fVertices->priv().colors()[i];
            }
            if (SkCustomMeshSpecificationPriv::HasLocalCoords(spec)) {
                SkASSERT(fVertices->priv().hasTexCoords());
                writer << fVertices->priv().texCoords()[i];
            }
        }
    } else {
        writer << skgpu::VertexWriter::Array(fCMData.vb, spec.stride()*fCMData.vcount);
    }
}

CustomMeshOp::CustomMeshOp(GrProcessorSet*          processorSet,
                           const SkPMColor4f&       color,
                           SkCustomMesh             cm,
                           GrAAType                 aaType,
                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                           const SkMatrixProvider&  matrixProvider)
        : INHERITED(ClassID())
        , fHelper(processorSet, aaType)
        , fPrimitiveType(primitive_type(cm.mode))
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fColor(color)
        , fViewMatrix(matrixProvider.localToDevice()) {
    fMeshes.emplace_back(cm);

    fSpecification = std::move(cm.spec);

    fVertexCount = fMeshes.back().vertexCount();
    fIndexCount  = fMeshes.back().indexCount();

    this->setTransformedBounds(cm.bounds, fViewMatrix, HasAABloat::kNo, IsHairline::kNo);
}

static sk_sp<SkCustomMeshSpecification> make_vertices_spec(bool hasColors, bool hasTex) {
    using Attribute = SkCustomMeshSpecification::Attribute;
    using Varying   = SkCustomMeshSpecification::Varying;
    std::vector<Attribute> attributes;
    attributes.reserve(3);
    attributes.push_back({Attribute::Type::kFloat2, 0, SkString{"pos"}});
    size_t size = 8;

    std::vector<Varying> varyings;
    attributes.reserve(2);

    SkString vs("float2 main(Attributes a, out Varyings v) {\n");
    SkString fs(hasTex ? "float2 " : "void ");

    if (hasColors) {
        attributes.push_back({Attribute::Type::kUByte4_unorm, size, SkString{"color"}});
        varyings.push_back({Varying::Type::kHalf4, SkString{"color"}});
        vs += "v.color = a.color;\n";
        fs += "main(Varyings v, out half4 color) {\n"
              "color = half4(v.color.bgr*v.color.a, v.color.a);\n";
        size += 4;
    } else {
        fs += "main(Varyings v) {\n";
    }

    if (hasTex) {
        attributes.push_back({Attribute::Type::kFloat2, size, SkString{"tex"}});
        varyings.push_back({Varying::Type::kFloat2, SkString{"tex"}});
        vs += "v.tex = a.tex;\n";
        fs += "return v.tex;\n";
        size += 8;
    }
    vs += "return a.pos;\n}";
    fs += "}";
    auto [spec, error] = SkCustomMeshSpecification::Make(
            SkMakeSpan(attributes),
            size,
            SkMakeSpan(varyings),
            vs,
            fs);
    SkASSERT(spec);
    return spec;
}

CustomMeshOp::CustomMeshOp(GrProcessorSet*          processorSet,
                           const SkPMColor4f&       color,
                           sk_sp<SkVertices>        vertices,
                           const GrPrimitiveType*   overridePrimitiveType,
                           GrAAType                 aaType,
                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                           const SkMatrixProvider&  matrixProvider)
        : INHERITED(ClassID())
        , fHelper(processorSet, aaType)
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fColor(color)
        , fViewMatrix(matrixProvider.localToDevice()) {
    int attrs = (vertices->priv().hasColors()    ? 0b01 : 0b00) |
                (vertices->priv().hasTexCoords() ? 0b10 : 0b00);
    switch (attrs) {
        case 0b00: {
            static const auto kSpec = make_vertices_spec(false, false);
            fSpecification = kSpec;
            break;
        }
        case 0b01: {
            static const auto kSpec = make_vertices_spec(true, false);
            fSpecification = kSpec;
            break;
        }
        case 0b10: {
            static const auto kSpec = make_vertices_spec(false, true);
            fSpecification = kSpec;
            break;
        }
        case 0b11: {
            static const auto kSpec = make_vertices_spec(true, true);
            fSpecification = kSpec;
            break;
        }
    }
    SkASSERT(fSpecification);

    if (overridePrimitiveType) {
        fPrimitiveType = *overridePrimitiveType;
    } else {
        switch (vertices->priv().mode()) {
            case SkVertices::kTriangles_VertexMode:
                fPrimitiveType = GrPrimitiveType::kTriangles;
                break;
            case SkVertices::kTriangleStrip_VertexMode:
                fPrimitiveType = GrPrimitiveType::kTriangleStrip;
                break;
            case SkVertices::kTriangleFan_VertexMode:
                SkUNREACHABLE;
        }
    }

    IsHairline isHairline = IsHairline::kNo;
    if (GrIsPrimTypeLines(fPrimitiveType) || fPrimitiveType == GrPrimitiveType::kPoints) {
        isHairline = IsHairline::kYes;
    }
    this->setTransformedBounds(vertices->bounds(), fViewMatrix, HasAABloat::kNo, isHairline);

    fMeshes.emplace_back(std::move(vertices), fViewMatrix);

    fVertexCount = fMeshes.back().vertexCount();
    fIndexCount  = fMeshes.back().indexCount();
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
    // Check if we're pre-transforming the vertices on the CPU.
    const SkMatrix& vm = fViewMatrix == SkMatrix::InvalidMatrix() ? SkMatrix::I() : fViewMatrix;
    return CustomMeshGP::Make(arena,
                              fSpecification,
                              fColorSpaceXform,
                              vm,
                              color,
                              fHelper.usesLocalCoords());
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
                                             fPrimitiveType,
                                             renderPassXferBarriers,
                                             colorLoadOp);
}

void CustomMeshOp::onPrepareDraws(GrMeshDrawTarget* target) {
    size_t vertexStride = fSpecification->stride();
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex = 0;
    skgpu::VertexWriter verts{target->makeVertexSpace(vertexStride,
                                                      fVertexCount,
                                                      &vertexBuffer,
                                                      &firstVertex)};
    if (!verts) {
        SkDebugf("Could not allocate vertices.\n");
        return;
    }

    bool transform = fViewMatrix == SkMatrix::InvalidMatrix();
    for (const auto& m : fMeshes) {
        m.writeVertices(verts, *fSpecification, transform);
    }

    sk_sp<const GrBuffer> indexBuffer;
    int firstIndex = 0;
    uint16_t* indices = nullptr;
    if (fIndexCount) {
        indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices.\n");
            return;
        }
        // We can just copy the first mesh's indices. Subsequent meshes need their indices adjusted.
        std::copy_n(fMeshes[0].indices(), fMeshes[0].indexCount(), indices);
        int voffset = fMeshes[0].vertexCount();
        int ioffset = fMeshes[0].indexCount();
        for (size_t m = 1; m < fMeshes.size(); ++m) {
            for (int i = 0; i < fMeshes[m].indexCount(); ++i) {
                indices[ioffset++] = fMeshes[m].indices()[i] + voffset;
            }
            voffset += fMeshes[m].vertexCount();
        }
        SkASSERT(voffset == fVertexCount);
        SkASSERT(ioffset == fIndexCount);
    }

    SkASSERT(!fMesh);
    fMesh = target->allocMesh();

    if (indices) {
        fMesh->setIndexed(std::move(indexBuffer),
                          fIndexCount,
                          firstIndex,
                          /*minIndexValue=*/0,
                          fVertexCount,
                          GrPrimitiveRestart::kNo,
                          std::move(vertexBuffer),
                          firstVertex);
    } else {
        fMesh->set(std::move(vertexBuffer), fVertexCount, firstVertex);
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

    // Check for a combinable primitive type.
    if (!(fPrimitiveType == GrPrimitiveType::kTriangles ||
          fPrimitiveType == GrPrimitiveType::kLines     ||
          fPrimitiveType == GrPrimitiveType::kPoints)) {
        return CombineResult::kCannotCombine;
    }

    if (fPrimitiveType != that->fPrimitiveType) {
        return CombineResult::kCannotCombine;
    }

    if (SkToBool(fIndexCount) != SkToBool(that->fIndexCount)) {
        return CombineResult::kCannotCombine;
    }
    if (SkToBool(fIndexCount) && fVertexCount + that->fVertexCount > SkToInt(UINT16_MAX)) {
        return CombineResult::kCannotCombine;
    }

    if (SkCustomMeshSpecificationPriv::Hash(*this->fSpecification) !=
        SkCustomMeshSpecificationPriv::Hash(*that->fSpecification)) {
        return CombineResult::kCannotCombine;
    }

    if (!SkCustomMeshSpecificationPriv::HasColors(*fSpecification) && fColor != that->fColor) {
        return CombineResult::kCannotCombine;
    }

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    if (fViewMatrix != that->fViewMatrix) {
        if (!fMeshes[0].isFromVertices() || !that->fMeshes[0].isFromVertices()) {
            // We don't know how to CPU transform actual custom meshes on CPU.
            return CombineResult::kCannotCombine;
        }
        // If we use local coords and the local coords come from positions then we can't pre-
        // transform the positions on the CPU.
        if (fHelper.usesLocalCoords() &&
            !SkCustomMeshSpecificationPriv::HasLocalCoords(*fSpecification)) {
            return CombineResult::kCannotCombine;
        }
        // We only support two-component position attributes. This means we would not get
        // perspective-correct interpolation of attributes if we transform on the CPU.
        if ((this->fViewMatrix.isFinite() && this->fViewMatrix.hasPerspective()) ||
            (that->fViewMatrix.isFinite() && that->fViewMatrix.hasPerspective())) {
            return CombineResult::kCannotCombine;
        }
        // This is how we record that we must CPU-transform the vertices.
        fViewMatrix = SkMatrix::InvalidMatrix();
    }

    // NOTE: The source color space is part of the spec, and the destination gamut is determined by
    // the render target context. A mis-match should be impossible.
    SkASSERT(GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get()));

    fMeshes.move_back_n(that->fMeshes.count(), that->fMeshes.begin());
    fVertexCount += that->fVertexCount;
    fIndexCount  += that->fIndexCount;
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

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 sk_sp<SkVertices> vertices,
                 const GrPrimitiveType* overridePrimitiveType,
                 const SkMatrixProvider& matrixProvider,
                 GrAAType aaType,
                 sk_sp<GrColorSpaceXform> colorSpaceXform) {
    return GrSimpleMeshDrawOpHelper::FactoryHelper<CustomMeshOp>(context,
                                                                 std::move(paint),
                                                                 std::move(vertices),
                                                                 overridePrimitiveType,
                                                                 aaType,
                                                                 std::move(colorSpaceXform),
                                                                 matrixProvider);
}

}  // namespace skgpu::v1::DrawCustomMeshOp
