/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawVerticesOp.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrInvariantOutput.h"
#include "GrOpFlushState.h"

static sk_sp<GrGeometryProcessor> make_gp(bool clientProvidedLocalCoords,
                                          bool pipelineReadsLocalCoords,
                                          GrRenderTargetContext::ColorArrayType colorArrayType,
                                          bool multipleViewMatrices,
                                          const SkMatrix& viewMatrixIfCommon,
                                          bool* hasLocalCoordAttribute) {
    using namespace GrDefaultGeoProcFactory;
    LocalCoords::Type localCoordsType;
    if (pipelineReadsLocalCoords) {
        if (clientProvidedLocalCoords || multipleViewMatrices) {
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

    Color::Type colorType =
            (colorArrayType == GrRenderTargetContext::ColorArrayType::kPremulGrColor)
                    ? Color::kPremulGrColorAttribute_Type
                    : Color::kUnpremulSkColorAttribute_Type;
    const SkMatrix& vm = multipleViewMatrices ? SkMatrix::I() : viewMatrixIfCommon;
    return GrDefaultGeoProcFactory::Make(colorType, Coverage::kSolid_Type, localCoordsType, vm);
}

GrDrawVerticesOp::GrDrawVerticesOp(GrColor color, GrPrimitiveType primitiveType,
                                   const SkMatrix& viewMatrix, const SkPoint* positions,
                                   int vertexCount, const uint16_t* indices, int indexCount,
                                   const uint32_t* colors, const SkPoint* localCoords,
                                   const SkRect& bounds,
                                   GrRenderTargetContext::ColorArrayType colorArrayType)
        : INHERITED(ClassID()) {
    SkASSERT(positions);

    Mesh& mesh = fMeshes.push_back();
    mesh.fColor = color;
    mesh.fViewMatrix = viewMatrix;

    mesh.fPositions.append(vertexCount, positions);
    if (indices) {
        mesh.fIndices.append(indexCount, indices);
    }

    if (colors) {
        fVariableColor = true;
        mesh.fColors.append(vertexCount, colors);
        fColorArrayType = colorArrayType;
    } else {
        fVariableColor = false;
        // When we tessellate we will fill a color array with the GrColor value passed above as
        // 'color'.
        fColorArrayType = GrRenderTargetContext::ColorArrayType::kPremulGrColor;
    }

    if (localCoords) {
        mesh.fLocalCoords.append(vertexCount, localCoords);
    }
    fVertexCount = vertexCount;
    fIndexCount = indexCount;
    fPrimitiveType = primitiveType;

    IsZeroArea zeroArea;
    if (GrIsPrimTypeLines(primitiveType) || kPoints_GrPrimitiveType == primitiveType) {
        zeroArea = IsZeroArea::kYes;
    } else {
        zeroArea = IsZeroArea::kNo;
    }
    this->setBounds(bounds, HasAABloat::kNo, zeroArea);
}

void GrDrawVerticesOp::getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput* input) const {
    if (fVariableColor) {
        input->pipelineColorInput()->setUnknownFourComponents();
    } else {
        input->pipelineColorInput()->setKnownFourComponents(fMeshes[0].fColor);
    }
    input->pipelineCoverageInput()->setKnownSingleComponent(0xff);
}

void GrDrawVerticesOp::applyPipelineOptimizations(const GrPipelineOptimizations& optimizations) {
    SkASSERT(fMeshes.count() == 1);
    GrColor overrideColor;
    if (optimizations.getOverrideColorIfSet(&overrideColor)) {
        fMeshes[0].fColor = overrideColor;
        fMeshes[0].fColors.reset();
        fVariableColor = false;
        fColorArrayType = GrRenderTargetContext::ColorArrayType::kPremulGrColor;
    }
    if (!(fPipelineNeedsLocalCoords = optimizations.readsLocalCoords())) {
        fMeshes[0].fLocalCoords.reset();
    }
}

void GrDrawVerticesOp::onPrepareDraws(Target* target) const {
    bool clientLocalCoords = !fMeshes[0].fLocalCoords.isEmpty();
    bool hasLocalCoordAttribute;
    sk_sp<GrGeometryProcessor> gp =
            make_gp(clientLocalCoords, fPipelineNeedsLocalCoords, fColorArrayType,
                    fMultipleViewMatrices, fMeshes[0].fViewMatrix, &hasLocalCoordAttribute);
    size_t vertexStride = gp->getVertexStride();

    SkASSERT(vertexStride ==
             sizeof(SkPoint) + (hasLocalCoordAttribute ? sizeof(SkPoint) : 0) + sizeof(uint32_t));

    int instanceCount = fMeshes.count();

    const GrBuffer* vertexBuffer;
    int firstVertex;

    void* verts = target->makeVertexSpace(vertexStride, fVertexCount, &vertexBuffer, &firstVertex);

    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    const GrBuffer* indexBuffer = nullptr;
    int firstIndex = 0;

    uint16_t* indices = nullptr;
    if (!fMeshes[0].fIndices.isEmpty()) {
        indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);

        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
    }

    int indexOffset = 0;
    int vertexOffset = 0;
    for (int i = 0; i < instanceCount; i++) {
        const Mesh& mesh = fMeshes[i];
        // Currently we require all meshes to either have explicit local coords or not, though it
        // wouldn't be hard to allow them to mix.
        SkASSERT(clientLocalCoords == !mesh.fLocalCoords.isEmpty());
        if (indices) {
            for (int j = 0; j < mesh.fIndices.count(); ++j, ++indexOffset) {
                *(indices + indexOffset) = mesh.fIndices[j] + vertexOffset;
            }
        }

        static constexpr size_t kColorOffset = sizeof(SkPoint);
        static constexpr size_t kLocalCoordOffset = kColorOffset + sizeof(uint32_t);

        for (int j = 0; j < mesh.fPositions.count(); ++j) {
            if (fMultipleViewMatrices) {
                mesh.fViewMatrix.mapPoints(((SkPoint*)verts), &mesh.fPositions[j], 1);
            } else {
                *((SkPoint*)verts) = mesh.fPositions[j];
            }
            if (mesh.fColors.isEmpty()) {
                *(uint32_t*)((intptr_t)verts + kColorOffset) = mesh.fColor;
            } else {
                *(uint32_t*)((intptr_t)verts + kColorOffset) = mesh.fColors[j];
            }
            if (hasLocalCoordAttribute) {
                if (clientLocalCoords) {
                    *(SkPoint*)((intptr_t)verts + kLocalCoordOffset) = mesh.fLocalCoords[j];
                } else {
                    *(SkPoint*)((intptr_t)verts + kLocalCoordOffset) = mesh.fPositions[j];
                }
            }
            verts = (void*)((intptr_t)verts + vertexStride);
            vertexOffset++;
        }
    }

    GrMesh mesh;
    if (indices) {
        mesh.initIndexed(this->primitiveType(), vertexBuffer, indexBuffer, firstVertex, firstIndex,
                         fVertexCount, fIndexCount);

    } else {
        mesh.init(this->primitiveType(), vertexBuffer, firstVertex, fVertexCount);
    }
    target->draw(gp.get(), mesh);
}

