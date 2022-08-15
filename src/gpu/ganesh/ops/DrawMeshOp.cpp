/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/DrawMeshOp.h"

#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace {

GrPrimitiveType primitive_type(SkMesh::Mode mode) {
    switch (mode) {
        case SkMesh::Mode::kTriangles:     return GrPrimitiveType::kTriangles;
        case SkMesh::Mode::kTriangleStrip: return GrPrimitiveType::kTriangleStrip;
    }
    SkUNREACHABLE;
}

class MeshGP : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     sk_sp<SkMeshSpecification> spec,
                                     sk_sp<GrColorSpaceXform> colorSpaceXform,
                                     const SkMatrix& viewMatrix,
                                     const std::optional<SkPMColor4f>& color,
                                     bool needsLocalCoords,
                                     sk_sp<const SkData> uniforms) {
        return arena->make([&](void* ptr) {
            return new (ptr) MeshGP(std::move(spec),
                                    std::move(colorSpaceXform),
                                    viewMatrix,
                                    std::move(color),
                                    needsLocalCoords,
                                    std::move(uniforms));
        });
    }

    const char* name() const override { return "MeshGP"; }

    void addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override {
        b->add32(SkMeshSpecificationPriv::Hash(*fSpec), "custom mesh spec hash");
        b->add32(ProgramImpl::ComputeMatrixKey(caps, fViewMatrix), "view matrix key");
        if (SkMeshSpecificationPriv::GetColorType(*fSpec) !=
            SkMeshSpecificationPriv::ColorType::kNone) {
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
            const auto& mgp = geomProc.cast<MeshGP>();
            SetTransform(pdman, shaderCaps, fViewMatrixUniform, mgp.fViewMatrix, &fViewMatrix);
            fColorSpaceHelper.setData(pdman, mgp.fColorSpaceXform.get());
            if (fColorUniform.isValid()) {
                pdman.set4fv(fColorUniform, 1, mgp.fColor.vec());
            }
            if (mgp.fUniforms) {
                pdman.setRuntimeEffectUniforms(mgp.fSpec->uniforms(),
                                               SkSpan(fSpecUniformHandles),
                                               mgp.fUniforms->data());
            }
        }

    private:
        class MeshCallbacks : public SkSL::PipelineStage::Callbacks {
        public:
            MeshCallbacks(Impl* self,
                          const MeshGP& gp,
                          GrGLSLShaderBuilder* builder,
                          GrGLSLUniformHandler* uniformHandler,
                          const char* mainName,
                          const SkSL::Context& context)
                    : fSelf(self)
                    , fGP(gp)
                    , fBuilder(builder)
                    , fUniformHandler(uniformHandler)
                    , fMainName(mainName)
                    , fContext(context) {}

            std::string declareUniform(const SkSL::VarDeclaration* decl) override {
                const SkSL::Variable& var = decl->var();
                SkASSERT(!var.type().isOpaque());

                const SkSL::Type* type = &var.type();
                bool isArray = false;
                if (type->isArray()) {
                    type = &type->componentType();
                    isArray = true;
                }

                SkSLType gpuType;
                SkAssertResult(SkSL::type_to_sksltype(fContext, *type, &gpuType));

                SkString name(var.name());
                const SkSpan<const SkMeshSpecification::Uniform> uniforms = fGP.fSpec->uniforms();
                auto it = std::find_if(uniforms.begin(),
                                       uniforms.end(),
                                       [&name](SkMeshSpecification::Uniform uniform) {
                    return uniform.name == std::string_view(name.c_str(), name.size());
                });
                SkASSERT(it != uniforms.end());

                UniformHandle* handle = &fSelf->fSpecUniformHandles[it - uniforms.begin()];
                if (handle->isValid()) {
                    const GrShaderVar& uniformVar = fUniformHandler->getUniformVariable(*handle);
                    return std::string(uniformVar.getName().c_str());
                }

                const SkMeshSpecification::Uniform& uniform = *it;
                GrShaderFlags shaderFlags = kNone_GrShaderFlags;
                if (uniform.flags & SkMeshSpecification::Uniform::Flags::kVertex_Flag) {
                    shaderFlags |= kVertex_GrShaderFlag;
                }
                if (uniform.flags & SkMeshSpecification::Uniform::Flags::kFragment_Flag) {
                    shaderFlags |= kFragment_GrShaderFlag;
                }
                SkASSERT(shaderFlags != kNone_GrShaderFlags);

                const char* mangledName = nullptr;
                *handle = fUniformHandler->addUniformArray(&fGP,
                                                           shaderFlags,
                                                           gpuType,
                                                           name.c_str(),
                                                           isArray ? var.type().columns() : 0,
                                                           &mangledName);
                return std::string(mangledName);
            }

            std::string getMangledName(const char* name) override {
                return std::string(fBuilder->getMangledFunctionName(name).c_str());
            }

            std::string getMainName() override { return fMainName; }

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

            std::string sampleShader(int index, std::string coords) override {
                SK_ABORT("No children allowed.");
            }

            std::string sampleColorFilter(int index, std::string color) override {
                SK_ABORT("No children allowed.");
            }

            std::string sampleBlender(int index, std::string src, std::string dst) override {
                SK_ABORT("No children allowed.");
            }

            std::string toLinearSrgb(std::string color) override {
                SK_ABORT("Color transform intrinsics not allowed.");
            }

            std::string fromLinearSrgb(std::string Color) override {
                SK_ABORT("Color transform intrinsics not allowed.");
            }

            Impl*                                            fSelf;
            const MeshGP&                                    fGP;
            GrGLSLShaderBuilder*                             fBuilder;
            GrGLSLUniformHandler*                            fUniformHandler;
            const char*                                      fMainName;
            const SkSL::Context&                             fContext;
        };

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const MeshGP& mgp = args.fGeomProc.cast<MeshGP>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            SkASSERT(fSpecUniformHandles.empty());
            fSpecUniformHandles.resize(mgp.fSpec->uniforms().size());

            ////// VS

            // emit attributes
            varyingHandler->emitAttributes(mgp);

            // Define the user's vert function.
            SkString userVertName = vertBuilder->getMangledFunctionName("custom_mesh_vs");
            const SkSL::Program* customVS = SkMeshSpecificationPriv::VS(*mgp.fSpec);
            MeshCallbacks vsCallbacks(this,
                                      mgp,
                                      vertBuilder,
                                      uniformHandler,
                                      userVertName.c_str(),
                                      *customVS->fContext);
            SkSL::PipelineStage::ConvertProgram(*customVS,
                                                /*sampleCoords=*/"",
                                                /*inputColor=*/"",
                                                /*destColor=*/"",
                                                &vsCallbacks);

            // Copy the individual attributes into a struct
            vertBuilder->codeAppendf("%s attributes;",
                                     vsCallbacks.getMangledName("Attributes").c_str());
            size_t i = 0;
            SkASSERT(mgp.vertexAttributes().count() == (int)mgp.fSpec->attributes().size());
            for (auto attr : mgp.vertexAttributes()) {
                vertBuilder->codeAppendf("attributes.%s = %s;",
                                         mgp.fSpec->attributes()[i++].name.c_str(),
                                         attr.name());
            }

            // Call the user's vert function.
            vertBuilder->codeAppendf("%s varyings;",
                                     vsCallbacks.getMangledName("Varyings").c_str());
            vertBuilder->codeAppendf("float2 pos = %s(attributes, varyings);",
                                     userVertName.c_str());

            // Unpack the varyings from the struct into individual varyings.
            std::vector<GrGLSLVarying> varyings;
            varyings.reserve(SkMeshSpecificationPriv::Varyings(*mgp.fSpec).size());
            for (const auto& v : SkMeshSpecificationPriv::Varyings(*mgp.fSpec)) {
                varyings.emplace_back(SkMeshSpecificationPriv::VaryingTypeAsSLType(v.type));
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
                                mgp.fViewMatrix,
                                &fViewMatrixUniform);

            ////// FS

            fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            fragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);

            // Define the user's frag function.
            SkString userFragName = fragBuilder->getMangledFunctionName("custom_mesh_fs");
            const SkSL::Program* customFS = SkMeshSpecificationPriv::FS(*mgp.fSpec);
            MeshCallbacks fsCallbacks(this,
                                      mgp,
                                      fragBuilder,
                                      uniformHandler,
                                      userFragName.c_str(),
                                      *customFS->fContext);
            SkSL::PipelineStage::ConvertProgram(*customFS,
                                                /*sampleCoords=*/"",
                                                /*inputColor=*/"",
                                                /*destColor=*/"",
                                                &fsCallbacks);

            // Pack the varyings into a struct to call the user's frag code.
            fragBuilder->codeAppendf("%s varyings;",
                                     fsCallbacks.getMangledName("Varyings").c_str());
            i = 0;
            for (const auto& varying : SkMeshSpecificationPriv::Varyings(*mgp.fSpec)) {
                fragBuilder->codeAppendf("varyings.%s = %s;",
                                         varying.name.c_str(),
                                         varyings[i++].vsOut());
            }
            SkMeshSpecificationPriv::ColorType meshColorType =
                    SkMeshSpecificationPriv::GetColorType(*mgp.fSpec);
            const char* uniformColorName = nullptr;
            if (mgp.fColor != SK_PMColor4fILLEGAL) {
                fColorUniform = uniformHandler->addUniform(nullptr,
                                                           kFragment_GrShaderFlag,
                                                           SkSLType::kHalf4,
                                                           "color",
                                                           &uniformColorName);
            }
            SkString localCoordAssignment;
            if (SkMeshSpecificationPriv::HasLocalCoords(*mgp.fSpec) && mgp.fNeedsLocalCoords) {
                localCoordAssignment = "float2 local =";
            }
            if (meshColorType == SkMeshSpecificationPriv::ColorType::kNone) {
                fragBuilder->codeAppendf("%s %s(varyings);",
                                         localCoordAssignment.c_str(),
                                         userFragName.c_str());
                SkASSERT(uniformColorName);
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, uniformColorName);
            } else {
                fColorSpaceHelper.emitCode(uniformHandler,
                                           mgp.fColorSpaceXform.get(),
                                           kFragment_GrShaderFlag);
                if (meshColorType == SkMeshSpecificationPriv::ColorType::kFloat4) {
                    fragBuilder->codeAppendf("float4 color;");
                } else {
                    SkASSERT(meshColorType == SkMeshSpecificationPriv::ColorType::kHalf4);
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
            if (mgp.fNeedsLocalCoords) {
                if (SkMeshSpecificationPriv::HasLocalCoords(*mgp.fSpec)) {
                    gpArgs->fLocalCoordVar = GrShaderVar("local", SkSLType::kFloat2);
                    gpArgs->fLocalCoordShader = kFragment_GrShaderType;
                } else {
                    gpArgs->fLocalCoordVar = GrShaderVar("pos", SkSLType::kFloat2);
                    gpArgs->fLocalCoordShader = kVertex_GrShaderType;
                }
            }
        }

    private:
        SkMatrix fViewMatrix = SkMatrix::InvalidMatrix();

        UniformHandle              fViewMatrixUniform;
        UniformHandle              fColorUniform;
        std::vector<UniformHandle> fSpecUniformHandles;

        GrGLSLColorSpaceXformHelper fColorSpaceHelper;
    };

    MeshGP(sk_sp<SkMeshSpecification>        spec,
           sk_sp<GrColorSpaceXform>          colorSpaceXform,
           const SkMatrix&                   viewMatrix,
           const std::optional<SkPMColor4f>& color,
           bool                              needsLocalCoords,
           sk_sp<const SkData>               uniforms)
            : INHERITED(kVerticesGP_ClassID)
            , fSpec(std::move(spec))
            , fUniforms(std::move(uniforms))
            , fViewMatrix(viewMatrix)
            , fColorSpaceXform(std::move(colorSpaceXform))
            , fNeedsLocalCoords(needsLocalCoords) {
        fColor = color.value_or(SK_PMColor4fILLEGAL);
        for (const auto& srcAttr : fSpec->attributes()) {
            fAttributes.emplace_back(
                    srcAttr.name.c_str(),
                    SkMeshSpecificationPriv::AttrTypeAsVertexAttribType(srcAttr.type),
                    SkMeshSpecificationPriv::AttrTypeAsSLType(srcAttr.type),
                    srcAttr.offset);
        }
        this->setVertexAttributes(fAttributes.data(), fAttributes.size(), fSpec->stride());
    }

    sk_sp<SkMeshSpecification> fSpec;
    sk_sp<const SkData>        fUniforms;
    std::vector<Attribute>     fAttributes;
    SkMatrix                   fViewMatrix;
    SkPMColor4f                fColor;
    sk_sp<GrColorSpaceXform>   fColorSpaceXform;
    bool                       fNeedsLocalCoords;

    using INHERITED = GrGeometryProcessor;
};

class MeshOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    MeshOp(GrProcessorSet*,
           const SkPMColor4f&,
           const SkMesh&,
           GrAAType,
           sk_sp<GrColorSpaceXform>,
           const SkMatrixProvider&);

    MeshOp(GrProcessorSet*,
           const SkPMColor4f&,
           sk_sp<SkVertices>,
           const GrPrimitiveType*,
           GrAAType,
           sk_sp<GrColorSpaceXform>,
           const SkMatrixProvider&);

    const char* name() const override { return "MeshOp"; }

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
     * Built either from a SkMesh or a SkVertices. In the former case the data is owned
     * by Mesh and in the latter it is not. Meshes made from SkVertices can contain a SkMatrix
     * to enable CPU-based transformation but Meshes made from SkMesh cannot.
     */
    class Mesh {
    public:
        Mesh() = delete;
        explicit Mesh(const SkMesh& mesh);
        Mesh(sk_sp<SkVertices>, const SkMatrix& viewMatrix);
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& m);

        Mesh& operator=(const Mesh&) = delete;
        Mesh& operator=(Mesh&&) = delete;  // not used by SkSTArray but could be implemented.

        ~Mesh();

        bool isFromVertices() const { return SkToBool(fVertices); }

        std::tuple<sk_sp<const GrGpuBuffer>, size_t> gpuVB() const {
            if (this->isFromVertices()) {
                return {};
            }
            return {fMeshData.vb->asGpuBuffer(), fMeshData.voffset};
        }

        std::tuple<sk_sp<const GrGpuBuffer>, size_t> gpuIB() const {
            if (this->isFromVertices() || !fMeshData.ib) {
                return {};
            }
            return {fMeshData.ib->asGpuBuffer(), fMeshData.ioffset};
        }

        void writeVertices(skgpu::VertexWriter& writer,
                           const SkMeshSpecification& spec,
                           bool transform) const;

        int vertexCount() const {
            return this->isFromVertices() ? fVertices->priv().vertexCount() : fMeshData.vcount;
        }

        const uint16_t* indices() const {
            if (this->isFromVertices()) {
                return fVertices->priv().indices();
            }
            if (!fMeshData.ib) {
                return nullptr;
            }
            auto data = fMeshData.ib->peek();
            if (!data) {
                return nullptr;
            }
            return SkTAddOffset<const uint16_t>(data, fMeshData.ioffset);
        }

        int indexCount() const {
            return this->isFromVertices() ? fVertices->priv().indexCount() : fMeshData.icount;
        }

    private:
        struct MeshData {
            sk_sp<const SkMeshPriv::VB> vb;
            sk_sp<const SkMeshPriv::IB> ib;

            size_t vcount = 0;
            size_t icount = 0;

            size_t voffset = 0;
            size_t ioffset = 0;
        };

        sk_sp<SkVertices> fVertices;

        union {
            SkMatrix fViewMatrix;
            MeshData fMeshData;
        };
    };

    Helper                     fHelper;
    sk_sp<SkMeshSpecification> fSpecification;
    bool                       fIgnoreSpecColor = false;
    GrPrimitiveType            fPrimitiveType;
    SkSTArray<1, Mesh>         fMeshes;
    sk_sp<GrColorSpaceXform>   fColorSpaceXform;
    SkPMColor4f                fColor; // Used if no color from spec or analysis overrides.
    SkMatrix                   fViewMatrix;
    sk_sp<const SkData>        fUniforms;
    int                        fVertexCount;
    int                        fIndexCount;
    GrSimpleMesh*              fMesh = nullptr;
    GrProgramInfo*             fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

