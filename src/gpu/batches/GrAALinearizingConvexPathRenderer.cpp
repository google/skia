/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAALinearizingConvexPathRenderer.h"

#include "GrAAConvexTessellator.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrPipelineBuilder.h"
#include "GrStyle.h"
#include "SkGeometry.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "SkPathPriv.h"
#include "batches/GrVertexBatch.h"
#include "glsl/GrGLSLGeometryProcessor.h"

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
    if (!args.fShape->knownToBeConvex()) {
        return false;
    }
    if (args.fShape->style().pathEffect()) {
        return false;
    }
    if (args.fShape->inverseFilled()) {
        return false;
    }
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();
    if (stroke.getStyle() == SkStrokeRec::kStroke_Style) {
        if (!args.fViewMatrix->isSimilarity()) {
            return false;
        }
        SkScalar strokeWidth = args.fViewMatrix->getMaxScale() * stroke.getWidth();
        return strokeWidth >= 1.0f && strokeWidth <= kMaxStrokeWidth &&
               args.fShape->knownToBeClosed() &&
               stroke.getJoin() != SkPaint::Join::kRound_Join;
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

static sk_sp<GrGeometryProcessor> create_fill_gp(bool tweakAlphaForCoverage,
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
    return MakeForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

class AAFlatteningConvexPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkScalar fStrokeWidth;
        SkPaint::Join fJoin;
        SkScalar fMiterLimit;
    };

    static GrDrawBatch* Create(const Geometry& geometry) {
        return new AAFlatteningConvexPathBatch(geometry);
    }

    const char* name() const override { return "AAConvexBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !overrides.readsColor();
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
        fBatch.fCoverageIgnored = !overrides.readsCoverage();
        fBatch.fLinesOnly = SkPath::kLine_SegmentMask == fGeoData[0].fPath.getSegmentMasks();
        fBatch.fCanTweakAlphaForCoverage = overrides.canTweakAlphaForCoverage();
    }

    void draw(GrVertexBatch::Target* target, const GrGeometryProcessor* gp, int vertexCount,
              size_t vertexStride, void* vertices, int indexCount, uint16_t* indices) const {
        if (vertexCount == 0 || indexCount == 0) {
            return;
        }
        const GrBuffer* vertexBuffer;
        GrMesh mesh;
        int firstVertex;
        void* verts = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer,
                                              &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        memcpy(verts, vertices, vertexCount * vertexStride);

        const GrBuffer* indexBuffer;
        int firstIndex;
        uint16_t* idxs = target->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
        if (!idxs) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        memcpy(idxs, indices, indexCount * sizeof(uint16_t));
        mesh.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer, firstVertex,
                         firstIndex, vertexCount, indexCount);
        target->draw(gp, mesh);
    }

    void onPrepareDraws(Target* target) const override {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        // Setup GrGeometryProcessor
        sk_sp<GrGeometryProcessor> gp(create_fill_gp(canTweakAlphaForCoverage,
                                                     this->viewMatrix(),
                                                     this->usesLocalCoords(),
                                                     this->coverageIgnored()));
        if (!gp) {
            SkDebugf("Couldn't create a GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        int instanceCount = fGeoData.count();

        int vertexCount = 0;
        int indexCount = 0;
        int maxVertices = DEFAULT_BUFFER_SIZE;
        int maxIndices = DEFAULT_BUFFER_SIZE;
        uint8_t* vertices = (uint8_t*) sk_malloc_throw(maxVertices * vertexStride);
        uint16_t* indices = (uint16_t*) sk_malloc_throw(maxIndices * sizeof(uint16_t));
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];
            GrAAConvexTessellator tess(args.fStrokeWidth, args.fJoin, args.fMiterLimit);

            if (!tess.tessellate(args.fViewMatrix, args.fPath)) {
                continue;
            }

            int currentIndices = tess.numIndices();
            SkASSERT(currentIndices <= UINT16_MAX);
            if (indexCount + currentIndices > UINT16_MAX) {
                // if we added the current instance, we would overflow the indices we can store in a
                // uint16_t. Draw what we've got so far and reset.
                this->draw(target, gp.get(),
                           vertexCount, vertexStride, vertices, indexCount, indices);
                vertexCount = 0;
                indexCount = 0;
            }
            int currentVertices = tess.numPts();
            if (vertexCount + currentVertices > maxVertices) {
                maxVertices = SkTMax(vertexCount + currentVertices, maxVertices * 2);
                vertices = (uint8_t*) sk_realloc_throw(vertices, maxVertices * vertexStride);
            }
            if (indexCount + currentIndices > maxIndices) {
                maxIndices = SkTMax(indexCount + currentIndices, maxIndices * 2);
                indices = (uint16_t*) sk_realloc_throw(indices, maxIndices * sizeof(uint16_t));
            }

            extract_verts(tess, vertices + vertexStride * vertexCount, vertexStride, args.fColor,
                    vertexCount, indices + indexCount, canTweakAlphaForCoverage);
            vertexCount += currentVertices;
            indexCount += currentIndices;
        }
        this->draw(target, gp.get(), vertexCount, vertexStride, vertices, indexCount, indices);
        sk_free(vertices);
        sk_free(indices);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    AAFlatteningConvexPathBatch(const Geometry& geometry) : INHERITED(ClassID()) {
        fGeoData.push_back(geometry);

        // compute bounds
        fBounds = geometry.fPath.getBounds();
        SkScalar w = geometry.fStrokeWidth;
        if (w > 0) {
            w /= 2;
            // If the miter limit is < 1 then we effectively fallback to bevel joins.
            if (SkPaint::kMiter_Join == geometry.fJoin && w > 1.f) {
                w *= geometry.fMiterLimit;
            }
            fBounds.outset(w, w);
        }
        geometry.fViewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        AAFlatteningConvexPathBatch* that = t->cast<AAFlatteningConvexPathBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

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

    typedef GrVertexBatch INHERITED;
};

bool GrAALinearizingConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrAALinearizingConvexPathRenderer::onDrawPath");
    SkASSERT(!args.fDrawContext->isUnifiedMultisampled());
    SkASSERT(!args.fShape->isEmpty());

    AAFlatteningConvexPathBatch::Geometry geometry;
    geometry.fColor = args.fColor;
    geometry.fViewMatrix = *args.fViewMatrix;
    args.fShape->asPath(&geometry.fPath);
    bool fill = args.fShape->style().isSimpleFill();
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();
    geometry.fStrokeWidth = fill ? -1.0f : stroke.getWidth();
    geometry.fJoin = fill ? SkPaint::Join::kMiter_Join : stroke.getJoin();
    geometry.fMiterLimit = stroke.getMiter();

    SkAutoTUnref<GrDrawBatch> batch(AAFlatteningConvexPathBatch::Create(geometry));

    GrPipelineBuilder pipelineBuilder(*args.fPaint);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(AAFlatteningConvexPathBatch) {
    AAFlatteningConvexPathBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    geometry.fViewMatrix = GrTest::TestMatrixInvertible(random);
    geometry.fPath = GrTest::TestPathConvex(random);

    return AAFlatteningConvexPathBatch::Create(geometry);
}

#endif
