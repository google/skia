
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAALinearizingConvexPathRenderer.h"

#include "GrAAConvexTessellator.h"
#include "GrBatch.h"
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

bool GrAALinearizingConvexPathRenderer::canDrawPath(const GrDrawTarget* target,
                                                    const GrPipelineBuilder*,
                                                    const SkMatrix& viewMatrix,
                                                    const SkPath& path,
                                                    const GrStrokeInfo& stroke,
                                                    bool antiAlias) const {
    if (!antiAlias) {
        return false;
    }
    if (path.isInverseFillType()) {
        return false;
    }
    if (!path.isConvex()) {
        return false;
    }
    if (stroke.getStyle() == SkStrokeRec::kStroke_Style) {
        return viewMatrix.isSimilarity() && stroke.getWidth() >= 1.0f && 
                stroke.getWidth() <= kMaxStrokeWidth && !stroke.isDashed() && 
                SkPathPriv::LastVerbIsClose(path) && stroke.getJoin() != SkPaint::Join::kRound_Join;
    }
    return stroke.getStyle() == SkStrokeRec::kFill_Style;
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
                                                 const SkMatrix& localMatrix,
                                                 bool usesLocalCoords,
                                                 bool coverageIgnored) {
    uint32_t flags = GrDefaultGeoProcFactory::kColor_GPType;
    if (!tweakAlphaForCoverage) {
        flags |= GrDefaultGeoProcFactory::kCoverage_GPType;
    }

    return GrDefaultGeoProcFactory::Create(flags, GrColor_WHITE, usesLocalCoords, coverageIgnored,
                                           SkMatrix::I(), localMatrix);
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
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
        fBatch.fLinesOnly = SkPath::kLine_SegmentMask == fGeoData[0].fPath.getSegmentMasks();
        fBatch.fCanTweakAlphaForCoverage = init.fCanTweakAlphaForCoverage;
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
    
    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        SkMatrix invert;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        // Setup GrGeometryProcessor
        SkAutoTUnref<const GrGeometryProcessor> gp(
                                                create_fill_gp(canTweakAlphaForCoverage, invert,
                                                               this->usesLocalCoords(),
                                                               this->coverageIgnored()));

        batchTarget->initDraw(gp, pipeline);

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
                draw(batchTarget, pipeline, vertexCount, vertexStride, vertices, indexCount, 
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
        draw(batchTarget, pipeline, vertexCount, vertexStride, vertices, indexCount, indices);
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

bool GrAALinearizingConvexPathRenderer::onDrawPath(GrDrawTarget* target,
                                                   GrPipelineBuilder* pipelineBuilder,
                                                   GrColor color,
                                                   const SkMatrix& vm,
                                                   const SkPath& path,
                                                   const GrStrokeInfo& stroke,
                                                   bool antiAlias) {
    if (path.isEmpty()) {
        return true;
    }
    AAFlatteningConvexPathBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fViewMatrix = vm;
    geometry.fPath = path;
    geometry.fStrokeWidth = stroke.isFillStyle() ? -1.0f : stroke.getWidth();
    geometry.fJoin = stroke.isFillStyle() ? SkPaint::Join::kMiter_Join : stroke.getJoin();
    geometry.fMiterLimit = stroke.getMiter();

    SkAutoTUnref<GrBatch> batch(AAFlatteningConvexPathBatch::Create(geometry));
    target->drawBatch(pipelineBuilder, batch);

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