MeshOp::Mesh::Mesh(const SkMesh& mesh) {
    new (&fMeshData) MeshData();
    fMeshData.vb = sk_ref_sp(static_cast<SkMeshPriv::VB*>(mesh.vertexBuffer().get()));
    if (mesh.indexBuffer()) {
        fMeshData.ib = sk_ref_sp(static_cast<SkMeshPriv::IB*>(mesh.indexBuffer().get()));
    }
    fMeshData.vcount  = mesh.vertexCount();
    fMeshData.voffset = mesh.vertexOffset();
    fMeshData.icount  = mesh.indexCount();
    fMeshData.ioffset = mesh.indexOffset();

    // The caller could modify CPU buffers after the draw so we must copy the data.
    if (fMeshData.vb->peek()) {
        auto data = SkTAddOffset<const void>(fMeshData.vb->peek(), fMeshData.voffset);
        size_t size = fMeshData.vcount*mesh.spec()->stride();
        fMeshData.vb = SkMeshPriv::CpuVertexBuffer::Make(data, size);
        fMeshData.voffset = 0;
    }

    if (fMeshData.ib && fMeshData.ib->peek()) {
        auto data = SkTAddOffset<const void>(fMeshData.ib->peek(), fMeshData.ioffset);
        size_t size = fMeshData.icount*sizeof(uint16_t);
        fMeshData.ib = SkMeshPriv::CpuIndexBuffer::Make(data, size);
        fMeshData.ioffset = 0;
    }
}

