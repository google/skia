/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawVerticesBatch.h"

#include "GrBatchTarget.h"
#include "GrInvariantOutput.h"
#include "GrDefaultGeoProcFactory.h"

static const GrGeometryProcessor* set_vertex_attributes(bool hasLocalCoords,
                                                        bool hasColors,
                                                        int* colorOffset,
                                                        int* texOffset,
                                                        GrColor color,
                                                        const SkMatrix& viewMatrix,
                                                        bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;
    *texOffset = -1;
    *colorOffset = -1;
    Color gpColor(color);
    if (hasColors) {
        gpColor.fType = Color::kAttribute_Type;
    }

    Coverage coverage(coverageIgnored ? Coverage::kNone_Type : Coverage::kSolid_Type);
    LocalCoords localCoords(hasLocalCoords ? LocalCoords::kHasExplicit_Type :
                                             LocalCoords::kUsePosition_Type);
    if (hasLocalCoords && hasColors) {
        *colorOffset = sizeof(SkPoint);
        *texOffset = sizeof(SkPoint) + sizeof(GrColor);
    } else if (hasLocalCoords) {
        *texOffset = sizeof(SkPoint);
    } else if (hasColors) {
        *colorOffset = sizeof(SkPoint);
    }
    return GrDefaultGeoProcFactory::Create(gpColor, coverage, localCoords, viewMatrix);
}

GrDrawVerticesBatch::GrDrawVerticesBatch(const Geometry& geometry, GrPrimitiveType primitiveType,
                                         const SkMatrix& viewMatrix,
                                         const SkPoint* positions, int vertexCount,
                                         const uint16_t* indices, int indexCount,
                                         const GrColor* colors, const SkPoint* localCoords,
                                         const SkRect& bounds) {
    this->initClassID<GrDrawVerticesBatch>();
    SkASSERT(positions);

    fBatch.fViewMatrix = viewMatrix;
    Geometry& installedGeo = fGeoData.push_back(geometry);

    installedGeo.fPositions.append(vertexCount, positions);
    if (indices) {
        installedGeo.fIndices.append(indexCount, indices);
        fBatch.fHasIndices = true;
    } else {
        fBatch.fHasIndices = false;
    }

    if (colors) {
        installedGeo.fColors.append(vertexCount, colors);
        fBatch.fHasColors = true;
    } else {
        fBatch.fHasColors = false;
    }

    if (localCoords) {
        installedGeo.fLocalCoords.append(vertexCount, localCoords);
        fBatch.fHasLocalCoords = true;
    } else {
        fBatch.fHasLocalCoords = false;
    }
    fBatch.fVertexCount = vertexCount;
    fBatch.fIndexCount = indexCount;
    fBatch.fPrimitiveType = primitiveType;

    this->setBounds(bounds);
}

void GrDrawVerticesBatch::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    // When this is called on a batch, there is only one geometry bundle
    if (this->hasColors()) {
        out->setUnknownFourComponents();
    } else {
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
}

void GrDrawVerticesBatch::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    out->setKnownSingleComponent(0xff);
}

void GrDrawVerticesBatch::initBatchTracker(const GrPipelineInfo& init) {
    // Handle any color overrides
    if (!init.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    init.getOverrideColorIfSet(&fGeoData[0].fColor);

    // setup batch properties
    fBatch.fColorIgnored = !init.readsColor();
    fBatch.fColor = fGeoData[0].fColor;
    fBatch.fUsesLocalCoords = init.readsLocalCoords();
    fBatch.fCoverageIgnored = !init.readsCoverage();
}

void GrDrawVerticesBatch::generateGeometry(GrBatchTarget* batchTarget) {
    int colorOffset = -1, texOffset = -1;
    SkAutoTUnref<const GrGeometryProcessor> gp(
            set_vertex_attributes(this->hasLocalCoords(), this->hasColors(), &colorOffset,
                                  &texOffset, this->color(), this->viewMatrix(),
                                  this->coverageIgnored()));

    batchTarget->initDraw(gp, this->pipeline());

    size_t vertexStride = gp->getVertexStride();

    SkASSERT(vertexStride == sizeof(SkPoint) + (this->hasLocalCoords() ? sizeof(SkPoint) : 0)
                                             + (this->hasColors() ? sizeof(GrColor) : 0));

    int instanceCount = fGeoData.count();

    const GrVertexBuffer* vertexBuffer;
    int firstVertex;

    void* verts = batchTarget->makeVertSpace(vertexStride, this->vertexCount(),
                                             &vertexBuffer, &firstVertex);

    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    const GrIndexBuffer* indexBuffer = NULL;
    int firstIndex = 0;

    uint16_t* indices = NULL;
    if (this->hasIndices()) {
        indices = batchTarget->makeIndexSpace(this->indexCount(), &indexBuffer, &firstIndex);

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
        if (this->hasIndices()) {
            for (int j = 0; j < args.fIndices.count(); ++j, ++indexOffset) {
                *(indices + indexOffset) = args.fIndices[j] + vertexOffset;
            }
        }

        for (int j = 0; j < args.fPositions.count(); ++j) {
            *((SkPoint*)verts) = args.fPositions[j];
            if (this->hasColors()) {
                *(GrColor*)((intptr_t)verts + colorOffset) = args.fColors[j];
            }
            if (this->hasLocalCoords()) {
                *(SkPoint*)((intptr_t)verts + texOffset) = args.fLocalCoords[j];
            }
            verts = (void*)((intptr_t)verts + vertexStride);
            vertexOffset++;
        }
    }

    GrVertices vertices;
    if (this->hasIndices()) {
        vertices.initIndexed(this->primitiveType(), vertexBuffer, indexBuffer, firstVertex,
                             firstIndex, this->vertexCount(), this->indexCount());

    } else {
        vertices.init(this->primitiveType(), vertexBuffer, firstVertex, this->vertexCount());
    }
    batchTarget->draw(vertices);
}

bool GrDrawVerticesBatch::onCombineIfPossible(GrBatch* t) {
    if (!this->pipeline()->isEqual(*t->pipeline())) {
        return false;
    }

    GrDrawVerticesBatch* that = t->cast<GrDrawVerticesBatch>();

    if (!this->batchablePrimitiveType() || this->primitiveType() != that->primitiveType()) {
        return false;
    }

    SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());

    // We currently use a uniform viewmatrix for this batch
    if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }

    if (this->hasColors() != that->hasColors()) {
        return false;
    }

    if (this->hasIndices() != that->hasIndices()) {
        return false;
    }

    if (this->hasLocalCoords() != that->hasLocalCoords()) {
        return false;
    }

    if (!this->hasColors() && this->color() != that->color()) {
        return false;
    }

    if (this->color() != that->color()) {
        fBatch.fColor = GrColor_ILLEGAL;
    }
    fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
    fBatch.fVertexCount += that->vertexCount();
    fBatch.fIndexCount += that->indexCount();

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

BATCH_TEST_DEFINE(VerticesBatch) {
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
