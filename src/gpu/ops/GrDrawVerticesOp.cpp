/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrDrawVerticesOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

class DrawVerticesOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    DrawVerticesOp(const Helper::MakeArgs&, const SkPMColor4f&, sk_sp<SkVertices>,
                   GrPrimitiveType, GrAAType, sk_sp<GrColorSpaceXform>, const SkMatrix& viewMatrix);

    const char* name() const override { return "DrawVerticesOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitProxies(func);
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
    enum class ColorArrayType {
        kPremulGrColor,
        kSkColor,
    };

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
        bool fIgnoreTexCoords;
        bool fIgnoreColors;

        bool hasExplicitLocalCoords() const {
            return fVertices->hasTexCoords() && !fIgnoreTexCoords;
        }

        bool hasPerVertexColors() const {
            return fVertices->hasColors() && !fIgnoreColors;
        }
    };

    bool isIndexed() const {
        // Consistency enforced in onCombineIfPossible.
        return fMeshes[0].fVertices->hasIndices();
    }

    bool requiresPerVertexColors() const {
        return SkToBool(kRequiresPerVertexColors_Flag & fFlags);
    }

    bool anyMeshHasExplicitLocalCoords() const {
        return SkToBool(kAnyMeshHasExplicitLocalCoords_Flag & fFlags);
    }

    GrDefaultGeoProcFactory::LocalCoords::Type localCoordsType() const {
        if (fHelper.usesLocalCoords()) {
            // If we have multiple view matrices we will transform the positions into device space.
            // We must then also provide untransformed positions as local coords.
            if (this->anyMeshHasExplicitLocalCoords() || this->hasMultipleViewMatrices()) {
                return GrDefaultGeoProcFactory::LocalCoords::kHasExplicit_Type;
            } else {
                return GrDefaultGeoProcFactory::LocalCoords::kUsePosition_Type;
            }
        } else {
            return GrDefaultGeoProcFactory::LocalCoords::kUnused_Type;
        }
    }

    bool requiresPerVertexLocalCoords() const {
        return this->localCoordsType() == GrDefaultGeoProcFactory::LocalCoords::kHasExplicit_Type;
    }

    bool hasMultipleViewMatrices() const {
        return SkToBool(kHasMultipleViewMatrices_Flag & fFlags);
    }

    size_t vertexStride() const {
        return sizeof(SkPoint) +
               (this->requiresPerVertexColors() ? sizeof(uint32_t) : 0) +
               (this->requiresPerVertexLocalCoords() ? sizeof(SkPoint) : 0);
    }

    enum Flags {
        kRequiresPerVertexColors_Flag       = 0x1,
        kAnyMeshHasExplicitLocalCoords_Flag = 0x2,
        kHasMultipleViewMatrices_Flag       = 0x4,
    };

    Helper fHelper;
    SkSTArray<1, Mesh, true> fMeshes;
    // GrPrimitiveType is more expressive than fVertices.mode() so it is used instead and we ignore
    // the SkVertices mode (though fPrimitiveType may have been inferred from it).
    GrPrimitiveType fPrimitiveType;
    uint32_t fFlags;
    int fVertexCount;
    int fIndexCount;
    ColorArrayType fColorArrayType;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    GrMesh*        fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    typedef GrMeshDrawOp INHERITED;
};

DrawVerticesOp::DrawVerticesOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                               sk_sp<SkVertices> vertices, GrPrimitiveType primitiveType,
                               GrAAType aaType, sk_sp<GrColorSpaceXform> colorSpaceXform,
                               const SkMatrix& viewMatrix)
        : INHERITED(ClassID())
        , fHelper(helperArgs, aaType)
        , fPrimitiveType(primitiveType)
        , fColorSpaceXform(std::move(colorSpaceXform)) {
    SkASSERT(vertices);

    fVertexCount = vertices->vertexCount();
    fIndexCount = vertices->indexCount();
    fColorArrayType = vertices->hasColors() ? ColorArrayType::kSkColor
                                            : ColorArrayType::kPremulGrColor;

    Mesh& mesh = fMeshes.push_back();
    mesh.fColor = color;
    mesh.fViewMatrix = viewMatrix;
    mesh.fVertices = std::move(vertices);
    mesh.fIgnoreTexCoords = false;
    mesh.fIgnoreColors = false;

    fFlags = 0;
    if (mesh.hasPerVertexColors()) {
        fFlags |= kRequiresPerVertexColors_Flag;
    }
    if (mesh.hasExplicitLocalCoords()) {
        fFlags |= kAnyMeshHasExplicitLocalCoords_Flag;
    }

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
        fFlags &= ~kRequiresPerVertexColors_Flag;
        fColorArrayType = ColorArrayType::kPremulGrColor;
    }
    if (!fHelper.usesLocalCoords()) {
        fMeshes[0].fIgnoreTexCoords = true;
        fFlags &= ~kAnyMeshHasExplicitLocalCoords_Flag;
    }
    return result;
}