MeshOp::Mesh::Mesh(sk_sp<SkVertices> vertices, const SkMatrix& viewMatrix)
        : fVertices(std::move(vertices)), fViewMatrix(viewMatrix) {
    SkASSERT(fVertices);
}

MeshOp::Mesh::Mesh(Mesh&& that) {
    fVertices = std::move(that.fVertices);
    if (fVertices) {
        fViewMatrix = that.fViewMatrix;
        // 'that' is now not-a-vertices. Make sure it can be safely destroyed.
        new (&that.fMeshData) MeshData();
    } else {
        fMeshData = std::move(that.fMeshData);
    }
}

MeshOp::Mesh::~Mesh() {
    if (!this->isFromVertices()) {
        fMeshData.~MeshData();
    }
}

void MeshOp::Mesh::writeVertices(skgpu::VertexWriter& writer,
                                 const SkMeshSpecification& spec,
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
            if (SkMeshSpecificationPriv::HasColors(spec)) {
                SkASSERT(fVertices->priv().hasColors());
                writer << fVertices->priv().colors()[i];
            }
            if (SkMeshSpecificationPriv::HasLocalCoords(spec)) {
                SkASSERT(fVertices->priv().hasTexCoords());
                writer << fVertices->priv().texCoords()[i];
            }
        }
    } else {
        const void* data = fMeshData.vb->peek();
        if (data) {
            auto vdata = SkTAddOffset<const char>(data, fMeshData.voffset);
            writer << skgpu::VertexWriter::Array(vdata, spec.stride()*fMeshData.vcount);
        }
    }
}

