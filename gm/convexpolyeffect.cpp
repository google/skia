/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkTLList.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrOp.h"

#include <memory>
#include <utility>

class GrAppliedClip;

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

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkRect& rect) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<PolyBoundsOp>(std::move(paint), rect);
    }

    const char* name() const override { return "PolyBoundsOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        return fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
                fsaaType, caps, clampType, &fColor);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    PolyBoundsOp(GrPaint&& paint, const SkRect& rect)
            : INHERITED(ClassID())
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint))
            , fRect(outset(rect)) {
        this->setBounds(sorted_rect(fRect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepareDraws(Target* target) override {
        using namespace GrDefaultGeoProcFactory;

        Color color(fColor);
        sk_sp<GrGeometryProcessor> gp(GrDefaultGeoProcFactory::Make(
                target->caps().shaderCaps(),
                color,
                Coverage::kSolid_Type,
                LocalCoords::kUnused_Type,
                SkMatrix::I()));

        SkASSERT(gp->vertexStride() == sizeof(SkPoint));
        QuadHelper helper(target, sizeof(SkPoint), 1);
        SkPoint* verts = reinterpret_cast<SkPoint*>(helper.vertices());
        if (!verts) {
            return;
        }

        SkPointPriv::SetRectTriStrip(verts, fRect, sizeof(SkPoint));
        helper.recordDraw(target, std::move(gp));
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, std::move(fProcessors));
    }

    SkPMColor4f fColor;
    GrProcessorSet fProcessors;
    SkRect fRect;

    typedef GrMeshDrawOp INHERITED;
};

/**
 * This GM directly exercises a GrProcessor that draws convex polygons.
 */
class ConvexPolyEffect : public GpuGM {
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
            SkPoint point = { SkScalarCos(angle), SkScalarSin(angle) };
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

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        SkScalar y = 0;
        constexpr SkScalar kDX = 12.f;
        for (PathList::Iter iter(fPaths, PathList::Iter::kHead_IterStart);
             iter.get();
             iter.next()) {
            const SkPath* path = iter.get();
            SkScalar x = 0;

            for (int et = 0; et < kGrClipEdgeTypeCnt; ++et) {
                const SkMatrix m = SkMatrix::MakeTrans(x, y);
                SkPath p;
                path->transform(m, &p);

                GrClipEdgeType edgeType = (GrClipEdgeType) et;
                std::unique_ptr<GrFragmentProcessor> fp(GrConvexPolyEffect::Make(edgeType, p));
                if (!fp) {
                    continue;
                }

                GrPaint grPaint;
                grPaint.setColor4f({ 0, 0, 0, 1.f });
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                std::unique_ptr<GrDrawOp> op =
                        PolyBoundsOp::Make(context, std::move(grPaint), p.getBounds());
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

            for (int et = 0; et < kGrClipEdgeTypeCnt; ++et) {
                SkRect rect = *iter.get();
                rect.offset(x, y);
                GrClipEdgeType edgeType = (GrClipEdgeType) et;
                std::unique_ptr<GrFragmentProcessor> fp(GrConvexPolyEffect::Make(edgeType, rect));
                if (!fp) {
                    continue;
                }

                GrPaint grPaint;
                grPaint.setColor4f({ 0, 0, 0, 1.f });
                grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                grPaint.addCoverageFragmentProcessor(std::move(fp));

                std::unique_ptr<GrDrawOp> op = PolyBoundsOp::Make(context, std::move(grPaint),
                                                                  rect);
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
