/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNonAAStrokeRectBatch.h"

#include "GrBatchTest.h"
#include "GrBatchFlushState.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrVertexBatch.h"
#include "SkRandom.h"

/*  create a triangle strip that strokes the specified rect. There are 8
    unique vertices, but we repeat the last 2 to close up. Alternatively we
    could use an indices array, and then only send 8 verts, but not sure that
    would be faster.
    */
static void init_stroke_rect_strip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
    const SkScalar rad = SkScalarHalf(width);
    // TODO we should be able to enable this assert, but we'd have to filter these draws
    // this is a bug
    //SkASSERT(rad < rect.width() / 2 && rad < rect.height() / 2);

    verts[0].set(rect.fLeft + rad, rect.fTop + rad);
    verts[1].set(rect.fLeft - rad, rect.fTop - rad);
    verts[2].set(rect.fRight - rad, rect.fTop + rad);
    verts[3].set(rect.fRight + rad, rect.fTop - rad);
    verts[4].set(rect.fRight - rad, rect.fBottom - rad);
    verts[5].set(rect.fRight + rad, rect.fBottom + rad);
    verts[6].set(rect.fLeft + rad, rect.fBottom - rad);
    verts[7].set(rect.fLeft - rad, rect.fBottom + rad);
    verts[8] = verts[0];
    verts[9] = verts[1];
}

class NonAAStrokeRectBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkScalar fStrokeWidth;
        GrColor fColor;
    };

    static NonAAStrokeRectBatch* Create() {
        return new NonAAStrokeRectBatch;
    }

    const char* name() const override { return "GrStrokeRectBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setKnownSingleComponent(0xff);
    }

    void append(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                SkScalar strokeWidth) {
        Geometry& geometry = fGeoData.push_back();
        geometry.fViewMatrix = viewMatrix;
        geometry.fRect = rect;
        geometry.fStrokeWidth = strokeWidth;
        geometry.fColor = color;

        // Sort the rect for hairlines
        geometry.fRect.sort();
    }

    void appendAndUpdateBounds(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                               SkScalar strokeWidth, bool snapToPixelCenters) {
        this->append(color, viewMatrix, rect, strokeWidth);

        SkRect bounds;
        this->setupBounds(&bounds, fGeoData.back(), snapToPixelCenters);
        this->joinBounds(bounds);
    }

    void init(bool snapToPixelCenters) {
        const Geometry& geo = fGeoData[0];
        fBatch.fHairline = geo.fStrokeWidth == 0;

        // setup bounds
        this->setupBounds(&fBounds, geo, snapToPixelCenters);
    }

private:
    void setupBounds(SkRect* bounds, const Geometry& geo, bool snapToPixelCenters) {
        *bounds = geo.fRect;
        SkScalar rad = SkScalarHalf(geo.fStrokeWidth);
        bounds->outset(rad, rad);
        geo.fViewMatrix.mapRect(&fBounds);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (snapToPixelCenters) {
            bounds->roundOut();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkAutoTUnref<const GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            Coverage coverage(this->coverageIgnored() ? Coverage::kSolid_Type :
                                                        Coverage::kNone_Type);
            LocalCoords localCoords(this->usesLocalCoords() ? LocalCoords::kUsePosition_Type :
                                                              LocalCoords::kUnused_Type);
            gp.reset(GrDefaultGeoProcFactory::Create(color, coverage, localCoords,
                                                     this->viewMatrix()));
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(GrDefaultGeoProcFactory::PositionAttr));

        const Geometry& args = fGeoData[0];

        int vertexCount = kVertsPerHairlineRect;
        if (args.fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        const GrBuffer* vertexBuffer;
        int firstVertex;

        void* verts = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer,
                                              &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        GrPrimitiveType primType;
        if (args.fStrokeWidth > 0) {
            primType = kTriangleStrip_GrPrimitiveType;
            init_stroke_rect_strip(vertex, args.fRect, args.fStrokeWidth);
        } else {
            // hairline
            primType = kLineStrip_GrPrimitiveType;
            vertex[0].set(args.fRect.fLeft, args.fRect.fTop);
            vertex[1].set(args.fRect.fRight, args.fRect.fTop);
            vertex[2].set(args.fRect.fRight, args.fRect.fBottom);
            vertex[3].set(args.fRect.fLeft, args.fRect.fBottom);
            vertex[4].set(args.fRect.fLeft, args.fRect.fTop);
        }

        GrMesh mesh;
        mesh.init(primType, vertexBuffer, firstVertex, vertexCount);
        target->draw(gp, mesh);
    }

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
    }

    NonAAStrokeRectBatch() : INHERITED(ClassID()) {}

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool hairline() const { return fBatch.fHairline; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override {
        // if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *t->pipeline(),
        //     t->bounds(), caps)) {
        //     return false;
        // }
        // GrStrokeRectBatch* that = t->cast<StrokeRectBatch>();

        // NonAA stroke rects cannot batch right now
        // TODO make these batchable
        return false;
    }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fHairline;
    };

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

namespace GrNonAAStrokeRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    SkScalar strokeWidth,
                    bool snapToPixelCenters) {
    NonAAStrokeRectBatch* batch = NonAAStrokeRectBatch::Create();
    batch->append(color, viewMatrix, rect, strokeWidth);
    batch->init(snapToPixelCenters);
    return batch;
}

void Append(GrBatch* origBatch,
            GrColor color,
            const SkMatrix& viewMatrix,
            const SkRect& rect,
            SkScalar strokeWidth,
            bool snapToPixelCenters) {
    NonAAStrokeRectBatch* batch = origBatch->cast<NonAAStrokeRectBatch>();
    batch->appendAndUpdateBounds(color, viewMatrix, rect, strokeWidth, snapToPixelCenters);
}

};

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(NonAAStrokeRectBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    SkRect rect = GrTest::TestRect(random);
    SkScalar strokeWidth = random->nextBool() ? 0.0f : 1.0f;

    return GrNonAAStrokeRectBatch::Create(color, viewMatrix, rect, strokeWidth, random->nextBool());
}

#endif