MeshOp::MeshOp(GrProcessorSet*          processorSet,
               const SkPMColor4f&       color,
               const SkMesh&            mesh,
               GrAAType                 aaType,
               sk_sp<GrColorSpaceXform> colorSpaceXform,
               const SkMatrixProvider&  matrixProvider)
        : INHERITED(ClassID())
        , fHelper(processorSet, aaType)
        , fPrimitiveType(primitive_type(mesh.mode()))
        , fColorSpaceXform(std::move(colorSpaceXform))
        , fColor(color)
        , fViewMatrix(matrixProvider.localToDevice()) {
    fMeshes.emplace_back(mesh);

    fSpecification = mesh.spec();
    if (fColorSpaceXform) {
        fUniforms = SkRuntimeEffectPriv::TransformUniforms(mesh.spec()->uniforms(),
                                                           mesh.uniforms(),
                                                           fColorSpaceXform->steps());
    } else {
        fUniforms = mesh.uniforms();
    }

    fVertexCount = fMeshes.back().vertexCount();
    fIndexCount  = fMeshes.back().indexCount();

    this->setTransformedBounds(mesh.bounds(), fViewMatrix, HasAABloat::kNo, IsHairline::kNo);
}

static sk_sp<SkMeshSpecification> make_vertices_spec(bool hasColors, bool hasTex) {
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;
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
        // Using float4 for the output color to work around skbug.com/12761
        fs += "main(Varyings v, out float4 color) {\n"
              "color = float4(v.color.bgr*v.color.a, v.color.a);\n";
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
    auto [spec, error] = SkMeshSpecification::Make(
            SkSpan(attributes),
            size,
            SkSpan(varyings),
            vs,
            fs);
    SkASSERT(spec);
    return spec;
}