bool GrDrawVerticesOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrDrawVerticesOp* that = t->cast<GrDrawVerticesOp>();

    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

    if (!this->combinablePrimitive() || this->primitiveType() != that->primitiveType()) {
        return false;
    }


    if (fMeshes[0].fIndices.isEmpty() != that->fMeshes[0].fIndices.isEmpty()) {
        return false;
    }

    // This could be relaxed by using positions for the one that doesn't already have explicit
    // local coordindates.
    if (fMeshes[0].fLocalCoords.isEmpty() != that->fMeshes[0].fLocalCoords.isEmpty()) {
        return false;
    }

    if (fColorArrayType != that->fColorArrayType) {
        return false;
    }

    if (fIndexCount + that->fIndexCount > SK_MaxU16) {
        return false;
    }

    if (!fVariableColor) {
        if (that->fVariableColor || that->fMeshes[0].fColor != fMeshes[0].fColor) {
            fVariableColor = true;
        }
    }

    // Check whether we are about to acquire a mesh with a different view matrix.
    if (!fMultipleViewMatrices) {
        if (that->fMultipleViewMatrices ||
            !fMeshes[0].fViewMatrix.cheapEqualTo(that->fMeshes[0].fViewMatrix)) {
            fMultipleViewMatrices = true;
        }
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
        case kTriangles_GrPrimitiveType:
        case kTriangleStrip_GrPrimitiveType:
        case kTriangleFan_GrPrimitiveType:
            return 3;
        case kPoints_GrPrimitiveType:
            return 1;
        case kLines_GrPrimitiveType:
        case kLineStrip_GrPrimitiveType:
            return 2;
    }
    SkFAIL("Incomplete switch\n");
    return 0;
}

static uint32_t primitive_vertices(GrPrimitiveType type) {
    switch (type) {
        case kTriangles_GrPrimitiveType:
            return 3;
        case kLines_GrPrimitiveType:
            return 2;
        case kTriangleStrip_GrPrimitiveType:
        case kTriangleFan_GrPrimitiveType:
        case kPoints_GrPrimitiveType:
        case kLineStrip_GrPrimitiveType:
            return 1;
    }
    SkFAIL("Incomplete switch\n");
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
            SkASSERT(maxVertex <= SK_MaxU16);
            indices->push_back(random->nextULessThan((uint16_t)maxVertex));
        }
    }
}

DRAW_OP_TEST_DEFINE(VerticesOp) {
    GrPrimitiveType type = GrPrimitiveType(random->nextULessThan(kLast_GrPrimitiveType + 1));
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

    GrRenderTargetContext::ColorArrayType colorArrayType =
            random->nextBool() ? GrRenderTargetContext::ColorArrayType::kPremulGrColor
                               : GrRenderTargetContext::ColorArrayType::kSkColor;
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect bounds;
    SkDEBUGCODE(bool result =) bounds.setBoundsCheck(positions.begin(), vertexCount);
    SkASSERT(result);

    viewMatrix.mapRect(&bounds);

    GrColor color = GrRandomColor(random);
    return GrDrawVerticesOp::Make(color, type, viewMatrix, positions.begin(), vertexCount,
                                  indices.begin(), hasIndices ? vertexCount : 0, colors.begin(),
                                  texCoords.begin(), bounds, colorArrayType);
}

#endif
