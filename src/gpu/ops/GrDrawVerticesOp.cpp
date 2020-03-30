/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrDrawVerticesOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

enum class ColorArrayType {
    kUnused,
    kPremulGrColor,
    kSkColor,
};

enum class LocalCoordsType {
    kUnused,
    kUsePosition,
    kExplicit,
};

static GrVertexAttribType SkVerticesAttributeToGrVertexAttribType(const SkVertices::Attribute& a) {
    switch (a.fType) {
        case SkVertices::Attribute::Type::kFloat:       return kFloat_GrVertexAttribType;
        case SkVertices::Attribute::Type::kFloat2:      return kFloat2_GrVertexAttribType;
        case SkVertices::Attribute::Type::kFloat3:      return kFloat3_GrVertexAttribType;
        case SkVertices::Attribute::Type::kFloat4:      return kFloat4_GrVertexAttribType;
        case SkVertices::Attribute::Type::kByte4_unorm: return kUByte4_norm_GrVertexAttribType;
    }
    SkUNREACHABLE;
}

static GrSLType SkVerticesAttributeToGrSLType(const SkVertices::Attribute& a) {
    switch (a.fType) {
        case SkVertices::Attribute::Type::kFloat:       return kFloat_GrSLType;
        case SkVertices::Attribute::Type::kFloat2:      return kFloat2_GrSLType;
        case SkVertices::Attribute::Type::kFloat3:      return kFloat3_GrSLType;
        case SkVertices::Attribute::Type::kFloat4:      return kFloat4_GrSLType;
        case SkVertices::Attribute::Type::kByte4_unorm: return kHalf4_GrSLType;
    }
    SkUNREACHABLE;
}