MeshOp::MeshOp(GrProcessorSet*          processorSet,
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
SkString MeshOp::onDumpInfo() const { return {}; }
#endif

GrDrawOp::FixedFunctionFlags MeshOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrProcessorSet::Analysis MeshOp::finalize(const GrCaps& caps,
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

GrGeometryProcessor* MeshOp::makeGP(SkArenaAlloc* arena) {
    std::optional<SkPMColor4f> color;
    if (fIgnoreSpecColor || !SkMeshSpecificationPriv::HasColors(*fSpecification)) {
        color.emplace(fColor);
    }
    // Check if we're pre-transforming the vertices on the CPU.
    const SkMatrix& vm = fViewMatrix == SkMatrix::InvalidMatrix() ? SkMatrix::I() : fViewMatrix;
    return MeshGP::Make(arena,
                        fSpecification,
                        fColorSpaceXform,
                        vm,
                        color,
                        fHelper.usesLocalCoords(),
                        fUniforms);
}

void MeshOp::onCreateProgramInfo(const GrCaps* caps,
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

void MeshOp::onPrepareDraws(GrMeshDrawTarget* target) {
    size_t vertexStride = fSpecification->stride();
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex;
    std::tie(vertexBuffer, firstVertex) = fMeshes[0].gpuVB();

    if (!vertexBuffer) {
        skgpu::VertexWriter verts = target->makeVertexWriter(vertexStride,
                                                             fVertexCount,
                                                             &vertexBuffer,
                                                             &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices.\n");
            return;
        }

        bool transform = fViewMatrix == SkMatrix::InvalidMatrix();
        for (const auto& m : fMeshes) {
            m.writeVertices(verts, *fSpecification, transform);
        }
    } else {
        SkASSERT(fMeshes.count() == 1);
        SkASSERT(firstVertex % fSpecification->stride() == 0);
        firstVertex /= fSpecification->stride();
    }

    sk_sp<const GrBuffer> indexBuffer;
    int firstIndex = 0;

    std::tie(indexBuffer, firstIndex) = fMeshes[0].gpuIB();
    if (fIndexCount && !indexBuffer) {
        uint16_t* indices = nullptr;
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
    } else if (indexBuffer) {
        SkASSERT(fMeshes.count() == 1);
        SkASSERT(firstIndex % sizeof(uint16_t) == 0);
        firstIndex /= sizeof(uint16_t);
    }

    SkASSERT(!fMesh);
    fMesh = target->allocMesh();

    if (indexBuffer) {
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

void MeshOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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

GrOp::CombineResult MeshOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) {
    auto that = t->cast<MeshOp>();
    if (!fMeshes[0].isFromVertices() || !that->fMeshes[0].isFromVertices()) {
        return GrOp::CombineResult::kCannotCombine;
    }

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

    if (SkMeshSpecificationPriv::Hash(*this->fSpecification) !=
        SkMeshSpecificationPriv::Hash(*that->fSpecification)) {
        return CombineResult::kCannotCombine;
    }

    if (fSpecification->uniformSize()) {
        size_t size = fSpecification->uniformSize();
        SkASSERT(fUniforms);
        SkASSERT(fUniforms->size() >= size);
        SkASSERT(that->fUniforms);
        SkASSERT(that->fUniforms->size() >= size);
        if (memcmp(fUniforms->data(), that->fUniforms->data(), size) != 0) {
            return GrOp::CombineResult::kCannotCombine;
        }
    }

    if (!SkMeshSpecificationPriv::HasColors(*fSpecification) && fColor != that->fColor) {
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
            !SkMeshSpecificationPriv::HasLocalCoords(*fSpecification)) {
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

namespace skgpu::v1::DrawMeshOp {

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 const SkMesh& mesh,
                 const SkMatrixProvider& matrixProvider,
                 GrAAType aaType,
                 sk_sp<GrColorSpaceXform> colorSpaceXform) {
    return GrSimpleMeshDrawOpHelper::FactoryHelper<MeshOp>(context,
                                                           std::move(paint),
                                                           mesh,
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
    return GrSimpleMeshDrawOpHelper::FactoryHelper<MeshOp>(context,
                                                           std::move(paint),
                                                           std::move(vertices),
                                                           overridePrimitiveType,
                                                           aaType,
                                                           std::move(colorSpaceXform),
                                                           matrixProvider);
}

}  // namespace skgpu::v1::DrawMeshOp
