/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawVerticesOp.h"
#include "GrCaps.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrOpFlushState.h"
#include "SkGr.h"
#include "SkRectPriv.h"

static constexpr int kNumFloatsPerSkMatrix = 9;

std::unique_ptr<GrDrawOp> GrDrawVerticesOp::Make(GrContext* context,
                                                 GrPaint&& paint,
                                                 sk_sp<SkVertices> vertices,
                                                 const SkMatrix bones[],
                                                 int boneCount,
                                                 const SkMatrix& viewMatrix,
                                                 GrAAType aaType,
                                                 sk_sp<GrColorSpaceXform> colorSpaceXform,
                                                 GrPrimitiveType* overridePrimType) {
    SkASSERT(vertices);
    GrPrimitiveType primType = overridePrimType ? *overridePrimType
                                                : SkVertexModeToGrPrimitiveType(vertices->mode());
    return Helper::FactoryHelper<GrDrawVerticesOp>(context, std::move(paint), std::move(vertices),
                                                   bones, boneCount, primType, aaType,
                                                   std::move(colorSpaceXform), viewMatrix);
}

GrDrawVerticesOp::GrDrawVerticesOp(const Helper::MakeArgs& helperArgs, GrColor color,
                                   sk_sp<SkVertices> vertices, const SkMatrix bones[],
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
    if (bones) {
        // Copy the bone data over in the format that the GPU would upload.
        mesh.fBones.reserve(boneCount * kNumFloatsPerSkMatrix);
        for (int i = 0; i < boneCount; i ++) {
            const SkMatrix& matrix = bones[i];
            mesh.fBones.push_back(matrix.get(SkMatrix::kMScaleX));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMSkewY));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMPersp0));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMSkewX));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMScaleY));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMPersp1));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMTransX));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMTransY));
            mesh.fBones.push_back(matrix.get(SkMatrix::kMPersp2));
        }
    }
    mesh.fIgnoreTexCoords = false;
    mesh.fIgnoreColors = false;
    mesh.fIgnoreBones = false;

    fFlags = 0;
    if (mesh.hasPerVertexColors()) {
        fFlags |= kRequiresPerVertexColors_Flag;
    }
    if (mesh.hasExplicitLocalCoords()) {
        fFlags |= kAnyMeshHasExplicitLocalCoords_Flag;
    }
    if (mesh.hasBones()) {
        fFlags |= kHasBones_Flag;
    }

    // Special case for meshes with a world transform but no bone weights.
    // These will be considered normal vertices draws without bones.
    if (!mesh.fVertices->hasBones() && boneCount == 1) {
        mesh.fViewMatrix.preConcat(bones[0]);
    }

    IsZeroArea zeroArea;
    if (GrIsPrimTypeLines(primitiveType) || GrPrimitiveType::kPoints == primitiveType) {
        zeroArea = IsZeroArea::kYes;
    } else {
        zeroArea = IsZeroArea::kNo;
    }

    if (this->hasBones()) {
        // We don't know the bounds if there are deformations involved, so attempt to calculate
        // the maximum possible.
        SkRect bounds = SkRect::MakeEmpty();
        const SkRect originalBounds = bones[0].mapRect(mesh.fVertices->bounds());
        for (int i = 1; i < boneCount; i++) {
            const SkMatrix& matrix = bones[i];
            bounds.join(matrix.mapRect(originalBounds));
        }

        this->setTransformedBounds(bounds,
                                   mesh.fViewMatrix,
                                   HasAABloat::kNo,
                                   zeroArea);
    } else {
        this->setTransformedBounds(mesh.fVertices->bounds(),
                                   mesh.fViewMatrix,
                                   HasAABloat::kNo,
                                   zeroArea);
    }
}

SkString GrDrawVerticesOp::dumpInfo() const {
    SkString string;
    string.appendf("PrimType: %d, MeshCount %d, VCount: %d, ICount: %d\n", (int)fPrimitiveType,
                   fMeshes.count(), fVertexCount, fIndexCount);
    string += fHelper.dumpInfo();
    string += INHERITED::dumpInfo();
    return string;
}

GrDrawOp::FixedFunctionFlags GrDrawVerticesOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrDrawOp::RequiresDstTexture GrDrawVerticesOp::finalize(const GrCaps& caps,
                                                        const GrAppliedClip* clip) {
    GrProcessorAnalysisColor gpColor;
    if (this->requiresPerVertexColors()) {
        gpColor.setToUnknown();
    } else {
        gpColor.setToConstant(fMeshes.front().fColor);
    }
    auto result = fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone,
                                               &gpColor);
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