class VerticesGP : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     LocalCoordsType localCoordsType,
                                     ColorArrayType colorArrayType,
                                     const SkPMColor4f& color,
                                     sk_sp<GrColorSpaceXform> colorSpaceXform,
                                     const SkMatrix& viewMatrix,
                                     const SkVertices::Attribute* attrs,
                                     int attrCount) {
        return arena->make<VerticesGP>(localCoordsType, colorArrayType, color,
                                       std::move(colorSpaceXform), viewMatrix, attrs, attrCount);
    }

    const char* name() const override { return "VerticesGP"; }

    const SkPMColor4f& color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }

    const Attribute& positionAttr() const { return fAttributes[kPositionIndex]; }
    const Attribute& colorAttr() const { return fAttributes[kColorIndex]; }
    const Attribute& localCoordsAttr() const { return fAttributes[kLocalCoordsIndex]; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor()
            : fViewMatrix(SkMatrix::InvalidMatrix())
            , fColor(SK_PMColor4fILLEGAL) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const VerticesGP& gp = args.fGP.cast<VerticesGP>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(gp);

            // Setup pass through color
            if (gp.colorAttr().isInitialized()) {
                GrGLSLVarying varying(kHalf4_GrSLType);
                varyingHandler->addVarying("color", &varying);
                vertBuilder->codeAppendf("half4 color = %s;", gp.colorAttr().name());

                // For SkColor, do a red/blue swap, possible color space conversion, and premul
                if (gp.fColorArrayType == ColorArrayType::kSkColor) {
                    vertBuilder->codeAppend("color = color.bgra;");

                    if (gp.fColorSpaceXform) {
                        fColorSpaceHelper.emitCode(uniformHandler, gp.fColorSpaceXform.get(),
                                                   kVertex_GrShaderFlag);
                        SkString xformedColor;
                        vertBuilder->appendColorGamutXform(&xformedColor, "color",
                                                           &fColorSpaceHelper);
                        vertBuilder->codeAppendf("color = %s;", xformedColor.c_str());
                    }

                    vertBuilder->codeAppend("color = half4(color.rgb * color.a, color.a);");
                }

                vertBuilder->codeAppendf("%s = color;\n", varying.vsOut());
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
            } else {
                this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor,
                                        &fColorUniform);
            }

            // Setup position
            this->writeOutputPosition(vertBuilder,
                                      uniformHandler,
                                      gpArgs,
                                      gp.positionAttr().name(),
                                      gp.viewMatrix(),
                                      &fViewMatrixUniform);

            // emit transforms using either explicit local coords or positions
            const auto& coordsAttr = gp.localCoordsAttr().isInitialized() ? gp.localCoordsAttr()
                                                                          : gp.positionAttr();
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 coordsAttr.asShaderVar(),
                                 SkMatrix::I(),
                                 args.fFPCoordTransformHandler);

            // Add varyings and globals for all custom attributes
            for (size_t i = kFirstCustomIndex; i < gp.fAttributes.size(); ++i) {
                const auto& attr(gp.fAttributes[i]);
                GrGLSLVarying varying(attr.gpuType());
                args.fVaryingHandler->addVarying(attr.name(), &varying);
                args.fVertBuilder->codeAppendf("%s = %s;", varying.vsOut(), attr.name());

                GrShaderVar var(SkStringPrintf("_vtx_attr_%d", int(i - kFirstCustomIndex)),
                                attr.gpuType());
                args.fFragBuilder->declareGlobal(var);
                args.fFragBuilder->codeAppendf("%s = %s;", var.c_str(), varying.fsIn());
            }

            fragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
        }

        static inline void GenKey(const GrGeometryProcessor& gp,
                                  const GrShaderCaps&,
                                  GrProcessorKeyBuilder* b) {
            const VerticesGP& vgp = gp.cast<VerticesGP>();
            uint32_t key = 0;
            key |= (vgp.fColorArrayType == ColorArrayType::kSkColor) ? 0x1 : 0;
            key |= ComputePosKey(vgp.viewMatrix()) << 20;
            b->add32(key);
            b->add32(GrColorSpaceXform::XformKey(vgp.fColorSpaceXform.get()));
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp,
                     const CoordTransformRange& transformRange) override {
            const VerticesGP& vgp = gp.cast<VerticesGP>();

            if (!vgp.viewMatrix().isIdentity() &&
                !SkMatrixPriv::CheapEqual(fViewMatrix, vgp.viewMatrix())) {
                fViewMatrix = vgp.viewMatrix();
                pdman.setSkMatrix(fViewMatrixUniform, fViewMatrix);
            }

            if (!vgp.colorAttr().isInitialized() && vgp.color() != fColor) {
                pdman.set4fv(fColorUniform, 1, vgp.color().vec());
                fColor = vgp.color();
            }

            this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);

            fColorSpaceHelper.setData(pdman, vgp.fColorSpaceXform.get());
        }

    private:
        SkMatrix fViewMatrix;
        SkPMColor4f fColor;
        UniformHandle fViewMatrixUniform;
        UniformHandle fColorUniform;
        GrGLSLColorSpaceXformHelper fColorSpaceHelper;

        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    friend class ::SkArenaAlloc; // for access to ctor

    VerticesGP(LocalCoordsType localCoordsType,
               ColorArrayType colorArrayType,
               const SkPMColor4f& color,
               sk_sp<GrColorSpaceXform> colorSpaceXform,
               const SkMatrix& viewMatrix,
               const SkVertices::Attribute* attrs,
               int attrCount)
            : INHERITED(kVerticesGP_ClassID)
            , fColorArrayType(colorArrayType)
            , fColor(color)
            , fViewMatrix(viewMatrix)
            , fColorSpaceXform(std::move(colorSpaceXform)) {
        constexpr Attribute missingAttr;
        fAttributes.push_back({"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType});
        fAttributes.push_back(fColorArrayType != ColorArrayType::kUnused
                                      ? MakeColorAttribute("inColor", false)
                                      : missingAttr);
        fAttributes.push_back(localCoordsType == LocalCoordsType::kExplicit
                        ? Attribute{"inLocalCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType}
                        : missingAttr);

        for (int i = 0; i < attrCount; ++i) {
            // Attributes store char*, so allocate long-lived storage for the (dynamic) names
            fAttrNames.push_back(SkStringPrintf("_vtx_attr%d", i));
            fAttributes.push_back({fAttrNames.back().c_str(),
                                   SkVerticesAttributeToGrVertexAttribType(attrs[i]),
                                   SkVerticesAttributeToGrSLType(attrs[i])});
        }

        this->setVertexAttributes(fAttributes.data(), fAttributes.size());
    }

    enum {
        kPositionIndex    = 0,
        kColorIndex       = 1,
        kLocalCoordsIndex = 2,
        kFirstCustomIndex = 3,
    };

    std::vector<SkString> fAttrNames;
    std::vector<Attribute> fAttributes;
    ColorArrayType fColorArrayType;
    SkPMColor4f fColor;
    SkMatrix fViewMatrix;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    typedef GrGeometryProcessor INHERITED;
};

class DrawVerticesOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    DrawVerticesOp(const Helper::MakeArgs&, const SkPMColor4f&, sk_sp<SkVertices>,
                   GrPrimitiveType, GrAAType, sk_sp<GrColorSpaceXform>, const SkMatrix& viewMatrix,
                   const SkRuntimeEffect*);

    const char* name() const override { return "DrawVerticesOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView* outputView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&) override;

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    GrGeometryProcessor* makeGP(SkArenaAlloc*);

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return GrPrimitiveType::kTriangles == fPrimitiveType ||
               GrPrimitiveType::kLines == fPrimitiveType ||
               GrPrimitiveType::kPoints == fPrimitiveType;
    }

    CombineResult onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*, const GrCaps&) override;

    struct Mesh {
        SkPMColor4f fColor;  // Used if this->hasPerVertexColors() is false.
        sk_sp<SkVertices> fVertices;
        SkMatrix fViewMatrix;
        bool fIgnoreColors;

        bool hasPerVertexColors() const {
            return fVertices->priv().hasColors() && !fIgnoreColors;
        }
    };

    bool isIndexed() const {
        // Consistency enforced in onCombineIfPossible.
        return fMeshes[0].fVertices->priv().hasIndices();
    }

    bool requiresPerVertexColors() const {
        return fColorArrayType != ColorArrayType::kUnused;
    }

    bool requiresPerVertexLocalCoords() const {
        return fLocalCoordsType == LocalCoordsType::kExplicit;
    }

    size_t vertexStride() const {
        return sizeof(SkPoint) +
               (this->requiresPerVertexColors() ? sizeof(uint32_t) : 0) +
               (this->requiresPerVertexLocalCoords() ? sizeof(SkPoint) : 0) +
               fMeshes[0].fVertices->priv().customDataSize();
    }

    Helper fHelper;
    SkSTArray<1, Mesh, true> fMeshes;
    // GrPrimitiveType is more expressive than fVertices.mode() so it is used instead and we ignore
    // the SkVertices mode (though fPrimitiveType may have been inferred from it).
    GrPrimitiveType fPrimitiveType;
    int fVertexCount;
    int fIndexCount;
    bool fMultipleViewMatrices;
    LocalCoordsType fLocalCoordsType;
    ColorArrayType fColorArrayType;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    typedef GrMeshDrawOp INHERITED;
};

