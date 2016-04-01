/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawVerticesBatch.h"

#include "GrBatchFlushState.h"
#include "GrInvariantOutput.h"
#include "GrDefaultGeoProcFactory.h"

static const GrGeometryProcessor* set_vertex_attributes(bool hasLocalCoords,
                                                        int* colorOffset,
                                                        int* texOffset,
                                                        const SkMatrix& viewMatrix,
                                                        bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;
    *texOffset = -1;
    *colorOffset = -1;

    Coverage coverage(coverageIgnored ? Coverage::kNone_Type : Coverage::kSolid_Type);
    LocalCoords localCoords(hasLocalCoords ? LocalCoords::kHasExplicit_Type :
                                             LocalCoords::kUsePosition_Type);
    *colorOffset = sizeof(SkPoint);
    if (hasLocalCoords) {
        *texOffset = sizeof(SkPoint) + sizeof(GrColor);
    }
    return GrDefaultGeoProcFactory::Create(Color(Color::kAttribute_Type),
                                           coverage, localCoords, viewMatrix);
}

GrDrawVerticesBatch::GrDrawVerticesBatch(const Geometry& geometry, GrPrimitiveType primitiveType,
                                         const SkMatrix& viewMatrix,
                                         const SkPoint* positions, int vertexCount,
                                         const uint16_t* indices, int indexCount,
                                         const GrColor* colors, const SkPoint* localCoords,
                                         const SkRect& bounds)
    : INHERITED(ClassID()) {
    SkASSERT(positions);

    fViewMatrix = viewMatrix;
    Geometry& installedGeo = fGeoData.push_back(geometry);

    installedGeo.fPositions.append(vertexCount, positions);
    if (indices) {
        installedGeo.fIndices.append(indexCount, indices);
    }

    if (colors) {
        fVariableColor = true;
        installedGeo.fColors.append(vertexCount, colors);
    } else {
        fVariableColor = false;
    }

    if (localCoords) {
        installedGeo.fLocalCoords.append(vertexCount, localCoords);
    }
    fVertexCount = vertexCount;
    fIndexCount = indexCount;
    fPrimitiveType = primitiveType;

    this->setBounds(bounds);
}

void GrDrawVerticesBatch::computePipelineOptimizations(GrInitInvariantOutput* color,
                                                       GrInitInvariantOutput* coverage,
                                                       GrBatchToXPOverrides* overrides) const {
    // When this is called on a batch, there is only one geometry bundle
    if (fVariableColor) {
        color->setUnknownFourComponents();
    } else {
        color->setKnownFourComponents(fGeoData[0].fColor);
    }
    coverage->setKnownSingleComponent(0xff);
}

void GrDrawVerticesBatch::initBatchTracker(const GrXPOverridesForBatch& overrides) {
    SkASSERT(fGeoData.count() == 1);
    GrColor overrideColor;
    if (overrides.getOverrideColorIfSet(&overrideColor)) {
        fGeoData[0].fColor = overrideColor;
        fGeoData[0].fColors.reset();
        fVariableColor = false;
    }
    fCoverageIgnored = !overrides.readsCoverage();
    if (!overrides.readsLocalCoords()) {
        fGeoData[0].fLocalCoords.reset();
    }
}