sk_sp<GrGeometryProcessor> GrDrawVerticesOp::makeGP(const GrShaderCaps* shaderCaps,
                                                    bool* hasColorAttribute,
                                                    bool* hasLocalCoordAttribute,
                                                    bool* hasBoneAttribute) const {
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
    };

    const SkMatrix& vm = this->hasMultipleViewMatrices() ? SkMatrix::I() : fMeshes[0].fViewMatrix;

    Bones bones(fMeshes[0].fBones.data(), fMeshes[0].fBones.size() / kNumFloatsPerSkMatrix);
    *hasBoneAttribute = this->hasBones();

    if (this->hasBones()) {
        return GrDefaultGeoProcFactory::MakeWithBones(shaderCaps,
                                                      color,
                                                      Coverage::kSolid_Type,
                                                      localCoordsType,
                                                      bones,
                                                      vm);
    } else {
        return GrDefaultGeoProcFactory::Make(shaderCaps,
                                             color,
                                             Coverage::kSolid_Type,
                                             localCoordsType,
                                             vm);
    }
}

void GrDrawVerticesOp::onPrepareDraws(Target* target) {
    bool hasMapBufferSupport = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
    if (fMeshes[0].fVertices->isVolatile() || !hasMapBufferSupport) {
        this->drawVolatile(target);
    } else {
        this->drawNonVolatile(target);
    }
}

void GrDrawVerticesOp::drawVolatile(Target* target) {
    bool hasColorAttribute;
    bool hasLocalCoordsAttribute;
    bool hasBoneAttribute;
    sk_sp<GrGeometryProcessor> gp = this->makeGP(target->caps().shaderCaps(),
                                                 &hasColorAttribute,
                                                 &hasLocalCoordsAttribute,
                                                 &hasBoneAttribute);

    // Calculate the stride.
    size_t vertexStride = sizeof(SkPoint) +
                          (hasColorAttribute ? sizeof(uint32_t) : 0) +
                          (hasLocalCoordsAttribute ? sizeof(SkPoint) : 0) +
                          (hasBoneAttribute ? 4 * (sizeof(int8_t) + sizeof(uint8_t)) : 0);
    SkASSERT(vertexStride == gp->debugOnly_vertexStride());

    // Allocate buffers.
    const GrBuffer* vertexBuffer = nullptr;
    int firstVertex = 0;
    void* verts = target->makeVertexSpace(vertexStride, fVertexCount, &vertexBuffer, &firstVertex);
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    const GrBuffer* indexBuffer = nullptr;
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
                      hasBoneAttribute,
                      vertexStride,
                      verts,
                      indices);

    // Draw the vertices.
    this->drawVertices(target, gp.get(), vertexBuffer, firstVertex, indexBuffer, firstIndex);
}

void GrDrawVerticesOp::drawNonVolatile(Target* target) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    bool hasColorAttribute;
    bool hasLocalCoordsAttribute;
    bool hasBoneAttribute;
    sk_sp<GrGeometryProcessor> gp = this->makeGP(target->caps().shaderCaps(),
                                                 &hasColorAttribute,
                                                 &hasLocalCoordsAttribute,
                                                 &hasBoneAttribute);

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
    sk_sp<GrBuffer> vertexBuffer = rp->findByUniqueKey<GrBuffer>(vertexKey);
    sk_sp<GrBuffer> indexBuffer = this->isIndexed() ?
            rp->findByUniqueKey<GrBuffer>(indexKey) :
            nullptr;

    // Draw using the cached buffers if possible.
    if (vertexBuffer && (!this->isIndexed() || indexBuffer)) {
        this->drawVertices(target, gp.get(), vertexBuffer.get(), 0, indexBuffer.get(), 0);
        return;
    }

    // Calculate the stride.
    size_t vertexStride = sizeof(SkPoint) +
                          (hasColorAttribute ? sizeof(uint32_t) : 0) +
                          (hasLocalCoordsAttribute ? sizeof(SkPoint) : 0) +
                          (hasBoneAttribute ? 4 * (sizeof(int8_t) + sizeof(uint8_t)) : 0);
    SkASSERT(vertexStride == gp->debugOnly_vertexStride());

    // Allocate vertex buffer.
    vertexBuffer.reset(rp->createBuffer(fVertexCount * vertexStride,
                                        kVertex_GrBufferType,
                                        kStatic_GrAccessPattern,
                                        0));
    void* verts = vertexBuffer ? vertexBuffer->map() : nullptr;
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    // Allocate index buffer.
    uint16_t* indices = nullptr;
    if (this->isIndexed()) {
        indexBuffer.reset(rp->createBuffer(fIndexCount * sizeof(uint16_t),
                                           kIndex_GrBufferType,
                                           kStatic_GrAccessPattern,
                                           0));
        indices = indexBuffer ? static_cast<uint16_t*>(indexBuffer->map()) : nullptr;
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
    }

    // Fill the buffers.
    this->fillBuffers(hasColorAttribute,
                      hasLocalCoordsAttribute,
                      hasBoneAttribute,
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
    this->drawVertices(target, gp.get(), vertexBuffer.get(), 0, indexBuffer.get(), 0);
}

