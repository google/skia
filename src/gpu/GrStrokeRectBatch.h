/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeRectBatch_DEFINED
#define GrStrokeRectBatch_DEFINED

#include "GrBatch.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"

class GrStrokeRectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkScalar fStrokeWidth;
    };

    static GrBatch* Create(const Geometry& geometry, bool snapToPixelCenters) {
        return SkNEW_ARGS(GrStrokeRectBatch, (geometry, snapToPixelCenters));
    }

    const char* name() const override { return "GrStrokeRectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
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
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
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

        batchTarget->initDraw(gp, pipeline);

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(GrDefaultGeoProcFactory::PositionAttr));

        Geometry& args = fGeoData[0];

        int vertexCount = kVertsPerHairlineRect;
        if (args.fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* verts = batchTarget->makeVertSpace(vertexStride, vertexCount,
                                                 &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        GrPrimitiveType primType;

        if (args.fStrokeWidth > 0) {;
            primType = kTriangleStrip_GrPrimitiveType;
            args.fRect.sort();
            this->setStrokeRectStrip(vertex, args.fRect, args.fStrokeWidth);
        } else {
            // hairline
            primType = kLineStrip_GrPrimitiveType;
            vertex[0].set(args.fRect.fLeft, args.fRect.fTop);
            vertex[1].set(args.fRect.fRight, args.fRect.fTop);
            vertex[2].set(args.fRect.fRight, args.fRect.fBottom);
            vertex[3].set(args.fRect.fLeft, args.fRect.fBottom);
            vertex[4].set(args.fRect.fLeft, args.fRect.fTop);
        }

        GrVertices vertices;
        vertices.init(primType, vertexBuffer, firstVertex, vertexCount);
        batchTarget->draw(vertices);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    GrStrokeRectBatch(const Geometry& geometry, bool snapToPixelCenters) {
        this->initClassID<GrStrokeRectBatch>();

        fBatch.fHairline = geometry.fStrokeWidth == 0;

        fGeoData.push_back(geometry);

        // setup bounds
        fBounds = geometry.fRect;
        SkScalar rad = SkScalarHalf(geometry.fStrokeWidth);
        fBounds.outset(rad, rad);
        geometry.fViewMatrix.mapRect(&fBounds);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (snapToPixelCenters) {
            fBounds.roundOut();
        }
    }

    /*  create a triangle strip that strokes the specified rect. There are 8
     unique vertices, but we repeat the last 2 to close up. Alternatively we
     could use an indices array, and then only send 8 verts, but not sure that
     would be faster.
     */
    void setStrokeRectStrip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
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


    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool hairline() const { return fBatch.fHairline; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override {
        //if (!this->pipeline()->isEqual(*t->pipeline())) {
        //    return false;
        //}
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
};

#endif
