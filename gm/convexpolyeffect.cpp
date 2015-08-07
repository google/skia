
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrBatchTarget.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPathUtils.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGeometry.h"
#include "SkTLList.h"

#include "batches/GrTestBatch.h"

#include "effects/GrConvexPolyEffect.h"

namespace skiagm {

class ConvexPolyTestBatch : public GrTestBatch {
public:
    struct Geometry : public GrTestBatch::Geometry {
        SkRect fBounds;
    };

    const char* name() const override { return "ConvexPolyTestBatch"; }

    static GrBatch* Create(const GrGeometryProcessor* gp, const Geometry& geo) {
        return SkNEW_ARGS(ConvexPolyTestBatch, (gp, geo));
    }

private:
    ConvexPolyTestBatch(const GrGeometryProcessor* gp, const Geometry& geo)
        : INHERITED(gp, geo.fBounds)
        , fGeometry(geo) {
        this->initClassID<ConvexPolyTestBatch>();
    }

    Geometry* geoData(int index) override {
        SkASSERT(0 == index);
        return &fGeometry;
    }

    const Geometry* geoData(int index) const override {
        SkASSERT(0 == index);
        return &fGeometry;
    }

    void onGenerateGeometry(GrBatchTarget* batchTarget) override {
        size_t vertexStride = this->geometryProcessor()->getVertexStride();
        SkASSERT(vertexStride == sizeof(SkPoint));
        QuadHelper helper;
        SkPoint* verts = reinterpret_cast<SkPoint*>(helper.init(batchTarget, vertexStride, 1));
        if (!verts) {
            return;
        }

        // Make sure any artifacts around the exterior of path are visible by using overly
        // conservative bounding geometry.
        fGeometry.fBounds.outset(5.f, 5.f);
        fGeometry.fBounds.toQuad(verts);

        helper.issueDraw(batchTarget);
    }

    Geometry fGeometry;

    typedef GrTestBatch INHERITED;
};

/**
 * This GM directly exercises a GrProcessor that draws convex polygons.
 */
class ConvexPolyEffect : public GM {
public:
    ConvexPolyEffect() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("convex_poly_effect");
    }

    SkISize onISize() override {
        return SkISize::Make(720, 800);
    }

    void onOnceBeforeDraw() override {
        SkPath tri;
        tri.moveTo(5.f, 5.f);
        tri.lineTo(100.f, 20.f);
        tri.lineTo(15.f, 100.f);

        fPaths.addToTail(tri);
        fPaths.addToTail(SkPath())->reverseAddPath(tri);

        tri.close();
        fPaths.addToTail(tri);

        SkPath ngon;
        static const SkScalar kRadius = 50.f;
        const SkPoint center = { kRadius, kRadius };
        for (int i = 0; i < GrConvexPolyEffect::kMaxEdges; ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / GrConvexPolyEffect::kMaxEdges;
            SkPoint point;
            point.fY = SkScalarSinCos(angle, &point.fX);
            point.scale(kRadius);
            point = center + point;
            if (0 == i) {
                ngon.moveTo(point);
            } else {
                ngon.lineTo(point);
            }
        }

        fPaths.addToTail(ngon);
        SkMatrix scaleM;
        scaleM.setScale(1.1f, 0.4f);
        ngon.transform(scaleM);
        fPaths.addToTail(ngon);

        // integer edges
        fRects.addToTail(SkRect::MakeLTRB(5.f, 1.f, 30.f, 25.f));
        // half-integer edges
        fRects.addToTail(SkRect::MakeLTRB(5.5f, 0.5f, 29.5f, 24.5f));
        // vertically/horizontally thin rects that cover pixel centers
        fRects.addToTail(SkRect::MakeLTRB(5.25f, 0.5f, 5.75f, 24.5f));
        fRects.addToTail(SkRect::MakeLTRB(5.5f,  0.5f, 29.5f, 0.75f));
        // vertically/horizontally thin rects that don't cover pixel centers
        fRects.addToTail(SkRect::MakeLTRB(5.55f, 0.5f, 5.75f, 24.5f));
        fRects.addToTail(SkRect::MakeLTRB(5.5f, .05f, 29.5f, .25f));
        // small in x and y
        fRects.addToTail(SkRect::MakeLTRB(5.05f, .55f, 5.45f, .85f));
        // inverted in x and y
        fRects.addToTail(SkRect::MakeLTRB(100.f, 50.5f, 5.f, 0.5f));
    }