void GrDrawVerticesOp::fillBuffers(bool hasColorAttribute,
                                   bool hasLocalCoordsAttribute,
                                   bool hasBoneAttribute,
                                   size_t vertexStride,
                                   void* verts,
                                   uint16_t* indices) const {
    int instanceCount = fMeshes.count();

    // Copy data into the buffers.
    int vertexOffset = 0;
    // We have a fast case below for uploading the vertex data when the matrix is translate
    // only and there are colors but not local coords. Fast case does not apply when there are bone
    // transformations.
    bool fastAttrs = hasColorAttribute && !hasLocalCoordsAttribute && !hasBoneAttribute;
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
        const SkVertices::BoneIndices* boneIndices = mesh.fVertices->boneIndices();
        const SkVertices::BoneWeights* boneWeights = mesh.fVertices->boneWeights();
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
            size_t boneIndexOffset = offset;
            if (hasBoneAttribute) {
                offset += 4 * sizeof(int8_t);
            }
            size_t boneWeightOffset = offset;

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
                        *(uint32_t*)((intptr_t)verts + kColorOffset) = mesh.fColor;
                    }
                }
                if (hasLocalCoordsAttribute) {
                    if (mesh.hasExplicitLocalCoords()) {
                        *(SkPoint*)((intptr_t)verts + localCoordOffset) = localCoords[j];
                    } else {
                        *(SkPoint*)((intptr_t)verts + localCoordOffset) = positions[j];
                    }
                }
                if (hasBoneAttribute) {
                    const SkVertices::BoneIndices& indices = boneIndices[j];
                    const SkVertices::BoneWeights& weights = boneWeights[j];
                    for (int k = 0; k < 4; k++) {
                        size_t indexOffset = boneIndexOffset + sizeof(int8_t) * k;
                        size_t weightOffset = boneWeightOffset + sizeof(uint8_t) * k;
                        *(int8_t*)((intptr_t)verts + indexOffset) = indices.indices[k];
                        *(uint8_t*)((intptr_t)verts + weightOffset) = weights.weights[k] * 255.0f;
                    }
                }
                verts = (void*)((intptr_t)verts + vertexStride);
            }
        }
        vertexOffset += vertexCount;
    }
}

void GrDrawVerticesOp::drawVertices(Target* target,
                                    GrGeometryProcessor* gp,
                                    const GrBuffer* vertexBuffer,
                                    int firstVertex,
                                    const GrBuffer* indexBuffer,
                                    int firstIndex) {
    GrMesh mesh(this->primitiveType());
    if (this->isIndexed()) {
        mesh.setIndexed(indexBuffer, fIndexCount,
                        firstIndex, 0, fVertexCount - 1,
                        GrPrimitiveRestart::kNo);
    } else {
        mesh.setNonIndexedNonInstanced(fVertexCount);
    }
    mesh.setVertexData(vertexBuffer, firstVertex);
    auto pipe = fHelper.makePipeline(target);
    target->draw(gp, pipe.fPipeline, pipe.fFixedDynamicState, mesh);
}

bool GrDrawVerticesOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrDrawVerticesOp* that = t->cast<GrDrawVerticesOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return false;
    }

    // Meshes with bones cannot be combined because different meshes use different bones, so to
    // combine them, the matrices would have to be combined, and the bone indices on each vertex
    // would change, thus making the vertices uncacheable.
    if (this->hasBones() || that->hasBones()) {
        return false;
    }

    // Non-volatile meshes cannot batch, because if a non-volatile mesh batches with another mesh,
    // then on the next frame, if that non-volatile mesh is drawn, it will draw the other mesh
    // that was saved in its vertex buffer, which is not necessarily there anymore.
    if (!this->fMeshes[0].fVertices->isVolatile() || !that->fMeshes[0].fVertices->isVolatile()) {
        return false;
    }

    if (!this->combinablePrimitive() || this->primitiveType() != that->primitiveType()) {
        return false;
    }

    if (fMeshes[0].fVertices->hasIndices() != that->fMeshes[0].fVertices->hasIndices()) {
        return false;
    }

    if (fColorArrayType != that->fColorArrayType) {
        return false;
    }

    if (fVertexCount + that->fVertexCount > SkTo<int>(UINT16_MAX)) {
        return false;
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

    this->joinBounds(*that);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrDrawOpTest.h"

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
    return 0;
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
    return 0;
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

GR_DRAW_OP_TEST_DEFINE(GrDrawVerticesOp) {
    GrPrimitiveType type;
    do {
       type = GrPrimitiveType(random->nextULessThan(kNumGrPrimitiveTypes));
    } while (GrPrimTypeRequiresGeometryShaderSupport(type) &&
             !context->contextPriv().caps()->shaderCaps()->geometryShaderSupport());

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
    if (GrFSAAType::kUnifiedMSAA == fsaaType && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return GrDrawVerticesOp::Make(context, std::move(paint), std::move(vertices), nullptr, 0,
                                  viewMatrix, aaType, std::move(colorSpaceXform), &type);
}

#endif
