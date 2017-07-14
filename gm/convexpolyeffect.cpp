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
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkTLList.h"
#include "effects/GrConvexPolyEffect.h"
#include "ops/GrMeshDrawOp.h"

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
class PolyBoundsOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "PolyBoundsOp"; }

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkRect& rect) {
        return std::unique_ptr<GrDrawOp>(new PolyBoundsOp(std::move(paint), rect));
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        auto analysis = fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, false, caps, &fColor);
        return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
    }

private:
    PolyBoundsOp(GrPaint&& paint, const SkRect& rect)
            : INHERITED(ClassID())
            , fColor(paint.getColor())
            , fProcessors(std::move(paint))
            , fRect(outset(rect)) {
        this->setBounds(sorted_rect(fRect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) const override {
        using namespace GrDefaultGeoProcFactory;

        Color color(fColor);
        sk_sp<GrGeometryProcessor> gp(GrDefaultGeoProcFactory::Make(
                color, Coverage::kSolid_Type, LocalCoords::kUnused_Type, SkMatrix::I()));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(SkPoint));
        QuadHelper helper;
        SkPoint* verts = reinterpret_cast<SkPoint*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }

        fRect.toQuad(verts);

        helper.recordDraw(target, gp.get(), target->makePipeline(0, &fProcessors));
    }

    bool onCombineIfPossible(GrOp* op, const GrCaps& caps) override { return false; }

    GrColor fColor;
    GrProcessorSet fProcessors;
    SkRect fRect;

    typedef GrMeshDrawOp INHERITED;
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
        constexpr SkScalar kRadius = 50.f;
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
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkScalar y = 0;
        constexpr SkScalar kDX = 12.f;
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
                grPaint.setColor4f(GrColor4f(0, 0, 0, 1.f));
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                std::unique_ptr<GrDrawOp> op =
                        PolyBoundsOp::Make(std::move(grPaint), p.getBounds());
                renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));

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
                grPaint.setColor4f(GrColor4f(0, 0, 0, 1.f));
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                std::unique_ptr<GrDrawOp> op = PolyBoundsOp::Make(std::move(grPaint), rect);
                renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));

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
