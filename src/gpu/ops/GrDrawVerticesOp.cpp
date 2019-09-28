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
                   const SkVertices::Bone bones[], int boneCount, GrPrimitiveType, GrAAType,
                   sk_sp<GrColorSpaceXform>, const SkMatrix& viewMatrix);

    const char* name() const override { return "DrawVerticesOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
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

    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    void drawVolatile(Target*);
    void drawNonVolatile(Target*);

    void fillBuffers(bool hasColorAttribute,
                     bool hasLocalCoordsAttribute,
                     size_t vertexStride,
                     void* verts,
                     uint16_t* indices) const;

    void drawVertices(Target*,
                      sk_sp<const GrGeometryProcessor>,
                      sk_sp<const GrBuffer> vertexBuffer,
                      int firstVertex,
                      sk_sp<const GrBuffer> indexBuffer,
                      int firstIndex);

    sk_sp<GrGeometryProcessor> makeGP(const GrShaderCaps* shaderCaps,
                                      bool* hasColorAttribute,
                                      bool* hasLocalCoordAttribute) const;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool combinablePrimitive() const {
        return GrPrimitiveType::kTriangles == fPrimitiveType ||
               GrPrimitiveType::kLines == fPrimitiveType ||
               GrPrimitiveType::kPoints == fPrimitiveType;
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps&) override;

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

    bool hasMultipleViewMatrices() const {
        return SkToBool(kHasMultipleViewMatrices_Flag & fFlags);
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

    typedef GrMeshDrawOp INHERITED;
};

DrawVerticesOp::DrawVerticesOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                               sk_sp<SkVertices> vertices, const SkVertices::Bone bones[],
                               int boneCount, GrPrimitiveType primitiveType, GrAAType aaType,
                               sk_sp<GrColorSpaceXform> colorSpaceXform,
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

    if (mesh.fVertices->hasBones() && bones) {
        // Perform the transformations on the CPU instead of the GPU.
        mesh.fVertices = mesh.fVertices->applyBones(bones, boneCount);
    } else {
        SkASSERT(!bones || boneCount == 1);
    }

    fFlags = 0;
    if (mesh.hasPerVertexColors()) {
        fFlags |= kRequiresPerVertexColors_Flag;
    }
    if (mesh.hasExplicitLocalCoords()) {
        fFlags |= kAnyMeshHasExplicitLocalCoords_Flag;
    }

    // Special case for meshes with a world transform but no bone weights.
    // These will be considered normal vertices draws without bones.
    if (!mesh.fVertices->hasBones() && boneCount == 1) {
        SkMatrix worldTransform;
        worldTransform.setAffine(bones[0].values);
        mesh.fViewMatrix.preConcat(worldTransform);
    }

    IsZeroArea zeroArea;
    if (GrIsPrimTypeLines(primitiveType) || GrPrimitiveType::kPoints == primitiveType) {
        zeroArea = IsZeroArea::kYes;
    } else {
        zeroArea = IsZeroArea::kNo;
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

sk_sp<GrGeometryProcessor> DrawVerticesOp::makeGP(const GrShaderCaps* shaderCaps,
                                                  bool* hasColorAttribute,
                                                  bool* hasLocalCoordAttribute) const {
    using namespace GrDefaultGeoProcFactory;
    LocalCoords::Type localCoordsType;
    if (fHelper.usesLocalCoords()) {
        // If we have multiple view matrices we will transform the positions into device space. We
        // must then also provide untransformed positions as local coords.
        if (this->anyMeshHasExplicitLocalCoords() || this->hasMultipleViewMatrices()) {
            *hasLocalCoordAttribute = true;
            localCoordsType = LocalCoords::kHasExplicit_Type;
        } else {
            *hasLocalCoordAttribute = false;
            localCoordsType = LocalCoords::kUsePosition_Type;
        }
    } else {
        localCoordsType = LocalCoords::kUnused_Type;
        *hasLocalCoordAttribute = false;
    }

    Color color(fMeshes[0].fColor);
    if (this->requiresPerVertexColors()) {
        if (fColorArrayType == ColorArrayType::kPremulGrColor) {
            color.fType = Color::kPremulGrColorAttribute_Type;
        } else {
            color.fType = Color::kUnpremulSkColorAttribute_Type;
            color.fColorSpaceXform = fColorSpaceXform;
        }
        *hasColorAttribute = true;
    } else {
        *hasColorAttribute = false;
    }

    const SkMatrix& vm = this->hasMultipleViewMatrices() ? SkMatrix::I() : fMeshes[0].fViewMatrix;

    return GrDefaultGeoProcFactory::Make(shaderCaps,
                                            color,
                                            Coverage::kSolid_Type,
                                            localCoordsType,
                                            vm);
}

void DrawVerticesOp::onPrepareDraws(Target* target) {
    bool hasMapBufferSupport = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
    if (fMeshes[0].fVertices->isVolatile() || !hasMapBufferSupport) {
        this->drawVolatile(target);
    } else {
        this->drawNonVolatile(target);
    }
}

void DrawVerticesOp::drawVolatile(Target* target) {
    bool hasColorAttribute;
    bool hasLocalCoordsAttribute;
    sk_sp<GrGeometryProcessor> gp = this->makeGP(target->caps().shaderCaps(),
                                                 &hasColorAttribute,
                                                 &hasLocalCoordsAttribute);

    // Allocate buffers.
    size_t vertexStride = gp->vertexStride();
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex = 0;
    void* verts = target->makeVertexSpace(vertexStride, fVertexCount, &vertexBuffer, &firstVertex);
    if (!verts) {
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

    // Fill the buffers.
    this->fillBuffers(hasColorAttribute,
                      hasLocalCoordsAttribute,
                      vertexStride,
                      verts,
                      indices);

    // Draw the vertices.
    this->drawVertices(target, std::move(gp), std::move(vertexBuffer), firstVertex, indexBuffer,
                       firstIndex);
}

void DrawVerticesOp::drawNonVolatile(Target* target) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    bool hasColorAttribute;
    bool hasLocalCoordsAttribute;
    sk_sp<GrGeometryProcessor> gp = this->makeGP(target->caps().shaderCaps(),
                                                 &hasColorAttribute,
                                                 &hasLocalCoordsAttribute);

    SkASSERT(fMeshes.count() == 1); // Non-volatile meshes should never combine.

    // Get the resource provider.
    GrResourceProvider* rp = target->resourceProvider();

    // Generate keys for the buffers.
    GrUniqueKey vertexKey, indexKey;
    GrUniqueKey::Builder vertexKeyBuilder(&vertexKey, kDomain, 2);
    GrUniqueKey::Builder indexKeyBuilder(&indexKey, kDomain, 2);
    vertexKeyBuilder[0] = indexKeyBuilder[0] = fMeshes[0].fVertices->uniqueID();
    vertexKeyBuilder[1] = 0;
    indexKeyBuilder[1] = 1;
    vertexKeyBuilder.finish();
    indexKeyBuilder.finish();

    // Try to grab data from the cache.
    sk_sp<GrGpuBuffer> vertexBuffer = rp->findByUniqueKey<GrGpuBuffer>(vertexKey);
    sk_sp<GrGpuBuffer> indexBuffer =
            this->isIndexed() ? rp->findByUniqueKey<GrGpuBuffer>(indexKey) : nullptr;

    // Draw using the cached buffers if possible.
    if (vertexBuffer && (!this->isIndexed() || indexBuffer)) {
        this->drawVertices(target, std::move(gp), std::move(vertexBuffer), 0,
                           std::move(indexBuffer), 0);
        return;
    }

    // Allocate vertex buffer.
    size_t vertexStride = gp->vertexStride();
    vertexBuffer = rp->createBuffer(
            fVertexCount * vertexStride, GrGpuBufferType::kVertex, kStatic_GrAccessPattern);
    void* verts = vertexBuffer ? vertexBuffer->map() : nullptr;
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    // Allocate index buffer.
    uint16_t* indices = nullptr;
    if (this->isIndexed()) {
        indexBuffer = rp->createBuffer(
                fIndexCount * sizeof(uint16_t), GrGpuBufferType::kIndex, kStatic_GrAccessPattern);
        indices = indexBuffer ? static_cast<uint16_t*>(indexBuffer->map()) : nullptr;
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
    }

    // Fill the buffers.
    this->fillBuffers(hasColorAttribute,
                      hasLocalCoordsAttribute,
                      vertexStride,
                      verts,
                      indices);

    // Unmap the buffers.
    vertexBuffer->unmap();
    if (indexBuffer) {
        indexBuffer->unmap();
    }

    // Cache the buffers.
    rp->assignUniqueKeyToResource(vertexKey, vertexBuffer.get());
    rp->assignUniqueKeyToResource(indexKey, indexBuffer.get());

    // Draw the vertices.
    this->drawVertices(target, std::move(gp), std::move(vertexBuffer), 0, std::move(indexBuffer),
                       0);
}

void DrawVerticesOp::fillBuffers(bool hasColorAttribute,
                                 bool hasLocalCoordsAttribute,
                                 size_t vertexStride,
                                 void* verts,
                                 uint16_t* indices) const {
    int instanceCount = fMeshes.count();

    // Copy data into the buffers.
    int vertexOffset = 0;
    // We have a fast case below for uploading the vertex data when the matrix is translate
    // only and there are colors but not local coords.
    bool fastAttrs = hasColorAttribute && !hasLocalCoordsAttribute;
    for (int i = 0; i < instanceCount; i++) {
        // Get each mesh.
        const Mesh& mesh = fMeshes[i];

        // Copy data into the index buffer.
        if (indices) {
            int indexCount = mesh.fVertices->indexCount();
            for (int j = 0; j < indexCount; ++j) {
                *indices++ = mesh.fVertices->indices()[j] + vertexOffset;
            }
        }

        // Copy data into the vertex buffer.
        int vertexCount = mesh.fVertices->vertexCount();
        const SkPoint* positions = mesh.fVertices->positions();
        const SkColor* colors = mesh.fVertices->colors();
        const SkPoint* localCoords = mesh.fVertices->texCoords();
        bool fastMesh = (!this->hasMultipleViewMatrices() ||
                         mesh.fViewMatrix.getType() <= SkMatrix::kTranslate_Mask) &&
                        mesh.hasPerVertexColors();
        if (fastAttrs && fastMesh) {
            // Fast case.
            struct V {
                SkPoint fPos;
                uint32_t fColor;
            };
            SkASSERT(sizeof(V) == vertexStride);
            V* v = (V*)verts;
            Sk2f t(0, 0);
            if (this->hasMultipleViewMatrices()) {
                t = Sk2f(mesh.fViewMatrix.getTranslateX(), mesh.fViewMatrix.getTranslateY());
            }
            for (int j = 0; j < vertexCount; ++j) {
                Sk2f p = Sk2f::Load(positions++) + t;
                p.store(&v[j].fPos);
                v[j].fColor = colors[j];
            }
            verts = v + vertexCount;
        } else {
            // Normal case.
            static constexpr size_t kColorOffset = sizeof(SkPoint);
            size_t offset = kColorOffset;
            if (hasColorAttribute) {
                offset += sizeof(uint32_t);
            }
            size_t localCoordOffset = offset;
            if (hasLocalCoordsAttribute) {
                offset += sizeof(SkPoint);
            }

            // TODO4F: Preserve float colors
            GrColor color = mesh.fColor.toBytes_RGBA();

            for (int j = 0; j < vertexCount; ++j) {
                if (this->hasMultipleViewMatrices()) {
                    mesh.fViewMatrix.mapPoints(((SkPoint*)verts), &positions[j], 1);
                } else {
                    *((SkPoint*)verts) = positions[j];
                }
                if (hasColorAttribute) {
                    if (mesh.hasPerVertexColors()) {
                        *(uint32_t*)((intptr_t)verts + kColorOffset) = colors[j];
                    } else {
                        *(uint32_t*)((intptr_t)verts + kColorOffset) = color;
                    }
                }
                if (hasLocalCoordsAttribute) {
                    if (mesh.hasExplicitLocalCoords()) {
                        *(SkPoint*)((intptr_t)verts + localCoordOffset) = localCoords[j];
                    } else {
                        *(SkPoint*)((intptr_t)verts + localCoordOffset) = positions[j];
                    }
                }
                verts = (void*)((intptr_t)verts + vertexStride);
            }
        }
        vertexOffset += vertexCount;
    }
}

void DrawVerticesOp::drawVertices(Target* target,
                                  sk_sp<const GrGeometryProcessor> gp,
                                  sk_sp<const GrBuffer> vertexBuffer,
                                  int firstVertex,
                                  sk_sp<const GrBuffer> indexBuffer,
                                  int firstIndex) {
    GrMesh* mesh = target->allocMesh(this->primitiveType());
    if (this->isIndexed()) {
        mesh->setIndexed(std::move(indexBuffer), fIndexCount, firstIndex, 0, fVertexCount - 1,
                         GrPrimitiveRestart::kNo);
    } else {
        mesh->setNonIndexedNonInstanced(fVertexCount);
    }
    mesh->setVertexData(std::move(vertexBuffer), firstVertex);
    target->recordDraw(std::move(gp), mesh);
}

void DrawVerticesOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
}

GrOp::CombineResult DrawVerticesOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    DrawVerticesOp* that = t->cast<DrawVerticesOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // Non-volatile meshes cannot batch, because if a non-volatile mesh batches with another mesh,
    // then on the next frame, if that non-volatile mesh is drawn, it will draw the other mesh
    // that was saved in its vertex buffer, which is not necessarily there anymore.
    if (!this->fMeshes[0].fVertices->isVolatile() || !that->fMeshes[0].fVertices->isVolatile()) {
        return CombineResult::kCannotCombine;
    }

    if (!this->combinablePrimitive() || this->primitiveType() != that->primitiveType()) {
        return CombineResult::kCannotCombine;
    }

    if (fMeshes[0].fVertices->hasIndices() != that->fMeshes[0].fVertices->hasIndices()) {
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

    // If either op required explicit local coords or per-vertex colors the combined mesh does. Same
    // with multiple view matrices.
    fFlags |= that->fFlags;

    if (!this->requiresPerVertexColors() && this->fMeshes[0].fColor != that->fMeshes[0].fColor) {
        fFlags |= kRequiresPerVertexColors_Flag;
    }
    // Check whether we are about to acquire a mesh with a different view matrix.
    if (!this->hasMultipleViewMatrices() &&
        !this->fMeshes[0].fViewMatrix.cheapEqualTo(that->fMeshes[0].fViewMatrix)) {
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
                                                 const SkVertices::Bone bones[],
                                                 int boneCount,
                                                 const SkMatrix& viewMatrix,
                                                 GrAAType aaType,
                                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                 GrPrimitiveType* overridePrimType) {
    SkASSERT(vertices);
    GrPrimitiveType primType = overridePrimType ? *overridePrimType
                                                : SkVertexModeToGrPrimitiveType(vertices->mode());
    return GrSimpleMeshDrawOpHelper::FactoryHelper<DrawVerticesOp>(context, std::move(paint),
                                                                   std::move(vertices),
                                                                   bones, boneCount,
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
        case GrPrimitiveType::kLinesAdjacency:
            return 4;
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
        case GrPrimitiveType::kLinesAdjacency:
            return 4;
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
    GrPrimitiveType type;
    do {
       type = GrPrimitiveType(random->nextULessThan(kNumGrPrimitiveTypes));
    } while (GrPrimTypeRequiresGeometryShaderSupport(type) &&
             !context->priv().caps()->shaderCaps()->geometryShaderSupport());

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
    return GrDrawVerticesOp::Make(context, std::move(paint), std::move(vertices), nullptr, 0,
                                  viewMatrix, aaType, std::move(colorSpaceXform), &type);
}

#endif
