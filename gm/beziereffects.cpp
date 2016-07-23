/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrDrawContextPriv.h"
#include "GrContext.h"
#include "GrPathUtils.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"

#include "batches/GrTestBatch.h"

#include "effects/GrBezierEffect.h"

static inline SkScalar eval_line(const SkPoint& p, const SkScalar lineEq[3], SkScalar sign) {
    return sign * (lineEq[0] * p.fX + lineEq[1] * p.fY + lineEq[2]);
}

namespace skiagm {

class BezierCubicOrConicTestBatch : public GrTestBatch {
public:
    DEFINE_BATCH_CLASS_ID

    const char* name() const override { return "BezierCubicOrConicTestBatch"; }

    BezierCubicOrConicTestBatch(sk_sp<GrGeometryProcessor> gp, const SkRect& bounds,
                                GrColor color, const SkScalar klmEqs[9], SkScalar sign)
        : INHERITED(ClassID(), bounds, color)
        , fGeometryProcessor(std::move(gp)) {
        for (int i = 0; i < 9; i++) {
            fKlmEqs[i] = klmEqs[i];
        }
        fSign = sign;
    }

private:

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) const override {
        QuadHelper helper;
        size_t vertexStride = fGeometryProcessor->getVertexStride();
        SkASSERT(vertexStride == sizeof(Vertex));
        Vertex* verts = reinterpret_cast<Vertex*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }
        const SkRect& bounds = this->bounds();
        verts[0].fPosition.setRectFan(bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom,
                                      sizeof(Vertex));
        for (int v = 0; v < 4; ++v) {
            verts[v].fKLM[0] = eval_line(verts[v].fPosition, fKlmEqs + 0, fSign);
            verts[v].fKLM[1] = eval_line(verts[v].fPosition, fKlmEqs + 3, fSign);
            verts[v].fKLM[2] = eval_line(verts[v].fPosition, fKlmEqs + 6, 1.f);
        }
        helper.recordDraw(target, fGeometryProcessor.get());
    }

    SkScalar                   fKlmEqs[9];
    SkScalar                   fSign;
    sk_sp<GrGeometryProcessor> fGeometryProcessor;

    static const int kVertsPerCubic = 4;
    static const int kIndicesPerCubic = 6;

    typedef GrTestBatch INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierCubicEffects : public GM {
public:
    BezierCubicEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("bezier_cubic_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        struct Vertex {
            SkPoint fPosition;
            float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
        };

        static const int kNumCubics = 15;
        SkRandom rand;

        // Mult by 3 for each edge effect type
        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumCubics*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumCubics*3) / numCols);
        SkScalar w = SkIntToScalar(drawContext->width()) / numCols;
        SkScalar h = SkIntToScalar(drawContext->height()) / numRows;
        int row = 0;
        int col = 0;
        static const GrColor color = 0xff000000;

        for (int i = 0; i < kNumCubics; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            for(int edgeType = 0; edgeType < kGrProcessorEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrPrimitiveEdgeType et = (GrPrimitiveEdgeType)edgeType;
                gp = GrCubicEffect::Make(color, SkMatrix::I(), et, *context->caps());
                if (!gp) {
                    continue;
                }
                SkScalar x = SkScalarMul(col, w);
                SkScalar y = SkScalarMul(row, h);
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY},
                    {x + baseControlPts[3].fX, y + baseControlPts[3].fY}
                };
                SkPoint chopped[10];
                SkScalar klmEqs[9];
                SkScalar klmSigns[3];
                int cnt = GrPathUtils::chopCubicAtLoopIntersection(controlPts,
                                                                   chopped,
                                                                   klmEqs,
                                                                   klmSigns);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                for (int i = 0; i < 4; ++i) {
                    canvas->drawCircle(controlPts[i].fX, controlPts[i].fY, 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = chopped + 3 * c;

                    for (int i = 0; i < 4; ++i) {
                        canvas->drawCircle(pts[i].fX, pts[i].fY, 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.set(pts, 4);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

                    SkAutoTUnref<GrDrawBatch> batch(
                        new BezierCubicOrConicTestBatch(gp, bounds, color, klmEqs, klmSigns[c]));

                    drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
                }
                ++col;
                if (numCols == col) {
                    col = 0;
                    ++row;
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierConicEffects : public GM {
public:
    BezierConicEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("bezier_conic_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }


    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        struct Vertex {
            SkPoint fPosition;
            float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
        };

        static const int kNumConics = 10;
        SkRandom rand;

        // Mult by 3 for each edge effect type
        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumConics*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumConics*3) / numCols);
        SkScalar w = SkIntToScalar(drawContext->width()) / numCols;
        SkScalar h = SkIntToScalar(drawContext->height()) / numRows;
        int row = 0;
        int col = 0;
        static const GrColor color = 0xff000000;

        for (int i = 0; i < kNumConics; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            SkScalar weight = rand.nextRangeF(0.f, 2.f);
            for(int edgeType = 0; edgeType < kGrProcessorEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrPrimitiveEdgeType et = (GrPrimitiveEdgeType)edgeType;
                gp = GrConicEffect::Make(color, SkMatrix::I(), et,
                                         *context->caps(), SkMatrix::I(), false);
                if (!gp) {
                    continue;
                }

                SkScalar x = SkScalarMul(col, w);
                SkScalar y = SkScalarMul(row, h);
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY}
                };
                SkConic dst[4];
                SkScalar klmEqs[9];
                int cnt = chop_conic(controlPts, dst, weight);
                GrPathUtils::getConicKLM(controlPts, weight, klmEqs);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i].fX, controlPts[i].fY, 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = dst[c].fPts;
                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i].fX, pts[i].fY, 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    //SkPoint bPts[] = {{0.f, 0.f}, {800.f, 800.f}};
                    //bounds.set(bPts, 2);
                    bounds.set(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

                    SkAutoTUnref<GrDrawBatch> batch(
                        new BezierCubicOrConicTestBatch(gp, bounds, color, klmEqs, 1.f));

                    drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
                }
                ++col;
                if (numCols == col) {
                    col = 0;
                    ++row;
                }
            }
        }
    }

