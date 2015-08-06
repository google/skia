/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeRectBatch.h"
#include "GrBatchTest.h"
#include "SkRandom.h"

GrStrokeRectBatch::GrStrokeRectBatch(const Geometry& geometry, bool snapToPixelCenters) {
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

void GrStrokeRectBatch::initBatchTracker(const GrPipelineInfo& init) {
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


void GrStrokeRectBatch::generateGeometry(GrBatchTarget* batchTarget) {
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

    batchTarget->initDraw(gp, this->pipeline());

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

    GrVertices vertices;
    vertices.init(primType, vertexBuffer, firstVertex, vertexCount);
    batchTarget->draw(vertices);
}

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(GrStrokeRectBatch) {
    GrStrokeRectBatch::Geometry geometry;
    geometry.fViewMatrix = GrTest::TestMatrix(random);
    geometry.fColor = GrRandomColor(random);
    geometry.fRect = GrTest::TestRect(random);
    geometry.fStrokeWidth = random->nextBool() ? 0.0f : 1.0f;

    return GrStrokeRectBatch::Create(geometry, random->nextBool());
}

#endif
