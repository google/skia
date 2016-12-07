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
#include "GrPipelineBuilder.h"
#include "GrProcessor.h"
#include "GrStyle.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "batches/GrMeshDrawOp.h"
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

    if (stroke.getStyle() == SkStrokeRec::kStroke_Style ||
        stroke.getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        if (!args.fViewMatrix->isSimilarity()) {
            return false;
        }
        SkScalar strokeWidth = args.fViewMatrix->getMaxScale() * stroke.getWidth();
        if (strokeWidth < 1.0f && stroke.getStyle() == SkStrokeRec::kStroke_Style) {
            return false;
        }
        return strokeWidth <= kMaxStrokeWidth &&
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

class AAFlatteningConvexPathBatch : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    AAFlatteningConvexPathBatch(GrColor color,
                                const SkMatrix& viewMatrix,
                                const SkPath& path,
                                SkScalar strokeWidth,
                                SkStrokeRec::Style style,
                                SkPaint::Join join,
                                SkScalar miterLimit) : INHERITED(ClassID()) {
        fGeoData.emplace_back(Geometry{ color, viewMatrix, path,
                                        strokeWidth, style, join, miterLimit });

        // compute bounds
        SkRect bounds = path.getBounds();
        SkScalar w = strokeWidth;
        if (w > 0) {
            w /= 2;
            // If the half stroke width is < 1 then we effectively fallback to bevel joins.
            if (SkPaint::kMiter_Join == join && w > 1.f) {
                w *= miterLimit;
            }
            bounds.outset(w, w);
        }
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
    }

    const char* name() const override { return "AAFlatteningConvexBatch"; }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& geo : fGeoData) {
            string.appendf("Color: 0x%08x, StrokeWidth: %.2f, Style: %d, Join: %d, "
                           "MiterLimit: %.2f\n",
                           geo.fColor, geo.fStrokeWidth, geo.fStyle, geo.fJoin, geo.fMiterLimit);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

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

    void draw(GrMeshDrawOp::Target* target, const GrGeometryProcessor* gp, int vertexCount,
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
            GrAAConvexTessellator tess(args.fStyle, args.fStrokeWidth,
                                       args.fJoin, args.fMiterLimit);

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

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
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

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(*that);
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

    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkScalar fStrokeWidth;
        SkStrokeRec::Style fStyle;
        SkPaint::Join fJoin;
        SkScalar fMiterLimit;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrMeshDrawOp INHERITED;
};

bool GrAALinearizingConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAALinearizingConvexPathRenderer::onDrawPath");
    SkASSERT(!args.fRenderTargetContext->isUnifiedMultisampled());
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(!args.fShape->style().pathEffect());

    SkPath path;
    args.fShape->asPath(&path);
    bool fill = args.fShape->style().isSimpleFill();
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();
    SkScalar strokeWidth = fill ? -1.0f : stroke.getWidth();
    SkPaint::Join join = fill ? SkPaint::Join::kMiter_Join : stroke.getJoin();
    SkScalar miterLimit = stroke.getMiter();

    sk_sp<GrDrawOp> batch(new AAFlatteningConvexPathBatch(args.fPaint->getColor(),
                                                          *args.fViewMatrix,
                                                          path, strokeWidth,
                                                          stroke.getStyle(),
                                                          join, miterLimit));

    GrPipelineBuilder pipelineBuilder(*args.fPaint);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fRenderTargetContext->addDrawOp(pipelineBuilder, *args.fClip, batch.get());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(AAFlatteningConvexPathBatch) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPath path = GrTest::TestPathConvex(random);

    SkStrokeRec::Style styles[3] = { SkStrokeRec::kFill_Style,
                                     SkStrokeRec::kStroke_Style, 
                                     SkStrokeRec::kStrokeAndFill_Style };

    SkStrokeRec::Style style = styles[random->nextU() % 3];

    SkScalar strokeWidth = -1.f;
    SkPaint::Join join = SkPaint::kMiter_Join;
    SkScalar miterLimit = 0.5f;

    if (SkStrokeRec::kFill_Style != style) {
        strokeWidth = random->nextRangeF(1.0f, 10.0f);
        if (random->nextBool()) {
            join = SkPaint::kMiter_Join;
        } else {
            join = SkPaint::kBevel_Join;
        }
        miterLimit = random->nextRangeF(0.5f, 2.0f);
    }

    return new AAFlatteningConvexPathBatch(color, viewMatrix, path, strokeWidth, 
                                           style, join, miterLimit);
}

#endif