private:
    // Uses the max curvature function for quads to estimate
    // where to chop the conic. If the max curvature is not
    // found along the curve segment it will return 1 and
    // dst[0] is the original conic. If it returns 2 the dst[0]
    // and dst[1] are the two new conics.
    int split_conic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
        SkScalar t = SkFindQuadMaxCurvature(src);
        if (t == 0) {
            if (dst) {
                dst[0].set(src, weight);
            }
            return 1;
        } else {
            if (dst) {
                SkConic conic;
                conic.set(src, weight);
                conic.chopAt(t, dst);
            }
            return 2;
        }
    }

    // Calls split_conic on the entire conic and then once more on each subsection.
    // Most cases will result in either 1 conic (chop point is not within t range)
    // or 3 points (split once and then one subsection is split again).
    int chop_conic(const SkPoint src[3], SkConic dst[4], const SkScalar weight) {
        SkConic dstTemp[2];
        int conicCnt = split_conic(src, dstTemp, weight);
        if (2 == conicCnt) {
            int conicCnt2 = split_conic(dstTemp[0].fPts, dst, dstTemp[0].fW);
            conicCnt = conicCnt2 + split_conic(dstTemp[1].fPts, &dst[conicCnt2], dstTemp[1].fW);
        } else {
            dst[0] = dstTemp[0];
        }
        return conicCnt;
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class BezierQuadTestBatch : public GrTestBatch {
public:
    DEFINE_BATCH_CLASS_ID
    const char* name() const override { return "BezierQuadTestBatch"; }

    BezierQuadTestBatch(sk_sp<GrGeometryProcessor> gp, const SkRect& bounds, GrColor color,
                        const GrPathUtils::QuadUVMatrix& devToUV)
        : INHERITED(ClassID(), bounds, color)
        , fDevToUV(devToUV)
        , fGeometryProcessor(std::move(gp)) {
    }

private:

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) const override {
        QuadHelper helper;
        size_t vertexStride = fGeometryProcessor->getVertexStride();
        SkASSERT(vertexStride == sizeof(Vertex));
        Vertex* verts = reinterpret_cast<Vertex*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }
        const SkRect& bounds = this->bounds();
        verts[0].fPosition.setRectFan(bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom,
                                      sizeof(Vertex));
        fDevToUV.apply<4, sizeof(Vertex), sizeof(SkPoint)>(verts);
        helper.recordDraw(target, fGeometryProcessor.get());
    }

    GrPathUtils::QuadUVMatrix  fDevToUV;
    sk_sp<GrGeometryProcessor> fGeometryProcessor;

    static const int kVertsPerCubic = 4;
    static const int kIndicesPerCubic = 6;

    typedef GrTestBatch INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier quad curves in the GPU backend.
 */
class BezierQuadEffects : public GM {
public:
    BezierQuadEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("bezier_quad_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }


    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (!drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        struct Vertex {
            SkPoint fPosition;
            float   fUV[4]; // The last two values are ignored. The effect expects a vec4f.
        };

        static const int kNumQuads = 5;
        SkRandom rand;

        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumQuads*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumQuads*3) / numCols);
        SkScalar w = SkIntToScalar(drawContext->width()) / numCols;
        SkScalar h = SkIntToScalar(drawContext->height()) / numRows;
        int row = 0;
        int col = 0;
        static const GrColor color = 0xff000000;

        for (int i = 0; i < kNumQuads; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            for(int edgeType = 0; edgeType < kGrProcessorEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrPrimitiveEdgeType et = (GrPrimitiveEdgeType)edgeType;
                gp = GrQuadEffect::Make(color, SkMatrix::I(), et,
                                        *context->caps(), SkMatrix::I(), false);
                if (!gp) {
                    continue;
                }

                SkScalar x = SkScalarMul(col, w);
                SkScalar y = SkScalarMul(row, h);
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY}
                };
                SkPoint chopped[5];
                int cnt = SkChopQuadAtMaxCurvature(controlPts, chopped);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i].fX, controlPts[i].fY, 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = chopped + 2 * c;

                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i].fX, pts[i].fY, 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.set(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

                    GrPathUtils::QuadUVMatrix DevToUV(pts);

                    SkAutoTUnref<GrDrawBatch> batch(
                        new BezierQuadTestBatch(gp, bounds, color, DevToUV));

                    drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
                }
                ++col;
                if (numCols == col) {
                    col = 0;
                    ++row;
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return new BezierCubicEffects;)
DEF_GM(return new BezierConicEffects;)
DEF_GM(return new BezierQuadEffects;)
}

#endif