DrawVerticesOp::DrawVerticesOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                               sk_sp<SkVertices> vertices, GrPrimitiveType primitiveType,
                               GrAAType aaType, sk_sp<GrColorSpaceXform> colorSpaceXform,
                               const SkMatrix& viewMatrix, const SkRuntimeEffect* effect)
        : INHERITED(ClassID())
        , fHelper(helperArgs, aaType)
        , fPrimitiveType(primitiveType)
        , fMultipleViewMatrices(false)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    SkASSERT(vertices);

    SkVerticesPriv info(vertices->priv());

    fVertexCount = info.vertexCount();
    fIndexCount = info.indexCount();
    fColorArrayType = info.hasColors() ? ColorArrayType::kSkColor
                                       : ColorArrayType::kUnused;
    fLocalCoordsType = info.hasTexCoords() ? LocalCoordsType::kExplicit
                                           : LocalCoordsType::kUsePosition;

    Mesh& mesh = fMeshes.push_back();
    mesh.fColor = color;
    mesh.fViewMatrix = viewMatrix;
    mesh.fVertices = std::move(vertices);
    mesh.fIgnoreColors = false;

    IsHairline zeroArea;
    if (GrIsPrimTypeLines(primitiveType) || GrPrimitiveType::kPoints == primitiveType) {
        zeroArea = IsHairline::kYes;
    } else {
        zeroArea = IsHairline::kNo;
    }

    this->setTransformedBounds(mesh.fVertices->bounds(),
                                mesh.fViewMatrix,
                                HasAABloat::kNo,
                                zeroArea);
}

#ifdef SK_DEBUG
SkString DrawVerticesOp::dumpInfo() const {
    SkString string;
    string.appendf("PrimType: %d, MeshCount %d, VCount: %d, ICount: %d\n", (int)fPrimitiveType,
                   fMeshes.count(), fVertexCount, fIndexCount);
    string += fHelper.dumpInfo();
    string += INHERITED::dumpInfo();
    return string;
}
#endif

