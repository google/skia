/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawAtlasBatch.h"
#include "GrBatchTest.h"
#include "SkRandom.h"

void GrDrawAtlasBatch::initBatchTracker(const GrPipelineInfo& init) {
    // Handle any color overrides
    if (!init.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    init.getOverrideColorIfSet(&fGeoData[0].fColor);
    
    // setup batch properties
    fColorIgnored = !init.readsColor();
    fColor = fGeoData[0].fColor;
    SkASSERT(init.readsLocalCoords());
    fCoverageIgnored = !init.readsCoverage();
}

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

void GrDrawAtlasBatch::generateGeometry(GrBatchTarget* batchTarget) {
    int colorOffset = -1, texOffset = -1;
    // Setup geometry processor
    SkAutoTUnref<const GrGeometryProcessor> gp(
                                set_vertex_attributes(true, this->hasColors(), &colorOffset,
                                                      &texOffset, this->color(), this->viewMatrix(),
                                                      this->coverageIgnored()));
    
    batchTarget->initDraw(gp, this->pipeline());
    
    int instanceCount = fGeoData.count();
    size_t vertexStride = gp->getVertexStride();
    SkASSERT(vertexStride == sizeof(SkPoint) + sizeof(SkPoint)
             + (this->hasColors() ? sizeof(GrColor) : 0));
    
    QuadHelper helper;
    int numQuads = this->vertexCount()/4;
    void* verts = helper.init(batchTarget, vertexStride, numQuads);
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }
    
    int vertexOffset = 0;
    for (int i = 0; i < instanceCount; i++) {
        const Geometry& args = fGeoData[i];
        
        for (int j = 0; j < args.fPositions.count(); ++j) {
            *((SkPoint*)verts) = args.fPositions[j];
            if (this->hasColors()) {
                *(GrColor*)((intptr_t)verts + colorOffset) = args.fColors[j];
            }
            *(SkPoint*)((intptr_t)verts + texOffset) = args.fLocalCoords[j];
            verts = (void*)((intptr_t)verts + vertexStride);
            vertexOffset++;
        }
    }
    helper.issueDraw(batchTarget);
}

GrDrawAtlasBatch::GrDrawAtlasBatch(const Geometry& geometry, const SkMatrix& viewMatrix,
                                   const SkPoint* positions, int vertexCount,
                                   const GrColor* colors, const SkPoint* localCoords,
                                   const SkRect& bounds) {
    this->initClassID<GrDrawAtlasBatch>();
    SkASSERT(positions);
    SkASSERT(localCoords);
    
    fViewMatrix = viewMatrix;
    Geometry& installedGeo = fGeoData.push_back(geometry);
    
    installedGeo.fPositions.append(vertexCount, positions);
    
    if (colors) {
        installedGeo.fColors.append(vertexCount, colors);
        fHasColors = true;
    } else {
        fHasColors = false;
    }
    
    installedGeo.fLocalCoords.append(vertexCount, localCoords);
    fVertexCount = vertexCount;
    
    this->setBounds(bounds);
}
 
bool GrDrawAtlasBatch::onCombineIfPossible(GrBatch* t) {
    if (!this->pipeline()->isEqual(*t->pipeline())) {
        return false;
    }
    
    GrDrawAtlasBatch* that = t->cast<GrDrawAtlasBatch>();
    
    // We currently use a uniform viewmatrix for this batch
    if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }
    
    if (this->hasColors() != that->hasColors()) {
        return false;
    }
    
    if (!this->hasColors() && this->color() != that->color()) {
        return false;
    }
    
    if (this->color() != that->color()) {
        fColor = GrColor_ILLEGAL;
    }
    fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
    fVertexCount += that->vertexCount();
    
    this->joinBounds(that->bounds());
    return true;
}

#ifdef GR_TEST_UTILS

static SkPoint random_point(SkRandom* random, SkScalar min, SkScalar max) {
    SkPoint p;
    p.fX = random->nextRangeScalar(min, max);
    p.fY = random->nextRangeScalar(min, max);
    return p;
}

static void randomize_params(size_t count, SkScalar min, SkScalar max,
                             SkRandom* random,
                             SkTArray<SkPoint>* positions,
                             SkTArray<SkPoint>* texCoords,
                             SkTArray<GrColor>* colors, bool hasColors) {
    for (uint32_t v = 0; v < count; v++) {
        positions->push_back(random_point(random, min, max));
        texCoords->push_back(random_point(random, min, max));
        if (hasColors) {
            colors->push_back(GrRandomColor(random));
        }
    }
}


BATCH_TEST_DEFINE(GrDrawAtlasBatch) {
    uint32_t spriteCount = random->nextRangeU(1, 100);
    
    // TODO make 'sensible' indexbuffers
    SkTArray<SkPoint> positions;
    SkTArray<SkPoint> texCoords;
    SkTArray<GrColor> colors;
    
    bool hasColors = random->nextBool();
    
    uint32_t vertexCount = 4*spriteCount;
    
    static const SkScalar kMinVertExtent = -100.f;
    static const SkScalar kMaxVertExtent = 100.f;
    randomize_params(vertexCount, kMinVertExtent, kMaxVertExtent,
                     random,
                     &positions,
                     &texCoords,
                     &colors, hasColors);
    
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect bounds;
    SkDEBUGCODE(bool result = ) bounds.setBoundsCheck(positions.begin(), vertexCount);
    SkASSERT(result);
    
    viewMatrix.mapRect(&bounds);
    
    GrDrawAtlasBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    return GrDrawAtlasBatch::Create(geometry, viewMatrix,
                                    positions.begin(), vertexCount,
                                    colors.begin(),
                                    texCoords.begin(),
                                    bounds);
}

#endif
