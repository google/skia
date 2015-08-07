
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAALinearizingConvexPathRenderer.h"

#include "GrAAConvexTessellator.h"
#include "GrBatchTarget.h"
#include "GrBatchTest.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrPipelineBuilder.h"
#include "GrStrokeInfo.h"
#include "SkGeometry.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "SkPathPriv.h"
#include "batches/GrBatch.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

static const int DEFAULT_BUFFER_SIZE = 100;

// The thicker the stroke, the harder it is to produce high-quality results using tessellation. For
// the time being, we simply drop back to software rendering above this stroke width.
static const SkScalar kMaxStrokeWidth = 20.0;

GrAALinearizingConvexPathRenderer::GrAALinearizingConvexPathRenderer() {
}

///////////////////////////////////////////////////////////////////////////////

bool GrAALinearizingConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fAntiAlias) {
        return false;
    }
    if (args.fPath->isInverseFillType()) {
        return false;
    }
    if (!args.fPath->isConvex()) {
        return false;
    }
    if (args.fStroke->getStyle() == SkStrokeRec::kStroke_Style) {
        return args.fViewMatrix->isSimilarity() && args.fStroke->getWidth() >= 1.0f && 
                args.fStroke->getWidth() <= kMaxStrokeWidth && !args.fStroke->isDashed() && 
                SkPathPriv::LastVerbIsClose(*args.fPath) &&
                args.fStroke->getJoin() != SkPaint::Join::kRound_Join;
    }
    return args.fStroke->getStyle() == SkStrokeRec::kFill_Style;
}

// extract the result vertices and indices from the GrAAConvexTessellator
static void extract_verts(const GrAAConvexTessellator& tess,
                          void* vertices,
                          size_t vertexStride,
                          GrColor color,
                          uint16_t firstIndex,
                          uint16_t* idxs,
                          bool tweakAlphaForCoverage) {
    intptr_t verts = reinterpret_cast<intptr_t>(vertices);

    for (int i = 0; i < tess.numPts(); ++i) {
        *((SkPoint*)((intptr_t)verts + i * vertexStride)) = tess.point(i);
    }

    // Make 'verts' point to the colors
    verts += sizeof(SkPoint);
    for (int i = 0; i < tess.numPts(); ++i) {
        if (tweakAlphaForCoverage) {
            SkASSERT(SkScalarRoundToInt(255.0f * tess.coverage(i)) <= 255);
            unsigned scale = SkScalarRoundToInt(255.0f * tess.coverage(i));
            GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = 
                    tess.coverage(i);
        }
    }

    for (int i = 0; i < tess.numIndices(); ++i) {
        idxs[i] = tess.index(i) + firstIndex;
    }
}