GrDrawOp::FixedFunctionFlags DrawVerticesOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrProcessorSet::Analysis DrawVerticesOp::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
    GrProcessorAnalysisColor gpColor;
    if (this->requiresPerVertexColors()) {
        gpColor.setToUnknown();
    } else {
        gpColor.setToConstant(fMeshes.front().fColor);
    }
    auto result = fHelper.finalizeProcessors(caps, clip, hasMixedSampledCoverage, clampType,
                                             GrProcessorAnalysisCoverage::kNone, &gpColor);
    if (gpColor.isConstant(&fMeshes.front().fColor)) {
        fMeshes.front().fIgnoreColors = true;
        fColorArrayType = ColorArrayType::kUnused;
    }
    if (!fHelper.usesLocalCoords()) {
        fLocalCoordsType = LocalCoordsType::kUnused;
    }
    return result;
}

GrGeometryProcessor* DrawVerticesOp::makeGP(SkArenaAlloc* arena) {
    sk_sp<GrColorSpaceXform> csxform =
            fColorArrayType == ColorArrayType::kSkColor ? fColorSpaceXform : nullptr;

    const SkMatrix& vm = fMultipleViewMatrices ? SkMatrix::I() : fMeshes[0].fViewMatrix;

    SkVerticesPriv info(fMeshes[0].fVertices->priv());

    auto gp = VerticesGP::Make(arena, fLocalCoordsType, fColorArrayType, fMeshes[0].fColor,
                               std::move(csxform), vm, info.attributes(), info.attributeCount());
    SkASSERT(this->vertexStride() == gp->vertexStride());
    return gp;
}

void DrawVerticesOp::onCreateProgramInfo(const GrCaps* caps,
                                         SkArenaAlloc* arena,
                                         const GrSurfaceProxyView* outputView,
                                         GrAppliedClip&& appliedClip,
                                         const GrXferProcessor::DstProxyView& dstProxyView) {
    GrGeometryProcessor* gp = this->makeGP(arena);
    fProgramInfo = fHelper.createProgramInfo(caps, arena, outputView, std::move(appliedClip),
                                             dstProxyView, gp, this->primitiveType());
}

void DrawVerticesOp::onPrepareDraws(Target* target) {
    // Allocate buffers.
    size_t vertexStride = this->vertexStride();
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex = 0;
    GrVertexWriter verts{
            target->makeVertexSpace(vertexStride, fVertexCount, &vertexBuffer, &firstVertex)};
    if (!verts.fPtr) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    sk_sp<const GrBuffer> indexBuffer;
    int firstIndex = 0;
    uint16_t* indices = nullptr;
    if (this->isIndexed()) {
        indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
    }

    // Copy data into the buffers.
    bool hasColorAttribute = this->requiresPerVertexColors();
    bool hasLocalCoordsAttribute = this->requiresPerVertexLocalCoords();
    int vertexOffset = 0;

    for (const auto& mesh : fMeshes) {
        SkVerticesPriv info(mesh.fVertices->priv());

        // Copy data into the index buffer.
        if (indices) {
            int indexCount = info.indexCount();
            for (int i = 0; i < indexCount; ++i) {
                *indices++ = info.indices()[i] + vertexOffset;
            }
        }

        // Copy data into the vertex buffer.
        int vertexCount = info.vertexCount();
        const SkPoint* positions = info.positions();
        const SkColor* colors = info.colors();
        const SkPoint* localCoords = info.texCoords() ? info.texCoords() : positions;
        const void* custom = info.customData();
        size_t customDataSize = info.customDataSize();

        // TODO4F: Preserve float colors
        GrColor meshColor = mesh.fColor.toBytes_RGBA();

        SkPoint* posBase = (SkPoint*)verts.fPtr;

        for (int i = 0; i < vertexCount; ++i) {
            verts.write(positions[i]);
            if (hasColorAttribute) {
                verts.write(mesh.hasPerVertexColors() ? colors[i] : meshColor);
            }
            if (hasLocalCoordsAttribute) {
                verts.write(localCoords[i]);
            }
            if (customDataSize) {
                verts.writeRaw(custom, customDataSize);
                custom = SkTAddOffset<const void>(custom, customDataSize);
            }
        }

        if (fMultipleViewMatrices) {
            SkASSERT(!mesh.fViewMatrix.hasPerspective());
            SkMatrixPriv::MapPointsWithStride(mesh.fViewMatrix, posBase, vertexStride, vertexCount);
        }

        vertexOffset += vertexCount;
    }

    SkASSERT(!fMesh);
    fMesh = target->allocMesh();
    if (this->isIndexed()) {
        fMesh->setIndexed(std::move(indexBuffer), fIndexCount, firstIndex, 0, fVertexCount - 1,
                         GrPrimitiveRestart::kNo, std::move(vertexBuffer), firstVertex);
    } else {
        fMesh->set(std::move(vertexBuffer), fVertexCount, firstVertex);
    }
}

void DrawVerticesOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    if (!fProgramInfo || !fMesh) {
        return;
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

GrOp::CombineResult DrawVerticesOp::onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                                        const GrCaps& caps) {
    DrawVerticesOp* that = t->cast<DrawVerticesOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    if (!this->combinablePrimitive() || this->primitiveType() != that->primitiveType()) {
        return CombineResult::kCannotCombine;
    }

    if (this->isIndexed() != that->isIndexed()) {
        return CombineResult::kCannotCombine;
    }

    if (fVertexCount + that->fVertexCount > SkTo<int>(UINT16_MAX)) {
        return CombineResult::kCannotCombine;
    }

    SkVerticesPriv vThis(this->fMeshes[0].fVertices->priv()),
                   vThat(that->fMeshes[0].fVertices->priv());
    if (vThis.attributeCount() != vThat.attributeCount() ||
        !std::equal(vThis.attributes(), vThis.attributes() + vThis.attributeCount(),
                    vThat.attributes())) {
        return CombineResult::kCannotCombine;
    }

    // We can't mix draws that use SkColor vertex colors with those that don't. We can mix uniform
    // color draws with GrColor draws (by expanding the uniform color into vertex color).
    if ((fColorArrayType == ColorArrayType::kSkColor) !=
        (that->fColorArrayType == ColorArrayType::kSkColor)) {
        return CombineResult::kCannotCombine;
    }

    // If we're acquiring a mesh with a different view matrix, or an op that needed multiple view
    // matrices, we need multiple view matrices.
    bool needMultipleViewMatrices =
            fMultipleViewMatrices || that->fMultipleViewMatrices ||
            !SkMatrixPriv::CheapEqual(this->fMeshes[0].fViewMatrix, that->fMeshes[0].fViewMatrix);

    // ... but we can't enable multiple view matrices if any of them have perspective, or our other
    // varyings won't be interpolated correctly.
    if (needMultipleViewMatrices && (this->fMeshes[0].fViewMatrix.hasPerspective() ||
                                     that->fMeshes[0].fViewMatrix.hasPerspective())) {
        return CombineResult::kCannotCombine;
    } else {
        fMultipleViewMatrices = needMultipleViewMatrices;
    }

    // If the other op already required per-vertex colors, the combined mesh does.
    if (that->fColorArrayType == ColorArrayType::kPremulGrColor) {
        fColorArrayType = ColorArrayType::kPremulGrColor;
    }

    // If we combine meshes with different (uniform) colors, switch to per-vertex colors.
    if (fColorArrayType == ColorArrayType::kUnused) {
        SkASSERT(that->fColorArrayType == ColorArrayType::kUnused);
        if (this->fMeshes[0].fColor != that->fMeshes[0].fColor) {
            fColorArrayType = ColorArrayType::kPremulGrColor;
        }
    }

    // NOTE: For SkColor vertex colors, the source color space is always sRGB, and the destination
    // gamut is determined by the render target context. A mis-match should be impossible.
    SkASSERT(GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get()));

    // If the other op already required explicit local coords the combined mesh does.
    if (that->fLocalCoordsType == LocalCoordsType::kExplicit) {
        fLocalCoordsType = LocalCoordsType::kExplicit;
    }

    // If we were planning to use positions for local coords but now have multiple view matrices,
    // switch to explicit local coords.
    if (fLocalCoordsType == LocalCoordsType::kUsePosition && fMultipleViewMatrices) {
        fLocalCoordsType = LocalCoordsType::kExplicit;
    }

    fMeshes.push_back_n(that->fMeshes.count(), that->fMeshes.begin());
    fVertexCount += that->fVertexCount;
    fIndexCount += that->fIndexCount;

    return CombineResult::kMerged;
}

} // anonymous namespace

