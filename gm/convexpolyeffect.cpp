/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawContextPriv.h"
#include "GrPathUtils.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkTLList.h"

#include "batches/GrTestBatch.h"
#include "batches/GrVertexBatch.h"

#include "effects/GrConvexPolyEffect.h"

/** outset rendered rect to visualize anti-aliased poly edges */
static SkRect outset(const SkRect& unsorted) {
    SkRect r = unsorted;
    r.outset(5.f, 5.f);
    return r;
}

/** sorts a rect */
static SkRect sorted_rect(const SkRect& unsorted) {
    SkRect r = unsorted;
    r.sort();
    return r;
}

namespace skiagm {
class PolyBoundsBatch : public GrTestBatch {
public:
    DEFINE_BATCH_CLASS_ID

    const char* name() const override { return "PolyBoundsBatch"; }

    PolyBoundsBatch(const SkRect& rect, GrColor color)
        : INHERITED(ClassID(), outset(sorted_rect(rect)), color)
        , fRect(outset(rect)) {
    }

private:
    void onPrepareDraws(Target* target) const override {
        using namespace GrDefaultGeoProcFactory;

        Color color(this->color());
        Coverage coverage(Coverage::kSolid_Type);
        LocalCoords localCoords(LocalCoords::kUnused_Type);
        sk_sp<GrGeometryProcessor> gp(
            GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I()));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(SkPoint));
        QuadHelper helper;
        SkPoint* verts = reinterpret_cast<SkPoint*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }

        fRect.toQuad(verts);

        helper.recordDraw(target, gp.get());
    }

    SkRect fRect;

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

        SkPath linePath;
        linePath.moveTo(5.f, 5.f);
        linePath.lineTo(6.f, 6.f);
        fPaths.addToTail(linePath);

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
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkScalar y = 0;
        static const SkScalar kDX = 12.f;
        for (PathList::Iter iter(fPaths, PathList::Iter::kHead_IterStart);
             iter.get();
             iter.next()) {
            const SkPath* path = iter.get();
            SkScalar x = 0;

            for (int et = 0; et < kGrProcessorEdgeTypeCnt; ++et) {
                const SkMatrix m = SkMatrix::MakeTrans(x, y);
                SkPath p;
                path->transform(m, &p);

                GrPrimitiveEdgeType edgeType = (GrPrimitiveEdgeType) et;
                sk_sp<GrFragmentProcessor> fp(GrConvexPolyEffect::Make(edgeType, p));
                if (!fp) {
                    continue;
                }

                GrPaint grPaint;
                grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                SkAutoTUnref<GrDrawBatch> batch(new PolyBoundsBatch(p.getBounds(), 0xff000000));

                drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);

                x += SkScalarCeilToScalar(path->getBounds().width() + kDX);
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

        for (RectList::Iter iter(fRects, RectList::Iter::kHead_IterStart);
             iter.get();
             iter.next()) {

            SkScalar x = 0;

            for (int et = 0; et < kGrProcessorEdgeTypeCnt; ++et) {
                SkRect rect = *iter.get();
                rect.offset(x, y);
                GrPrimitiveEdgeType edgeType = (GrPrimitiveEdgeType) et;
                sk_sp<GrFragmentProcessor> fp(GrConvexPolyEffect::Make(edgeType, rect));
                if (!fp) {
                    continue;
                }

                GrPaint grPaint;
                grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                SkAutoTUnref<GrDrawBatch> batch(new PolyBoundsBatch(rect, 0xff000000));

                drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);

                x += SkScalarCeilToScalar(rect.width() + kDX);
            }

            // Draw rect without and with AA using normal API for reference
            canvas->save();
            canvas->translate(x, y);
            SkPaint paint;
            canvas->drawRect(*iter.get(), paint);
            x += SkScalarCeilToScalar(iter.get()->width() + kDX);
            paint.setAntiAlias(true);
            canvas->drawRect(*iter.get(), paint);
            canvas->restore();

            y += SkScalarCeilToScalar(iter.get()->height() + 20.f);
        }
    }

private:
    typedef SkTLList<SkPath, 1> PathList;
    typedef SkTLList<SkRect, 1> RectList;
    PathList fPaths;
    RectList fRects;

    typedef GM INHERITED;
};

DEF_GM(return new ConvexPolyEffect;)
}

#endif