    void onDraw(SkCanvas* canvas) override {
        using namespace GrDefaultGeoProcFactory;
        GrRenderTarget* rt = canvas->internal_private_accessTopLayerRenderTarget();
        if (NULL == rt) {
            this->drawGpuOnlyMessage(canvas);
            return;
        }
        GrContext* context = rt->getContext();
        if (NULL == context) {
            return;
        }

        Color color(0xff000000);
        Coverage coverage(Coverage::kSolid_Type);
        LocalCoords localCoords(LocalCoords::kUnused_Type);
        SkAutoTUnref<const GrGeometryProcessor> gp(
                GrDefaultGeoProcFactory::Create(color, coverage, localCoords, SkMatrix::I()));

        SkScalar y = 0;
        for (SkTLList<SkPath>::Iter iter(fPaths, SkTLList<SkPath>::Iter::kHead_IterStart);
             iter.get();
             iter.next()) {
            const SkPath* path = iter.get();
            SkScalar x = 0;

            for (int et = 0; et < kGrProcessorEdgeTypeCnt; ++et) {
                GrTestTarget tt;
                context->getTestTarget(&tt);
                if (NULL == tt.target()) {
                    SkDEBUGFAIL("Couldn't get Gr test target.");
                    return;
                }
                const SkMatrix m = SkMatrix::MakeTrans(x, y);
                SkPath p;
                path->transform(m, &p);

                GrPrimitiveEdgeType edgeType = (GrPrimitiveEdgeType) et;
                SkAutoTUnref<GrFragmentProcessor> fp(GrConvexPolyEffect::Create(edgeType, p));
                if (!fp) {
                    continue;
                }

                GrPipelineBuilder pipelineBuilder;
                pipelineBuilder.addCoverageProcessor(fp);
                pipelineBuilder.setRenderTarget(rt);

                ConvexPolyTestBatch::Geometry geometry;
                geometry.fColor = color.fColor;
                geometry.fBounds = p.getBounds();

                SkAutoTUnref<GrBatch> batch(ConvexPolyTestBatch::Create(gp, geometry));

                tt.target()->drawBatch(pipelineBuilder, batch);

                x += SkScalarCeilToScalar(path->getBounds().width() + 10.f);
            }

            // Draw AA and non AA paths using normal API for reference.
            canvas->save();
            canvas->translate(x, y);
            SkPaint paint;
            canvas->drawPath(*path, paint);
            canvas->translate(path->getBounds().width() + 10.f, 0);
            paint.setAntiAlias(true);
            canvas->drawPath(*path, paint);
            canvas->restore();

            y += SkScalarCeilToScalar(path->getBounds().height() + 20.f);
        }

        for (SkTLList<SkRect>::Iter iter(fRects, SkTLList<SkRect>::Iter::kHead_IterStart);
             iter.get();
             iter.next()) {

            SkScalar x = 0;

            for (int et = 0; et < kGrProcessorEdgeTypeCnt; ++et) {
                GrTestTarget tt;
                context->getTestTarget(&tt);
                if (NULL == tt.target()) {
                    SkDEBUGFAIL("Couldn't get Gr test target.");
                    return;
                }
                SkRect rect = *iter.get();
                rect.offset(x, y);
                GrPrimitiveEdgeType edgeType = (GrPrimitiveEdgeType) et;
                SkAutoTUnref<GrFragmentProcessor> fp(GrConvexPolyEffect::Create(edgeType, rect));
                if (!fp) {
                    continue;
                }

                GrPipelineBuilder pipelineBuilder;
                pipelineBuilder.addCoverageProcessor(fp);
                pipelineBuilder.setRenderTarget(rt);

                ConvexPolyTestBatch::Geometry geometry;
                geometry.fColor = color.fColor;
                geometry.fBounds = rect;

                SkAutoTUnref<GrBatch> batch(ConvexPolyTestBatch::Create(gp, geometry));

                tt.target()->drawBatch(pipelineBuilder, batch);

                x += SkScalarCeilToScalar(rect.width() + 10.f);
            }

            // Draw rect without and with AA using normal API for reference
            canvas->save();
            canvas->translate(x, y);
            SkPaint paint;
            canvas->drawRect(*iter.get(), paint);
            x += SkScalarCeilToScalar(iter.get()->width() + 10.f);
            paint.setAntiAlias(true);
            canvas->drawRect(*iter.get(), paint);
            canvas->restore();

            y += SkScalarCeilToScalar(iter.get()->height() + 20.f);
        }
    }

private:
    SkTLList<SkPath> fPaths;
    SkTLList<SkRect> fRects;

    typedef GM INHERITED;
};

DEF_GM( return SkNEW(ConvexPolyEffect); )
}

#endif