std::unique_ptr<GrDrawOp> GrDrawVerticesOp::Make(GrRecordingContext* context,
                                                 GrPaint&& paint,
                                                 sk_sp<SkVertices> vertices,
                                                 const SkMatrix& viewMatrix,
                                                 GrAAType aaType,
                                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                 GrPrimitiveType* overridePrimType,
                                                 const SkRuntimeEffect* effect) {
    SkASSERT(vertices);
    GrPrimitiveType primType = overridePrimType
                                       ? *overridePrimType
                                       : SkVertexModeToGrPrimitiveType(vertices->priv().mode());
    return GrSimpleMeshDrawOpHelper::FactoryHelper<DrawVerticesOp>(context, std::move(paint),
                                                                   std::move(vertices),
                                                                   primType, aaType,
                                                                   std::move(colorSpaceXform),
                                                                   viewMatrix, effect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "src/gpu/GrDrawOpTest.h"

static uint32_t seed_vertices(GrPrimitiveType type) {
    switch (type) {
        case GrPrimitiveType::kTriangles:
        case GrPrimitiveType::kTriangleStrip:
            return 3;
        case GrPrimitiveType::kPoints:
            return 1;
        case GrPrimitiveType::kLines:
        case GrPrimitiveType::kLineStrip:
            return 2;
        case GrPrimitiveType::kPatches:
        case GrPrimitiveType::kPath:
            SkASSERT(0);
            return 0;
    }
    SK_ABORT("Incomplete switch\n");
}

static uint32_t primitive_vertices(GrPrimitiveType type) {
    switch (type) {
        case GrPrimitiveType::kTriangles:
            return 3;
        case GrPrimitiveType::kLines:
            return 2;
        case GrPrimitiveType::kTriangleStrip:
        case GrPrimitiveType::kPoints:
        case GrPrimitiveType::kLineStrip:
            return 1;
        case GrPrimitiveType::kPatches:
        case GrPrimitiveType::kPath:
            SkASSERT(0);
            return 0;
    }
    SK_ABORT("Incomplete switch\n");
}

static SkPoint random_point(SkRandom* random, SkScalar min, SkScalar max) {
    SkPoint p;
    p.fX = random->nextRangeScalar(min, max);
    p.fY = random->nextRangeScalar(min, max);
    return p;
}

static void randomize_params(size_t count, size_t maxVertex, SkScalar min, SkScalar max,
                             SkRandom* random, SkTArray<SkPoint>* positions,
                             SkTArray<SkPoint>* texCoords, bool hasTexCoords,
                             SkTArray<uint32_t>* colors, bool hasColors,
                             SkTArray<uint16_t>* indices, bool hasIndices) {
    for (uint32_t v = 0; v < count; v++) {
        positions->push_back(random_point(random, min, max));
        if (hasTexCoords) {
            texCoords->push_back(random_point(random, min, max));
        }
        if (hasColors) {
            colors->push_back(GrRandomColor(random));
        }
        if (hasIndices) {
            SkASSERT(maxVertex <= UINT16_MAX);
            indices->push_back(random->nextULessThan((uint16_t)maxVertex));
        }
    }
}

GR_DRAW_OP_TEST_DEFINE(DrawVerticesOp) {
    GrPrimitiveType types[] = {
        GrPrimitiveType::kTriangles,
        GrPrimitiveType::kTriangleStrip,
        GrPrimitiveType::kPoints,
        GrPrimitiveType::kLines,
        GrPrimitiveType::kLineStrip
    };
    auto type = types[random->nextULessThan(SK_ARRAY_COUNT(types))];

    uint32_t primitiveCount = random->nextRangeU(1, 100);

    // TODO make 'sensible' indexbuffers
    SkTArray<SkPoint> positions;
    SkTArray<SkPoint> texCoords;
    SkTArray<uint32_t> colors;
    SkTArray<uint16_t> indices;

    bool hasTexCoords = random->nextBool();
    bool hasIndices = random->nextBool();
    bool hasColors = random->nextBool();

    uint32_t vertexCount = seed_vertices(type) + (primitiveCount - 1) * primitive_vertices(type);

    static const SkScalar kMinVertExtent = -100.f;
    static const SkScalar kMaxVertExtent = 100.f;
    randomize_params(seed_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent, random,
                     &positions, &texCoords, hasTexCoords, &colors, hasColors, &indices,
                     hasIndices);

    for (uint32_t i = 1; i < primitiveCount; i++) {
        randomize_params(primitive_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent,
                         random, &positions, &texCoords, hasTexCoords, &colors, hasColors, &indices,
                         hasIndices);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);

    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(random);

    static constexpr SkVertices::VertexMode kIgnoredMode = SkVertices::kTriangles_VertexMode;
    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(kIgnoredMode, vertexCount, positions.begin(),
                                                      texCoords.begin(), colors.begin(),
                                                      hasIndices ? indices.count() : 0,
                                                      indices.begin());
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1 && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return GrDrawVerticesOp::Make(context, std::move(paint), std::move(vertices),
                                  viewMatrix, aaType, std::move(colorSpaceXform), &type);
}

#endif