GrGeometryProcessor* DrawVerticesOp::makeGP(SkArenaAlloc* arena) {
    using namespace GrDefaultGeoProcFactory;

    Color color(fMeshes[0].fColor);
    if (this->requiresPerVertexColors()) {
        if (fColorArrayType == ColorArrayType::kPremulGrColor) {
            color.fType = Color::kPremulGrColorAttribute_Type;
        } else {
            color.fType = Color::kUnpremulSkColorAttribute_Type;
            color.fColorSpaceXform = fColorSpaceXform;
        }
    }

    const SkMatrix& vm = this->hasMultipleViewMatrices() ? SkMatrix::I() : fMeshes[0].fViewMatrix;

    auto gp = GrDefaultGeoProcFactory::Make(arena, color, Coverage::kSolid_Type,
                                            this->localCoordsType(), vm);
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
        // Copy data into the index buffer.
        if (indices) {
            int indexCount = mesh.fVertices->indexCount();
            for (int i = 0; i < indexCount; ++i) {
                *indices++ = mesh.fVertices->indices()[i] + vertexOffset;
            }
        }

        // Copy data into the vertex buffer.
        int vertexCount = mesh.fVertices->vertexCount();
        const SkPoint* positions = mesh.fVertices->positions();
        const SkColor* colors = mesh.fVertices->colors();
        const SkPoint* localCoords =
                mesh.hasExplicitLocalCoords() ? mesh.fVertices->texCoords() : positions;

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
        }

        if (this->hasMultipleViewMatrices()) {
            SkMatrixPriv::MapPointsWithStride(mesh.fViewMatrix, posBase, vertexStride, vertexCount);
        }

        vertexOffset += vertexCount;
    }

    SkASSERT(!fMesh);
    fMesh = target->allocMesh();
    if (this->isIndexed()) {
        fMesh->setIndexed(std::move(indexBuffer), fIndexCount, firstIndex, 0, fVertexCount - 1,
                         GrPrimitiveRestart::kNo);
    } else {
        fMesh->setNonIndexedNonInstanced(fVertexCount);
    }
    fMesh->setVertexData(std::move(vertexBuffer), firstVertex);
}

void DrawVerticesOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    if (!fProgramInfo || !fMesh) {
        return;
    }

    flushState->opsRenderPass()->bindPipeline(*fProgramInfo, chainBounds);
    flushState->opsRenderPass()->drawMeshes(*fProgramInfo, fMesh, 1);
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

    if (fColorArrayType != that->fColorArrayType) {
        return CombineResult::kCannotCombine;
    }

    if (fVertexCount + that->fVertexCount > SkTo<int>(UINT16_MAX)) {
        return CombineResult::kCannotCombine;
    }

    // NOTE: For SkColor vertex colors, the source color space is always sRGB, and the destination
    // gamut is determined by the render target context. A mis-match should be impossible.
    SkASSERT(GrColorSpaceXform::Equals(fColorSpaceXform.get(), that->fColorSpaceXform.get()));

    // If either op required explicit local coords or per-vertex colors the combined mesh does.
    // Same with multiple view matrices.
    fFlags |= that->fFlags;

    if (this->fMeshes[0].fColor != that->fMeshes[0].fColor) {
        fFlags |= kRequiresPerVertexColors_Flag;
    }
    // Check whether we are about to acquire a mesh with a different view matrix.
    if (!SkMatrixPriv::CheapEqual(this->fMeshes[0].fViewMatrix, that->fMeshes[0].fViewMatrix)) {
        fFlags |= kHasMultipleViewMatrices_Flag;
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
                                                 GrPrimitiveType* overridePrimType) {
    SkASSERT(vertices);
    GrPrimitiveType primType = overridePrimType ? *overridePrimType
                                                : SkVertexModeToGrPrimitiveType(vertices->mode());
    return GrSimpleMeshDrawOpHelper::FactoryHelper<DrawVerticesOp>(context, std::move(paint),
                                                                   std::move(vertices),
                                                                   primType, aaType,
                                                                   std::move(colorSpaceXform),
                                                                   viewMatrix);
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