void GrDrawVerticesBatch::onPrepareDraws(Target* target) const {
    bool hasLocalCoords = !fGeoData[0].fLocalCoords.isEmpty();
    int colorOffset = -1, texOffset = -1;
    SkAutoTUnref<const GrGeometryProcessor> gp(
        set_vertex_attributes(hasLocalCoords, &colorOffset, &texOffset, fViewMatrix,
                              fCoverageIgnored));
    size_t vertexStride = gp->getVertexStride();

    SkASSERT(vertexStride == sizeof(SkPoint) + (hasLocalCoords ? sizeof(SkPoint) : 0)
                                             + sizeof(GrColor));

    int instanceCount = fGeoData.count();

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
    if (!fGeoData[0].fIndices.isEmpty()) {
        indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);

        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
    }

    int indexOffset = 0;
    int vertexOffset = 0;
    for (int i = 0; i < instanceCount; i++) {
        const Geometry& args = fGeoData[i];

        // TODO we can actually cache this interleaved and then just memcopy
        if (indices) {
            for (int j = 0; j < args.fIndices.count(); ++j, ++indexOffset) {
                *(indices + indexOffset) = args.fIndices[j] + vertexOffset;
            }
        }

        for (int j = 0; j < args.fPositions.count(); ++j) {
            *((SkPoint*)verts) = args.fPositions[j];
            if (args.fColors.isEmpty()) {
                *(GrColor*)((intptr_t)verts + colorOffset) = args.fColor;
            } else {
                *(GrColor*)((intptr_t)verts + colorOffset) = args.fColors[j];
            }
            if (hasLocalCoords) {
                *(SkPoint*)((intptr_t)verts + texOffset) = args.fLocalCoords[j];
            }
            verts = (void*)((intptr_t)verts + vertexStride);
            vertexOffset++;
        }
    }

    GrMesh mesh;
    if (indices) {
        mesh.initIndexed(this->primitiveType(), vertexBuffer, indexBuffer, firstVertex,
                         firstIndex, fVertexCount, fIndexCount);

    } else {
        mesh.init(this->primitiveType(), vertexBuffer, firstVertex, fVertexCount);
    }
    target->draw(gp, mesh);
}

bool GrDrawVerticesBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrDrawVerticesBatch* that = t->cast<GrDrawVerticesBatch>();

    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

    if (!this->batchablePrimitiveType() || this->primitiveType() != that->primitiveType()) {
        return false;
    }

    // We currently use a uniform viewmatrix for this batch
    if (!fViewMatrix.cheapEqualTo(that->fViewMatrix)) {
        return false;
    }

    if (fGeoData[0].fIndices.isEmpty() != that->fGeoData[0].fIndices.isEmpty()) {
        return false;
    }

    if (fGeoData[0].fLocalCoords.isEmpty() != that->fGeoData[0].fLocalCoords.isEmpty()) {
        return false;
    }

    if (!fVariableColor) {
        if (that->fVariableColor || that->fGeoData[0].fColor != fGeoData[0].fColor) {
            fVariableColor = true;
        }
    }

    fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
    fVertexCount += that->fVertexCount;
    fIndexCount += that->fIndexCount;

    this->joinBounds(that->bounds());
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

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
                             SkRandom* random,
                             SkTArray<SkPoint>* positions,
                             SkTArray<SkPoint>* texCoords, bool hasTexCoords,
                             SkTArray<GrColor>* colors, bool hasColors,
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

DRAW_BATCH_TEST_DEFINE(VerticesBatch) {
    GrPrimitiveType type = GrPrimitiveType(random->nextULessThan(kLast_GrPrimitiveType + 1));
    uint32_t primitiveCount = random->nextRangeU(1, 100);

    // TODO make 'sensible' indexbuffers
    SkTArray<SkPoint> positions;
    SkTArray<SkPoint> texCoords;
    SkTArray<GrColor> colors;
    SkTArray<uint16_t> indices;

    bool hasTexCoords = random->nextBool();
    bool hasIndices = random->nextBool();
    bool hasColors = random->nextBool();

    uint32_t vertexCount = seed_vertices(type) + (primitiveCount - 1) * primitive_vertices(type);

    static const SkScalar kMinVertExtent = -100.f;
    static const SkScalar kMaxVertExtent = 100.f;
    randomize_params(seed_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent,
                     random,
                     &positions,
                     &texCoords, hasTexCoords,
                     &colors, hasColors,
                     &indices, hasIndices);

    for (uint32_t i = 1; i < primitiveCount; i++) {
        randomize_params(primitive_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent,
                         random,
                         &positions,
                         &texCoords, hasTexCoords,
                         &colors, hasColors,
                         &indices, hasIndices);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect bounds;
    SkDEBUGCODE(bool result = ) bounds.setBoundsCheck(positions.begin(), vertexCount);
    SkASSERT(result);

    viewMatrix.mapRect(&bounds);

    GrDrawVerticesBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    return GrDrawVerticesBatch::Create(geometry, type, viewMatrix,
                                       positions.begin(), vertexCount,
                                       indices.begin(), hasIndices ? vertexCount : 0,
                                       colors.begin(),
                                       texCoords.begin(),
                                       bounds);
}

#endif