static const GrGeometryProcessor* create_fill_gp(bool tweakAlphaForCoverage,
                                                 const SkMatrix& viewMatrix,
                                                 bool usesLocalCoords,
                                                 bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;

    Color color(Color::kAttribute_Type);
    Coverage::Type coverageType;
    // TODO remove coverage if coverage is ignored
    /*if (coverageIgnored) {
        coverageType = Coverage::kNone_Type;
    } else*/ if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    Coverage coverage(coverageType);
    LocalCoords localCoords(usesLocalCoords ? LocalCoords::kUsePosition_Type :
                                              LocalCoords::kUnused_Type);
    return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

class AAFlatteningConvexPathBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkScalar fStrokeWidth;
        SkPaint::Join fJoin;
        SkScalar fMiterLimit;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(AAFlatteningConvexPathBatch, (geometry));
    }

    const char* name() const override { return "AAConvexBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
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
        fBatch.fLinesOnly = SkPath::kLine_SegmentMask == fGeoData[0].fPath.getSegmentMasks();
        fBatch.fCanTweakAlphaForCoverage = init.canTweakAlphaForCoverage();
    }

    void draw(GrBatchTarget* batchTarget, const GrPipeline* pipeline, int vertexCount, 
            size_t vertexStride, void* vertices, int indexCount, uint16_t* indices) {
        if (vertexCount == 0 || indexCount == 0) {
            return;
        }
        const GrVertexBuffer* vertexBuffer;
        GrVertices info;
        int firstVertex;
        void* verts = batchTarget->makeVertSpace(vertexStride, vertexCount, &vertexBuffer, 
                &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        memcpy(verts, vertices, vertexCount * vertexStride);

        const GrIndexBuffer* indexBuffer;
        int firstIndex;
        uint16_t* idxs = batchTarget->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
        if (!idxs) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        memcpy(idxs, indices, indexCount * sizeof(uint16_t));
        info.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer, firstVertex, 
                firstIndex, vertexCount, indexCount);
        batchTarget->draw(info);
    }
    
    void generateGeometry(GrBatchTarget* batchTarget) override {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        // Setup GrGeometryProcessor
        SkAutoTUnref<const GrGeometryProcessor> gp(create_fill_gp(canTweakAlphaForCoverage,
                                                                  this->viewMatrix(),
                                                                  this->usesLocalCoords(),
                                                                  this->coverageIgnored()));
        if (!gp) {
            SkDebugf("Couldn't create a GrGeometryProcessor\n");
            return;
        }

        batchTarget->initDraw(gp, this->pipeline());

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        int instanceCount = fGeoData.count();

        int vertexCount = 0;
        int indexCount = 0;
        int maxVertices = DEFAULT_BUFFER_SIZE;
        int maxIndices = DEFAULT_BUFFER_SIZE;
        uint8_t* vertices = (uint8_t*) malloc(maxVertices * vertexStride);
        uint16_t* indices = (uint16_t*) malloc(maxIndices * sizeof(uint16_t));
        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];
            GrAAConvexTessellator tess(args.fStrokeWidth, args.fJoin, args.fMiterLimit);

            if (!tess.tessellate(args.fViewMatrix, args.fPath)) {
                continue;
            }

            int currentIndices = tess.numIndices();
            SkASSERT(currentIndices <= UINT16_MAX);
            if (indexCount + currentIndices > UINT16_MAX) {
                // if we added the current instance, we would overflow the indices we can store in a 
                // uint16_t. Draw what we've got so far and reset.
                draw(batchTarget, this->pipeline(), vertexCount, vertexStride, vertices, indexCount, 
                        indices);
                vertexCount = 0;
                indexCount = 0;
            }
            int currentVertices = tess.numPts();
            if (vertexCount + currentVertices > maxVertices) {
                maxVertices = SkTMax(vertexCount + currentVertices, maxVertices * 2);
                vertices = (uint8_t*) realloc(vertices, maxVertices * vertexStride);
            }
            if (indexCount + currentIndices > maxIndices) {
                maxIndices = SkTMax(indexCount + currentIndices, maxIndices * 2);
                indices = (uint16_t*) realloc(indices, maxIndices * sizeof(uint16_t));
            }

            extract_verts(tess, vertices + vertexStride * vertexCount, vertexStride, args.fColor, 
                    vertexCount, indices + indexCount, canTweakAlphaForCoverage);
            vertexCount += currentVertices;
            indexCount += currentIndices;
        }
        draw(batchTarget, this->pipeline(), vertexCount, vertexStride, vertices, indexCount,
             indices);
        free(vertices);
        free(indices);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    AAFlatteningConvexPathBatch(const Geometry& geometry) {
        this->initClassID<AAFlatteningConvexPathBatch>();
        fGeoData.push_back(geometry);

        // compute bounds
        fBounds = geometry.fPath.getBounds();
        geometry.fViewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t) override {
        if (!this->pipeline()->isEqual(*t->pipeline())) {
            return false;
        }

        AAFlatteningConvexPathBatch* that = t->cast<AAFlatteningConvexPathBatch>();

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
            fBatch.fCanTweakAlphaForCoverage = false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool linesOnly() const { return fBatch.fLinesOnly; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fLinesOnly;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

bool GrAALinearizingConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    if (args.fPath->isEmpty()) {
        return true;
    }
    AAFlatteningConvexPathBatch::Geometry geometry;
    geometry.fColor = args.fColor;
    geometry.fViewMatrix = *args.fViewMatrix;
    geometry.fPath = *args.fPath;
    geometry.fStrokeWidth = args.fStroke->isFillStyle() ? -1.0f : args.fStroke->getWidth();
    geometry.fJoin = args.fStroke->isFillStyle() ? SkPaint::Join::kMiter_Join :
                                                   args.fStroke->getJoin();
    geometry.fMiterLimit = args.fStroke->getMiter();

    SkAutoTUnref<GrBatch> batch(AAFlatteningConvexPathBatch::Create(geometry));
    args.fTarget->drawBatch(*args.fPipelineBuilder, batch);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(AAFlatteningConvexPathBatch) {
    AAFlatteningConvexPathBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    geometry.fViewMatrix = GrTest::TestMatrixInvertible(random);
    geometry.fPath = GrTest::TestPathConvex(random);

    return AAFlatteningConvexPathBatch::Create(geometry);
}

#endif
